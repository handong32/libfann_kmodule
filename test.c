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
#include <math.h>
 
#define handle_error(msg)                                               \
    do {								\
	perror(msg);							\
	exit(EXIT_FAILURE);						\
    } while (0)

#define MAJOR_NUM 101
#define IOCTL_TEST _IO(MAJOR_NUM, 0)

#define MAX_PRECISION	(10)
static const double rounders[MAX_PRECISION + 1] =
{
	0.5,				// 0
	0.05,				// 1
	0.005,				// 2
	0.0005,				// 3
	0.00005,			// 4
	0.000005,			// 5
	0.0000005,			// 6
	0.00000005,			// 7
	0.000000005,		// 8
	0.0000000005,		// 9
	0.00000000005		// 10
};

char* mftoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;

	// check precision bounds
	if (precision > MAX_PRECISION)
		precision = MAX_PRECISION;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		*ptr++ = '-';
	}

	if (precision < 0)  // negative precision == automatic precision guess
	{
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}

	// round value according the precision
	if (precision)
		f += rounders[precision];

	// integer part...
	intPart = f;
	f -= intPart;

	if (!intPart)
		*ptr++ = '0';
	else
	{
		// save start pointer
		p = ptr;

		// convert (reverse order)
		while (intPart)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}

		// save end pos
		p1 = p;

		// reverse result
		while (p > ptr)
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}

		// restore end pos
		ptr = p1;
	}

	// decimal part
	if (precision)
	{
		// place decimal point
		*ptr++ = '.';

		// convert
		while (precision--)
		{
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}

	// terminating zero
	*ptr = 0;

	//return buf;
	return;
}

 
int main(){
   int ret, fd;
   double test, test2;
   char tbuf[100];

   printf("Starting device test code example...\n");
   fd = open("/dev/NN", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   ret = ioctl(fd, IOCTL_TEST);
   if (ret < 0) handle_error("IOCTL_TEST");
   
   close(fd);
   
   /*   test = exp(5.0);
   mftoa(test, &(tbuf[0]), 5);
   printf("exp = %s\n", tbuf);
   
   test2 = log(5.5);
   mftoa(test2, &(tbuf[0]), 5);
   printf("mlog = %s\n", tbuf);


   test2 = pow(7.0, 3.0);
   mftoa(test2, &(tbuf[0]), 5);
   printf("7 ^ 3 = %s\n", tbuf);

   test2 = pow(4.73, 12);
   mftoa(test2, &(tbuf[0]), 5);
   printf("4.73 ^ 12 = %s\n", tbuf);

   test2 = pow(32.01, 1.54);
   mftoa(test2, &(tbuf[0]), 5);
   printf("32.91 ^ 1.54 = %s\n", tbuf);

   test = sin(45.0 * (3.14159265/180.0));
   mftoa(test, &(tbuf[0]), 5);
   printf("sine = %s\n", tbuf);
    
   test = cos(60.0 * (3.14159265/180.0));
   mftoa(test, &(tbuf[0]), 5);
   printf("cosine = %s\n", tbuf);*/

   return 0;
}
