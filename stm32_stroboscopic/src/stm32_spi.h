#ifndef __CONFIG_STM32_STROBOSCOPIC_SRC_STM32_SPI_H
#define __CONFIG_STM32_STROBOSCOPIC_SRC_STM32_SPI_H

// SPI 

#define UNLOCKED 0
#define LOCKED 1
#define GETSTATUS 2

// Mohamed :: SPI Lock condition structure, this strucute is used 
// to verify the SPI locking mechanism from ADC to memory.
struct SPI_condition {
  bool condition;
};

struct SPI_condition SPI_STATUS;

// ADC SPI related

#define SELECT_ADC 0

#endif