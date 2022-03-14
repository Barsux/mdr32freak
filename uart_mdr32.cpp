#include "uart_mdr32.h"
#ifdef __MDR32F9Qx__

#endif

bool rxready;
bool txready;
U8 rx_buff[256];
U8 tx_buff[256];
U8 rx_nbuff = 0;
U8 tx_nbuff = 0;
U8 tx_pointer = 0;

//IRQ
extern "C" void UART1_IRQHandler(void)
{
	if(MDR_UART1->MIS & UART_IT_TX){
		if(tx_pointer != tx_nbuff){
			MDR_UART1->DR = tx_buff[tx_pointer];
			tx_pointer++;
		}
		else{
			memset(tx_buff, 0, tx_nbuff);
			tx_pointer = 0;
		}
		MDR_UART1->ICR |= UART_IT_TX;
	}
}

extern "C" void UART2_IRQHandler(void)
{
	if (MDR_UART2->MIS & UART_IT_RX)
	{
		U8 data = MDR_UART2->DR;
		if(data == 0x0D) {
			rxready = true;
			return;
		}
		tx_buff[tx_nbuff] = data;
		tx_nbuff++;
		MDR_UART2->ICR |= UART_IT_RX;
	}
}

//ENDIRQ

class UART_MDR32: public WaitSystem::Module, public UART {
public:
  bool init;
  class Rx: public Queue_rx {public:
    UART_MDR32 &base;
    Rx(UART_MDR32 &base): base(base) {
    }
  } queue_rx;
  class Tx: public Queue_tx {public:
    UART_MDR32 &base;
    Tx(UART_MDR32 &base): base(base) {
    }
  } queue_tx;
	
  UART_MDR32(WaitSystem* waitSystem): WaitSystem::Module(waitSystem)
		,init(false),queue_rx(*this), queue_tx(*this)
  {
    module_debug = "UART";
    rx 		= &queue_rx; 	memset(rx->rxbuf, 0, 256); 	rx->rxlen = 0;
    tx 		= &queue_tx; 	enable_wait(tx); 						tx->txbuf = 0;
    flags = (Flags)(flags | evaluate_every_cycle);
  }
	void uart1_rx()
	{}
	
	void uart_init(){
		txready = false;
		rxready = false;
		init = true;
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTC, ENABLE);
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);

		UART_InitTypeDef uart_init;
		uart_init.UART_BaudRate 						= 115200;
		uart_init.UART_WordLength 					= UART_WordLength8b;
		uart_init.UART_StopBits 						= UART_StopBits1;
		uart_init.UART_Parity 							= UART_Parity_0;
		uart_init.UART_FIFOMode 						= UART_FIFO_OFF;
		uart_init.UART_HardwareFlowControl 	= UART_HardwareFlowControl_TXE | UART_HardwareFlowControl_RXE;
		PORT_InitTypeDef uart_port;
		
		//UART1 INIT
		//UART1 RX INIT
		uart_port.PORT_Pin 				= PORT_Pin_3;
		uart_port.PORT_OE 				= PORT_OE_OUT;
		uart_port.PORT_PULL_UP 		= PORT_PULL_UP_OFF;
		uart_port.PORT_PULL_DOWN 	= PORT_PULL_DOWN_OFF;
		uart_port.PORT_PD_SHM 		= PORT_PD_SHM_OFF;
		uart_port.PORT_PD 				= PORT_PD_DRIVER;
		uart_port.PORT_GFEN 			= PORT_GFEN_OFF;
		uart_port.PORT_FUNC 			= PORT_FUNC_MAIN;
		uart_port.PORT_SPEED 			= PORT_SPEED_MAXFAST;
		uart_port.PORT_MODE				= PORT_MODE_DIGITAL;
		PORT_Init(MDR_PORTC, &uart_port);
		//END RX
		//UART1 TX INIT
		uart_port.PORT_Pin 				= PORT_Pin_4;
		uart_port.PORT_OE 				= PORT_OE_IN;
		PORT_Init(MDR_PORTC, &uart_port);
		//END TX
		RST_CLK_PCLKcmd(RST_CLK_PCLK_UART1, ENABLE);
		UART_BRGInit(MDR_UART1, UART_HCLKdiv1);
		UART_ITConfig(MDR_UART1, UART_IT_TX, ENABLE);
		NVIC_EnableIRQ(UART1_IRQn);
		UART_Init(MDR_UART1, &uart_init);
		//END
		
		//UART2 INIT
		//UART2 RX INIT
		uart_port.PORT_Pin 				= PORT_Pin_13;
		uart_port.PORT_OE 				= PORT_OE_IN;
		PORT_Init(MDR_PORTD, &uart_port);
		//END RX
		uart_init.UART_HardwareFlowControl	 = UART_HardwareFlowControl_RXE;
		RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);
		UART_BRGInit(MDR_UART2, UART_HCLKdiv1);
		UART_ITConfig(MDR_UART2, UART_IT_RX, ENABLE);
		NVIC_EnableIRQ(UART2_IRQn);
		UART_Init(MDR_UART2, &uart_init);
		//END
				
		
		UART_Cmd(MDR_UART1, ENABLE);
	
	}
	
  void evaluate() { 
    if(!init)uart_init();
  }
};

UART* new_UART(WaitSystem* waitSystem) {
  return new UART_MDR32(waitSystem);
}
