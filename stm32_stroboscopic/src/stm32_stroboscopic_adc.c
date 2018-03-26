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
#include "stm32_spi.h"
#include "stm32.h"

void send_extadc (unsigned short entree);

typedef FAR struct file		file_t;

static int     adcext_open(file_t *);
static int     adcext_close(file_t *);
static ssize_t adcext_read(file_t *, FAR char *, size_t );
static ssize_t adcext_write(file_t *, FAR const char *, size_t );
static int     adcext_ioctl(FAR struct file *, int , unsigned long );

static const struct file_operations adcext_ops = { // External ADC file operations
	adcext_open,
	adcext_close,
	adcext_read,
	adcext_write,
	0,
	adcext_ioctl,
};

FAR struct spi_dev_s * spi; // SPI Control

/* Reading the recent stm32_adc board schematic, 
 * we can find out the CS, SCK, MISO, MOSI pins of the external ADC  
 * PB3 : SCK
 * PB4 : MISO
 * PB5 : MOSI
 */

static int adcext_open(file_t * filep){
	printf("Openning ADC SPI Control, locking SPI..\n");

	// Initialise the SPI 
	stm32_spidev_initialize();
	spi=stm32_spibus_initialize(1); // The STM32F334 has only 1 SPI bus.

	if (spi == NULL){
		printf("Unsupported SPI BUS \n");
	}
	// Lock SPI on open, unlock on close
	(void)SPI_LOCK(spi, true);

	// Used to verify SPI status for the memory driver
	spi->ops->status(spi, 1);
	// Mohamed :: Changed from SPIDEV_MOD1, to check the memory workings
  	SPI_SETMODE(spi, SPIDEV_MODE0); // Mohamed :: Changed from SPIDEV_MODE2
  	SPI_SETBITS(spi, 8); // 8 bit word (used later in the strobo main prog)
  	(void)SPI_HWFEATURES(spi, 0);

  	printf("SPI Locked..\n");

  	return (0);
	}

static int adcext_close(file_t *filep) {
  printf("Closing ADC control, unlocking SPI... \n");
  (void)SPI_LOCK(spi, false);

  // Used to verify SPI status for the memory driver
  spi->ops->status(spi, 0);
  
  printf("SPI unlocked..\n");
  return OK;
  }

static ssize_t adcext_read(file_t *filep, FAR char *buf, size_t buflen) {
	//int i;
	  // arch/arm/src/stm32/stm32_spi.c:  
	  // .select            = stm32_spi1select,

	// Control of SPI_SELECT transfered to IOCTL for the MEMORY MISO issue

	// Receive a continuous stream of bytes until the buffer ends

	// Check if the SPI_SELECT are needed directly here 

	//SPI_SELECT(spi, SPIDEV_USER(0), true);

	//for (i=0; i < buflen ; i++){
	//SPI_RECVBLOCK(spi, &buf[i], buflen); 
	//}

	// Send something to initiate clock cycles
//	SPI_SEND(spi, 0xFF);
	

	//SPI_RECVBLOCK(spi, &buf[0], 4);
	/*
	*/

	// Testing the IRQ Flag : Don't allow the scheduler to switch contexts 
	// within the call to read 

	//irqstate_t flags;

	// flags = enter_critical_section();
	SPI_SEND(spi, 0xFF);
	SPI_SEND(spi, 0xFF);
	SPI_SEND(spi, 0xFF);
	SPI_SEND(spi, 0xFF);

	
	//SPI_RECVBLOCK(spi, &buf[0], 4);
//	SPI_RECVBLOCK(spi, &buf[0], 1);
//	SPI_RECVBLOCK(spi, &buf[0], 1);
//	SPI_RECVBLOCK(spi, &buf[0], 1);
	//spi->ops->send(spi, 0xFFFF);
	//spi->ops->send(spi, 0xFFFF);
	//spi->ops->send(spi, 0xFFFF);
	//spi->ops->send(spi, 0xFFFF);

	// leave_critical_section(flags);
	
	//SPI_RECVBLOCK(spi, &buf[1], buflen);
	
	// Check if the SPI_SELECT are needed directly here 

	//SPI_SELECT(spi, SPIDEV_USER(0), false);

	//printf("BUFFER : %u\n", buf[i]);

	// Control of SPI_SELECT transfered to IOCTL for the MEMORY MISO issue
 	
 	/*
 	for (i=0; i < buflen ; i++){
 		printf("buf : %d\n", buf[i]);
	}
	*/
 	return buflen;
  }

static ssize_t adcext_write(file_t *filep, FAR const char *buf, size_t buflen){
 	
 	//printf("Sending a word : %d  \n", *buf);
 	printf("Sending a word : %c  \n", *buf);
	send_extadc(*buf);      

 return buflen;
 }


static int adcext_ioctl(FAR struct file * filep, int cmd, unsigned long arg){

	switch(cmd){
		// This command controls the selection of the ADC, must be used at all times 
		// when reading from it.
		// It also solves the MISO control problem
		case SELECT_ADC : 
		{
			if ((bool)arg == true){

				// Force MISO for the MEMORY on off state, otherwise
    			// it will influence our reading sequence from the ADC
				
				stm32_unconfiggpio(GPIO_SPI2_MISO);

				printf("Selecting ADC, disabling MISO of MEMORY\n");
				
				SPI_SELECT(spi, SPIDEV_USER(0), true);

			} 

			if((bool)arg == false){
				// Force MISO for the MEMORY on ON State, when not selecting the ADC
				
				SPI_SELECT(spi, SPIDEV_USER(0), false);
				
				stm32_configgpio(GPIO_SPI2_MISO);
				
				printf("De-Selecting ADC, enabling MISO of MEMORY\n");
			}

		}

	}

	return OK;
}



void send_extadc (unsigned short entree){ // 16 bit sent
 SPI_SELECT(spi, SPIDEV_USER(0), true); // which component on the SPI bus to select
  SPI_SEND(spi, entree);
  SPI_SELECT(spi, SPIDEV_USER(0), false);
  }


void stm32_stroboscopic_adc_setup (void){
  (void)register_driver("/dev/externaladc", &adcext_ops, 0444, NULL);
}