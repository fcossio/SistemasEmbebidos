/*****************************************************************************
 *
 * \file
 *
 * \brief ET024006DHU TFT display driver example.
 *
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *****************************************************************************/

/*! \mainpage
 * \section intro Introduction
 * This is the documentation for the ET024006DHU TFT display driver example.
 *
 * \section files Main Files
 * - et024006dhu.c, .h: the ET024006DHU driver;
 * - et024006dhu_example.c: the example 1
 * - smc_et024006dhu.h: SMC configuration
 *
 * \section compilinfo Compilation Information
 * This software is written for GNU GCC for AVR32 and for IAR Embedded Workbench
 * for Atmel AVR32. Other compilers may or may not work.
 *
 * \section deviceinfo Device Information
 * All AVR32 devices with an EBI and SMC module can be used.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/uc3">Atmel AVR UC3</A>.\n
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "board.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "et024006dhu.h"
#include "delay.h"
#include "avr32_logo.h"
#include "conf_clock.h"
#include "stdio.h"
#include "stdlib.h"

#if BOARD == EVK1105
#include "pwm.h"
avr32_pwm_channel_t pwm_channel6 = {
/*
  .cmr = ((PWM_MODE_LEFT_ALIGNED << AVR32_PWM_CMR_CALG_OFFSET)
    | (PWM_POLARITY_HIGH << AVR32_PWM_CMR_CPOL_OFFSET)
    | (PWM_UPDATE_DUTY << AVR32_PWM_CMR_CPD_OFFSET)
    | AVR32_PWM_CMR_CPRE_MCK_DIV_2),
    */
  //.cdty = 0,
  .cdty = 0,
  .cprd = 100
};
void draw_gradient_rectangle( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color1, uint16_t color2, uint8_t vertical);
uint16_t color16(uint8_t r, uint8_t g, uint8_t b);
static void tft_bl_init(void)
{

  pwm_opt_t opt = {
    .diva = 0,
    .divb = 0,
    .prea = 0,
    .preb = 0
  };
  /* MCK = OSC0 = 12MHz
   * Desired output 60kHz
   * Chosen MCK_DIV_2
   * CPRD = 12MHz / (60kHz * 2) = 100
   *
   * The duty cycle is 100% (CPRD = CDTY)
   * */
  pwm_init(&opt);
  pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
  pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
  pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
  pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;

  pwm_channel_init(6, &pwm_channel6);
  pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);

}
#endif

// Main function
int main(void)
{
  U32 i;

  // Set CPU and PBA clock
  pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);

  gpio_enable_gpio_pin(LED0_GPIO);
  gpio_enable_gpio_pin(LED1_GPIO);
  gpio_enable_gpio_pin(LED2_GPIO);
  gpio_enable_gpio_pin(LED3_GPIO);

  et024006_Init( FOSC0, FOSC0 );

#if BOARD == EVK1105
  /* PWM is fed by PBA bus clock which is by default the same
   * as the CPU speed. We set a 0 duty cycle and thus keep the
   * display black*/
  tft_bl_init();
#elif BOARD == EVK1104 || BOARD == UC3C_EK
  gpio_set_gpio_pin(ET024006DHU_BL_PIN);
#endif
#if BOARD == EVK1105
  /* Lets do a nice fade in by increasing the duty cycle */
  while(pwm_channel6.cdty < pwm_channel6.cprd)
  {
    pwm_channel6.cdty++;
    pwm_channel6.cupd = pwm_channel6.cdty;
    //pwm_channel6.cdty--;
    pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
    delay_ms(1);
  }
#endif
  // Clear the display i.e. make it black
  et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );

  // Draw the background AVR32 logo.
  et024006_PutPixmap(avr32_logo, 320, 0, 0, 0, 0, 320, 240);

  //Pure colors
  int colors[] = {
    color16(63,000,00), //red 0
    color16(00,127,00), //green 1
    color16(00,000,63), //blue 2
    color16(63,127,63), //white 3
    color16(63,127,00), //yellow 4
    color16(00,127,63), //cyan 5
    color16(63,000,63), //magenta 6
    color16(63,127,63), //white 7
    color16(00,000,00) //black 8
  };
  int position = 0;
  for( int i=0 ; i<7 ; i++ ){
    if(i%3){
      et024006_DrawFilledRect(position, 0, 46, 120, colors[i] );
      position += 46;
    }else{
      et024006_DrawFilledRect(position, 0, 45, 120, colors[i] );
      position += 45;
    }
  }
  for( int i=0 ; i<8 ; i++ ){
      et024006_DrawFilledRect(40 * i, 120, 40, 40, colors[ (i+2) % 8 ] );
  }
  draw_gradient_rectangle( 0, 160, 320, 10, color16(63,127,63), color16(00,000,00),0);
  draw_gradient_rectangle( 0, 170, 160, 10, color16(00,000,00), color16(63,000,00),0);
  draw_gradient_rectangle( 160, 170, 160, 10, color16(63,000,00), color16(63,127,63),0);
  draw_gradient_rectangle( 0, 180, 160, 10, color16(00,000,00), color16(00,127,00),0);
  draw_gradient_rectangle( 160, 180, 160, 10, color16(00,127,00), color16(63,127,63),0);
  draw_gradient_rectangle( 0, 190, 160, 10, color16(00,000,00), color16(00,000,63),0);
  draw_gradient_rectangle( 160, 190, 160, 10, color16(00,000,63), color16(63,127,63),0);

  draw_gradient_rectangle( 0, 200, 160, 10, color16(63,000,00), color16(63,127,00),0);
  draw_gradient_rectangle( 160, 200, 160, 10, color16(63,127,00), color16(00,127,00),0);
  draw_gradient_rectangle( 0, 210, 160, 10, color16(00,127,00), color16(00,127,63),0);
  draw_gradient_rectangle( 160, 210, 160, 10, color16(00,127,63), color16(00,000,63),0);
  draw_gradient_rectangle( 0, 220, 160, 10, color16(00,000,63), color16(63,000,63),0);
  draw_gradient_rectangle( 160, 220, 160, 10, color16(63,000,63), color16(63,000,00),0);


  for( int i=0 ; i<16 ; i++ ){
      et024006_DrawFilledRect(20 * i, 230, 20, 10, colors[ (i+4) % 8 ] );
  }
  // Display lines of colored squares.
  // for( i=0 ; i<16 ; i++ )
  // {
  //   // From black to white.
  //   et024006_DrawFilledRect(20*i,   0, 20, 20, (2*i)/*B:5*/ | ((4*i)<<5)/*G:6*/ | ((2*i)<<11)/*R:5*/ );
  //   // From black to blue.
  //   et024006_DrawFilledRect(20*i,  20, 20, 20, (2*i) /*B:5*/);
  //   // From black to green
  //   et024006_DrawFilledRect(20*i, 200, 20, 20, ((4*i)<<5) /*G:6*/);
  //   // From black to red
  //   et024006_DrawFilledRect(20*i, 220, 20, 20, ((2*i)<<11) /*R:5*/);
  // }




  // Draw a crossed square.
  et024006_DrawHorizLine(10, 50, 20, BLACK);
  et024006_DrawVertLine(10, 50, 20, BLACK);
  et024006_DrawHorizLine(10, 70, 20, BLACK);
  et024006_DrawVertLine(30, 50, 20, BLACK);
  et024006_DrawLine(10, 50, 30, 70, BLACK);
  et024006_DrawLine(30, 50, 10, 70, BLACK);

  // Display text.
  et024006_PrintString("Actividad 7: prueba de color",
    (const unsigned char *)&FONT6x8, 0, 0, BLUE, -1
  );
  while(true);
}

void draw_gradient_rectangle( uint16_t x, uint16_t y, uint16_t width,
    uint16_t height, uint16_t color1, uint16_t color2, uint8_t vertical){
  int r, g, b, delta_r, delta_g, delta_b;
  delta_r = ((color2&0xF800)>>11) - ((color1&0xF800)>>11);
  delta_g = ((color2&0x7E0)>>5) - ((color1&0x7E0)>>5);
  delta_b = (color2&0x1F) - (color1&0x1F);
  if(vertical){
    for (uint16_t i = 0; i < height; i++){
      r = delta_r * i;
      r = r/height + (color1>>11);
      g = delta_g *  i;
      g = g/height + ((color1&0x7E0)>>5);
      b = delta_b * i;
      b = b/height + (color1&0x1F);
      et024006_DrawHorizLine(x, y + i, width, color16(r,g,b));
    }
  }else{
    for (uint16_t i = 0; i < width; i++){
      r = delta_r * i;
      r = r/width + (color1>>11);
      g = delta_g *  i;
      g = g/width + ((color1&0x7E0)>>5);
      b = delta_b * i;
      b = b/width + (color1&0x1F);
      et024006_DrawVertLine(x + i, y, height, color16(r,g,b));
      //debugging
      // char r_str[8], g_str[8], b_str[8];
      // et024006_DrawFilledRect(0, 0, 45, 120, color16(63,127,63) );
      // sprintf(r_str,"%d", delta_r);
      // sprintf(g_str,"%d", delta_g);
      // sprintf(b_str,"%d", delta_b);
      // et024006_PrintString(r_str, (const unsigned char *)&FONT6x8, 0, 0, BLUE, -1);
      // et024006_PrintString(g_str, (const unsigned char *)&FONT6x8, 0, 10, BLUE, -1);
      // et024006_PrintString(b_str, (const unsigned char *)&FONT6x8, 0, 20, BLUE, -1);
      delay_ms(1);
    }
  }
}
uint16_t color16(uint8_t r, uint8_t g, uint8_t b){
  uint16_t color = (b)|((g)<<5)|((r)<<11);
  return(color);
}
