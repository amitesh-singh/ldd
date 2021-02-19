obj-m += basic.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert:
	sudo insmod basic.ko

remove:
	sudo rmmod basic

read:
	cat /proc/my_proc

write:
	sudo echo "Hello baby!" > /proc/my_proc
