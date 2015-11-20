# ldd

minimal setup on ubuntu
-----------------------
sudo apt-get build-dep linux-image-$(uname -r)


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


Links
------

1. http://lwn.net/Kernel/LDD3/
2. https://github.com/martinezjavier/ldd3/
3. http://lwn.net/Articles/2.6-kernel-api/
4. http://opensourceforu.efytimes.com/2015/05/writing-a-basic-framebuffer-driver/
5. https://sysplay.in/ -> interesting way
6. http://www.tutorialsdaddy.com/2015/05/writing-character-device-driver/
7. https://appusajeev.wordpress.com/2011/06/18/writing-a-linux-character-device-driver/
8. http://derekmolloy.ie/writing-a-linux-kernel-module-part-1-introduction/
