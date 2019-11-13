
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
#include "et024006dhu.h"
#include "delay.h"
#include "pwm.h"

#define RGB(r,g,b) r<<11|g<<5|b

//SPI SDCARD
const char dummy_data[] =
#include "dummy.h"
;//No borrar, dummy_data

//Keys and LEDs
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
#define AVR32_PDCA_CHANNEL_SPI_RX 0 // In the example we will use the pdca channel 0.
#define AVR32_PDCA_CHANNEL_SPI_TX 1 // In the example we will use the pdca channel 1.

//Logic
enum btn{NONE, UP, DOWN, LEFT, RIGHT, CENTER};
enum btn btn_pressed = NONE;
volatile int state = 0;

//Variables
avr32_pwm_channel_t pwm_channel6 = {.cdty = 0,.cprd = 100};
volatile avr32_pdca_channel_t* pdca_channelrx ;
volatile avr32_pdca_channel_t* pdca_channeltx ;
volatile bool end_of_transfer;
volatile char ram_buffer[1000];
volatile char usart_message[51]; //Read from USART (UP Key)

//Functions
static void tft_bl_init(void);
void CLR_disp(void);
void wait();
__attribute__((__interrupt__)) static void pdca_int_handler(void);
__attribute__ ((__interrupt__)) void btn_interrupt_routine (void);
static void sd_mmc_resources_init(void);
void local_pdca_init(void);
void leds(uint8_t value);
void init_button_interrupt(void);

int main(void){

	int i, j; //j for sectors, i for bytes

	//PM
	pm_switch_to_osc0(&AVR32_PM, PBA_HZ, 3);

	//Button interrupt
	init_button_interrupt();

	//SDCARD
	init_dbg_rs232(PBA_HZ);
	print_dbg("\r\nInit SD/MMC Driver");
	print_dbg("\r\nInsert SD/MMC...");
	sd_mmc_resources_init();
	while (!sd_mmc_spi_mem_check());
	print_dbg("\r\nCard detected!");
	sd_mmc_spi_get_capacity();
	print_dbg("Capacity = ");
	print_dbg_ulong(capacity >> 20);
	print_dbg(" MBytes");

	Enable_global_interrupt();
	local_pdca_init();

	for(j = 1; j <= 5; j++){ //5 Sectores

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
			while(!end_of_transfer);

			for( i = 0; i < 20; i++){ //20 primeros bytes
				print_dbg_char_hex( (U8)(*(ram_buffer + i)));
			}//For
		}else{
			print_dbg("\r\n! Unable to open memory \r\n");
		}//IF
	}//For

	print_dbg("\r\nEnd of the example.\r\n");

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

	while (1){
		switch (state){
			case 0://Do nothing
				et024006_PrintString("Estado 0", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
			break;
			case 1:/*
				if (!usart_dma_started){
					//inicializar DMA
				}else{
					//Transfer complete
					if(end_of_transfer){
						leds(0b1000);
						delay_ms(100);
					}//If
				}//IF */
				leds(1);//Change to LED0
			break;
			case 2://Desplegar mensaje de memoria en display
				//Don at handler
				leds(2);
			break;
			case 3://Guardar mensaje en la SD
				leds(3);
			break;
			case 4://Leer ultimo mensaje guardado de la SD y su sector y mostrarlo
				leds(4);
			break;
			case 5://Mostrar todos los mensajes guardados
				leds(5);
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
}//CLR_disp

void wait(){
	volatile int i;
	for(i = 0 ; i < 5000; i++);
}//Wait

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
		usart_read();
		gpio_clear_pin_interrupt_flag(BTN_UP);
	}
	if (gpio_get_pin_interrupt_flag(BTN_DOWN)){
		btn_pressed=DOWN;
		state=2;
		//et024006_PrintString("Estado 2", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
		et024006_PrintString("Mensaje recibido:", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
		et024006_PrintString(usart_message, (const unsigned char *)&FONT8x8, 30, 50, WHITE, -1); //test
		gpio_clear_pin_interrupt_flag(BTN_DOWN);
	}
	if (gpio_get_pin_interrupt_flag(BTN_RIGHT)){
		btn_pressed=RIGHT;
		state=3;
		et024006_PrintString("Estado 3", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_LEFT)){
		btn_pressed=LEFT;
		state=4;
		et024006_PrintString("Estado 4", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
		gpio_clear_pin_interrupt_flag(BTN_LEFT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
		btn_pressed=CENTER;
		state=5;
		et024006_PrintString("Estado 5", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
	}
	gpio_get_pin_interrupt_flag(BTN_CENTER);
} //Fin Botones

void usart_read(void){
	//We should read USART
	usart_message = {"usart message"};
	state=1;
	et024006_PrintString("Estado 1", (const unsigned char *)&FONT8x8, 30, 30, WHITE, -1);
}//usart_read
