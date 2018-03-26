#include <nuttx/config.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <debug.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/boardctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <nuttx/board.h>
#include <nuttx/fs/fs.h>

#include <nuttx/analog/comp.h>
#include <nuttx/analog/dac.h>

#include <nuttx/power/powerled.h>

#include "stm32_comp.h"
#include "stm32_hrtim.h"

// MACROS
#include "stm32_hrtim_control.h"

#if !defined(CONFIG_STM32_HRTIM1) || !defined(CONFIG_HRTIM)
#  error "Powerled example requires HRTIM1 support"
#endif

/* The struct hrtim_dev_s is defined in arc/arm/src/stm32/stm32_hrtim.h 

struct hrtim_dev_s
{
#ifdef CONFIG_HRTIM
  // Fields managed by common upper half HRTIM logic 

  uint8_t hd_ocount; // The number of times the device has been opened 
  sem_t hd_closesem; // Locks out new opens while close is in progress 
#endif

  // Fields provided by lower half HRTIM logic 

  FAR const struct stm32_hrtim_ops_s *hd_ops; // HRTIM operations 
  FAR void *hd_priv;                          // Used by the arch-specific logic 
  bool initialized;                           // true: HRTIM driver has been initialized 
};

  The correct IRQ settings are defined in stm32f33xxx_irq.h

  Trying to implement the role of a HRTIM PWM.
*/


// ******************************************************************
// ************************* Structures Defs ************************
// ******************************************************************
struct hrtim_dev_s * hrtim;
typedef FAR struct file 	file_t;
bool hrtim_configured = false;


// The hrtim_config structures might not be necessary after all
struct hrtim_config_ch1 {
	uint16_t per;
	uint16_t cmp; 
	uint8_t enabled;
};

struct hrtim_config_ch2{
	uint16_t cmp2;
	uint16_t cmp3;
	uint8_t enabled;
};

struct hrtim_config_ch1 HRTIMD_CONFIG_CH1;
struct hrtim_config_ch2 HRTIMD_CONFIG_CH2;

// ******************************************************************
// ************************* Prototypes *****************************
// ******************************************************************
// Check HRTIM
static int check_hrtim(struct hrtim_dev_s * hrtim);
static int cmp2_mask(uint16_t reg);
static int set2x_mask(uint16_t reg);

// Function calls
static int stm32_hrtim_open(file_t *);
static int stm32_hrtim_close(file_t *);
static ssize_t stm32_hrtim_read(file_t *, FAR char *, size_t );
static ssize_t stm32_hrtim_write(file_t *, FAR const char *, size_t );
static int     stm32_hrtim_ioctl(FAR struct file *, int , unsigned long );

// ******************************************************************
// ************************* Main structure *************************
// ******************************************************************

static const struct file_operations stm32_hrtim_control_ops ={
	stm32_hrtim_open,
	stm32_hrtim_close,
	stm32_hrtim_read,
	stm32_hrtim_write,
	0,
	stm32_hrtim_ioctl,
};

static int stm32_hrtim_open(file_t * filep){
	bool initialize = false;

	// Necessary, even if the driver is already intialized

	if (!initialize){
		printf("HRTIM uninitialized, initalising...\n");
		hrtim = stm32_hrtiminitialize();
		initialize = true;
	}

	if (!hrtim_configured){
		// ##################################################
		// HRTIMD CH1 configuration 
		// ##################################################
		
		struct hrtim_config_ch1 * HRTIMD_CH1_LOCAL_CONF = &HRTIMD_CONFIG_CH1;
		
		/*

		HRTIMD_CH1_LOCAL_CONF->per = 55300; // 12 us gap
		HRTIMD_CH1_LOCAL_CONF->cmp = 2304; // 0.5 us step :

		*/ 

		check_hrtim(hrtim);

		// Disable HRTIM PWM, for configuration
		// stm32_hrtim.c  :
		/*
		#if defined(CONFIG_STM32_HRTIM_PWM)
		static int hrtim_outputs_config(FAR struct stm32_hrtim_s *priv);
		*/

		// printf("Configuring the hrtim control driver...\n");
		//HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH1, false); 

		/*
		HRTIM_PER_SET(hrtim, HRTIM_TIMER_TIMD, HRTIMD_CH1_LOCAL_CONF->per); 
		HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP1, HRTIMD_CH1_LOCAL_CONF->cmp); 

		//printf("Enabling the hrtim control driver...\n");
		
		HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH1, true); // HRTIMD CH1 Configuration
		
		*/

		// printf("HRTIM TIMD Channel 1 enabled \n");

		HRTIMD_CH1_LOCAL_CONF->enabled=0;

		// ##################################################
		// HRTIMD CH2 configuration 
		// ##################################################

		// Testing the deactivation of start_conv at the start :
		struct hrtim_config_ch2 * HRTIMD_CH2_LOCAL_CONF = &HRTIMD_CONFIG_CH2;

/*
		HRTIMD_CH2_LOCAL_CONF->cmp2 = 4148; // 0.9 us gap
		HRTIMD_CH2_LOCAL_CONF->cmp3 = 9217; // 2 us step 
*/
		// We want to use CH2 instead of CH1, since our physical PIN is connected to PB15
		// HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH2, false); 
		
		//HRTIM_PER_SET(hrtim, HRTIM_TIMER_TIMD, HRTIMD_CH1_LOCAL_CONF->per); 

/*
		HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP2, HRTIMD_CH2_LOCAL_CONF->cmp2);
		HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP3, HRTIMD_CH2_LOCAL_CONF->cmp3);

*/
		// Set hrtim_configured flag to true 
		hrtim_configured = true;

		// HRTIMD CH2 Control transfered to IOCTL 
		//HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH2, true); // HRTIMD CH2 Configuration
		//printf("HRTIM TIMD Channel 2 enabled \n");

		// Set the HRTIMD CH2 Control flag to false : HRTIMD CH2 is still not enabled
		HRTIMD_CH2_LOCAL_CONF->enabled=0;

	}
		// Enable HRTIM PWM : We should now detect a signal on IOupdate, if the
		// stroboscopic application is used
  	return (0);
	}

static int stm32_hrtim_close(file_t * filep){
	printf("Disabling the HRTIM TIMD CH1 output...\n");
	HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH1, false); 
	printf("Disabling the HRTIM TIMD CH2 output...\n");
	HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH2, false);
	return OK;
	}

static ssize_t stm32_hrtim_read(file_t * filep, FAR char * buf, size_t buflen){
	// void for the moment
	return OK;
	}

static ssize_t stm32_hrtim_write(file_t * filep, FAR const char * buf, size_t buflen){
	// void for the moment
	return OK;
	}


	int ret;
	struct hrtim_config_ch1 * HRTIMD_CH1_LOCAL_CONF = &HRTIMD_CONFIG_CH1;
	struct hrtim_config_ch2 * HRTIMD_CH2_LOCAL_CONF = &HRTIMD_CONFIG_CH2;


static int     stm32_hrtim_ioctl(FAR struct file * filep, int cmd, unsigned long arg){
	/*
	The IOCTL function should be controlled from the application level to control the duty 
	cycle of the HRTIM.
	The commands would be defined in the stm32_hrtim_control.h, since it's called early at program 
	compilation.	
	*/


	// The definition are made outside of ioctl function to reduce the access time
	/*

	int ret;
	struct hrtim_config_ch1 * HRTIMD_CH1_LOCAL_CONF = &HRTIMD_CONFIG_CH1;
	HRTIMD_CH1_LOCAL_CONF->per = 55300; // Initiate periode for the timer (global on both channels).

	struct hrtim_config_ch2 * HRTIMD_CH2_LOCAL_CONF = &HRTIMD_CONFIG_CH2;

	*/


	// Mohamed :: Tests
	// printf("HRTIM IOCTL called\n");

	switch(cmd) {
		default:
		{
			ret = -ENOSYS;
			break;
		}
		case STM32_HRTIM_MOD_DUTY:
		{
			// Mohamed :: Tests
			// printf("HRTIM MOD Duty invoked, arg is : %d, per is %d\n", (uint16_t)arg, HRTIMD_CH1_LOCAL_CONF->cmp);
			HRTIMD_CH1_LOCAL_CONF->cmp = (uint16_t)arg;
			HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP1, HRTIMD_CH1_LOCAL_CONF->cmp); // 5% duty cycle
			break;
		}
		case STM32_HRTIM_MOD_TIMD_CH2_SET:
		{
			// Mohamed :: Tests
			//printf("HRTIM MOD for CH2 SET (CMP2) invoked, arg is : %d \n\n", (uint16_t)arg);
			HRTIMD_CH2_LOCAL_CONF->cmp2 = (uint16_t)arg;
			HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP2, HRTIMD_CH2_LOCAL_CONF->cmp2);			
			break;
		}
		case STM32_HRTIM_MOD_TIMD_CH2_RST:
		{
			// Mohamed :: Tests
			//printf("HRTIM MOD for CH2 RST (CMP3) invoked, arg is : %d \n", (uint16_t)arg);
			HRTIMD_CH2_LOCAL_CONF->cmp3 = (uint16_t)arg;
			HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP3, HRTIMD_CH2_LOCAL_CONF->cmp3);
			break;
		}
		// Must be called for the activation of HRTIMD CH2
		case STM32_HRTIM_MOD_ENABLE_HRTIMD_CH2:
		{
			if (HRTIMD_CH2_LOCAL_CONF->enabled == 0){
				HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH2, false); 
				

				HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH2, (bool)arg); // HRTIMD CH2 Configuration
				
				// printf("Enabled HRTIMD CH2\n");
				/*
				for (int j =0; j < 750000; j++){
					// Initial wait to enable the HRTIM initialization
				}
				*/

				HRTIMD_CH2_LOCAL_CONF->enabled = 1;
			} 
				break;
		}
		case STM32_HRTIM_MOD_ENABLE_HRTIMD_CH1:
		{

			// Check the flag if it is enabled or not 

			if (HRTIMD_CH1_LOCAL_CONF->enabled == 0){
				// HRTIMD_CH1_LOCAL_CONF->per = 55300; // 12 us gap
				// HRTIMD_CH1_LOCAL_CONF->cmp = 2304; // 0.5 us step :
				HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH1, false); 


				//HRTIMD_CH1_LOCAL_CONF->per = 55300; // 12 us gap
				HRTIMD_CH1_LOCAL_CONF->per = 65535; // 12 us gap
				HRTIMD_CH1_LOCAL_CONF->cmp = 2304; // 0.5 us step :


				HRTIM_PER_SET(hrtim, HRTIM_TIMER_TIMD, HRTIMD_CH1_LOCAL_CONF->per); 
				HRTIM_CMP_SET(hrtim, HRTIM_TIMER_TIMD, HRTIM_CMP1, HRTIMD_CH1_LOCAL_CONF->cmp); 

			
				HRTIM_OUTPUTS_ENABLE(hrtim, HRTIM_OUT_TIMD_CH1, (bool)arg); // HRTIMD CH1 Configuration
	
				// Set the flag to enabled
				HRTIMD_CH1_LOCAL_CONF->enabled = 1;
			}
			
			break;
		}
		case STM32_HRTIM_IRQ_TESTING:
		{
			// The interrupt handling is defined in board.h at HRTIM_IRQ_MCMP2
			// Master Compare 2 Interrupt

			printf("HRTIM IRQ GET %u\n", HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD));
			//printf("HRTIM IRQ GET 2 %u\n", HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD));

			break;
		}
		case STM32_HRTIM_IRQ_CLEAR_FLAGS:
		{
			//if (HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD) == 15895){
		//	if (cmp2_mask(HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD)) == 1){
			//	HRTIM_IRQ_ACK(hrtim, HRTIM_TIMER_TIMD, 2);
			//if (set2x_mask(HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD)) == 1){
				HRTIM_IRQ_ACK(hrtim, HRTIM_TIMER_TIMD, 2048);
				//printf("HRTIM IRQ GET %u\n", HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD));	
				//printf("Acquited\n");
			//}
			break;
		}
		case STM32_HRTIM_IRQ_STATUS:
		{
		//	if (HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD) == 8720){
			//if(cmp2_mask(HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD))){
			if(set2x_mask(HRTIM_IRQ_GET(hrtim, HRTIM_TIMER_TIMD))){
				ret = 1;
			} else {
				ret = 0;
			}
			break;
		}
	}



	return ret;
	}


// ******************************************************************
// ************************* Functions *****************************
// ******************************************************************

void stm32_hrtim_control_setup(void){
	(void)register_driver("/dev/hrtimcontrol", &stm32_hrtim_control_ops, 0444, NULL);
}

static int check_hrtim(struct hrtim_dev_s * hrtimc){
	if(hrtimc->initialized){
		printf("HRTIM driver initialized.\n");
		return OK;
	} 
	else 
	{
		printf("ERROR: failed to get hrtim\n");
		return 1;
	}
}

static int cmp2_mask(uint16_t reg){
	uint16_t mask = 0x0002; // 0000 0000 0000 0010 : The CMP2 register

//	printf("cmp2_mask : %u\n", ( (mask &reg) >> 1) );
	return ((mask &reg) >> 1);

}

static int set2x_mask(uint16_t reg){
	uint16_t mask = 0x0800; // 0000 1000 0000 0000 : The RSTX2 register

	return ((mask &reg) >> 11);
}