CFLAGS = -Iinclude -fno-stack-protector
K8=kernel8
BL=bootloader
LINKER=linker.ld
K=kernel
START=start
CPIO:=initramfs.cpio
DTB:=bcm2710-rpi-3-b-plus.dtb
USER=user_prog/user


SRC_C:=$(shell find src/ -name '*.c') kernel_main.c
SRC_S:=$(shell find src/ -name '*.s')

BL_C:=$(shell find bootloader/ -name '*.c')
BL_S:=$(shell find bootloader/ -name '*.s')

BL_OBJS:=$(BL_C:%.c=%.o) \
		$(BL_S:%.s=%.o) \
		src/mini_uart.o

OBJS:= $(SRC_C:%.c=%.o) \
		$(SRC_S:%.s=%.o) \
		start.o

all:$(K8).img $(BL).img

run_qe_bl:$(BL).img $(K8).img $(CPIO)
	qemu-system-aarch64 -M raspi3 -kernel $(BL).img -serial null -serial pty -initrd $(CPIO) -dtb $(DTB)

run_qe_k8:$(K8).img $(CPIO)
	qemu-system-aarch64 -M raspi3 -kernel $(K8).img -serial null -serial stdio -initrd $(CPIO) -dtb $(DTB)

start.o: start.s
	aarch64-linux-gnu-gcc -o $@ -c -O0 $< 

%.o: %.s
	aarch64-linux-gnu-gcc -o $@ -c $<

%.o: %.c
	aarch64-linux-gnu-gcc -o $@ $(CFLAGS) -c $<


$(K8).img: $(OBJS)
	aarch64-linux-gnu-ld -T $(LINKER) -o $(K8).elf $^
	aarch64-linux-gnu-objcopy -O binary $(K8).elf $@

$(BL).img: $(BL_OBJS) 
	aarch64-linux-gnu-ld -T bootloader/linker.ld -o $(BL).elf $^
	aarch64-linux-gnu-objcopy -O binary $(BL).elf $@

$(CPIO): $(USER)
	$(MAKE) -C rootfs

$(USER):
	$(MAKE) -C user_prog

clean:
	rm -f $(K8).elf $(K8).img $(OBJS) $(BL).img $(BL).elf $(BL_OBJS)