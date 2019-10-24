
//Directivas
#include "board.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "et024006dhu.h"
#include "delay.h"
#include "avr32_logo.h"
#include "conf_clock.h"
#include "pwm.h"
#define RGB(r,g,b) r<<11|g<<5|b

//Funciones
void Chess_1(void);
void ChessSize_2(void);
void RectangleMove_3(void);
void RandomPixel_4(void);
void ChessColor_5(void);
void Matrix_6(void);
void TVBars_7(void);
void Mondrian_8(void);
void Perspective_11(void);
void Fibonacci_13(void);
void TriangleIllusion_12(void);
void ColorTriangles_19(void);

static void tft_bl_init(void);
void verticalPrint(char *str,size_t size, uint16_t x, et024006_color_t color, uint8_t space_rows); 
void scrollVertical(char* str,size_t size, uint16_t x);
void addToArr(char * str, char add, size_t size);
uint32_t Debounce( uint32_t GPIO_PIN );
void CLR_disp(void);
void drawHollowTriangle(int centerx, int centery, int radio, int dir, int color);
void drawFilledTriangle(int centerx, int centery, int radio, int dir, int color);
__attribute__ ((__interrupt__)) void teclas(void);

//Variables Globales
volatile uint16_t RectX = 160;
volatile uint16_t RectY = 120;
volatile uint32_t enter= 1;
volatile uint16_t actividad = 0;
int l,b,p1x,p1y,p2x,p2y,p3x,p3y; //Para triangulos
avr32_pwm_channel_t pwm_channel6 = {.cdty = 0,.cprd = 100};
	
int main(void){
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	et024006_Init( FOSC0, FOSC0 );
	tft_bl_init();
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );

	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&teclas, 71, 0); //por formula sale el 71
	INTC_register_interrupt(&teclas, 70,0);  //por formula sale el 70
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_LEFT,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_RIGHT,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_ENTER,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_UP,GPIO_RISING_EDGE);
	gpio_enable_pin_interrupt(QT1081_TOUCH_SENSOR_DOWN,GPIO_RISING_EDGE);
	Enable_global_interrupt();
	
	while(pwm_channel6.cdty < pwm_channel6.cprd){
		pwm_channel6.cdty++;
		pwm_channel6.cupd = pwm_channel6.cdty;
		pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
		delay_ms(10);
	}//PWM

	while(1){
	switch (actividad){
		case 0:
		RectangleMove_3();
		actividad++;
		break;

		case 1:
		Matrix_6();
		actividad++;
		break;

		case 2:
		CLR_disp();
		Mondrian_8();
		actividad++;
		break;

		case 3:
		CLR_disp();
		Chess_1();
		actividad++;
		break;

		case 4:
		Perspective_11();
		actividad++;
		break;

		case 5:
		CLR_disp();
		TVBars_7();
		actividad++;
		break;

		case 6:
		CLR_disp();
		RandomPixel_4();
		actividad++;
		break;

		case 7:
		ColorTriangles_19();
		actividad++;
		break;

		case 8:
		ChessSize_2();
		actividad++;
		break;

		case 9:
		ChessColor_5();
		actividad++;
		break;
		
		case 10:
		TriangleIllusion_12();
		actividad++;
		break;
		
		case 11:
		CLR_disp();
		Fibonacci_13();
		actividad=0;
		break;
		
		}//Switch
	}//While Actividad

	while(true);
}//Main

//funciones generales

static void tft_bl_init(void){
  pwm_opt_t opt = {.diva = 0,.divb = 0,.prea = 0,.preb = 0};
  pwm_init(&opt);
  pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
  pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
  pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
  pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;
  pwm_channel_init(6, &pwm_channel6);
  pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);
}//tft_bl_init

void CLR_disp(void){
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
}

void teclas(void){//handler teclas left, right o center
	if (gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_RIGHT)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_RIGHT);
		if (actividad==0){
			while(gpio_get_pin_value(QT1081_TOUCH_SENSOR_RIGHT)){
				et024006_DrawFilledRect(RectX , RectY, 10, 10, WHITE);//borra la posicion pasada
				RectX+=2;
				et024006_DrawFilledRect(RectX , RectY, 10, 10, BLACK);
				delay_ms(10);
			}//While
		}//If
	}//RIGHT
	if (gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_LEFT)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_LEFT);
		if (actividad==0){
			while(gpio_get_pin_value(QT1081_TOUCH_SENSOR_LEFT)){
				et024006_DrawFilledRect(RectX , RectY, 10, 10, WHITE);//borra la posicion pasada
				RectX-=2;
				et024006_DrawFilledRect(RectX , RectY, 10, 10, BLACK);
				delay_ms(10);
			}//While
		}//IF
	}//LEFT
	if (gpio_get_pin_interrupt_flag(QT1081_TOUCH_SENSOR_DOWN)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_DOWN);
		if (actividad==0){
			while(gpio_get_pin_value(QT1081_TOUCH_SENSOR_DOWN)){
				et024006_DrawFilledRect(RectX , RectY, 10, 10, WHITE);//borra la posicion pasada
				RectY+=2;
				et024006_DrawFilledRect(RectX , RectY, 10, 10, BLACK);
				delay_ms(10);
			}//While
		}//IF
	}//DOWN
	if (gpio_get_pin_interrupt_flag(QT1081_TOUCH_SENSOR_UP)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_UP);
		if (actividad==0){
			while(gpio_get_pin_value(QT1081_TOUCH_SENSOR_UP)){
				et024006_DrawFilledRect(RectX , RectY, 10, 10, WHITE);//borra la posicion pasada
				RectY-=2;
				et024006_DrawFilledRect(RectX , RectY, 10, 10, BLACK);
				delay_ms(10);
			}//While
		}//If
	}//UP
	if (gpio_get_pin_interrupt_flag (QT1081_TOUCH_SENSOR_ENTER)){
		gpio_clear_pin_interrupt_flag(QT1081_TOUCH_SENSOR_ENTER);
		enter=0;
	}//IF
}//Teclas

uint32_t Debounce( uint32_t GPIO_PIN ){//regresar se presiono el boton o no
	if(gpio_get_pin_value(GPIO_PIN)==1){// se presiono el boton?, sino salir de la funcion
		delay_ms(10);
		if (gpio_get_pin_value(GPIO_PIN)==0){//Si ya se libero, es ruido, salir sin hacer nada
			goto salir;
		}//If
		espera://espera a que suelte el botón
		while (gpio_get_pin_value(GPIO_PIN)==1){}
		delay_ms(10);
		if (gpio_get_pin_value(GPIO_PIN)==1) {//si ya lo presiono otra vez , es ruido, regresa a esperar
			goto espera;
		}//If
		return 1;//debounce completo regresa 1
	}//If
	salir:
	return 0;
}//Debounce

//Funciones de actividades

void RectangleMove_3(void){
	enter=1;
	et024006_DrawFilledRect(0 , 0, 320, 240, WHITE);
	et024006_DrawFilledRect(RectX , RectY, 10, 10, BLACK);
	while(enter);//interrupcion de enter saca de aqui
}//RectangleMove_3

void Matrix_6(void){
	
	#define COLS 11  //Numero de columnas a mostrar
	#define SPACEC 25 //Espacio entre columnas
	#define ROWS 20 //Numero de caracteres verticalmente
	#define SPACER 13 //Espacio entre filas

	et024006_DrawFilledRect(0 , 0, 320, 240, BLACK ); //fondo negro
	enter=1;
	char* col;
	col = (char*)malloc(sizeof(char)*ROWS*COLS); //Asignar memoria
	char* colPointers[COLS];

	for (uint16_t i=0; i<COLS; i++){
		colPointers[i]=&col[i*ROWS];
	}//For

	while(enter){
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
}//Matrix_6

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

void TriangleIllusion_12(void){ //(Necesita variables globales int l,b,p1x,p1y,p2x,p2y,p3x,p3y; //Para triangulos)
	
	enter = 1;
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE ); //Lienzo en Blanco	enter=1;
	int temp1, temp2, temp3,temp4,temp5;
	
	for (int i=1; i<=10; i++){
		drawHollowTriangle(160, 120, 6*i,0, BLACK); //T. invertido
		
		if (i == 10){
			temp1 = l; //l maxima
			temp2=p2x;
			temp3=p3x;
			temp4=2;
			temp5=0;

			for (int j=1; j<=10; j++){
				for (int y=0; y<=3-(j*0.3); y++){ // Y toma los valores: 3,3,3,2,2,2,1,1,1 (Grosor)
					temp4= (6*j)-y;
					drawHollowTriangle(160, 2*temp1,(temp4),1, BLACK); //Triangulo arriba
					drawHollowTriangle((temp3-1),(121+60-temp1-1),temp4,1, BLACK);//T derecha
					drawHollowTriangle((temp2+1),(121+60-temp1-1),temp4,1, BLACK);//T izq
				}//Fin for 2
			}//Fin for 1
		} //Fin if principal
	} //Fin for principal
	while(enter){}
}//TriangleIllusion_12

void drawHollowTriangle(int centerx, int centery, int radio, int dir, int color){ //dir=0 VOLTEADO ; dir=1 NORMAL
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

void ColorTriangles_19(void){
	CLR_disp(); //limplia el display
	enter=1;
	#define RADIO 30
	for(int y=RADIO; y<=(ET024006_HEIGHT); y+=1.5*RADIO){ //2*sin(60)
		for (int x=(-(y%2)*(0.8*RADIO)); x<=(ET024006_WIDTH+RADIO); x+=(0.866*RADIO)){ //sin(60)
			if(x%2==1){
				drawFilledTriangle(x,y,RADIO,1,RGB(rand()%31,rand()%63,rand()%31));//No invertido
			}else{
				drawFilledTriangle(x,y-RADIO/2,RADIO,0,RGB(rand()%31,rand()%63,rand()%31));//Invertido
			}//If
		}//For X
	}//For Y
	while (enter){
	}
}//ColorTriangles_19

void ChessSize_2(void){
	uint16_t size=1,j=1,k=1,rnd=0;
	et024006_DrawFilledRect(0 , 0, 320, 240, BLACK );
	enter=1;
	while(enter){
		if(Debounce(QT1081_TOUCH_SENSOR_UP)){
			size<<=1;
			size=(size>32)?(size>>1):size;
		}//If
		if(Debounce(QT1081_TOUCH_SENSOR_DOWN)){
			size>>=1;
			size=(size<1)?(1):size;
			et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
		}//If
		for( k=0; k<8; k++){
			for(j=0;j<8;j++){
				et024006_DrawFilledRect(k*size,j*size,size,size,(k+j)%2==0?BLACK:WHITE);
			}//For
		}//For
	}//While
}//ChessSize_2

void ChessColor_5(void){
	uint16_t size=1,j=1,k=1,rr,rg,rb;
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
	enter=1;
	while(enter){
		if(Debounce(QT1081_TOUCH_SENSOR_UP)){
			size<<=1;
			size=(size>32)?(size>>1):size;
		}//IF
		if(Debounce(QT1081_TOUCH_SENSOR_DOWN)){
			size>>=1;
			size=(size<1)?(1):size;
			et024006_DrawFilledRect(0 , 0, 320, 240, BLACK );
		}//If
		rr=rand();
		rg=rand();
		rb=rand();
		for( k=0; k<8; k++){
			for(j=0;j<8;j++){
				et024006_DrawFilledRect(k*size,j*size,size,size,(k+j)%2==0?RGB(rr%31,rg%63,rb%31):WHITE);
			}//For
		}//For
		delay_ms(1);
	}//While
}//ChessColor_5

void Chess_1(void){
	enter=1;
	U32 i,j,c;
	c=WHITE;
	for(i=0; i<8; i++){
		if(i%2)
		c=BLACK;
		else
		c=WHITE;
		for(j=0;j<8;j++){
			et024006_DrawFilledRect(j*30,i*30,30,30,c);
			if(c==BLACK)
			c=WHITE;
			else
			c=BLACK;
			delay_ms(10);
		}//For
	}//For
	while(enter){};
}//Chess_1

void RandomPixel_4(void){
	enter=1;
	while(enter){
		et024006_DrawPixel(rand()%ET024006_WIDTH,rand()%ET024006_HEIGHT,RGB(rand()%31, rand()%63 , rand()%31));
	}//While
}//RandomPixel_4

void TVBars_7(void){
	U32 i;
	enter=1;
	//Colores Arriba
	et024006_DrawFilledRect(0,0,45,150,WHITE);
	et024006_DrawFilledRect(45,0,45,150,RGB(31,63,0)); //amarillo
	et024006_DrawFilledRect(45*2,0,45,150,RGB(0,63,31));//azul 1
	et024006_DrawFilledRect(45*3,0,45,150,RGB(0,63,0));//verde
	et024006_DrawFilledRect(45*4,0,45,150,RGB(31,0,31));//violeta
	et024006_DrawFilledRect(45*5,0,45,150,RGB(31,0,0));//rojo
	et024006_DrawFilledRect(45*6,0,45,150,RGB(0,0,31));//azul 2
	//Colores Enmedio
	et024006_DrawFilledRect(0,150,45,20,RGB(0,0,31));//azul 2
	et024006_DrawFilledRect(45,150,45,20,RGB(31,0,31));//violeta
	et024006_DrawFilledRect(45*2,150,45,20,RGB(31,63,0)); //amarillo
	et024006_DrawFilledRect(45*3,150,45,20,RGB(31,0,0));//rojo
	et024006_DrawFilledRect(45*4,150,45,20,RGB(0,63,31));//azul 1
	et024006_DrawFilledRect(45*5,150,45,20,BLACK);
	et024006_DrawFilledRect(45*6,150,45,20,WHITE);

	for(i=0;i<190;i++){
		et024006_DrawVertLine(i,170,30,RGB(i*31/190,i*63/190,i*31/190));
	}//For
	for(i=0;i<10;i++) {
		et024006_DrawVertLine(i,170,30,RGB(31,i*6,0));//rojo a amarillo
	}//For
	for(i=0;i<10;i++) {
		et024006_DrawVertLine(i+10,170,30,RGB((10-i)*3,63,0));//amarillo a verde
	}//For
	for(i=0;i<10;i++) {
		et024006_DrawVertLine(i+20,170,30,RGB(0,63,i));//verde a azul1
	}//For
	for(i=0;i<10;i++) {
		et024006_DrawVertLine(i+30,170,30,RGB(0,(10-i)*6,31));//azul 1 a azul2
	}//For
	for(i=0;i<10;i++) {
		et024006_DrawVertLine(i+40,170,30,RGB(i*3,0,31));//azul 2 a violeta
	}//For
	for(i=0;i<12;i++){
		et024006_DrawFilledRect(i*15,200,15,40,RGB(i*31/12,i*63/12,i*31/12));//difuminado cuadros
	}//For
	et024006_DrawFilledRect(180,200,40,40,RGB(i*31/12,i*63/12,i*31/12));//cuadro negro
	while (enter){}
}//TVBars_7

void Perspective_11(void){
	CLR_disp();
	enter=1;
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
	while (enter){}
}//Perspective_11

void Mondrian_8(void){
	enter=1;
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, WHITE );
	et024006_DrawLine(1, 30, 300, 30, BLACK);
	et024006_DrawLine(1, 95, 25, 95, BLACK);
	et024006_DrawLine(200, 95, 300, 95, BLACK);
	et024006_DrawLine(25, 160, 300, 160, BLACK);
	et024006_DrawLine(1, 185, 319, 185, BLACK);
	et024006_DrawLine(25, 210, 200, 210, BLACK);
	et024006_DrawLine(100, 220, 300, 220, BLACK);
	//verticales
	et024006_DrawLine(25, 30, 25, 239, BLACK);
	et024006_DrawLine(100, 160, 100, 239, BLACK);
	et024006_DrawLine(70, 1, 70, 30, BLACK);
	et024006_DrawLine(200, 1, 200, 220, BLACK);
	et024006_DrawLine(250, 95, 250, 160, BLACK);
	et024006_DrawLine(300, 1, 300, 239, BLACK);
	//rectangulos
	et024006_DrawFilledRect( 26, 31, 174, 129, RED);
	et024006_DrawFilledRect( 26, 161, 74, 49, BLACK);
	et024006_DrawFilledRect( 1, 186, 23, 53, 0xFFE0); //YELLOW
	et024006_DrawFilledRect( 101, 211, 99, 9, BLACK);
	et024006_DrawFilledRect( 201, 1, 98, 28, 0xFFE0); //YELLOW
	et024006_DrawFilledRect( 201, 31, 98, 63, 0xFFE0);//YELLOW
	et024006_DrawFilledRect( 201, 186, 99, 34, BLUE);
	et024006_DrawFilledRect( 301, 186, 19, 54, RED);
	while (enter){}
}//Mondrian_8

void Fibonacci_13(void){
	enter=1;
	#define TFT_QUADRANT0   ((1 << 1) | (1 << 0))
	#define TFT_QUADRANT1   ((1 << 3) | (1 << 2))
	#define TFT_QUADRANT2   ((1 << 5) | (1 << 4))
	#define TFT_QUADRANT3   ((1 << 7) | (1 << 6))
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
	while (enter){}
}//Fibonacci_13
