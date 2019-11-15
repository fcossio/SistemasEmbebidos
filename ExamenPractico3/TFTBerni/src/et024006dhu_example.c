
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

//Interface
#define BTN_UP   AVR32_PIN_PB22
#define BTN_DOWN AVR32_PIN_PB23
#define BTN_RIGHT AVR32_PIN_PB24
#define BTN_LEFT AVR32_PIN_PB25
#define BTN_CENTER AVR32_PIN_PB26
#define LED0   AVR32_PIN_PB27
#define LED1   AVR32_PIN_PB28
#define LED2   AVR32_PIN_PA05
#define LED3   AVR32_PIN_PA06

//Serial Channels
#define USART_DMA_CHANNEL			0
#define AVR32_PDCA_CHANNEL_SPI_RX	1
#define AVR32_PDCA_CHANNEL_SPI_TX	2

//SPI SDCARD
const char dummy_data[] =
#include "dummy.h"
;//Dummy_data

//Variables:

//TFT Variables
avr32_pwm_channel_t pwm_channel6 = {.cdty = 0,.cprd = 100};
//SPI Variables
volatile bool spi_flag; //DMA SD flag
volatile char spi_message[1000]; //DMA SD buffer
volatile uint8_t current_sector = 0; //Current sector
//USART Variables
volatile short usart_flag=1;
volatile char usart_message[50];//={"Nothing"};
volatile char usart_char;
//Interface Variables
char current_sector_print[1]; //Converts current sector to string
char usart_message_print[50]; //Converts message to string
volatile int state = 0;

//Functions:

//TFT Functions
static void tft_init(void);
void tft_clear(void);
//SPI Functions
static void sd_init(void);
void spi_dma(void);
__attribute__((__interrupt__)) static void spi_dma_handler(void);
//USART Functions
void usart_init(void);
void usart_dma(void);
__attribute__ ((__interrupt__))static void usart_dma_handler(void);
//Interface Functions
void init_button_interrupt(void);
__attribute__((__interrupt__)) void button_handler (void);
void leds(uint8_t value);

int main(void){
	
	//Variable init
	state = 0;
	int i, j; //j:sectors i: bytes

	//PM
	pm_switch_to_osc0(&AVR32_PM, FOSC0, 3);

	//INTC
	init_button_interrupt();
	
	//Debug
	init_dbg_rs232(FOSC0);

	//SDCARD
	sd_init();
	REINIT: while (!sd_mmc_spi_mem_check());

	//DMA
	Enable_global_interrupt();
	spi_dma();

	//TFT
	et024006_Init( FOSC0, FOSC0 );
	tft_init();
	tft_clear();
	while(pwm_channel6.cdty < pwm_channel6.cprd){
		pwm_channel6.cdty++;
		pwm_channel6.cupd = pwm_channel6.cdty;
		pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
		delay_ms(1);
	}//PWM
	tft_clear();
	et024006_PrintString("Welcome", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
	et024006_PrintString("No keys pressed yet", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
	
	//USART
	usart_init();
	Enable_global_interrupt();
	
	for (short i=0;i<sizeof(usart_message);i++){
		usart_message[i]=0x000;
	}//Fill message array

	while (1){
		switch (state){

			case 0://Do nothing
			break;

			case 1://Wait for USART message
			tft_clear();
			leds(0);//Turn LEDs off
			et024006_PrintString("Waiting for message ... ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
			et024006_PrintString("Last key pressed: UP", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
			
			for (short i=0;i<sizeof(usart_message);i++){
				usart_message[i]=0x000;
			}//Fill message array
			
			usart_char = "a";
			for (short i = 0; i<50; i++){
				usart_dma();
				while (usart_flag == 0);
				if (usart_char == '\r'){
					break;//Exit character
				}//If break
				usart_message[i]=usart_char;
				usart_write_line(&AVR32_USART0,&usart_char);
			}//For 50 chars max
			
			leds(8);//Turn LED0 on
			tft_clear();
			et024006_PrintString("Message Received", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
			et024006_PrintString("Last key pressed: UP", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
			state = 0;
			break;

			case 2://Show the received message
			if(usart_message[0]==0x000){//No received value
				tft_clear();
				et024006_PrintString("Last key pressed: DOWN", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				et024006_PrintString("Message has not been received", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				}else{
				tft_clear();
				leds(0);//Turn LEDs off
				et024006_PrintString("Received Message:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				et024006_PrintString(usart_message, (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
				et024006_PrintString("Last key pressed: DOWN", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
			}//No received value
			state = 0;
			break;

			case 3://Store message in SD
			tft_clear();
			leds(0);//Turn LEDs off
			et024006_PrintString("Last key pressed: RIGHT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
			if(sd_mmc_spi_mem_check()){
				if(usart_message[0]!=0x000){//flag
					current_sector=(current_sector % 5)+1;//Increase current sector
					sd_mmc_spi_write_open (current_sector); //Write in a Sector
					sd_mmc_spi_write_sector_from_ram(&usart_message);
					sd_mmc_spi_write_close ();
					et024006_PrintString("Message successfully stored", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					current_sector_print[0] = current_sector+'0';
					et024006_PrintString("Sector:", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
					et024006_PrintString(current_sector_print, (const unsigned char *)&FONT8x8, 100, 50, WHITE, -1);
					et024006_PrintString("Message:", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
					et024006_PrintString(usart_message, (const unsigned char *)&FONT8x8, 30, 90, WHITE, -1);
					} else {
					tft_clear();
					et024006_PrintString("Last key pressed: RIGHT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
					et024006_PrintString("Message has not been received", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				}//No received value
				} else {
				tft_clear();
				et024006_PrintString("Last key pressed: RIGHT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				et024006_PrintString("No SD card detected: ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				et024006_PrintString("Will reinit the system ", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
				et024006_PrintString("Waiting for SD card ...", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
				state = 0;
				goto REINIT;//wait for SD to be reinserted Re-init everything
			}//If check mem
			state=0;
			break;

			case 4://Show last stored value and its sector
			if(usart_message[0]==0x000){//No received value
				tft_clear();
				et024006_PrintString("Last key pressed: LEFT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
				et024006_PrintString("Message has not been received", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
				}else{
				leds(0);//Turn LEDs off
				if (current_sector == 0){
					tft_clear();
					et024006_PrintString("Nothing saved yet", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					et024006_PrintString("Last key pressed: LEFT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
					}else{
					tft_clear();
					current_sector_print[0] = current_sector+'0';
					et024006_PrintString("Last written sector:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
					et024006_PrintString(current_sector_print, (const unsigned char *)&FONT8x8, 200, 30, WHITE, -1);
					et024006_PrintString("Message:", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
					et024006_PrintString("Last key pressed: LEFT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
					
					//Read SD
					pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_RX, &spi_message,512);
					pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_TX,(void *)&dummy_data,512); //send dummy
					spi_flag = false;
					if(sd_mmc_spi_read_open_PDCA (current_sector)){
						spi_write(SD_MMC_SPI,0xFF); // Write a first dummy data to synchronize transfer
						pdca_enable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
						pdca_enable(AVR32_PDCA_CHANNEL_SPI_RX);
						pdca_enable(AVR32_PDCA_CHANNEL_SPI_TX);
						while(!spi_flag);
						for( i = 0; i < 50; i++){ //First 50 chars
							usart_message_print[i] = (U8)(*(spi_message + i));
						}//For
						et024006_PrintString(usart_message_print, (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
						}else{
						tft_clear();
						et024006_PrintString("Last key pressed: LEFT", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
						et024006_PrintString("No SD card detected: ", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
						et024006_PrintString("Will reinit the system ", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
						et024006_PrintString("Waiting for SD card ...", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
						state = 0;
						goto REINIT;//wait for SD to be reinserted Re-init everything
					}//IF
				}//Sector 0
			}//No received value
			state = 0;
			break;

			case 5://Show stored messages
			tft_clear();
			leds(0);//Turn LEDs off
			et024006_PrintString("Last key pressed: CENTER", (const unsigned char *)&FONT8x8, 30, 200, WHITE, -1);
			et024006_PrintString("The SD card data is shown below:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
			et024006_PrintString("Sector 1:", (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1);
			et024006_PrintString("Sector 2:", (const unsigned char *)&FONT8x8, 30, 70, WHITE, -1);
			et024006_PrintString("Sector 3:", (const unsigned char *)&FONT8x8, 30, 90, WHITE, -1);
			et024006_PrintString("Sector 4:", (const unsigned char *)&FONT8x8, 30, 110, WHITE, -1);
			et024006_PrintString("Sector 5:", (const unsigned char *)&FONT8x8, 30, 130, WHITE, -1);

			//Read SD
			for(j = 1; j <= 5; j++){ //5 Sectors
				pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_RX, &spi_message,512);
				pdca_load_channel( AVR32_PDCA_CHANNEL_SPI_TX,(void *)&dummy_data,512); //send dummy
				spi_flag = false;
				if(sd_mmc_spi_read_open_PDCA (j)){
					spi_write(SD_MMC_SPI,0xFF); // Write a first dummy data to synchronize transfer
					pdca_enable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
					pdca_enable(AVR32_PDCA_CHANNEL_SPI_RX);
					pdca_enable(AVR32_PDCA_CHANNEL_SPI_TX);
					while(!spi_flag);
					for( i = 0; i < 25; i++){ //First 50 chars
						usart_message_print[i] = (U8)(*(spi_message + i));
					}//For
					et024006_PrintString(usart_message_print, (const unsigned char *)&FONT8x8, 100, 30+20*j, WHITE, -1);
					}else{
					tft_clear();
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

//TFT Functions

static void tft_init(void){
	pwm_opt_t opt = {.diva = 0,.divb = 0,.prea = 0,.preb = 0};
	pwm_init(&opt);
	pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
	pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
	pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
	pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;
	pwm_channel_init(6, &pwm_channel6);
	pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);
}//tft_init

void tft_clear(void){
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
	et024006_PutPixmap(avr32_logo, 320, 0, 0, 0, 0, 320, 240);
}//tft_clear

//SPI Functions

static void sd_init(void) {

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
	sd_mmc_spi_init(spiOptions, FOSC0);

}//sd_init

void spi_dma(void){

	pdca_channel_options_t pdca_options_SPI_RX ={ //RX
		.addr = spi_message,						  //Dummy data h
		.size = 512,                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = AVR32_PDCA_PID_SPI0_RX,        // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
	};//pdca_options_SPI_RX

	pdca_channel_options_t pdca_options_SPI_TX ={ //TX
		.addr = (void *) &dummy_data,              // memory address.
		.size = 512,                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = AVR32_PDCA_PID_SPI0_TX,        // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE  // select size of the transfer: 8,16,32 bits
	};//pdca_options_SPI_TX

	pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_TX, &pdca_options_SPI_TX);
	pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_RX, &pdca_options_SPI_RX);
	INTC_register_interrupt(&spi_dma_handler, AVR32_PDCA_IRQ_1, 3);  // FOR SPI (SD CARD)

} //spi_dma

static void spi_dma_handler(void){
	Disable_global_interrupt();
	pdca_disable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);
	sd_mmc_spi_read_close_PDCA();
	delay_us(10);
	pdca_disable(AVR32_PDCA_CHANNEL_SPI_TX);
	pdca_disable(AVR32_PDCA_CHANNEL_SPI_RX);
	Enable_global_interrupt();
	spi_flag = true;

}//spi_dma_handler

//USART Functions

void usart_init(void){
	static const gpio_map_t USART_GPIO_MAP = {
		{AVR32_USART0_RXD_0_0_PIN, AVR32_USART0_RXD_0_0_FUNCTION},
		{AVR32_USART0_TXD_0_0_PIN, AVR32_USART0_TXD_0_0_FUNCTION}
	};//USART Map
	static const usart_options_t USART_OPTIONS = {
		.baudrate = 57600,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = USART_1_STOPBIT,
		.channelmode = USART_NORMAL_CHMODE
	};//USART options
	gpio_enable_module(USART_GPIO_MAP,sizeof(USART_GPIO_MAP) /
	sizeof(USART_GPIO_MAP[0]));
	usart_init_rs232(&AVR32_USART0, &USART_OPTIONS, FOSC0);
}//conf_usart

void usart_dma(void) {
	const pdca_channel_options_t PDCA_OPTIONS = {
		.pid = AVR32_PDCA_PID_USART0_RX,
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE,
		.addr = &usart_char,
		.size =sizeof(usart_char),
		.r_addr = NULL,
		.r_size =0,
	};//PDCA_OPTIONS
	pdca_init_channel(USART_DMA_CHANNEL,&PDCA_OPTIONS);
	INTC_register_interrupt(&usart_dma_handler,96,1);
	pdca_enable_interrupt_transfer_complete(USART_DMA_CHANNEL);
	pdca_enable(USART_DMA_CHANNEL);
	usart_flag=0;
}//config_UsartDma

static void usart_dma_handler(void){
	pdca_disable_interrupt_transfer_complete(USART_DMA_CHANNEL);
	pdca_disable(USART_DMA_CHANNEL);
	usart_flag =1;
}//usart_dma_handler

//Interface Functions

void init_button_interrupt(void){//inicializar interrupciones de botones
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&button_handler, 70, 3);
	INTC_register_interrupt(&button_handler, 71, 3);
	uint16_t button_ref [] = {BTN_UP,BTN_DOWN,BTN_RIGHT,BTN_LEFT,BTN_CENTER};
	for(uint8_t i=0; i<5; i++){
		gpio_enable_gpio_pin(button_ref[i]);
		gpio_enable_pin_pull_up(button_ref[i]);
		gpio_enable_pin_interrupt(button_ref[i],GPIO_FALLING_EDGE);
	}//For
	Enable_global_interrupt();
}//init_button_interrupt

void button_handler(void){
	tft_clear();
	if (gpio_get_pin_interrupt_flag(BTN_UP)) {
		state = 1;
		gpio_clear_pin_interrupt_flag(BTN_UP);
	}//UP
	if (gpio_get_pin_interrupt_flag(BTN_DOWN)){
		state=2;
		gpio_clear_pin_interrupt_flag(BTN_DOWN);
	}//DOWN
	if (gpio_get_pin_interrupt_flag(BTN_RIGHT)){
		state=3;
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}//RIGHT
	if (gpio_get_pin_interrupt_flag(BTN_LEFT)){
		state=4;
		gpio_clear_pin_interrupt_flag(BTN_LEFT);
	}//LEFT
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		state=5;
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
	}//CENTER
	gpio_get_pin_interrupt_flag(BTN_CENTER);
}//button_handler

void leds(uint8_t value){
	if ((value & 0b1000)>>3)gpio_clr_gpio_pin(LED0); else gpio_set_gpio_pin(LED0);
	if ((value & 0b0100)>>2)gpio_clr_gpio_pin(LED1); else gpio_set_gpio_pin(LED1);
	if ((value & 0b0010)>>1)gpio_clr_gpio_pin(LED2); else gpio_set_gpio_pin(LED2);
	if (value & 0b0001)gpio_clr_gpio_pin(LED3); else gpio_set_gpio_pin(LED3);
}//Fin Fn



