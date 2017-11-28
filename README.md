## ldd tips/setups

### minimal setup on ubuntu 16.04

`apt-get source linux-image-$(uname -r)` in /usr/src in case you are going to
run your kernel  
`sudo apt-get build-dep linux-image-$(uname -r)`  
`sudo apt-get install kernel-package`

#### rpi2

The below stuff did not work, so don't try it.
~~`sudo apt-get install linux-image-rpi-rpfv linux-headers-rpi-rpfv`~~

use `rpi-update` (do update it)  
   `$ sudo rpi-update`  
   It should give you the latest kernel and then reboot.  
   
   After that, Get `rpi-source`  
   `$ sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source`  
   `$ sudo chmod +x /usr/bin/rpi-source`  
   `$ /usr/bin/rpi-source -q --tag-update`  
   `$ rpi-source`

### how to build kernel on ubuntu

http://www.howtogeek.com/howto/ubuntu/how-to-customize-your-ubuntu-kernel/  

`make menuconfig` - arrow keys were not working. the issue was with bash.
to make it work, issue `sh` command and then `make menuconfig` should work.  

`sudo apt-get install libqt4-dev`  
`#cd linux-xxx/`  
`#cp /boot/config-`uname -r` .config`  
`#make xconfig `  
`# make-kpkg clean`  
`$ fakeroot make-kpkg --initrd --append-to-version=-custom kernel_image kernel_headers`

two .deb files are generated. one is linux image and other is linux headers.  
install both .deb files. those are located in parent dir.  
dpkg -i **linux**.deb  and then reboot.  
check uname -r after reboot.

`Aug 27 01:19:03 ami-desktop kernel: [    0.000000] Hello to ami's kernel
log file to be checked for boot logs: /var/log/kern.log `  

### how to remove the custom kernel 

#### installed kernel via sudo make install 

1. locate 2.6.28.9-custom
2. delete the files manually and then sudo update-grub2

To uninstall custom kernel, make sure you are not using that kernel currently.

#### installed kernel in debian way,  

 `dpkg --list | grep kernel-image`  
 `sudo apt-get remove kernel-image-your-kernel-image`  
To see all the kernels list in grub bootloader screen, press SHIFT key.

### how to load/unload ldds

insmod hello.ko --> insert the module  
lsmode -> list all the loaded modules  
rmmod hello -> remove the loaded module if present  

modinfo hello.ko -> gives info abt that module  

   dmesg | tail -f -> to check the printk() output  

### Tips

1. how to login as root in ubuntu
first change the password of root (if u r doing it for first time)
sudo passwd root  
//change the password  
//to login into root, do  
su root  

All compiled modules are in /lib/modules/`uname -r`/kernel/drivers/

### Links

best one: http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html#AEN687

1. http://lwn.net/Kernel/LDD3/
2. https://github.com/martinezjavier/ldd3/
3. http://lwn.net/Articles/2.6-kernel-api/
4. http://opensourceforu.efytimes.com/2015/05/writing-a-basic-framebuffer-driver/
5. https://sysplay.in/ -> interesting way
6. http://www.tutorialsdaddy.com/2015/05/writing-character-device-driver/
7. https://appusajeev.wordpress.com/2011/06/18/writing-a-linux-character-device-driver/
8. http://derekmolloy.ie/writing-a-linux-kernel-module-part-1-introduction/
9. https://fabiobaltieri.com/2011/09/21/linux-led-subsystem/
10. https://0xax.gitbooks.io/linux-insides/content/  --> best one
11. http://linux-sunxi.org/SPIdev
12. nice doc on writing spi driver - http://invo-tronics.com/spi-driver-for-linux-based-embedded-system/

#### how to implement icotls on a char driver

http://linux.die.net/lkmpg/x892.html
https://fabiobaltieri.com/2012/05/20/linux-kernel-device-drivers-for-avr-v-usb-devices/

### how to check module info using objdump

`modinfo module.ko`
`or`
`objdump hello-1.ko  --full-contents --section=.modinfo`

### how to send kernel patches

#### install mutt

`sudo apt-get install mutt`

#### Raise the patch
`mutt -H 001....path`  
OR  
#### install git-send-email

`sudo apt-get install git-email`

#### Raise the patch
`git format-patch HEAD^`  
`git send-email --to <email address>  --cc gregkh@linuxfoundation.org --cc linux-gpio@vger.kernel.org --cc linux-kernel@vger.kernel.org 001....patch`

`nice ascii flow diagram:` http://asciiflow.com/   useful while sending RFCs  
`Reference:` https://burzalodowa.wordpress.com/2013/10/05/how-to-send-patches-with-git-send-email/

### how to make doc

`make htmldocs`

### How to fix kernel module warnings

1. *kernel module signature loading failed:* Add  
`CONFIG_MODULE_SIG=n`  
in your module Makefile

### Linux interrupts

1. http://www.slideshare.net/kerneltlv/linux-interrupts-63978201?qid=aba0244c-f180-42f6-a9ad-e5791dda14db&v=&b=&from_search=5

### Device Tree
- https://events.linuxfoundation.org/sites/events/files/slides/petazzoni-device-tree-dummies.pdf
- install sudo apt install device-tree-compiler @ dtc  

### how to compile latest kernel

#### Ubuntu
cd linux-stable  

cp /boot/config-`uname -r` .config  

yes '' | make oldconfig  

make menuconfig   
make clean  

make -j `getconf _NPROCESSORS_ONLN` deb-pkg LOCALVERSION=-custom  

You’ll find your new kernel packages one directory up. They’ll be easily identifiable by their version number  

cd ..  
sudo dpkg -i linux-firmware-image-4.11.1-custom_4.11.1-custom-1_amd64.deb  
sudo dpkg -i linux-libc-dev_4.11.1-custom-1_amd64.deb  
sudo dpkg -i linux-headers-4.11.1-custom_4.11.1-custom-1_amd64.deb  
sudo dpkg -i linux-image-4.11.1-custom-dbg_4.11.1-custom-1_amd64.deb  
sudo dpkg -i linux-image-4.11.1-custom_4.11.1-custom-1_amd64.deb  

then reboot

#### Arch linux

cd linux-src  
zcat /proc/config.gz > .config  
yes '' | make oldconfig  
make menuconfig  
make clean  
make -j `getconf _NPROCESSORS_ONLN` LOCALVERSION=-custom  
make modules_install  

make modules_install  

Copy the kernel to /boot directory  
cp -v arch/x86_64/boot/bzImage /boot/vmlinuz-linux414

Make initial RAM disk  
cp /etc/mkinitcpio.d/linux.preset /etc/mkinitcpio.d/linux414.preset
sudo vim /etc/mkinitcpio.d/linux414.preset

...
ALL_kver="/boot/vmlinuz-linux48"  
...  
default_image="/boot/initramfs-linux48.img"  
...  
fallback_image="/boot/initramfs-linux48-fallback.img"  

Finally, generate the initramfs images for the custom kernel in the same way as for an official kernel:   
 mkinitcpio -p linux414  

 Copy System.map

The System.map file is not required for booting Linux. It is a type of "phone directory" list of functions in a particular build of a kernel. The System.map contains a list of kernel symbols (i.e function names, variable names etc) and their corresponding addresses. This "symbol-name to address mapping" is used by:  

cp System.map /boot/System.map-YourKernelName
ln -sf /boot/System.map-YourKernelName /boot/System.map

After completing all steps above, you should have the following 3 files and 1 soft symlink in your /boot directory along with any other previously existing files:

    Kernel: vmlinuz-YourKernelName
    Initramfs: Initramfs-YourKernelName.img
    System Map: System.map-YourKernelName
    System Map kernel symlink


grub-mkconfig -o /boot/grub/grub.cfg  

and then reboot

#### Rpi3 - arch linux
- pacman -S linux-headers  
  choose 14) linux-raspberry-headers

