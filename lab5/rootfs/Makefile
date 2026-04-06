
CPIO_NAME = ../initramfs.cpio
USER_IMG = user.img

all: $(CPIO_NAME)

$(CPIO_NAME): $(USER_IMG)
	find . | cpio -o -H newc > $(CPIO_NAME)

# 複製 user.img 到 rootfs/
$(USER_IMG): ../user_prog/user.img
	cp $< $@

clean:
	rm -f $(CPIO_NAME) $(USER_IMG)