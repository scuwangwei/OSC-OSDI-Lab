GCC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

USER_C = main.c
USER_LD = linker.ld

all: user.img

user.img: user.elf
	$(OBJCOPY) -O binary $< $@

user.elf: user.o
	$(LD) -T $(USER_LD) -o $@ $^

user.o: $(USER_C)
	$(GCC) -c -ffreestanding -nostdlib -nostartfiles -o $@ $<

clean:
	rm -f user.o user.elf user.img