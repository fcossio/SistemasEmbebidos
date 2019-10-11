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
#include "display_utils.c"

// #include "act1.c"
// #include "act2.c"
// #include "act3.c"
#include "act4.c"
// #include "act5.c"
// #include "act6.c"
#include "act7.c"
#include "act8.c"
// #include "act9.c"
// #include "act10.c"
// #include "act11.c"
// #include "act12.c"
// #include "act13.c"
#include "act14.c"
// #include "act15.c"
// #include "act16.c"
// #include "act17.c"
// #include "act18.c"
// #include "act19.c"
// #include "act20.c"


// Init pwm
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

  /* PWM is fed by PBA bus clock which is by default the same
   * as the CPU speed. We set a 0 duty cycle and thus keep the
   * display black*/
  tft_bl_init();
  /* Lets do a nice fade in by increasing the duty cycle */
  while(pwm_channel6.cdty < pwm_channel6.cprd)
  {
    pwm_channel6.cdty++;
    pwm_channel6.cupd = pwm_channel6.cdty;
    //pwm_channel6.cdty--;
    pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
    delay_ms(1);
  }
  // Clear the display i.e. make it black
  et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
  // Draw the background AVR32 logo.
  // et024006_PutPixmap(avr32_logo, 320, 0, 0, 0, 0, 320, 240);
  // Draw a crossed square.
  et024006_DrawHorizLine(10, 50, 20, BLACK);
  et024006_DrawVertLine(10, 50, 20, BLACK);
  et024006_DrawHorizLine(10, 70, 20, BLACK);
  et024006_DrawVertLine(30, 50, 20, BLACK);
  et024006_DrawLine(10, 50, 30, 70, BLACK);
  et024006_DrawLine(30, 50, 10, 70, BLACK);

  uint8_t act = 14;
  switch(act){
    case 1:
      //act1();
      break;
    case 2:
      //act2();
      break;
    case 3:
      //act3();
      break;
    case 4:
      act4();
      break;
    case 5:
      //act5();
      break;
    case 6:
      //act6();
      break;
    case 7:
      act7();
      break;
    case 8:
      act8();
      break;
    case 9:
      //act9();
      break;
    case 10:
      //act10();
      break;
    case 11:
      //act11();
      break;
    case 12:
      // act12();
      break;
    case 13:
      //act13();
      break;
    case 14:
      act14();
      break;
    case 15:
      // act15();
      break;
    case 16:
      // act16();
      break;
  }

  // Display text.
  et024006_PrintString(
    "Actividad 7: prueba de color",
    (const unsigned char *)&FONT6x8,
    0, 0, BLUE, -1
  );
  while(true);
}
