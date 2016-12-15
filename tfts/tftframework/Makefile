obj-m += st7735rRGB666_serial.o st7735rRGB666_experimental.o st7735rRGB666.o st7735r.o tft.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
