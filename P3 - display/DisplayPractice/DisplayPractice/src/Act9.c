
#include "board.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "et024006dhu.h"
#include "delay.h"
#include "avr32_logo.h"
#include "conf_clock.h"
#include <math.h>

#include "pwm.h"

#define TFT_QUADRANT0 ((1 << 1) | (1 << 0))
#define TFT_QUADRANT1 ((1 << 3) | (1 << 2))
#define TFT_QUADRANT2 ((1 << 5) | (1 << 4))
#define TFT_QUADRANT3 ((1 << 7) | (1 << 6))

avr32_pwm_channel_t pwm_channel6 = {
	.cdty = 0,
	.cprd = 100
};

static void tft_bl_init(void){

	pwm_opt_t opt = {
		.diva = 0,
		.divb = 0,
		.prea = 0,
		.preb = 0
	};

	pwm_init(&opt);
	pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
	pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
	pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
	pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;

	pwm_channel_init(6, &pwm_channel6);
	pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);

}

//Def Funciones
void Circulos();

// Main function
int main(void)
{

	// Set CPU and PBA clock
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);

	gpio_enable_gpio_pin(LED0_GPIO);
	gpio_enable_gpio_pin(LED1_GPIO);
	gpio_enable_gpio_pin(LED2_GPIO);
	gpio_enable_gpio_pin(LED3_GPIO);

	et024006_Init( FOSC0, FOSC0 );
	tft_bl_init();
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE ); //Lienzo en Blanco
	//*********************************
	
	Circulos();
	
	
	
	//*********************************
	
	// Lets do a nice fade in by increasing the duty cycle
	while(pwm_channel6.cdty < pwm_channel6.cprd)
	{
		pwm_channel6.cdty++;
		pwm_channel6.cupd = pwm_channel6.cdty;
		//pwm_channel6.cdty--;
		pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
		delay_ms(10);
	}


	while(true);
}

void Circulos(){
		int x = 160;
		int y = 120; //para obtener el centro
		int radio = 200;//320 sería el diametro
		int r,g,b;
		r = 255;
		g = 0;
		b = 0;

		while(radio > 0){
			
			if (r == 255 && g < 255 && b == 0 ){
				g = g + 17;
				}else if (r > 0 && g == 255 && b == 0){
				r = r - 17; //17
				}else if(r == 0 && g == 255 && b < 255){
				b = b + 17;
				}else if(r == 0 && g > 0 && b == 255){
				g = g - 17;
				}else if(r < 255 && g == 0 && b == 255){
				r = r + 17;
				}else if(r == 255 && g == 0 && b > 0){
				b = b - 17;
			}

			radio = radio - 2;
			et024006_DrawFilledCircle( x,  y,  radio,  et024006_Color(r ,g , b), (TFT_QUADRANT0|TFT_QUADRANT1|TFT_QUADRANT2|TFT_QUADRANT3));
			
		}
	
}

