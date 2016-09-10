CONFIG_MODULE_SIG=n
obj-m += platform_driver.o
obj-m += platform_device.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
