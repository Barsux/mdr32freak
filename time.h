#ifndef TIMEH
#define TIMEH

#include "base.h"
#include "uart.h"
/*
Инициализация таймера
*/
void time_init();

/*
Проверка на переполнение таймера
*/
void check_overloading();


class TsNs{
	public:
		TsNs();
		uint32_t high, low;
		void renew();
		uint64_t toutc();
		uint64_t operator - (TsNs ts2);
		uint64_t operator + (TsNs ts2);
		bool operator == (TsNs ts2);
		bool operator != (TsNs ts2);
		bool operator > (TsNs ts2);
		bool operator < (TsNs ts2);
};

#endif 