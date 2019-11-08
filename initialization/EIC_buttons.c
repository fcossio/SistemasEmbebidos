// From module: EIC - External Interrupt Controller
#include <eic.h>
// From module: GPIO - General-Purpose Input/Output
#include <gpio.h>
// From module: Generic board support
#include <board.h>
// From module: INTC - Interrupt Controller
#include <intc.h>
// From module: Interrupt management - UC3 implementation
#include <interrupt.h>

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

__attribute__ ((__interrupt__));
void Botones (void);
int main(void){
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
  while(True){
    //codigo
  }
}

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
