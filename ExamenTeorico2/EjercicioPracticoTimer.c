//Actividad Práctica Timer  14 octubre 2019

//Bernardo Urriza A01336299
//Antonio Corona A01337294
//Fernando Cossio A00759499

#include <avr32/io.h>
#include "compiler.h"
#include "power_clocks_lib.h"
#include "board.h"
#include "gpio.h"
#include "pwm.h"
#include "tc.h"
#include "pm.h"
#include "intc.h"

#define BTN_RIGHT AVR32_PIN_PB24
#define BTN_CENTER AVR32_PIN_PB26

//Funciones de usuario
__attribute__ ((__interrupt__))
void teclas(void);
static void init_tc_input(volatile avr32_tc_t *tc, unsigned int channel);
static void init_tc_output(volatile avr32_tc_t *tc, unsigned int channel);

//Variables globales
uint8_t state=0;
int ra_input = 0, ra_output = 0;

int main (void){

	// Configuracion de PM
	pm_switch_to_osc0(&AVR32_PM, 16000000, 3); //F pba= 16MHz; startup: 18ms
	//Inicializacion de canales de captura y PWM
	init_tc_input(&AVR32_TC, 0); //Canal 0 como captura
	init_tc_output(&AVR32_TC, 2); //Canal 2 como waveform
	//Inicializacion de GPIO por IRQ
	static const gpio_map_t TC_GPIO_MAP =
	{
		{106, 2}, //GPIO 106, TIOA0, FN especial C, 2
		{86, 2} //GPIO 86, FN especial C, 2
	};
	gpio_enable_module(TC_GPIO_MAP, sizeof(TC_GPIO_MAP) / sizeof(TC_GPIO_MAP[0]));//Activar Fn especiales para TIOA0 y TIOA2
	gpio_enable_gpio_pin(BTN_RIGHT);
	gpio_enable_gpio_pin(BTN_CENTER);

	//Interrupciones
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&teclas, 71, 3); //Hab interrupcion para teclas RIGHT Y CENTER,
	gpio_enable_pin_interrupt(BTN_RIGHT , GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(BTN_CENTER , GPIO_FALLING_EDGE);
	Enable_global_interrupt();

	//fPBA=16MHz; fPBA/32=500kHz => TPBA=2us
	//Tpwm= 30ms =>  rc = Tpwm/TPBA = 15,000
	tc_write_rc(&AVR32_TC0, 2, 15000); //Periodo 30ms en pwm.
	tc_write_ra(&AVR32_TC0, 2, 7500); //Valor por defecto de pwm al 50% Duty
	gpio_set_gpio_pin(86); //Para iniciar PWM en 1

	while (1) {
		switch(state){
			case 0: //Int tecla center
			// deshabilitar timers
			tc_stop(&AVR32_TC0,0);
			tc_stop(&AVR32_TC0,2);
			break;
			case 1: //Se presiona tecla Right Comienza a capturar
			tc_start(&AVR32_TC0,0); //Inicia Captura
			tc_start(&AVR32_TC0,2); //Inicia PWM, trigger por SW
			while(state == 1){ //evita que se repita el tc_start
				ra_input = tc_read_ra(tc, 0); //Inicia con este valor el PWM
				if (ra_input < 4500){ //30%(15000) = 4500
					//generar pwm con T=30ms y DC=20%
					//raPWM = rc * 0.2 = 3000
					ra_output = 3000;
					tc_write_ra(&AVR32_TC0, 2, ra_output);
					}else if(ra_input <= 10500){ //70%(15000) = 10500
					//generar pwm con T=30ms y DC=50%
					//raPWM = rc * 0.5 = 7500
					ra_output = 7500;
					tc_write_ra(&AVR32_TC0, 2, ra_output);
					}else{
					//generar pwm con T=30ms y DC=80%
					//raPWM = rc * 0.8 = 12000
					ra_output = 12000;
					tc_write_ra(&AVR32_TC0, 2, ra_output);
				}
			}
			break;
		}//Fin switch
	}//Fin While
} //Fin Main

//Handler
void teclas (void) {
	if (gpio_get_pin_interrupt_flag (BTN_CENTER)){
		state = 0;
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
	}
	if (gpio_get_pin_interrupt_flag (BTN_RIGHT)){
		state = 1;
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}
	gpio_get_pin_interrupt_flag (BTN_RIGHT);//Para que funcione en EVK1105
}//Fin Teclas

static void init_tc_input(volatile avr32_tc_t *tc, unsigned int channel){//Para captura, carga en RA el tiempo en alto
	// Options for capture mode.
	tc_capture_opt_t capture_opt =
	{
		.channel  = channel, //Canal

		.ldrb     = 0, //No hay carga en TC_SEL_NO_EDGE,
		.ldra     = 2, //Carga en Falling de la entrada TIOA TC_SEL_FALLING_EDGE,

		.cpctrg   = 0, //Compare con RC no detiene la captura TC_NO_TRIGGER_COMPARE_RC
		.abetrg   = 1, //Trigger por la misma TIOA TC_EXT_TRIG_SEL_TIOA
		.etrgedg  = 1, //Rising es trigger, TC_SEL_RISING_EDGE

		.ldbdis   = FALSE, //Se va a medir mas de un periodo
		.ldbstop  = FALSE, //Se va a medir mas de un periodo

		.burst    = 0, //Sin Burst, TC_BURST_NOT_GATED
		.clki     = 0, //Reloj no invertido, TC_CLOCK_RISING_EDGE
		.tcclks   = 3, // fPBA/32, TC4, TC_CLOCK_SOURCE_TC4
	};

	// Initialize the timer/counter capture.
	tc_init_capture(tc, &capture_opt);
}//init_tc_input


static void init_tc_output(volatile avr32_tc_t *tc, unsigned int channel){
	// Options for waveform generation.
	tc_waveform_opt_t waveform_opt =
	{
		.channel  = channel,                                       // Channel selection.

		.bswtrg   = 0, //TC_EVT_EFFECT_NOOP,           // Software trigger effect on TIOB.
		.beevt    = 0, //TC_EVT_EFFECT_NOOP,           // External event effect on TIOB.
		.bcpc     = 0, //TC_EVT_EFFECT_NOOP,           // RC compare effect on TIOB.
		.bcpb     = 0, //TC_EVT_EFFECT_NOOP,           // RB compare effect on TIOB.

		.aswtrg   = 0, //TC_EVT_EFFECT_NOOP, // Trigger no cambia la salida
		.aeevt    = 0, //TC_EVT_EFFECT_NOOP, // Trigger no cambia la salida
		.acpc     = 1, //TC_EVT_EFFECT_SET,               // RC compare effect on TIOA.
		.acpa     = 2, //TC_EVT_EFFECT_CLEAR,          // RA compare effect on TIOA.

		.wavsel   = 2, //Simple pendiente, RC determina Periodo, RA Duty
		.enetrg   = 0, //No hay trigger por evento externo FALSE,
		.eevt     = 0, //No hay trigger por evento externo TC_EXT_EVENT_SEL_TIOB_INPUT,
		.eevtedg  = 0, //No hay trigger por evento externo TC_SEL_NO_EDGE,
		.cpcdis   = FALSE, //Se va a generar mas de un perido
		.cpcstop  = FALSE, //Se va a generar mas de un perido

		.burst    = 0, //Sin Burst, TC_BURST_NOT_GATED
		.clki     = 0, //Reloj no invertido, TC_CLOCK_RISING_EDGE
		.tcclks   = 3, // fPBA/32, TC4, TC_CLOCK_SOURCE_TC4
	};

	// Initialize the timer/counter waveform.
	tc_init_waveform(tc, &waveform_opt);
}//init_tc_output
