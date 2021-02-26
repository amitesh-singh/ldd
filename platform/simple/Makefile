CONFIG_MODULE_SIG=n
obj-m += platform_driver.o
obj-m += platform_device.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert:
	sudo insmod platform_driver.ko
	sudo insmod platform_device.ko

remove:
	sudo rmmod platform_driver.ko
	sudo rmmod platform_device.ko

