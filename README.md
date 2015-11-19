# ldd

minimal setup on ubuntu
-----------------------
sudo apt-get build-dep linux-image-$(uname -r)


   insmod hello.ko --> insert the module
   lsmode -> list all the loaded modules
   rmmod hello -> remove the loaded module if present

   modinfo hello.ko -> gives info abt that module

   dmesg | tail -> to check the printk() output

Links
------

1. http://lwn.net/Kernel/LDD3/
2. https://github.com/martinezjavier/ldd3/
3. http://lwn.net/Articles/2.6-kernel-api/
4. http://opensourceforu.efytimes.com/2015/05/writing-a-basic-framebuffer-driver/
5. https://sysplay.in/ -> interesting way
6. http://www.tutorialsdaddy.com/2015/05/writing-character-device-driver/
