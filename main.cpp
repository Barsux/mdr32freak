#include "base.h"
#include "core.h"
#include "l2_transport_mdr32.h"
#include "packetizer_mdr32.h"
#include "uart_mdr32.h"

#pragma argsused

#ifdef __MDR32F9Qx__

void cpu_init()
{
	//Разрешение работы PCLK
	RST_CLK_PCLKcmd (RST_CLK_PCLK_BKP, ENABLE);
	//Включить HSE
	RST_CLK_HSEconfig (RST_CLK_HSE_ON); 
	//Дождаться включения
	while (RST_CLK_HSEstatus () != SUCCESS);
	//Множитель частоты процессора(максимальный 13*16 = 208Мгц)
	RST_CLK_CPU_PLLconfig (RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul13);
	//Включить PLL
	RST_CLK_CPU_PLLcmd (ENABLE);
	//Дождаться включения PLL
	while (RST_CLK_CPU_PLLstatus () != SUCCESS);
	//Задать делитель
	RST_CLK_CPUclkPrescaler (RST_CLK_CPUclkDIV1);
	//Перейти на PLL
	RST_CLK_CPU_PLLuse (ENABLE); 
	RST_CLK_CPUclkSelection (RST_CLK_CPUclkCPU_C3);
	//Обновить частоту в регистре
	SystemCoreClockUpdate();
}


void init(){
	//EN - D14                  PPS - D15
	//Разрешить работу PORTC PORTB и UART1, 2
	MDR_RST_CLK->PER_CLOCK 	|= (1 << 6) | (1 << 23) | (1 << 24);
	//Выбрать цифровой тип порта
	MDR_PORTD->ANALOG 		|= ((1<<7) | (1<<8) | (1<<15));
	MDR_PORTC->ANALOG 		|= (1<<6);
	//Выбрать вывод
	MDR_PORTC->OE 			|= (1<<6);
	MDR_PORTD->OE 			|= ((1<<7) | (1<<8) | (0<<15));
	//Разрешить питание
	MDR_PORTD->PWR 			|= ((0x1 << PORT_PWR7_Pos) | (0x1 << PORT_PWR8_Pos));
	MDR_PORTC->PWR			|= (0x1 << PORT_PWR6_Pos);
	cpu_init();
	uart_init();
	MDR_PORTC->RXTX 		|= (1<<6);
	MDR_PORTD->RXTX 		|= ((1<<7) | (1<<8));
}
#else
void init(){}
#endif


int main()
{
	char source_mac[] = "72:27:72:AA:BB:CC";
	char source_ip[] = "192.168.1.12";
	init();
	PRINT("INIT");
	WaitSystem* waitSystem = new_WaitSystem();

	Core::Setup coreSetup;
	Core* core = new_Core(waitSystem, coreSetup);
	L2Transport::Setup l2Transport_setup;
	l2Transport_setup.srcMAC = source_mac;
	L2Transport* l2Transport = new_L2Transport(waitSystem, l2Transport_setup);
	
	UART* UART_mdr32 = new_UART(waitSystem);
	
	Packetizer::Setup packetizer_setup;
	packetizer_setup.srcMAC = source_mac;
	packetizer_setup.ip4 = source_ip;
	Packetizer* packetizer = new_Packetizer(waitSystem, packetizer_setup);
	packetizer->attach_l2_transport(l2Transport->rx, l2Transport->tx, l2Transport->sent);
 
	core->attach_packetizer(packetizer->rx, packetizer->tx, packetizer->sent);

	waitSystem->run();
}


