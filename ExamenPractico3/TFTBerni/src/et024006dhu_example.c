#define dbg 0
#include <avr32/io.h>
#include "compiler.h"
#include "board.h"
#include "gpio.h"
#include "spi.h"
#include "conf_sd_mmc_spi.h"
#include "sd_mmc_spi.h"
#include "usart.h"
#include "pdca.h"
#include "intc.h"
#include "pm.h"
#include "et024006dhu.h"
#include "delay.h"
#include "pwm.h"
#include "avr32_logo.h"
#include "print_funcs.h"

//SPI SDCARD
const char dummy_data[] =
#include "dummy.h"
;//Dummy_data

//GPIOs
#define BTN_UP   AVR32_PIN_PB22
#define BTN_DOWN AVR32_PIN_PB23
#define BTN_RIGHT AVR32_PIN_PB24
#define BTN_LEFT AVR32_PIN_PB25
#define BTN_CENTER AVR32_PIN_PB26

#define LED0   AVR32_PIN_PB27
#define LED1   AVR32_PIN_PB28
#define LED2   AVR32_PIN_PA05
#define LED3   AVR32_PIN_PA06

#define PBA_HZ                12000000
#define BUFFERSIZE            64    //Number of bytes in the receive buffer in slave
#define AVR32_PDCA_CHANNEL_USED_RX AVR32_PDCA_PID_SPI0_RX
#define AVR32_PDCA_CHANNEL_USED_TX AVR32_PDCA_PID_SPI0_TX
#define AVR32_PDCA_CHANNEL_SPI_RX 1 // In the example we will use the pdca channel 0.
#define AVR32_PDCA_CHANNEL_SPI_TX 2 // In the example we will use the pdca channel 1.

//USART POR DMA
#define AVR32_PDCA_CHANNEL_USED_RX_USART  AVR32_PDCA_PID_USART0_RX
#define AVR32_PDCA_CHANNEL_USART_RX 0 // In the example we will use the pdca channel 2 for USART0 in RX.

//State machine logic
enum btn{NONE, UP, DOWN, LEFT, RIGHT, CENTER};
enum btn btn_pressed = NONE;
volatile int state = 0;

//Variables
avr32_pwm_channel_t pwm_channel6 = {.cdty = 0,.cprd = 100};
volatile avr32_pdca_channel_t* pdca_channelrx ;
volatile avr32_pdca_channel_t* pdca_channeltx ;
volatile bool end_of_transfer; //DMA SD flag
volatile char ram_buffer[1000]; //DMA SD buffer
volatile uint8_t usart_message_rx_complete = 0;
volatile char usart_message [51] = {"NoMessage\0"}; //Read from USART (UP Key)
volatile uint8_t Sector_Counter = 0; //Current sector
char sector_counter_print[1]; //Converts current sector to string
char usart_message_print[51]; //Converts message to string

//Functions
static void tft_bl_init(void);
void CLR_disp(void);
__attribute__((__interrupt__)) static void pdca_int_handler(void);
__attribute__((__interrupt__)) static void pdca_int_handler_usart(void);
__attribute__((__interrupt__)) void btn_interrupt_routine (void);
static void sd_mmc_resources_init(void);
void local_pdca_init(void);
void leds(uint8_t value);
void init_button_interrupt(void);

/**************************************************************************/

int main(void){

	int i, j; //j for sectors, i for bytes

	//PM
	pm_switch_to_osc0(&AVR32_PM, PBA_HZ, 3);
	if(dbg) init_dbg_rs232(PBA_HZ);

	//Key interrupts
	init_button_interrupt();

	//SDCARD
	sd_mmc_resources_init();
	REINIT:while (!sd_mmc_spi_mem_check());

	//DMA for SDCARD
	Enable_global_interrupt();
	local_pdca_init(); //DMA initialization INCLUDES USART and SPI

	//DMA for USART
	//First Configuring USART0
	static const gpio_map_t USART_GPIO_MAP ={
	  {AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION}
	};// USART GPIOs
	static const usart_options_t USART_OPTIONS ={
	  .baudrate     = 9600,
	  .charlength   = 8,
	  .paritytype   = USART_NO_PARITY,
	  .stopbits     = USART_1_STOPBIT,
	  .channelmode  = USART_NORMAL_CHMODE
	};// USART options

	if(!dbg) gpio_enable_module(USART_GPIO_MAP,sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));
	if(!dbg) usart_init_rs232(&AVR32_USART0, &USART_OPTIONS, 12000000);

	//TFT
	et024006_Init( FOSC0, FOSC0 );
	tft_bl_init();
	CLR_disp();

	while(pwm_channel6.cdty < pwm_channel6.cprd){
		pwm_channel6.cdty++;
		pwm_channel6.cupd = pwm_channel6.cdty;
		pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
		delay_ms(1);
	}//PWM
	CLR_disp();
	et024006_PrintString("Welcome", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
	et024006_PrintString("No keys pressed yet", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);

	while (1){
		switch (state){

			case 0://Do nothing
				//---
				if(usart_message_rx_complete) leds(8);

				if(dbg){
					print_dbg("STATE 0 \r\n");
					CLR_disp();
					et024006_PrintString("STATE 0",(const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					et024006_PrintString(usart_message, (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
				}

			break;

			case 1:
				if(dbg) print_dbg("STATE TECLA UP \r\n");
				CLR_disp();
				et024006_PrintString("Getting Message ... ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				et024006_PrintString("Last key pressed: UP", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				usart_message_rx_complete=0;
				pdca_enable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_USART_RX); //Enable DMA and its RX intertupt
				pdca_enable(AVR32_PDCA_CHANNEL_USART_RX);
				state = 0;
				//In its handler, turn LED0 on
				//The message gets stored in usart_message
			break;

			case 2://Show the received message
				if(dbg) print_dbg("STATE TECLA DOWN \r\n");
				CLR_disp();
				et024006_PrintString("Received Message:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				et024006_PrintString(usart_message, (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
				et024006_PrintString("Last key pressed: DOWN", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				state = 0;
			break;

			case 3://Store message in SD
				if(dbg) print_dbg("STATE TECLA RIGHT \r\n");
				CLR_disp();
				et024006_PrintString("Last key pressed: RIGHT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				if(sd_mmc_spi_mem_check()){
					if(usart_message_rx_complete){
						usart_message_rx_complete = 0; //Do this routine only once
						Sector_Counter=(Sector_Counter % 5)+1;//Increase current sector
						sd_mmc_spi_write_open (Sector_Counter); //Write in a Sector
						sd_mmc_spi_write_sector_from_ram(&usart_message);
						sd_mmc_spi_write_close ();
						et024006_PrintString("Message successfully stored", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
						sector_counter_print[0] = Sector_Counter+'0';
						et024006_PrintString("Sector:", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
						et024006_PrintString(sector_counter_print, (const unsigned char *)&FONT8x8, 100, 50, WHITE, -1);
						et024006_PrintString("Message:", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
						et024006_PrintString(usart_message, (const unsigned char *)&FONT8x8, 30, 90, WHITE, -1);
					} else {
						CLR_disp();
						et024006_PrintString("Last key pressed: RIGHT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
						et024006_PrintString("Message has not been received", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					}//If empty message
				} else {
					CLR_disp();
					et024006_PrintString("Last key pressed: RIGHT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
					et024006_PrintString("No SD card detected: ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					et024006_PrintString("Will reinit the system ", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
					et024006_PrintString("Waiting for SD card ...", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
					state = 0;
					goto REINIT;//wait for SD to be reinserted Re-init everything
				}//If check mem
				state=0;
			break;

			case 4://Swhow last stored value and its sector
				if(dbg) print_dbg("STATE TECLA LEFT \r\n");

				CLR_disp();
				et024006_PrintString("Last key pressed: LEFT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				et024006_PrintString("Last written sector:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				sector_counter_print[0] = Sector_Counter+'0';
				et024006_PrintString(sector_counter_print, (const unsigned char *)&FONT8x8, 200, 30, WHITE, -1);
				et024006_PrintString("Message:", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);

				//Read SD
				pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_RX, &ram_buffer,512);
				pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_TX,(void *)&dummy_data,512); //send dummy
				end_of_transfer = false;
				if(sd_mmc_spi_read_open_PDCA (Sector_Counter)){
					spi_write(SD_MMC_SPI,0xFF); // Write a first dummy data to synchronize transfer
					pdca_enable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
					pdca_enable(AVR32_PDCA_CHANNEL_SPI_RX);
					pdca_enable(AVR32_PDCA_CHANNEL_SPI_TX);
					while(!end_of_transfer);
					for( i = 0; i < 25; i++){ //First 50 chars
						usart_message_print[i] = (U8)(*(ram_buffer + i));
					}//For
					et024006_PrintString(usart_message_print, (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
				}else{
					CLR_disp();
					et024006_PrintString("Last key pressed: LEFT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
					et024006_PrintString("No SD card detected: ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					et024006_PrintString("Will reinit the system ", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
					et024006_PrintString("Waiting for SD card ...", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
					state = 0;
					goto REINIT;//wait for SD to be reinserted Re-init everything
				}//IF

				state = 0;
			break;

			case 5://Show stored messages
				print_dbg("STATE TECLA CENTER \r\n");
			  CLR_disp();
				et024006_PrintString("Last key pressed: CENTER", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				et024006_PrintString("The SD card data is shown below:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				et024006_PrintString("Sector 1:", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
				et024006_PrintString("Sector 2:", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
				et024006_PrintString("Sector 3:", (const unsigned char *)&FONT8x8, 30, 90, WHITE, -1);
				et024006_PrintString("Sector 4:", (const unsigned char *)&FONT8x8, 30, 110, WHITE, -1);
				et024006_PrintString("Sector 5:", (const unsigned char *)&FONT8x8, 30, 130, WHITE, -1);

				//Read SD
				for(j = 1; j <= 5; j++){ //5 Sectors
					pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_RX, &ram_buffer,512);
					pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_TX,(void *)&dummy_data,512); //send dummy
					end_of_transfer = false;
					if(sd_mmc_spi_read_open_PDCA (j)){
						spi_write(SD_MMC_SPI,0xFF); // Write a first dummy data to synchronize transfer
						pdca_enable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
						pdca_enable(AVR32_PDCA_CHANNEL_SPI_RX);
						pdca_enable(AVR32_PDCA_CHANNEL_SPI_TX);
						while(!end_of_transfer);
						for( i = 0; i < 25; i++){ //First 50 chars
							usart_message_print[i] = (U8)(*(ram_buffer + i));
						}//For
						et024006_PrintString(usart_message_print, (const unsigned char *)&FONT8x8, 100, 30+20*j, WHITE, -1);
					}else{
						CLR_disp();
						et024006_PrintString("Last key pressed: CENTER", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
						et024006_PrintString("No SD card detected: ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
						et024006_PrintString("Will reinit the system ", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
						et024006_PrintString("Waiting for SD card ...", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
						state = 0;
						goto REINIT;//wait for SD to be reinserted Re-init everything
					}//IF
				}//For

				state = 0;
			break;

		}//Switch
	}//While
}//Main

static void tft_bl_init(void){
  pwm_opt_t opt = {.diva = 0,.divb = 0,.prea = 0,.preb = 0};
  pwm_init(&opt);
  pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
  pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
  pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
  pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;
  pwm_channel_init(6, &pwm_channel6);
  pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);
}//tft_bl_init

void CLR_disp(void){
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
	et024006_PutPixmap(avr32_logo, 320, 0, 0, 0, 0, 320, 240);
}//CLR_disp


static void pdca_int_handler(void){
	Disable_global_interrupt();
	pdca_disable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
	sd_mmc_spi_read_close_PDCA();
	delay_us(10);
	pdca_disable(AVR32_PDCA_CHANNEL_SPI_TX);
	pdca_disable(AVR32_PDCA_CHANNEL_SPI_RX);
	Enable_global_interrupt();
	end_of_transfer = true;

}//pdca_int_handler

static void pdca_int_handler_usart(void){

	//et024006_PrintString("INTERRUPT", (const unsigned char *)&FONT8x8, 100, 100, WHITE, -1);
	Disable_global_interrupt();
	pdca_disable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_USART_RX);
	//sd_mmc_spi_read_close_PDCA();
	//delay_us(10);
	pdca_disable(AVR32_PDCA_CHANNEL_USART_RX);
	Enable_global_interrupt();
	usart_message_rx_complete=1;
	//leds(8); //Turn LED0 on
}//pdca_int_handler_usart

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
		.addr = (void *) &dummy_data,              // memory address.
		.size = 512,                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = AVR32_PDCA_CHANNEL_USED_TX,        // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
	};//pdca_options_SPI_TX

	pdca_channel_options_t pdca_options_USART_RX ={ //RX
		.addr = &usart_message[0],            // memory address.
		.size = 5,                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = AVR32_PDCA_CHANNEL_USED_RX_USART,        // select peripheral ID - data are on reception from USART0
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
	};//pdca_options_SPI_TX

	pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_TX, &pdca_options_SPI_TX);
	pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_RX, &pdca_options_SPI_RX);
	pdca_init_channel(AVR32_PDCA_CHANNEL_USART_RX, &pdca_options_USART_RX);
	INTC_register_interrupt(&pdca_int_handler, AVR32_PDCA_IRQ_1, AVR32_INTC_INT1);  // pdca_channel_spi1_RX = 0
	INTC_register_interrupt(&pdca_int_handler_usart, AVR32_PDCA_IRQ_0, AVR32_INTC_INT1);  // pdca_channel_usart_RX = 0

} //local_pdca_init

void init_button_interrupt(void){//inicializar interrupciones de botones
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&btn_interrupt_routine, 70, 3);
	INTC_register_interrupt(&btn_interrupt_routine, 71, 3);
	uint16_t button_ref [] = {BTN_UP,BTN_DOWN,BTN_RIGHT,BTN_LEFT,BTN_CENTER};
	for(uint8_t i=0; i<5; i++){
		gpio_enable_gpio_pin(button_ref[i]);
		gpio_enable_pin_pull_up(button_ref[i]);
		gpio_enable_pin_interrupt(button_ref[i],GPIO_FALLING_EDGE);
	}//For
	Enable_global_interrupt();
}//init_button_interrupt

void leds(uint8_t value){
	if ((value & 0b1000)>>3)gpio_clr_gpio_pin(LED0); else gpio_set_gpio_pin(LED0);
	if ((value & 0b0100)>>2)gpio_clr_gpio_pin(LED1); else gpio_set_gpio_pin(LED1);
	if ((value & 0b0010)>>1)gpio_clr_gpio_pin(LED2); else gpio_set_gpio_pin(LED2);
	if (value & 0b0001)gpio_clr_gpio_pin(LED3); else gpio_set_gpio_pin(LED3);
}//Fin Fn

void btn_interrupt_routine (void){
	CLR_disp();
	if (gpio_get_pin_interrupt_flag(BTN_UP)) {
		btn_pressed=UP;
		state = 1;
		gpio_clear_pin_interrupt_flag(BTN_UP);
	}
	if (gpio_get_pin_interrupt_flag(BTN_DOWN)){
		btn_pressed=DOWN;
		state=2;
		gpio_clear_pin_interrupt_flag(BTN_DOWN);
	}
	if (gpio_get_pin_interrupt_flag(BTN_RIGHT)){
		btn_pressed=RIGHT;
		state=3;
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_LEFT)){
		btn_pressed=LEFT;
		state=4;
		gpio_clear_pin_interrupt_flag(BTN_LEFT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		btn_pressed=CENTER;
		state=5;
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
	}
	gpio_get_pin_interrupt_flag(BTN_CENTER);
} //Fin Botones
