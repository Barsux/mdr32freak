#ifndef UARTH
#define UARTH
#define PRINT(...) sendstr(__VA_ARGS__)

void uart_init();
void uart_evaluate();
int sendstr(char * fmt, ...);
#endif

#ifndef l2_transportH
#define l2_transportH

#include "base.h"

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