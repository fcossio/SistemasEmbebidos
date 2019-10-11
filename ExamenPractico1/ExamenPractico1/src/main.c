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

uint8_t tecla_oprimida = 5;

__attribute__ ((__interrupt__));
void Botones (void);
void Inicializa_PLL(uint8_t mul);

int main (void){

	Inicializa_PLL(3);
	delay_init(96000000);//Calcular Hz

	board_init();

	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&Botones, 33, 3); //IRQ 33 para EIC1

	gpio_enable_gpio_pin(BTN_UP);
	gpio_enable_gpio_pin(BTN_DOWN);
	gpio_enable_gpio_pin(BTN_RIGHT);
	gpio_enable_gpio_pin(BTN_LEFT);
	gpio_enable_gpio_pin(BTN_CENTER);

	gpio_enable_pin_pull_up(BTN_UP);
	gpio_enable_pin_pull_up(BTN_DOWN);
	gpio_enable_pin_pull_up(BTN_RIGHT);
	gpio_enable_pin_pull_up(BTN_LEFT);
	gpio_enable_pin_pull_up(BTN_CENTER);

	gpio_enable_gpio_pin(22);
	gpio_enable_pin_pull_up(22);
	gpio_enable_pin_interrupt(22,1); //GPIO22 en Falling

	eic_options_t eic_options;

	eic_options.eic_mode  = EIC_MODE_EDGE_TRIGGERED;
	eic_options.eic_edge  = EIC_EDGE_FALLING_EDGE;
	eic_options.eic_async = EIC_SYNCH_MODE;
	eic_options.eic_line  = 1; //EIC1
	eic_init(&AVR32_EIC, &eic_options, 1); //EIC1

	eic_enable_line(&AVR32_EIC, 1); //Alternativa 1
	//eic_enable_lines(&AVR32_EIC,(1<<eic_options.eic_line)); //Alternativa 2
	eic_enable_interrupt_line(&AVR32_EIC, 1); //Alternativa 1
	//eic_enable_interrupt_lines(&AVR32_EIC,(1<<eic_options.eic_line)); //Alternativa 2

	Enable_global_interrupt();

	gpio_enable_module_pin(22, 1); //GPIO 22, Funcion B (EIC1)

	//LEDS apagados
	gpio_set_gpio_pin(LED0);
	gpio_set_gpio_pin(LED1);
	gpio_set_gpio_pin(LED2);
	gpio_set_gpio_pin(LED3);

	while (true) {
		switch (tecla_oprimida) {
			case 0: //UP
				Inicializa_PLL(3);
				gpio_clr_gpio_pin(LED1);
				gpio_clr_gpio_pin(LED2);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				gpio_set_gpio_pin(LED1);
				gpio_set_gpio_pin(LED2);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
			break;
			case 1: //DOWN
				Inicializa_PLL(5);
				gpio_clr_gpio_pin(LED1);
				gpio_clr_gpio_pin(LED2);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				gpio_set_gpio_pin(LED1);
				gpio_set_gpio_pin(LED2);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
			break;
			case 2: //RIGHT
				Inicializa_PLL(3);
				gpio_clr_gpio_pin(LED0);
				gpio_clr_gpio_pin(LED3);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				gpio_set_gpio_pin(LED0);
				gpio_set_gpio_pin(LED3);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
			break;
			case 3: //LEFT
				Inicializa_PLL(5);
				gpio_clr_gpio_pin(LED0);
				gpio_clr_gpio_pin(LED3);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
				gpio_set_gpio_pin(LED0);
				gpio_set_gpio_pin(LED3);
				for (U32 i = 0; i<100000; i++){int d = 0; d++;}
			break;
			case 4: //CENTER
				//LEDS apagados
				gpio_set_gpio_pin(LED0);
				gpio_set_gpio_pin(LED1);
				gpio_set_gpio_pin(LED2);
				gpio_set_gpio_pin(LED3);
			break;
			default:
				//LEDS apagados
				gpio_set_gpio_pin(LED0);
				gpio_set_gpio_pin(LED1);
				gpio_set_gpio_pin(LED2);
				gpio_set_gpio_pin(LED3);
			break;
		} //Switch
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

void Botones (void){

	//Checar cual tecla fue presionada
	if (!gpio_get_pin_value(BTN_UP)){tecla_oprimida = 0;}
	if (!gpio_get_pin_value(BTN_DOWN)){tecla_oprimida = 1;}
	if (!gpio_get_pin_value(BTN_RIGHT)){tecla_oprimida = 2;}
	if (!gpio_get_pin_value(BTN_LEFT)){tecla_oprimida = 3;}
	if (!gpio_get_pin_value(BTN_CENTER)){tecla_oprimida = 4;}

	eic_clear_interrupt_line(&AVR32_EIC, 1); //Limpiar bandera de EIC
	gpio_clear_pin_interrupt_flag(22); //Limpiar bandera de INTC

} //Botones

//PARA FOSC0 = 12 MHz
//mul=3 fpll=96MHz
//mul=4 fpll=120MHz
//mul=5 fpll=144MHz
//mul=6 fpll=168MHz
