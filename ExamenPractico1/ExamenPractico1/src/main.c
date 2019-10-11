#include <asf.h>

#define BTN_UP   AVR32_PIN_PB22
#define BTN_DOWN AVR32_PIN_PB23
#define BTN_RIGHT AVR32_PIN_PB24
#define BTN_LEFT AVR32_PIN_PB25
#define BTN_CENTER AVR32_PIN_PB26

#define LED0   AVR32_PIN_PB27
#define LED1   AVR32_PIN_PB28
#define LED2   AVR32_PIN_PA05
#define LED3   AVR32_PIN_PA06

uint8_t generalFlag=5;
uint8_t center =0;
uint8_t specificFlag=0;
uint8_t counter =0;

__attribute__ ((__interrupt__));
void Botones (void);

//Init FN y Variables Globales
void inicializa_PM(void);
void Inicializa_PLL(uint8_t mul);
void Prender_Leds(uint8_t value);
void Izq_Der(void);
void Der_Izq(void);
void Prender_Leds_Cero(uint8_t value);

int main (void)
{

	inicializa_PM();
	delay_init(12000000);
	
	board_init();
	
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&Botones, 70, 3);
	INTC_register_interrupt(&Botones, 71, 3);
	
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
	
	gpio_enable_pin_interrupt(BTN_UP,GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(BTN_DOWN,GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(BTN_RIGHT,GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(BTN_LEFT,GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(BTN_CENTER,GPIO_FALLING_EDGE);
	
	Enable_global_interrupt();
	
	
	
	gpio_set_gpio_pin(LED0); //LEDS apagados
	gpio_set_gpio_pin(LED1);
	gpio_set_gpio_pin(LED2);
	gpio_set_gpio_pin(LED3);
	
	while (true)
	{
		//generalFlag=2;
		switch (generalFlag) {
			
			case 0: //UP
			specificFlag=1;
			if (counter !=15){counter ++; }
			Prender_Leds(counter);
			break;
			
			case 1: //DOWN
			specificFlag=1;
			if(counter != 0){ counter--;}
			Prender_Leds(counter);
			break;
			
			case 2: //RIGHT
			specificFlag=2;
			Prender_Leds(0); //Todos los Leds Apagados
			delay_ms(5000);
			Izq_Der(); //Inicia Secuencia
			break;
			
			case 3: //LEFT
			specificFlag=3;
			Prender_Leds_Cero(0); //Todos los Leds Apagados
			Der_Izq(); //Inicia Secuencia
			break;
			
			case 4: //CENTER
			counter=0;
			Prender_Leds(0);
			break;
			if (specificFlag==2){ center=1; } //En el caso de que se haya presionado la tecla RIGHT para corrimiento
			
			default: //Default
			specificFlag=0;
			delay_ms(10);
			break;
		} //Fin switch
		
	} //Fin While
	
}//Fin de Main


void Der_Izq(void){
	uint8_t numero=1;
	uint8_t mul=3; //Para PLL0
	uint8_t back=0;
	
	while (center!=1){   //Mientras no haya entrado un Enter
		for (int i=0; i<4; i++){ //Prende de DER a IZQ
			Prender_Leds(numero);
			numero=numero*2;
			delay_ms(500);
			if (i==3)
			{
				Inicializa_PLL(mul); //FN PARA CAMBIAR FRECUENCIA DE PLL
				if (mul ==6){
					back=1;
					} else if (mul == 3){ //limite para Fout, tiene que ir de regreso
					back=0;
				}
				if (back == 0){mul++;} else {mul --;}
				numero=8; //Empieza el corrimiento otra vez
				i=0;
			}//Fin IF
			if (center == 1){break;}
		}//Fin FOR
	}//Fin de While
}//Fin Fn


void Prender_Leds_Cero(uint8_t value){

	uint8_t num = value;
	uint8_t result;
	gpio_set_gpio_pin(LED0);
	gpio_set_gpio_pin(LED1);
	gpio_set_gpio_pin(LED2);
	gpio_set_gpio_pin(LED3);
	result= num % 2;
	if (result ==1){gpio_set_gpio_pin(LED0);}else {gpio_clr_gpio_pin(LED0);} //Con Set apaga los LEDS!
	num=num/2;
	result = num % 2;
	if (result ==1){gpio_set_gpio_pin(LED1);}else {gpio_clr_gpio_pin(LED1);}
	num=num/2;
	result= num % 2;
	if (result ==1){gpio_set_gpio_pin(LED2);}else {gpio_clr_gpio_pin(LED2);}
	num=num/2;
	result= num % 2;
	if (result ==1){gpio_set_gpio_pin(LED3);}else {gpio_clr_gpio_pin(LED3);}
	num=num/2;
}//Fin Fn

void Izq_Der(void){
	uint8_t numero=8;//Este numero en bin: 1000, 0100, 0010, 0001 (8,4,2,1) 
	uint8_t mul=3; //Para PLL0
	uint8_t back=0;
	
	while (center!=1){   //Mientras no haya entrado un Enter
		for (int i=0; i<4; i++){ //Prende de Izq a der
			delay_ms(500);
			Prender_Leds(numero); //Imprime la primera vez 1000
			numero=numero/2;
			delay_ms(500);
			
			if (i==3) //Si se cumple, ya hizo una corrida entera 1000, 0100, 0010, 0001
			{
				//FN PARA CAMBIAR FRECUENCIA DE PLL
				Inicializa_PLL(mul);
				if (mul ==6){
					back=1;
					} else if (mul == 3){ //limite para Fout, tiene que ir de regreso
					back=0;
				}
				if (back == 0){mul++;} else {mul --;}
				numero=8; //Empieza el corrimiento otra vez
				i=0;
			}//Fin IF
			if (center == 1){break;} //Si se PRESIONO LA TECLA CENTER, Sale de aqui y mantiene el valor que quedo
		}//Fin FOR
	}//Fin de While
}//Fin IzqDer


void Prender_Leds(uint8_t value){
	
	uint8_t num=value;
	uint8_t result;
	
	gpio_set_gpio_pin(LED0);
	gpio_set_gpio_pin(LED1);
	gpio_set_gpio_pin(LED2);
	gpio_set_gpio_pin(LED3);
	result= num % 2;
	if (result ==1){gpio_clr_gpio_pin(LED3);}else {gpio_set_gpio_pin(LED3);}
	num=(int) num/2;
	result = num % 2;
	if (result ==1){gpio_clr_gpio_pin(LED2);}else {gpio_set_gpio_pin(LED2);}
	num=(int)num/2;
	result= num % 2;
	if (result ==1){gpio_clr_gpio_pin(LED1);}else {gpio_set_gpio_pin(LED1);}
	num=(int)num/2;
	result= num % 2;
	if (result ==1){gpio_clr_gpio_pin(LED0);}else {gpio_set_gpio_pin(LED0);}
	num=(int)num/2;
	generalFlag=5;
}//Fin Fn



void Botones (void){
	
	if (gpio_get_pin_interrupt_flag(BTN_UP) == 1) {
		generalFlag=0;
		gpio_clear_pin_interrupt_flag(BTN_UP);
	}
	
	if (gpio_get_pin_interrupt_flag(BTN_DOWN)){
		generalFlag=1;
		gpio_clear_pin_interrupt_flag(BTN_DOWN);
	}
	
	if (gpio_get_pin_interrupt_flag(BTN_RIGHT)){
		generalFlag=2;
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}
	
	if (gpio_get_pin_interrupt_flag(BTN_LEFT)){
		generalFlag=3;
		gpio_clear_pin_interrupt_flag(BTN_LEFT);
	}
		
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
		generalFlag=4;
		}
		
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
		generalFlag=4;
	}
	
} //Fin Botones



void inicializa_PM (void){
	pm_switch_to_osc0(&AVR32_PM,12000000,3); //fOSC= 12MHz, startup 18ms
	flashc_set_wait_state(1);
} //Fin PM

void Inicializa_PLL(uint8_t mul){
	pm_switch_to_osc0(&AVR32_PM, 12000000,3);
	pm_pll_setup(&AVR32_PM,0, mul, 0,0,16); //pll0, mul variable, sin div, 16 lockcount
	pm_pll_set_option(&AVR32_PM,0,1,0,0);  //pll0, 80-180, no divide/2, start normal
	pm_pll_enable(&AVR32_PM,0);
	pm_wait_for_pll0_locked(&AVR32_PM);
	pm_switch_to_clock(&AVR32_PM,2);//PLL como MC
	flashc_set_wait_state(1);
}//Fin Fn

//PARA FOSC=12 MHz
//mul=3 fpll=96MHz
//mul=4 fpll=120MHz
//mul=5 fpll=144MHz
//mul=6 fpll=168MHz