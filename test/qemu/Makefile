AASM  := ../../bin/AASM
SRC   := test_qemu.aasm
OUT   := out.bin
QEMU  := qemu-system-riscv32

# -device loader con force-raw carica il bin grezzo e setta il PC
QEMU_FLAGS := \
	-machine virt,aclint=off     \
	-bios none                   \
	-nographic                   \
	-no-reboot                   \
	-serial mon:stdio            \
	-device loader,file=$(OUT),addr=0x80000000,force-raw=on,cpu-num=0

.PHONY: all asm run clean

all: asm run

asm: $(OUT)

$(OUT): $(SRC)
	$(AASM) $(SRC) $(OUT)
	@echo "  assembled  $(SRC) -> $(OUT)  ($$(wc -c < $(OUT)) bytes)"

run: $(OUT)
	@echo "  starting QEMU...  (Ctrl-A X to quit)"
	$(QEMU) $(QEMU_FLAGS)

clean:
	rm -f $(OUT)
