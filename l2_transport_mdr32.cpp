#include "l2_transport_mdr32.h"


class L2Transport_mdr32: public WaitSystem::Module, public L2Transport {
  L2Transport::Setup &setup;
public:
  int fd, sequence;
	bool sended = false;
  class Rx: public Queue_rx {public:
    L2Transport_mdr32 &base;
    Rx(L2Transport_mdr32 &base): base(base) {
    }
      int recv(void * dst, TsNs &utc_rx , int maxsize) {
			return base.recv(dst, utc_rx, maxsize);
    }
  } queue_rx;
  class Tx: public Queue_tx {public:
    L2Transport_mdr32 &base;
    Tx(L2Transport_mdr32 &base): base(base) {
    }
			int send(U8 * buffer, int seq, int size) {
			return base.send(buffer);
    }
  } queue_tx;
  Queue_sent queue_sent;
  L2Transport_mdr32(WaitSystem* waitSystem, L2Transport::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), fd(), queue_rx(*this), queue_tx(*this)
  {
		awaited = true;
    module_debug = "ETH";
    rx = &queue_rx; sent = &queue_sent;
    tx = &queue_tx; enable_wait(tx);
    flags = (Flags)(flags | evaluate_every_cycle);
  }
	
	int recv(void * dst, TsNs &utc_rx, int maxsize){
		return recvto((U32*)dst, utc_rx);
	}
	
	int send(void * buffer){
		sendto((U32*)buffer);
		sended = true;
		return 1;
	}
	void check(){
		volatile U16 STAT = MDR_ETHERNET1->ETH_STAT;
		if(!((STAT << ETH_STAT_R_EMPTY_Pos) & 1))rx->setReady();
		if(!((STAT << ETH_STAT_X_EMPTY_Pos) & 1)){
			if(sended){
				sended = false;
				sent->utc_sent = TsNs();
				sent->setReady();
			}
			tx->setReady();
		}
		
	}
	void evaluate(){
		
	}
};
L2Transport* new_L2Transport(WaitSystem* waitSystem, L2Transport::Setup &setup) {
  return new L2Transport_mdr32(waitSystem, setup);
}

