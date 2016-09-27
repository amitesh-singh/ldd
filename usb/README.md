### NOTES - abt usb

1. how does usb matching happen?
 http://www.crashcourse.ca/wiki/index.php/USB_devices_and_matching

2. understanding lsusb output: http://diego.assencio.com/?index=1363692dafeabeff8e3f975077f92dfe
3. good explaination abt USB: http://www.beyondlogic.org/usbnutshell/usb5.shtml

### about urb

which is FAST? creating and reusing URBs or using helper functions like
usb_bulk/interrupt/control_msg()?  
reusing it is only safe when you can sure that it never queued up.
usb_bulk/interrupt/control_msg() is sync function.
async is useful in case of lots of data.
