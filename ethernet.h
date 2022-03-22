#ifndef ETHERNETH
#define ETHERNETH
#include "tcpip.h"
#include "base.h"

int eth_init(MAC &srcMAC);
void debug_eval(MDR_ETHERNET_TypeDef * ETHERNETx);
void debug_send(MDR_ETHERNET_TypeDef * ETHERNETx, MAC src, MAC dst);
void create_packet(MAC src, MAC dst);
struct ethheader{
		
    U8 h_dest[6];
    U8 h_source[6];
    U16 h_proto;
};




#endif
