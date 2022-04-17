#include "l2_transport_mdr32.h"


class L2Transport_mdr32: public WaitSystem::Module, public L2Transport {
	L2Transport::Setup &setup;
public:
	int sequence;
	bool inited;
	bool sended;
	class Rx: public Queue_rx {public:
		L2Transport_mdr32 &base;
		Rx(L2Transport_mdr32 &base): base(base) {
    }
		int recv(void * dst, TsNs * utc_rx , int maxsize) {
			return base.recv(dst, utc_rx, maxsize);
		}
	}queue_rx;
	class Tx: public Queue_tx {public:
		L2Transport_mdr32 &base;
		Tx(L2Transport_mdr32 &base): base(base) {
    }
		int send(U8 * buffer, int seq, U32 * size) {
			return base.send(buffer, size);
		}
	}queue_tx;
	Queue_sent queue_sent;
	L2Transport_mdr32(WaitSystem* waitSystem, L2Transport::Setup &setup): WaitSystem::Module(waitSystem)
	, inited(false), sended(false), setup(setup), queue_rx(*this), queue_tx(*this)
	{
		awaited = true;
		module_debug = "ETH";
		rx = &queue_rx; sent = &queue_sent;
		tx = &queue_tx; enable_wait(tx);
		flags = (Flags)(flags | evaluate_every_cycle);
  }
	
	void inits(){
		MAC mac;
		memset(mac, 0, 6);
		str2mac(mac, setup.srcMAC);
		eth_init(mac);
		inited = true;
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
		bool RX_EMPTY = (bool)((STAT >> 0) & 1);
		bool TX_EMPTY = (bool)((STAT >> 8) & 1);
		if(!RX_EMPTY){
			MDR_PORTD->RXTX ^= (1<<8);
			RX_EMPTY = !RX_EMPTY;
			rx->setReady();
		}
		if(!TX_EMPTY){
			TX_EMPTY = !TX_EMPTY;
			if(sended){
				sended = false;
				sent->utc_sent = TsNs();
				sent->setReady();
			}
			tx->setReady();
		}
		
	}
	void evaluate(){
		if(!inited)inits();		
	}
};
L2Transport* new_L2Transport(WaitSystem* waitSystem, L2Transport::Setup &setup) {
  return new L2Transport_mdr32(waitSystem, setup);
}

