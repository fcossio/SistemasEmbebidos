#include "et024006dhu.h"
#include "delay.h"

void act19(uint8_t valid_state, uint8_t * actual_state){
  clr_disp(); //limplia el display
  #define RADIO 30
  for(int y=RADIO; y<=(ET024006_HEIGHT); y+=1.5*RADIO){ //2*sin(60)
    for (int x=(-(y%2)*(0.8*RADIO)); x<=(ET024006_WIDTH+RADIO); x+=(0.866*RADIO)){ //sin(60)
      if(x%2==1){
        drawFilledTriangle(x,y,RADIO,1,color16(rand()%31,rand()%63,rand()%31));//No invertido
      }else{
        drawFilledTriangle(x,y-RADIO/2,RADIO,0,color16(rand()%31,rand()%63,rand()%31));//Invertido
      }//If
    }//For X
  }//For Y
  while (valid_state == *actual_state){}
}
