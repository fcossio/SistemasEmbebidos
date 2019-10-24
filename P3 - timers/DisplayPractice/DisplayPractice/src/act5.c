#include "et024006dhu.h"
#include "stdlib.h"

void act5(uint8_t valid_state, uint8_t * actual_state, enum btn * btn_pressed){

    uint16_t size=1,j=1,k=1,rr,rg,rb;
  	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
  	while(*actual_state == valid_state){
  		if(*btn_pressed==UP){
  			size<<=1;
  			size=(size>32)?(size>>1):size;
        *btn_pressed = NONE;
  		}//IF
  		if(*btn_pressed==DOWN){
  			size>>=1;
  			size=(size<1)?(1):size;
  			et024006_DrawFilledRect(0 , 0, 320, 240, BLACK );
        *btn_pressed = NONE;
  		}//If
  		rr=rand();
  		rg=rand();
  		rb=rand();
  		for( k=0; k<8; k++){
  			for(j=0;j<8;j++){
  				et024006_DrawFilledRect(k*size,j*size,size,size,(k+j)%2==0?color16(rr%31,rg%63,rb%31):WHITE);
  			}//For
  		}//For
  		delay_ms(1);
  	}//WhilerawPixel( rnd_px_x, rnd_px_y, color16(rnd_r, rnd_g, rnd_b) );
}
