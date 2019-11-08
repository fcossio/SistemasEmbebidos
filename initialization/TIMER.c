#include "intc.h"
#include "pwm.h"
#include "tc.h"
#include "board.h"
#include "compiler.h"
#include "gpio.h"

static void init_tc_input(volatile avr32_tc_t *tc, unsigned int channel);
static void init_tc_output(volatile avr32_tc_t *tc, unsigned int channel);
int ra_input = 0, ra_output = 0;
static const gpio_map_t TC_GPIO_MAP =
{
  {106, 2}, //GPIO 106, TIOA0, FN especial C, 2
  {86, 2} //GPIO 86, FN especial C, 2
};
gpio_enable_module(TC_GPIO_MAP, sizeof(TC_GPIO_MAP) / sizeof(TC_GPIO_MAP[0]));//Activar Fn especiales para TIOA0 y TIOA2
gpio_enable_gpio_pin(BTN_RIGHT);

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
