# ldd

minimal setup on ubuntu
-----------------------
apt-get source linux-image-$(uname -r) in /usr/src in case you are going to
run your kernel

sudo apt-get build-dep linux-image-$(uname -r)

sudo apt-get install kernel-package

how to build kernel on ubuntu
-----------------------------

http://www.howtogeek.com/howto/ubuntu/how-to-customize-your-ubuntu-kernel/

install libqt4-dev

#cd linux-xxx/
#cp /boot/config-`uname -r` .config

#make xconfig 

#    make-kpkg clean

$ fakeroot make-kpkg --initrd --append-to-version=-custom kernel_image kernel_headers

two .deb files are generated. one is linux image and other is linux headers.
install both .deb files. those are located in parent dir.
dpkg -i linux**.deb

and then reboot.

check uname -r then. 


check Aug 27 01:19:03 ami-desktop kernel: [    0.000000] Hello to ami's kernel

log file to be checked for boot logs: /var/log/kern.log - 

how to remove the custom kernel 
-------------------------------
// this the case when you install kernel via sudo make install 
locate 2.6.28.9-custom

delete the files manually and then sudo update-grub2

To uninstall custom kernel, make sure you are not using that kernel currently.

since i installed kernel in debian way,
      dpkg --list | grep kernel-image
      sudo apt-get remove kernel-image-your-kernel-image

To see all the kernels list in grub bootloader screen, press SHIFT key.


how to load/unload ldds
-----------------------

   insmod hello.ko --> insert the module
   lsmode -> list all the loaded modules
   rmmod hello -> remove the loaded module if present

   modinfo hello.ko -> gives info abt that module

   dmesg | tail -f -> to check the printk() output

Tips
-----
1. how to login as root in ubuntu
first change the password of root (if u r doing it for first time)
sudo passwd root
//change the password
//to login into root, do
su root

All compiled modules are in /lib/modules/`uname -r`/kernel/drivers/

Links
------
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

how to implement icotls on a char driver
----------------------------------------
http://linux.die.net/lkmpg/x892.html
https://fabiobaltieri.com/2012/05/20/linux-kernel-device-drivers-for-avr-v-usb-devices/

how to check module info using objdump
-------------------------

modinfo module.ko
or
objdump hello-1.ko  --full-contents --section=.modinfo

how to send kernel patches
--------------------------


sudo apt-get install mutt
mutt -H 001....path

OR

sudo apt-get install git-email

git send-email --to <email address>  --cc gregkh@linuxfoundation.org --cc linux-gpio@vger.kernel.org --cc linux-kernel@vger.kernel.org 001....patch

nice ascii flow diagram: http://asciiflow.com/
Reference: https://burzalodowa.wordpress.com/2013/10/05/how-to-send-patches-with-git-send-email/
