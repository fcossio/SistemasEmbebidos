#include <pm.h>

pm_switch_to_osc0(&AVR32_PM, 12000000, 0);
delay_init(12000000);

void Inicializa_PLL(uint8_t mul){
	pm_switch_to_osc0(&AVR32_PM, 12000000,3);
	pm_pll_disable(&AVR32_PM,0);
	pm_pll_setup(&AVR32_PM,0, mul, 1,0,16); //pll0, mul variable, div = 1, 16 lockcount
	pm_pll_set_option(&AVR32_PM,0,1,0,0);  //pll0, 80-180, no divide/2, start normal
	pm_pll_enable(&AVR32_PM,0);
	pm_wait_for_pll0_locked(&AVR32_PM);
	pm_switch_to_clock(&AVR32_PM,2);//PLL como MC
	flashc_set_wait_state(1);
}//Inicializa_PLL
