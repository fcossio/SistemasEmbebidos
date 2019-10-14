#include "et024006dhu.h"
#include "delay.h"



void act12(uint8_t valid_state, uint8_t * actual_state){
#define off 20
  int l,b,p1x,p1y,p2x,p2y,p3x,p3y;
  //Triangle illusion
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE ); //Lienzo en Blanco	enter=1;
	int temp1, temp2, temp3, temp4, temp5;

	for (int i=1; i<=10; i++){
		drawHollowTriangle(160, 120+off, 6*i,0, BLACK); //T. invertido

		if (i == 10){
			temp1 = 6*5; //l maxima
			temp2=160+60*(0.866);//p2x;
			temp3=160-60*(0.866);//p3x;
			temp4=2;
			temp5=0;

			for (int j=1; j<=10; j++){
				for (int y=0; y<=3-(j*0.3); y++){ // Y toma los valores: 3,3,3,2,2,2,1,1,1 (Grosor)
					temp4= (6*j)-y;
					drawHollowTriangle(160, 2*temp1+off,(temp4),1, BLACK); //Triangulo arriba
					drawHollowTriangle((temp3-1),(121+60-temp1-1)+off,temp4,1, BLACK);//T derecha
					drawHollowTriangle((temp2+1),(121+60-temp1-1)+off,temp4,1, BLACK);//T izq
				}//Fin for 2
			}//Fin for 1
		} //Fin if principal
	} //Fin for principal
  while(*actual_state == valid_state){
	  delay_us(1);
  }
}
