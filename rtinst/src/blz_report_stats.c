#define PE_CODE
typedef long long int int64 ;
#include "blz_stats.h"

extern BlzStats event_stats ;

int test_this_feature(int trash)
{
	return 123456 ;
}

int report_send_tries(int trash)
{
	return  event_stats.send_tries ;
}

int report_buffed_pkts(int barf)
{
	int total = 0 ;
	int i ;
	for (i=0; i< NUM_PKT_BUF_REASONS; i++)
		total += event_stats.buffered_pkts[i] ;
	return total ;
}

int report_packets_sent(int gabbage)
{
	int total = 0 ;
	int i ;
	for (i=0; i< NUM_PKT_BUF_REASONS; i++)
		total += event_stats.buffered_pkts[i] ;
	return total + event_stats.unbuffered_pkts ;
}

