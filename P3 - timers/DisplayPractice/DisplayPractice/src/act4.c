#include "et024006dhu.h"
#include "stdlib.h"

void act4(uint8_t valid_state, uint8_t * actual_state){
et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
  while(*actual_state == valid_state){
    uint16_t rnd_px_x = rand()%320;
    uint16_t rnd_px_y = rand()%240;
    uint8_t rnd_r = rand()%64;
    uint8_t rnd_g = rand()%128;
    uint8_t rnd_b = rand()%64;
    et024006_DrawPixel( rnd_px_x, rnd_px_y, color16(rnd_r, rnd_g, rnd_b) );
  }
}
