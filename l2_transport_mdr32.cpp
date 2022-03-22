#include "l2_transport_mdr32.h"


class L2Transport_mdr32: public WaitSystem::Module, public L2Transport {
  L2Transport::Setup &setup;
public:
  int fd, sequence;
  class Rx: public Queue_rx {public:
    L2Transport_mdr32 &base;
    Rx(L2Transport_mdr32 &base): base(base) {
    }
      int recv(void * dst, TsNs &utc_rx , int maxsize) {
    }
  } queue_rx;
  class Tx: public Queue_tx {public:
    L2Transport_mdr32 &base;
    Tx(L2Transport_mdr32 &base): base(base) {
    }
    int send(U8 * buffer, int seq, int size) {
    }
  } queue_tx;
  Queue_sent queue_sent;
  //WaitSystem::Queue txPackets;
  L2Transport_mdr32(WaitSystem* waitSystem, L2Transport::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), fd(), queue_rx(*this), queue_tx(*this)
  {
    module_debug = "ETH";
    rx = &queue_rx; sent = &queue_sent;
    tx = &queue_tx; enable_wait(tx);
    flags |= evaluate_every_cycle;
  }
	
	int recv(void * dst, TsNs &utc_rx, int maxsize){
		erecv((U32 * )dst);
	}
	
	int send(void * buffer, int seq, int size){
		esend((U8 *) buffer);
	}

L2Transport* new_L2Transport(WaitSystem* waitSystem, L2Transport::Setup &setup) {
  return new L2Transport_mdr32(waitSystem, setup);
}
