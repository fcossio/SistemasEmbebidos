
#include "board.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "et024006dhu.h"
#include "delay.h"
#include "conf_clock.h"
#include "pwm.h"
#include "tc.h"
#include <stdio.h>
#include <stdlib.h>

static void tft_bl_init(void);
__attribute__ ((__interrupt__)) void teclas(void);
__attribute__ ((__interrupt__)) void timer(void);

void selectionSort(short *salida, short longitud);
void bubbleSort(short *salida, short longitud);
void quickSort(short *salida, short inicio, short fin, short longitud);
void shellSort(short *salida, short longitud);
void copiarArreglo(short *a, short *b, short longitud);
void CLR_disp(void);
void drawSort(short *arreglo);

volatile uint32_t enter= 1;
volatile uint16_t actividad = 0;
avr32_pwm_channel_t pwm_channel6 = {.cdty = 0,.cprd = 100};
short contadorMs;
char tiempoTranscurrido[4];
short centecimas;
short currentSort = 1;

int main(){
	
	  static const tc_waveform_opt_t WAVEFORM_OPT = {
		  .channel  = 0,                        // Channel selection.
		  .bswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOB.
		  .beevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOB.
		  .bcpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOB.
		  .bcpb     = TC_EVT_EFFECT_NOOP,                // RB compare effect on TIOB.
		  .aswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOA.
		  .aeevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOA.
		  .acpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOA: toggle.
		  .acpa     = TC_EVT_EFFECT_NOOP,                  // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).
		  .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
		  .enetrg   = 0,                                          // External event trigger enable.
		  .eevt     = 0,                                                    // External event selection.
		  .eevtedg  = TC_SEL_NO_EDGE,                   // External event edge selection.
		  .cpcdis   = 0,                                         // Counter disable when RC compare.
		  .cpcstop  = 0,                                        // Counter clock stopped with RC compare.
		  .burst    = 0,                                           // Burst signal selection.
		  .clki     = 0,                                            // Clock inversion.
		  .tcclks   = TC_CLOCK_SOURCE_TC3         // Internal source clock 3, connected to fPBA / 8.
	  };//Config

	  static const tc_interrupt_t TC_INTERRUPT = {
		  .etrgs = 0,
		  .ldrbs = 0,
		  .ldras = 0,
		  .cpcs  = 1,   // Habilitar interrupción por comparación con RC
		  .cpbs  = 0,
		  .cpas  = 0,
		  .lovrs = 0,
		  .covfs = 0
	  };//Interrupcion timer

	
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	et024006_Init( FOSC0, FOSC0 );
	tft_bl_init();
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );

	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&teclas, 71, 0);
	INTC_register_interrupt(&teclas, 70,0);
	INTC_register_interrupt(&timer, AVR32_TC_IRQ0, AVR32_INTC_INT0); //Timer
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_LEFT,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_RIGHT,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_ENTER,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_UP,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_DOWN,GPIO_RISING_EDGE);
	Enable_global_interrupt();
	
	tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT);
	tc_write_rc(&AVR32_TC, 0, 15000);//15000 - 10ms
	tc_configure_interrupts(&AVR32_TC, 0, &TC_INTERRUPT);
	tc_start(&AVR32_TC, 0);

	while(pwm_channel6.cdty < pwm_channel6.cprd){
		pwm_channel6.cdty++;
		pwm_channel6.cupd = pwm_channel6.cdty;
		pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
	}//PWM
	
	short desordenado[320];
	short longitudDeArreglo = sizeof(desordenado)/sizeof(desordenado[0]);
	short ordenadoSelectionSort[longitudDeArreglo];
	short ordenadoBubbleSort[longitudDeArreglo];
	short ordenadoQuickSort[longitudDeArreglo];
	short ordenadoShellSort[longitudDeArreglo];

	for(short k=0; k<longitudDeArreglo; k++){desordenado[k] = rand()%200;}

	copiarArreglo(desordenado, ordenadoBubbleSort, longitudDeArreglo);
	copiarArreglo(desordenado, ordenadoSelectionSort, longitudDeArreglo);
	copiarArreglo(desordenado, ordenadoQuickSort, longitudDeArreglo);
	copiarArreglo(desordenado, ordenadoShellSort, longitudDeArreglo);
	
	gpio_clr_gpio_pin(LED1_GPIO);

	while(1){
		switch (actividad){
			case 0:
			CLR_disp();
			bubbleSort(ordenadoBubbleSort, longitudDeArreglo);
			while(enter);
			actividad++;
			break;
			
			case 1:
			CLR_disp();
			selectionSort(ordenadoSelectionSort, longitudDeArreglo);
			while(enter);
			actividad++;
			break;

			case 2:
			CLR_disp();
			quickSort(ordenadoQuickSort, 0, longitudDeArreglo-1, longitudDeArreglo);
			while(enter);
			actividad++;
			break;

			case 3:
			CLR_disp();
			shellSort(ordenadoShellSort, longitudDeArreglo);
			while(enter);
			actividad++;
			break;
			
		}//Switch
	}//While Actividad

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
}

void teclas(void){//handler teclas left, right o center
	if (gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_RIGHT)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_RIGHT);
	}//RIGHT
	if (gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_LEFT)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_LEFT);
	}//LEFT
	if (gpio_get_pin_interrupt_flag(QT1081_TOUCH_SENSOR_DOWN)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_DOWN);
	}//DOWN
	if (gpio_get_pin_interrupt_flag(QT1081_TOUCH_SENSOR_UP)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_UP);
	}//UP
	if (gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_ENTER)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_ENTER);
		enter=0;
		contadorMs=0;
		centecimas=0;
	}//IF
}//Teclas

void bubbleSort(short *salida, short longitud){
	enter = 1;
	currentSort = 1;
    short aux1=0, aux2=0, cont1=0, cont2=0;
    int direccion = salida;
    while(cont2 < longitud-1){
        do{ aux1 = *salida;
            salida++;
            if (*salida < aux1){
                aux2 = *salida;
                *salida = aux1;salida--;
                *salida = aux2;salida++;
            }//if
            cont1++;
        }while (cont1 < longitud-cont2-1);
        salida = direccion;
        cont2++; cont1=0;
	drawSort(salida);delay_ms(4);
    }//While
}//bubbleSort

void selectionSort(short *salida, short longitud) {
	enter = 1;
	currentSort = 2;
	short x, y, min, tmp;
	for(x = 0; x < longitud; x++) {
		min = x;
		for(y = x + 1; y < longitud; y++) {
			if(salida[min] > salida[y]) {
				min = y;
			}//if
		}//for
		tmp = salida[min];
		salida[min] = salida[x];
		salida[x] = tmp;
		drawSort(salida);delay_ms(4);
	}//for
}//selectionSort

void quickSort(short *salida, short inicio, short fin, short longitud){
	enter = 1;
	currentSort = 3;
    short i = inicio, f=fin, t;
    short x = salida[(inicio+fin)/2];
    do{
        while(salida[i]<x&&f<=fin){i++;}//while
        while(x<salida[f]&&f>inicio){f--;}//while bhvgcfvgbjhknlm,
        if(i<=f){
            t=salida[i];
            salida[i]=salida[f];
            salida[f]=t;
            i++; f--;
        }//if
		drawSort(salida);
    }while (i<=f);//do
    if(inicio<f){
        quickSort(salida,inicio,f,longitud);
    }//if
    if(i<fin){
        quickSort(salida,i,fin,longitud);
    }//if
}//quickSort

void shellSort(short *salida, short longitud){
	enter = 1;
	currentSort = 4;
    short i,x,y,t;
    for(i=1;i<longitud;i=i*3+1){}
    while (i>0){
        for (x=i; x<longitud; x++){
            y=x;
            t=salida[x];
            while(y>=i&&salida[y-i]>t){
                salida[y]=salida[y-i];
                y=y-i;
            }//while
            salida[y]=t;
			drawSort(salida);
        }//for
        i=i/2;
    }//while
}//shellSort

void copiarArreglo(short *a, short *b, short longitud){
	short contador = 0;
	while(contador < longitud){
		*b = *a;
		b++; a++; contador++;
	}//While
}//copiarArreglo

void drawSort(short *arreglo){
	for (int l=0; l<ET024006_WIDTH; l++){
		et024006_DrawVertLine( l, 0, ET024006_HEIGHT, BLACK);
		et024006_DrawVertLine( l, ET024006_HEIGHT-arreglo[l], arreglo[l], WHITE);
	}//for
	switch(currentSort){
		case 1:
		et024006_PrintString("Bubble Sort:", (const unsigned char *)&FONT8x8, 150, 20, WHITE, -1);
		break;
		case 2:
		et024006_PrintString("Select Sort:", (const unsigned char *)&FONT8x8, 150, 20, WHITE, -1);
		break;
		case 3:
		et024006_PrintString("Quick Sort:", (const unsigned char *)&FONT8x8, 150, 20, WHITE, -1);
		break;
		case 4:
		et024006_PrintString("Shell Sort:", (const unsigned char *)&FONT8x8, 150, 20, WHITE, -1);
		break;
	}//Switch
	et024006_PrintString(tiempoTranscurrido, (const unsigned char *)&FONT8x8, 260, 20, WHITE, -1);
}//drawSort

void timer(void){
	tc_read_sr(&AVR32_TC, 0);//Limpiar bandera
	contadorMs++;
	if(contadorMs==10){
		centecimas++;
		tiempoTranscurrido[3] = (centecimas%10)+'0';
		tiempoTranscurrido[2] = '.';
		tiempoTranscurrido[1] = (centecimas%100)/10+'0';
		tiempoTranscurrido[0] = (centecimas%1000)/100+'0';
		gpio_tgl_gpio_pin(LED0_GPIO);//Cada 100ms
		contadorMs=0;
	}//If
}//timer
