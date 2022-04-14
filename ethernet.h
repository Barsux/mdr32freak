#ifndef ETHERNETH
#define ETHERNETH
#include "base.h"

int eth_init(MAC &srcMAC);
void sendto(U32 * packet, U32 * size);
U16 recvto(U32 * packet, TsNs * UTC_Recv);

#endif
