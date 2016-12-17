## Understanding basics of /dev/fb0

### Debug info

After loading the driver, you could use `fbset` command to get information
abt the fb device.
`fbset -i -fb /dev/fb1`

### How to setup ST7735R (black tab) with rpi2

Refer pinout diagram: http://www.jameco.com/Jameco/workshop/circuitnotes/raspberry_pi_circuit_note_fig2a.jpg

#### Connections
rpi2                st7735  

1  3.3v              Vcc  
39 GND               GND   
19 MOSI GPIO10       SDA  
CS                   0 no need  
18 GPIO24               RS/DC  
22 GPIO25               RST  

### running x 
#### Load driver
`sudo modprobe fbtft_device name=sainsmart18`  
or
`sudo modprobe fbtft_device name=flexfb gpios=dc:24, reset:25`  
` modprobe flexfb width=128 height=160 init=-1,0x01,-2,150,-1,0x11,-2,500,-1,0xB1,0x01,0x2C,0x2D,-1,0xB2,0x01,0x2C,0x2D,-1,0xB3,0x01,0x2C,0x2D,0x01,0x2C,0x2D,-1,0xB4,0x07,-1,0xC0,0xA2,0x02,0x84,-1,0xC1,0xC5,-1,0xC2,0x0A,0x00,-1,0xC3,0x8A,0x2A,-1,0xC4,0x8A,0xEE,-1,0xC5,0x0E,-1,0x20,-1,0x36,0xC0,-1,0x3A,0x05,-1,0xE0,0x0f,0x1a,0x0f,0x18,0x2f,0x28,0x20,0x22,0x1f,0x1b,0x23,0x37,0x00,0x07,0x02,0x10,-1,0xE1,0x0f,0x1b,0x0f,0x17,0x33,0x2c,0x29,0x2e,0x30,0x30,0x39,0x3f,0x00,0x07,0x03,0x10,-1,0x29,-2,100,-1,0x13,-2,10,-3
`  

Flexfb wiki: https://github.com/notro/fbtft/wiki/flexfb  

`sudo FRAMEBUFFER=/dev/fb1 startx`
Note: You might need to move an xorg config out of the way if X doesn't start:  
`sudo mv /usr/share/X11/xorg.conf.d/99-fbturbo.conf ~`  
To make permanent settings:

Add to file /etc/modules-load.d/fbtft.conf  

`spi-bcm2835`  
`fbtft_device`  

(the auto loading of spi-bcm2835 is too late for fbtft_device to find the spi bus, so it has to be manually loaded)  

   Add to file /etc/modprobe.d/fbtft.conf  

   `options fbtft_device name=sainsmart18`  

### issue when GL is enabled.

   I noticed X won't start if i enable experimental GL on raspberry pi. don't know why.

### Run EFL App on fb

`ELM_DISPLAY=fb`  
`EVAS_FB_DEV=/dev/fb0 or /dev/fb1`  

`./eflapp1`  
`./eflapp2`  
This would run efl without X11 which is pretty cool! ;)
if you want x11 or wayland to involved, set the DISPLAY env variable for x11
set the corresponding env variable for wayland.

Tips on make usb drive faster by RASTER  

~/.ccache to make i/o faster  

### show photos

`sudo apt-get install -y fbi`  
`sudo fbi -d /dev/fb1 -T 1 -noverbose -a ami.jpg`

### run videos

`sudo SDL_VIDEODRIVER=fbcon SDL_FBDEV=/dev/fb1 mplayer -vo sdl -framedrop test.mpg`

#### Run videos FASTER
sudo modprobe fbtft dma
sudo modprobe fbtft_device name=sainsmart18 rotate=90 speed=62000000 fps=50

#### Start fb copying process in the background
`fbcp &`  

Play video on /dev/fb0, which will also show up on /dev/fb1  

`omxplayer test_480_320.mpg`  

Stop framebuffer copy

killall fbcp

### connecting with 3.5" tft display

`sudo modprobe fbtft_device name=flexpfb rotate=180 fps=30 gpios=dc:22,reset:10,wr:24,cs:23,db00:2,db01:3,db02:4,db03:14,db04:15,db05:17,db06:18,db07:27`

`sudo modprobe flexfb width=480 height=320 buswidth=8 init=-1,0xb0,0x0,-1,0x11,-2,120,-1,0x3A,0x55,-1,0xC2,0x33,-1,0xC5,0x00,0x1E,0x80,-1,0x36,0x28,-1,0xB1,0xB0,-1,0xE0,0x00,0x04,0x0E,0x08,0x17,0x0A,0x40,0x79,0x4D,0x07,0x0E,0x0A,0x1A,0x1D,0x0F,-1,0xE1,0x00,0x1B,0x1F,0x02,0x10,0x05,0x32,0x34,0x43,0x02,0x0A,0x09,0x33,0x37,0x0F,-1,0x11,-1,0x29,-3`

### connection for 3.5" tft display with my driver.

D0 - 2  
D1 - 3  
D2 - 4  
D3 - 14  
D4 - 15  
D5 - 17  
D6 - 18  
D7 - 27  

RS - 22  
CS - 23  
WR  - 24  
RST - 10  


### Speed

A monitor draws an image on the screen by using an electron beam (3 electron
                                                                  beams for color models, 1 electron beam for monochrome monitors). The front of
the screen is covered by a pattern of colored phosphors (pixels). If a phosphor
is hit by an electron, it emits a photon and thus becomes visible.

The electron beam draws horizontal lines (scanlines) from left to right, and
from the top to the bottom of the screen. By modifying the intensity of the
electron beam, pixels with various colors and intensities can be shown.

After each scanline the electron beam has to move back to the left side of the
screen and to the next line: this is called the horizontal retrace. After the
whole screen (frame) was painted, the beam moves back to the upper left corner:
this is called the vertical retrace. During both the horizontal and vertical
retrace, the electron beam is turned off (blanked).

The speed at which the electron beam paints the pixels is determined by the
dotclock in the graphics board. For a dotclock of e.g. 28.37516 MHz (millions
                                                                     of cycles per second), each pixel is 35242 ps (picoseconds) long:
Say the driver board speed is 28.37516 Mhz, each pixel is 35242 picoseconds.  
1/(28.37516 E6) =  3.524209202697007e-08

If the screen resolution is 640 x 480, it will take
640 * 3.524209202697007e-08 = 22.555E-6 s  
to paint 640 xres pixels on one scanline. but the horizontal retrace
also takes time (e.g. 272 `pixels`) so a full scanline takes  
(640 + 272) * 35.242E-9 s = 32.141E-6 s  
We will say the horizontal scanrate is abt 31 Khz.
1/32.141E-6 s) = 31.113E3 Hz  


A full screen counts 480 (yres) lines, but we have to consider the vertical
retrace too (e.g. 49 `lines'). So a full screen will take  

(480+49)*32.141E-6 s = 17.002E-3 s  

The vertical scanrate is about 59 Hz:  

1/(17.002E-3 s) = 58.815 Hz  



This means the screen data is refreshed about 59 times per second. To have a
stable picture without visible flicker, VESA recommends a vertical scanrate of
at least 72 Hz. But the perceived flicker is very human dependent: some people
can use 50 Hz without any trouble, while I'll notice if it's less than 80 Hz.



