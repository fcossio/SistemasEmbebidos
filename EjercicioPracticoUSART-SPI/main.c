//Actividad practica de SPI y USART

//Fernando Cossio
//Bernardo Urriza
//Antonio Rivera

#include <avr32/io.h>
#include "compiler.h"
#include "board.h"
#include "power_clocks_lib.h"
#include "gpio.h"
#include "usart.h"
#include "spi.h"
#include "intc.h"
#include "pm.h"
#include "tc.h"

#define FOSC0           12000000
#define OSC0_STARTUP    3
#define PBA_HZ          FOSC0

//Rutinas de interrupcion-------------------------
__attribute__ ((__interrupt__)) void teclas(void);
__attribute__ ((__interrupt__)) void timer(void);

volatile static int state=0;//maquina de estados
volatile static char data[1000];//1000 datos

int main(void){
  /*PM ************************************************************************/
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
  /****************************************************************************/

  /*TIMER *********************************************************************/
	//Estructura Timer
	static const tc_waveform_opt_t WAVEFORM_OPT = {
		.channel  = 1,                        // Channel selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,                // RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOA: toggle.
		.acpa     = TC_EVT_EFFECT_NOOP,                  // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
		.enetrg   = 0,                                 // External event trigger enable.
		.eevt     = 0,                                 // External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,                    // External event edge selection.
		.cpcdis   = 0,                                 // Counter disable when RC compare.
		.cpcstop  = 0,                                 // Counter clock stopped with RC compare.
		.burst    = 0,                                 // Burst signal selection.
		.clki     = 0,                                 // Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC3         // Internal source clock 3, connected to fPBA / 8.
	};//Config
  //Interrupciones de timer
	static const tc_interrupt_t TC_INTERRUPT = {
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1,   // Habilitar interrupción por comparación con RC
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};//Interrupcion timer 1!
  //Se activa hasta que se empieza a contar
  /****************************************************************************/

  /*ISR ***********************************************************************/
  //Habilitar interrupciones
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&teclas, 70,0);  //por formula sale el 70
	INTC_register_interrupt(&timer, AVR32_TC_IRQ1, 0); //Timer 1,; nivel = 0
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_UP,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_DOWN,GPIO_RISING_EDGE);
	Enable_global_interrupt();

  /****************************************************************************/

  /*USART**********************************************************************/
  //Canal 1
  static const gpio_map_t USART_GPIO_MAP =
  {
    {AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION},
    {AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}
  };
  // USART options
	static const usart_options_t USART_OPTIONS =
	{
		.baudrate     = 9600,
		.charlength   = 16,
		.paritytype   = USART_EVEN_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};
	// Assign GPIO to USART.
	gpio_enable_module(USART_GPIO_MAP,
	sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));
	//Inicializar USART
	usart_init_rs232(&AVR32_USART1, &USART_OPTIONS, FOSC0);
  /****************************************************************************/

	/*SPI************************************************************************/
  // SPI options.
  //Mapa SPI
	static const gpio_map_t SPI_GPIO_MAP = {
		{AVR32_SPI0_SCK_0_0_PIN , AVR32_SPI0_SCK_0_0_FUNCTION },  // SCK: SPI Clock.
		{AVR32_SPI0_MISO_0_0_PIN, AVR32_SPI0_MISO_0_0_FUNCTION},  // MISO.
		{AVR32_SPI0_MOSI_0_0_PIN, AVR32_SPI0_MOSI_0_0_FUNCTION},  // MOSI.
		{AVR32_SPI0_NPCS_1_0_PIN, AVR32_SPI0_NPCS_1_0_FUNCTION}   // NPCS: Chip Select
	};//Pines y funciones SPI
	spi_options_t spiOptions = {
		.reg		      =  1,   //CHIP SELECT 1
		.baudrate     =  400000,//BAUDRATE: 400Kbps (sin modulacion)*
		.bits         =  16,//Número de bits a transmitir: 16*
		.spck_delay   =  48,//Delay antes del SPCK (DLYBS = CLK*DLY = 12M*4u = 48)*
		.trans_delay  =  0,//Delay entre transiciones consecutivas (DLYBCT = 0, no se especifica un delay)*
		.stay_act     =  0,//Deselección de perífericos (CSAAT = 0, se desactiva en la ultima transferencia)*
		.spi_mode     =  0,//Modo (CPOL y NCPHA): 0*
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

	spi_selectChip(AVR32_SPI0_ADDRESS, 1);

	// Enable SPI module.
	spi_enable(AVR32_SPI0_ADDRESS);
	spi_setupChipReg(AVR32_SPI0_ADDRESS,&spiOptions,PBA_HZ);
  /****************************************************************************/

  /*LOGICA*********************************************************************/
  int received_char = 0;

	while (true){
    switch (state){
      case 0:
        //Hacer nada :)
        break;
      case 1:
        //USART
        /*recibir una trama de 1000 datos guardarlos en un arreglo y prender un LED*/
        tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT); //config para timer 1
        tc_write_rc(&AVR32_TC, 1, 30000);//30000 - 20ms
        tc_configure_interrupts(&AVR32_TC, 1, &TC_INTERRUPT);
        tc_start(&AVR32_TC, 1); //Iniciar timer
        state=0;//Regresar a no hacer nada
        break;

      case 2://lectura de los 1000 datos... un dato cada 20ms
      if (received_char < 1000){
        gpio_set_gpio_pin(LED0_GPIO); //apagar led
        data[received_char] = USART_getchar(&AVR32_USART1);
        received_char++;
        state = 0;
      }else{
        received_char == 0;
        gpio_clr_gpio_pin(LED0_GPIO); //Prende led
        tc_stop(&AVR32_TC, 1); //Detiene recepcion de datos USART
        state = 0;
      }

      case 3:
        tc_stop(&AVR32_TC, 1); //Detiene contador, y por lo tanto USART
        //SPI
        //Mandar 1000 datos al apretar tecla DOWN
        for (int i=0; i<1000; i++){
          spi_write(AVR32_SPI0_ADDRESS, data[i]);
        }//for
        break;
    } //Fin Switch
  }; //Fin de While
} //FIN DE MAIN!

/*FUNCIONES********************************************************************/
void teclas(void){//handler teclas up y down

  if (gpio_get_pin_interrupt_flag(QT1081_TOUCH_SENSOR_UP)){
    state = 1; //INICIA CONTADOR
    gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_UP);
  }//UP

	if (gpio_get_pin_interrupt_flag(QT1081_TOUCH_SENSOR_DOWN)){
    state = 3; //INICIA TX SPI
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_DOWN);
	}//DOWN
	gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_UP); // Para que no se trabe...

}//Fin de Teclas



void timer(void){
	tc_read_sr(&AVR32_TC, 1);//Limpiar bandera de interrupción
  // empieza a leer RX en USART de acuerdo al estado 2
  state = 2;
}//timer
