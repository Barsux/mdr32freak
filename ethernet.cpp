#include "ethernet.h"
#define PRINT(...) sendstr(__VA_ARGS__)
#define U16SWAP(digit) ((digit << 8)| (digit >> 8))

int eth_init(MAC &srcMAC){
	ETH_ClockDeInit();
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);
	RST_CLK_PCLKcmd(RST_CLK_PCLK_DMA, ENABLE); // Dma here now, idk.
	

	RST_CLK_HSE2config(RST_CLK_HSE2_ON);
	if(RST_CLK_HSE2status() == ERROR) return -1;
	
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);
	ETH_BRGInit(ETH_HCLKdiv1);
	ETH_ClockCMD(ETH_CLK1, ENABLE);
	ETH_DeInit(MDR_ETHERNET1);
	
	
	ETH_InitTypeDef  ETH_InitStruct;
	ETH_StructInit((ETH_InitTypeDef * ) &ETH_InitStruct);
	
	ETH_InitStruct.ETH_PHY_Mode = ETH_PHY_MODE_AutoNegotiation;
	ETH_InitStruct.ETH_Transmitter_RST = SET;
	ETH_InitStruct.ETH_Receiver_RST = SET;
	
	ETH_InitStruct.ETH_Buffer_Mode = ETH_BUFFER_MODE_LINEAR;
	ETH_InitStruct.ETH_Source_Addr_HASH_Filter = DISABLE;

	ETH_InitStruct.ETH_MAC_Address[2] = ((int)srcMAC[0]<<8)| (int)srcMAC[1];
	ETH_InitStruct.ETH_MAC_Address[1] = ((int)srcMAC[2]<<8)| (int)srcMAC[3];
	ETH_InitStruct.ETH_MAC_Address[0] = ((int)srcMAC[4]<<8)| (int)srcMAC[5];
	MDR_ETHERNET1->PHY_Status |= 0 << 1;
	ETH_InitStruct.ETH_Dilimiter = 0x5DC;
	
	ETH_Init(MDR_ETHERNET1, (ETH_InitTypeDef *) &ETH_InitStruct);
	ETH_PHYCmd(MDR_ETHERNET1, ENABLE);
	ETH_Start(MDR_ETHERNET1);
	return 1;
}

void sendto(U32 * packet, U32 * size){
	ETH_SendFrame(MDR_ETHERNET1, packet, *(U32*)&packet[0]);
}

U16 recvto(U32 * packet, TsNs &UTC_Recv){
	volatile ETH_StatusPacketReceptionTypeDef ETH_StatusPacketReceptionStruct;
	if(MDR_ETHERNET1->ETH_R_Head != MDR_ETHERNET1->ETH_R_Tail) {
		UTC_Recv.renew();
		ETH_StatusPacketReceptionStruct.Status = ETH_ReceivedFrame(MDR_ETHERNET1, packet);
		return (U16)(MDR_ETHERNET1->ETH_R_Head - MDR_ETHERNET1->ETH_R_Tail);
	}
	return 0; 
}