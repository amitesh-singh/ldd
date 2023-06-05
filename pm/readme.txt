power management in linux kernel drivers

sudo apt-get install pm-utils

sudo pm-suspend can be used to check dev_pm_ops operations

sudo insmod platform-driver.ko
sudo insmod platform-device.ko

sudo pm-suspend


following commands would work as well.

sudo systemctl suspend: This command is widely supported on modern Linux distributions that use systemd as the init system. It initiates a system suspend operation.

systemctl hibernate: This command triggers system hibernation, which saves the system state to disk and powers off the computer. It allows you to resume to the exact state when powered back on.

systemctl hybrid-sleep: This command performs hybrid sleep, a combination of suspend and hibernate. It saves the system state to both memory and disk, providing a fast resume from suspend while also offering the ability to recover from power loss during suspend.
