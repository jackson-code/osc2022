.PHONY:all clean

all:
	make clean
	cd rootfs&&find . | cpio -o -H newc > ../ramfs.cp

clean:
	-rm *.cp
