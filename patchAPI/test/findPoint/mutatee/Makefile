all: c liblib.so

c: main.c
	gcc -o c main.c
	objdump -S c > bin

liblib.so: lib.c
	gcc -o liblib.so lib.c -shared -fPIC

clean:
	rm -rf c bin *.so
