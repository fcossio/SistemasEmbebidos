#include "et024006dhu.h"

void act9(){

		int x = 160;
		int y = 120; //para obtener el centro
		int radio = 200;//320 ser�a el diametro
		int r,g,b;
		r = 255;
		g = 0;
		b = 0;

		while(radio > 0){

			if (r == 255 && g < 255 && b == 0 ){
				g = g + 17;
				}else if (r > 0 && g == 255 && b == 0){
				r = r - 17; //17
				}else if(r == 0 && g == 255 && b < 255){
				b = b + 17;
				}else if(r == 0 && g > 0 && b == 255){
				g = g - 17;
				}else if(r < 255 && g == 0 && b == 255){
				r = r + 17;
				}else if(r == 255 && g == 0 && b > 0){
				b = b - 17;
			}

			radio = radio - 2;
			et024006_DrawFilledCircle( x,  y,  radio,
				et024006_Color(r ,g , b),
				(TFT_QUADRANT0|TFT_QUADRANT1|TFT_QUADRANT2|TFT_QUADRANT3)
			);

		}

}
