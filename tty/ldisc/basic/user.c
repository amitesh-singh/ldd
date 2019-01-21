#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define N_TTY 0 // don't know where is this defined in userspace header file.
/* on slave side, open a tty file descriptor.
 * open("serial device", O_RDONLY | O_NOCTTY);
 * ldisc = N_TTY;
 * 
 * ioctl(fd, TIOCSETD, &ldisc);
 */
int main()
{
   int fd = open("/dev/ttyACM0", O_RDONLY | O_NOCTTY);
   int ldisc = N_TTY;

   //this adds the line display N_TTY to serial device.
   ioctl(fd, TIOCSETD, &ldisc);

   char buf[64];
   strcpy(buf, "hello");
   write(fd, buf, 10);

   int bytesRead = read(fd, buf, 10);

   printf("fd = %d, bytesRead = %d", fd, bytesRead);

   close(fd);

   return 0;
}


