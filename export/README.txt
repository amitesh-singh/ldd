Removing the module
-------------------

[ami@aarav export]$ lsmod  | grep dep
dep2                   16384  0
dep1                   16384  1 dep2

so you have to remove dep2 first, and then dep1
rmmod dep2
rmmod dep1

Ordering
--------
module.order shows the order in which the modules should be loaded.
Module.symvers shows which symbols are exported.

see the exported symbols
------------------------

dep1 exports the symbols

/proc/kallsyms file
The exported symbols and other functions in the kernel can be seen in the /proc/kallsyms file. Its is a huge file.


