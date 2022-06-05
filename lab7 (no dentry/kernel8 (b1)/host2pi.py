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
		step = 16										# transmit speed (linker script align ?, file size % 16 = 0)
		for i in range(0, length, step):
			serial.write(bytes[i:i+step])
			serial.flush()
			#if (i%100) == 0:
			print("{:>6}/{:>6} bytes".format(i, length))
		
		
		left_bytes = length % step		
		if left_bytes != 0:						# some bytes didn't send
			print("Warning: {} bytes have not send yet!".format(left_bytes))		
			for i in range(length - left_bytes, length, 1):
				serial.write(bytes[i:i+1])
				serial.flush()
		
		
		
		'''
		left_bytes = length % step		
		if left_bytes != 0:						# some bytes didn't send
			print("Warning: {} bytes have not send yet!".format(length-1 - (i+step)))		
			for i in range(left_bytes, length, 1):
				serial.write(bytes[i:i+1])
				serial.flush()'''
		
		print("{:>6}/{:>6} bytes".format(length, length))
		print("Send finished!")

