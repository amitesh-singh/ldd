#### This driver is for avr(atmega16a) based led module

`./firmware - firmware code`  
`#cd firmware`  
`# make hex`  
upload main.hex file into your vusb based usb device.  
`#avrdude -c usbasp -p atmega16a -U flash:w:main.hex`  

`./driver/ - linux kernel driver which blinks the led on the usb board`  

`#make`  
`#sudo insmod blink-led.ko`  

To unload the module, sudo rmmod blink_led
