# ldd

minimal setup on ubuntu
-----------------------
sudo apt-get build-dep linux-image-$(uname -r)


   insmod hello.ko --> insert the module
   lsmode -> list all the loaded modules
   rmmod hello -> remove the loaded module if present

   modinfo hello.ko -> gives info abt that module
