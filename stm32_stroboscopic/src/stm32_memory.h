#ifndef __CONFIG_STM32_STROBOSCOPIC_SRC_STM32_MEMORY_H
#define __CONFIG_STM32_STROBOSCOPIC_SRC_STM32_MEMORY_H


/* Memory Modes 
	To find out the memory current mode, we need to send 
	a Read Mode Register Instruction (RDMR), wich is essentially 
	00 00 01 01 (0x05) to the memory, and wait for a byte of time
	for the response.

	The memory will reply with the MSB first (from 7 to 0).
	The bits 5 to 0 are reserved and are generaly set to 0.
	
	Modes :
	
	0 : 00 00 00 00 : Byte mode
	128 : 10 00 00 00 : Page mode
	64 : 01 00 00 00 : Sequential mode (default operation) 
	192 : 11 00 00 00 : Reserved

	The defines are done through the unsigned integer values
*/ 

/* 
	Procedure for verifying the functionality of the READ memory

	FAR uint8_t buf[10] = {0};
	
	int i;
	for (i=0; i < 10 ; i++){
	SPI_SELECT(spi, SPIDEV_USER(1) , true); // Select RAM
	// Send the Read mode register instruction and read the response
	SPI_SEND(spi, 0x05);
	SPI_RECVBLOCK(spi, &buf[i], 1);
	//printf("Mode : %d\n", SPI_SEND(spi, 0x05));
	SPI_SELECT(spi, SPIDEV_USER(1) , false);	
	}
	for (i=0; i < 10 ; i++){
		printf("SPI MODE : %u\n", buf[i]);
	}

*/ 

/*******************************************************
	Printf define for debugging
 *******************************************************/

#undef PRINTF_DEFINE
// #define PRINTF_DEFINE 1
 
/*******************************************************
	Defines
 *******************************************************/

// Memory Modes
#define MEMORY_BYTE_MODE 0
#define MEMORY_PAGE_MODE 128
#define MEMORY_SEQUENTIAL_MODE 64 
#define MEMORY_RESERVED 192

// IOCTL Commands
#define SET_MEMORY_MODE 0
#define CS_CONDITION 1
#define SET_CS_MEMORY 2
#define SET_READ_CONDITION 3

/*******************************************************
	Prototypes
 *******************************************************/

void check_memory_mode (void); // Check Memory current mode
void write_memory_mode (int MODE); // Change Memory current mode, to be controled from IOCTL

/*******************************************************
	Structures
 *******************************************************/

// Sets the current mode with IOCTL, as well as the activation status
// for the Sequential mode : we need to send the write command and the address once

struct MEMORY {
	int mode; 
	int write_activated; 
	int read_activated;
	int CS; // Controls the CS within write and read
};



#endif 

