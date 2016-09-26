### Tips

1. it is actually not possible to have `bulk` endpoints on low speed usb device.  
  bulk endpoints are treated like interrupts.  
  warnings in kernel log  
  usb 2-1.8.1: config 1 interface 0 altsetting 0 endpoint 0x1 is Bulk; changing to Interrupt

