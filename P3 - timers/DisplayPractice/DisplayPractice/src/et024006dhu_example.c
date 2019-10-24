#include "board.h"
#include "gpio.h"
#include "tc.h"
#include "power_clocks_lib.h"
#include "et024006dhu.h"
#include "delay.h"
#include "avr32_logo.h"
#include "conf_clock.h"
#include "stdio.h"
#include "stdlib.h"
#include "display_utils.c"
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
  pwm_init(&opt);
  pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
  pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
  pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
  pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;
  pwm_channel_init(6, &pwm_channel6);
  pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);
}

#define BTN_UP   AVR32_PIN_PB22
#define BTN_DOWN AVR32_PIN_PB23
#define BTN_RIGHT AVR32_PIN_PB24
#define BTN_LEFT AVR32_PIN_PB25
#define BTN_CENTER AVR32_PIN_PB26

#define BOARD_HEIGHT 6
#define BOARD_WIDTH 7
#
enum btn{NONE, UP, DOWN, LEFT, RIGHT, CENTER};
enum btn btn_pressed = NONE;
uint8_t state = 0;
uint8_t state_num = 2; //state_num will keep state within possible states

enum spc{NOPLAYER, BLUE_PLAYER, GREEN_PLAYER};

enum spc board[BOARD_WIDTH][BOARD_HEIGHT];

uint8_t drop_pos = 0;
uint8_t win = 0;
int delay_time = 100000;
uint8_t counter=100;

__attribute__ ((__interrupt__));
void buttons_interrupt_routine (void);
__attribute__ ((__interrupt__)) void timer(void);

//spc * get_spc_ptr(spc * c00, int x, int y);
void rst_board();
void draw_game();
void drop_chip();
void winner ();
void print_time_left();
void take_turn(uint8_t selected_state);
// Main function


int main(void)
{
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
  // Set CPU and PBA clock
  pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
  gpio_enable_gpio_pin(LED0_GPIO);
  gpio_enable_gpio_pin(LED1_GPIO);
  gpio_enable_gpio_pin(LED2_GPIO);
  gpio_enable_gpio_pin(LED3_GPIO);
  et024006_Init( FOSC0, FOSC0 );
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&buttons_interrupt_routine, 70, 3);
	INTC_register_interrupt(&buttons_interrupt_routine, 71, 3);
  INTC_register_interrupt(&timer, AVR32_TC_IRQ0, AVR32_INTC_INT0); //Timer
	uint16_t button_ref [] = {BTN_UP,BTN_DOWN,BTN_RIGHT,BTN_LEFT,BTN_CENTER};
  tc_init_waveform(&AVR32_TC, &WAVEFORM_OPT);
	tc_write_rc(&AVR32_TC, 0, 15000);//15000 - 10ms
	tc_configure_interrupts(&AVR32_TC, 0, &TC_INTERRUPT);
	   tc_start(&AVR32_TC, 0);
	for(uint8_t i=0; i<5; i++){
		gpio_enable_gpio_pin(button_ref[i]);
		gpio_enable_pin_pull_up(button_ref[i]);
		gpio_enable_pin_interrupt(button_ref[i],GPIO_FALLING_EDGE);
	}
	Enable_global_interrupt();
  tft_bl_init();
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
  et024006_DrawFilledCircle(drop_pos*34 + 17, 17, 15, 0xFFFF, 0b11111111);
  while(1){
    rst_board();
    if (delay_time>10000){
      delay_time -= 5000;
    }
    tc_write_rc(&AVR32_TC, 0, delay_time);//15000 - 10ms
    win = 0;
    while(!win){
      counter = 100;
      switch(state){
        case 0:
          take_turn(0);
          break;
        case 1:
        take_turn(1);
        break;
        default:
        state = (state + 1) % state_num;
      }
    };
    if (win == 1){
      et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLUE );
      delay_ms(1000);
      state = 0;
    }
    if(win == 2){
      et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, GREEN );
      delay_ms(1000);
      state = 1;
    }
  }
} // main end
void take_turn(uint8_t selected_state){
  draw_game();
  while(state == selected_state){
    if( btn_pressed==LEFT && drop_pos>0){
      drop_pos--;
      et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
      draw_game();
    }
    if( btn_pressed==RIGHT && drop_pos<BOARD_WIDTH-1){
      drop_pos++;
      et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
      draw_game();
    }
    btn_pressed=NONE;
    print_time_left();
    gpio_tgl_gpio_pin(LED0_GPIO);
    if (!counter){
      drop_pos = rand()%BOARD_WIDTH;
      state = (state+1)%state_num;
      break;
    }
  }
  drop_chip();
  winner();
  btn_pressed=NONE;
}
// spc get_spc_ptr(spc * c00, int x, int y){//coordenadas no indexadas en 0
//   spc * coord = c00;
//   coord += x;
//   coord += (y*BOARD_WIDTH);
//   return coord;
// }
void print_time_left(){
  et024006_DrawFilledRect(240,40,80,200-counter*2,0xFFFF);
}
void rst_board (/*spc * c00*/){
  for (int i = 0; i < BOARD_WIDTH; i++){
    for (int j = 0;  j < BOARD_HEIGHT; j++){
      // spc * c = get_spc_ptr(c00, i, j);
      // * c = NONE;
      board[i][j] = NOPLAYER;
    }
  }
}
void draw_game(){
  et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
  uint8_t color = 0xFFFF;
  //position to drop
  if (state){
    et024006_DrawFilledCircle(drop_pos*34 + 17, 17, 15, BLUE, 0b11111111);
  }else{
    et024006_DrawFilledCircle(drop_pos*34 + 17, 17, 15, GREEN, 0b11111111);
  }

  //all board
  for (int i = 0; i < BOARD_WIDTH; i++){
    for (int j = 0;  j < BOARD_HEIGHT; j++){
      switch (board[i][j]){
        case NOPLAYER:
          et024006_DrawFilledCircle(i*34+17, (j+1)*34+17, 15, WHITE, 0b11111111);
          break;
        case BLUE_PLAYER:
          et024006_DrawFilledCircle(i*34+17, (j+1)*34+17, 15, BLUE, 0b11111111);
          break;
        case GREEN_PLAYER:
          et024006_DrawFilledCircle(i*34+17, (j+1)*34+17, 15, GREEN, 0b11111111);
          break;
      }

    }
  }
}
void winner (){
  //board full no winner
  uint8_t c=0;
  for (int i = 0; i < BOARD_WIDTH; i++){
    for (int j = 0; j < BOARD_HEIGHT; j++){
      if (board[i][j]==NOPLAYER){
        c++;
      }
    }
  }
  if (c==0){
    win=3;
  }
  //vertical
  enum spc last = NOPLAYER;
  for (int i = 0; i < BOARD_WIDTH; i++){
    last = NOPLAYER;
    for (int j = 0;  j < BOARD_HEIGHT; j++){
      if(board[i][j] != NOPLAYER){
        if(last == board[i][j]){
          c++;
          if (c==3){
            win = board[i][j];
          }
        }
        else{
          last = board[i][j];
          c=0;
        }
      }
    }
  }
  //Horizontal
  for (int j = 0;  j < BOARD_HEIGHT; j++){
    last = NOPLAYER;
    for (int i = 0; i < BOARD_WIDTH; i++){
      if(board[i][j] != NOPLAYER){
        if(last == board[i][j]){
          c++;
          if (c==3){
            win = board[i][j];
          }
        }
        else{
          last = board[i][j];
          c=0;
        }
      }else{
        c=0;
      }
    }
  }
  //diagonal1
  for (int i = 0; i < BOARD_WIDTH-3; i++){
    for (int j = 3;  j < BOARD_HEIGHT; j++){
      if(board[i][j] == board[i+1][j-1] && board[i][j] == board[i+2][j-2]
          && board[i][j] == board[i+3][j-3] && board[i][j]!=NOPLAYER){
          win = board[i][j];
      }
    }
  }
  //diagonal2
  for (int i = 3; i < BOARD_WIDTH; i++){
    for (int j = 3;  j < BOARD_HEIGHT; j++){
      if(board[i][j] == board[i-1][j-1] && board[i][j] == board[i-2][j-2]
          && board[i][j] == board[i-3][j-3] && board[i][j]!=NOPLAYER){
          win = board[i][j];
      }
    }
  }

}

void drop_chip(){
  if (board[drop_pos][BOARD_HEIGHT-1] == NOPLAYER){
    board[drop_pos][BOARD_HEIGHT-1] = state ? GREEN_PLAYER : BLUE_PLAYER;
  }else{
    for (int j=0; j<BOARD_HEIGHT; j++){
      if (board[drop_pos][j] != NOPLAYER && board[drop_pos][0]==NOPLAYER){
        board[drop_pos][j-1] = state ? GREEN_PLAYER : BLUE_PLAYER;
        break;
      }
    }
  }
  draw_game();
}

void buttons_interrupt_routine (void){
	if (gpio_get_pin_interrupt_flag(BTN_UP)) {
		btn_pressed=UP;
		gpio_clear_pin_interrupt_flag(BTN_UP);
	}
	if (gpio_get_pin_interrupt_flag(BTN_DOWN)){
		btn_pressed=DOWN;
		gpio_clear_pin_interrupt_flag(BTN_DOWN);
	}
	if (gpio_get_pin_interrupt_flag(BTN_RIGHT)){
		btn_pressed=RIGHT;
		gpio_clear_pin_interrupt_flag(BTN_RIGHT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_LEFT)){
		btn_pressed=LEFT;
		gpio_clear_pin_interrupt_flag(BTN_LEFT);
	}
	if (gpio_get_pin_interrupt_flag(BTN_CENTER)){
		btn_pressed=CENTER;
		state = (state + 1) % state_num;
		gpio_clear_pin_interrupt_flag(BTN_CENTER);
		gpio_tgl_gpio_pin(LED0_GPIO);
	}
	gpio_get_pin_interrupt_flag(BTN_CENTER);
} //Fin Botones

void timer(void){
	tc_read_sr(&AVR32_TC, 0);//Limpiar bandera
  if (counter) counter--;
}//timer
