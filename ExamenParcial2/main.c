//Sistemas Embebidos - Examen parcial 2
//Fernando Cossio Ramirez

#include <avr32/io.h>
#include "compiler.h"

#include <pm.h>
// From module: GPIO - General-Purpose Input/Output
#include <gpio.h>
// From module: Generic board support
#include <board.h>
// From module: INTC - Interrupt Controller
#include <intc.h>
// From module: Interrupt management - UC3 implementation
#include <interrupt.h>

#include <tc.h>

#include <spi.h>

#define PBA_HZ          FOSC0
#define BTN_UP   AVR32_PIN_PB22
#define BTN_DOWN AVR32_PIN_PB23
#define BTN_RIGHT AVR32_PIN_PB24
#define BTN_LEFT AVR32_PIN_PB25
#define BTN_CENTER AVR32_PIN_PB26
#define LED0   AVR32_PIN_PB27
#define LED1   AVR32_PIN_PB28
#define LED2   AVR32_PIN_PA05
#define LED3   AVR32_PIN_PA06

enum btn{NONE, UP, DOWN, LEFT, RIGHT, CENTER};
enum btn btn_pressed = NONE;
uint8_t state = 0;
uint8_t humidity = 0;
uint8_t temperature = 0; //0:NONE, 1:LOW, 2:HIGH
uint8_t timer_configured = 0;
__attribute__ ((__interrupt__));
void btn_interrupt_routine (void);
void leds(uint8_t value);
static void init_tc_output(volatile avr32_tc_t *tc, unsigned int channel);

void state1(void);
void state2(void);
void state3(void);


int main(void){

  pm_switch_to_osc0(&AVR32_PM, 16000000, 6);
  board_init();

  Disable_global_interrupt();
  INTC_init_interrupts();
  INTC_register_interrupt(&Botones, 70, 3);
  INTC_register_interrupt(&Botones, 71, 3);

  uint16_t button_ref [] = {BTN_UP,BTN_DOWN,BTN_RIGHT,BTN_LEFT,BTN_CENTER};
  for(uint8_t i=0; i<5; i++){
    gpio_enable_gpio_pin(button_ref[i]);
    gpio_enable_pin_pull_up(button_ref[i]);
    gpio_enable
    _pin_interrupt(button_ref[i],GPIO_FALLING_EDGE);
  }
  Enable_global_interrupt();
  init_tc_output(&AVR32_TC, 2); //Canal 2 como waveform
  static const gpio_map_t TC_GPIO_MAP =
  {
    {86, 2} //GPIO 86, FN especial C, 2
  };
  gpio_enable_module(TC_GPIO_MAP, sizeof(TC_GPIO_MAP) / sizeof(TC_GPIO_MAP[0]));//Activar Fn especial para TIOA2

  /*SPI************************************************************************/
  // SPI options.
  //Mapa SPI
	static const gpio_map_t SPI_GPIO_MAP = {
		{AVR32_SPI0_SCK_0_0_PIN , AVR32_SPI0_SCK_0_0_FUNCTION },  // SCK: SPI Clock.
		{AVR32_SPI0_MISO_0_0_PIN, AVR32_SPI0_MISO_0_0_FUNCTION},  // MISO.
		{AVR32_SPI0_MOSI_0_0_PIN, AVR32_SPI0_MOSI_0_0_FUNCTION},  // MOSI.
		{AVR32_SPI0_NPCS_3_0_PIN, AVR32_SPI0_NPCS_3_0_FUNCTION}   // NPCS: Chip Select
	};//Pines y funciones SPI
	spi_options_t spiOptions = {
		.reg		      =  3,   //CHIP SELECT 1
		.baudrate     =  100000,//BAUDRATE: 100Kbps (sin modulacion)*
		.bits         =  8,//Número de bits a transmitir: 8
		.spck_delay   =  48,//Delay antes del SPCK (DLYBS = CLK*DLY = 12M*4u = 48)*
		.trans_delay  =  0,//Delay entre transiciones consecutivas (DLYBCT = 0, no se especifica un delay)*
		.stay_act     =  0,//Deselección de perífericos (CSAAT = 0, se desactiva en la ultima transferencia)*
		.spi_mode     =  3,//Modo (CPOL y NCPHA): 0*
		.modfdis      =  1,//Modo Fault Detection:  1 - Inhabilitado*
	};//Estructura SPI

	gpio_enable_module(SPI_GPIO_MAP,
	sizeof(SPI_GPIO_MAP) / sizeof(SPI_GPIO_MAP[0]));
	spi_initMaster(AVR32_SPI0_ADDRESS, &spiOptions);
	// Set SPI selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(AVR32_SPI0_ADDRESS, 0, 0, 0); //PS, PCS_decode, DLYBCS
	//PS = 0: fija
	//PCS = 0: sin decodificación
	//DLYBCS = DLY*CLK, no especificado

	spi_selectChip(AVR32_SPI0_ADDRESS, 3);

	// Enable SPI module.
	spi_enable(AVR32_SPI0_ADDRESS);
	spi_setupChipReg(AVR32_SPI0_ADDRESS,&spiOptions,PBA_HZ);
  /****************************************************************************/
  while (true)
  {
    switch (state) {
      case 0:
        //do nothing
        break;
      case 1: //programacion de humedad
        state1();
        break;
      case 2://programar temp
        state2();
        break;
      case 3: //contador arriba y abajo
        state3();
        break;
      case 4: //spi
        state4();
        break;
    } //Fin switch
  } //Fin While
}


void state1(void){//increment humidity
  if(btn_pressed == UP){
    humidity++;
    if (humidity > 4) humidity = 1;
    leds(0b1000>>(humidity-1));
  }
  else if(btn_pressed == CENTER){
    if(temperature)
      state = 3;
    else
      state = 0;
    timer_configured = 0;
  }
  else if (btn_pressed == RIGHT || btn_pressed == LEFT || btn_pressed == DOWN){
    humidity = 0; //invalidar seleccion
  }
  btn_pressed = NONE;
}
void state2(void){
  if(btn_pressed == LEFT){
    temperature = 1;
  }
  else if(btn_pressed == RIGHT){
    temperature = 2;
  }
  else if(btn_pressed == CENTER){
    if(humidity)
      state = 3; //pasar a generar PWM
    else
      state = 0;
    timer_configured = 0;
  }
  else if (btn_pressed == UP || btn_pressed == DOWN){
    temperature = 0;
  }
  btn_pressed = NONE;
}
void state3(void){
  if (!timer_configured){
    //fPBA=16MHz; fPBA/32=500kHz => TPBA=8e-6 seg
    //Tpwm= 30ms =>  rc = Tpwm/TPBA = 3750
    //20%(3750) = 750
    //40%(3750) = 1500
    //60%(3750) = 2250
    //80%(3750) = 3000
    //Tpwm= 70ms =>  rc = Tpwm/TPBA = 8750
    //20%(8750) = 1750
    //40%(8750) = 3500
    //60%(8750) = 5250
    //80%(8750) = 7000
    gpio_set_gpio_pin(86); //Para iniciar PWM en 1
    if(temperature == 1){
      //period = 30 ms
      tc_write_rc(&AVR32_TC0, 2, 3750);
      tc_write_ra(&AVR32_TC0, 2, 750*humidity);
    }
    else if(temperature ==2){
      //period = 70ms
      tc_write_rc(&AVR32_TC0, 2, 8750);
      tc_write_ra(&AVR32_TC0, 2, 1750*humidity);
    }
    tc_start(&AVR32_TC0,2);
    timer_configured = 1;
  }

}
void state4(void){
  if (humidity && temperature){
    //send SPI
    spi_write(AVR32_SPI0_ADDRESS, humidity);
    spi_write(AVR32_SPI0_ADDRESS, temperature-1);
    state = 0;
  }
}

void leds(uint8_t value){
	if ((value & 0b1000)>>3)gpio_clr_gpio_pin(LED0); else gpio_set_gpio_pin(LED0);
	if ((value & 0b0100)>>2)gpio_clr_gpio_pin(LED1); else gpio_set_gpio_pin(LED1);
	if ((value & 0b0010)>>1)gpio_clr_gpio_pin(LED2); else gpio_set_gpio_pin(LED2);
	if (value & 0b0001)gpio_clr_gpio_pin(LED3); else gpio_set_gpio_pin(LED3);
}//Fin Fn
void btn_interrupt_routine (void){
	if (gpio_get_pin_interrupt_flag(BTN_UP)) {
		btn_pressed=UP;
		state=1;
		gpio_clear_pin_interrupt_flag(BTN_UP);
	}
	if (gpio_get_pin_interrupt_flag(BTN_DOWN)){
		btn_pressed=DOWN;
		state=4;
		gpio_clear_pin_interrupt_flag(BTN_DOWN);
	}
	if (gpio_get_pin_interrupt_flag(BTN_RIGHT)){
		btn_pressed=RIGHT;
		state=2;
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_LEFT)){
		btn_pressed=LEFT;
		state=2;
		gpio_clear_pin_interrupt_flag(BTN_LEFT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
		btn_pressed=CENTER;
		}
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
	}
} //Fin Botones
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
		.tcclks   = 5, // fPBA/128, TC4, TC_CLOCK_SOURCE_TC4
	};

	// Initialize the timer/counter waveform.
	tc_init_waveform(tc, &waveform_opt);
}
