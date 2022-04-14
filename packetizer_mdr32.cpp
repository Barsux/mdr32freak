#include "packetizer_mdr32.h"


class PacketizerObject: public WaitSystem::Module, public Packetizer {
	Packetizer::Setup &setup;
public:
    struct pckt packet;
    U8 ready_packet[2048];
    bool have_ready_packet;
    bool setted;
    bool have_values;
		U32 ip4;
    I4 total;
    L2Transport::Queue_rx*     l2_transport_rx;
    L2Transport::Queue_tx*     l2_transport_tx;
    L2Transport::Queue_sent*  l2_transport_sent;


    class Rx: public Queue_prx {public:
        PacketizerObject &base;
        Rx(PacketizerObject &base): base(base){}
        int recv(int &seq, TsNs * tstmp){
            return base.recv(seq, tstmp);
        }
    } prx;
    class Tx: public Queue_ptx {public:
        PacketizerObject &base;
        Tx(PacketizerObject &base): base(base){}
        int send(int seq){
            return base.send(seq);
        }
    } ptx;
    class Sent: public Queue_psent {public:
        PacketizerObject &base;
        Sent(PacketizerObject &base): base(base){}

    } psent;

    U2 IPCHECK(U2 *addr, U4 count) {
        register U32 sum = 0;
        while (count > 1) {
            sum += * addr++;
            count -= 2;
        }
        if(count > 0) sum += ((*addr)&htons(0xFF00));
        while (sum>>16) sum = (sum & 0xffff) + (sum >> 16);
        sum = ~sum;
        return ((U2)sum);
    }

    U2 CRC8(U8 *pcBlock, U2 len)
    {
        U2 crc = 0xFF;
        while (len--)
            crc = CRC8TABLE[crc ^ *pcBlock++];
        return crc;
    }

    void attach_l2_transport(L2Transport::Queue_rx* rx, L2Transport::Queue_tx* tx, L2Transport::Queue_sent* sent) {
        disable_wait(l2_transport_rx); disable_wait(l2_transport_tx); disable_wait(l2_transport_sent);
        l2_transport_rx = rx; l2_transport_tx = tx; l2_transport_sent = sent;
        enable_wait(l2_transport_rx);
        enable_wait(l2_transport_sent);
        enable_wait(l2_transport_tx);
    }


    PacketizerObject(WaitSystem* waitSystem, Packetizer::Setup &setup): WaitSystem::Module(waitSystem)
            , setup(setup), setted(false), have_values(false), prx(*this), ptx(*this), psent(*this)
    {
				awaited = false;
        module_debug = "PACKETIZER";
        rx = &prx;
        tx = &ptx;
        sent = &psent;
        enable_wait(tx);
        enable_wait(rx);
        memset(ready_packet, 0, 2048);
        total = sizeof(struct ethheader) + sizeof(struct ipheader) + sizeof(struct udpheader);
        flags = (Flags)(flags | evaluate_every_cycle);
				str2ip4(setup.ip4, ip4);
    }
    void create_packet(){
        U8 buffer[packet.size + SPACER];
				buffer[0] = packet.size;
        memset(buffer, 0, packet.size + SPACER);
        struct ethheader *eth = (struct ethheader *)(buffer + SPACER);
        for(int i = 0; i < ETH_ALEN; i++){
            eth->h_source[i] = packet.srcMAC[i];
            eth->h_dest[i] = packet.dstMAC[i];
        }
        eth->h_proto = htons(0x0800);

        struct ipheader *iphdr = (struct ipheader*)(buffer + SPACER + sizeof(struct ethheader));
        iphdr->ihl = 5;
        iphdr->version = 4;
        iphdr->tos=16;
        iphdr->id = htons(10241);
        iphdr->ttl = 64;
        iphdr->protocol = IPPROTO_UDP;
        iphdr->saddr = packet.srcIP;
        iphdr->daddr = packet.dstIP;
        iphdr->check = 0;

        struct udpheader *udp = (struct udpheader *)(buffer + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader));
        udp->source = htons(packet.srcPORT);
        udp->dest = htons(packet.dstPORT);
        udp->check = 0;

        udp->len = htons((packet.size- sizeof(struct ipheader) - sizeof(struct ethheader)));
        iphdr->tot_len = htons(packet.size - sizeof(struct ethheader));
        iphdr->check = IPCHECK((U2 *)iphdr, iphdr->ihl<<2);
        int i = SPACER + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader) + sizeof(rttheader);
        for (i; i < packet.size; i++){
            buffer[i] = i % 10 + 48;
        }
        for(int j = 0; j < packet.size; j++){
            ready_packet[j] = buffer[j];
        }
        have_ready_packet = true;
    }


    int send(int seq){
        if(!have_ready_packet)create_packet();
        struct rttheader *rtt = (struct rttheader *)(ready_packet + SPACER + sizeof(struct ipheader) + sizeof(struct ethheader) + sizeof(struct udpheader));
        rtt->size = packet.size;
        rtt->sequence = seq;
        rtt->CRC = 0;
        rtt->CRC = CRC8(ready_packet + total, packet.size - total);

				U32 * size = (U32*)&packet.size; 
        short status = l2_transport_tx->send(ready_packet, seq, size) ;
        return status>=0? status: -1;
    }

    int recv(int &seq, TsNs * tstmp){
        int MAXSIZE = 2048;
        int status;
        U8 buffer[MAXSIZE];
        bool valid_buffer = false;

        status = l2_transport_rx->recv(buffer, tstmp, MAXSIZE); 
				if(status < 0) return -1;
        U16 ip_proto = htons(0x0800);
			
			
        struct ethheader *eth = (struct ethheader *)(buffer);
        if(eth->h_proto != ip_proto) return -1;
			
        struct ipheader *ip = (struct ipheader *) (buffer + sizeof(struct ethheader));
        if(ip->protocol != IPPROTO_UDP || ip->daddr != ip4) return -1;
			
        struct udpheader *uh = (struct udpheader *)(buffer +sizeof(ethheader) + sizeof(ipheader));
        if(ntohs(uh->source) != 5850 || ntohs(uh->dest) != 5850) return -1;
			
        struct rttheader *rtt = (struct rttheader *)(buffer + sizeof(ethheader) + sizeof(ipheader) + sizeof(udpheader));
        seq = rtt->sequence;
        U2 inCRC = rtt->CRC;
        rtt->CRC = 0;
        U2 upCRC = CRC8(buffer + total, packet.size - total);
        if(!have_values && packet.is_server){
					packet.size = rtt->size;
          memcpy(packet.srcMAC, eth->h_dest, ETH_ALEN);
          memcpy(packet.dstMAC, eth->h_source, ETH_ALEN);
          packet.srcIP = ip->daddr;
          packet.dstIP = ip->saddr;
        }
        if(inCRC == upCRC) return 1;
				return -1;
}
   
		void check(){}

    void evaluate(){
        while (WaitSystem::Queue* queue = enum_ready_queues()){
            if(queue == l2_transport_tx && setted){
                tx->setReady();
            }
            else if(queue == l2_transport_rx && setted){
                rx->setReady();
            }
            else if(queue == rx && !setted){
                packet = rx->packet;
                rx->clear();
                setted = true;
            }
            else if(queue == tx){
                enable_wait(l2_transport_tx);
            }
            else if(queue == l2_transport_sent){
                l2_transport_sent->clear();
                sent->utc_sent = l2_transport_sent->utc_sent;
                sent->sequence = l2_transport_sent->sequence;
                sent->setReady();
            }
        }

    }
};
Packetizer* new_Packetizer(WaitSystem* waitSystem, Packetizer::Setup &setup){
    return new PacketizerObject(waitSystem, setup);
}