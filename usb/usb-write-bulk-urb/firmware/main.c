#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

#define USB_LED_ON 1
#define USB_LED_OFF 0

PROGMEM const char configDescr[] = {    /* USB configuration descriptor */
    9,          /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    18 + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT3 +  7 + 
                (USB_CFG_DESCR_PROPS_HID & 0xff), 0,
                /* total length of data returned (including inlined descriptors) */
    1,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7) | USBATTR_REMOTEWAKE,      /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */

/* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT + USB_CFG_HAVE_INTRIN_ENDPOINT3 + 1, /* endpoints excl 0: number of endpoint descriptors to follow, 1 - is our OUT bulk */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */

// ********************************************************
// ENDPOINT -- out end point - interrupt out end point - ami
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    (char)0x01, /* out endpoint number 1 */
    0x02,       // attrib: for bulk - its 0x02, 0b00..10
    8, 0,       /* maximum packet size, for slow device, only 8 bytes */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
};

uchar usbFunctionDescriptor(usbRequest_t *rq)
{
   if (rq->wValue.bytes[1] == USBDESCR_DEVICE)
     {
        usbMsgPtr = (uchar *)usbDescriptorDevice;
        return usbDescriptorDevice[0];
     }
   else if (rq->wValue.bytes[1] == USBDESCR_CONFIG)
     {
        usbMsgPtr = (uchar *)configDescr;
        return sizeof(configDescr);
     }

   return 0;
}

void usbFunctionWriteOut(uchar *data, uchar len)
{
   //usbDisableAllRequests();
   if (data[0] == 1)
     {
         PORTB |= (1 << PB1); // turn LED on
     }
   else
     {
         PORTB &= ~(1 << PB1); // turn LED off
     }

   //usbEnableAllRequests();
}

//This is where custom message is handled from HOST
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
   usbRequest_t *rq = (void *)data; // cast data to correct type

   switch(rq->bRequest)
     { // custom command is in the bRequest field
      case USB_LED_ON:
         PORTB |= (1 << PB1); // turn LED on
         return 0;
      case USB_LED_OFF:
         PORTB &= ~(1 << PB1); // turn LED off
         return 0;
     }

   return 0; // should not get here
}

int __attribute__((noreturn))
main(void)
{
   uchar   i = 0;
   DDRB |= (1 << PB1); //define DDRB before doing usb stuffs

   wdt_enable(WDTO_1S);
   usbInit();
   usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
   while(--i)
     {             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
     }
   usbDeviceConnect();
   sei();
   while(1)
     {
        wdt_reset();
        usbPoll();
     }
}
