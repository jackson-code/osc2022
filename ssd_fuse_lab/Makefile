all: clean make_new_folder ssd_fulse ssd_fuse_dut

ssd_fulse: ssd_fuse.c
	gcc -Wall ssd_fuse.c `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse

ssd_fuse_dut: ssd_fuse_dut.c
	gcc -Wall ssd_fuse_dut.c -o ssd_fuse_dut

clean:
	rm -f ssd_fuse_dut
	rm -f ssd_fulse
	fusermount -u /tmp/ssd
	rm -rf /tmp/ssd
	
cleanf:
	fusermount -u /tmp/ssd

make_new_folder:
	mkdir /tmp/ssd

ta:
	./ssd_fuse -d /tmp/ssd