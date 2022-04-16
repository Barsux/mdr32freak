#include "core.h"

class CoreObject: public WaitSystem::Module, public Core {public:
  Core::Setup &setup;
  Packetizer::Queue_prx*        packetizer_rx;
  Packetizer::Queue_ptx*        packetizer_tx;
  Packetizer::Queue_psent*    packetizer_sent;
	volatile U16 counter = 0;
	volatile I4 seq;
	bool __init = false, can_send = false;
  CoreObject(WaitSystem* waitSystem, Core::Setup &setup): WaitSystem::Module(waitSystem)
    , setup(setup), packetizer_tx(), packetizer_rx(), packetizer_sent()
  {
		awaited = false;
    module_debug = "CORE";
		flags = (Flags)(flags | evaluate_every_cycle);
  }
  WaitSystem::Queue timer;
	
	void attach_packetizer(Packetizer::Queue_prx* rx, Packetizer::Queue_ptx* tx, Packetizer::Queue_psent* sent){
        disable_wait(packetizer_tx); disable_wait(packetizer_rx); disable_wait(packetizer_sent);
        packetizer_tx = tx; enable_wait(packetizer_rx);
        packetizer_rx = rx; enable_wait(packetizer_tx);
        packetizer_sent = sent; enable_wait(packetizer_sent);
  }
	
	void init();
	void check(){}
  void evaluate() {
		while (WaitSystem::Queue* queue = enum_ready_queues()){
		packetizer_tx->setReady();
		if(queue == packetizer_tx) {
								PRINT("PTX");
                can_send = true;
                packetizer_tx->clear();
     }
    if(queue == packetizer_rx){
								PRINT("PRX");
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
}};


Core* new_Core(WaitSystem* waitSystem, Core::Setup &setup) {
  return new CoreObject(waitSystem, setup);
}

