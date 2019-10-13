#include "et024006dhu.h"
#include "delay.h"

void act13(uint8_t valid_state, uint8_t * actual_state){
  clr_disp();
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE );
	et024006_DrawFilledCircle(120,0,195,BLACK,TFT_QUADRANT3);
	et024006_DrawFilledCircle(120,0,194,WHITE,TFT_QUADRANT3);
	et024006_DrawFilledCircle(120,75,120,BLACK,TFT_QUADRANT2);
	et024006_DrawFilledCircle(120,75,119,WHITE,TFT_QUADRANT2);
	et024006_DrawFilledCircle(75,75,75,BLACK,TFT_QUADRANT1);
	et024006_DrawFilledCircle(75,75,74,WHITE,TFT_QUADRANT1);
	et024006_DrawFilledCircle(75,45,45,BLACK,TFT_QUADRANT0);
	et024006_DrawFilledCircle(75,45,44,WHITE,TFT_QUADRANT0);
	et024006_DrawFilledCircle(90,45,30,BLACK,TFT_QUADRANT3);
	et024006_DrawFilledCircle(90,45,29,WHITE,TFT_QUADRANT3);
	et024006_DrawFilledCircle(90,60,15,BLACK,TFT_QUADRANT2);
	et024006_DrawFilledCircle(90,60,14,WHITE,TFT_QUADRANT2);
	et024006_DrawFilledCircle(90,60,15,BLACK,TFT_QUADRANT1);
	et024006_DrawFilledCircle(90,60,14,WHITE,TFT_QUADRANT1);
  while(*actual_state == valid_state){}
}
