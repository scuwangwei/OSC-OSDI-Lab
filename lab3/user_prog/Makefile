AS = aarch64-linux-gnu-as
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

USER_S = main.s
USER_LD = linker.ld

all: user.img

user.img: user.elf
	$(OBJCOPY) -O binary $< $@

user.elf: user.o
	$(LD) -T $(USER_LD) -o $@ $^

user.o: $(USER_S)
	$(AS) -o $@ $<

clean:
	rm -f user.o user.elf user.img