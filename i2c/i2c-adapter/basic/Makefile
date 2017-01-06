obj-m += i2c-device.o i2c-adapter-driver.o i2c-client-driver.o i2c-client.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
