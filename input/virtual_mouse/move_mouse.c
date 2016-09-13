//This moves the mouse randomly after every 5 seconds

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
   int fd;
   int x, y;
   char buf[10];

   fd = open("/sys/devices/platform/vmd/coordinates",
             O_RDWR);
   if (fd < 0)
     {
        perror("could not able to open vmd file");
        return -1;
     }

   while (1)
     {
        x = random() % 20;
        y = random() % 20;
        if (x % 2) x = -x;
        if (y % 2) y = -y;

        sprintf(buf, "%d %d", x, y);
        write(fd, buf, strlen(buf));
        fsync(fd);
        sleep(5);
     }

   close(fd);

   return 0;
}
