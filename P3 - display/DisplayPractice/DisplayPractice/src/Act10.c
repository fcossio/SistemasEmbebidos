#include "et024006dhu.h"
void act10(uint8_t valid_state, uint8_t * actual_state){
	do{
  // Draw the picture.
  et024006_PutPixmap(avr32_logo, 320, 0, 0, 0, 0, 320, 240);
  }while(*actual_state == valid_state);
}
