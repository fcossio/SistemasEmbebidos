#include "et024006dhu.h"
#include "stdlib.h"

void act1(uint8_t valid_state, uint8_t * actual_state){
    int c=WHITE;
  	for(int i=0; i<8; i++){
  		if(i%2)
  		c=BLACK;
  		else
  		c=WHITE;
  		for(int j=0;j<8;j++){
  			et024006_DrawFilledRect(j*30,i*30,30,30,c);
  			if(c==BLACK)
  			c=WHITE;
  			else
  			c=BLACK;
  			delay_ms(10);
  		}//For
  	}//For
  while(*actual_state == valid_state){}
}
