#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/spi/spi.h>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/spi/spi.h>
#include <arch/board/board.h>

#include "stm32f334-disco.h"

#include "stm32.h"

#include "stm32_spi.h"

typedef FAR struct file		file_t;

void write_memory (unsigned short input);

/*
	The memory communication protocol is extracted from the 
	23LCV1024 Datasheet

	Mode of operation : 
	* Initialize the memory mode in open.
	  Modes are toggled by the bits 7 and 6 in the MODE register.
	  In our case, we need the Byte Operation to begin with. 
	  Check the Read and write modes register instruction for more 
	  information.

	* The Data is accessed on the SI pin, clocked on the
	  rising edge of the SCK. CS must be low on all operations
	  (read, write..) (means that the memory element of the SPI
	  is selected).

*/



static int     memory_open(file_t *);
static int     memory_close(file_t *);
static ssize_t memory_read(file_t *, FAR char *, size_t );
static ssize_t memory_write(file_t *, FAR const char *, size_t );
static int     memory_ioctl(FAR struct file *, int , unsigned long );

static const struct file_operations memory_ops = {
	memory_open,
	memory_close,
	memory_read,
	memory_write,
	0,
	memory_ioctl,
};

FAR struct spi_dev_s * spi; // Used to verify if SPI is initialized

static int memory_open(file_t * filep){
	// stm32_spi1status, used in stm32_spi.c return conditions,
	// which is located in spi_dev_s structure.

	if (!(spi->ops->status(spi, GETSTATUS))){ // If unlocked 
		printf("Memory : SPI uninitialized, initializing\n");
		
		// Initialise the SPI 
		stm32_spidev_initialize();

		// The STM32F334 has only 1 SPI bus.
		spi=stm32_spibus_initialize(1); 

		if (spi == NULL){
			printf("Unsupported SPI BUS \n");
		}

			// Lock SPI on open, unlock on close
		(void)SPI_LOCK(spi, true);
  		SPI_SETMODE(spi, SPIDEV_MODE1); // Mohamed :: Changed from SPIDEV_MODE2
  		SPI_SETBITS(spi, 8); // 8 bit word (used later in the strobo main prog)
  		(void)SPI_HWFEATURES(spi, 0);

  		printf("Memory : SPI Locked\n");

	} 

	// Grab the current memory mode :
	
	printf("Reading Memory condition :\n");
	while(1){
	SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
	// Send the Read mode register instruction and read the response
	SPI_SEND(spi, 0x05);
	//printf("Mode : %d\n", SPI_SEND(spi, 0x05));
	SPI_SELECT(spi, SPIDEV_USER(1) , false);	
	}


	printf("Opened Memory\n");
	return(0);
}

static int memory_close(file_t *filep){
  printf("Closing memory control\n");

  	if (spi->ops->status(spi, GETSTATUS)){ // If locked 
  		(void)SPI_LOCK(spi, false);
  		printf("Memory : SPI unlocked\n");
  	}
  return OK;
}

static ssize_t memory_read(file_t *filep, FAR char *buf, size_t buflen) {
	SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
 	SPI_RECVBLOCK(spi, buf, buflen);
 	printf("buf : %d\n", *buf);
 	SPI_SELECT(spi, SPIDEV_USER(1) , false); // De-select RAM

 	return buflen;
 }

 static ssize_t memory_write(file_t *filep, FAR const char *buf, size_t buflen){
 	printf("Memory : Sending a word : %c  \n", *buf);
 	write_memory(*buf);
 	return buflen;
 }

static int memory_ioctl(FAR struct file * filep, int cmd, unsigned long arg){
	return OK;
}

void stm32_memory_setup (void){
  (void)register_driver("/dev/memory", &memory_ops, 0444, NULL);
}

void write_memory (unsigned short input){
	SPI_SELECT(spi, SPIDEV_USER(1), true);
	SPI_SEND(spi, input);
  	SPI_SELECT(spi, SPIDEV_USER(1), false);
}