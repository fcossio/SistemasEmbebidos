#include "et024006dhu.h"
#include "stdlib.h"

void act2(uint8_t valid_state, uint8_t * actual_state, enum btn * btn_pressed){
  uint16_t size=1,j=1,k=1,rnd=0;
  et024006_DrawFilledRect(0 , 0, 320, 240, BLACK );
  while(*actual_state == valid_state){
    if(*btn_pressed==UP){
      size<<=1;
      size=(size>32)?(size>>1):size;
      *btn_pressed = NONE;
    }//If
    if(*btn_pressed==DOWN){
      size>>=1;
      size=(size<1)?(1):size;
      et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
      *btn_pressed = NONE;
    }//If
    for( k=0; k<8; k++){
      for(j=0;j<8;j++){
        et024006_DrawFilledRect(k*size,j*size,size,size,(k+j)%2==0?BLACK:WHITE);
      }//For
    }//For
  }//While
}
