#ifndef coreH
#define coreH

#include "base.h"
#include "l2_transport.h"

class Core {public:
  virtual ~Core() {}
  class Setup {public:
  };
	/*
  virtual void attach_l2_transport(
    L2Transport::Queue_rx*   l2_transport_rx,
    L2Transport::Queue_tx*   l2_transport_tx,
    L2Transport::Queue_sent* sent
  ) = 0;
	*/
};

Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup);


#endif
