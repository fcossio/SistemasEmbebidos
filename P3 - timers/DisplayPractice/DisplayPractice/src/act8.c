#include "et024006dhu.h"
#include "stdlib.h"
#include "delay.h"

void act8(uint8_t valid_state, uint8_t * actual_state){

  uint16_t colors[] = {
    0xD9E6, //red 0
    0x2A12, //blue 1
    0xF6C7, //yellow 2
    color16(63,127,63), //white 3
    color16(63,127,63), //whitish white 4
    color16(00,000,00) //black 8
  };
  // background grid
  et024006_DrawFilledRect(0, 0, 320, 240, 0x0000 );
  for( uint8_t i = 0; i<16; i++ ){
    for( uint8_t j = 0; j<12; j++ ){
      et024006_DrawFilledRect(20*i + 1, 20* j + 1, 18, 18, 0xFFFF );
    }
  }
  // random rectangles without collision
  // uint8_t used_square[16][12];
  for(uint8_t piece = 0; piece < 20; piece++){
    uint16_t color = colors[3];
    int x1 = rand()%15;
    int y1 = rand()%11;
    int width, height;
    do{
      width = rand()%5 + 1;
      et024006_DrawFilledRect(0, 0, 20, 20, 0x0000);
    }while((width + x1)>16);
    do{
      height = rand()%5 + 1;
      et024006_DrawFilledRect(0, 0, 20, 20, 0xFF00);
    }while((height + y1)>12);
    et024006_DrawFilledRect(20*x1+1, 20*y1+1, 20*width-2, 20*height-2, color);
    delay_ms(50);
  }
  //create pieces
  for(uint8_t piece = 0; piece < 15; piece++){
    uint16_t color = colors[rand()%5];
    int x1 = rand()%15;
    int y1 = rand()%11;
    int width, height;
    do{
      width = rand()%5 + 1;
      et024006_DrawFilledRect(0, 0, 20, 20, 0x0000);
    }while((width + x1)>=16);
    do{
      height = rand()%5 + 1;
      et024006_DrawFilledRect(0, 0, 20, 20, 0xFF00);
    }while((height + y1)>=12);
    if (x1 && y1){
      et024006_DrawFilledRect(20*x1-1, 20*y1-1, 20*width+2, 20*height+2, 0x0000);
    }else{
      et024006_DrawFilledRect(20*x1, 20*y1, 20*width+1, 20*height+1, 0x0000);
    }
    et024006_DrawFilledRect(20*x1+1, 20*y1+1, 20*width-2, 20*height-2, color);
    delay_ms(50);
  }
  while(valid_state == *actual_state){
    //et024006_DrawFilledRect(0, 0, 20, 20, 0x0000);
    //delay_ms(500);
    //et024006_DrawFilledRect(0, 0, 20, 20, 0xFF00);
    //delay_ms(500);
	delay_us(1);
  }
}
