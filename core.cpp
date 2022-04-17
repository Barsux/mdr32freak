#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
	Core::Setup &setup;
	Packetizer::Queue_prx*		packetizer_rx;
	Packetizer::Queue_ptx*		packetizer_tx;
	Packetizer::Queue_psent*    packetizer_sent;
	volatile U16 counter = 0;
	volatile I4 seq;
	bool can_send = false;
	CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
		,setup(setup), packetizer_tx(), packetizer_rx(), packetizer_sent()
	{
		awaited = false;
		module_debug = "CORE";
		flags = (Flags)(flags | evaluate_every_cycle);
	}
	WaitSystem::Queue timer;
	
	void attach_packetizer(Packetizer::Queue_prx* rx, Packetizer::Queue_ptx* tx, Packetizer::Queue_psent* sent){
		disable_wait(packetizer_tx); disable_wait(packetizer_rx); disable_wait(packetizer_sent);
		packetizer_tx = tx; 
		packetizer_rx = rx; 
		packetizer_sent = sent; 
		enable_wait(packetizer_rx);
		enable_wait(packetizer_tx);
		enable_wait(packetizer_sent);
	}
	void init(){}
	void check(){}
	void evaluate() {
		while (WaitSystem::Queue* queue = enum_ready_queues()){
			if(queue == packetizer_tx) {
				packetizer_tx->clear();
				can_send = true;
			}
			else if(queue == packetizer_rx){
				packetizer_rx->clear();
				
				TsNs UTC = TsNs();
				I4 sequence; TsNs * ts = &UTC;
				I2 r = packetizer_rx->recv(sequence, ts);
				if(r > 0){
					seq = sequence;
					if(can_send){
						packetizer_tx->send(sequence);
					}
				}
			}
		}
	}
};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

