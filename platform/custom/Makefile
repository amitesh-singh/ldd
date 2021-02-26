obj-m += custom_driver.o custom_device.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert:
	sudo insmod custom_driver.ko
	sudo insmod custom_device.ko

remove:
	sudo rmmod custom_device
	sudo rmmod custom_driver
