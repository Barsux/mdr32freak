#include "base.h"
#include "core.h"

#pragma argsused

#ifdef __MDR32F9Qx__

void cpu_init()
{
	RST_CLK_PCLKcmd (RST_CLK_PCLK_BKP, ENABLE);
  RST_CLK_HSEconfig (RST_CLK_HSE_ON); 
  while (RST_CLK_HSEstatus () != SUCCESS);
  RST_CLK_CPU_PLLconfig (RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul9); 
  RST_CLK_CPU_PLLcmd (ENABLE); 
  while (RST_CLK_CPU_PLLstatus () != SUCCESS);
  RST_CLK_CPUclkPrescaler (RST_CLK_CPUclkDIV1); 
  RST_CLK_CPU_PLLuse (ENABLE); 
  RST_CLK_CPUclkSelection (RST_CLK_CPUclkCPU_C3); 
	SystemCoreClockUpdate();
}


void init(){
	MDR_RST_CLK->PER_CLOCK 	|= (1 << 6) | (1 << 23) | (1 << 24);
	MDR_PORTD->ANALOG 			|= (1<<7);
	MDR_PORTD->OE 					|= (1<<7);
	MDR_PORTD->PWR 					|= (0x1 << PORT_PWR7_Pos);
	MDR_PORTD->PWR 					|= (0x1 << PORT_PWR8_Pos);
	MDR_PORTD->RXTX 				|= (1<<8);
	cpu_init();
	
	uart_init();
	MDR_PORTD->RXTX 				|= (1<<7);
}
#else
void init(){}
#endif
int main()
{
	init();
  WaitSystem* waitSystem = new_WaitSystem();
  Core::Setup coreSetup;
  Core* core = new_Core(waitSystem, coreSetup);

  //L2Transport::Setup l2Transport_setup;
  //l2Transport_setup.physicalId = "eth0";
  //L2Transport* l2Transport = new_L2Transport(waitSystem, l2Transport_setup);
  //core->attach_l2_transport(l2Transport->rx, l2Transport->tx, l2Transport->sent);
  waitSystem->run();
}


