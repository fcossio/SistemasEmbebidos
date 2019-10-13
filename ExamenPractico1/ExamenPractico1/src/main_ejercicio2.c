/*
-Si se oprimen las teclas UP/DOWN
hacer parpadear los LEDS del centro (LED 1 y 2). Se deberán utilizar
dos frecuencias que puedan ser visualmente distinguibles. Cuando se oprima UP parpadear los LEDS a una
frecuencia y cuando se oprima DOWN a otra. Se debe detener el parpadeo con la tecla CENTER.

-Si  se  oprimen  las  teclasLEFT/RIGHT
realizar  el  mismo  procedimiento  para  los  LEDS  de  los  extremos (LED0 y LED3)
*/

#include <asf.h>

#define BTN_UP		AVR32_PIN_PB22
#define BTN_DOWN	AVR32_PIN_PB23
#define BTN_RIGHT	AVR32_PIN_PB24
#define BTN_LEFT	AVR32_PIN_PB25
#define BTN_CENTER	AVR32_PIN_PB26

#define LED0   AVR32_PIN_PB27
#define LED1   AVR32_PIN_PB28
#define LED2   AVR32_PIN_PA05
#define LED3   AVR32_PIN_PA06

enum btn{NONE, UP, DOWN, LEFT, RIGHT, CENTER};
enum btn btn_pressed = NONE;
uint8_t state = 0;

__attribute__ ((__interrupt__));
void Botones (void);
void Inicializa_PLL(uint8_t mul);
void Prender_Leds(uint8_t value);
void parpadear(uint8_t delay);
int main (void){

	board_init();

	pm_switch_to_osc0(&AVR32_PM, 12000000, 0);
	delay_init(12000000);
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&Botones, 33, 3); //IRQ 33 para EIC1
	Enable_global_interrupt();  //Habilita interrupciones globales
	eic_options_t eic_options;
	// Enable edge-triggered interrupt.
	eic_options.eic_mode  = EIC_MODE_EDGE_TRIGGERED;
	eic_options.eic_edge  = EIC_EDGE_RISING_EDGE;
	eic_options.eic_async = EIC_SYNCH_MODE;
	eic_options.eic_line  = 1;
	eic_init(&AVR32_EIC, &eic_options, 1);
	eic_enable_line(&AVR32_EIC, 1);
	eic_enable_interrupt_line(&AVR32_EIC, 1);
	Enable_global_interrupt();
	gpio_enable_module_pin(22,1);



	//LEDS apagados
	gpio_set_gpio_pin(LED0);
	gpio_set_gpio_pin(LED1);
	gpio_set_gpio_pin(LED2);
	gpio_set_gpio_pin(LED3);

	while (true) {
		switch (state) {
			case 0: //UP
			Prender_Leds(0b1000);
				// Inicializa_PLL(3);
				// gpio_clr_gpio_pin(LED1);
				// gpio_clr_gpio_pin(LED2);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				// gpio_set_gpio_pin(LED1);
				// gpio_set_gpio_pin(LED2);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				break;
			case 1: //DOWN
			Prender_Leds(0b0100);
				// Inicializa_PLL(5);
				// gpio_clr_gpio_pin(LED1);
				// gpio_clr_gpio_pin(LED2);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				// gpio_set_gpio_pin(LED1);
				// gpio_set_gpio_pin(LED2);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				break;
			case 2: //RIGHT
			Prender_Leds(0b0010);
				// Inicializa_PLL(3);
				// gpio_clr_gpio_pin(LED0);
				// gpio_clr_gpio_pin(LED3);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				// gpio_set_gpio_pin(LED0);
				// gpio_set_gpio_pin(LED3);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				break;
			case 3: //LEFT
			Prender_Leds(0b0011);
				// Inicializa_PLL(5);
				// gpio_clr_gpio_pin(LED0);
				// gpio_clr_gpio_pin(LED3);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				// gpio_set_gpio_pin(LED0);
				// gpio_set_gpio_pin(LED3);
				// for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				break;
			case 4: //CENTER
			Prender_Leds(0b0111);
				//LEDS apagados
				// gpio_set_gpio_pin(LED0);
				// gpio_set_gpio_pin(LED1);
				// gpio_set_gpio_pin(LED2);
				// gpio_set_gpio_pin(LED3);
				break;
			// default:
			// 	//LEDS apagados
			// 	// gpio_set_gpio_pin(LED0);
			// 	// gpio_set_gpio_pin(LED1);
			// 	// gpio_set_gpio_pin(LED2);
			// 	// gpio_set_gpio_pin(LED3);
			// break;
		} //Switch
		btn_pressed=NONE;
	} //While
} //Main

void Inicializa_PLL(uint8_t mul){
	pm_switch_to_osc0(&AVR32_PM, 12000000,3);
	pm_pll_disable(&AVR32_PM,0);
	pm_pll_setup(&AVR32_PM,0, mul, 1,0,16); //pll0, mul variable, div = 1, 16 lockcount
	pm_pll_set_option(&AVR32_PM,0,1,0,0);  //pll0, 80-180, no divide/2, start normal
	pm_pll_enable(&AVR32_PM,0);
	pm_wait_for_pll0_locked(&AVR32_PM);
	pm_switch_to_clock(&AVR32_PM,2);//PLL como MC
	flashc_set_wait_state(1);
}//Inicializa_PLL


void Prender_Leds(uint8_t value){
	if ( (value & 0b1000)>>3 ) gpio_clr_gpio_pin(LED0); else gpio_set_gpio_pin(LED0);
	if ( (value & 0b0100)>>2 ) gpio_clr_gpio_pin(LED1); else gpio_set_gpio_pin(LED1);
	if ( (value & 0b0010)>>1 ) gpio_clr_gpio_pin(LED2); else gpio_set_gpio_pin(LED2);
	if ( value & 0b0001 ) 		 gpio_clr_gpio_pin(LED3); else gpio_set_gpio_pin(LED3);
}//Fin Fn

void Botones (void){
	//Checar cual tecla fue presionada
	if (gpio_get_pin_value(BTN_UP)){btn_pressed = UP; state = 0;}
	if (gpio_get_pin_value(BTN_DOWN)){btn_pressed = DOWN; state = 1;}
	if (gpio_get_pin_value(BTN_RIGHT)){btn_pressed = RIGHT; state = 2;}
	if (gpio_get_pin_value(BTN_LEFT)){btn_pressed = LEFT; state = 3;}
	if (gpio_get_pin_value(BTN_CENTER)){btn_pressed = CENTER; state = 4;}

	eic_clear_interrupt_line(&AVR32_EIC, 1); //Limpiar bandera de EIC
	gpio_clear_pin_interrupt_flag(22); //Limpiar bandera de INTC
} //Botones

//PARA FOSC0 = 12 MHz
//mul=3 fpll=96MHz
//mul=4 fpll=120MHz
//mul=5 fpll=144MHz
//mul=6 fpll=168MHz
