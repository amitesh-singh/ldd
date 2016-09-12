obj-m += test.o
test-objs := module.o main.o
main.o: main.asm
	nasm -f elf64 -o $@ $^
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

