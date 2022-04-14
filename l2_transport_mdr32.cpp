#include "l2_transport_mdr32.h"


class L2Transport_mdr32: public WaitSystem::Module, public L2Transport {
  L2Transport::Setup &setup;
public:
  int sequence;
	bool inited = false;
	bool sended = false;
  class Rx: public Queue_rx {public:
    L2Transport_mdr32 &base;
    Rx(L2Transport_mdr32 &base): base(base) {
    }
      int recv(void * dst, TsNs * utc_rx , int maxsize) {
			return base.recv(dst, utc_rx, maxsize);
    }
  } queue_rx;
  class Tx: public Queue_tx {public:
    L2Transport_mdr32 &base;
    Tx(L2Transport_mdr32 &base): base(base) {
    }
			int send(U8 * buffer, int seq, U32 * size) {
			return base.send(buffer, size);
    }
  } queue_tx;
  Queue_sent queue_sent;
  L2Transport_mdr32(WaitSystem* waitSystem, L2Transport::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), queue_rx(*this), queue_tx(*this)
  {
		awaited = true;
    module_debug = "ETH";
    rx = &queue_rx; sent = &queue_sent;
    tx = &queue_tx; enable_wait(tx);
    flags = (Flags)(flags | evaluate_every_cycle);
  }
	
	void init(){
		MAC mac;
		memset(mac, 0, 6);
		str2mac(mac, setup.srcMAC);
		eth_init(mac);
	}
	
	int recv(void * dst, TsNs * utc_rx, int maxsize){
		return recvto((U32*)dst, utc_rx);
	}
	
	int send(void * buffer, U32 * size){
		sendto((U32*)buffer, size);
		sended = true;
		return 1;
	}
	void check(){
		volatile U16 STAT = MDR_ETHERNET1->ETH_STAT;
		bool rempty = (bool((1 << 0)  &  STAT));
		bool xempty = (bool((1 << 8)  &  STAT));
		if((STAT >> 0) & 1u){
			rx->setReady();
			PRINT("REMPTY");
		}
		if((STAT >> 8) & 1u){
			PRINT("XEMPTY");
			if(sended){
				sended = false;
				sent->utc_sent = TsNs();
				sent->setReady();
			}
			tx->setReady();
		}
		
	}
	void evaluate(){
		if(!inited)init();
	}
};
L2Transport* new_L2Transport(WaitSystem* waitSystem, L2Transport::Setup &setup) {
  return new L2Transport_mdr32(waitSystem, setup);
}

