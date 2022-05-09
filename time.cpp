#include "time.h"
U32 global_high = 0;
U32 cntstamp = 0;
U16 divider = SystemCoreClock / 1000000;

void time_init(){
	TIMER_CntInitTypeDef timer_struct;
	RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1, ENABLE);
	TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv1 );
	timer_struct.TIMER_ARR_UpdateMode 	= TIMER_ARR_Update_Immediately;
	timer_struct.TIMER_BRK_Polarity 		= TIMER_BRKPolarity_NonInverted;
	timer_struct.TIMER_CounterDirection = TIMER_CntDir_Up;
	timer_struct.TIMER_CounterMode 			= TIMER_CntMode_ClkFixedDir;
	timer_struct.TIMER_ETR_FilterConf 	= TIMER_Filter_1FF_at_TIMER_CLK;
	timer_struct.TIMER_ETR_Polarity 		= TIMER_ETRPolarity_NonInverted;
	timer_struct.TIMER_ETR_Prescaler 		= TIMER_ETR_Prescaler_None;
	timer_struct.TIMER_EventSource 			= TIMER_EvSrc_None;
	timer_struct.TIMER_FilterSampling 	= TIMER_FDTS_TIMER_CLK_div_1;
	timer_struct.TIMER_IniCounter 			= 0;
	timer_struct.TIMER_Period 					= 0xFFFFFFFF;
	timer_struct.TIMER_Prescaler 				= 0;
	
	TIMER_CntInit(MDR_TIMER1, &timer_struct);
	
	TIMER_Cmd(MDR_TIMER1, ENABLE);
}

void check_overloading(){
	if(MDR_TIMER1->CNT <= cntstamp){
		global_high++;
	}
	cntstamp = MDR_TIMER1->CNT;
}


void set_utc(U64 time){
	time *= divider;
	global_high = (U32)(time >> 32);
	MDR_TIMER1->CNT = (volatile uint32_t)time;
}

TsNs::TsNs(){
	check_overloading();
	high = global_high;
	low = MDR_TIMER1->CNT;
}

TsNs::TsNs(U64 ticks){
	check_overloading();
	high = (U32)(ticks >> 32);
	low = (U32)(ticks);
}

void TsNs::renew(){
	check_overloading();
	high = global_high;
	low = MDR_TIMER1->CNT;
}

U64 TsNs::toUTC(){
	/*
	U64 UTE = (((uint64_t)high << 32) | ((uint64_t)low));
	PRINT("%llu", UTE);
	PRINT("%lu", SystemCoreClock);
	*/
	return (((uint64_t)high << 32) | ((uint64_t)low)) / divider;
}

U64 TsNs::toU64(){
	return ((uint64_t)high << 32) | ((uint64_t)low);
}

U64 TsNs::operator - (TsNs ts2){
	U64 time = ((uint64_t)(this->high - ts2.high) << 32) + this->low - ts2.low;
	return time / divider;
}
		
U64 TsNs::operator + (TsNs ts2){
	return (((U64)(this->high + ts2.high)<<32)|((U64)(this->low + ts2.low))) / divider;
}
		
bool TsNs::operator == (TsNs ts2){
	return this->high == ts2.high && this->low == ts2.low;
}
		
bool TsNs::operator != (TsNs ts2){
	return (this->high != ts2.high || this->low != ts2.low);
}
		
bool TsNs::operator > (TsNs ts2){
	return ((this->high > ts2.high) || (this->high == ts2.high && this->low > ts2.low));
}
		
bool TsNs::operator < (TsNs ts2){
	return ((this->high < ts2.high) || (this->high == ts2.high && this->low < ts2.low));
}
