#ifndef PACKETIZER_H
#define PACKETIZER_H

#include "base.h"
#include "l2_transport_mdr32.h"

#define ETH_ALEN 6
#define IPPROTO_UDP 17
#define RTTHDR_LEN 8
#define UDPHDR_LEN 8
#define IPHDR_LEN 20
#define ETHHDR_LEN 14


class Packetizer{public:
    virtual ~Packetizer() {}
		class Setup {public:
			char* ip4;
			char* srcMAC;
			Setup(): ip4(), srcMAC() {}
		};
    class Queue_prx: public WaitSystem::Queue {public:
        struct pckt packet;
        virtual int recv(int &seq, TsNs * tstmp) = 0;
    }* rx;
    class Queue_ptx: public WaitSystem::Queue {public:
        virtual int send(int seq) = 0;
    }* tx;
    class Queue_psent: public WaitSystem::Queue {public:
        TsNs utc_sent;
        U32 sequence;
    }* sent;
		
    virtual void attach_l2_transport(
            L2Transport::Queue_rx*   l2_transport_rx,
            L2Transport::Queue_tx*   l2_transport_tx,
            L2Transport::Queue_sent* sent
    ) = 0;
};




#endif PACKETIZER_H