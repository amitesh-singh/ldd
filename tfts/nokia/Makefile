#obj-m += fbtft.o
#obj-m += fbtft-sysfs.o fbtft-core.o fbtft_device.o fbtft-bus.o fbtft-io.o

#obj-m+= fb_st7735r.o
obj-m+= nokia.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
