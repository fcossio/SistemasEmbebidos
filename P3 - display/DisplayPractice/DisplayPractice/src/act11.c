#include "et024006dhu.h"
#include "delay.h"

void act11(uint8_t valid_state, uint8_t * actual_state){
    clr_disp();
    et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE );
    et024006_DrawHorizLine( 49, 235, 231, BLACK);delay_ms(50);
    et024006_DrawHorizLine( 98, 192, 107, BLACK);delay_ms(50);
    et024006_DrawHorizLine( 49, 137, 49, BLACK);delay_ms(50);
    et024006_DrawHorizLine( 205, 137, 75, BLACK);delay_ms(50);
    et024006_DrawHorizLine( 111, 155, 85, BLACK);delay_ms(50);
    et024006_DrawHorizLine( 70, 112, 41, BLACK);delay_ms(50);
    et024006_DrawHorizLine( 196, 112, 61, BLACK);delay_ms(50);

    et024006_DrawVertLine( 49, 137, 98, BLACK);delay_ms(50);
    et024006_DrawVertLine( 98, 137, 55, BLACK);delay_ms(50);
    et024006_DrawVertLine( 205, 137, 55, BLACK);delay_ms(50);
    et024006_DrawVertLine( 280, 137, 98, BLACK);delay_ms(50);
    et024006_DrawVertLine( 111, 112, 43, BLACK);delay_ms(50);
    et024006_DrawVertLine( 196, 112, 43, BLACK);delay_ms(50);

    et024006_DrawLine( 49, 137, 70, 112, BLACK);delay_ms(50);
    et024006_DrawLine( 98, 137, 111, 112, BLACK);delay_ms(50);
    et024006_DrawLine( 98, 192, 111, 155, BLACK);delay_ms(50);
    et024006_DrawLine( 205, 192, 196, 155, BLACK);delay_ms(50);
    et024006_DrawLine( 205, 137, 196, 112, BLACK);delay_ms(50);
    et024006_DrawLine( 280, 137, 257, 112, BLACK);delay_ms(50);

    et024006_DrawLine( 71, 111, 160, 0, BLUE);delay_ms(50);
    et024006_DrawLine( 112, 111, 160, 0, BLUE);delay_ms(50);
    et024006_DrawLine( 112, 154, 160, 0, BLUE);delay_ms(50);
    et024006_DrawLine( 195, 154, 160, 0, BLUE);delay_ms(50);
    et024006_DrawLine( 195, 111, 160, 0, BLUE);delay_ms(50);
    et024006_DrawLine( 256, 111, 160, 0, BLUE);delay_ms(50);
  while(*actual_state == valid_state){}
}
