
#include <avr32/io.h>
#include "compiler.h"
#include "board.h"
#include "gpio.h"
#include "spi.h"
#include "conf_sd_mmc_spi.h"
#include "sd_mmc_spi.h"
#include "usart.h"
#include "print_funcs.h"
#include "pdca.h"
#include "intc.h"
#include "pm.h"


//cgvhbn

const char dummy_data[] =
#include "dummy.h"
;

#define PBA_HZ                FOSC0

//! \brief Number of bytes in the receive buffer when operating in slave mode
#define BUFFERSIZE            64
#define AVR32_PDCA_CHANNEL_USED_RX AVR32_PDCA_PID_SPI0_RX
#define AVR32_PDCA_CHANNEL_USED_TX AVR32_PDCA_PID_SPI0_TX

#define AVR32_PDCA_CHANNEL_SPI_RX 0 // In the example we will use the pdca channel 0.
#define AVR32_PDCA_CHANNEL_SPI_TX 1 // In the example we will use the pdca channel 1.

volatile avr32_pdca_channel_t* pdca_channelrx ;
volatile avr32_pdca_channel_t* pdca_channeltx ;

volatile bool end_of_transfer;

// Local RAM buffer for the example to store data received from the SD/MMC card
volatile char ram_buffer[1000];

void wait(){
	volatile int i;
	for(i = 0 ; i < 5000; i++);
}//Wait

#if defined (__GNUC__)
__attribute__((__interrupt__))
#elif defined (__ICCAVR32__)
__interrupt
#endif

static void pdca_int_handler(void){
	Disable_global_interrupt();
	pdca_disable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
	sd_mmc_spi_read_close_PDCA();
	wait();
	pdca_disable(AVR32_PDCA_CHANNEL_SPI_TX);
	pdca_disable(AVR32_PDCA_CHANNEL_SPI_RX);
	Enable_global_interrupt();
	end_of_transfer = true;
}//pdca_int_handler

static void sd_mmc_resources_init(void) {

	static const gpio_map_t SD_MMC_SPI_GPIO_MAP = {
		{SD_MMC_SPI_SCK_PIN,  SD_MMC_SPI_SCK_FUNCTION },  // SPI Clock.
		{SD_MMC_SPI_MISO_PIN, SD_MMC_SPI_MISO_FUNCTION},  // MISO.
		{SD_MMC_SPI_MOSI_PIN, SD_MMC_SPI_MOSI_FUNCTION},  // MOSI.
		{SD_MMC_SPI_NPCS_PIN, SD_MMC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};//SPI Map

	spi_options_t spiOptions = {
		.reg          = SD_MMC_SPI_NPCS,
		.baudrate     = 57600,//SD_MMC_SPI_MASTER_SPEED,  // Defined in conf_sd_mmc_spi.h.
		.bits         = SD_MMC_SPI_BITS,          // Defined in conf_sd_mmc_spi.h.
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};//SPI Options

	gpio_enable_module(SD_MMC_SPI_GPIO_MAP,
	sizeof(SD_MMC_SPI_GPIO_MAP) / sizeof(SD_MMC_SPI_GPIO_MAP[0]));
	spi_initMaster(SD_MMC_SPI, &spiOptions);
	spi_selectionMode(SD_MMC_SPI, 0, 0, 0);
	spi_enable(SD_MMC_SPI);
	sd_mmc_spi_init(spiOptions, PBA_HZ);

}//sd_mmc_resources_init

void local_pdca_init(void){

	pdca_channel_options_t pdca_options_SPI_RX ={ //RX
		.addr = ram_buffer,						  //Dummy data h
		.size = 512,                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = AVR32_PDCA_CHANNEL_USED_RX,        // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
	};//pdca_options_SPI_RX

	pdca_channel_options_t pdca_options_SPI_TX ={ //TX
		.addr = (void *)&dummy_data,              // memory address.
		.size = 512,                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = AVR32_PDCA_CHANNEL_USED_TX,        // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
	};//pdca_options_SPI_TX

	pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_TX, &pdca_options_SPI_TX);
	pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_RX, &pdca_options_SPI_RX);
	INTC_register_interrupt(&pdca_int_handler, AVR32_PDCA_IRQ_0, AVR32_INTC_INT1);  // pdca_channel_spi1_RX = 0

} //local_pdca_init

int main(void){

	int i, j;
	pm_switch_to_osc0(&AVR32_PM, PBA_HZ, 3);
	init_dbg_rs232(PBA_HZ);
	print_dbg("\r\nInit SD/MMC Driver");
	print_dbg("\r\nInsert SD/MMC...");
	INTC_init_interrupts();
	sd_mmc_resources_init();

	while (!sd_mmc_spi_mem_check());
	print_dbg("\r\nCard detected!");

	sd_mmc_spi_get_capacity();
	print_dbg("Capacity = ");
	print_dbg_ulong(capacity >> 20);
	print_dbg(" MBytes");

	Enable_global_interrupt();

	local_pdca_init();

	for(j = 1; j <= 3; j++){ //3 Sectores

		pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_RX, &ram_buffer,512);
		pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_TX,(void *)&dummy_data,512); //send dummy
		end_of_transfer = false;

		if(sd_mmc_spi_read_open_PDCA (j)){

			print_dbg("\r\nFirst 512 Bytes of Transfer number ");
			print_dbg_ulong(j);
			print_dbg(" :\r\n");

			spi_write(SD_MMC_SPI,0xFF); // Write a first dummy data to synchronize transfer
			pdca_enable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
			pdca_enable(AVR32_PDCA_CHANNEL_SPI_RX);
			pdca_enable(AVR32_PDCA_CHANNEL_SPI_TX);
			/*
			pdca_channelrx =(volatile avr32_pdca_channel_t*) pdca_get_handler(AVR32_PDCA_CHANNEL_SPI_RX); // get the correct PDCA channel pointer
			pdca_channeltx =(volatile avr32_pdca_channel_t*) pdca_get_handler(AVR32_PDCA_CHANNEL_SPI_TX); // get the correct PDCA channel pointer
			pdca_channelrx->cr = AVR32_PDCA_TEN_MASK; // Enable RX PDCA transfer first
			pdca_channeltx->cr = AVR32_PDCA_TEN_MASK; // and TX PDCA transfer
			*/
			while(!end_of_transfer);

			for( i = 0; i < 20; i++){ //20 primeros bytes
				print_dbg_char_hex( (U8)(*(ram_buffer + i)));
			}//For
		}else{
			print_dbg("\r\n! Unable to open memory \r\n");
		}//IF
	}//For

	print_dbg("\r\nEnd of the example.\r\n");

	while (1);

}//Main
