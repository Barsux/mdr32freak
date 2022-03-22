#ifndef l2_transport_mdr32
#define l2_transport_mdr32

#include "l2_transport.h"
#include "ethernet.h"

L2Transport* new_L2Transport(WaitSystem* waitSystem, L2Transport::Setup &setup);
#endif