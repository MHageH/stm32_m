#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/spi/spi.h>

#include <stdio.h>

typedef FAR struct file    file_t;

static int		dummy_open(file_t * filep);
static int		dummy_close(file_t * filep);
static ssize_t 	dummy_read(file_t *filep, FAR char *buffer, size_t buflen);
static ssize_t	dummy_write(file_t *filep, FAR const char *buf, size_t buflen);

static const struct file_operations	dummy_ops ={
	dummy_open, // Open
	dummy_close, // Close
	dummy_read,  // Read
	dummy_write, // Write
	0,			 // Seek
	0,			 // ioctl
};

static int dummy_open(file_t *filep) {
  printf("Open\n");
  return OK;
}

static int dummy_close(file_t *filep) {
  printf("Close\n");
  return OK;
}

static ssize_t dummy_read(file_t *filep, FAR char *buf, size_t buflen) {
	int k;
 	if (buflen>10) {
 		buflen=10;
 	}
 	for (k=0 ; k<buflen ; k++) {
 		buf[k]='0'+k;
 	}
 	printf("Read %d\n",buflen);
 
 return buflen;
}

static ssize_t dummy_write(file_t *filep, FAR const char *buf, size_t buflen) {
	int k;

 	printf("Write %d - ",buflen);

 	for (k=0; k<buflen ; k++) {
 		printf("%c",buf[k]);
 	}
 	printf("\n");

 	return buflen;
}

/****************************************************************************
 * Initialize device, add /dev/... nodes
 ****************************************************************************/
void stm32_dummy_setup(void) {
  (void)register_driver("/dev/dummy", &dummy_ops, 0444, NULL);
}
