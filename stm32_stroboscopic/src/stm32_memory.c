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
#include "stm32_memory.h"

typedef FAR struct file		file_t;

// Memory mode structure used to define the current mode, in IOCTL
struct MEMORY SRAM;


uint8_t condition = 0;

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

	// Check memory mode first
	//check_memory_mode();

	/*
	printf("Focing mode Byte : \n");
	write_memory_mode(MEMORY_BYTE_MODE);
	printf("Done\n");
	*/

	printf("Focing mode Sequential : \n");
	write_memory_mode(MEMORY_SEQUENTIAL_MODE);


	// Check memory mode after the change
	// check_memory_mode();

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
 	int i;
 	struct MEMORY * PSRAM = &SRAM;

 	switch(PSRAM->mode){
 		case MEMORY_SEQUENTIAL_MODE :
 			{
 				 // Read procedure 
				// SPI Select is controlled from IOCTL

 				if(!PSRAM->read_activated){

 				//printf("\n \n Mohamed : Debug \n\n");	

				// Begin by sending 0x03, signaling read 
				SPI_SEND(spi, 0x03); 

				// Send the first 24-bit address, in our case start at 0  
				SPI_SEND(spi, 0x00);
				SPI_SEND(spi, 0x00);
				SPI_SEND(spi, 0x00);

				PSRAM->read_activated = 1;
				}

				// Receive a continuous stream of bytes until the buffer ends
				for (i=0; i < buflen ; i++){
					// 1 means receive one word in the buffer
					SPI_RECVBLOCK(spi, &buf[i], 1); 
				}

				// SPI Select is controlled from IOCTL

 				break;
 			}
 		default :
 			break;
 	}

 	return buflen;
 }

 static ssize_t memory_write(file_t *filep, FAR const char *buf, size_t buflen){
	int i;
	struct MEMORY * PSRAM = &SRAM;


	switch(PSRAM->mode){
		case MEMORY_SEQUENTIAL_MODE :
			{
				// Write procedure 
				// SPI Select is controlled from IOCTL

				// Check the activation flag : if 0, send the first address
				// and the write command 
				if(!PSRAM->write_activated){
					// Begin by sending 0x02, signaling write 
					SPI_SEND(spi, 0x02); 

					// Send the first 24-bit address, in our case start at 0  
					SPI_SEND(spi, 0x00);
					SPI_SEND(spi, 0x00);
					SPI_SEND(spi, 0x00);

					// Activate the flag
					PSRAM->write_activated = 1;
				}
					// Send continuous stream of bytes until the buffer ends
				for (i=0; i < buflen ; i++){
					SPI_SEND(spi, buf[i]); 
				}

				// SPI Select is controlled from IOCTL
				break;
			}
		default:
			break;
	}

 	return buflen;
 }

static int memory_ioctl(FAR struct file * filep, int cmd, unsigned long arg){
	struct MEMORY * PSRAM = &SRAM;

	switch(cmd){
		// Set the memory mode 
		case SET_MEMORY_MODE :
			{
				PSRAM->mode = arg;

				// If we are in sequential mode, set the activation flag to 0,
				// we'll set it to 1 once

				if(arg == MEMORY_SEQUENTIAL_MODE){
					PSRAM->write_activated = 0;
					PSRAM->read_activated = 0;
				} 

				break;
			}
		// Sets the CS toggle if needed
		case CS_CONDITION:
			{	
				PSRAM->CS = arg;
			}
		// Used in conjuction with the Sequential mode
		case SET_CS_MEMORY:
			{
				SPI_SELECT(spi, SPIDEV_USER(1), (bool)arg);
			}
	
		case SET_READ_CONDITION :
			{
				// Don't assign read_activated to PSRAM directly, or it will be changed
				// to 1 on the way.
				if (arg == 0){
					PSRAM->read_activated = 0;					
				}
			}

		default:
			break;
	}

	return OK;
}

void stm32_memory_setup (void){
  (void)register_driver("/dev/memory", &memory_ops, 0444, NULL);
}

// Memory Specific Functions

void check_memory_mode (void) {
	// Grab the current memory mode :
	
	#ifdef PRINTF_DEFINE
	printf("Reading Memory condition :\n");
	#endif

	// uint8_t condition;
	SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
	SPI_SEND(spi, 0x05); // Send memory RDMR
	SPI_RECVBLOCK(spi, &condition, 1); // Send one byte
	SPI_SELECT(spi, SPIDEV_USER(1) , false);

	#ifdef PRINTF_DEFINE

	switch (condition){
		case MEMORY_BYTE_MODE :
			{
				printf("Memory in Byte Mode : %u\n", condition);
				break;
			}
		case MEMORY_PAGE_MODE :
			{
				printf("Memory in Page Mode : %u\n", condition);
				break;
			}
		case MEMORY_SEQUENTIAL_MODE :
			{
				printf("Memory in Sequential Mode : %u\n", condition);
				break;
			}		
		case MEMORY_RESERVED :
			{
				printf("Memory in Reserved Mode : %u\n", condition);
				break;
			}
		default:
			break;
	}

	#endif 

	}

void write_memory_mode (int MODE) {
	#ifdef PRINTF_DEFINE
	printf("Changing Memory Mode :\n");
	#endif

	
	// Send memory WDMR
	switch(MODE){
		case MEMORY_BYTE_MODE :
			{
				// Send 0000 0001 to initiate writing 
				SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
				
				SPI_SEND(spi, 0x01);
				// Force Byte Mode (00) 
				SPI_SEND(spi, 0x00);
				SPI_SELECT(spi, SPIDEV_USER(1) , false);
				break;
			}
		case MEMORY_PAGE_MODE :
			{
				SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
				SPI_SEND(spi, 0x01);

				// Force Page Mode (10)
				SPI_SEND(spi, 0x80);
				SPI_SELECT(spi, SPIDEV_USER(1) , false);
				break;
			}
		case MEMORY_SEQUENTIAL_MODE :
			{
				SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
				SPI_SEND(spi, 0x01);
				
				// Force Sequential Mode (01)
				SPI_SEND(spi, 0x40);
				SPI_SELECT(spi, SPIDEV_USER(1) , false);
				break;
			}		
		case MEMORY_RESERVED :
			{
				SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
				SPI_SEND(spi, 0x01);
				// Force Reserved Mode (11)
				SPI_SEND(spi, 0xC0); 
				SPI_SELECT(spi, SPIDEV_USER(1) , false);
				break;
			}
		default:
			break;
	}	

}