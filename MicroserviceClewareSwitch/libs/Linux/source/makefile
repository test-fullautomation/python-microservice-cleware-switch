# Build shared library for control Cleware device on Linux
all: ../USBAccessLinux.so

../USBAccessLinux.so: USBAccessLinux.o USBaccess.o USBaccessBasic.o
	g++ USBAccessLinux.o USBaccess.o USBaccessBasic.o -shared -o USBAccessLinux.so

USBaccess.o: USBaccess.cpp
	g++ -c -fPIC USBaccess.cpp -lstdc++ -o USBaccess.o

USBaccessBasic.o: USBaccessBasic.c
	gcc -c -fPIC USBaccessBasic.c -o USBaccessBasic.o

USBAccessLinux.o: USBAccessLinux.cpp
	g++ -c -fPIC USBAccessLinux.cpp -lstdc++ -o USBAccessLinux.o

clean:
	rm -f ../USBAccessLinux.so USBAccessLinux.o USBaccess.o USBaccessBasic.o

