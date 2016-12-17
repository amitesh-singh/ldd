obj-m += 4dtft.o st7735rRGB666.o st7735r.o tft.o 4dtftrgb666.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
