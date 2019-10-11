#include "et024006dhu.h"

void act7(void){

  uint16_t colors[] = {
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

  uint16_t position = 0;
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
      et024006_DrawFilledRect(40 * i, 120, 40, 40, colors[ (i+2) % 9 ] );
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
      et024006_DrawFilledRect(20 * i, 230, 20, 10, colors[ (i+4) % 9 ] );
  }
}
