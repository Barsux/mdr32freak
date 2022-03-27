#ifndef l2_transportH
#define l2_transportH

#include "base.h"

class L2Transport {public:
  virtual ~L2Transport() {}
  class Setup {public:
    char* physicalId;
    Setup(): physicalId() {}
  };
  class Queue_rx: public WaitSystem::Queue {public:
    virtual int recv(void * dst, TsNs &utc_rx , int maxsize) = 0;
  }* rx;
  class Queue_tx: public WaitSystem::Queue {public:
    virtual int send(U8 * buffer, int seq, int size) = 0;
  }* tx;
  class Queue_sent: public WaitSystem::Queue {public:
    TsNs utc_sent;
  }* sent;
};
#endif