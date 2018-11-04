all: p8conv
	./p8conv focal.bin rom.bin
	xxd -i <rom.bin >../src/rom.h

clean:
	rm -f p8conv rom.bin
