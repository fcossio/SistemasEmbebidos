#include "et024006dhu.h"
#include "delay.h"

void act3(uint8_t valid_state, uint8_t * actual_state, enum btn * btn_pressed){
  point2D v = {.x = 0.0,.y = 0.0}, r = {.x = 0.0,.y = 0.0};
  while(*actual_state == valid_state){
    if(*btn_pressed == UP){
      v.y = v.y - 10.0;
      *btn_pressed = NONE;
    }
    if(*btn_pressed == DOWN){
      v.y = v.y + 10.0;
      *btn_pressed = NONE;
    }
    if(*btn_pressed == RIGHT){
      v.x = v.x + 10.0;
      *btn_pressed = NONE;
    }
    if(*btn_pressed == LEFT){
      v.x = v.x - 10.0;
      *btn_pressed = NONE;
    }
    if((r.x + v.x)< ET024006_WIDTH - 11 && (r.x + v.x)> 0.0 ){
      r.x = r.x + v.x;
    }
    if((r.y + v.y)<ET024006_HEIGHT - 11 && (r.y + v.y)> 0.0 ){
      r.y = r.y + v.y;
    }

    clr_disp();
    draw_square2D(&r, 10, WHITE);
    v.x /= 2.0;
    v.y /= 2.0;
    delay_ms(33);
  }
}
