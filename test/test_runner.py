#!/usr/bin/env python3
"""
test_runner.py — AASM assembler + RV32IM emulator + test harness
=================================================================

Usage:
    python3 test_runner.py [test_dir]

    test_dir defaults to ./test  (relative to this script).

For every *.aasm file found it:
  1. Assembles the source into a flat binary in memory.
  2. Executes it on a minimal RV32IM soft-core (max 1 000 000 steps).
  3. Compares final register/memory values against per-test expectations.

Exit code: 0 if all tests pass, 1 otherwise.
"""

import os, re, sys, struct
from pathlib import Path

# ─────────────────────────────────────────────────────────────────────────────
#  Helpers
# ─────────────────────────────────────────────────────────────────────────────

MASK32 = 0xFFFF_FFFF

def u32(v):  return int(v) & MASK32
def s32(v):  return struct.unpack("<i", struct.pack("<I", u32(v)))[0]

def sign_ext(v, bits):
    v = int(v) & ((1 << bits) - 1)
    if v >> (bits - 1):
        v -= (1 << bits)
    return v

# ─────────────────────────────────────────────────────────────────────────────
#  Register name table
# ─────────────────────────────────────────────────────────────────────────────

_ABI = [
    "ZERO","RA","SP","GP","TP","T0","T1","T2",
    "S0","S1","A0","A1","A2","A3","A4","A5",
    "A6","A7","S2","S3","S4","S5","S6","S7",
    "S8","S9","S10","S11","T3","T4","T5","T6",
]
REG_MAP = {name: i for i, name in enumerate(_ABI)}
REG_MAP.update({f"X{i}": i for i in range(32)})

# ─────────────────────────────────────────────────────────────────────────────
#  Assembler
# ─────────────────────────────────────────────────────────────────────────────

class AssemblerError(Exception):
    pass

class Assembler:
    """Two-pass assembler for AASM."""

    # Tokeniser: produces uppercase tokens, strips comments
    _TOK = re.compile(
        r"#[^\n]*"                              # comment  (discarded)
        r"|\.(?:8BYTE|4BYTE|2BYTE|BYTE|CODE|DATA)"  # directives (longest first)
        r"|&[A-Z0-9_]+"                         # label def
        r"|@[A-Z0-9_]+"                         # label ref
        r"|-?\d+"                               # signed integer literal
        r"|[A-Z][A-Z0-9]*"                      # mnemonic / register
        r"|[(),]",                              # punctuation
        re.ASCII,
    )

    _MNEMONICS = re.compile(
        r"^(?:ADD|ADDI|SUB|AND|ANDI|OR|ORI|XOR|XORI"
        r"|SLL|SLLI|SRL|SRLI|SRA|SRAI"
        r"|SLT|SLTI|SLTU|SLTIU"
        r"|LB|LH|LW|LBU|LHU"
        r"|SB|SH|SW"
        r"|BEQ|BNE|BLT|BGE|BLTU|BGEU"
        r"|JAL|JALR|LUI|AUIPC"
        r"|MUL|MULH|MULHSU|MULHU|DIV|DIVU|REM|REMU)$"
    )

    def __init__(self, source: str):
        self.source = source
        self.binary = bytearray()
        self.labels = {}          # name -> byte address

    def _tokenise(self):
        raw = self.source.upper()
        return [m.group() for m in self._TOK.finditer(raw)
                if not m.group().startswith("#")]

    def _is_mnemonic(self, t):
        return bool(self._MNEMONICS.match(t))

    # ── pass 1: simulate PC to resolve label addresses ─────────────────────

    def _pass1(self, tokens):
        pc = 0
        i  = 0
        n  = len(tokens)
        while i < n:
            t = tokens[i]
            if t in (".CODE", ".DATA"):
                i += 1
            elif t in (".BYTE",".2BYTE",".4BYTE",".8BYTE"):
                pc += {".BYTE":1,".2BYTE":2,".4BYTE":4,".8BYTE":8}[t]
                i += 2          # directive + value
            elif t.startswith("&"):
                self.labels[t[1:]] = pc
                i += 1
            elif t.startswith("@") or t in REG_MAP or re.fullmatch(r"-?\d+", t) or t in (",","(",")"):
                i += 1
            elif self._is_mnemonic(t):
                pc += 4
                i  += 1
                # skip operand tokens until next mnemonic / label / directive
                while i < n:
                    nx = tokens[i]
                    if nx.startswith("&") or nx in (".CODE",".DATA",".BYTE",".2BYTE",".4BYTE",".8BYTE") or self._is_mnemonic(nx):
                        break
                    i += 1
            else:
                i += 1          # unknown token, skip

    # ── pass 2 tokeniser helpers ───────────────────────────────────────────


    def assemble(self) -> bytearray:
        tokens = self._tokenise()
        self._pass1(tokens)
        self._pass2(tokens)
        return self.binary


# ─────────────────────────────────────────────────────────────────────────────
#  RV32IM Emulator
# ─────────────────────────────────────────────────────────────────────────────

class Emulator:
    MEM_SIZE = 0x0010_0000   # 1 MiB

    def __init__(self, binary: bytearray, base: int = 0):
        self.mem  = bytearray(self.MEM_SIZE)
        self.regs = [0] * 32
        self.pc   = base
        self.base = base
        self.mem[base : base + len(binary)] = binary

    def _addr(self, a): return a & (self.MEM_SIZE - 1)

    def _lb(self, a):  return sign_ext(self.mem[self._addr(a)], 8)
    def _lh(self, a):  return sign_ext(struct.unpack_from("<H", self.mem, self._addr(a))[0], 16)
    def _lw(self, a):  return s32(struct.unpack_from("<I", self.mem, self._addr(a))[0])
    def _lbu(self, a): return self.mem[self._addr(a)]
    def _lhu(self, a): return struct.unpack_from("<H", self.mem, self._addr(a))[0]
    def _sb(self, a, v): self.mem[self._addr(a)] = u32(v) & 0xFF
    def _sh(self, a, v): struct.pack_into("<H", self.mem, self._addr(a), u32(v) & 0xFFFF)
    def _sw(self, a, v): struct.pack_into("<I", self.mem, self._addr(a), u32(v))

    def _wr(self, rd, v):
        if rd: self.regs[rd] = u32(v)

    def step(self) -> bool:
        pc   = self.pc
        word = struct.unpack_from("<I", self.mem, self._addr(pc))[0]
        op   = word & 0x7F
        npc  = pc + 4

        RD   = (word >>  7) & 0x1F
        F3   = (word >> 12) & 0x7
        RS1  = (word >> 15) & 0x1F
        RS2  = (word >> 20) & 0x1F
        F7   = (word >> 25) & 0x7F

        r1s  = s32(self.regs[RS1])
        r1u  = self.regs[RS1]
        r2s  = s32(self.regs[RS2])
        r2u  = self.regs[RS2]

        def imm_i(): return sign_ext(word >> 20, 12)
        def imm_s(): return sign_ext(((word>>25)<<5)|((word>>7)&0x1F), 12)
        def imm_b(): return sign_ext(
            (((word>>31)&1)<<12)|(((word>>7)&1)<<11)|
            (((word>>25)&0x3F)<<5)|(((word>>8)&0xF)<<1), 13)
        def imm_u(): return (word & 0xFFFFF000)
        def imm_j(): return sign_ext(
            (((word>>31)&1)<<20)|(((word>>12)&0xFF)<<12)|
            (((word>>20)&1)<<11)|(((word>>21)&0x3FF)<<1), 21)

        if op == 0x37:   # LUI
            self._wr(RD, u32(imm_u()))
        elif op == 0x17: # AUIPC
            self._wr(RD, u32(pc + imm_u()))
        elif op == 0x6F: # JAL
            target = pc + imm_j()
            self._wr(RD, u32(npc))
            npc = target
            if RD == 0 and target == pc:
                self.pc = pc; return False
        elif op == 0x67: # JALR
            target = (r1u + imm_i()) & ~1
            self._wr(RD, u32(npc))
            npc = target
        elif op == 0x63: # BRANCH
            taken = {
                0x0: r1s == r2s, 0x1: r1s != r2s,
                0x4: r1s <  r2s, 0x5: r1s >= r2s,
                0x6: r1u <  r2u, 0x7: r1u >= r2u,
            }.get(F3, False)
            if taken: npc = pc + imm_b()
        elif op == 0x03: # LOAD
            addr = u32(r1u + imm_i())
            v = {0x0:self._lb,0x1:self._lh,0x2:self._lw,
                 0x4:self._lbu,0x5:self._lhu}.get(F3, lambda a:0)(addr)
            self._wr(RD, u32(v))
        elif op == 0x23: # STORE
            addr = u32(r1u + imm_s())
            {0x0:self._sb,0x1:self._sh,0x2:self._sw}.get(F3, lambda a,v:None)(addr, r2u)
        elif op == 0x13: # OP-IMM
            shamt = (word >> 20) & 0x1F
            v = imm_i()
            res = {
                0x0: lambda: u32(r1s + v),
                0x2: lambda: 1 if r1s < v else 0,
                0x3: lambda: 1 if r1u < u32(v) else 0,
                0x4: lambda: u32(r1u ^ u32(v)),
                0x6: lambda: u32(r1u | u32(v)),
                0x7: lambda: u32(r1u & u32(v)),
                0x1: lambda: u32(r1u << shamt),
                0x5: lambda: u32(r1s >> shamt) if F7==0x20 else u32(r1u >> shamt),
            }.get(F3, lambda: 0)()
            self._wr(RD, res)
        elif op == 0x33: # OP / OP-M
            if F7 == 0x01:  # RV32M
                res = {
                    0x0: lambda: u32(r1s * r2s),
                    0x1: lambda: u32((r1s * r2s) >> 32),
                    0x2: lambda: u32((r1s * r2u) >> 32),
                    0x3: lambda: u32((r1u * r2u) >> 32),
                    0x4: lambda: u32(0 if r2s==0 else int(r1s/r2s)),
                    0x5: lambda: u32(0 if r2u==0 else r1u//r2u),
                    0x6: lambda: u32(r1s if r2s==0 else r1s % r2s),
                    0x7: lambda: u32(r1u if r2u==0 else r1u % r2u),
                }.get(F3, lambda: 0)()
            else:
                res = {
                    0x0: lambda: u32(r1s - r2s) if F7==0x20 else u32(r1s + r2s),
                    0x1: lambda: u32(r1u << (r2u&31)),
                    0x2: lambda: 1 if r1s < r2s else 0,
                    0x3: lambda: 1 if r1u < r2u else 0,
                    0x4: lambda: u32(r1u ^ r2u),
                    0x5: lambda: u32(r1s >> (r2u&31)) if F7==0x20 else u32(r1u >> (r2u&31)),
                    0x6: lambda: u32(r1u | r2u),
                    0x7: lambda: u32(r1u & r2u),
                }.get(F3, lambda: 0)()
            self._wr(RD, res)

        self.pc = npc
        return True

    def run(self, max_steps=1_000_000):
        for step in range(max_steps):
            if not self.step():
                return step + 1
        return max_steps


# ─────────────────────────────────────────────────────────────────────────────
#  Fix: _pass2 uses nested functions with nonlocal i — rewrite reg() cleanly
# ─────────────────────────────────────────────────────────────────────────────
# The class above has a Python scoping bug in reg() (double nonlocal reference).
# Patch it by monkey-patching assemble() to use a standalone _pass2_fixed().

def _pass2_fixed(self, tokens):
    pos = [0]      # mutable int via list so inner funcs can write it
    pc  = [0]

    n = len(tokens)

    def eat_commas():
        while pos[0] < n and tokens[pos[0]] == ",":
            pos[0] += 1

    def eat(expected):
        if pos[0] < n and tokens[pos[0]] == expected:
            pos[0] += 1

    def reg():
        eat_commas()
        t = tokens[pos[0]]; pos[0] += 1
        if t not in REG_MAP:
            raise AssemblerError(f"expected register, got '{t}'")
        return REG_MAP[t]

    def imm_val():
        eat_commas()
        t = tokens[pos[0]]; pos[0] += 1
        if re.fullmatch(r"-?\d+", t):
            return int(t)
        raise AssemblerError(f"expected immediate, got '{t}'")

    def mem():
        off = imm_val()
        eat("(")
        r = reg()
        eat(")")
        return off, r

    def lbl_or_imm():
        eat_commas()
        t = tokens[pos[0]]; pos[0] += 1
        if t.startswith("@"):
            name = t[1:]
            if name not in self.labels:
                raise AssemblerError(f"undefined label '{name}'")
            return self.labels[name] - pc[0]
        if re.fullmatch(r"-?\d+", t):
            return int(t)
        raise AssemblerError(f"expected label-ref or imm, got '{t}'")

    def e32(w):
        self.binary += struct.pack("<I", u32(w))
        pc[0] += 4

    def r_enc(op, rd, rs1, rs2, f3, f7):
        e32((f7<<25)|(rs2<<20)|(rs1<<15)|(f3<<12)|(rd<<7)|op)

    def i_enc(op, rd, rs1, imm12, f3):
        v = u32(sign_ext(imm12, 12)) & 0xFFF
        e32((v<<20)|(rs1<<15)|(f3<<12)|(rd<<7)|op)

    def s_enc(op, rs1, rs2, imm12, f3):
        v = u32(sign_ext(imm12, 12))
        e32(((v>>5&0x7F)<<25)|(rs2<<20)|(rs1<<15)|(f3<<12)|((v&0x1F)<<7)|op)

    def b_enc(op, rs1, rs2, off, f3):
        o = u32(sign_ext(off, 13))
        e32(((o>>12&1)<<31)|((o>>5&0x3F)<<25)|(rs2<<20)|(rs1<<15)|
            (f3<<12)|((o>>1&0xF)<<8)|((o>>11&1)<<7)|op)

    def u_enc(op, rd, imm20):
        e32(((imm20 & 0xFFFFF) << 12) | (rd << 7) | op)

    def j_enc(op, rd, off):
        o = u32(sign_ext(off, 21))
        e32(((o>>20&1)<<31)|((o>>12&0xFF)<<12)|((o>>11&1)<<20)|
            ((o>>1&0x3FF)<<21)|(rd<<7)|op)

    R3 = {
        "ADD":(0x33,0x0,0x00),"SUB":(0x33,0x0,0x20),
        "SLL":(0x33,0x1,0x00),"SLT":(0x33,0x2,0x00),
        "SLTU":(0x33,0x3,0x00),"XOR":(0x33,0x4,0x00),
        "SRL":(0x33,0x5,0x00),"SRA":(0x33,0x5,0x20),
        "OR" :(0x33,0x6,0x00),"AND":(0x33,0x7,0x00),
        "MUL":(0x33,0x0,0x01),"MULH":(0x33,0x1,0x01),
        "MULHSU":(0x33,0x2,0x01),"MULHU":(0x33,0x3,0x01),
        "DIV":(0x33,0x4,0x01),"DIVU":(0x33,0x5,0x01),
        "REM":(0x33,0x6,0x01),"REMU":(0x33,0x7,0x01),
    }
    I_ALU = {
        "ADDI":(0x13,0x0),"SLTI":(0x13,0x2),"SLTIU":(0x13,0x3),
        "XORI":(0x13,0x4),"ORI" :(0x13,0x6),"ANDI" :(0x13,0x7),
    }
    LOADS   = {"LB":(0x03,0x0),"LH":(0x03,0x1),"LW":(0x03,0x2),
               "LBU":(0x03,0x4),"LHU":(0x03,0x5)}
    STORES  = {"SB":(0x23,0x0),"SH":(0x23,0x1),"SW":(0x23,0x2)}
    BRANCHES= {"BEQ":(0x63,0x0),"BNE":(0x63,0x1),
               "BLT":(0x63,0x4),"BGE":(0x63,0x5),
               "BLTU":(0x63,0x6),"BGEU":(0x63,0x7)}

    while pos[0] < n:
        t = tokens[pos[0]]

        if t in (".CODE", ".DATA") or t.startswith("&"):
            pos[0] += 1; continue

        if t in (".BYTE",".2BYTE",".4BYTE",".8BYTE"):
            nb = {".BYTE":1,".2BYTE":2,".4BYTE":4,".8BYTE":8}[t]
            pos[0] += 1
            v = int(tokens[pos[0]]); pos[0] += 1
            fmt = {1:"<B",2:"<H",4:"<I",8:"<Q"}[nb]
            self.binary += struct.pack(fmt, int(v) & ((1<<(nb*8))-1))
            pc[0] += nb
            continue

        if not self._is_mnemonic(t):
            pos[0] += 1; continue

        pos[0] += 1
        mn = t

        if mn in R3:
            op,f3,f7 = R3[mn]
            r_enc(op, reg(), reg(), reg(), f3, f7)
        elif mn in I_ALU:
            op,f3 = I_ALU[mn]
            i_enc(op, reg(), reg(), imm_val(), f3)
        elif mn in ("SLLI","SRLI","SRAI"):
            f3 = {"SLLI":0x1,"SRLI":0x5,"SRAI":0x5}[mn]
            f7 = 0x20 if mn == "SRAI" else 0x00
            rd = reg(); rs1 = reg(); shamt = imm_val() & 0x1F
            i_enc(0x13, rd, rs1, (f7<<5)|shamt, f3)
        elif mn in LOADS:
            op,f3 = LOADS[mn]
            rd = reg(); off,rs1 = mem()
            i_enc(op, rd, rs1, off, f3)
        elif mn in STORES:
            op,f3 = STORES[mn]
            rs2 = reg(); off,rs1 = mem()
            s_enc(op, rs1, rs2, off, f3)
        elif mn in BRANCHES:
            op,f3 = BRANCHES[mn]
            b_enc(op, reg(), reg(), lbl_or_imm(), f3)
        elif mn in ("LUI","AUIPC"):
            op = 0x37 if mn == "LUI" else 0x17
            u_enc(op, reg(), imm_val())
        elif mn == "JAL":
            j_enc(0x6F, reg(), lbl_or_imm())
        elif mn == "JALR":
            rd = reg(); rs1 = reg(); v = imm_val()
            i_enc(0x67, rd, rs1, v, 0x0)
        else:
            raise AssemblerError(f"unknown mnemonic '{mn}'")

# Bind the fixed pass2 to the class
Assembler._pass2 = _pass2_fixed


# ─────────────────────────────────────────────────────────────────────────────
#  Per-test expectations
# ─────────────────────────────────────────────────────────────────────────────

EXPECTATIONS = {
    "test03.aasm": {
        1:10, 2:3, 3:13, 4:7, 5:2, 6:11, 7:9,
    },
    "test04.aasm": {
        1:8, 2:2, 3:32, 4:2, 5:2, 6:64, 7:4, 8:4,
    },
    "test05.aasm": {
        1:5, 2:10, 3:1, 4:0, 5:1, 6:1, 7:0, 8:1,
    },
    "test06.aasm": {
        2:42, 3:42, 4:42, 5:42, 6:42, 7:42, 8:42,
    },
    "test07.aasm": {
        1:0, 2:42, 3:255,
    },
    "test08.aasm": {
        1:5, 2:5, 3:1,
    },
    "test09.aasm": {
        1: u32(1 << 12),
        2: u32(65536 << 12),
        3: u32(524287 << 12),
    },
    "test10.aasm": {
        2:99, 3:42,
    },
    "test11.aasm": {
        1:6, 2:7, 3:42, 4:0, 5:0, 6:0,
    },
    "test12.aasm": {
        1:20, 2:3, 3:6, 4:6, 5:2, 6:2, 7:1, 8:0,
    },
    "test13.aasm": {
        # .DATA at addr 0: [0x01, 0x02, 0xE8,0x03, 0x78,0x56,0x34,0x12]
        # X1 = LUI 65536 -> 65536<<12 = 0x10000000; wraps mod MEM_SIZE (1MiB=0x100000)
        # 0x10000000 & 0xFFFFF = 0x00000  → addr 0
        2: 1,
        3: 1000,
        4: 305419896,
    },
    "test14.aasm": {
        1:0, 2:1,
    },
    "test15.aasm": {
        # after 10 iters: a=fib(10)=55 in X1, b=fib(11)=89 in X2, counter=0
        1:55, 3:0,
    },
    "test16.aasm": {
        1:7,
    },
    "test17.aasm": {
        3:27,
    },
    "test18.aasm": {
        5:99,
    },
    "test19.aasm": {
        1:255, 2:15, 3:240, 4:0, 5:240, 6:255, 7:0, 8:u32(-1),
    },
    "test20.aasm": {
        3:0,
    },
    "test21.aasm": {
        3:42,
    },
    "test22.aasm": {
        1:u32(-1),    2:u32(-128),   3:u32(-2048),
        4:u32(-1),    5:u32(-16),    6:u32(0x0FFF_FFFF),
        7:128,        8:u32(-2049),
    },
    "test23.aasm": {
        "__mem_checks": [(512, [1,3,4,5,8])],
    },
    "test24.aasm": {
        2:32,
    },
    "test25.aasm": {
        1:6, 2:0,
    },
    "test26.aasm": {
        3:243, 2:0,
    },
    "test27.aasm": {
        "__mem_byte_checks": [(512, [8,7,6,5,4,3,2,1])],
    },
    "test28.aasm": {
        3:360,
    },
    "test29.aasm": {
        3:1,
    },
    "test30.aasm": {
        6:2,
    },
    "test31.aasm": {
        3:3, 4:2,
    },
    "test32.aasm": {
        2:u32(0x8000_0000),
    },
    "test33.aasm": {
        10:120,
    },
    "test34.aasm": {
        5:u32(0xEFBE_ADDE),
    },
    "test35.aasm": {
        # LUI X10, 74565 → X10 = 74565 << 12 = 0x12345000
        # bytes: 0x00, 0x50, 0x34, 0x12
        1: 0x00,
        2: 0x50,
        3: 0x34,
        4: 0x12,
    },
    "test36.aasm": {
        2:100, 3:200, 4:300, 5:600,
    },
    "test37.aasm": {
        3:1, 4:1,
    },
    "test38.aasm": {
        # loop exits when X2 = 91
        2:91,
    },
    "test39.aasm": {
        # X1 = LUI 1 → 0x1000 = 4096
        # X4 = low16(4096) = 4096  (since 4096 < 65536)
        4:4096,
        6:12288,   # 4096 * 3
        8:4096,    # 4096 + 0
    },
    "test40.aasm": {
        # base=512; i=1..8 → words at 512+4*i; odd→0, even→1
        "__mem_checks": [(512+4, [0,1,0,1,0,1,0,1])],
    },
    "test41.aasm": {
        3:45,
    },
    "test42.aasm": {
        10:4, 11:0, 12:4, 13:0, 14:1,
    },
}


# ─────────────────────────────────────────────────────────────────────────────
#  Test runner
# ─────────────────────────────────────────────────────────────────────────────

RESET = "\033[0m"; GREEN = "\033[32m"; RED = "\033[31m"
YELLOW = "\033[33m"; BOLD = "\033[1m"; DIM = "\033[2m"

def run_test(path: Path, expected: dict) -> bool:
    try:
        src    = path.read_text()
        binary = Assembler(src).assemble()
    except AssemblerError as e:
        print(f"  {RED}ASSEMBLE ERROR{RESET}: {e}"); return False
    except Exception as e:
        print(f"  {RED}ASSEMBLE CRASH{RESET}: {e}"); return False

    try:
        emu   = Emulator(binary, base=0)
        steps = emu.run(max_steps=1_000_000)
    except Exception as e:
        print(f"  {RED}EMULATOR CRASH{RESET}: {e}"); return False

    failures = []

    for key, exp_val in expected.items():
        if key == "__mem_checks":
            for base_addr, words in exp_val:
                for j, w in enumerate(words):
                    addr = base_addr + j * 4
                    got  = struct.unpack_from("<I", emu.mem, addr)[0]
                    if got != u32(w):
                        failures.append(
                            f"mem[0x{addr:04X}] expected {w} ({u32(w):#010x}) "
                            f"got {got} ({got:#010x})")
            continue

        if key == "__mem_byte_checks":
            for base_addr, blist in exp_val:
                for j, b in enumerate(blist):
                    got = emu.mem[base_addr + j]
                    if got != b:
                        failures.append(
                            f"mem[0x{base_addr+j:04X}] expected {b} got {got}")
            continue

        got = emu.regs[key]
        exp = u32(exp_val)
        if got != exp:
            failures.append(
                f"X{key:<2} expected {s32(exp):12d} ({exp:#010x}) "
                f"got {s32(got):12d} ({got:#010x})")

    if failures:
        print(f"  {RED}FAIL{RESET}  [{steps:>7} steps]")
        for f in failures:
            print(f"         {YELLOW}✗ {f}{RESET}")
        return False
    else:
        print(f"  {GREEN}PASS{RESET}  [{steps:>7} steps]")
        return True


def main():
    test_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent / "test"
    if not test_dir.is_dir():
        print(f"test directory not found: {test_dir}", file=sys.stderr); sys.exit(1)

    files = sorted(test_dir.glob("*.aasm"))
    if not files:
        print(f"no .aasm files in {test_dir}", file=sys.stderr); sys.exit(1)

    passed = failed = skipped = 0
    col = 28
    print(f"\n{BOLD}AASM test runner{RESET}  ({len(files)} files in {test_dir})\n")

    for path in files:
        name  = path.name
        label = f"{name:<{col}}"
        if name not in EXPECTATIONS:
            print(f"{label}  {DIM}SKIP  (no expectations){RESET}")
            skipped += 1; continue
        print(label, end="", flush=True)
        ok = run_test(path, EXPECTATIONS[name])
        passed += ok; failed += not ok

    total = passed + failed
    print(f"\n{'─'*52}")
    print(f"  {GREEN}{passed}{RESET}/{total} passed"
          + (f"  {RED}{failed} failed{RESET}" if failed else "")
          + (f"  {DIM}{skipped} skipped{RESET}" if skipped else ""))
    print()
    sys.exit(0 if failed == 0 else 1)


if __name__ == "__main__":
    main()
