#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
 
#define handle_error(msg)                                               \
    do {								\
	perror(msg);							\
	exit(EXIT_FAILURE);						\
    } while (0)

#define MAJOR_NUM 101
#define IOCTL_TEST _IO(MAJOR_NUM, 0)
 
int main(){
   int ret, fd;
   
   printf("Starting device test code example...\n");
   fd = open("/dev/NN", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   ret = ioctl(fd, IOCTL_TEST);
   if (ret < 0) handle_error("IOCTL_TEST");
   
   close(fd);
   
   return 0;
}
