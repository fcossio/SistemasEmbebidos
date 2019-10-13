// Actividad 7 Desplegar en todo el display la prueba decolor utilizada en los
// sistemas de video

#include "et024006dhu.h"
#include "delay.h"

#define TFT_QUADRANT0   ((1 << 1) | (1 << 0))
#define TFT_QUADRANT1   ((1 << 3) | (1 << 2))
#define TFT_QUADRANT2   ((1 << 5) | (1 << 4))
#define TFT_QUADRANT3   ((1 << 7) | (1 << 6))

// Function names and descriptions
uint16_t color16(uint8_t r, uint8_t g, uint8_t b);
void draw_gradient_rectangle( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color1, uint16_t color2, uint8_t vertical);
void drawHollowTriangle(int centerx, int centery, int radio, int dir, int color);
void drawFilledTriangle(int centerx, int centery, int radio, int dir, int color);
void clr_disp(void);

// Functions
uint16_t color16(uint8_t r, uint8_t g, uint8_t b){
  uint16_t color = (b)|((g)<<5)|((r)<<11);
  return(color);
}
void clr_disp(void){
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
}
void draw_gradient_rectangle( uint16_t x, uint16_t y, uint16_t width,
    uint16_t height, uint16_t color1, uint16_t color2, uint8_t vertical){
  int r, g, b, delta_r, delta_g, delta_b;
  delta_r = ((color2&0xF800)>>11) - ((color1&0xF800)>>11);
  delta_g = ((color2&0x7E0)>>5) - ((color1&0x7E0)>>5);
  delta_b = (color2&0x1F) - (color1&0x1F);
  if(vertical){
    for (uint16_t i = 0; i < height; i++){
      r = delta_r * i;
      r = r/height + (color1>>11);
      g = delta_g *  i;
      g = g/height + ((color1&0x7E0)>>5);
      b = delta_b * i;
      b = b/height + (color1&0x1F);
      et024006_DrawHorizLine(x, y + i, width, color16(r,g,b));
    }
  }else{
    for (uint16_t i = 0; i < width; i++){
      r = delta_r * i;
      r = r/width + (color1>>11);
      g = delta_g *  i;
      g = g/width + ((color1&0x7E0)>>5);
      b = delta_b * i;
      b = b/width + (color1&0x1F);
      et024006_DrawVertLine(x + i, y, height, color16(r,g,b));
      //debugging
      // char r_str[8], g_str[8], b_str[8];
      // et024006_DrawFilledRect(0, 0, 45, 120, color16(63,127,63) );
      // sprintf(r_str,"%d", delta_r);
      // sprintf(g_str,"%d", delta_g);
      // sprintf(b_str,"%d", delta_b);
      // et024006_PrintString(r_str, (const unsigned char *)&FONT6x8, 0, 0, BLUE, -1);
      // et024006_PrintString(g_str, (const unsigned char *)&FONT6x8, 0, 10, BLUE, -1);
      // et024006_PrintString(b_str, (const unsigned char *)&FONT6x8, 0, 20, BLUE, -1);
      delay_ms(1);
    }
  }
}

void drawHollowTriangle(int centerx, int centery, int radio, int dir, int color){ //dir=0 VOLTEADO ; dir=1 NORMAL
  int l,b,p1x,p1y,p2x,p2y,p3x,p3y;
  b= radio*(0.866);
	l = (radio)*(0.5);
	if (dir ==1) //TRIANGULO NORMAL
	{	p1x=centerx;
		p1y=centery-radio;
		p2x=centerx-b;
		p2y=centery+l;
		p3x=centerx+b;
		p3y=centery+l;
		et024006_DrawLine(p1x,p1y,p2x,p2y,color);
		et024006_DrawLine(p1x,p1y,p3x,p3y,color);
		et024006_DrawLine(p2x,p2y,p3x,p3y,color);
		} else { //TRIANGULO INVERTIDO
		p1x=centerx;
		p1y=centery+radio;
		p2x=centerx-b;
		p2y=centery-l;
		p3x=centerx+b;
		p3y=centery-l;
		et024006_DrawLine(p1x,p1y,p2x,p2y,color);
		et024006_DrawLine(p1x,p1y,p3x,p3y,color);
		et024006_DrawLine(p2x,p2y,p3x,p3y,color);
	}//Fin if
}//Fin fn

void drawFilledTriangle(int centerx, int centery, int radio, int dir, int color){
	for(int i=0; i<=radio; i++){
		drawHollowTriangle(centerx,centery,radio-i,dir,color);
	}//For
}//drawFilledTriangle
