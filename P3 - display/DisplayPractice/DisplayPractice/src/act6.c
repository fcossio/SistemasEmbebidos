#include "et024006dhu.h"

void verticalPrint(char *str,size_t size, uint16_t x, et024006_color_t color, uint8_t space_rows);
void addToArr(char * str, char add, size_t size);

void act6(uint8_t valid_state, uint8_t * actual_state){//Video test screen

	#define COLS 11  //Numero de columnas a mostrar
	#define SPACEC 25 //Espacio entre columnas
	#define ROWS 20 //Numero de caracteres verticalmente
	#define SPACER 13 //Espacio entre filas

	et024006_DrawFilledRect(0 , 0, 320, 240, BLACK ); //fondo negro
	char* col;
	col = (char*)malloc(sizeof(char)*ROWS*COLS); //Asignar memoria
	char* colPointers[COLS];

	for (uint16_t i=0; i<COLS; i++){
		colPointers[i]=&col[i*ROWS];
	}//For

	while(*actual_state == valid_state){
		char matrix_char;
		for (uint16_t i=1; i<COLS+1 ;i++){
			if (rand()%2!=1){ //Espacios con mas probabilidad que caracteres
				matrix_char= ' ';
			}else{
				matrix_char = (rand()%(126-32))+32;
			}//If

			et024006_DrawFilledRect(i*SPACEC , 0, 30, ET024006_HEIGHT, BLACK );
			addToArr(colPointers[i-1],matrix_char,ROWS-1);

			if (rand()%12!=1){//Detalles en blanco con baja probabilidad
				verticalPrint(colPointers[i-1],ROWS-1,i*SPACEC,GREEN, SPACER);
			}else{
				verticalPrint(colPointers[i-1],ROWS-1,i*SPACEC,WHITE, SPACER);
			}//If
		}//For
	}//While
	free(col); //liberar memoria
}
void verticalPrint(char *str,size_t size, uint16_t x, et024006_color_t color, uint8_t space_rows){
	char str2[]={' ','\0'};
	uint16_t i=0;
	for (uint16_t y=0;y<230;y+=space_rows){
		str2[0]=str[i++];
		et024006_PrintString(str2, (const unsigned char *)&FONT8x8, x, y, color, -1);
		if (i>=size){break;}
	}//For
}//verticalPrint

void addToArr(char * str, char add, size_t size){
	for (uint16_t i=size;i>0;i--){
		str[i]=str[i-1];
	}//For
	str[0]=add;
}//addToArr
