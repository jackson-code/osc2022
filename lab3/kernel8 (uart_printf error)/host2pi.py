#!usr/bin/python3
# defines where the interpreter is located

from serial import Serial

with open("kernel8.img", "rb", buffering=0) as raw_io:		# read only as binary, "with as" make sure file will be close,
															# buffering=0 return rawIO
	with Serial(port = '/dev/ttyUSB0', baudrate = 115200) as serial:
		
		# size of the img
		raw_io.seek(0, 2)		# 2 means move read/write pointer to the end of the file
		length = raw_io.tell()	# position of the pointer
		print("size of kernel8.img :", length)
		raw_io.seek(0, 0)		# 0 means move read/write pointer to the start of the file
		
		# send size to pi(uart)
		serial.write(str(length).encode() + b'\n')		# b convert str to the type: bytes, ref: pySerial doc website
		serial.flush()									# wait until all data is written
	
		bytes = raw_io.read()
		print("Send img to pi...")
		for i in range(length):
			serial.write(bytes[i:i+1])
			serial.flush()
			if (i%100) == 0:
				print("{:>6}/{:>6} bytes".format(i, length))
		print("{:>6}/{:>6} bytes".format(length, length))
		print("Send finished!")

