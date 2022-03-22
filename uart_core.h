#ifndef UARTCOREH
#define UARTCOREH
#define PRINT(...) sendstr(__VA_ARGS__)
#include "base.h"

void uart_init();
void uart_evaluate();
int sendstr(char * fmt, ...);

class UART {public:
  virtual ~UART() {}
  class Setup {public:
    char* physicalId;
    Setup(): physicalId() {}
  };
	enum Uarts {
      UART1 = 0x01,
      UART2 = 0x02,
    } uarts;
  class Queue_rx: public WaitSystem::Queue {public:
		Uarts source;
		U8 rxlen;
    U8 rxbuf[256];
  }* rx;
  class Queue_tx: public WaitSystem::Queue {public:
    U8 txbuf;
  }* tx;
};

#endif