#dmesg -wH on another terminal
sudo insmode mynull.ko

sudo chmod 777 /dev/mynull

echo "hello" > /dev/mynull -> observe output at dmesg -wH terminal
