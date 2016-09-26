#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

#define USB_LED_ON 1
#define USB_LED_OFF 0

static uint8_t ack_remaining = 0;

//This is where custom message is handled from HOST
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
   usbRequest_t *rq = (void *)data; // cast data to correct type

   switch(rq->bRequest)
     { // custom command is in the bRequest field
      case USB_LED_ON:
         PORTB |= (1 << PB1); // turn LED on
         ack_remaining = 1;
         return 0;
      case USB_LED_OFF:
         PORTB &= ~(1 << PB1); // turn LED off
         ack_remaining = 1;
         return 0;
     }

   return 0; // should not get here
}

static uchar buf[20];

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
        if (usbInterruptIsReady() && ack_remaining == 1)
          {
             //urb complete function callback will 
             // get the data urb->context or sd->int_in_buf
             buf[0] = 0x41;
             // 0x45 is our ACK
             usbSetInterrupt(buf, 1);
             ack_remaining = 0;
          }
     }
}
