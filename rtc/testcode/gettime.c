#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>

int main(int argc, char *argv[])
{

   int fd, retval;
   struct rtc_time rtc_tm;

   if (argc < 2)
     {
        fprintf(stderr, "%s: /dev/rtc\n", argv[0]);
        exit(1);
     }
   /* Creating a file descriptor for RTC */
   fd = open(argv[1], O_RDONLY);
   if (fd == -1)
     {
        perror("Requested device cannot be opened!");
        _exit(errno);
     }

   /* Reading Current RTC Date/Time */
   retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
   if (retval == -1)
     {
        perror("ioctl");
        _exit(errno);
     }
   fprintf(stdout, "\nCurrent RTC Date/Time: %d-%d-%d %02d:%02d:%02d\n\n",
           rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
           rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
   fflush(stdout);
   fflush(stdout);
   fflush(stdout);
   close(fd);
   return 0;
}
