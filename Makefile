CFLAGS:=-Os

ip-lpm: lpm.o ip-lpm.o

all: ip-lpm

clean:
	rm -rf ip-lpm *.o