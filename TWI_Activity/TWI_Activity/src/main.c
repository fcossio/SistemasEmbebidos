
//Bernardo Urriza, Fernando Cossío, Antonio Rivera

#include <avr32/io.h>
#include "board.h"
#include "gpio.h"
#include "pm.h"
#include "twi.h"

#define LCD_ADDRESS				0x0A
#define LCD_ADDRESS_LENGTH		2
#define LCD_INTERNAL_ADDRESS	0x300
#define LCD_SPEED				200000

#define POT_ADDRESS				0x04
#define POT_SPEED				200000

#define  PATTERN_LENGTH (sizeof(pattern)/sizeof(U8))
const U8 pattern[] = {0x38,0x0C,0x01,0x06,0x83,0x0D}; //To send to LCD

int main(void){
	
  static const gpio_map_t TWI_GPIO_MAP = {
    {AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
    {AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}
  };//TWI_GPIO_MAP

  twi_options_t optLCD;
  twi_options_t optPOT;
  twi_package_t packet_sent_LCD, packet_received_POT;
  int statusLCD, statusPOT;
  U8 POT[1] = {0};

  pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);
  irq_initialize_vectors();
  cpu_irq_enable();
  gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));

  // Options settings LCD
  optLCD.pba_hz = FOSC0;
  optLCD.speed = LCD_SPEED;
  optLCD.chip = LCD_ADDRESS;
  statusLCD = twi_master_init(&AVR32_TWI, &optLCD);

  if (statusLCD==TWI_SUCCESS){
	  //LCD init successful
  }//Status

  // Options settings LCD
  optPOT.pba_hz = FOSC0;
  optPOT.speed = POT_SPEED;
  optPOT.chip = POT_ADDRESS;
  statusPOT = twi_master_init(&AVR32_TWI, &optPOT);

  if (statusPOT==TWI_SUCCESS){
	  //Pot init successful
  }//Status

  //LCD:

  packet_sent_LCD.chip = LCD_ADDRESS;
  packet_sent_LCD.addr[0] = LCD_INTERNAL_ADDRESS >> 16;
  packet_sent_LCD.addr[1] = LCD_INTERNAL_ADDRESS >> 8;
  packet_sent_LCD.addr[2] = LCD_INTERNAL_ADDRESS;
  packet_sent_LCD.addr_length = LCD_ADDRESS_LENGTH;
  packet_sent_LCD.buffer = (void*) pattern;
  packet_sent_LCD.length = PATTERN_LENGTH;

  statusLCD = twi_master_write(&AVR32_TWI, &packet_sent_LCD);

  if ((statusLCD==TWI_SUCCESS)){
	  //LCD write successful
  }//Status
  
  //Potentiometer:

  packet_received_POT.chip = POT_ADDRESS;
  packet_received_POT.addr_length = 0;
  packet_received_POT.length = 1;//Un dato
  packet_received_POT.addr[0] = 0;
  packet_received_POT.addr[1] = 0;
  packet_received_POT.addr[2] = 0;
  packet_received_POT.buffer = POT;

  statusPOT = twi_master_read(&AVR32_TWI, &POT);

  if ((statusPOT==TWI_SUCCESS)){
	  //Pot read successful
  }//Status
	  
}//Main
