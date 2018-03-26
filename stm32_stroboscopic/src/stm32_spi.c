#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <debug.h>
#include <errno.h>
#include <stdio.h>

#include <nuttx/spi/spi.h>
#include <arch/board/board.h>

#include "up_arch.h"
#include "chip.h"
#include "stm32.h"

#include "stm32_spi.h"

#include "stm32f334-disco.h"


// Check if SPI1 is used
#if defined(CONFIG_STM32_SPI1) // There's only 1 SPI for STM32F334C8

#ifdef CONFIG_STM32_SPI1
struct spi_dev_s * g_spi1;
#endif 


// Mohamed :: Added support for CS of the RAM 
#define GPIO_SPI_CS_RAM \
    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
     GPIO_OUTPUT_SET | GPIO_PORTA | GPIO_PIN4)


// Define MOSI, MISO and SCK for the ADC

void weak_function stm32_spidev_initialize(void) {

  #ifdef CONFIG_STM32_SPI1

	printf("STM32F334C8 Stroboscopic boot : SPI1\n");

	g_spi1 = stm32_spibus_initialize(1);
	if(!g_spi1) {
		      spierr("ERROR: FAILED to initialize SPI port 1\n");
	}

  printf("SPI MISO, MOSI, SCK initialzed\n");

  // Mohamed :: Mod_ :: SPI on the other peripherals

  stm32_configgpio(GPIO_SPI2_SCK);
  stm32_configgpio(GPIO_SPI2_MISO);
  stm32_configgpio(GPIO_SPI2_MOSI);

  printf("Initialized SPI Pins for the RAM\n");

  stm32_configgpio(GPIO_SPI_CS_RAM);

  printf("Initialized SPI CS for RAM\n");

  // Maximal clock frequency for SPI SCK on the ADC : 51 MHz
  // Maxmial clock frequency for the MEMORY : 20 MHz

  // The logic analyser have problems catching with the SPI bus at 20 MHz
  SPI_SETFREQUENCY(g_spi1, 10000000);
  #endif
}

#ifdef CONFIG_STM32_SPI1

void stm32_spi1select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
{
  spiinfo("devid: %d Pin: %s\n", (int)devid, selected ? "assert" : "de-assert");
    {

      switch(devid){
        case SPIDEV_USER(0) :
            {
              // No chip select is needed for the ADC 
                break;
            }
        case SPIDEV_USER(1) :
            {
                // Mohamed :: Mod_ :: Select Memory
               // printf("Mohamed :: CS RAM selected\n");
                stm32_gpiowrite(GPIO_SPI_CS_RAM, !selected);
                break;
            }
        }
    }
}

uint8_t stm32_spi1status(FAR struct spi_dev_s *dev, uint32_t devid)
{
  // arch/arm/...stm32/stm32_spi.c
  // static int spi_lock(FAR struct spi_dev_s *dev, bool lock) 


  // Mohamed :: SPI status
  struct SPI_condition * SPI_CURRENT_STATUS = &SPI_STATUS;
  switch(devid){

    case (UNLOCKED):
      {
        SPI_CURRENT_STATUS->condition = UNLOCKED;
        break;
      }
    case (LOCKED) :
      { 
        SPI_CURRENT_STATUS->condition = LOCKED;
        break;  
      }
    case (GETSTATUS) :
      {
        if(SPI_CURRENT_STATUS->condition){
          return 1;
        } else {
          return 0;
        } 
        break;
      }
  }


  return 0;
}
#endif

#ifdef CONFIG_SPI_CMDDATA
#ifdef CONFIG_STM32_SPI1
int stm32_spi1cmddata(FAR struct spi_dev_s *dev, uint32_t devid, bool cmd) {
  return OK;
}
#endif

#endif /* CONFIG_SPI_CMDDATA */

#endif /* CONFIG_STM32_SPI1 */