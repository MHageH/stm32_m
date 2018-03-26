#ifndef __CONFIG_STM32_STROBOSCOPIC_SRC_STM32_HRTIM_CONTROL_H
#define __CONFIG_STM32_STROBOSCOPIC_SRC_STM32_HRTIM_CONTROL_H

// STM32 HRTIM Control driver specific MACROS

// Can be also defined at strobo.h application layer

// IOCTL
#define STM32_HRTIM_MOD_DUTY 0
#define STM32_HRTIM_MOD_TIMD_CH2_SET 1
#define STM32_HRTIM_MOD_TIMD_CH2_RST 2
#define STM32_HRTIM_MOD_ENABLE_HRTIMD_CH2 3 
#define STM32_HRTIM_MOD_ENABLE_HRTIMD_CH1 4
#define STM32_HRTIM_IRQ_TESTING 5
#define STM32_HRTIM_IRQ_CLEAR_FLAGS 6
#define STM32_HRTIM_IRQ_STATUS 7
#define STM32_HRTIM_ISR_HANDLER_EXECUTION 8


// Mohamed :: Added a special structure to modifiy the HRTIM parameters

struct stm32_hrtim_s
{
	uint8_t timer; // Timer to use
	bool running; // True : if the timer is running
	struct hrtim_dev_s * hrtim; // Handle returned by stm32_hrtiminitialize()

	uint32_t counter; // Counter to test the interruption counts
};

//


#endif
