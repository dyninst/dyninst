/* Test application (Mutatee) */

/* $Id: test9.mutatee.c,v 1.4 2003/09/11 18:21:03 chadd Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ldr.h>


#include <unistd.h>

#ifdef __cplusplus
#include "cpp_test.h"
#include <iostream.h>
#endif

#include "test1.h"

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif
#define USAGE "Usage: test1.mutatee [-attach] [-verbose] -run <num> .."

#define MAX_TEST 6 
#define TRUE 1
#define FALSE 0
int debugPrint = 0;
#define dprintf if (debugPrint) printf
int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];
const char Builder_id[]=COMPILER; /* defined on compile line */


int globalVariable1_1 = 0;
int globalVariable2_1 = 0;
int globalVariable4_1 = 42;
int globalVariable5_1 = 66;
int globalVariable6_1 = 11;
#ifdef __cplusplus
extern "C"{
#endif

extern void func6_2(); /*this is in libInstMe.so */

#ifdef __cplusplus
}
#endif

 
void func6_1(){

#if !defined(i386_unknown_linux2_0) 
	/* !defined(sparc_sun_solaris2_4) && */
    printf("Skipped test #6 (instrument a shared library and save the world)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[6] = TRUE;
#else
	
	func6_2(); /*this is in libInstMe.so */
	if(globalVariable6_1 == 22){
		printf("Passed Test #6 (instrument a shared library and save the world)\n");
		passedTest[6] = TRUE;	
	}else{
		printf("**Failed Test #6 (instrument a shared library and save the world)\n");
	}

#endif
	
}

void call2_1000(){
	globalVariable2_1 += 1;
}
int func2_1000(){
	if(globalVariable2_1== 1000){
		return  1;
	}else{
		return 0;
	}
}

void call2_999(){
	globalVariable2_1 += 1;
}
int func2_999(){
	int ret=0;
	if(globalVariable2_1== 999){
		ret=func2_1000();
		return ret;
	}else{
			return 0;
	}
}
void call2_998(){
	globalVariable2_1 += 1;
}
int func2_998(){
	int ret=0;
	if(globalVariable2_1== 998){
		ret=func2_999();
		return ret;
	}else{
			return 0;
	}
}
void call2_997(){
	globalVariable2_1 += 1;
}
int func2_997(){
	int ret=0;
	if(globalVariable2_1== 997){
		ret=func2_998();
		return ret;
	}else{
			return 0;
	}
}
void call2_996(){
	globalVariable2_1 += 1;
}
int func2_996(){
	int ret=0;
	if(globalVariable2_1== 996){
		ret=func2_997();
		return ret;
	}else{
			return 0;
	}
}
void call2_995(){
	globalVariable2_1 += 1;
}
int func2_995(){
	int ret=0;
	if(globalVariable2_1== 995){
		ret=func2_996();
		return ret;
	}else{
			return 0;
	}
}
void call2_994(){
	globalVariable2_1 += 1;
}
int func2_994(){
	int ret=0;
	if(globalVariable2_1== 994){
		ret=func2_995();
		return ret;
	}else{
			return 0;
	}
}
void call2_993(){
	globalVariable2_1 += 1;
}
int func2_993(){
	int ret=0;
	if(globalVariable2_1== 993){
		ret=func2_994();
		return ret;
	}else{
			return 0;
	}
}
void call2_992(){
	globalVariable2_1 += 1;
}
int func2_992(){
	int ret=0;
	if(globalVariable2_1== 992){
		ret=func2_993();
		return ret;
	}else{
			return 0;
	}
}
void call2_991(){
	globalVariable2_1 += 1;
}
int func2_991(){
	int ret=0;
	if(globalVariable2_1== 991){
		ret=func2_992();
		return ret;
	}else{
			return 0;
	}
}
void call2_990(){
	globalVariable2_1 += 1;
}
int func2_990(){
	int ret=0;
	if(globalVariable2_1== 990){
		ret=func2_991();
		return ret;
	}else{
			return 0;
	}
}
void call2_989(){
	globalVariable2_1 += 1;
}
int func2_989(){
	int ret=0;
	if(globalVariable2_1== 989){
		ret=func2_990();
		return ret;
	}else{
			return 0;
	}
}
void call2_988(){
	globalVariable2_1 += 1;
}
int func2_988(){
	int ret=0;
	if(globalVariable2_1== 988){
		ret=func2_989();
		return ret;
	}else{
			return 0;
	}
}
void call2_987(){
	globalVariable2_1 += 1;
}
int func2_987(){
	int ret=0;
	if(globalVariable2_1== 987){
		ret=func2_988();
		return ret;
	}else{
			return 0;
	}
}
void call2_986(){
	globalVariable2_1 += 1;
}
int func2_986(){
	int ret=0;
	if(globalVariable2_1== 986){
		ret=func2_987();
		return ret;
	}else{
			return 0;
	}
}
void call2_985(){
	globalVariable2_1 += 1;
}
int func2_985(){
	int ret=0;
	if(globalVariable2_1== 985){
		ret=func2_986();
		return ret;
	}else{
			return 0;
	}
}
void call2_984(){
	globalVariable2_1 += 1;
}
int func2_984(){
	int ret=0;
	if(globalVariable2_1== 984){
		ret=func2_985();
		return ret;
	}else{
			return 0;
	}
}
void call2_983(){
	globalVariable2_1 += 1;
}
int func2_983(){
	int ret=0;
	if(globalVariable2_1== 983){
		ret=func2_984();
		return ret;
	}else{
			return 0;
	}
}
void call2_982(){
	globalVariable2_1 += 1;
}
int func2_982(){
	int ret=0;
	if(globalVariable2_1== 982){
		ret=func2_983();
		return ret;
	}else{
			return 0;
	}
}
void call2_981(){
	globalVariable2_1 += 1;
}
int func2_981(){
	int ret=0;
	if(globalVariable2_1== 981){
		ret=func2_982();
		return ret;
	}else{
			return 0;
	}
}
void call2_980(){
	globalVariable2_1 += 1;
}
int func2_980(){
	int ret=0;
	if(globalVariable2_1== 980){
		ret=func2_981();
		return ret;
	}else{
			return 0;
	}
}
void call2_979(){
	globalVariable2_1 += 1;
}
int func2_979(){
	int ret=0;
	if(globalVariable2_1== 979){
		ret=func2_980();
		return ret;
	}else{
			return 0;
	}
}
void call2_978(){
	globalVariable2_1 += 1;
}
int func2_978(){
	int ret=0;
	if(globalVariable2_1== 978){
		ret=func2_979();
		return ret;
	}else{
			return 0;
	}
}
void call2_977(){
	globalVariable2_1 += 1;
}
int func2_977(){
	int ret=0;
	if(globalVariable2_1== 977){
		ret=func2_978();
		return ret;
	}else{
			return 0;
	}
}
void call2_976(){
	globalVariable2_1 += 1;
}
int func2_976(){
	int ret=0;
	if(globalVariable2_1== 976){
		ret=func2_977();
		return ret;
	}else{
			return 0;
	}
}
void call2_975(){
	globalVariable2_1 += 1;
}
int func2_975(){
	int ret=0;
	if(globalVariable2_1== 975){
		ret=func2_976();
		return ret;
	}else{
			return 0;
	}
}
void call2_974(){
	globalVariable2_1 += 1;
}
int func2_974(){
	int ret=0;
	if(globalVariable2_1== 974){
		ret=func2_975();
		return ret;
	}else{
			return 0;
	}
}
void call2_973(){
	globalVariable2_1 += 1;
}
int func2_973(){
	int ret=0;
	if(globalVariable2_1== 973){
		ret=func2_974();
		return ret;
	}else{
			return 0;
	}
}
void call2_972(){
	globalVariable2_1 += 1;
}
int func2_972(){
	int ret=0;
	if(globalVariable2_1== 972){
		ret=func2_973();
		return ret;
	}else{
			return 0;
	}
}
void call2_971(){
	globalVariable2_1 += 1;
}
int func2_971(){
	int ret=0;
	if(globalVariable2_1== 971){
		ret=func2_972();
		return ret;
	}else{
			return 0;
	}
}
void call2_970(){
	globalVariable2_1 += 1;
}
int func2_970(){
	int ret=0;
	if(globalVariable2_1== 970){
		ret=func2_971();
		return ret;
	}else{
			return 0;
	}
}
void call2_969(){
	globalVariable2_1 += 1;
}
int func2_969(){
	int ret=0;
	if(globalVariable2_1== 969){
		ret=func2_970();
		return ret;
	}else{
			return 0;
	}
}
void call2_968(){
	globalVariable2_1 += 1;
}
int func2_968(){
	int ret=0;
	if(globalVariable2_1== 968){
		ret=func2_969();
		return ret;
	}else{
			return 0;
	}
}
void call2_967(){
	globalVariable2_1 += 1;
}
int func2_967(){
	int ret=0;
	if(globalVariable2_1== 967){
		ret=func2_968();
		return ret;
	}else{
			return 0;
	}
}
void call2_966(){
	globalVariable2_1 += 1;
}
int func2_966(){
	int ret=0;
	if(globalVariable2_1== 966){
		ret=func2_967();
		return ret;
	}else{
			return 0;
	}
}
void call2_965(){
	globalVariable2_1 += 1;
}
int func2_965(){
	int ret=0;
	if(globalVariable2_1== 965){
		ret=func2_966();
		return ret;
	}else{
			return 0;
	}
}
void call2_964(){
	globalVariable2_1 += 1;
}
int func2_964(){
	int ret=0;
	if(globalVariable2_1== 964){
		ret=func2_965();
		return ret;
	}else{
			return 0;
	}
}
void call2_963(){
	globalVariable2_1 += 1;
}
int func2_963(){
	int ret=0;
	if(globalVariable2_1== 963){
		ret=func2_964();
		return ret;
	}else{
			return 0;
	}
}
void call2_962(){
	globalVariable2_1 += 1;
}
int func2_962(){
	int ret=0;
	if(globalVariable2_1== 962){
		ret=func2_963();
		return ret;
	}else{
			return 0;
	}
}
void call2_961(){
	globalVariable2_1 += 1;
}
int func2_961(){
	int ret=0;
	if(globalVariable2_1== 961){
		ret=func2_962();
		return ret;
	}else{
			return 0;
	}
}
void call2_960(){
	globalVariable2_1 += 1;
}
int func2_960(){
	int ret=0;
	if(globalVariable2_1== 960){
		ret=func2_961();
		return ret;
	}else{
			return 0;
	}
}
void call2_959(){
	globalVariable2_1 += 1;
}
int func2_959(){
	int ret=0;
	if(globalVariable2_1== 959){
		ret=func2_960();
		return ret;
	}else{
			return 0;
	}
}
void call2_958(){
	globalVariable2_1 += 1;
}
int func2_958(){
	int ret=0;
	if(globalVariable2_1== 958){
		ret=func2_959();
		return ret;
	}else{
			return 0;
	}
}
void call2_957(){
	globalVariable2_1 += 1;
}
int func2_957(){
	int ret=0;
	if(globalVariable2_1== 957){
		ret=func2_958();
		return ret;
	}else{
			return 0;
	}
}
void call2_956(){
	globalVariable2_1 += 1;
}
int func2_956(){
	int ret=0;
	if(globalVariable2_1== 956){
		ret=func2_957();
		return ret;
	}else{
			return 0;
	}
}
void call2_955(){
	globalVariable2_1 += 1;
}
int func2_955(){
	int ret=0;
	if(globalVariable2_1== 955){
		ret=func2_956();
		return ret;
	}else{
			return 0;
	}
}
void call2_954(){
	globalVariable2_1 += 1;
}
int func2_954(){
	int ret=0;
	if(globalVariable2_1== 954){
		ret=func2_955();
		return ret;
	}else{
			return 0;
	}
}
void call2_953(){
	globalVariable2_1 += 1;
}
int func2_953(){
	int ret=0;
	if(globalVariable2_1== 953){
		ret=func2_954();
		return ret;
	}else{
			return 0;
	}
}
void call2_952(){
	globalVariable2_1 += 1;
}
int func2_952(){
	int ret=0;
	if(globalVariable2_1== 952){
		ret=func2_953();
		return ret;
	}else{
			return 0;
	}
}
void call2_951(){
	globalVariable2_1 += 1;
}
int func2_951(){
	int ret=0;
	if(globalVariable2_1== 951){
		ret=func2_952();
		return ret;
	}else{
			return 0;
	}
}
void call2_950(){
	globalVariable2_1 += 1;
}
int func2_950(){
	int ret=0;
	if(globalVariable2_1== 950){
		ret=func2_951();
		return ret;
	}else{
			return 0;
	}
}
void call2_949(){
	globalVariable2_1 += 1;
}
int func2_949(){
	int ret=0;
	if(globalVariable2_1== 949){
		ret=func2_950();
		return ret;
	}else{
			return 0;
	}
}
void call2_948(){
	globalVariable2_1 += 1;
}
int func2_948(){
	int ret=0;
	if(globalVariable2_1== 948){
		ret=func2_949();
		return ret;
	}else{
			return 0;
	}
}
void call2_947(){
	globalVariable2_1 += 1;
}
int func2_947(){
	int ret=0;
	if(globalVariable2_1== 947){
		ret=func2_948();
		return ret;
	}else{
			return 0;
	}
}
void call2_946(){
	globalVariable2_1 += 1;
}
int func2_946(){
	int ret=0;
	if(globalVariable2_1== 946){
		ret=func2_947();
		return ret;
	}else{
			return 0;
	}
}
void call2_945(){
	globalVariable2_1 += 1;
}
int func2_945(){
	int ret=0;
	if(globalVariable2_1== 945){
		ret=func2_946();
		return ret;
	}else{
			return 0;
	}
}
void call2_944(){
	globalVariable2_1 += 1;
}
int func2_944(){
	int ret=0;
	if(globalVariable2_1== 944){
		ret=func2_945();
		return ret;
	}else{
			return 0;
	}
}
void call2_943(){
	globalVariable2_1 += 1;
}
int func2_943(){
	int ret=0;
	if(globalVariable2_1== 943){
		ret=func2_944();
		return ret;
	}else{
			return 0;
	}
}
void call2_942(){
	globalVariable2_1 += 1;
}
int func2_942(){
	int ret=0;
	if(globalVariable2_1== 942){
		ret=func2_943();
		return ret;
	}else{
			return 0;
	}
}
void call2_941(){
	globalVariable2_1 += 1;
}
int func2_941(){
	int ret=0;
	if(globalVariable2_1== 941){
		ret=func2_942();
		return ret;
	}else{
			return 0;
	}
}
void call2_940(){
	globalVariable2_1 += 1;
}
int func2_940(){
	int ret=0;
	if(globalVariable2_1== 940){
		ret=func2_941();
		return ret;
	}else{
			return 0;
	}
}
void call2_939(){
	globalVariable2_1 += 1;
}
int func2_939(){
	int ret=0;
	if(globalVariable2_1== 939){
		ret=func2_940();
		return ret;
	}else{
			return 0;
	}
}
void call2_938(){
	globalVariable2_1 += 1;
}
int func2_938(){
	int ret=0;
	if(globalVariable2_1== 938){
		ret=func2_939();
		return ret;
	}else{
			return 0;
	}
}
void call2_937(){
	globalVariable2_1 += 1;
}
int func2_937(){
	int ret=0;
	if(globalVariable2_1== 937){
		ret=func2_938();
		return ret;
	}else{
			return 0;
	}
}
void call2_936(){
	globalVariable2_1 += 1;
}
int func2_936(){
	int ret=0;
	if(globalVariable2_1== 936){
		ret=func2_937();
		return ret;
	}else{
			return 0;
	}
}
void call2_935(){
	globalVariable2_1 += 1;
}
int func2_935(){
	int ret=0;
	if(globalVariable2_1== 935){
		ret=func2_936();
		return ret;
	}else{
			return 0;
	}
}
void call2_934(){
	globalVariable2_1 += 1;
}
int func2_934(){
	int ret=0;
	if(globalVariable2_1== 934){
		ret=func2_935();
		return ret;
	}else{
			return 0;
	}
}
void call2_933(){
	globalVariable2_1 += 1;
}
int func2_933(){
	int ret=0;
	if(globalVariable2_1== 933){
		ret=func2_934();
		return ret;
	}else{
			return 0;
	}
}
void call2_932(){
	globalVariable2_1 += 1;
}
int func2_932(){
	int ret=0;
	if(globalVariable2_1== 932){
		ret=func2_933();
		return ret;
	}else{
			return 0;
	}
}
void call2_931(){
	globalVariable2_1 += 1;
}
int func2_931(){
	int ret=0;
	if(globalVariable2_1== 931){
		ret=func2_932();
		return ret;
	}else{
			return 0;
	}
}
void call2_930(){
	globalVariable2_1 += 1;
}
int func2_930(){
	int ret=0;
	if(globalVariable2_1== 930){
		ret=func2_931();
		return ret;
	}else{
			return 0;
	}
}
void call2_929(){
	globalVariable2_1 += 1;
}
int func2_929(){
	int ret=0;
	if(globalVariable2_1== 929){
		ret=func2_930();
		return ret;
	}else{
			return 0;
	}
}
void call2_928(){
	globalVariable2_1 += 1;
}
int func2_928(){
	int ret=0;
	if(globalVariable2_1== 928){
		ret=func2_929();
		return ret;
	}else{
			return 0;
	}
}
void call2_927(){
	globalVariable2_1 += 1;
}
int func2_927(){
	int ret=0;
	if(globalVariable2_1== 927){
		ret=func2_928();
		return ret;
	}else{
			return 0;
	}
}
void call2_926(){
	globalVariable2_1 += 1;
}
int func2_926(){
	int ret=0;
	if(globalVariable2_1== 926){
		ret=func2_927();
		return ret;
	}else{
			return 0;
	}
}
void call2_925(){
	globalVariable2_1 += 1;
}
int func2_925(){
	int ret=0;
	if(globalVariable2_1== 925){
		ret=func2_926();
		return ret;
	}else{
			return 0;
	}
}
void call2_924(){
	globalVariable2_1 += 1;
}
int func2_924(){
	int ret=0;
	if(globalVariable2_1== 924){
		ret=func2_925();
		return ret;
	}else{
			return 0;
	}
}
void call2_923(){
	globalVariable2_1 += 1;
}
int func2_923(){
	int ret=0;
	if(globalVariable2_1== 923){
		ret=func2_924();
		return ret;
	}else{
			return 0;
	}
}
void call2_922(){
	globalVariable2_1 += 1;
}
int func2_922(){
	int ret=0;
	if(globalVariable2_1== 922){
		ret=func2_923();
		return ret;
	}else{
			return 0;
	}
}
void call2_921(){
	globalVariable2_1 += 1;
}
int func2_921(){
	int ret=0;
	if(globalVariable2_1== 921){
		ret=func2_922();
		return ret;
	}else{
			return 0;
	}
}
void call2_920(){
	globalVariable2_1 += 1;
}
int func2_920(){
	int ret=0;
	if(globalVariable2_1== 920){
		ret=func2_921();
		return ret;
	}else{
			return 0;
	}
}
void call2_919(){
	globalVariable2_1 += 1;
}
int func2_919(){
	int ret=0;
	if(globalVariable2_1== 919){
		ret=func2_920();
		return ret;
	}else{
			return 0;
	}
}
void call2_918(){
	globalVariable2_1 += 1;
}
int func2_918(){
	int ret=0;
	if(globalVariable2_1== 918){
		ret=func2_919();
		return ret;
	}else{
			return 0;
	}
}
void call2_917(){
	globalVariable2_1 += 1;
}
int func2_917(){
	int ret=0;
	if(globalVariable2_1== 917){
		ret=func2_918();
		return ret;
	}else{
			return 0;
	}
}
void call2_916(){
	globalVariable2_1 += 1;
}
int func2_916(){
	int ret=0;
	if(globalVariable2_1== 916){
		ret=func2_917();
		return ret;
	}else{
			return 0;
	}
}
void call2_915(){
	globalVariable2_1 += 1;
}
int func2_915(){
	int ret=0;
	if(globalVariable2_1== 915){
		ret=func2_916();
		return ret;
	}else{
			return 0;
	}
}
void call2_914(){
	globalVariable2_1 += 1;
}
int func2_914(){
	int ret=0;
	if(globalVariable2_1== 914){
		ret=func2_915();
		return ret;
	}else{
			return 0;
	}
}
void call2_913(){
	globalVariable2_1 += 1;
}
int func2_913(){
	int ret=0;
	if(globalVariable2_1== 913){
		ret=func2_914();
		return ret;
	}else{
			return 0;
	}
}
void call2_912(){
	globalVariable2_1 += 1;
}
int func2_912(){
	int ret=0;
	if(globalVariable2_1== 912){
		ret=func2_913();
		return ret;
	}else{
			return 0;
	}
}
void call2_911(){
	globalVariable2_1 += 1;
}
int func2_911(){
	int ret=0;
	if(globalVariable2_1== 911){
		ret=func2_912();
		return ret;
	}else{
			return 0;
	}
}
void call2_910(){
	globalVariable2_1 += 1;
}
int func2_910(){
	int ret=0;
	if(globalVariable2_1== 910){
		ret=func2_911();
		return ret;
	}else{
			return 0;
	}
}
void call2_909(){
	globalVariable2_1 += 1;
}
int func2_909(){
	int ret=0;
	if(globalVariable2_1== 909){
		ret=func2_910();
		return ret;
	}else{
			return 0;
	}
}
void call2_908(){
	globalVariable2_1 += 1;
}
int func2_908(){
	int ret=0;
	if(globalVariable2_1== 908){
		ret=func2_909();
		return ret;
	}else{
			return 0;
	}
}
void call2_907(){
	globalVariable2_1 += 1;
}
int func2_907(){
	int ret=0;
	if(globalVariable2_1== 907){
		ret=func2_908();
		return ret;
	}else{
			return 0;
	}
}
void call2_906(){
	globalVariable2_1 += 1;
}
int func2_906(){
	int ret=0;
	if(globalVariable2_1== 906){
		ret=func2_907();
		return ret;
	}else{
			return 0;
	}
}
void call2_905(){
	globalVariable2_1 += 1;
}
int func2_905(){
	int ret=0;
	if(globalVariable2_1== 905){
		ret=func2_906();
		return ret;
	}else{
			return 0;
	}
}
void call2_904(){
	globalVariable2_1 += 1;
}
int func2_904(){
	int ret=0;
	if(globalVariable2_1== 904){
		ret=func2_905();
		return ret;
	}else{
			return 0;
	}
}
void call2_903(){
	globalVariable2_1 += 1;
}
int func2_903(){
	int ret=0;
	if(globalVariable2_1== 903){
		ret=func2_904();
		return ret;
	}else{
			return 0;
	}
}
void call2_902(){
	globalVariable2_1 += 1;
}
int func2_902(){
	int ret=0;
	if(globalVariable2_1== 902){
		ret=func2_903();
		return ret;
	}else{
			return 0;
	}
}
void call2_901(){
	globalVariable2_1 += 1;
}
int func2_901(){
	int ret=0;
	if(globalVariable2_1== 901){
		ret=func2_902();
		return ret;
	}else{
			return 0;
	}
}
void call2_900(){
	globalVariable2_1 += 1;
}
int func2_900(){
	int ret=0;
	if(globalVariable2_1== 900){
		ret=func2_901();
		return ret;
	}else{
			return 0;
	}
}
void call2_899(){
	globalVariable2_1 += 1;
}
int func2_899(){
	int ret=0;
	if(globalVariable2_1== 899){
		ret=func2_900();
		return ret;
	}else{
			return 0;
	}
}
void call2_898(){
	globalVariable2_1 += 1;
}
int func2_898(){
	int ret=0;
	if(globalVariable2_1== 898){
		ret=func2_899();
		return ret;
	}else{
			return 0;
	}
}
void call2_897(){
	globalVariable2_1 += 1;
}
int func2_897(){
	int ret=0;
	if(globalVariable2_1== 897){
		ret=func2_898();
		return ret;
	}else{
			return 0;
	}
}
void call2_896(){
	globalVariable2_1 += 1;
}
int func2_896(){
	int ret=0;
	if(globalVariable2_1== 896){
		ret=func2_897();
		return ret;
	}else{
			return 0;
	}
}
void call2_895(){
	globalVariable2_1 += 1;
}
int func2_895(){
	int ret=0;
	if(globalVariable2_1== 895){
		ret=func2_896();
		return ret;
	}else{
			return 0;
	}
}
void call2_894(){
	globalVariable2_1 += 1;
}
int func2_894(){
	int ret=0;
	if(globalVariable2_1== 894){
		ret=func2_895();
		return ret;
	}else{
			return 0;
	}
}
void call2_893(){
	globalVariable2_1 += 1;
}
int func2_893(){
	int ret=0;
	if(globalVariable2_1== 893){
		ret=func2_894();
		return ret;
	}else{
			return 0;
	}
}
void call2_892(){
	globalVariable2_1 += 1;
}
int func2_892(){
	int ret=0;
	if(globalVariable2_1== 892){
		ret=func2_893();
		return ret;
	}else{
			return 0;
	}
}
void call2_891(){
	globalVariable2_1 += 1;
}
int func2_891(){
	int ret=0;
	if(globalVariable2_1== 891){
		ret=func2_892();
		return ret;
	}else{
			return 0;
	}
}
void call2_890(){
	globalVariable2_1 += 1;
}
int func2_890(){
	int ret=0;
	if(globalVariable2_1== 890){
		ret=func2_891();
		return ret;
	}else{
			return 0;
	}
}
void call2_889(){
	globalVariable2_1 += 1;
}
int func2_889(){
	int ret=0;
	if(globalVariable2_1== 889){
		ret=func2_890();
		return ret;
	}else{
			return 0;
	}
}
void call2_888(){
	globalVariable2_1 += 1;
}
int func2_888(){
	int ret=0;
	if(globalVariable2_1== 888){
		ret=func2_889();
		return ret;
	}else{
			return 0;
	}
}
void call2_887(){
	globalVariable2_1 += 1;
}
int func2_887(){
	int ret=0;
	if(globalVariable2_1== 887){
		ret=func2_888();
		return ret;
	}else{
			return 0;
	}
}
void call2_886(){
	globalVariable2_1 += 1;
}
int func2_886(){
	int ret=0;
	if(globalVariable2_1== 886){
		ret=func2_887();
		return ret;
	}else{
			return 0;
	}
}
void call2_885(){
	globalVariable2_1 += 1;
}
int func2_885(){
	int ret=0;
	if(globalVariable2_1== 885){
		ret=func2_886();
		return ret;
	}else{
			return 0;
	}
}
void call2_884(){
	globalVariable2_1 += 1;
}
int func2_884(){
	int ret=0;
	if(globalVariable2_1== 884){
		ret=func2_885();
		return ret;
	}else{
			return 0;
	}
}
void call2_883(){
	globalVariable2_1 += 1;
}
int func2_883(){
	int ret=0;
	if(globalVariable2_1== 883){
		ret=func2_884();
		return ret;
	}else{
			return 0;
	}
}
void call2_882(){
	globalVariable2_1 += 1;
}
int func2_882(){
	int ret=0;
	if(globalVariable2_1== 882){
		ret=func2_883();
		return ret;
	}else{
			return 0;
	}
}
void call2_881(){
	globalVariable2_1 += 1;
}
int func2_881(){
	int ret=0;
	if(globalVariable2_1== 881){
		ret=func2_882();
		return ret;
	}else{
			return 0;
	}
}
void call2_880(){
	globalVariable2_1 += 1;
}
int func2_880(){
	int ret=0;
	if(globalVariable2_1== 880){
		ret=func2_881();
		return ret;
	}else{
			return 0;
	}
}
void call2_879(){
	globalVariable2_1 += 1;
}
int func2_879(){
	int ret=0;
	if(globalVariable2_1== 879){
		ret=func2_880();
		return ret;
	}else{
			return 0;
	}
}
void call2_878(){
	globalVariable2_1 += 1;
}
int func2_878(){
	int ret=0;
	if(globalVariable2_1== 878){
		ret=func2_879();
		return ret;
	}else{
			return 0;
	}
}
void call2_877(){
	globalVariable2_1 += 1;
}
int func2_877(){
	int ret=0;
	if(globalVariable2_1== 877){
		ret=func2_878();
		return ret;
	}else{
			return 0;
	}
}
void call2_876(){
	globalVariable2_1 += 1;
}
int func2_876(){
	int ret=0;
	if(globalVariable2_1== 876){
		ret=func2_877();
		return ret;
	}else{
			return 0;
	}
}
void call2_875(){
	globalVariable2_1 += 1;
}
int func2_875(){
	int ret=0;
	if(globalVariable2_1== 875){
		ret=func2_876();
		return ret;
	}else{
			return 0;
	}
}
void call2_874(){
	globalVariable2_1 += 1;
}
int func2_874(){
	int ret=0;
	if(globalVariable2_1== 874){
		ret=func2_875();
		return ret;
	}else{
			return 0;
	}
}
void call2_873(){
	globalVariable2_1 += 1;
}
int func2_873(){
	int ret=0;
	if(globalVariable2_1== 873){
		ret=func2_874();
		return ret;
	}else{
			return 0;
	}
}
void call2_872(){
	globalVariable2_1 += 1;
}
int func2_872(){
	int ret=0;
	if(globalVariable2_1== 872){
		ret=func2_873();
		return ret;
	}else{
			return 0;
	}
}
void call2_871(){
	globalVariable2_1 += 1;
}
int func2_871(){
	int ret=0;
	if(globalVariable2_1== 871){
		ret=func2_872();
		return ret;
	}else{
			return 0;
	}
}
void call2_870(){
	globalVariable2_1 += 1;
}
int func2_870(){
	int ret=0;
	if(globalVariable2_1== 870){
		ret=func2_871();
		return ret;
	}else{
			return 0;
	}
}
void call2_869(){
	globalVariable2_1 += 1;
}
int func2_869(){
	int ret=0;
	if(globalVariable2_1== 869){
		ret=func2_870();
		return ret;
	}else{
			return 0;
	}
}
void call2_868(){
	globalVariable2_1 += 1;
}
int func2_868(){
	int ret=0;
	if(globalVariable2_1== 868){
		ret=func2_869();
		return ret;
	}else{
			return 0;
	}
}
void call2_867(){
	globalVariable2_1 += 1;
}
int func2_867(){
	int ret=0;
	if(globalVariable2_1== 867){
		ret=func2_868();
		return ret;
	}else{
			return 0;
	}
}
void call2_866(){
	globalVariable2_1 += 1;
}
int func2_866(){
	int ret=0;
	if(globalVariable2_1== 866){
		ret=func2_867();
		return ret;
	}else{
			return 0;
	}
}
void call2_865(){
	globalVariable2_1 += 1;
}
int func2_865(){
	int ret=0;
	if(globalVariable2_1== 865){
		ret=func2_866();
		return ret;
	}else{
			return 0;
	}
}
void call2_864(){
	globalVariable2_1 += 1;
}
int func2_864(){
	int ret=0;
	if(globalVariable2_1== 864){
		ret=func2_865();
		return ret;
	}else{
			return 0;
	}
}
void call2_863(){
	globalVariable2_1 += 1;
}
int func2_863(){
	int ret=0;
	if(globalVariable2_1== 863){
		ret=func2_864();
		return ret;
	}else{
			return 0;
	}
}
void call2_862(){
	globalVariable2_1 += 1;
}
int func2_862(){
	int ret=0;
	if(globalVariable2_1== 862){
		ret=func2_863();
		return ret;
	}else{
			return 0;
	}
}
void call2_861(){
	globalVariable2_1 += 1;
}
int func2_861(){
	int ret=0;
	if(globalVariable2_1== 861){
		ret=func2_862();
		return ret;
	}else{
			return 0;
	}
}
void call2_860(){
	globalVariable2_1 += 1;
}
int func2_860(){
	int ret=0;
	if(globalVariable2_1== 860){
		ret=func2_861();
		return ret;
	}else{
			return 0;
	}
}
void call2_859(){
	globalVariable2_1 += 1;
}
int func2_859(){
	int ret=0;
	if(globalVariable2_1== 859){
		ret=func2_860();
		return ret;
	}else{
			return 0;
	}
}
void call2_858(){
	globalVariable2_1 += 1;
}
int func2_858(){
	int ret=0;
	if(globalVariable2_1== 858){
		ret=func2_859();
		return ret;
	}else{
			return 0;
	}
}
void call2_857(){
	globalVariable2_1 += 1;
}
int func2_857(){
	int ret=0;
	if(globalVariable2_1== 857){
		ret=func2_858();
		return ret;
	}else{
			return 0;
	}
}
void call2_856(){
	globalVariable2_1 += 1;
}
int func2_856(){
	int ret=0;
	if(globalVariable2_1== 856){
		ret=func2_857();
		return ret;
	}else{
			return 0;
	}
}
void call2_855(){
	globalVariable2_1 += 1;
}
int func2_855(){
	int ret=0;
	if(globalVariable2_1== 855){
		ret=func2_856();
		return ret;
	}else{
			return 0;
	}
}
void call2_854(){
	globalVariable2_1 += 1;
}
int func2_854(){
	int ret=0;
	if(globalVariable2_1== 854){
		ret=func2_855();
		return ret;
	}else{
			return 0;
	}
}
void call2_853(){
	globalVariable2_1 += 1;
}
int func2_853(){
	int ret=0;
	if(globalVariable2_1== 853){
		ret=func2_854();
		return ret;
	}else{
			return 0;
	}
}
void call2_852(){
	globalVariable2_1 += 1;
}
int func2_852(){
	int ret=0;
	if(globalVariable2_1== 852){
		ret=func2_853();
		return ret;
	}else{
			return 0;
	}
}
void call2_851(){
	globalVariable2_1 += 1;
}
int func2_851(){
	int ret=0;
	if(globalVariable2_1== 851){
		ret=func2_852();
		return ret;
	}else{
			return 0;
	}
}
void call2_850(){
	globalVariable2_1 += 1;
}
int func2_850(){
	int ret=0;
	if(globalVariable2_1== 850){
		ret=func2_851();
		return ret;
	}else{
			return 0;
	}
}
void call2_849(){
	globalVariable2_1 += 1;
}
int func2_849(){
	int ret=0;
	if(globalVariable2_1== 849){
		ret=func2_850();
		return ret;
	}else{
			return 0;
	}
}
void call2_848(){
	globalVariable2_1 += 1;
}
int func2_848(){
	int ret=0;
	if(globalVariable2_1== 848){
		ret=func2_849();
		return ret;
	}else{
			return 0;
	}
}
void call2_847(){
	globalVariable2_1 += 1;
}
int func2_847(){
	int ret=0;
	if(globalVariable2_1== 847){
		ret=func2_848();
		return ret;
	}else{
			return 0;
	}
}
void call2_846(){
	globalVariable2_1 += 1;
}
int func2_846(){
	int ret=0;
	if(globalVariable2_1== 846){
		ret=func2_847();
		return ret;
	}else{
			return 0;
	}
}
void call2_845(){
	globalVariable2_1 += 1;
}
int func2_845(){
	int ret=0;
	if(globalVariable2_1== 845){
		ret=func2_846();
		return ret;
	}else{
			return 0;
	}
}
void call2_844(){
	globalVariable2_1 += 1;
}
int func2_844(){
	int ret=0;
	if(globalVariable2_1== 844){
		ret=func2_845();
		return ret;
	}else{
			return 0;
	}
}
void call2_843(){
	globalVariable2_1 += 1;
}
int func2_843(){
	int ret=0;
	if(globalVariable2_1== 843){
		ret=func2_844();
		return ret;
	}else{
			return 0;
	}
}
void call2_842(){
	globalVariable2_1 += 1;
}
int func2_842(){
	int ret=0;
	if(globalVariable2_1== 842){
		ret=func2_843();
		return ret;
	}else{
			return 0;
	}
}
void call2_841(){
	globalVariable2_1 += 1;
}
int func2_841(){
	int ret=0;
	if(globalVariable2_1== 841){
		ret=func2_842();
		return ret;
	}else{
			return 0;
	}
}
void call2_840(){
	globalVariable2_1 += 1;
}
int func2_840(){
	int ret=0;
	if(globalVariable2_1== 840){
		ret=func2_841();
		return ret;
	}else{
			return 0;
	}
}
void call2_839(){
	globalVariable2_1 += 1;
}
int func2_839(){
	int ret=0;
	if(globalVariable2_1== 839){
		ret=func2_840();
		return ret;
	}else{
			return 0;
	}
}
void call2_838(){
	globalVariable2_1 += 1;
}
int func2_838(){
	int ret=0;
	if(globalVariable2_1== 838){
		ret=func2_839();
		return ret;
	}else{
			return 0;
	}
}
void call2_837(){
	globalVariable2_1 += 1;
}
int func2_837(){
	int ret=0;
	if(globalVariable2_1== 837){
		ret=func2_838();
		return ret;
	}else{
			return 0;
	}
}
void call2_836(){
	globalVariable2_1 += 1;
}
int func2_836(){
	int ret=0;
	if(globalVariable2_1== 836){
		ret=func2_837();
		return ret;
	}else{
			return 0;
	}
}
void call2_835(){
	globalVariable2_1 += 1;
}
int func2_835(){
	int ret=0;
	if(globalVariable2_1== 835){
		ret=func2_836();
		return ret;
	}else{
			return 0;
	}
}
void call2_834(){
	globalVariable2_1 += 1;
}
int func2_834(){
	int ret=0;
	if(globalVariable2_1== 834){
		ret=func2_835();
		return ret;
	}else{
			return 0;
	}
}
void call2_833(){
	globalVariable2_1 += 1;
}
int func2_833(){
	int ret=0;
	if(globalVariable2_1== 833){
		ret=func2_834();
		return ret;
	}else{
			return 0;
	}
}
void call2_832(){
	globalVariable2_1 += 1;
}
int func2_832(){
	int ret=0;
	if(globalVariable2_1== 832){
		ret=func2_833();
		return ret;
	}else{
			return 0;
	}
}
void call2_831(){
	globalVariable2_1 += 1;
}
int func2_831(){
	int ret=0;
	if(globalVariable2_1== 831){
		ret=func2_832();
		return ret;
	}else{
			return 0;
	}
}
void call2_830(){
	globalVariable2_1 += 1;
}
int func2_830(){
	int ret=0;
	if(globalVariable2_1== 830){
		ret=func2_831();
		return ret;
	}else{
			return 0;
	}
}
void call2_829(){
	globalVariable2_1 += 1;
}
int func2_829(){
	int ret=0;
	if(globalVariable2_1== 829){
		ret=func2_830();
		return ret;
	}else{
			return 0;
	}
}
void call2_828(){
	globalVariable2_1 += 1;
}
int func2_828(){
	int ret=0;
	if(globalVariable2_1== 828){
		ret=func2_829();
		return ret;
	}else{
			return 0;
	}
}
void call2_827(){
	globalVariable2_1 += 1;
}
int func2_827(){
	int ret=0;
	if(globalVariable2_1== 827){
		ret=func2_828();
		return ret;
	}else{
			return 0;
	}
}
void call2_826(){
	globalVariable2_1 += 1;
}
int func2_826(){
	int ret=0;
	if(globalVariable2_1== 826){
		ret=func2_827();
		return ret;
	}else{
			return 0;
	}
}
void call2_825(){
	globalVariable2_1 += 1;
}
int func2_825(){
	int ret=0;
	if(globalVariable2_1== 825){
		ret=func2_826();
		return ret;
	}else{
			return 0;
	}
}
void call2_824(){
	globalVariable2_1 += 1;
}
int func2_824(){
	int ret=0;
	if(globalVariable2_1== 824){
		ret=func2_825();
		return ret;
	}else{
			return 0;
	}
}
void call2_823(){
	globalVariable2_1 += 1;
}
int func2_823(){
	int ret=0;
	if(globalVariable2_1== 823){
		ret=func2_824();
		return ret;
	}else{
			return 0;
	}
}
void call2_822(){
	globalVariable2_1 += 1;
}
int func2_822(){
	int ret=0;
	if(globalVariable2_1== 822){
		ret=func2_823();
		return ret;
	}else{
			return 0;
	}
}
void call2_821(){
	globalVariable2_1 += 1;
}
int func2_821(){
	int ret=0;
	if(globalVariable2_1== 821){
		ret=func2_822();
		return ret;
	}else{
			return 0;
	}
}
void call2_820(){
	globalVariable2_1 += 1;
}
int func2_820(){
	int ret=0;
	if(globalVariable2_1== 820){
		ret=func2_821();
		return ret;
	}else{
			return 0;
	}
}
void call2_819(){
	globalVariable2_1 += 1;
}
int func2_819(){
	int ret=0;
	if(globalVariable2_1== 819){
		ret=func2_820();
		return ret;
	}else{
			return 0;
	}
}
void call2_818(){
	globalVariable2_1 += 1;
}
int func2_818(){
	int ret=0;
	if(globalVariable2_1== 818){
		ret=func2_819();
		return ret;
	}else{
			return 0;
	}
}
void call2_817(){
	globalVariable2_1 += 1;
}
int func2_817(){
	int ret=0;
	if(globalVariable2_1== 817){
		ret=func2_818();
		return ret;
	}else{
			return 0;
	}
}
void call2_816(){
	globalVariable2_1 += 1;
}
int func2_816(){
	int ret=0;
	if(globalVariable2_1== 816){
		ret=func2_817();
		return ret;
	}else{
			return 0;
	}
}
void call2_815(){
	globalVariable2_1 += 1;
}
int func2_815(){
	int ret=0;
	if(globalVariable2_1== 815){
		ret=func2_816();
		return ret;
	}else{
			return 0;
	}
}
void call2_814(){
	globalVariable2_1 += 1;
}
int func2_814(){
	int ret=0;
	if(globalVariable2_1== 814){
		ret=func2_815();
		return ret;
	}else{
			return 0;
	}
}
void call2_813(){
	globalVariable2_1 += 1;
}
int func2_813(){
	int ret=0;
	if(globalVariable2_1== 813){
		ret=func2_814();
		return ret;
	}else{
			return 0;
	}
}
void call2_812(){
	globalVariable2_1 += 1;
}
int func2_812(){
	int ret=0;
	if(globalVariable2_1== 812){
		ret=func2_813();
		return ret;
	}else{
			return 0;
	}
}
void call2_811(){
	globalVariable2_1 += 1;
}
int func2_811(){
	int ret=0;
	if(globalVariable2_1== 811){
		ret=func2_812();
		return ret;
	}else{
			return 0;
	}
}
void call2_810(){
	globalVariable2_1 += 1;
}
int func2_810(){
	int ret=0;
	if(globalVariable2_1== 810){
		ret=func2_811();
		return ret;
	}else{
			return 0;
	}
}
void call2_809(){
	globalVariable2_1 += 1;
}
int func2_809(){
	int ret=0;
	if(globalVariable2_1== 809){
		ret=func2_810();
		return ret;
	}else{
			return 0;
	}
}
void call2_808(){
	globalVariable2_1 += 1;
}
int func2_808(){
	int ret=0;
	if(globalVariable2_1== 808){
		ret=func2_809();
		return ret;
	}else{
			return 0;
	}
}
void call2_807(){
	globalVariable2_1 += 1;
}
int func2_807(){
	int ret=0;
	if(globalVariable2_1== 807){
		ret=func2_808();
		return ret;
	}else{
			return 0;
	}
}
void call2_806(){
	globalVariable2_1 += 1;
}
int func2_806(){
	int ret=0;
	if(globalVariable2_1== 806){
		ret=func2_807();
		return ret;
	}else{
			return 0;
	}
}
void call2_805(){
	globalVariable2_1 += 1;
}
int func2_805(){
	int ret=0;
	if(globalVariable2_1== 805){
		ret=func2_806();
		return ret;
	}else{
			return 0;
	}
}
void call2_804(){
	globalVariable2_1 += 1;
}
int func2_804(){
	int ret=0;
	if(globalVariable2_1== 804){
		ret=func2_805();
		return ret;
	}else{
			return 0;
	}
}
void call2_803(){
	globalVariable2_1 += 1;
}
int func2_803(){
	int ret=0;
	if(globalVariable2_1== 803){
		ret=func2_804();
		return ret;
	}else{
			return 0;
	}
}
void call2_802(){
	globalVariable2_1 += 1;
}
int func2_802(){
	int ret=0;
	if(globalVariable2_1== 802){
		ret=func2_803();
		return ret;
	}else{
			return 0;
	}
}
void call2_801(){
	globalVariable2_1 += 1;
}
int func2_801(){
	int ret=0;
	if(globalVariable2_1== 801){
		ret=func2_802();
		return ret;
	}else{
			return 0;
	}
}
void call2_800(){
	globalVariable2_1 += 1;
}
int func2_800(){
	int ret=0;
	if(globalVariable2_1== 800){
		ret=func2_801();
		return ret;
	}else{
			return 0;
	}
}
void call2_799(){
	globalVariable2_1 += 1;
}
int func2_799(){
	int ret=0;
	if(globalVariable2_1== 799){
		ret=func2_800();
		return ret;
	}else{
			return 0;
	}
}
void call2_798(){
	globalVariable2_1 += 1;
}
int func2_798(){
	int ret=0;
	if(globalVariable2_1== 798){
		ret=func2_799();
		return ret;
	}else{
			return 0;
	}
}
void call2_797(){
	globalVariable2_1 += 1;
}
int func2_797(){
	int ret=0;
	if(globalVariable2_1== 797){
		ret=func2_798();
		return ret;
	}else{
			return 0;
	}
}
void call2_796(){
	globalVariable2_1 += 1;
}
int func2_796(){
	int ret=0;
	if(globalVariable2_1== 796){
		ret=func2_797();
		return ret;
	}else{
			return 0;
	}
}
void call2_795(){
	globalVariable2_1 += 1;
}
int func2_795(){
	int ret=0;
	if(globalVariable2_1== 795){
		ret=func2_796();
		return ret;
	}else{
			return 0;
	}
}
void call2_794(){
	globalVariable2_1 += 1;
}
int func2_794(){
	int ret=0;
	if(globalVariable2_1== 794){
		ret=func2_795();
		return ret;
	}else{
			return 0;
	}
}
void call2_793(){
	globalVariable2_1 += 1;
}
int func2_793(){
	int ret=0;
	if(globalVariable2_1== 793){
		ret=func2_794();
		return ret;
	}else{
			return 0;
	}
}
void call2_792(){
	globalVariable2_1 += 1;
}
int func2_792(){
	int ret=0;
	if(globalVariable2_1== 792){
		ret=func2_793();
		return ret;
	}else{
			return 0;
	}
}
void call2_791(){
	globalVariable2_1 += 1;
}
int func2_791(){
	int ret=0;
	if(globalVariable2_1== 791){
		ret=func2_792();
		return ret;
	}else{
			return 0;
	}
}
void call2_790(){
	globalVariable2_1 += 1;
}
int func2_790(){
	int ret=0;
	if(globalVariable2_1== 790){
		ret=func2_791();
		return ret;
	}else{
			return 0;
	}
}
void call2_789(){
	globalVariable2_1 += 1;
}
int func2_789(){
	int ret=0;
	if(globalVariable2_1== 789){
		ret=func2_790();
		return ret;
	}else{
			return 0;
	}
}
void call2_788(){
	globalVariable2_1 += 1;
}
int func2_788(){
	int ret=0;
	if(globalVariable2_1== 788){
		ret=func2_789();
		return ret;
	}else{
			return 0;
	}
}
void call2_787(){
	globalVariable2_1 += 1;
}
int func2_787(){
	int ret=0;
	if(globalVariable2_1== 787){
		ret=func2_788();
		return ret;
	}else{
			return 0;
	}
}
void call2_786(){
	globalVariable2_1 += 1;
}
int func2_786(){
	int ret=0;
	if(globalVariable2_1== 786){
		ret=func2_787();
		return ret;
	}else{
			return 0;
	}
}
void call2_785(){
	globalVariable2_1 += 1;
}
int func2_785(){
	int ret=0;
	if(globalVariable2_1== 785){
		ret=func2_786();
		return ret;
	}else{
			return 0;
	}
}
void call2_784(){
	globalVariable2_1 += 1;
}
int func2_784(){
	int ret=0;
	if(globalVariable2_1== 784){
		ret=func2_785();
		return ret;
	}else{
			return 0;
	}
}
void call2_783(){
	globalVariable2_1 += 1;
}
int func2_783(){
	int ret=0;
	if(globalVariable2_1== 783){
		ret=func2_784();
		return ret;
	}else{
			return 0;
	}
}
void call2_782(){
	globalVariable2_1 += 1;
}
int func2_782(){
	int ret=0;
	if(globalVariable2_1== 782){
		ret=func2_783();
		return ret;
	}else{
			return 0;
	}
}
void call2_781(){
	globalVariable2_1 += 1;
}
int func2_781(){
	int ret=0;
	if(globalVariable2_1== 781){
		ret=func2_782();
		return ret;
	}else{
			return 0;
	}
}
void call2_780(){
	globalVariable2_1 += 1;
}
int func2_780(){
	int ret=0;
	if(globalVariable2_1== 780){
		ret=func2_781();
		return ret;
	}else{
			return 0;
	}
}
void call2_779(){
	globalVariable2_1 += 1;
}
int func2_779(){
	int ret=0;
	if(globalVariable2_1== 779){
		ret=func2_780();
		return ret;
	}else{
			return 0;
	}
}
void call2_778(){
	globalVariable2_1 += 1;
}
int func2_778(){
	int ret=0;
	if(globalVariable2_1== 778){
		ret=func2_779();
		return ret;
	}else{
			return 0;
	}
}
void call2_777(){
	globalVariable2_1 += 1;
}
int func2_777(){
	int ret=0;
	if(globalVariable2_1== 777){
		ret=func2_778();
		return ret;
	}else{
			return 0;
	}
}
void call2_776(){
	globalVariable2_1 += 1;
}
int func2_776(){
	int ret=0;
	if(globalVariable2_1== 776){
		ret=func2_777();
		return ret;
	}else{
			return 0;
	}
}
void call2_775(){
	globalVariable2_1 += 1;
}
int func2_775(){
	int ret=0;
	if(globalVariable2_1== 775){
		ret=func2_776();
		return ret;
	}else{
			return 0;
	}
}
void call2_774(){
	globalVariable2_1 += 1;
}
int func2_774(){
	int ret=0;
	if(globalVariable2_1== 774){
		ret=func2_775();
		return ret;
	}else{
			return 0;
	}
}
void call2_773(){
	globalVariable2_1 += 1;
}
int func2_773(){
	int ret=0;
	if(globalVariable2_1== 773){
		ret=func2_774();
		return ret;
	}else{
			return 0;
	}
}
void call2_772(){
	globalVariable2_1 += 1;
}
int func2_772(){
	int ret=0;
	if(globalVariable2_1== 772){
		ret=func2_773();
		return ret;
	}else{
			return 0;
	}
}
void call2_771(){
	globalVariable2_1 += 1;
}
int func2_771(){
	int ret=0;
	if(globalVariable2_1== 771){
		ret=func2_772();
		return ret;
	}else{
			return 0;
	}
}
void call2_770(){
	globalVariable2_1 += 1;
}
int func2_770(){
	int ret=0;
	if(globalVariable2_1== 770){
		ret=func2_771();
		return ret;
	}else{
			return 0;
	}
}
void call2_769(){
	globalVariable2_1 += 1;
}
int func2_769(){
	int ret=0;
	if(globalVariable2_1== 769){
		ret=func2_770();
		return ret;
	}else{
			return 0;
	}
}
void call2_768(){
	globalVariable2_1 += 1;
}
int func2_768(){
	int ret=0;
	if(globalVariable2_1== 768){
		ret=func2_769();
		return ret;
	}else{
			return 0;
	}
}
void call2_767(){
	globalVariable2_1 += 1;
}
int func2_767(){
	int ret=0;
	if(globalVariable2_1== 767){
		ret=func2_768();
		return ret;
	}else{
			return 0;
	}
}
void call2_766(){
	globalVariable2_1 += 1;
}
int func2_766(){
	int ret=0;
	if(globalVariable2_1== 766){
		ret=func2_767();
		return ret;
	}else{
			return 0;
	}
}
void call2_765(){
	globalVariable2_1 += 1;
}
int func2_765(){
	int ret=0;
	if(globalVariable2_1== 765){
		ret=func2_766();
		return ret;
	}else{
			return 0;
	}
}
void call2_764(){
	globalVariable2_1 += 1;
}
int func2_764(){
	int ret=0;
	if(globalVariable2_1== 764){
		ret=func2_765();
		return ret;
	}else{
			return 0;
	}
}
void call2_763(){
	globalVariable2_1 += 1;
}
int func2_763(){
	int ret=0;
	if(globalVariable2_1== 763){
		ret=func2_764();
		return ret;
	}else{
			return 0;
	}
}
void call2_762(){
	globalVariable2_1 += 1;
}
int func2_762(){
	int ret=0;
	if(globalVariable2_1== 762){
		ret=func2_763();
		return ret;
	}else{
			return 0;
	}
}
void call2_761(){
	globalVariable2_1 += 1;
}
int func2_761(){
	int ret=0;
	if(globalVariable2_1== 761){
		ret=func2_762();
		return ret;
	}else{
			return 0;
	}
}
void call2_760(){
	globalVariable2_1 += 1;
}
int func2_760(){
	int ret=0;
	if(globalVariable2_1== 760){
		ret=func2_761();
		return ret;
	}else{
			return 0;
	}
}
void call2_759(){
	globalVariable2_1 += 1;
}
int func2_759(){
	int ret=0;
	if(globalVariable2_1== 759){
		ret=func2_760();
		return ret;
	}else{
			return 0;
	}
}
void call2_758(){
	globalVariable2_1 += 1;
}
int func2_758(){
	int ret=0;
	if(globalVariable2_1== 758){
		ret=func2_759();
		return ret;
	}else{
			return 0;
	}
}
void call2_757(){
	globalVariable2_1 += 1;
}
int func2_757(){
	int ret=0;
	if(globalVariable2_1== 757){
		ret=func2_758();
		return ret;
	}else{
			return 0;
	}
}
void call2_756(){
	globalVariable2_1 += 1;
}
int func2_756(){
	int ret=0;
	if(globalVariable2_1== 756){
		ret=func2_757();
		return ret;
	}else{
			return 0;
	}
}
void call2_755(){
	globalVariable2_1 += 1;
}
int func2_755(){
	int ret=0;
	if(globalVariable2_1== 755){
		ret=func2_756();
		return ret;
	}else{
			return 0;
	}
}
void call2_754(){
	globalVariable2_1 += 1;
}
int func2_754(){
	int ret=0;
	if(globalVariable2_1== 754){
		ret=func2_755();
		return ret;
	}else{
			return 0;
	}
}
void call2_753(){
	globalVariable2_1 += 1;
}
int func2_753(){
	int ret=0;
	if(globalVariable2_1== 753){
		ret=func2_754();
		return ret;
	}else{
			return 0;
	}
}
void call2_752(){
	globalVariable2_1 += 1;
}
int func2_752(){
	int ret=0;
	if(globalVariable2_1== 752){
		ret=func2_753();
		return ret;
	}else{
			return 0;
	}
}
void call2_751(){
	globalVariable2_1 += 1;
}
int func2_751(){
	int ret=0;
	if(globalVariable2_1== 751){
		ret=func2_752();
		return ret;
	}else{
			return 0;
	}
}
void call2_750(){
	globalVariable2_1 += 1;
}
int func2_750(){
	int ret=0;
	if(globalVariable2_1== 750){
		ret=func2_751();
		return ret;
	}else{
			return 0;
	}
}
void call2_749(){
	globalVariable2_1 += 1;
}
int func2_749(){
	int ret=0;
	if(globalVariable2_1== 749){
		ret=func2_750();
		return ret;
	}else{
			return 0;
	}
}
void call2_748(){
	globalVariable2_1 += 1;
}
int func2_748(){
	int ret=0;
	if(globalVariable2_1== 748){
		ret=func2_749();
		return ret;
	}else{
			return 0;
	}
}
void call2_747(){
	globalVariable2_1 += 1;
}
int func2_747(){
	int ret=0;
	if(globalVariable2_1== 747){
		ret=func2_748();
		return ret;
	}else{
			return 0;
	}
}
void call2_746(){
	globalVariable2_1 += 1;
}
int func2_746(){
	int ret=0;
	if(globalVariable2_1== 746){
		ret=func2_747();
		return ret;
	}else{
			return 0;
	}
}
void call2_745(){
	globalVariable2_1 += 1;
}
int func2_745(){
	int ret=0;
	if(globalVariable2_1== 745){
		ret=func2_746();
		return ret;
	}else{
			return 0;
	}
}
void call2_744(){
	globalVariable2_1 += 1;
}
int func2_744(){
	int ret=0;
	if(globalVariable2_1== 744){
		ret=func2_745();
		return ret;
	}else{
			return 0;
	}
}
void call2_743(){
	globalVariable2_1 += 1;
}
int func2_743(){
	int ret=0;
	if(globalVariable2_1== 743){
		ret=func2_744();
		return ret;
	}else{
			return 0;
	}
}
void call2_742(){
	globalVariable2_1 += 1;
}
int func2_742(){
	int ret=0;
	if(globalVariable2_1== 742){
		ret=func2_743();
		return ret;
	}else{
			return 0;
	}
}
void call2_741(){
	globalVariable2_1 += 1;
}
int func2_741(){
	int ret=0;
	if(globalVariable2_1== 741){
		ret=func2_742();
		return ret;
	}else{
			return 0;
	}
}
void call2_740(){
	globalVariable2_1 += 1;
}
int func2_740(){
	int ret=0;
	if(globalVariable2_1== 740){
		ret=func2_741();
		return ret;
	}else{
			return 0;
	}
}
void call2_739(){
	globalVariable2_1 += 1;
}
int func2_739(){
	int ret=0;
	if(globalVariable2_1== 739){
		ret=func2_740();
		return ret;
	}else{
			return 0;
	}
}
void call2_738(){
	globalVariable2_1 += 1;
}
int func2_738(){
	int ret=0;
	if(globalVariable2_1== 738){
		ret=func2_739();
		return ret;
	}else{
			return 0;
	}
}
void call2_737(){
	globalVariable2_1 += 1;
}
int func2_737(){
	int ret=0;
	if(globalVariable2_1== 737){
		ret=func2_738();
		return ret;
	}else{
			return 0;
	}
}
void call2_736(){
	globalVariable2_1 += 1;
}
int func2_736(){
	int ret=0;
	if(globalVariable2_1== 736){
		ret=func2_737();
		return ret;
	}else{
			return 0;
	}
}
void call2_735(){
	globalVariable2_1 += 1;
}
int func2_735(){
	int ret=0;
	if(globalVariable2_1== 735){
		ret=func2_736();
		return ret;
	}else{
			return 0;
	}
}
void call2_734(){
	globalVariable2_1 += 1;
}
int func2_734(){
	int ret=0;
	if(globalVariable2_1== 734){
		ret=func2_735();
		return ret;
	}else{
			return 0;
	}
}
void call2_733(){
	globalVariable2_1 += 1;
}
int func2_733(){
	int ret=0;
	if(globalVariable2_1== 733){
		ret=func2_734();
		return ret;
	}else{
			return 0;
	}
}
void call2_732(){
	globalVariable2_1 += 1;
}
int func2_732(){
	int ret=0;
	if(globalVariable2_1== 732){
		ret=func2_733();
		return ret;
	}else{
			return 0;
	}
}
void call2_731(){
	globalVariable2_1 += 1;
}
int func2_731(){
	int ret=0;
	if(globalVariable2_1== 731){
		ret=func2_732();
		return ret;
	}else{
			return 0;
	}
}
void call2_730(){
	globalVariable2_1 += 1;
}
int func2_730(){
	int ret=0;
	if(globalVariable2_1== 730){
		ret=func2_731();
		return ret;
	}else{
			return 0;
	}
}
void call2_729(){
	globalVariable2_1 += 1;
}
int func2_729(){
	int ret=0;
	if(globalVariable2_1== 729){
		ret=func2_730();
		return ret;
	}else{
			return 0;
	}
}
void call2_728(){
	globalVariable2_1 += 1;
}
int func2_728(){
	int ret=0;
	if(globalVariable2_1== 728){
		ret=func2_729();
		return ret;
	}else{
			return 0;
	}
}
void call2_727(){
	globalVariable2_1 += 1;
}
int func2_727(){
	int ret=0;
	if(globalVariable2_1== 727){
		ret=func2_728();
		return ret;
	}else{
			return 0;
	}
}
void call2_726(){
	globalVariable2_1 += 1;
}
int func2_726(){
	int ret=0;
	if(globalVariable2_1== 726){
		ret=func2_727();
		return ret;
	}else{
			return 0;
	}
}
void call2_725(){
	globalVariable2_1 += 1;
}
int func2_725(){
	int ret=0;
	if(globalVariable2_1== 725){
		ret=func2_726();
		return ret;
	}else{
			return 0;
	}
}
void call2_724(){
	globalVariable2_1 += 1;
}
int func2_724(){
	int ret=0;
	if(globalVariable2_1== 724){
		ret=func2_725();
		return ret;
	}else{
			return 0;
	}
}
void call2_723(){
	globalVariable2_1 += 1;
}
int func2_723(){
	int ret=0;
	if(globalVariable2_1== 723){
		ret=func2_724();
		return ret;
	}else{
			return 0;
	}
}
void call2_722(){
	globalVariable2_1 += 1;
}
int func2_722(){
	int ret=0;
	if(globalVariable2_1== 722){
		ret=func2_723();
		return ret;
	}else{
			return 0;
	}
}
void call2_721(){
	globalVariable2_1 += 1;
}
int func2_721(){
	int ret=0;
	if(globalVariable2_1== 721){
		ret=func2_722();
		return ret;
	}else{
			return 0;
	}
}
void call2_720(){
	globalVariable2_1 += 1;
}
int func2_720(){
	int ret=0;
	if(globalVariable2_1== 720){
		ret=func2_721();
		return ret;
	}else{
			return 0;
	}
}
void call2_719(){
	globalVariable2_1 += 1;
}
int func2_719(){
	int ret=0;
	if(globalVariable2_1== 719){
		ret=func2_720();
		return ret;
	}else{
			return 0;
	}
}
void call2_718(){
	globalVariable2_1 += 1;
}
int func2_718(){
	int ret=0;
	if(globalVariable2_1== 718){
		ret=func2_719();
		return ret;
	}else{
			return 0;
	}
}
void call2_717(){
	globalVariable2_1 += 1;
}
int func2_717(){
	int ret=0;
	if(globalVariable2_1== 717){
		ret=func2_718();
		return ret;
	}else{
			return 0;
	}
}
void call2_716(){
	globalVariable2_1 += 1;
}
int func2_716(){
	int ret=0;
	if(globalVariable2_1== 716){
		ret=func2_717();
		return ret;
	}else{
			return 0;
	}
}
void call2_715(){
	globalVariable2_1 += 1;
}
int func2_715(){
	int ret=0;
	if(globalVariable2_1== 715){
		ret=func2_716();
		return ret;
	}else{
			return 0;
	}
}
void call2_714(){
	globalVariable2_1 += 1;
}
int func2_714(){
	int ret=0;
	if(globalVariable2_1== 714){
		ret=func2_715();
		return ret;
	}else{
			return 0;
	}
}
void call2_713(){
	globalVariable2_1 += 1;
}
int func2_713(){
	int ret=0;
	if(globalVariable2_1== 713){
		ret=func2_714();
		return ret;
	}else{
			return 0;
	}
}
void call2_712(){
	globalVariable2_1 += 1;
}
int func2_712(){
	int ret=0;
	if(globalVariable2_1== 712){
		ret=func2_713();
		return ret;
	}else{
			return 0;
	}
}
void call2_711(){
	globalVariable2_1 += 1;
}
int func2_711(){
	int ret=0;
	if(globalVariable2_1== 711){
		ret=func2_712();
		return ret;
	}else{
			return 0;
	}
}
void call2_710(){
	globalVariable2_1 += 1;
}
int func2_710(){
	int ret=0;
	if(globalVariable2_1== 710){
		ret=func2_711();
		return ret;
	}else{
			return 0;
	}
}
void call2_709(){
	globalVariable2_1 += 1;
}
int func2_709(){
	int ret=0;
	if(globalVariable2_1== 709){
		ret=func2_710();
		return ret;
	}else{
			return 0;
	}
}
void call2_708(){
	globalVariable2_1 += 1;
}
int func2_708(){
	int ret=0;
	if(globalVariable2_1== 708){
		ret=func2_709();
		return ret;
	}else{
			return 0;
	}
}
void call2_707(){
	globalVariable2_1 += 1;
}
int func2_707(){
	int ret=0;
	if(globalVariable2_1== 707){
		ret=func2_708();
		return ret;
	}else{
			return 0;
	}
}
void call2_706(){
	globalVariable2_1 += 1;
}
int func2_706(){
	int ret=0;
	if(globalVariable2_1== 706){
		ret=func2_707();
		return ret;
	}else{
			return 0;
	}
}
void call2_705(){
	globalVariable2_1 += 1;
}
int func2_705(){
	int ret=0;
	if(globalVariable2_1== 705){
		ret=func2_706();
		return ret;
	}else{
			return 0;
	}
}
void call2_704(){
	globalVariable2_1 += 1;
}
int func2_704(){
	int ret=0;
	if(globalVariable2_1== 704){
		ret=func2_705();
		return ret;
	}else{
			return 0;
	}
}
void call2_703(){
	globalVariable2_1 += 1;
}
int func2_703(){
	int ret=0;
	if(globalVariable2_1== 703){
		ret=func2_704();
		return ret;
	}else{
			return 0;
	}
}
void call2_702(){
	globalVariable2_1 += 1;
}
int func2_702(){
	int ret=0;
	if(globalVariable2_1== 702){
		ret=func2_703();
		return ret;
	}else{
			return 0;
	}
}
void call2_701(){
	globalVariable2_1 += 1;
}
int func2_701(){
	int ret=0;
	if(globalVariable2_1== 701){
		ret=func2_702();
		return ret;
	}else{
			return 0;
	}
}
void call2_700(){
	globalVariable2_1 += 1;
}
int func2_700(){
	int ret=0;
	if(globalVariable2_1== 700){
		ret=func2_701();
		return ret;
	}else{
			return 0;
	}
}
void call2_699(){
	globalVariable2_1 += 1;
}
int func2_699(){
	int ret=0;
	if(globalVariable2_1== 699){
		ret=func2_700();
		return ret;
	}else{
			return 0;
	}
}
void call2_698(){
	globalVariable2_1 += 1;
}
int func2_698(){
	int ret=0;
	if(globalVariable2_1== 698){
		ret=func2_699();
		return ret;
	}else{
			return 0;
	}
}
void call2_697(){
	globalVariable2_1 += 1;
}
int func2_697(){
	int ret=0;
	if(globalVariable2_1== 697){
		ret=func2_698();
		return ret;
	}else{
			return 0;
	}
}
void call2_696(){
	globalVariable2_1 += 1;
}
int func2_696(){
	int ret=0;
	if(globalVariable2_1== 696){
		ret=func2_697();
		return ret;
	}else{
			return 0;
	}
}
void call2_695(){
	globalVariable2_1 += 1;
}
int func2_695(){
	int ret=0;
	if(globalVariable2_1== 695){
		ret=func2_696();
		return ret;
	}else{
			return 0;
	}
}
void call2_694(){
	globalVariable2_1 += 1;
}
int func2_694(){
	int ret=0;
	if(globalVariable2_1== 694){
		ret=func2_695();
		return ret;
	}else{
			return 0;
	}
}
void call2_693(){
	globalVariable2_1 += 1;
}
int func2_693(){
	int ret=0;
	if(globalVariable2_1== 693){
		ret=func2_694();
		return ret;
	}else{
			return 0;
	}
}
void call2_692(){
	globalVariable2_1 += 1;
}
int func2_692(){
	int ret=0;
	if(globalVariable2_1== 692){
		ret=func2_693();
		return ret;
	}else{
			return 0;
	}
}
void call2_691(){
	globalVariable2_1 += 1;
}
int func2_691(){
	int ret=0;
	if(globalVariable2_1== 691){
		ret=func2_692();
		return ret;
	}else{
			return 0;
	}
}
void call2_690(){
	globalVariable2_1 += 1;
}
int func2_690(){
	int ret=0;
	if(globalVariable2_1== 690){
		ret=func2_691();
		return ret;
	}else{
			return 0;
	}
}
void call2_689(){
	globalVariable2_1 += 1;
}
int func2_689(){
	int ret=0;
	if(globalVariable2_1== 689){
		ret=func2_690();
		return ret;
	}else{
			return 0;
	}
}
void call2_688(){
	globalVariable2_1 += 1;
}
int func2_688(){
	int ret=0;
	if(globalVariable2_1== 688){
		ret=func2_689();
		return ret;
	}else{
			return 0;
	}
}
void call2_687(){
	globalVariable2_1 += 1;
}
int func2_687(){
	int ret=0;
	if(globalVariable2_1== 687){
		ret=func2_688();
		return ret;
	}else{
			return 0;
	}
}
void call2_686(){
	globalVariable2_1 += 1;
}
int func2_686(){
	int ret=0;
	if(globalVariable2_1== 686){
		ret=func2_687();
		return ret;
	}else{
			return 0;
	}
}
void call2_685(){
	globalVariable2_1 += 1;
}
int func2_685(){
	int ret=0;
	if(globalVariable2_1== 685){
		ret=func2_686();
		return ret;
	}else{
			return 0;
	}
}
void call2_684(){
	globalVariable2_1 += 1;
}
int func2_684(){
	int ret=0;
	if(globalVariable2_1== 684){
		ret=func2_685();
		return ret;
	}else{
			return 0;
	}
}
void call2_683(){
	globalVariable2_1 += 1;
}
int func2_683(){
	int ret=0;
	if(globalVariable2_1== 683){
		ret=func2_684();
		return ret;
	}else{
			return 0;
	}
}
void call2_682(){
	globalVariable2_1 += 1;
}
int func2_682(){
	int ret=0;
	if(globalVariable2_1== 682){
		ret=func2_683();
		return ret;
	}else{
			return 0;
	}
}
void call2_681(){
	globalVariable2_1 += 1;
}
int func2_681(){
	int ret=0;
	if(globalVariable2_1== 681){
		ret=func2_682();
		return ret;
	}else{
			return 0;
	}
}
void call2_680(){
	globalVariable2_1 += 1;
}
int func2_680(){
	int ret=0;
	if(globalVariable2_1== 680){
		ret=func2_681();
		return ret;
	}else{
			return 0;
	}
}
void call2_679(){
	globalVariable2_1 += 1;
}
int func2_679(){
	int ret=0;
	if(globalVariable2_1== 679){
		ret=func2_680();
		return ret;
	}else{
			return 0;
	}
}
void call2_678(){
	globalVariable2_1 += 1;
}
int func2_678(){
	int ret=0;
	if(globalVariable2_1== 678){
		ret=func2_679();
		return ret;
	}else{
			return 0;
	}
}
void call2_677(){
	globalVariable2_1 += 1;
}
int func2_677(){
	int ret=0;
	if(globalVariable2_1== 677){
		ret=func2_678();
		return ret;
	}else{
			return 0;
	}
}
void call2_676(){
	globalVariable2_1 += 1;
}
int func2_676(){
	int ret=0;
	if(globalVariable2_1== 676){
		ret=func2_677();
		return ret;
	}else{
			return 0;
	}
}
void call2_675(){
	globalVariable2_1 += 1;
}
int func2_675(){
	int ret=0;
	if(globalVariable2_1== 675){
		ret=func2_676();
		return ret;
	}else{
			return 0;
	}
}
void call2_674(){
	globalVariable2_1 += 1;
}
int func2_674(){
	int ret=0;
	if(globalVariable2_1== 674){
		ret=func2_675();
		return ret;
	}else{
			return 0;
	}
}
void call2_673(){
	globalVariable2_1 += 1;
}
int func2_673(){
	int ret=0;
	if(globalVariable2_1== 673){
		ret=func2_674();
		return ret;
	}else{
			return 0;
	}
}
void call2_672(){
	globalVariable2_1 += 1;
}
int func2_672(){
	int ret=0;
	if(globalVariable2_1== 672){
		ret=func2_673();
		return ret;
	}else{
			return 0;
	}
}
void call2_671(){
	globalVariable2_1 += 1;
}
int func2_671(){
	int ret=0;
	if(globalVariable2_1== 671){
		ret=func2_672();
		return ret;
	}else{
			return 0;
	}
}
void call2_670(){
	globalVariable2_1 += 1;
}
int func2_670(){
	int ret=0;
	if(globalVariable2_1== 670){
		ret=func2_671();
		return ret;
	}else{
			return 0;
	}
}
void call2_669(){
	globalVariable2_1 += 1;
}
int func2_669(){
	int ret=0;
	if(globalVariable2_1== 669){
		ret=func2_670();
		return ret;
	}else{
			return 0;
	}
}
void call2_668(){
	globalVariable2_1 += 1;
}
int func2_668(){
	int ret=0;
	if(globalVariable2_1== 668){
		ret=func2_669();
		return ret;
	}else{
			return 0;
	}
}
void call2_667(){
	globalVariable2_1 += 1;
}
int func2_667(){
	int ret=0;
	if(globalVariable2_1== 667){
		ret=func2_668();
		return ret;
	}else{
			return 0;
	}
}
void call2_666(){
	globalVariable2_1 += 1;
}
int func2_666(){
	int ret=0;
	if(globalVariable2_1== 666){
		ret=func2_667();
		return ret;
	}else{
			return 0;
	}
}
void call2_665(){
	globalVariable2_1 += 1;
}
int func2_665(){
	int ret=0;
	if(globalVariable2_1== 665){
		ret=func2_666();
		return ret;
	}else{
			return 0;
	}
}
void call2_664(){
	globalVariable2_1 += 1;
}
int func2_664(){
	int ret=0;
	if(globalVariable2_1== 664){
		ret=func2_665();
		return ret;
	}else{
			return 0;
	}
}
void call2_663(){
	globalVariable2_1 += 1;
}
int func2_663(){
	int ret=0;
	if(globalVariable2_1== 663){
		ret=func2_664();
		return ret;
	}else{
			return 0;
	}
}
void call2_662(){
	globalVariable2_1 += 1;
}
int func2_662(){
	int ret=0;
	if(globalVariable2_1== 662){
		ret=func2_663();
		return ret;
	}else{
			return 0;
	}
}
void call2_661(){
	globalVariable2_1 += 1;
}
int func2_661(){
	int ret=0;
	if(globalVariable2_1== 661){
		ret=func2_662();
		return ret;
	}else{
			return 0;
	}
}
void call2_660(){
	globalVariable2_1 += 1;
}
int func2_660(){
	int ret=0;
	if(globalVariable2_1== 660){
		ret=func2_661();
		return ret;
	}else{
			return 0;
	}
}
void call2_659(){
	globalVariable2_1 += 1;
}
int func2_659(){
	int ret=0;
	if(globalVariable2_1== 659){
		ret=func2_660();
		return ret;
	}else{
			return 0;
	}
}
void call2_658(){
	globalVariable2_1 += 1;
}
int func2_658(){
	int ret=0;
	if(globalVariable2_1== 658){
		ret=func2_659();
		return ret;
	}else{
			return 0;
	}
}
void call2_657(){
	globalVariable2_1 += 1;
}
int func2_657(){
	int ret=0;
	if(globalVariable2_1== 657){
		ret=func2_658();
		return ret;
	}else{
			return 0;
	}
}
void call2_656(){
	globalVariable2_1 += 1;
}
int func2_656(){
	int ret=0;
	if(globalVariable2_1== 656){
		ret=func2_657();
		return ret;
	}else{
			return 0;
	}
}
void call2_655(){
	globalVariable2_1 += 1;
}
int func2_655(){
	int ret=0;
	if(globalVariable2_1== 655){
		ret=func2_656();
		return ret;
	}else{
			return 0;
	}
}
void call2_654(){
	globalVariable2_1 += 1;
}
int func2_654(){
	int ret=0;
	if(globalVariable2_1== 654){
		ret=func2_655();
		return ret;
	}else{
			return 0;
	}
}
void call2_653(){
	globalVariable2_1 += 1;
}
int func2_653(){
	int ret=0;
	if(globalVariable2_1== 653){
		ret=func2_654();
		return ret;
	}else{
			return 0;
	}
}
void call2_652(){
	globalVariable2_1 += 1;
}
int func2_652(){
	int ret=0;
	if(globalVariable2_1== 652){
		ret=func2_653();
		return ret;
	}else{
			return 0;
	}
}
void call2_651(){
	globalVariable2_1 += 1;
}
int func2_651(){
	int ret=0;
	if(globalVariable2_1== 651){
		ret=func2_652();
		return ret;
	}else{
			return 0;
	}
}
void call2_650(){
	globalVariable2_1 += 1;
}
int func2_650(){
	int ret=0;
	if(globalVariable2_1== 650){
		ret=func2_651();
		return ret;
	}else{
			return 0;
	}
}
void call2_649(){
	globalVariable2_1 += 1;
}
int func2_649(){
	int ret=0;
	if(globalVariable2_1== 649){
		ret=func2_650();
		return ret;
	}else{
			return 0;
	}
}
void call2_648(){
	globalVariable2_1 += 1;
}
int func2_648(){
	int ret=0;
	if(globalVariable2_1== 648){
		ret=func2_649();
		return ret;
	}else{
			return 0;
	}
}
void call2_647(){
	globalVariable2_1 += 1;
}
int func2_647(){
	int ret=0;
	if(globalVariable2_1== 647){
		ret=func2_648();
		return ret;
	}else{
			return 0;
	}
}
void call2_646(){
	globalVariable2_1 += 1;
}
int func2_646(){
	int ret=0;
	if(globalVariable2_1== 646){
		ret=func2_647();
		return ret;
	}else{
			return 0;
	}
}
void call2_645(){
	globalVariable2_1 += 1;
}
int func2_645(){
	int ret=0;
	if(globalVariable2_1== 645){
		ret=func2_646();
		return ret;
	}else{
			return 0;
	}
}
void call2_644(){
	globalVariable2_1 += 1;
}
int func2_644(){
	int ret=0;
	if(globalVariable2_1== 644){
		ret=func2_645();
		return ret;
	}else{
			return 0;
	}
}
void call2_643(){
	globalVariable2_1 += 1;
}
int func2_643(){
	int ret=0;
	if(globalVariable2_1== 643){
		ret=func2_644();
		return ret;
	}else{
			return 0;
	}
}
void call2_642(){
	globalVariable2_1 += 1;
}
int func2_642(){
	int ret=0;
	if(globalVariable2_1== 642){
		ret=func2_643();
		return ret;
	}else{
			return 0;
	}
}
void call2_641(){
	globalVariable2_1 += 1;
}
int func2_641(){
	int ret=0;
	if(globalVariable2_1== 641){
		ret=func2_642();
		return ret;
	}else{
			return 0;
	}
}
void call2_640(){
	globalVariable2_1 += 1;
}
int func2_640(){
	int ret=0;
	if(globalVariable2_1== 640){
		ret=func2_641();
		return ret;
	}else{
			return 0;
	}
}
void call2_639(){
	globalVariable2_1 += 1;
}
int func2_639(){
	int ret=0;
	if(globalVariable2_1== 639){
		ret=func2_640();
		return ret;
	}else{
			return 0;
	}
}
void call2_638(){
	globalVariable2_1 += 1;
}
int func2_638(){
	int ret=0;
	if(globalVariable2_1== 638){
		ret=func2_639();
		return ret;
	}else{
			return 0;
	}
}
void call2_637(){
	globalVariable2_1 += 1;
}
int func2_637(){
	int ret=0;
	if(globalVariable2_1== 637){
		ret=func2_638();
		return ret;
	}else{
			return 0;
	}
}
void call2_636(){
	globalVariable2_1 += 1;
}
int func2_636(){
	int ret=0;
	if(globalVariable2_1== 636){
		ret=func2_637();
		return ret;
	}else{
			return 0;
	}
}
void call2_635(){
	globalVariable2_1 += 1;
}
int func2_635(){
	int ret=0;
	if(globalVariable2_1== 635){
		ret=func2_636();
		return ret;
	}else{
			return 0;
	}
}
void call2_634(){
	globalVariable2_1 += 1;
}
int func2_634(){
	int ret=0;
	if(globalVariable2_1== 634){
		ret=func2_635();
		return ret;
	}else{
			return 0;
	}
}
void call2_633(){
	globalVariable2_1 += 1;
}
int func2_633(){
	int ret=0;
	if(globalVariable2_1== 633){
		ret=func2_634();
		return ret;
	}else{
			return 0;
	}
}
void call2_632(){
	globalVariable2_1 += 1;
}
int func2_632(){
	int ret=0;
	if(globalVariable2_1== 632){
		ret=func2_633();
		return ret;
	}else{
			return 0;
	}
}
void call2_631(){
	globalVariable2_1 += 1;
}
int func2_631(){
	int ret=0;
	if(globalVariable2_1== 631){
		ret=func2_632();
		return ret;
	}else{
			return 0;
	}
}
void call2_630(){
	globalVariable2_1 += 1;
}
int func2_630(){
	int ret=0;
	if(globalVariable2_1== 630){
		ret=func2_631();
		return ret;
	}else{
			return 0;
	}
}
void call2_629(){
	globalVariable2_1 += 1;
}
int func2_629(){
	int ret=0;
	if(globalVariable2_1== 629){
		ret=func2_630();
		return ret;
	}else{
			return 0;
	}
}
void call2_628(){
	globalVariable2_1 += 1;
}
int func2_628(){
	int ret=0;
	if(globalVariable2_1== 628){
		ret=func2_629();
		return ret;
	}else{
			return 0;
	}
}
void call2_627(){
	globalVariable2_1 += 1;
}
int func2_627(){
	int ret=0;
	if(globalVariable2_1== 627){
		ret=func2_628();
		return ret;
	}else{
			return 0;
	}
}
void call2_626(){
	globalVariable2_1 += 1;
}
int func2_626(){
	int ret=0;
	if(globalVariable2_1== 626){
		ret=func2_627();
		return ret;
	}else{
			return 0;
	}
}
void call2_625(){
	globalVariable2_1 += 1;
}
int func2_625(){
	int ret=0;
	if(globalVariable2_1== 625){
		ret=func2_626();
		return ret;
	}else{
			return 0;
	}
}
void call2_624(){
	globalVariable2_1 += 1;
}
int func2_624(){
	int ret=0;
	if(globalVariable2_1== 624){
		ret=func2_625();
		return ret;
	}else{
			return 0;
	}
}
void call2_623(){
	globalVariable2_1 += 1;
}
int func2_623(){
	int ret=0;
	if(globalVariable2_1== 623){
		ret=func2_624();
		return ret;
	}else{
			return 0;
	}
}
void call2_622(){
	globalVariable2_1 += 1;
}
int func2_622(){
	int ret=0;
	if(globalVariable2_1== 622){
		ret=func2_623();
		return ret;
	}else{
			return 0;
	}
}
void call2_621(){
	globalVariable2_1 += 1;
}
int func2_621(){
	int ret=0;
	if(globalVariable2_1== 621){
		ret=func2_622();
		return ret;
	}else{
			return 0;
	}
}
void call2_620(){
	globalVariable2_1 += 1;
}
int func2_620(){
	int ret=0;
	if(globalVariable2_1== 620){
		ret=func2_621();
		return ret;
	}else{
			return 0;
	}
}
void call2_619(){
	globalVariable2_1 += 1;
}
int func2_619(){
	int ret=0;
	if(globalVariable2_1== 619){
		ret=func2_620();
		return ret;
	}else{
			return 0;
	}
}
void call2_618(){
	globalVariable2_1 += 1;
}
int func2_618(){
	int ret=0;
	if(globalVariable2_1== 618){
		ret=func2_619();
		return ret;
	}else{
			return 0;
	}
}
void call2_617(){
	globalVariable2_1 += 1;
}
int func2_617(){
	int ret=0;
	if(globalVariable2_1== 617){
		ret=func2_618();
		return ret;
	}else{
			return 0;
	}
}
void call2_616(){
	globalVariable2_1 += 1;
}
int func2_616(){
	int ret=0;
	if(globalVariable2_1== 616){
		ret=func2_617();
		return ret;
	}else{
			return 0;
	}
}
void call2_615(){
	globalVariable2_1 += 1;
}
int func2_615(){
	int ret=0;
	if(globalVariable2_1== 615){
		ret=func2_616();
		return ret;
	}else{
			return 0;
	}
}
void call2_614(){
	globalVariable2_1 += 1;
}
int func2_614(){
	int ret=0;
	if(globalVariable2_1== 614){
		ret=func2_615();
		return ret;
	}else{
			return 0;
	}
}
void call2_613(){
	globalVariable2_1 += 1;
}
int func2_613(){
	int ret=0;
	if(globalVariable2_1== 613){
		ret=func2_614();
		return ret;
	}else{
			return 0;
	}
}
void call2_612(){
	globalVariable2_1 += 1;
}
int func2_612(){
	int ret=0;
	if(globalVariable2_1== 612){
		ret=func2_613();
		return ret;
	}else{
			return 0;
	}
}
void call2_611(){
	globalVariable2_1 += 1;
}
int func2_611(){
	int ret=0;
	if(globalVariable2_1== 611){
		ret=func2_612();
		return ret;
	}else{
			return 0;
	}
}
void call2_610(){
	globalVariable2_1 += 1;
}
int func2_610(){
	int ret=0;
	if(globalVariable2_1== 610){
		ret=func2_611();
		return ret;
	}else{
			return 0;
	}
}
void call2_609(){
	globalVariable2_1 += 1;
}
int func2_609(){
	int ret=0;
	if(globalVariable2_1== 609){
		ret=func2_610();
		return ret;
	}else{
			return 0;
	}
}
void call2_608(){
	globalVariable2_1 += 1;
}
int func2_608(){
	int ret=0;
	if(globalVariable2_1== 608){
		ret=func2_609();
		return ret;
	}else{
			return 0;
	}
}
void call2_607(){
	globalVariable2_1 += 1;
}
int func2_607(){
	int ret=0;
	if(globalVariable2_1== 607){
		ret=func2_608();
		return ret;
	}else{
			return 0;
	}
}
void call2_606(){
	globalVariable2_1 += 1;
}
int func2_606(){
	int ret=0;
	if(globalVariable2_1== 606){
		ret=func2_607();
		return ret;
	}else{
			return 0;
	}
}
void call2_605(){
	globalVariable2_1 += 1;
}
int func2_605(){
	int ret=0;
	if(globalVariable2_1== 605){
		ret=func2_606();
		return ret;
	}else{
			return 0;
	}
}
void call2_604(){
	globalVariable2_1 += 1;
}
int func2_604(){
	int ret=0;
	if(globalVariable2_1== 604){
		ret=func2_605();
		return ret;
	}else{
			return 0;
	}
}
void call2_603(){
	globalVariable2_1 += 1;
}
int func2_603(){
	int ret=0;
	if(globalVariable2_1== 603){
		ret=func2_604();
		return ret;
	}else{
			return 0;
	}
}
void call2_602(){
	globalVariable2_1 += 1;
}
int func2_602(){
	int ret=0;
	if(globalVariable2_1== 602){
		ret=func2_603();
		return ret;
	}else{
			return 0;
	}
}
void call2_601(){
	globalVariable2_1 += 1;
}
int func2_601(){
	int ret=0;
	if(globalVariable2_1== 601){
		ret=func2_602();
		return ret;
	}else{
			return 0;
	}
}
void call2_600(){
	globalVariable2_1 += 1;
}
int func2_600(){
	int ret=0;
	if(globalVariable2_1== 600){
		ret=func2_601();
		return ret;
	}else{
			return 0;
	}
}
void call2_599(){
	globalVariable2_1 += 1;
}
int func2_599(){
	int ret=0;
	if(globalVariable2_1== 599){
		ret=func2_600();
		return ret;
	}else{
			return 0;
	}
}
void call2_598(){
	globalVariable2_1 += 1;
}
int func2_598(){
	int ret=0;
	if(globalVariable2_1== 598){
		ret=func2_599();
		return ret;
	}else{
			return 0;
	}
}
void call2_597(){
	globalVariable2_1 += 1;
}
int func2_597(){
	int ret=0;
	if(globalVariable2_1== 597){
		ret=func2_598();
		return ret;
	}else{
			return 0;
	}
}
void call2_596(){
	globalVariable2_1 += 1;
}
int func2_596(){
	int ret=0;
	if(globalVariable2_1== 596){
		ret=func2_597();
		return ret;
	}else{
			return 0;
	}
}
void call2_595(){
	globalVariable2_1 += 1;
}
int func2_595(){
	int ret=0;
	if(globalVariable2_1== 595){
		ret=func2_596();
		return ret;
	}else{
			return 0;
	}
}
void call2_594(){
	globalVariable2_1 += 1;
}
int func2_594(){
	int ret=0;
	if(globalVariable2_1== 594){
		ret=func2_595();
		return ret;
	}else{
			return 0;
	}
}
void call2_593(){
	globalVariable2_1 += 1;
}
int func2_593(){
	int ret=0;
	if(globalVariable2_1== 593){
		ret=func2_594();
		return ret;
	}else{
			return 0;
	}
}
void call2_592(){
	globalVariable2_1 += 1;
}
int func2_592(){
	int ret=0;
	if(globalVariable2_1== 592){
		ret=func2_593();
		return ret;
	}else{
			return 0;
	}
}
void call2_591(){
	globalVariable2_1 += 1;
}
int func2_591(){
	int ret=0;
	if(globalVariable2_1== 591){
		ret=func2_592();
		return ret;
	}else{
			return 0;
	}
}
void call2_590(){
	globalVariable2_1 += 1;
}
int func2_590(){
	int ret=0;
	if(globalVariable2_1== 590){
		ret=func2_591();
		return ret;
	}else{
			return 0;
	}
}
void call2_589(){
	globalVariable2_1 += 1;
}
int func2_589(){
	int ret=0;
	if(globalVariable2_1== 589){
		ret=func2_590();
		return ret;
	}else{
			return 0;
	}
}
void call2_588(){
	globalVariable2_1 += 1;
}
int func2_588(){
	int ret=0;
	if(globalVariable2_1== 588){
		ret=func2_589();
		return ret;
	}else{
			return 0;
	}
}
void call2_587(){
	globalVariable2_1 += 1;
}
int func2_587(){
	int ret=0;
	if(globalVariable2_1== 587){
		ret=func2_588();
		return ret;
	}else{
			return 0;
	}
}
void call2_586(){
	globalVariable2_1 += 1;
}
int func2_586(){
	int ret=0;
	if(globalVariable2_1== 586){
		ret=func2_587();
		return ret;
	}else{
			return 0;
	}
}
void call2_585(){
	globalVariable2_1 += 1;
}
int func2_585(){
	int ret=0;
	if(globalVariable2_1== 585){
		ret=func2_586();
		return ret;
	}else{
			return 0;
	}
}
void call2_584(){
	globalVariable2_1 += 1;
}
int func2_584(){
	int ret=0;
	if(globalVariable2_1== 584){
		ret=func2_585();
		return ret;
	}else{
			return 0;
	}
}
void call2_583(){
	globalVariable2_1 += 1;
}
int func2_583(){
	int ret=0;
	if(globalVariable2_1== 583){
		ret=func2_584();
		return ret;
	}else{
			return 0;
	}
}
void call2_582(){
	globalVariable2_1 += 1;
}
int func2_582(){
	int ret=0;
	if(globalVariable2_1== 582){
		ret=func2_583();
		return ret;
	}else{
			return 0;
	}
}
void call2_581(){
	globalVariable2_1 += 1;
}
int func2_581(){
	int ret=0;
	if(globalVariable2_1== 581){
		ret=func2_582();
		return ret;
	}else{
			return 0;
	}
}
void call2_580(){
	globalVariable2_1 += 1;
}
int func2_580(){
	int ret=0;
	if(globalVariable2_1== 580){
		ret=func2_581();
		return ret;
	}else{
			return 0;
	}
}
void call2_579(){
	globalVariable2_1 += 1;
}
int func2_579(){
	int ret=0;
	if(globalVariable2_1== 579){
		ret=func2_580();
		return ret;
	}else{
			return 0;
	}
}
void call2_578(){
	globalVariable2_1 += 1;
}
int func2_578(){
	int ret=0;
	if(globalVariable2_1== 578){
		ret=func2_579();
		return ret;
	}else{
			return 0;
	}
}
void call2_577(){
	globalVariable2_1 += 1;
}
int func2_577(){
	int ret=0;
	if(globalVariable2_1== 577){
		ret=func2_578();
		return ret;
	}else{
			return 0;
	}
}
void call2_576(){
	globalVariable2_1 += 1;
}
int func2_576(){
	int ret=0;
	if(globalVariable2_1== 576){
		ret=func2_577();
		return ret;
	}else{
			return 0;
	}
}
void call2_575(){
	globalVariable2_1 += 1;
}
int func2_575(){
	int ret=0;
	if(globalVariable2_1== 575){
		ret=func2_576();
		return ret;
	}else{
			return 0;
	}
}
void call2_574(){
	globalVariable2_1 += 1;
}
int func2_574(){
	int ret=0;
	if(globalVariable2_1== 574){
		ret=func2_575();
		return ret;
	}else{
			return 0;
	}
}
void call2_573(){
	globalVariable2_1 += 1;
}
int func2_573(){
	int ret=0;
	if(globalVariable2_1== 573){
		ret=func2_574();
		return ret;
	}else{
			return 0;
	}
}
void call2_572(){
	globalVariable2_1 += 1;
}
int func2_572(){
	int ret=0;
	if(globalVariable2_1== 572){
		ret=func2_573();
		return ret;
	}else{
			return 0;
	}
}
void call2_571(){
	globalVariable2_1 += 1;
}
int func2_571(){
	int ret=0;
	if(globalVariable2_1== 571){
		ret=func2_572();
		return ret;
	}else{
			return 0;
	}
}
void call2_570(){
	globalVariable2_1 += 1;
}
int func2_570(){
	int ret=0;
	if(globalVariable2_1== 570){
		ret=func2_571();
		return ret;
	}else{
			return 0;
	}
}
void call2_569(){
	globalVariable2_1 += 1;
}
int func2_569(){
	int ret=0;
	if(globalVariable2_1== 569){
		ret=func2_570();
		return ret;
	}else{
			return 0;
	}
}
void call2_568(){
	globalVariable2_1 += 1;
}
int func2_568(){
	int ret=0;
	if(globalVariable2_1== 568){
		ret=func2_569();
		return ret;
	}else{
			return 0;
	}
}
void call2_567(){
	globalVariable2_1 += 1;
}
int func2_567(){
	int ret=0;
	if(globalVariable2_1== 567){
		ret=func2_568();
		return ret;
	}else{
			return 0;
	}
}
void call2_566(){
	globalVariable2_1 += 1;
}
int func2_566(){
	int ret=0;
	if(globalVariable2_1== 566){
		ret=func2_567();
		return ret;
	}else{
			return 0;
	}
}
void call2_565(){
	globalVariable2_1 += 1;
}
int func2_565(){
	int ret=0;
	if(globalVariable2_1== 565){
		ret=func2_566();
		return ret;
	}else{
			return 0;
	}
}
void call2_564(){
	globalVariable2_1 += 1;
}
int func2_564(){
	int ret=0;
	if(globalVariable2_1== 564){
		ret=func2_565();
		return ret;
	}else{
			return 0;
	}
}
void call2_563(){
	globalVariable2_1 += 1;
}
int func2_563(){
	int ret=0;
	if(globalVariable2_1== 563){
		ret=func2_564();
		return ret;
	}else{
			return 0;
	}
}
void call2_562(){
	globalVariable2_1 += 1;
}
int func2_562(){
	int ret=0;
	if(globalVariable2_1== 562){
		ret=func2_563();
		return ret;
	}else{
			return 0;
	}
}
void call2_561(){
	globalVariable2_1 += 1;
}
int func2_561(){
	int ret=0;
	if(globalVariable2_1== 561){
		ret=func2_562();
		return ret;
	}else{
			return 0;
	}
}
void call2_560(){
	globalVariable2_1 += 1;
}
int func2_560(){
	int ret=0;
	if(globalVariable2_1== 560){
		ret=func2_561();
		return ret;
	}else{
			return 0;
	}
}
void call2_559(){
	globalVariable2_1 += 1;
}
int func2_559(){
	int ret=0;
	if(globalVariable2_1== 559){
		ret=func2_560();
		return ret;
	}else{
			return 0;
	}
}
void call2_558(){
	globalVariable2_1 += 1;
}
int func2_558(){
	int ret=0;
	if(globalVariable2_1== 558){
		ret=func2_559();
		return ret;
	}else{
			return 0;
	}
}
void call2_557(){
	globalVariable2_1 += 1;
}
int func2_557(){
	int ret=0;
	if(globalVariable2_1== 557){
		ret=func2_558();
		return ret;
	}else{
			return 0;
	}
}
void call2_556(){
	globalVariable2_1 += 1;
}
int func2_556(){
	int ret=0;
	if(globalVariable2_1== 556){
		ret=func2_557();
		return ret;
	}else{
			return 0;
	}
}
void call2_555(){
	globalVariable2_1 += 1;
}
int func2_555(){
	int ret=0;
	if(globalVariable2_1== 555){
		ret=func2_556();
		return ret;
	}else{
			return 0;
	}
}
void call2_554(){
	globalVariable2_1 += 1;
}
int func2_554(){
	int ret=0;
	if(globalVariable2_1== 554){
		ret=func2_555();
		return ret;
	}else{
			return 0;
	}
}
void call2_553(){
	globalVariable2_1 += 1;
}
int func2_553(){
	int ret=0;
	if(globalVariable2_1== 553){
		ret=func2_554();
		return ret;
	}else{
			return 0;
	}
}
void call2_552(){
	globalVariable2_1 += 1;
}
int func2_552(){
	int ret=0;
	if(globalVariable2_1== 552){
		ret=func2_553();
		return ret;
	}else{
			return 0;
	}
}
void call2_551(){
	globalVariable2_1 += 1;
}
int func2_551(){
	int ret=0;
	if(globalVariable2_1== 551){
		ret=func2_552();
		return ret;
	}else{
			return 0;
	}
}
void call2_550(){
	globalVariable2_1 += 1;
}
int func2_550(){
	int ret=0;
	if(globalVariable2_1== 550){
		ret=func2_551();
		return ret;
	}else{
			return 0;
	}
}
void call2_549(){
	globalVariable2_1 += 1;
}
int func2_549(){
	int ret=0;
	if(globalVariable2_1== 549){
		ret=func2_550();
		return ret;
	}else{
			return 0;
	}
}
void call2_548(){
	globalVariable2_1 += 1;
}
int func2_548(){
	int ret=0;
	if(globalVariable2_1== 548){
		ret=func2_549();
		return ret;
	}else{
			return 0;
	}
}
void call2_547(){
	globalVariable2_1 += 1;
}
int func2_547(){
	int ret=0;
	if(globalVariable2_1== 547){
		ret=func2_548();
		return ret;
	}else{
			return 0;
	}
}
void call2_546(){
	globalVariable2_1 += 1;
}
int func2_546(){
	int ret=0;
	if(globalVariable2_1== 546){
		ret=func2_547();
		return ret;
	}else{
			return 0;
	}
}
void call2_545(){
	globalVariable2_1 += 1;
}
int func2_545(){
	int ret=0;
	if(globalVariable2_1== 545){
		ret=func2_546();
		return ret;
	}else{
			return 0;
	}
}
void call2_544(){
	globalVariable2_1 += 1;
}
int func2_544(){
	int ret=0;
	if(globalVariable2_1== 544){
		ret=func2_545();
		return ret;
	}else{
			return 0;
	}
}
void call2_543(){
	globalVariable2_1 += 1;
}
int func2_543(){
	int ret=0;
	if(globalVariable2_1== 543){
		ret=func2_544();
		return ret;
	}else{
			return 0;
	}
}
void call2_542(){
	globalVariable2_1 += 1;
}
int func2_542(){
	int ret=0;
	if(globalVariable2_1== 542){
		ret=func2_543();
		return ret;
	}else{
			return 0;
	}
}
void call2_541(){
	globalVariable2_1 += 1;
}
int func2_541(){
	int ret=0;
	if(globalVariable2_1== 541){
		ret=func2_542();
		return ret;
	}else{
			return 0;
	}
}
void call2_540(){
	globalVariable2_1 += 1;
}
int func2_540(){
	int ret=0;
	if(globalVariable2_1== 540){
		ret=func2_541();
		return ret;
	}else{
			return 0;
	}
}
void call2_539(){
	globalVariable2_1 += 1;
}
int func2_539(){
	int ret=0;
	if(globalVariable2_1== 539){
		ret=func2_540();
		return ret;
	}else{
			return 0;
	}
}
void call2_538(){
	globalVariable2_1 += 1;
}
int func2_538(){
	int ret=0;
	if(globalVariable2_1== 538){
		ret=func2_539();
		return ret;
	}else{
			return 0;
	}
}
void call2_537(){
	globalVariable2_1 += 1;
}
int func2_537(){
	int ret=0;
	if(globalVariable2_1== 537){
		ret=func2_538();
		return ret;
	}else{
			return 0;
	}
}
void call2_536(){
	globalVariable2_1 += 1;
}
int func2_536(){
	int ret=0;
	if(globalVariable2_1== 536){
		ret=func2_537();
		return ret;
	}else{
			return 0;
	}
}
void call2_535(){
	globalVariable2_1 += 1;
}
int func2_535(){
	int ret=0;
	if(globalVariable2_1== 535){
		ret=func2_536();
		return ret;
	}else{
			return 0;
	}
}
void call2_534(){
	globalVariable2_1 += 1;
}
int func2_534(){
	int ret=0;
	if(globalVariable2_1== 534){
		ret=func2_535();
		return ret;
	}else{
			return 0;
	}
}
void call2_533(){
	globalVariable2_1 += 1;
}
int func2_533(){
	int ret=0;
	if(globalVariable2_1== 533){
		ret=func2_534();
		return ret;
	}else{
			return 0;
	}
}
void call2_532(){
	globalVariable2_1 += 1;
}
int func2_532(){
	int ret=0;
	if(globalVariable2_1== 532){
		ret=func2_533();
		return ret;
	}else{
			return 0;
	}
}
void call2_531(){
	globalVariable2_1 += 1;
}
int func2_531(){
	int ret=0;
	if(globalVariable2_1== 531){
		ret=func2_532();
		return ret;
	}else{
			return 0;
	}
}
void call2_530(){
	globalVariable2_1 += 1;
}
int func2_530(){
	int ret=0;
	if(globalVariable2_1== 530){
		ret=func2_531();
		return ret;
	}else{
			return 0;
	}
}
void call2_529(){
	globalVariable2_1 += 1;
}
int func2_529(){
	int ret=0;
	if(globalVariable2_1== 529){
		ret=func2_530();
		return ret;
	}else{
			return 0;
	}
}
void call2_528(){
	globalVariable2_1 += 1;
}
int func2_528(){
	int ret=0;
	if(globalVariable2_1== 528){
		ret=func2_529();
		return ret;
	}else{
			return 0;
	}
}
void call2_527(){
	globalVariable2_1 += 1;
}
int func2_527(){
	int ret=0;
	if(globalVariable2_1== 527){
		ret=func2_528();
		return ret;
	}else{
			return 0;
	}
}
void call2_526(){
	globalVariable2_1 += 1;
}
int func2_526(){
	int ret=0;
	if(globalVariable2_1== 526){
		ret=func2_527();
		return ret;
	}else{
			return 0;
	}
}
void call2_525(){
	globalVariable2_1 += 1;
}
int func2_525(){
	int ret=0;
	if(globalVariable2_1== 525){
		ret=func2_526();
		return ret;
	}else{
			return 0;
	}
}
void call2_524(){
	globalVariable2_1 += 1;
}
int func2_524(){
	int ret=0;
	if(globalVariable2_1== 524){
		ret=func2_525();
		return ret;
	}else{
			return 0;
	}
}
void call2_523(){
	globalVariable2_1 += 1;
}
int func2_523(){
	int ret=0;
	if(globalVariable2_1== 523){
		ret=func2_524();
		return ret;
	}else{
			return 0;
	}
}
void call2_522(){
	globalVariable2_1 += 1;
}
int func2_522(){
	int ret=0;
	if(globalVariable2_1== 522){
		ret=func2_523();
		return ret;
	}else{
			return 0;
	}
}
void call2_521(){
	globalVariable2_1 += 1;
}
int func2_521(){
	int ret=0;
	if(globalVariable2_1== 521){
		ret=func2_522();
		return ret;
	}else{
			return 0;
	}
}
void call2_520(){
	globalVariable2_1 += 1;
}
int func2_520(){
	int ret=0;
	if(globalVariable2_1== 520){
		ret=func2_521();
		return ret;
	}else{
			return 0;
	}
}
void call2_519(){
	globalVariable2_1 += 1;
}
int func2_519(){
	int ret=0;
	if(globalVariable2_1== 519){
		ret=func2_520();
		return ret;
	}else{
			return 0;
	}
}
void call2_518(){
	globalVariable2_1 += 1;
}
int func2_518(){
	int ret=0;
	if(globalVariable2_1== 518){
		ret=func2_519();
		return ret;
	}else{
			return 0;
	}
}
void call2_517(){
	globalVariable2_1 += 1;
}
int func2_517(){
	int ret=0;
	if(globalVariable2_1== 517){
		ret=func2_518();
		return ret;
	}else{
			return 0;
	}
}
void call2_516(){
	globalVariable2_1 += 1;
}
int func2_516(){
	int ret=0;
	if(globalVariable2_1== 516){
		ret=func2_517();
		return ret;
	}else{
			return 0;
	}
}
void call2_515(){
	globalVariable2_1 += 1;
}
int func2_515(){
	int ret=0;
	if(globalVariable2_1== 515){
		ret=func2_516();
		return ret;
	}else{
			return 0;
	}
}
void call2_514(){
	globalVariable2_1 += 1;
}
int func2_514(){
	int ret=0;
	if(globalVariable2_1== 514){
		ret=func2_515();
		return ret;
	}else{
			return 0;
	}
}
void call2_513(){
	globalVariable2_1 += 1;
}
int func2_513(){
	int ret=0;
	if(globalVariable2_1== 513){
		ret=func2_514();
		return ret;
	}else{
			return 0;
	}
}
void call2_512(){
	globalVariable2_1 += 1;
}
int func2_512(){
	int ret=0;
	if(globalVariable2_1== 512){
		ret=func2_513();
		return ret;
	}else{
			return 0;
	}
}
void call2_511(){
	globalVariable2_1 += 1;
}
int func2_511(){
	int ret=0;
	if(globalVariable2_1== 511){
		ret=func2_512();
		return ret;
	}else{
			return 0;
	}
}
void call2_510(){
	globalVariable2_1 += 1;
}
int func2_510(){
	int ret=0;
	if(globalVariable2_1== 510){
		ret=func2_511();
		return ret;
	}else{
			return 0;
	}
}
void call2_509(){
	globalVariable2_1 += 1;
}
int func2_509(){
	int ret=0;
	if(globalVariable2_1== 509){
		ret=func2_510();
		return ret;
	}else{
			return 0;
	}
}
void call2_508(){
	globalVariable2_1 += 1;
}
int func2_508(){
	int ret=0;
	if(globalVariable2_1== 508){
		ret=func2_509();
		return ret;
	}else{
			return 0;
	}
}
void call2_507(){
	globalVariable2_1 += 1;
}
int func2_507(){
	int ret=0;
	if(globalVariable2_1== 507){
		ret=func2_508();
		return ret;
	}else{
			return 0;
	}
}
void call2_506(){
	globalVariable2_1 += 1;
}
int func2_506(){
	int ret=0;
	if(globalVariable2_1== 506){
		ret=func2_507();
		return ret;
	}else{
			return 0;
	}
}
void call2_505(){
	globalVariable2_1 += 1;
}
int func2_505(){
	int ret=0;
	if(globalVariable2_1== 505){
		ret=func2_506();
		return ret;
	}else{
			return 0;
	}
}
void call2_504(){
	globalVariable2_1 += 1;
}
int func2_504(){
	int ret=0;
	if(globalVariable2_1== 504){
		ret=func2_505();
		return ret;
	}else{
			return 0;
	}
}
void call2_503(){
	globalVariable2_1 += 1;
}
int func2_503(){
	int ret=0;
	if(globalVariable2_1== 503){
		ret=func2_504();
		return ret;
	}else{
			return 0;
	}
}
void call2_502(){
	globalVariable2_1 += 1;
}
int func2_502(){
	int ret=0;
	if(globalVariable2_1== 502){
		ret=func2_503();
		return ret;
	}else{
			return 0;
	}
}
void call2_501(){
	globalVariable2_1 += 1;
}
int func2_501(){
	int ret=0;
	if(globalVariable2_1== 501){
		ret=func2_502();
		return ret;
	}else{
			return 0;
	}
}
void call2_500(){
	globalVariable2_1 += 1;
}
int func2_500(){
	int ret=0;
	if(globalVariable2_1== 500){
		ret=func2_501();
		return ret;
	}else{
			return 0;
	}
}
void call2_499(){
	globalVariable2_1 += 1;
}
int func2_499(){
	int ret=0;
	if(globalVariable2_1== 499){
		ret=func2_500();
		return ret;
	}else{
			return 0;
	}
}
void call2_498(){
	globalVariable2_1 += 1;
}
int func2_498(){
	int ret=0;
	if(globalVariable2_1== 498){
		ret=func2_499();
		return ret;
	}else{
			return 0;
	}
}
void call2_497(){
	globalVariable2_1 += 1;
}
int func2_497(){
	int ret=0;
	if(globalVariable2_1== 497){
		ret=func2_498();
		return ret;
	}else{
			return 0;
	}
}
void call2_496(){
	globalVariable2_1 += 1;
}
int func2_496(){
	int ret=0;
	if(globalVariable2_1== 496){
		ret=func2_497();
		return ret;
	}else{
			return 0;
	}
}
void call2_495(){
	globalVariable2_1 += 1;
}
int func2_495(){
	int ret=0;
	if(globalVariable2_1== 495){
		ret=func2_496();
		return ret;
	}else{
			return 0;
	}
}
void call2_494(){
	globalVariable2_1 += 1;
}
int func2_494(){
	int ret=0;
	if(globalVariable2_1== 494){
		ret=func2_495();
		return ret;
	}else{
			return 0;
	}
}
void call2_493(){
	globalVariable2_1 += 1;
}
int func2_493(){
	int ret=0;
	if(globalVariable2_1== 493){
		ret=func2_494();
		return ret;
	}else{
			return 0;
	}
}
void call2_492(){
	globalVariable2_1 += 1;
}
int func2_492(){
	int ret=0;
	if(globalVariable2_1== 492){
		ret=func2_493();
		return ret;
	}else{
			return 0;
	}
}
void call2_491(){
	globalVariable2_1 += 1;
}
int func2_491(){
	int ret=0;
	if(globalVariable2_1== 491){
		ret=func2_492();
		return ret;
	}else{
			return 0;
	}
}
void call2_490(){
	globalVariable2_1 += 1;
}
int func2_490(){
	int ret=0;
	if(globalVariable2_1== 490){
		ret=func2_491();
		return ret;
	}else{
			return 0;
	}
}
void call2_489(){
	globalVariable2_1 += 1;
}
int func2_489(){
	int ret=0;
	if(globalVariable2_1== 489){
		ret=func2_490();
		return ret;
	}else{
			return 0;
	}
}
void call2_488(){
	globalVariable2_1 += 1;
}
int func2_488(){
	int ret=0;
	if(globalVariable2_1== 488){
		ret=func2_489();
		return ret;
	}else{
			return 0;
	}
}
void call2_487(){
	globalVariable2_1 += 1;
}
int func2_487(){
	int ret=0;
	if(globalVariable2_1== 487){
		ret=func2_488();
		return ret;
	}else{
			return 0;
	}
}
void call2_486(){
	globalVariable2_1 += 1;
}
int func2_486(){
	int ret=0;
	if(globalVariable2_1== 486){
		ret=func2_487();
		return ret;
	}else{
			return 0;
	}
}
void call2_485(){
	globalVariable2_1 += 1;
}
int func2_485(){
	int ret=0;
	if(globalVariable2_1== 485){
		ret=func2_486();
		return ret;
	}else{
			return 0;
	}
}
void call2_484(){
	globalVariable2_1 += 1;
}
int func2_484(){
	int ret=0;
	if(globalVariable2_1== 484){
		ret=func2_485();
		return ret;
	}else{
			return 0;
	}
}
void call2_483(){
	globalVariable2_1 += 1;
}
int func2_483(){
	int ret=0;
	if(globalVariable2_1== 483){
		ret=func2_484();
		return ret;
	}else{
			return 0;
	}
}
void call2_482(){
	globalVariable2_1 += 1;
}
int func2_482(){
	int ret=0;
	if(globalVariable2_1== 482){
		ret=func2_483();
		return ret;
	}else{
			return 0;
	}
}
void call2_481(){
	globalVariable2_1 += 1;
}
int func2_481(){
	int ret=0;
	if(globalVariable2_1== 481){
		ret=func2_482();
		return ret;
	}else{
			return 0;
	}
}
void call2_480(){
	globalVariable2_1 += 1;
}
int func2_480(){
	int ret=0;
	if(globalVariable2_1== 480){
		ret=func2_481();
		return ret;
	}else{
			return 0;
	}
}
void call2_479(){
	globalVariable2_1 += 1;
}
int func2_479(){
	int ret=0;
	if(globalVariable2_1== 479){
		ret=func2_480();
		return ret;
	}else{
			return 0;
	}
}
void call2_478(){
	globalVariable2_1 += 1;
}
int func2_478(){
	int ret=0;
	if(globalVariable2_1== 478){
		ret=func2_479();
		return ret;
	}else{
			return 0;
	}
}
void call2_477(){
	globalVariable2_1 += 1;
}
int func2_477(){
	int ret=0;
	if(globalVariable2_1== 477){
		ret=func2_478();
		return ret;
	}else{
			return 0;
	}
}
void call2_476(){
	globalVariable2_1 += 1;
}
int func2_476(){
	int ret=0;
	if(globalVariable2_1== 476){
		ret=func2_477();
		return ret;
	}else{
			return 0;
	}
}
void call2_475(){
	globalVariable2_1 += 1;
}
int func2_475(){
	int ret=0;
	if(globalVariable2_1== 475){
		ret=func2_476();
		return ret;
	}else{
			return 0;
	}
}
void call2_474(){
	globalVariable2_1 += 1;
}
int func2_474(){
	int ret=0;
	if(globalVariable2_1== 474){
		ret=func2_475();
		return ret;
	}else{
			return 0;
	}
}
void call2_473(){
	globalVariable2_1 += 1;
}
int func2_473(){
	int ret=0;
	if(globalVariable2_1== 473){
		ret=func2_474();
		return ret;
	}else{
			return 0;
	}
}
void call2_472(){
	globalVariable2_1 += 1;
}
int func2_472(){
	int ret=0;
	if(globalVariable2_1== 472){
		ret=func2_473();
		return ret;
	}else{
			return 0;
	}
}
void call2_471(){
	globalVariable2_1 += 1;
}
int func2_471(){
	int ret=0;
	if(globalVariable2_1== 471){
		ret=func2_472();
		return ret;
	}else{
			return 0;
	}
}
void call2_470(){
	globalVariable2_1 += 1;
}
int func2_470(){
	int ret=0;
	if(globalVariable2_1== 470){
		ret=func2_471();
		return ret;
	}else{
			return 0;
	}
}
void call2_469(){
	globalVariable2_1 += 1;
}
int func2_469(){
	int ret=0;
	if(globalVariable2_1== 469){
		ret=func2_470();
		return ret;
	}else{
			return 0;
	}
}
void call2_468(){
	globalVariable2_1 += 1;
}
int func2_468(){
	int ret=0;
	if(globalVariable2_1== 468){
		ret=func2_469();
		return ret;
	}else{
			return 0;
	}
}
void call2_467(){
	globalVariable2_1 += 1;
}
int func2_467(){
	int ret=0;
	if(globalVariable2_1== 467){
		ret=func2_468();
		return ret;
	}else{
			return 0;
	}
}
void call2_466(){
	globalVariable2_1 += 1;
}
int func2_466(){
	int ret=0;
	if(globalVariable2_1== 466){
		ret=func2_467();
		return ret;
	}else{
			return 0;
	}
}
void call2_465(){
	globalVariable2_1 += 1;
}
int func2_465(){
	int ret=0;
	if(globalVariable2_1== 465){
		ret=func2_466();
		return ret;
	}else{
			return 0;
	}
}
void call2_464(){
	globalVariable2_1 += 1;
}
int func2_464(){
	int ret=0;
	if(globalVariable2_1== 464){
		ret=func2_465();
		return ret;
	}else{
			return 0;
	}
}
void call2_463(){
	globalVariable2_1 += 1;
}
int func2_463(){
	int ret=0;
	if(globalVariable2_1== 463){
		ret=func2_464();
		return ret;
	}else{
			return 0;
	}
}
void call2_462(){
	globalVariable2_1 += 1;
}
int func2_462(){
	int ret=0;
	if(globalVariable2_1== 462){
		ret=func2_463();
		return ret;
	}else{
			return 0;
	}
}
void call2_461(){
	globalVariable2_1 += 1;
}
int func2_461(){
	int ret=0;
	if(globalVariable2_1== 461){
		ret=func2_462();
		return ret;
	}else{
			return 0;
	}
}
void call2_460(){
	globalVariable2_1 += 1;
}
int func2_460(){
	int ret=0;
	if(globalVariable2_1== 460){
		ret=func2_461();
		return ret;
	}else{
			return 0;
	}
}
void call2_459(){
	globalVariable2_1 += 1;
}
int func2_459(){
	int ret=0;
	if(globalVariable2_1== 459){
		ret=func2_460();
		return ret;
	}else{
			return 0;
	}
}
void call2_458(){
	globalVariable2_1 += 1;
}
int func2_458(){
	int ret=0;
	if(globalVariable2_1== 458){
		ret=func2_459();
		return ret;
	}else{
			return 0;
	}
}
void call2_457(){
	globalVariable2_1 += 1;
}
int func2_457(){
	int ret=0;
	if(globalVariable2_1== 457){
		ret=func2_458();
		return ret;
	}else{
			return 0;
	}
}
void call2_456(){
	globalVariable2_1 += 1;
}
int func2_456(){
	int ret=0;
	if(globalVariable2_1== 456){
		ret=func2_457();
		return ret;
	}else{
			return 0;
	}
}
void call2_455(){
	globalVariable2_1 += 1;
}
int func2_455(){
	int ret=0;
	if(globalVariable2_1== 455){
		ret=func2_456();
		return ret;
	}else{
			return 0;
	}
}
void call2_454(){
	globalVariable2_1 += 1;
}
int func2_454(){
	int ret=0;
	if(globalVariable2_1== 454){
		ret=func2_455();
		return ret;
	}else{
			return 0;
	}
}
void call2_453(){
	globalVariable2_1 += 1;
}
int func2_453(){
	int ret=0;
	if(globalVariable2_1== 453){
		ret=func2_454();
		return ret;
	}else{
			return 0;
	}
}
void call2_452(){
	globalVariable2_1 += 1;
}
int func2_452(){
	int ret=0;
	if(globalVariable2_1== 452){
		ret=func2_453();
		return ret;
	}else{
			return 0;
	}
}
void call2_451(){
	globalVariable2_1 += 1;
}
int func2_451(){
	int ret=0;
	if(globalVariable2_1== 451){
		ret=func2_452();
		return ret;
	}else{
			return 0;
	}
}
void call2_450(){
	globalVariable2_1 += 1;
}
int func2_450(){
	int ret=0;
	if(globalVariable2_1== 450){
		ret=func2_451();
		return ret;
	}else{
			return 0;
	}
}
void call2_449(){
	globalVariable2_1 += 1;
}
int func2_449(){
	int ret=0;
	if(globalVariable2_1== 449){
		ret=func2_450();
		return ret;
	}else{
			return 0;
	}
}
void call2_448(){
	globalVariable2_1 += 1;
}
int func2_448(){
	int ret=0;
	if(globalVariable2_1== 448){
		ret=func2_449();
		return ret;
	}else{
			return 0;
	}
}
void call2_447(){
	globalVariable2_1 += 1;
}
int func2_447(){
	int ret=0;
	if(globalVariable2_1== 447){
		ret=func2_448();
		return ret;
	}else{
			return 0;
	}
}
void call2_446(){
	globalVariable2_1 += 1;
}
int func2_446(){
	int ret=0;
	if(globalVariable2_1== 446){
		ret=func2_447();
		return ret;
	}else{
			return 0;
	}
}
void call2_445(){
	globalVariable2_1 += 1;
}
int func2_445(){
	int ret=0;
	if(globalVariable2_1== 445){
		ret=func2_446();
		return ret;
	}else{
			return 0;
	}
}
void call2_444(){
	globalVariable2_1 += 1;
}
int func2_444(){
	int ret=0;
	if(globalVariable2_1== 444){
		ret=func2_445();
		return ret;
	}else{
			return 0;
	}
}
void call2_443(){
	globalVariable2_1 += 1;
}
int func2_443(){
	int ret=0;
	if(globalVariable2_1== 443){
		ret=func2_444();
		return ret;
	}else{
			return 0;
	}
}
void call2_442(){
	globalVariable2_1 += 1;
}
int func2_442(){
	int ret=0;
	if(globalVariable2_1== 442){
		ret=func2_443();
		return ret;
	}else{
			return 0;
	}
}
void call2_441(){
	globalVariable2_1 += 1;
}
int func2_441(){
	int ret=0;
	if(globalVariable2_1== 441){
		ret=func2_442();
		return ret;
	}else{
			return 0;
	}
}
void call2_440(){
	globalVariable2_1 += 1;
}
int func2_440(){
	int ret=0;
	if(globalVariable2_1== 440){
		ret=func2_441();
		return ret;
	}else{
			return 0;
	}
}
void call2_439(){
	globalVariable2_1 += 1;
}
int func2_439(){
	int ret=0;
	if(globalVariable2_1== 439){
		ret=func2_440();
		return ret;
	}else{
			return 0;
	}
}
void call2_438(){
	globalVariable2_1 += 1;
}
int func2_438(){
	int ret=0;
	if(globalVariable2_1== 438){
		ret=func2_439();
		return ret;
	}else{
			return 0;
	}
}
void call2_437(){
	globalVariable2_1 += 1;
}
int func2_437(){
	int ret=0;
	if(globalVariable2_1== 437){
		ret=func2_438();
		return ret;
	}else{
			return 0;
	}
}
void call2_436(){
	globalVariable2_1 += 1;
}
int func2_436(){
	int ret=0;
	if(globalVariable2_1== 436){
		ret=func2_437();
		return ret;
	}else{
			return 0;
	}
}
void call2_435(){
	globalVariable2_1 += 1;
}
int func2_435(){
	int ret=0;
	if(globalVariable2_1== 435){
		ret=func2_436();
		return ret;
	}else{
			return 0;
	}
}
void call2_434(){
	globalVariable2_1 += 1;
}
int func2_434(){
	int ret=0;
	if(globalVariable2_1== 434){
		ret=func2_435();
		return ret;
	}else{
			return 0;
	}
}
void call2_433(){
	globalVariable2_1 += 1;
}
int func2_433(){
	int ret=0;
	if(globalVariable2_1== 433){
		ret=func2_434();
		return ret;
	}else{
			return 0;
	}
}
void call2_432(){
	globalVariable2_1 += 1;
}
int func2_432(){
	int ret=0;
	if(globalVariable2_1== 432){
		ret=func2_433();
		return ret;
	}else{
			return 0;
	}
}
void call2_431(){
	globalVariable2_1 += 1;
}
int func2_431(){
	int ret=0;
	if(globalVariable2_1== 431){
		ret=func2_432();
		return ret;
	}else{
			return 0;
	}
}
void call2_430(){
	globalVariable2_1 += 1;
}
int func2_430(){
	int ret=0;
	if(globalVariable2_1== 430){
		ret=func2_431();
		return ret;
	}else{
			return 0;
	}
}
void call2_429(){
	globalVariable2_1 += 1;
}
int func2_429(){
	int ret=0;
	if(globalVariable2_1== 429){
		ret=func2_430();
		return ret;
	}else{
			return 0;
	}
}
void call2_428(){
	globalVariable2_1 += 1;
}
int func2_428(){
	int ret=0;
	if(globalVariable2_1== 428){
		ret=func2_429();
		return ret;
	}else{
			return 0;
	}
}
void call2_427(){
	globalVariable2_1 += 1;
}
int func2_427(){
	int ret=0;
	if(globalVariable2_1== 427){
		ret=func2_428();
		return ret;
	}else{
			return 0;
	}
}
void call2_426(){
	globalVariable2_1 += 1;
}
int func2_426(){
	int ret=0;
	if(globalVariable2_1== 426){
		ret=func2_427();
		return ret;
	}else{
			return 0;
	}
}
void call2_425(){
	globalVariable2_1 += 1;
}
int func2_425(){
	int ret=0;
	if(globalVariable2_1== 425){
		ret=func2_426();
		return ret;
	}else{
			return 0;
	}
}
void call2_424(){
	globalVariable2_1 += 1;
}
int func2_424(){
	int ret=0;
	if(globalVariable2_1== 424){
		ret=func2_425();
		return ret;
	}else{
			return 0;
	}
}
void call2_423(){
	globalVariable2_1 += 1;
}
int func2_423(){
	int ret=0;
	if(globalVariable2_1== 423){
		ret=func2_424();
		return ret;
	}else{
			return 0;
	}
}
void call2_422(){
	globalVariable2_1 += 1;
}
int func2_422(){
	int ret=0;
	if(globalVariable2_1== 422){
		ret=func2_423();
		return ret;
	}else{
			return 0;
	}
}
void call2_421(){
	globalVariable2_1 += 1;
}
int func2_421(){
	int ret=0;
	if(globalVariable2_1== 421){
		ret=func2_422();
		return ret;
	}else{
			return 0;
	}
}
void call2_420(){
	globalVariable2_1 += 1;
}
int func2_420(){
	int ret=0;
	if(globalVariable2_1== 420){
		ret=func2_421();
		return ret;
	}else{
			return 0;
	}
}
void call2_419(){
	globalVariable2_1 += 1;
}
int func2_419(){
	int ret=0;
	if(globalVariable2_1== 419){
		ret=func2_420();
		return ret;
	}else{
			return 0;
	}
}
void call2_418(){
	globalVariable2_1 += 1;
}
int func2_418(){
	int ret=0;
	if(globalVariable2_1== 418){
		ret=func2_419();
		return ret;
	}else{
			return 0;
	}
}
void call2_417(){
	globalVariable2_1 += 1;
}
int func2_417(){
	int ret=0;
	if(globalVariable2_1== 417){
		ret=func2_418();
		return ret;
	}else{
			return 0;
	}
}
void call2_416(){
	globalVariable2_1 += 1;
}
int func2_416(){
	int ret=0;
	if(globalVariable2_1== 416){
		ret=func2_417();
		return ret;
	}else{
			return 0;
	}
}
void call2_415(){
	globalVariable2_1 += 1;
}
int func2_415(){
	int ret=0;
	if(globalVariable2_1== 415){
		ret=func2_416();
		return ret;
	}else{
			return 0;
	}
}
void call2_414(){
	globalVariable2_1 += 1;
}
int func2_414(){
	int ret=0;
	if(globalVariable2_1== 414){
		ret=func2_415();
		return ret;
	}else{
			return 0;
	}
}
void call2_413(){
	globalVariable2_1 += 1;
}
int func2_413(){
	int ret=0;
	if(globalVariable2_1== 413){
		ret=func2_414();
		return ret;
	}else{
			return 0;
	}
}
void call2_412(){
	globalVariable2_1 += 1;
}
int func2_412(){
	int ret=0;
	if(globalVariable2_1== 412){
		ret=func2_413();
		return ret;
	}else{
			return 0;
	}
}
void call2_411(){
	globalVariable2_1 += 1;
}
int func2_411(){
	int ret=0;
	if(globalVariable2_1== 411){
		ret=func2_412();
		return ret;
	}else{
			return 0;
	}
}
void call2_410(){
	globalVariable2_1 += 1;
}
int func2_410(){
	int ret=0;
	if(globalVariable2_1== 410){
		ret=func2_411();
		return ret;
	}else{
			return 0;
	}
}
void call2_409(){
	globalVariable2_1 += 1;
}
int func2_409(){
	int ret=0;
	if(globalVariable2_1== 409){
		ret=func2_410();
		return ret;
	}else{
			return 0;
	}
}
void call2_408(){
	globalVariable2_1 += 1;
}
int func2_408(){
	int ret=0;
	if(globalVariable2_1== 408){
		ret=func2_409();
		return ret;
	}else{
			return 0;
	}
}
void call2_407(){
	globalVariable2_1 += 1;
}
int func2_407(){
	int ret=0;
	if(globalVariable2_1== 407){
		ret=func2_408();
		return ret;
	}else{
			return 0;
	}
}
void call2_406(){
	globalVariable2_1 += 1;
}
int func2_406(){
	int ret=0;
	if(globalVariable2_1== 406){
		ret=func2_407();
		return ret;
	}else{
			return 0;
	}
}
void call2_405(){
	globalVariable2_1 += 1;
}
int func2_405(){
	int ret=0;
	if(globalVariable2_1== 405){
		ret=func2_406();
		return ret;
	}else{
			return 0;
	}
}
void call2_404(){
	globalVariable2_1 += 1;
}
int func2_404(){
	int ret=0;
	if(globalVariable2_1== 404){
		ret=func2_405();
		return ret;
	}else{
			return 0;
	}
}
void call2_403(){
	globalVariable2_1 += 1;
}
int func2_403(){
	int ret=0;
	if(globalVariable2_1== 403){
		ret=func2_404();
		return ret;
	}else{
			return 0;
	}
}
void call2_402(){
	globalVariable2_1 += 1;
}
int func2_402(){
	int ret=0;
	if(globalVariable2_1== 402){
		ret=func2_403();
		return ret;
	}else{
			return 0;
	}
}
void call2_401(){
	globalVariable2_1 += 1;
}
int func2_401(){
	int ret=0;
	if(globalVariable2_1== 401){
		ret=func2_402();
		return ret;
	}else{
			return 0;
	}
}
void call2_400(){
	globalVariable2_1 += 1;
}
int func2_400(){
	int ret=0;
	if(globalVariable2_1== 400){
		ret=func2_401();
		return ret;
	}else{
			return 0;
	}
}
void call2_399(){
	globalVariable2_1 += 1;
}
int func2_399(){
	int ret=0;
	if(globalVariable2_1== 399){
		ret=func2_400();
		return ret;
	}else{
			return 0;
	}
}
void call2_398(){
	globalVariable2_1 += 1;
}
int func2_398(){
	int ret=0;
	if(globalVariable2_1== 398){
		ret=func2_399();
		return ret;
	}else{
			return 0;
	}
}
void call2_397(){
	globalVariable2_1 += 1;
}
int func2_397(){
	int ret=0;
	if(globalVariable2_1== 397){
		ret=func2_398();
		return ret;
	}else{
			return 0;
	}
}
void call2_396(){
	globalVariable2_1 += 1;
}
int func2_396(){
	int ret=0;
	if(globalVariable2_1== 396){
		ret=func2_397();
		return ret;
	}else{
			return 0;
	}
}
void call2_395(){
	globalVariable2_1 += 1;
}
int func2_395(){
	int ret=0;
	if(globalVariable2_1== 395){
		ret=func2_396();
		return ret;
	}else{
			return 0;
	}
}
void call2_394(){
	globalVariable2_1 += 1;
}
int func2_394(){
	int ret=0;
	if(globalVariable2_1== 394){
		ret=func2_395();
		return ret;
	}else{
			return 0;
	}
}
void call2_393(){
	globalVariable2_1 += 1;
}
int func2_393(){
	int ret=0;
	if(globalVariable2_1== 393){
		ret=func2_394();
		return ret;
	}else{
			return 0;
	}
}
void call2_392(){
	globalVariable2_1 += 1;
}
int func2_392(){
	int ret=0;
	if(globalVariable2_1== 392){
		ret=func2_393();
		return ret;
	}else{
			return 0;
	}
}
void call2_391(){
	globalVariable2_1 += 1;
}
int func2_391(){
	int ret=0;
	if(globalVariable2_1== 391){
		ret=func2_392();
		return ret;
	}else{
			return 0;
	}
}
void call2_390(){
	globalVariable2_1 += 1;
}
int func2_390(){
	int ret=0;
	if(globalVariable2_1== 390){
		ret=func2_391();
		return ret;
	}else{
			return 0;
	}
}
void call2_389(){
	globalVariable2_1 += 1;
}
int func2_389(){
	int ret=0;
	if(globalVariable2_1== 389){
		ret=func2_390();
		return ret;
	}else{
			return 0;
	}
}
void call2_388(){
	globalVariable2_1 += 1;
}
int func2_388(){
	int ret=0;
	if(globalVariable2_1== 388){
		ret=func2_389();
		return ret;
	}else{
			return 0;
	}
}
void call2_387(){
	globalVariable2_1 += 1;
}
int func2_387(){
	int ret=0;
	if(globalVariable2_1== 387){
		ret=func2_388();
		return ret;
	}else{
			return 0;
	}
}
void call2_386(){
	globalVariable2_1 += 1;
}
int func2_386(){
	int ret=0;
	if(globalVariable2_1== 386){
		ret=func2_387();
		return ret;
	}else{
			return 0;
	}
}
void call2_385(){
	globalVariable2_1 += 1;
}
int func2_385(){
	int ret=0;
	if(globalVariable2_1== 385){
		ret=func2_386();
		return ret;
	}else{
			return 0;
	}
}
void call2_384(){
	globalVariable2_1 += 1;
}
int func2_384(){
	int ret=0;
	if(globalVariable2_1== 384){
		ret=func2_385();
		return ret;
	}else{
			return 0;
	}
}
void call2_383(){
	globalVariable2_1 += 1;
}
int func2_383(){
	int ret=0;
	if(globalVariable2_1== 383){
		ret=func2_384();
		return ret;
	}else{
			return 0;
	}
}
void call2_382(){
	globalVariable2_1 += 1;
}
int func2_382(){
	int ret=0;
	if(globalVariable2_1== 382){
		ret=func2_383();
		return ret;
	}else{
			return 0;
	}
}
void call2_381(){
	globalVariable2_1 += 1;
}
int func2_381(){
	int ret=0;
	if(globalVariable2_1== 381){
		ret=func2_382();
		return ret;
	}else{
			return 0;
	}
}
void call2_380(){
	globalVariable2_1 += 1;
}
int func2_380(){
	int ret=0;
	if(globalVariable2_1== 380){
		ret=func2_381();
		return ret;
	}else{
			return 0;
	}
}
void call2_379(){
	globalVariable2_1 += 1;
}
int func2_379(){
	int ret=0;
	if(globalVariable2_1== 379){
		ret=func2_380();
		return ret;
	}else{
			return 0;
	}
}
void call2_378(){
	globalVariable2_1 += 1;
}
int func2_378(){
	int ret=0;
	if(globalVariable2_1== 378){
		ret=func2_379();
		return ret;
	}else{
			return 0;
	}
}
void call2_377(){
	globalVariable2_1 += 1;
}
int func2_377(){
	int ret=0;
	if(globalVariable2_1== 377){
		ret=func2_378();
		return ret;
	}else{
			return 0;
	}
}
void call2_376(){
	globalVariable2_1 += 1;
}
int func2_376(){
	int ret=0;
	if(globalVariable2_1== 376){
		ret=func2_377();
		return ret;
	}else{
			return 0;
	}
}
void call2_375(){
	globalVariable2_1 += 1;
}
int func2_375(){
	int ret=0;
	if(globalVariable2_1== 375){
		ret=func2_376();
		return ret;
	}else{
			return 0;
	}
}
void call2_374(){
	globalVariable2_1 += 1;
}
int func2_374(){
	int ret=0;
	if(globalVariable2_1== 374){
		ret=func2_375();
		return ret;
	}else{
			return 0;
	}
}
void call2_373(){
	globalVariable2_1 += 1;
}
int func2_373(){
	int ret=0;
	if(globalVariable2_1== 373){
		ret=func2_374();
		return ret;
	}else{
			return 0;
	}
}
void call2_372(){
	globalVariable2_1 += 1;
}
int func2_372(){
	int ret=0;
	if(globalVariable2_1== 372){
		ret=func2_373();
		return ret;
	}else{
			return 0;
	}
}
void call2_371(){
	globalVariable2_1 += 1;
}
int func2_371(){
	int ret=0;
	if(globalVariable2_1== 371){
		ret=func2_372();
		return ret;
	}else{
			return 0;
	}
}
void call2_370(){
	globalVariable2_1 += 1;
}
int func2_370(){
	int ret=0;
	if(globalVariable2_1== 370){
		ret=func2_371();
		return ret;
	}else{
			return 0;
	}
}
void call2_369(){
	globalVariable2_1 += 1;
}
int func2_369(){
	int ret=0;
	if(globalVariable2_1== 369){
		ret=func2_370();
		return ret;
	}else{
			return 0;
	}
}
void call2_368(){
	globalVariable2_1 += 1;
}
int func2_368(){
	int ret=0;
	if(globalVariable2_1== 368){
		ret=func2_369();
		return ret;
	}else{
			return 0;
	}
}
void call2_367(){
	globalVariable2_1 += 1;
}
int func2_367(){
	int ret=0;
	if(globalVariable2_1== 367){
		ret=func2_368();
		return ret;
	}else{
			return 0;
	}
}
void call2_366(){
	globalVariable2_1 += 1;
}
int func2_366(){
	int ret=0;
	if(globalVariable2_1== 366){
		ret=func2_367();
		return ret;
	}else{
			return 0;
	}
}
void call2_365(){
	globalVariable2_1 += 1;
}
int func2_365(){
	int ret=0;
	if(globalVariable2_1== 365){
		ret=func2_366();
		return ret;
	}else{
			return 0;
	}
}
void call2_364(){
	globalVariable2_1 += 1;
}
int func2_364(){
	int ret=0;
	if(globalVariable2_1== 364){
		ret=func2_365();
		return ret;
	}else{
			return 0;
	}
}
void call2_363(){
	globalVariable2_1 += 1;
}
int func2_363(){
	int ret=0;
	if(globalVariable2_1== 363){
		ret=func2_364();
		return ret;
	}else{
			return 0;
	}
}
void call2_362(){
	globalVariable2_1 += 1;
}
int func2_362(){
	int ret=0;
	if(globalVariable2_1== 362){
		ret=func2_363();
		return ret;
	}else{
			return 0;
	}
}
void call2_361(){
	globalVariable2_1 += 1;
}
int func2_361(){
	int ret=0;
	if(globalVariable2_1== 361){
		ret=func2_362();
		return ret;
	}else{
			return 0;
	}
}
void call2_360(){
	globalVariable2_1 += 1;
}
int func2_360(){
	int ret=0;
	if(globalVariable2_1== 360){
		ret=func2_361();
		return ret;
	}else{
			return 0;
	}
}
void call2_359(){
	globalVariable2_1 += 1;
}
int func2_359(){
	int ret=0;
	if(globalVariable2_1== 359){
		ret=func2_360();
		return ret;
	}else{
			return 0;
	}
}
void call2_358(){
	globalVariable2_1 += 1;
}
int func2_358(){
	int ret=0;
	if(globalVariable2_1== 358){
		ret=func2_359();
		return ret;
	}else{
			return 0;
	}
}
void call2_357(){
	globalVariable2_1 += 1;
}
int func2_357(){
	int ret=0;
	if(globalVariable2_1== 357){
		ret=func2_358();
		return ret;
	}else{
			return 0;
	}
}
void call2_356(){
	globalVariable2_1 += 1;
}
int func2_356(){
	int ret=0;
	if(globalVariable2_1== 356){
		ret=func2_357();
		return ret;
	}else{
			return 0;
	}
}
void call2_355(){
	globalVariable2_1 += 1;
}
int func2_355(){
	int ret=0;
	if(globalVariable2_1== 355){
		ret=func2_356();
		return ret;
	}else{
			return 0;
	}
}
void call2_354(){
	globalVariable2_1 += 1;
}
int func2_354(){
	int ret=0;
	if(globalVariable2_1== 354){
		ret=func2_355();
		return ret;
	}else{
			return 0;
	}
}
void call2_353(){
	globalVariable2_1 += 1;
}
int func2_353(){
	int ret=0;
	if(globalVariable2_1== 353){
		ret=func2_354();
		return ret;
	}else{
			return 0;
	}
}
void call2_352(){
	globalVariable2_1 += 1;
}
int func2_352(){
	int ret=0;
	if(globalVariable2_1== 352){
		ret=func2_353();
		return ret;
	}else{
			return 0;
	}
}
void call2_351(){
	globalVariable2_1 += 1;
}
int func2_351(){
	int ret=0;
	if(globalVariable2_1== 351){
		ret=func2_352();
		return ret;
	}else{
			return 0;
	}
}
void call2_350(){
	globalVariable2_1 += 1;
}
int func2_350(){
	int ret=0;
	if(globalVariable2_1== 350){
		ret=func2_351();
		return ret;
	}else{
			return 0;
	}
}
void call2_349(){
	globalVariable2_1 += 1;
}
int func2_349(){
	int ret=0;
	if(globalVariable2_1== 349){
		ret=func2_350();
		return ret;
	}else{
			return 0;
	}
}
void call2_348(){
	globalVariable2_1 += 1;
}
int func2_348(){
	int ret=0;
	if(globalVariable2_1== 348){
		ret=func2_349();
		return ret;
	}else{
			return 0;
	}
}
void call2_347(){
	globalVariable2_1 += 1;
}
int func2_347(){
	int ret=0;
	if(globalVariable2_1== 347){
		ret=func2_348();
		return ret;
	}else{
			return 0;
	}
}
void call2_346(){
	globalVariable2_1 += 1;
}
int func2_346(){
	int ret=0;
	if(globalVariable2_1== 346){
		ret=func2_347();
		return ret;
	}else{
			return 0;
	}
}
void call2_345(){
	globalVariable2_1 += 1;
}
int func2_345(){
	int ret=0;
	if(globalVariable2_1== 345){
		ret=func2_346();
		return ret;
	}else{
			return 0;
	}
}
void call2_344(){
	globalVariable2_1 += 1;
}
int func2_344(){
	int ret=0;
	if(globalVariable2_1== 344){
		ret=func2_345();
		return ret;
	}else{
			return 0;
	}
}
void call2_343(){
	globalVariable2_1 += 1;
}
int func2_343(){
	int ret=0;
	if(globalVariable2_1== 343){
		ret=func2_344();
		return ret;
	}else{
			return 0;
	}
}
void call2_342(){
	globalVariable2_1 += 1;
}
int func2_342(){
	int ret=0;
	if(globalVariable2_1== 342){
		ret=func2_343();
		return ret;
	}else{
			return 0;
	}
}
void call2_341(){
	globalVariable2_1 += 1;
}
int func2_341(){
	int ret=0;
	if(globalVariable2_1== 341){
		ret=func2_342();
		return ret;
	}else{
			return 0;
	}
}
void call2_340(){
	globalVariable2_1 += 1;
}
int func2_340(){
	int ret=0;
	if(globalVariable2_1== 340){
		ret=func2_341();
		return ret;
	}else{
			return 0;
	}
}
void call2_339(){
	globalVariable2_1 += 1;
}
int func2_339(){
	int ret=0;
	if(globalVariable2_1== 339){
		ret=func2_340();
		return ret;
	}else{
			return 0;
	}
}
void call2_338(){
	globalVariable2_1 += 1;
}
int func2_338(){
	int ret=0;
	if(globalVariable2_1== 338){
		ret=func2_339();
		return ret;
	}else{
			return 0;
	}
}
void call2_337(){
	globalVariable2_1 += 1;
}
int func2_337(){
	int ret=0;
	if(globalVariable2_1== 337){
		ret=func2_338();
		return ret;
	}else{
			return 0;
	}
}
void call2_336(){
	globalVariable2_1 += 1;
}
int func2_336(){
	int ret=0;
	if(globalVariable2_1== 336){
		ret=func2_337();
		return ret;
	}else{
			return 0;
	}
}
void call2_335(){
	globalVariable2_1 += 1;
}
int func2_335(){
	int ret=0;
	if(globalVariable2_1== 335){
		ret=func2_336();
		return ret;
	}else{
			return 0;
	}
}
void call2_334(){
	globalVariable2_1 += 1;
}
int func2_334(){
	int ret=0;
	if(globalVariable2_1== 334){
		ret=func2_335();
		return ret;
	}else{
			return 0;
	}
}
void call2_333(){
	globalVariable2_1 += 1;
}
int func2_333(){
	int ret=0;
	if(globalVariable2_1== 333){
		ret=func2_334();
		return ret;
	}else{
			return 0;
	}
}
void call2_332(){
	globalVariable2_1 += 1;
}
int func2_332(){
	int ret=0;
	if(globalVariable2_1== 332){
		ret=func2_333();
		return ret;
	}else{
			return 0;
	}
}
void call2_331(){
	globalVariable2_1 += 1;
}
int func2_331(){
	int ret=0;
	if(globalVariable2_1== 331){
		ret=func2_332();
		return ret;
	}else{
			return 0;
	}
}
void call2_330(){
	globalVariable2_1 += 1;
}
int func2_330(){
	int ret=0;
	if(globalVariable2_1== 330){
		ret=func2_331();
		return ret;
	}else{
			return 0;
	}
}
void call2_329(){
	globalVariable2_1 += 1;
}
int func2_329(){
	int ret=0;
	if(globalVariable2_1== 329){
		ret=func2_330();
		return ret;
	}else{
			return 0;
	}
}
void call2_328(){
	globalVariable2_1 += 1;
}
int func2_328(){
	int ret=0;
	if(globalVariable2_1== 328){
		ret=func2_329();
		return ret;
	}else{
			return 0;
	}
}
void call2_327(){
	globalVariable2_1 += 1;
}
int func2_327(){
	int ret=0;
	if(globalVariable2_1== 327){
		ret=func2_328();
		return ret;
	}else{
			return 0;
	}
}
void call2_326(){
	globalVariable2_1 += 1;
}
int func2_326(){
	int ret=0;
	if(globalVariable2_1== 326){
		ret=func2_327();
		return ret;
	}else{
			return 0;
	}
}
void call2_325(){
	globalVariable2_1 += 1;
}
int func2_325(){
	int ret=0;
	if(globalVariable2_1== 325){
		ret=func2_326();
		return ret;
	}else{
			return 0;
	}
}
void call2_324(){
	globalVariable2_1 += 1;
}
int func2_324(){
	int ret=0;
	if(globalVariable2_1== 324){
		ret=func2_325();
		return ret;
	}else{
			return 0;
	}
}
void call2_323(){
	globalVariable2_1 += 1;
}
int func2_323(){
	int ret=0;
	if(globalVariable2_1== 323){
		ret=func2_324();
		return ret;
	}else{
			return 0;
	}
}
void call2_322(){
	globalVariable2_1 += 1;
}
int func2_322(){
	int ret=0;
	if(globalVariable2_1== 322){
		ret=func2_323();
		return ret;
	}else{
			return 0;
	}
}
void call2_321(){
	globalVariable2_1 += 1;
}
int func2_321(){
	int ret=0;
	if(globalVariable2_1== 321){
		ret=func2_322();
		return ret;
	}else{
			return 0;
	}
}
void call2_320(){
	globalVariable2_1 += 1;
}
int func2_320(){
	int ret=0;
	if(globalVariable2_1== 320){
		ret=func2_321();
		return ret;
	}else{
			return 0;
	}
}
void call2_319(){
	globalVariable2_1 += 1;
}
int func2_319(){
	int ret=0;
	if(globalVariable2_1== 319){
		ret=func2_320();
		return ret;
	}else{
			return 0;
	}
}
void call2_318(){
	globalVariable2_1 += 1;
}
int func2_318(){
	int ret=0;
	if(globalVariable2_1== 318){
		ret=func2_319();
		return ret;
	}else{
			return 0;
	}
}
void call2_317(){
	globalVariable2_1 += 1;
}
int func2_317(){
	int ret=0;
	if(globalVariable2_1== 317){
		ret=func2_318();
		return ret;
	}else{
			return 0;
	}
}
void call2_316(){
	globalVariable2_1 += 1;
}
int func2_316(){
	int ret=0;
	if(globalVariable2_1== 316){
		ret=func2_317();
		return ret;
	}else{
			return 0;
	}
}
void call2_315(){
	globalVariable2_1 += 1;
}
int func2_315(){
	int ret=0;
	if(globalVariable2_1== 315){
		ret=func2_316();
		return ret;
	}else{
			return 0;
	}
}
void call2_314(){
	globalVariable2_1 += 1;
}
int func2_314(){
	int ret=0;
	if(globalVariable2_1== 314){
		ret=func2_315();
		return ret;
	}else{
			return 0;
	}
}
void call2_313(){
	globalVariable2_1 += 1;
}
int func2_313(){
	int ret=0;
	if(globalVariable2_1== 313){
		ret=func2_314();
		return ret;
	}else{
			return 0;
	}
}
void call2_312(){
	globalVariable2_1 += 1;
}
int func2_312(){
	int ret=0;
	if(globalVariable2_1== 312){
		ret=func2_313();
		return ret;
	}else{
			return 0;
	}
}
void call2_311(){
	globalVariable2_1 += 1;
}
int func2_311(){
	int ret=0;
	if(globalVariable2_1== 311){
		ret=func2_312();
		return ret;
	}else{
			return 0;
	}
}
void call2_310(){
	globalVariable2_1 += 1;
}
int func2_310(){
	int ret=0;
	if(globalVariable2_1== 310){
		ret=func2_311();
		return ret;
	}else{
			return 0;
	}
}
void call2_309(){
	globalVariable2_1 += 1;
}
int func2_309(){
	int ret=0;
	if(globalVariable2_1== 309){
		ret=func2_310();
		return ret;
	}else{
			return 0;
	}
}
void call2_308(){
	globalVariable2_1 += 1;
}
int func2_308(){
	int ret=0;
	if(globalVariable2_1== 308){
		ret=func2_309();
		return ret;
	}else{
			return 0;
	}
}
void call2_307(){
	globalVariable2_1 += 1;
}
int func2_307(){
	int ret=0;
	if(globalVariable2_1== 307){
		ret=func2_308();
		return ret;
	}else{
			return 0;
	}
}
void call2_306(){
	globalVariable2_1 += 1;
}
int func2_306(){
	int ret=0;
	if(globalVariable2_1== 306){
		ret=func2_307();
		return ret;
	}else{
			return 0;
	}
}
void call2_305(){
	globalVariable2_1 += 1;
}
int func2_305(){
	int ret=0;
	if(globalVariable2_1== 305){
		ret=func2_306();
		return ret;
	}else{
			return 0;
	}
}
void call2_304(){
	globalVariable2_1 += 1;
}
int func2_304(){
	int ret=0;
	if(globalVariable2_1== 304){
		ret=func2_305();
		return ret;
	}else{
			return 0;
	}
}
void call2_303(){
	globalVariable2_1 += 1;
}
int func2_303(){
	int ret=0;
	if(globalVariable2_1== 303){
		ret=func2_304();
		return ret;
	}else{
			return 0;
	}
}
void call2_302(){
	globalVariable2_1 += 1;
}
int func2_302(){
	int ret=0;
	if(globalVariable2_1== 302){
		ret=func2_303();
		return ret;
	}else{
			return 0;
	}
}
void call2_301(){
	globalVariable2_1 += 1;
}
int func2_301(){
	int ret=0;
	if(globalVariable2_1== 301){
		ret=func2_302();
		return ret;
	}else{
			return 0;
	}
}
void call2_300(){
	globalVariable2_1 += 1;
}
int func2_300(){
	int ret=0;
	if(globalVariable2_1== 300){
		ret=func2_301();
		return ret;
	}else{
			return 0;
	}
}
void call2_299(){
	globalVariable2_1 += 1;
}
int func2_299(){
	int ret=0;
	if(globalVariable2_1== 299){
		ret=func2_300();
		return ret;
	}else{
			return 0;
	}
}
void call2_298(){
	globalVariable2_1 += 1;
}
int func2_298(){
	int ret=0;
	if(globalVariable2_1== 298){
		ret=func2_299();
		return ret;
	}else{
			return 0;
	}
}
void call2_297(){
	globalVariable2_1 += 1;
}
int func2_297(){
	int ret=0;
	if(globalVariable2_1== 297){
		ret=func2_298();
		return ret;
	}else{
			return 0;
	}
}
void call2_296(){
	globalVariable2_1 += 1;
}
int func2_296(){
	int ret=0;
	if(globalVariable2_1== 296){
		ret=func2_297();
		return ret;
	}else{
			return 0;
	}
}
void call2_295(){
	globalVariable2_1 += 1;
}
int func2_295(){
	int ret=0;
	if(globalVariable2_1== 295){
		ret=func2_296();
		return ret;
	}else{
			return 0;
	}
}
void call2_294(){
	globalVariable2_1 += 1;
}
int func2_294(){
	int ret=0;
	if(globalVariable2_1== 294){
		ret=func2_295();
		return ret;
	}else{
			return 0;
	}
}
void call2_293(){
	globalVariable2_1 += 1;
}
int func2_293(){
	int ret=0;
	if(globalVariable2_1== 293){
		ret=func2_294();
		return ret;
	}else{
			return 0;
	}
}
void call2_292(){
	globalVariable2_1 += 1;
}
int func2_292(){
	int ret=0;
	if(globalVariable2_1== 292){
		ret=func2_293();
		return ret;
	}else{
			return 0;
	}
}
void call2_291(){
	globalVariable2_1 += 1;
}
int func2_291(){
	int ret=0;
	if(globalVariable2_1== 291){
		ret=func2_292();
		return ret;
	}else{
			return 0;
	}
}
void call2_290(){
	globalVariable2_1 += 1;
}
int func2_290(){
	int ret=0;
	if(globalVariable2_1== 290){
		ret=func2_291();
		return ret;
	}else{
			return 0;
	}
}
void call2_289(){
	globalVariable2_1 += 1;
}
int func2_289(){
	int ret=0;
	if(globalVariable2_1== 289){
		ret=func2_290();
		return ret;
	}else{
			return 0;
	}
}
void call2_288(){
	globalVariable2_1 += 1;
}
int func2_288(){
	int ret=0;
	if(globalVariable2_1== 288){
		ret=func2_289();
		return ret;
	}else{
			return 0;
	}
}
void call2_287(){
	globalVariable2_1 += 1;
}
int func2_287(){
	int ret=0;
	if(globalVariable2_1== 287){
		ret=func2_288();
		return ret;
	}else{
			return 0;
	}
}
void call2_286(){
	globalVariable2_1 += 1;
}
int func2_286(){
	int ret=0;
	if(globalVariable2_1== 286){
		ret=func2_287();
		return ret;
	}else{
			return 0;
	}
}
void call2_285(){
	globalVariable2_1 += 1;
}
int func2_285(){
	int ret=0;
	if(globalVariable2_1== 285){
		ret=func2_286();
		return ret;
	}else{
			return 0;
	}
}
void call2_284(){
	globalVariable2_1 += 1;
}
int func2_284(){
	int ret=0;
	if(globalVariable2_1== 284){
		ret=func2_285();
		return ret;
	}else{
			return 0;
	}
}
void call2_283(){
	globalVariable2_1 += 1;
}
int func2_283(){
	int ret=0;
	if(globalVariable2_1== 283){
		ret=func2_284();
		return ret;
	}else{
			return 0;
	}
}
void call2_282(){
	globalVariable2_1 += 1;
}
int func2_282(){
	int ret=0;
	if(globalVariable2_1== 282){
		ret=func2_283();
		return ret;
	}else{
			return 0;
	}
}
void call2_281(){
	globalVariable2_1 += 1;
}
int func2_281(){
	int ret=0;
	if(globalVariable2_1== 281){
		ret=func2_282();
		return ret;
	}else{
			return 0;
	}
}
void call2_280(){
	globalVariable2_1 += 1;
}
int func2_280(){
	int ret=0;
	if(globalVariable2_1== 280){
		ret=func2_281();
		return ret;
	}else{
			return 0;
	}
}
void call2_279(){
	globalVariable2_1 += 1;
}
int func2_279(){
	int ret=0;
	if(globalVariable2_1== 279){
		ret=func2_280();
		return ret;
	}else{
			return 0;
	}
}
void call2_278(){
	globalVariable2_1 += 1;
}
int func2_278(){
	int ret=0;
	if(globalVariable2_1== 278){
		ret=func2_279();
		return ret;
	}else{
			return 0;
	}
}
void call2_277(){
	globalVariable2_1 += 1;
}
int func2_277(){
	int ret=0;
	if(globalVariable2_1== 277){
		ret=func2_278();
		return ret;
	}else{
			return 0;
	}
}
void call2_276(){
	globalVariable2_1 += 1;
}
int func2_276(){
	int ret=0;
	if(globalVariable2_1== 276){
		ret=func2_277();
		return ret;
	}else{
			return 0;
	}
}
void call2_275(){
	globalVariable2_1 += 1;
}
int func2_275(){
	int ret=0;
	if(globalVariable2_1== 275){
		ret=func2_276();
		return ret;
	}else{
			return 0;
	}
}
void call2_274(){
	globalVariable2_1 += 1;
}
int func2_274(){
	int ret=0;
	if(globalVariable2_1== 274){
		ret=func2_275();
		return ret;
	}else{
			return 0;
	}
}
void call2_273(){
	globalVariable2_1 += 1;
}
int func2_273(){
	int ret=0;
	if(globalVariable2_1== 273){
		ret=func2_274();
		return ret;
	}else{
			return 0;
	}
}
void call2_272(){
	globalVariable2_1 += 1;
}
int func2_272(){
	int ret=0;
	if(globalVariable2_1== 272){
		ret=func2_273();
		return ret;
	}else{
			return 0;
	}
}
void call2_271(){
	globalVariable2_1 += 1;
}
int func2_271(){
	int ret=0;
	if(globalVariable2_1== 271){
		ret=func2_272();
		return ret;
	}else{
			return 0;
	}
}
void call2_270(){
	globalVariable2_1 += 1;
}
int func2_270(){
	int ret=0;
	if(globalVariable2_1== 270){
		ret=func2_271();
		return ret;
	}else{
			return 0;
	}
}
void call2_269(){
	globalVariable2_1 += 1;
}
int func2_269(){
	int ret=0;
	if(globalVariable2_1== 269){
		ret=func2_270();
		return ret;
	}else{
			return 0;
	}
}
void call2_268(){
	globalVariable2_1 += 1;
}
int func2_268(){
	int ret=0;
	if(globalVariable2_1== 268){
		ret=func2_269();
		return ret;
	}else{
			return 0;
	}
}
void call2_267(){
	globalVariable2_1 += 1;
}
int func2_267(){
	int ret=0;
	if(globalVariable2_1== 267){
		ret=func2_268();
		return ret;
	}else{
			return 0;
	}
}
void call2_266(){
	globalVariable2_1 += 1;
}
int func2_266(){
	int ret=0;
	if(globalVariable2_1== 266){
		ret=func2_267();
		return ret;
	}else{
			return 0;
	}
}
void call2_265(){
	globalVariable2_1 += 1;
}
int func2_265(){
	int ret=0;
	if(globalVariable2_1== 265){
		ret=func2_266();
		return ret;
	}else{
			return 0;
	}
}
void call2_264(){
	globalVariable2_1 += 1;
}
int func2_264(){
	int ret=0;
	if(globalVariable2_1== 264){
		ret=func2_265();
		return ret;
	}else{
			return 0;
	}
}
void call2_263(){
	globalVariable2_1 += 1;
}
int func2_263(){
	int ret=0;
	if(globalVariable2_1== 263){
		ret=func2_264();
		return ret;
	}else{
			return 0;
	}
}
void call2_262(){
	globalVariable2_1 += 1;
}
int func2_262(){
	int ret=0;
	if(globalVariable2_1== 262){
		ret=func2_263();
		return ret;
	}else{
			return 0;
	}
}
void call2_261(){
	globalVariable2_1 += 1;
}
int func2_261(){
	int ret=0;
	if(globalVariable2_1== 261){
		ret=func2_262();
		return ret;
	}else{
			return 0;
	}
}
void call2_260(){
	globalVariable2_1 += 1;
}
int func2_260(){
	int ret=0;
	if(globalVariable2_1== 260){
		ret=func2_261();
		return ret;
	}else{
			return 0;
	}
}
void call2_259(){
	globalVariable2_1 += 1;
}
int func2_259(){
	int ret=0;
	if(globalVariable2_1== 259){
		ret=func2_260();
		return ret;
	}else{
			return 0;
	}
}
void call2_258(){
	globalVariable2_1 += 1;
}
int func2_258(){
	int ret=0;
	if(globalVariable2_1== 258){
		ret=func2_259();
		return ret;
	}else{
			return 0;
	}
}
void call2_257(){
	globalVariable2_1 += 1;
}
int func2_257(){
	int ret=0;
	if(globalVariable2_1== 257){
		ret=func2_258();
		return ret;
	}else{
			return 0;
	}
}
void call2_256(){
	globalVariable2_1 += 1;
}
int func2_256(){
	int ret=0;
	if(globalVariable2_1== 256){
		ret=func2_257();
		return ret;
	}else{
			return 0;
	}
}
void call2_255(){
	globalVariable2_1 += 1;
}
int func2_255(){
	int ret=0;
	if(globalVariable2_1== 255){
		ret=func2_256();
		return ret;
	}else{
			return 0;
	}
}
void call2_254(){
	globalVariable2_1 += 1;
}
int func2_254(){
	int ret=0;
	if(globalVariable2_1== 254){
		ret=func2_255();
		return ret;
	}else{
			return 0;
	}
}
void call2_253(){
	globalVariable2_1 += 1;
}
int func2_253(){
	int ret=0;
	if(globalVariable2_1== 253){
		ret=func2_254();
		return ret;
	}else{
			return 0;
	}
}
void call2_252(){
	globalVariable2_1 += 1;
}
int func2_252(){
	int ret=0;
	if(globalVariable2_1== 252){
		ret=func2_253();
		return ret;
	}else{
			return 0;
	}
}
void call2_251(){
	globalVariable2_1 += 1;
}
int func2_251(){
	int ret=0;
	if(globalVariable2_1== 251){
		ret=func2_252();
		return ret;
	}else{
			return 0;
	}
}
void call2_250(){
	globalVariable2_1 += 1;
}
int func2_250(){
	int ret=0;
	if(globalVariable2_1== 250){
		ret=func2_251();
		return ret;
	}else{
			return 0;
	}
}
void call2_249(){
	globalVariable2_1 += 1;
}
int func2_249(){
	int ret=0;
	if(globalVariable2_1== 249){
		ret=func2_250();
		return ret;
	}else{
			return 0;
	}
}
void call2_248(){
	globalVariable2_1 += 1;
}
int func2_248(){
	int ret=0;
	if(globalVariable2_1== 248){
		ret=func2_249();
		return ret;
	}else{
			return 0;
	}
}
void call2_247(){
	globalVariable2_1 += 1;
}
int func2_247(){
	int ret=0;
	if(globalVariable2_1== 247){
		ret=func2_248();
		return ret;
	}else{
			return 0;
	}
}
void call2_246(){
	globalVariable2_1 += 1;
}
int func2_246(){
	int ret=0;
	if(globalVariable2_1== 246){
		ret=func2_247();
		return ret;
	}else{
			return 0;
	}
}
void call2_245(){
	globalVariable2_1 += 1;
}
int func2_245(){
	int ret=0;
	if(globalVariable2_1== 245){
		ret=func2_246();
		return ret;
	}else{
			return 0;
	}
}
void call2_244(){
	globalVariable2_1 += 1;
}
int func2_244(){
	int ret=0;
	if(globalVariable2_1== 244){
		ret=func2_245();
		return ret;
	}else{
			return 0;
	}
}
void call2_243(){
	globalVariable2_1 += 1;
}
int func2_243(){
	int ret=0;
	if(globalVariable2_1== 243){
		ret=func2_244();
		return ret;
	}else{
			return 0;
	}
}
void call2_242(){
	globalVariable2_1 += 1;
}
int func2_242(){
	int ret=0;
	if(globalVariable2_1== 242){
		ret=func2_243();
		return ret;
	}else{
			return 0;
	}
}
void call2_241(){
	globalVariable2_1 += 1;
}
int func2_241(){
	int ret=0;
	if(globalVariable2_1== 241){
		ret=func2_242();
		return ret;
	}else{
			return 0;
	}
}
void call2_240(){
	globalVariable2_1 += 1;
}
int func2_240(){
	int ret=0;
	if(globalVariable2_1== 240){
		ret=func2_241();
		return ret;
	}else{
			return 0;
	}
}
void call2_239(){
	globalVariable2_1 += 1;
}
int func2_239(){
	int ret=0;
	if(globalVariable2_1== 239){
		ret=func2_240();
		return ret;
	}else{
			return 0;
	}
}
void call2_238(){
	globalVariable2_1 += 1;
}
int func2_238(){
	int ret=0;
	if(globalVariable2_1== 238){
		ret=func2_239();
		return ret;
	}else{
			return 0;
	}
}
void call2_237(){
	globalVariable2_1 += 1;
}
int func2_237(){
	int ret=0;
	if(globalVariable2_1== 237){
		ret=func2_238();
		return ret;
	}else{
			return 0;
	}
}
void call2_236(){
	globalVariable2_1 += 1;
}
int func2_236(){
	int ret=0;
	if(globalVariable2_1== 236){
		ret=func2_237();
		return ret;
	}else{
			return 0;
	}
}
void call2_235(){
	globalVariable2_1 += 1;
}
int func2_235(){
	int ret=0;
	if(globalVariable2_1== 235){
		ret=func2_236();
		return ret;
	}else{
			return 0;
	}
}
void call2_234(){
	globalVariable2_1 += 1;
}
int func2_234(){
	int ret=0;
	if(globalVariable2_1== 234){
		ret=func2_235();
		return ret;
	}else{
			return 0;
	}
}
void call2_233(){
	globalVariable2_1 += 1;
}
int func2_233(){
	int ret=0;
	if(globalVariable2_1== 233){
		ret=func2_234();
		return ret;
	}else{
			return 0;
	}
}
void call2_232(){
	globalVariable2_1 += 1;
}
int func2_232(){
	int ret=0;
	if(globalVariable2_1== 232){
		ret=func2_233();
		return ret;
	}else{
			return 0;
	}
}
void call2_231(){
	globalVariable2_1 += 1;
}
int func2_231(){
	int ret=0;
	if(globalVariable2_1== 231){
		ret=func2_232();
		return ret;
	}else{
			return 0;
	}
}
void call2_230(){
	globalVariable2_1 += 1;
}
int func2_230(){
	int ret=0;
	if(globalVariable2_1== 230){
		ret=func2_231();
		return ret;
	}else{
			return 0;
	}
}
void call2_229(){
	globalVariable2_1 += 1;
}
int func2_229(){
	int ret=0;
	if(globalVariable2_1== 229){
		ret=func2_230();
		return ret;
	}else{
			return 0;
	}
}
void call2_228(){
	globalVariable2_1 += 1;
}
int func2_228(){
	int ret=0;
	if(globalVariable2_1== 228){
		ret=func2_229();
		return ret;
	}else{
			return 0;
	}
}
void call2_227(){
	globalVariable2_1 += 1;
}
int func2_227(){
	int ret=0;
	if(globalVariable2_1== 227){
		ret=func2_228();
		return ret;
	}else{
			return 0;
	}
}
void call2_226(){
	globalVariable2_1 += 1;
}
int func2_226(){
	int ret=0;
	if(globalVariable2_1== 226){
		ret=func2_227();
		return ret;
	}else{
			return 0;
	}
}
void call2_225(){
	globalVariable2_1 += 1;
}
int func2_225(){
	int ret=0;
	if(globalVariable2_1== 225){
		ret=func2_226();
		return ret;
	}else{
			return 0;
	}
}
void call2_224(){
	globalVariable2_1 += 1;
}
int func2_224(){
	int ret=0;
	if(globalVariable2_1== 224){
		ret=func2_225();
		return ret;
	}else{
			return 0;
	}
}
void call2_223(){
	globalVariable2_1 += 1;
}
int func2_223(){
	int ret=0;
	if(globalVariable2_1== 223){
		ret=func2_224();
		return ret;
	}else{
			return 0;
	}
}
void call2_222(){
	globalVariable2_1 += 1;
}
int func2_222(){
	int ret=0;
	if(globalVariable2_1== 222){
		ret=func2_223();
		return ret;
	}else{
			return 0;
	}
}
void call2_221(){
	globalVariable2_1 += 1;
}
int func2_221(){
	int ret=0;
	if(globalVariable2_1== 221){
		ret=func2_222();
		return ret;
	}else{
			return 0;
	}
}
void call2_220(){
	globalVariable2_1 += 1;
}
int func2_220(){
	int ret=0;
	if(globalVariable2_1== 220){
		ret=func2_221();
		return ret;
	}else{
			return 0;
	}
}
void call2_219(){
	globalVariable2_1 += 1;
}
int func2_219(){
	int ret=0;
	if(globalVariable2_1== 219){
		ret=func2_220();
		return ret;
	}else{
			return 0;
	}
}
void call2_218(){
	globalVariable2_1 += 1;
}
int func2_218(){
	int ret=0;
	if(globalVariable2_1== 218){
		ret=func2_219();
		return ret;
	}else{
			return 0;
	}
}
void call2_217(){
	globalVariable2_1 += 1;
}
int func2_217(){
	int ret=0;
	if(globalVariable2_1== 217){
		ret=func2_218();
		return ret;
	}else{
			return 0;
	}
}
void call2_216(){
	globalVariable2_1 += 1;
}
int func2_216(){
	int ret=0;
	if(globalVariable2_1== 216){
		ret=func2_217();
		return ret;
	}else{
			return 0;
	}
}
void call2_215(){
	globalVariable2_1 += 1;
}
int func2_215(){
	int ret=0;
	if(globalVariable2_1== 215){
		ret=func2_216();
		return ret;
	}else{
			return 0;
	}
}
void call2_214(){
	globalVariable2_1 += 1;
}
int func2_214(){
	int ret=0;
	if(globalVariable2_1== 214){
		ret=func2_215();
		return ret;
	}else{
			return 0;
	}
}
void call2_213(){
	globalVariable2_1 += 1;
}
int func2_213(){
	int ret=0;
	if(globalVariable2_1== 213){
		ret=func2_214();
		return ret;
	}else{
			return 0;
	}
}
void call2_212(){
	globalVariable2_1 += 1;
}
int func2_212(){
	int ret=0;
	if(globalVariable2_1== 212){
		ret=func2_213();
		return ret;
	}else{
			return 0;
	}
}
void call2_211(){
	globalVariable2_1 += 1;
}
int func2_211(){
	int ret=0;
	if(globalVariable2_1== 211){
		ret=func2_212();
		return ret;
	}else{
			return 0;
	}
}
void call2_210(){
	globalVariable2_1 += 1;
}
int func2_210(){
	int ret=0;
	if(globalVariable2_1== 210){
		ret=func2_211();
		return ret;
	}else{
			return 0;
	}
}
void call2_209(){
	globalVariable2_1 += 1;
}
int func2_209(){
	int ret=0;
	if(globalVariable2_1== 209){
		ret=func2_210();
		return ret;
	}else{
			return 0;
	}
}
void call2_208(){
	globalVariable2_1 += 1;
}
int func2_208(){
	int ret=0;
	if(globalVariable2_1== 208){
		ret=func2_209();
		return ret;
	}else{
			return 0;
	}
}
void call2_207(){
	globalVariable2_1 += 1;
}
int func2_207(){
	int ret=0;
	if(globalVariable2_1== 207){
		ret=func2_208();
		return ret;
	}else{
			return 0;
	}
}
void call2_206(){
	globalVariable2_1 += 1;
}
int func2_206(){
	int ret=0;
	if(globalVariable2_1== 206){
		ret=func2_207();
		return ret;
	}else{
			return 0;
	}
}
void call2_205(){
	globalVariable2_1 += 1;
}
int func2_205(){
	int ret=0;
	if(globalVariable2_1== 205){
		ret=func2_206();
		return ret;
	}else{
			return 0;
	}
}
void call2_204(){
	globalVariable2_1 += 1;
}
int func2_204(){
	int ret=0;
	if(globalVariable2_1== 204){
		ret=func2_205();
		return ret;
	}else{
			return 0;
	}
}
void call2_203(){
	globalVariable2_1 += 1;
}
int func2_203(){
	int ret=0;
	if(globalVariable2_1== 203){
		ret=func2_204();
		return ret;
	}else{
			return 0;
	}
}
void call2_202(){
	globalVariable2_1 += 1;
}
int func2_202(){
	int ret=0;
	if(globalVariable2_1== 202){
		ret=func2_203();
		return ret;
	}else{
			return 0;
	}
}
void call2_201(){
	globalVariable2_1 += 1;
}
int func2_201(){
	int ret=0;
	if(globalVariable2_1== 201){
		ret=func2_202();
		return ret;
	}else{
			return 0;
	}
}
void call2_200(){
	globalVariable2_1 += 1;
}
int func2_200(){
	int ret=0;
	if(globalVariable2_1== 200){
		ret=func2_201();
		return ret;
	}else{
			return 0;
	}
}
void call2_199(){
	globalVariable2_1 += 1;
}
int func2_199(){
	int ret=0;
	if(globalVariable2_1== 199){
		ret=func2_200();
		return ret;
	}else{
			return 0;
	}
}
void call2_198(){
	globalVariable2_1 += 1;
}
int func2_198(){
	int ret=0;
	if(globalVariable2_1== 198){
		ret=func2_199();
		return ret;
	}else{
			return 0;
	}
}
void call2_197(){
	globalVariable2_1 += 1;
}
int func2_197(){
	int ret=0;
	if(globalVariable2_1== 197){
		ret=func2_198();
		return ret;
	}else{
			return 0;
	}
}
void call2_196(){
	globalVariable2_1 += 1;
}
int func2_196(){
	int ret=0;
	if(globalVariable2_1== 196){
		ret=func2_197();
		return ret;
	}else{
			return 0;
	}
}
void call2_195(){
	globalVariable2_1 += 1;
}
int func2_195(){
	int ret=0;
	if(globalVariable2_1== 195){
		ret=func2_196();
		return ret;
	}else{
			return 0;
	}
}
void call2_194(){
	globalVariable2_1 += 1;
}
int func2_194(){
	int ret=0;
	if(globalVariable2_1== 194){
		ret=func2_195();
		return ret;
	}else{
			return 0;
	}
}
void call2_193(){
	globalVariable2_1 += 1;
}
int func2_193(){
	int ret=0;
	if(globalVariable2_1== 193){
		ret=func2_194();
		return ret;
	}else{
			return 0;
	}
}
void call2_192(){
	globalVariable2_1 += 1;
}
int func2_192(){
	int ret=0;
	if(globalVariable2_1== 192){
		ret=func2_193();
		return ret;
	}else{
			return 0;
	}
}
void call2_191(){
	globalVariable2_1 += 1;
}
int func2_191(){
	int ret=0;
	if(globalVariable2_1== 191){
		ret=func2_192();
		return ret;
	}else{
			return 0;
	}
}
void call2_190(){
	globalVariable2_1 += 1;
}
int func2_190(){
	int ret=0;
	if(globalVariable2_1== 190){
		ret=func2_191();
		return ret;
	}else{
			return 0;
	}
}
void call2_189(){
	globalVariable2_1 += 1;
}
int func2_189(){
	int ret=0;
	if(globalVariable2_1== 189){
		ret=func2_190();
		return ret;
	}else{
			return 0;
	}
}
void call2_188(){
	globalVariable2_1 += 1;
}
int func2_188(){
	int ret=0;
	if(globalVariable2_1== 188){
		ret=func2_189();
		return ret;
	}else{
			return 0;
	}
}
void call2_187(){
	globalVariable2_1 += 1;
}
int func2_187(){
	int ret=0;
	if(globalVariable2_1== 187){
		ret=func2_188();
		return ret;
	}else{
			return 0;
	}
}
void call2_186(){
	globalVariable2_1 += 1;
}
int func2_186(){
	int ret=0;
	if(globalVariable2_1== 186){
		ret=func2_187();
		return ret;
	}else{
			return 0;
	}
}
void call2_185(){
	globalVariable2_1 += 1;
}
int func2_185(){
	int ret=0;
	if(globalVariable2_1== 185){
		ret=func2_186();
		return ret;
	}else{
			return 0;
	}
}
void call2_184(){
	globalVariable2_1 += 1;
}
int func2_184(){
	int ret=0;
	if(globalVariable2_1== 184){
		ret=func2_185();
		return ret;
	}else{
			return 0;
	}
}
void call2_183(){
	globalVariable2_1 += 1;
}
int func2_183(){
	int ret=0;
	if(globalVariable2_1== 183){
		ret=func2_184();
		return ret;
	}else{
			return 0;
	}
}
void call2_182(){
	globalVariable2_1 += 1;
}
int func2_182(){
	int ret=0;
	if(globalVariable2_1== 182){
		ret=func2_183();
		return ret;
	}else{
			return 0;
	}
}
void call2_181(){
	globalVariable2_1 += 1;
}
int func2_181(){
	int ret=0;
	if(globalVariable2_1== 181){
		ret=func2_182();
		return ret;
	}else{
			return 0;
	}
}
void call2_180(){
	globalVariable2_1 += 1;
}
int func2_180(){
	int ret=0;
	if(globalVariable2_1== 180){
		ret=func2_181();
		return ret;
	}else{
			return 0;
	}
}
void call2_179(){
	globalVariable2_1 += 1;
}
int func2_179(){
	int ret=0;
	if(globalVariable2_1== 179){
		ret=func2_180();
		return ret;
	}else{
			return 0;
	}
}
void call2_178(){
	globalVariable2_1 += 1;
}
int func2_178(){
	int ret=0;
	if(globalVariable2_1== 178){
		ret=func2_179();
		return ret;
	}else{
			return 0;
	}
}
void call2_177(){
	globalVariable2_1 += 1;
}
int func2_177(){
	int ret=0;
	if(globalVariable2_1== 177){
		ret=func2_178();
		return ret;
	}else{
			return 0;
	}
}
void call2_176(){
	globalVariable2_1 += 1;
}
int func2_176(){
	int ret=0;
	if(globalVariable2_1== 176){
		ret=func2_177();
		return ret;
	}else{
			return 0;
	}
}
void call2_175(){
	globalVariable2_1 += 1;
}
int func2_175(){
	int ret=0;
	if(globalVariable2_1== 175){
		ret=func2_176();
		return ret;
	}else{
			return 0;
	}
}
void call2_174(){
	globalVariable2_1 += 1;
}
int func2_174(){
	int ret=0;
	if(globalVariable2_1== 174){
		ret=func2_175();
		return ret;
	}else{
			return 0;
	}
}
void call2_173(){
	globalVariable2_1 += 1;
}
int func2_173(){
	int ret=0;
	if(globalVariable2_1== 173){
		ret=func2_174();
		return ret;
	}else{
			return 0;
	}
}
void call2_172(){
	globalVariable2_1 += 1;
}
int func2_172(){
	int ret=0;
	if(globalVariable2_1== 172){
		ret=func2_173();
		return ret;
	}else{
			return 0;
	}
}
void call2_171(){
	globalVariable2_1 += 1;
}
int func2_171(){
	int ret=0;
	if(globalVariable2_1== 171){
		ret=func2_172();
		return ret;
	}else{
			return 0;
	}
}
void call2_170(){
	globalVariable2_1 += 1;
}
int func2_170(){
	int ret=0;
	if(globalVariable2_1== 170){
		ret=func2_171();
		return ret;
	}else{
			return 0;
	}
}
void call2_169(){
	globalVariable2_1 += 1;
}
int func2_169(){
	int ret=0;
	if(globalVariable2_1== 169){
		ret=func2_170();
		return ret;
	}else{
			return 0;
	}
}
void call2_168(){
	globalVariable2_1 += 1;
}
int func2_168(){
	int ret=0;
	if(globalVariable2_1== 168){
		ret=func2_169();
		return ret;
	}else{
			return 0;
	}
}
void call2_167(){
	globalVariable2_1 += 1;
}
int func2_167(){
	int ret=0;
	if(globalVariable2_1== 167){
		ret=func2_168();
		return ret;
	}else{
			return 0;
	}
}
void call2_166(){
	globalVariable2_1 += 1;
}
int func2_166(){
	int ret=0;
	if(globalVariable2_1== 166){
		ret=func2_167();
		return ret;
	}else{
			return 0;
	}
}
void call2_165(){
	globalVariable2_1 += 1;
}
int func2_165(){
	int ret=0;
	if(globalVariable2_1== 165){
		ret=func2_166();
		return ret;
	}else{
			return 0;
	}
}
void call2_164(){
	globalVariable2_1 += 1;
}
int func2_164(){
	int ret=0;
	if(globalVariable2_1== 164){
		ret=func2_165();
		return ret;
	}else{
			return 0;
	}
}
void call2_163(){
	globalVariable2_1 += 1;
}
int func2_163(){
	int ret=0;
	if(globalVariable2_1== 163){
		ret=func2_164();
		return ret;
	}else{
			return 0;
	}
}
void call2_162(){
	globalVariable2_1 += 1;
}
int func2_162(){
	int ret=0;
	if(globalVariable2_1== 162){
		ret=func2_163();
		return ret;
	}else{
			return 0;
	}
}
void call2_161(){
	globalVariable2_1 += 1;
}
int func2_161(){
	int ret=0;
	if(globalVariable2_1== 161){
		ret=func2_162();
		return ret;
	}else{
			return 0;
	}
}
void call2_160(){
	globalVariable2_1 += 1;
}
int func2_160(){
	int ret=0;
	if(globalVariable2_1== 160){
		ret=func2_161();
		return ret;
	}else{
			return 0;
	}
}
void call2_159(){
	globalVariable2_1 += 1;
}
int func2_159(){
	int ret=0;
	if(globalVariable2_1== 159){
		ret=func2_160();
		return ret;
	}else{
			return 0;
	}
}
void call2_158(){
	globalVariable2_1 += 1;
}
int func2_158(){
	int ret=0;
	if(globalVariable2_1== 158){
		ret=func2_159();
		return ret;
	}else{
			return 0;
	}
}
void call2_157(){
	globalVariable2_1 += 1;
}
int func2_157(){
	int ret=0;
	if(globalVariable2_1== 157){
		ret=func2_158();
		return ret;
	}else{
			return 0;
	}
}
void call2_156(){
	globalVariable2_1 += 1;
}
int func2_156(){
	int ret=0;
	if(globalVariable2_1== 156){
		ret=func2_157();
		return ret;
	}else{
			return 0;
	}
}
void call2_155(){
	globalVariable2_1 += 1;
}
int func2_155(){
	int ret=0;
	if(globalVariable2_1== 155){
		ret=func2_156();
		return ret;
	}else{
			return 0;
	}
}
void call2_154(){
	globalVariable2_1 += 1;
}
int func2_154(){
	int ret=0;
	if(globalVariable2_1== 154){
		ret=func2_155();
		return ret;
	}else{
			return 0;
	}
}
void call2_153(){
	globalVariable2_1 += 1;
}
int func2_153(){
	int ret=0;
	if(globalVariable2_1== 153){
		ret=func2_154();
		return ret;
	}else{
			return 0;
	}
}
void call2_152(){
	globalVariable2_1 += 1;
}
int func2_152(){
	int ret=0;
	if(globalVariable2_1== 152){
		ret=func2_153();
		return ret;
	}else{
			return 0;
	}
}
void call2_151(){
	globalVariable2_1 += 1;
}
int func2_151(){
	int ret=0;
	if(globalVariable2_1== 151){
		ret=func2_152();
		return ret;
	}else{
			return 0;
	}
}
void call2_150(){
	globalVariable2_1 += 1;
}
int func2_150(){
	int ret=0;
	if(globalVariable2_1== 150){
		ret=func2_151();
		return ret;
	}else{
			return 0;
	}
}
void call2_149(){
	globalVariable2_1 += 1;
}
int func2_149(){
	int ret=0;
	if(globalVariable2_1== 149){
		ret=func2_150();
		return ret;
	}else{
			return 0;
	}
}
void call2_148(){
	globalVariable2_1 += 1;
}
int func2_148(){
	int ret=0;
	if(globalVariable2_1== 148){
		ret=func2_149();
		return ret;
	}else{
			return 0;
	}
}
void call2_147(){
	globalVariable2_1 += 1;
}
int func2_147(){
	int ret=0;
	if(globalVariable2_1== 147){
		ret=func2_148();
		return ret;
	}else{
			return 0;
	}
}
void call2_146(){
	globalVariable2_1 += 1;
}
int func2_146(){
	int ret=0;
	if(globalVariable2_1== 146){
		ret=func2_147();
		return ret;
	}else{
			return 0;
	}
}
void call2_145(){
	globalVariable2_1 += 1;
}
int func2_145(){
	int ret=0;
	if(globalVariable2_1== 145){
		ret=func2_146();
		return ret;
	}else{
			return 0;
	}
}
void call2_144(){
	globalVariable2_1 += 1;
}
int func2_144(){
	int ret=0;
	if(globalVariable2_1== 144){
		ret=func2_145();
		return ret;
	}else{
			return 0;
	}
}
void call2_143(){
	globalVariable2_1 += 1;
}
int func2_143(){
	int ret=0;
	if(globalVariable2_1== 143){
		ret=func2_144();
		return ret;
	}else{
			return 0;
	}
}
void call2_142(){
	globalVariable2_1 += 1;
}
int func2_142(){
	int ret=0;
	if(globalVariable2_1== 142){
		ret=func2_143();
		return ret;
	}else{
			return 0;
	}
}
void call2_141(){
	globalVariable2_1 += 1;
}
int func2_141(){
	int ret=0;
	if(globalVariable2_1== 141){
		ret=func2_142();
		return ret;
	}else{
			return 0;
	}
}
void call2_140(){
	globalVariable2_1 += 1;
}
int func2_140(){
	int ret=0;
	if(globalVariable2_1== 140){
		ret=func2_141();
		return ret;
	}else{
			return 0;
	}
}
void call2_139(){
	globalVariable2_1 += 1;
}
int func2_139(){
	int ret=0;
	if(globalVariable2_1== 139){
		ret=func2_140();
		return ret;
	}else{
			return 0;
	}
}
void call2_138(){
	globalVariable2_1 += 1;
}
int func2_138(){
	int ret=0;
	if(globalVariable2_1== 138){
		ret=func2_139();
		return ret;
	}else{
			return 0;
	}
}
void call2_137(){
	globalVariable2_1 += 1;
}
int func2_137(){
	int ret=0;
	if(globalVariable2_1== 137){
		ret=func2_138();
		return ret;
	}else{
			return 0;
	}
}
void call2_136(){
	globalVariable2_1 += 1;
}
int func2_136(){
	int ret=0;
	if(globalVariable2_1== 136){
		ret=func2_137();
		return ret;
	}else{
			return 0;
	}
}
void call2_135(){
	globalVariable2_1 += 1;
}
int func2_135(){
	int ret=0;
	if(globalVariable2_1== 135){
		ret=func2_136();
		return ret;
	}else{
			return 0;
	}
}
void call2_134(){
	globalVariable2_1 += 1;
}
int func2_134(){
	int ret=0;
	if(globalVariable2_1== 134){
		ret=func2_135();
		return ret;
	}else{
			return 0;
	}
}
void call2_133(){
	globalVariable2_1 += 1;
}
int func2_133(){
	int ret=0;
	if(globalVariable2_1== 133){
		ret=func2_134();
		return ret;
	}else{
			return 0;
	}
}
void call2_132(){
	globalVariable2_1 += 1;
}
int func2_132(){
	int ret=0;
	if(globalVariable2_1== 132){
		ret=func2_133();
		return ret;
	}else{
			return 0;
	}
}
void call2_131(){
	globalVariable2_1 += 1;
}
int func2_131(){
	int ret=0;
	if(globalVariable2_1== 131){
		ret=func2_132();
		return ret;
	}else{
			return 0;
	}
}
void call2_130(){
	globalVariable2_1 += 1;
}
int func2_130(){
	int ret=0;
	if(globalVariable2_1== 130){
		ret=func2_131();
		return ret;
	}else{
			return 0;
	}
}
void call2_129(){
	globalVariable2_1 += 1;
}
int func2_129(){
	int ret=0;
	if(globalVariable2_1== 129){
		ret=func2_130();
		return ret;
	}else{
			return 0;
	}
}
void call2_128(){
	globalVariable2_1 += 1;
}
int func2_128(){
	int ret=0;
	if(globalVariable2_1== 128){
		ret=func2_129();
		return ret;
	}else{
			return 0;
	}
}
void call2_127(){
	globalVariable2_1 += 1;
}
int func2_127(){
	int ret=0;
	if(globalVariable2_1== 127){
		ret=func2_128();
		return ret;
	}else{
			return 0;
	}
}
void call2_126(){
	globalVariable2_1 += 1;
}
int func2_126(){
	int ret=0;
	if(globalVariable2_1== 126){
		ret=func2_127();
		return ret;
	}else{
			return 0;
	}
}
void call2_125(){
	globalVariable2_1 += 1;
}
int func2_125(){
	int ret=0;
	if(globalVariable2_1== 125){
		ret=func2_126();
		return ret;
	}else{
			return 0;
	}
}
void call2_124(){
	globalVariable2_1 += 1;
}
int func2_124(){
	int ret=0;
	if(globalVariable2_1== 124){
		ret=func2_125();
		return ret;
	}else{
			return 0;
	}
}
void call2_123(){
	globalVariable2_1 += 1;
}
int func2_123(){
	int ret=0;
	if(globalVariable2_1== 123){
		ret=func2_124();
		return ret;
	}else{
			return 0;
	}
}
void call2_122(){
	globalVariable2_1 += 1;
}
int func2_122(){
	int ret=0;
	if(globalVariable2_1== 122){
		ret=func2_123();
		return ret;
	}else{
			return 0;
	}
}
void call2_121(){
	globalVariable2_1 += 1;
}
int func2_121(){
	int ret=0;
	if(globalVariable2_1== 121){
		ret=func2_122();
		return ret;
	}else{
			return 0;
	}
}
void call2_120(){
	globalVariable2_1 += 1;
}
int func2_120(){
	int ret=0;
	if(globalVariable2_1== 120){
		ret=func2_121();
		return ret;
	}else{
			return 0;
	}
}
void call2_119(){
	globalVariable2_1 += 1;
}
int func2_119(){
	int ret=0;
	if(globalVariable2_1== 119){
		ret=func2_120();
		return ret;
	}else{
			return 0;
	}
}
void call2_118(){
	globalVariable2_1 += 1;
}
int func2_118(){
	int ret=0;
	if(globalVariable2_1== 118){
		ret=func2_119();
		return ret;
	}else{
			return 0;
	}
}
void call2_117(){
	globalVariable2_1 += 1;
}
int func2_117(){
	int ret=0;
	if(globalVariable2_1== 117){
		ret=func2_118();
		return ret;
	}else{
			return 0;
	}
}
void call2_116(){
	globalVariable2_1 += 1;
}
int func2_116(){
	int ret=0;
	if(globalVariable2_1== 116){
		ret=func2_117();
		return ret;
	}else{
			return 0;
	}
}
void call2_115(){
	globalVariable2_1 += 1;
}
int func2_115(){
	int ret=0;
	if(globalVariable2_1== 115){
		ret=func2_116();
		return ret;
	}else{
			return 0;
	}
}
void call2_114(){
	globalVariable2_1 += 1;
}
int func2_114(){
	int ret=0;
	if(globalVariable2_1== 114){
		ret=func2_115();
		return ret;
	}else{
			return 0;
	}
}
void call2_113(){
	globalVariable2_1 += 1;
}
int func2_113(){
	int ret=0;
	if(globalVariable2_1== 113){
		ret=func2_114();
		return ret;
	}else{
			return 0;
	}
}
void call2_112(){
	globalVariable2_1 += 1;
}
int func2_112(){
	int ret=0;
	if(globalVariable2_1== 112){
		ret=func2_113();
		return ret;
	}else{
			return 0;
	}
}
void call2_111(){
	globalVariable2_1 += 1;
}
int func2_111(){
	int ret=0;
	if(globalVariable2_1== 111){
		ret=func2_112();
		return ret;
	}else{
			return 0;
	}
}
void call2_110(){
	globalVariable2_1 += 1;
}
int func2_110(){
	int ret=0;
	if(globalVariable2_1== 110){
		ret=func2_111();
		return ret;
	}else{
			return 0;
	}
}
void call2_109(){
	globalVariable2_1 += 1;
}
int func2_109(){
	int ret=0;
	if(globalVariable2_1== 109){
		ret=func2_110();
		return ret;
	}else{
			return 0;
	}
}
void call2_108(){
	globalVariable2_1 += 1;
}
int func2_108(){
	int ret=0;
	if(globalVariable2_1== 108){
		ret=func2_109();
		return ret;
	}else{
			return 0;
	}
}
void call2_107(){
	globalVariable2_1 += 1;
}
int func2_107(){
	int ret=0;
	if(globalVariable2_1== 107){
		ret=func2_108();
		return ret;
	}else{
			return 0;
	}
}
void call2_106(){
	globalVariable2_1 += 1;
}
int func2_106(){
	int ret=0;
	if(globalVariable2_1== 106){
		ret=func2_107();
		return ret;
	}else{
			return 0;
	}
}
void call2_105(){
	globalVariable2_1 += 1;
}
int func2_105(){
	int ret=0;
	if(globalVariable2_1== 105){
		ret=func2_106();
		return ret;
	}else{
			return 0;
	}
}
void call2_104(){
	globalVariable2_1 += 1;
}
int func2_104(){
	int ret=0;
	if(globalVariable2_1== 104){
		ret=func2_105();
		return ret;
	}else{
			return 0;
	}
}
void call2_103(){
	globalVariable2_1 += 1;
}
int func2_103(){
	int ret=0;
	if(globalVariable2_1== 103){
		ret=func2_104();
		return ret;
	}else{
			return 0;
	}
}
void call2_102(){
	globalVariable2_1 += 1;
}
int func2_102(){
	int ret=0;
	if(globalVariable2_1== 102){
		ret=func2_103();
		return ret;
	}else{
			return 0;
	}
}
void call2_101(){
	globalVariable2_1 += 1;
}
int func2_101(){
	int ret=0;
	if(globalVariable2_1== 101){
		ret=func2_102();
		return ret;
	}else{
			return 0;
	}
}
void call2_100(){
	globalVariable2_1 += 1;
}
int func2_100(){
	int ret=0;
	if(globalVariable2_1== 100){
		ret=func2_101();
		return ret;
	}else{
			return 0;
	}
}
void call2_99(){
	globalVariable2_1 += 1;
}
int func2_99(){
	int ret=0;
	if(globalVariable2_1== 99){
		ret=func2_100();
		return ret;
	}else{
			return 0;
	}
}
void call2_98(){
	globalVariable2_1 += 1;
}
int func2_98(){
	int ret=0;
	if(globalVariable2_1== 98){
		ret=func2_99();
		return ret;
	}else{
			return 0;
	}
}
void call2_97(){
	globalVariable2_1 += 1;
}
int func2_97(){
	int ret=0;
	if(globalVariable2_1== 97){
		ret=func2_98();
		return ret;
	}else{
			return 0;
	}
}
void call2_96(){
	globalVariable2_1 += 1;
}
int func2_96(){
	int ret=0;
	if(globalVariable2_1== 96){
		ret=func2_97();
		return ret;
	}else{
			return 0;
	}
}
void call2_95(){
	globalVariable2_1 += 1;
}
int func2_95(){
	int ret=0;
	if(globalVariable2_1== 95){
		ret=func2_96();
		return ret;
	}else{
			return 0;
	}
}
void call2_94(){
	globalVariable2_1 += 1;
}
int func2_94(){
	int ret=0;
	if(globalVariable2_1== 94){
		ret=func2_95();
		return ret;
	}else{
			return 0;
	}
}
void call2_93(){
	globalVariable2_1 += 1;
}
int func2_93(){
	int ret=0;
	if(globalVariable2_1== 93){
		ret=func2_94();
		return ret;
	}else{
			return 0;
	}
}
void call2_92(){
	globalVariable2_1 += 1;
}
int func2_92(){
	int ret=0;
	if(globalVariable2_1== 92){
		ret=func2_93();
		return ret;
	}else{
			return 0;
	}
}
void call2_91(){
	globalVariable2_1 += 1;
}
int func2_91(){
	int ret=0;
	if(globalVariable2_1== 91){
		ret=func2_92();
		return ret;
	}else{
			return 0;
	}
}
void call2_90(){
	globalVariable2_1 += 1;
}
int func2_90(){
	int ret=0;
	if(globalVariable2_1== 90){
		ret=func2_91();
		return ret;
	}else{
			return 0;
	}
}
void call2_89(){
	globalVariable2_1 += 1;
}
int func2_89(){
	int ret=0;
	if(globalVariable2_1== 89){
		ret=func2_90();
		return ret;
	}else{
			return 0;
	}
}
void call2_88(){
	globalVariable2_1 += 1;
}
int func2_88(){
	int ret=0;
	if(globalVariable2_1== 88){
		ret=func2_89();
		return ret;
	}else{
			return 0;
	}
}
void call2_87(){
	globalVariable2_1 += 1;
}
int func2_87(){
	int ret=0;
	if(globalVariable2_1== 87){
		ret=func2_88();
		return ret;
	}else{
			return 0;
	}
}
void call2_86(){
	globalVariable2_1 += 1;
}
int func2_86(){
	int ret=0;
	if(globalVariable2_1== 86){
		ret=func2_87();
		return ret;
	}else{
			return 0;
	}
}
void call2_85(){
	globalVariable2_1 += 1;
}
int func2_85(){
	int ret=0;
	if(globalVariable2_1== 85){
		ret=func2_86();
		return ret;
	}else{
			return 0;
	}
}
void call2_84(){
	globalVariable2_1 += 1;
}
int func2_84(){
	int ret=0;
	if(globalVariable2_1== 84){
		ret=func2_85();
		return ret;
	}else{
			return 0;
	}
}
void call2_83(){
	globalVariable2_1 += 1;
}
int func2_83(){
	int ret=0;
	if(globalVariable2_1== 83){
		ret=func2_84();
		return ret;
	}else{
			return 0;
	}
}
void call2_82(){
	globalVariable2_1 += 1;
}
int func2_82(){
	int ret=0;
	if(globalVariable2_1== 82){
		ret=func2_83();
		return ret;
	}else{
			return 0;
	}
}
void call2_81(){
	globalVariable2_1 += 1;
}
int func2_81(){
	int ret=0;
	if(globalVariable2_1== 81){
		ret=func2_82();
		return ret;
	}else{
			return 0;
	}
}
void call2_80(){
	globalVariable2_1 += 1;
}
int func2_80(){
	int ret=0;
	if(globalVariable2_1== 80){
		ret=func2_81();
		return ret;
	}else{
			return 0;
	}
}
void call2_79(){
	globalVariable2_1 += 1;
}
int func2_79(){
	int ret=0;
	if(globalVariable2_1== 79){
		ret=func2_80();
		return ret;
	}else{
			return 0;
	}
}
void call2_78(){
	globalVariable2_1 += 1;
}
int func2_78(){
	int ret=0;
	if(globalVariable2_1== 78){
		ret=func2_79();
		return ret;
	}else{
			return 0;
	}
}
void call2_77(){
	globalVariable2_1 += 1;
}
int func2_77(){
	int ret=0;
	if(globalVariable2_1== 77){
		ret=func2_78();
		return ret;
	}else{
			return 0;
	}
}
void call2_76(){
	globalVariable2_1 += 1;
}
int func2_76(){
	int ret=0;
	if(globalVariable2_1== 76){
		ret=func2_77();
		return ret;
	}else{
			return 0;
	}
}
void call2_75(){
	globalVariable2_1 += 1;
}
int func2_75(){
	int ret=0;
	if(globalVariable2_1== 75){
		ret=func2_76();
		return ret;
	}else{
			return 0;
	}
}
void call2_74(){
	globalVariable2_1 += 1;
}
int func2_74(){
	int ret=0;
	if(globalVariable2_1== 74){
		ret=func2_75();
		return ret;
	}else{
			return 0;
	}
}
void call2_73(){
	globalVariable2_1 += 1;
}
int func2_73(){
	int ret=0;
	if(globalVariable2_1== 73){
		ret=func2_74();
		return ret;
	}else{
			return 0;
	}
}
void call2_72(){
	globalVariable2_1 += 1;
}
int func2_72(){
	int ret=0;
	if(globalVariable2_1== 72){
		ret=func2_73();
		return ret;
	}else{
			return 0;
	}
}
void call2_71(){
	globalVariable2_1 += 1;
}
int func2_71(){
	int ret=0;
	if(globalVariable2_1== 71){
		ret=func2_72();
		return ret;
	}else{
			return 0;
	}
}
void call2_70(){
	globalVariable2_1 += 1;
}
int func2_70(){
	int ret=0;
	if(globalVariable2_1== 70){
		ret=func2_71();
		return ret;
	}else{
			return 0;
	}
}
void call2_69(){
	globalVariable2_1 += 1;
}
int func2_69(){
	int ret=0;
	if(globalVariable2_1== 69){
		ret=func2_70();
		return ret;
	}else{
			return 0;
	}
}
void call2_68(){
	globalVariable2_1 += 1;
}
int func2_68(){
	int ret=0;
	if(globalVariable2_1== 68){
		ret=func2_69();
		return ret;
	}else{
			return 0;
	}
}
void call2_67(){
	globalVariable2_1 += 1;
}
int func2_67(){
	int ret=0;
	if(globalVariable2_1== 67){
		ret=func2_68();
		return ret;
	}else{
			return 0;
	}
}
void call2_66(){
	globalVariable2_1 += 1;
}
int func2_66(){
	int ret=0;
	if(globalVariable2_1== 66){
		ret=func2_67();
		return ret;
	}else{
			return 0;
	}
}
void call2_65(){
	globalVariable2_1 += 1;
}
int func2_65(){
	int ret=0;
	if(globalVariable2_1== 65){
		ret=func2_66();
		return ret;
	}else{
			return 0;
	}
}
void call2_64(){
	globalVariable2_1 += 1;
}
int func2_64(){
	int ret=0;
	if(globalVariable2_1== 64){
		ret=func2_65();
		return ret;
	}else{
			return 0;
	}
}
void call2_63(){
	globalVariable2_1 += 1;
}
int func2_63(){
	int ret=0;
	if(globalVariable2_1== 63){
		ret=func2_64();
		return ret;
	}else{
			return 0;
	}
}
void call2_62(){
	globalVariable2_1 += 1;
}
int func2_62(){
	int ret=0;
	if(globalVariable2_1== 62){
		ret=func2_63();
		return ret;
	}else{
			return 0;
	}
}
void call2_61(){
	globalVariable2_1 += 1;
}
int func2_61(){
	int ret=0;
	if(globalVariable2_1== 61){
		ret=func2_62();
		return ret;
	}else{
			return 0;
	}
}
void call2_60(){
	globalVariable2_1 += 1;
}
int func2_60(){
	int ret=0;
	if(globalVariable2_1== 60){
		ret=func2_61();
		return ret;
	}else{
			return 0;
	}
}
void call2_59(){
	globalVariable2_1 += 1;
}
int func2_59(){
	int ret=0;
	if(globalVariable2_1== 59){
		ret=func2_60();
		return ret;
	}else{
			return 0;
	}
}
void call2_58(){
	globalVariable2_1 += 1;
}
int func2_58(){
	int ret=0;
	if(globalVariable2_1== 58){
		ret=func2_59();
		return ret;
	}else{
			return 0;
	}
}
void call2_57(){
	globalVariable2_1 += 1;
}
int func2_57(){
	int ret=0;
	if(globalVariable2_1== 57){
		ret=func2_58();
		return ret;
	}else{
			return 0;
	}
}
void call2_56(){
	globalVariable2_1 += 1;
}
int func2_56(){
	int ret=0;
	if(globalVariable2_1== 56){
		ret=func2_57();
		return ret;
	}else{
			return 0;
	}
}
void call2_55(){
	globalVariable2_1 += 1;
}
int func2_55(){
	int ret=0;
	if(globalVariable2_1== 55){
		ret=func2_56();
		return ret;
	}else{
			return 0;
	}
}
void call2_54(){
	globalVariable2_1 += 1;
}
int func2_54(){
	int ret=0;
	if(globalVariable2_1== 54){
		ret=func2_55();
		return ret;
	}else{
			return 0;
	}
}
void call2_53(){
	globalVariable2_1 += 1;
}
int func2_53(){
	int ret=0;
	if(globalVariable2_1== 53){
		ret=func2_54();
		return ret;
	}else{
			return 0;
	}
}
void call2_52(){
	globalVariable2_1 += 1;
}
int func2_52(){
	int ret=0;
	if(globalVariable2_1== 52){
		ret=func2_53();
		return ret;
	}else{
			return 0;
	}
}
void call2_51(){
	globalVariable2_1 += 1;
}
int func2_51(){
	int ret=0;
	if(globalVariable2_1== 51){
		ret=func2_52();
		return ret;
	}else{
			return 0;
	}
}
void call2_50(){
	globalVariable2_1 += 1;
}
int func2_50(){
	int ret=0;
	if(globalVariable2_1== 50){
		ret=func2_51();
		return ret;
	}else{
			return 0;
	}
}
void call2_49(){
	globalVariable2_1 += 1;
}
int func2_49(){
	int ret=0;
	if(globalVariable2_1== 49){
		ret=func2_50();
		return ret;
	}else{
			return 0;
	}
}
void call2_48(){
	globalVariable2_1 += 1;
}
int func2_48(){
	int ret=0;
	if(globalVariable2_1== 48){
		ret=func2_49();
		return ret;
	}else{
			return 0;
	}
}
void call2_47(){
	globalVariable2_1 += 1;
}
int func2_47(){
	int ret=0;
	if(globalVariable2_1== 47){
		ret=func2_48();
		return ret;
	}else{
			return 0;
	}
}
void call2_46(){
	globalVariable2_1 += 1;
}
int func2_46(){
	int ret=0;
	if(globalVariable2_1== 46){
		ret=func2_47();
		return ret;
	}else{
			return 0;
	}
}
void call2_45(){
	globalVariable2_1 += 1;
}
int func2_45(){
	int ret=0;
	if(globalVariable2_1== 45){
		ret=func2_46();
		return ret;
	}else{
			return 0;
	}
}
void call2_44(){
	globalVariable2_1 += 1;
}
int func2_44(){
	int ret=0;
	if(globalVariable2_1== 44){
		ret=func2_45();
		return ret;
	}else{
			return 0;
	}
}
void call2_43(){
	globalVariable2_1 += 1;
}
int func2_43(){
	int ret=0;
	if(globalVariable2_1== 43){
		ret=func2_44();
		return ret;
	}else{
			return 0;
	}
}
void call2_42(){
	globalVariable2_1 += 1;
}
int func2_42(){
	int ret=0;
	if(globalVariable2_1== 42){
		ret=func2_43();
		return ret;
	}else{
			return 0;
	}
}
void call2_41(){
	globalVariable2_1 += 1;
}
int func2_41(){
	int ret=0;
	if(globalVariable2_1== 41){
		ret=func2_42();
		return ret;
	}else{
			return 0;
	}
}
void call2_40(){
	globalVariable2_1 += 1;
}
int func2_40(){
	int ret=0;
	if(globalVariable2_1== 40){
		ret=func2_41();
		return ret;
	}else{
			return 0;
	}
}
void call2_39(){
	globalVariable2_1 += 1;
}
int func2_39(){
	int ret=0;
	if(globalVariable2_1== 39){
		ret=func2_40();
		return ret;
	}else{
			return 0;
	}
}
void call2_38(){
	globalVariable2_1 += 1;
}
int func2_38(){
	int ret=0;
	if(globalVariable2_1== 38){
		ret=func2_39();
		return ret;
	}else{
			return 0;
	}
}
void call2_37(){
	globalVariable2_1 += 1;
}
int func2_37(){
	int ret=0;
	if(globalVariable2_1== 37){
		ret=func2_38();
		return ret;
	}else{
			return 0;
	}
}
void call2_36(){
	globalVariable2_1 += 1;
}
int func2_36(){
	int ret=0;
	if(globalVariable2_1== 36){
		ret=func2_37();
		return ret;
	}else{
			return 0;
	}
}
void call2_35(){
	globalVariable2_1 += 1;
}
int func2_35(){
	int ret=0;
	if(globalVariable2_1== 35){
		ret=func2_36();
		return ret;
	}else{
			return 0;
	}
}
void call2_34(){
	globalVariable2_1 += 1;
}
int func2_34(){
	int ret=0;
	if(globalVariable2_1== 34){
		ret=func2_35();
		return ret;
	}else{
			return 0;
	}
}
void call2_33(){
	globalVariable2_1 += 1;
}
int func2_33(){
	int ret=0;
	if(globalVariable2_1== 33){
		ret=func2_34();
		return ret;
	}else{
			return 0;
	}
}
void call2_32(){
	globalVariable2_1 += 1;
}
int func2_32(){
	int ret=0;
	if(globalVariable2_1== 32){
		ret=func2_33();
		return ret;
	}else{
			return 0;
	}
}
void call2_31(){
	globalVariable2_1 += 1;
}
int func2_31(){
	int ret=0;
	if(globalVariable2_1== 31){
		ret=func2_32();
		return ret;
	}else{
			return 0;
	}
}
void call2_30(){
	globalVariable2_1 += 1;
}
int func2_30(){
	int ret=0;
	if(globalVariable2_1== 30){
		ret=func2_31();
		return ret;
	}else{
			return 0;
	}
}
void call2_29(){
	globalVariable2_1 += 1;
}
int func2_29(){
	int ret=0;
	if(globalVariable2_1== 29){
		ret=func2_30();
		return ret;
	}else{
			return 0;
	}
}
void call2_28(){
	globalVariable2_1 += 1;
}
int func2_28(){
	int ret=0;
	if(globalVariable2_1== 28){
		ret=func2_29();
		return ret;
	}else{
			return 0;
	}
}
void call2_27(){
	globalVariable2_1 += 1;
}
int func2_27(){
	int ret=0;
	if(globalVariable2_1== 27){
		ret=func2_28();
		return ret;
	}else{
			return 0;
	}
}
void call2_26(){
	globalVariable2_1 += 1;
}
int func2_26(){
	int ret=0;
	if(globalVariable2_1== 26){
		ret=func2_27();
		return ret;
	}else{
			return 0;
	}
}
void call2_25(){
	globalVariable2_1 += 1;
}
int func2_25(){
	int ret=0;
	if(globalVariable2_1== 25){
		ret=func2_26();
		return ret;
	}else{
			return 0;
	}
}
void call2_24(){
	globalVariable2_1 += 1;
}
int func2_24(){
	int ret=0;
	if(globalVariable2_1== 24){
		ret=func2_25();
		return ret;
	}else{
			return 0;
	}
}
void call2_23(){
	globalVariable2_1 += 1;
}
int func2_23(){
	int ret=0;
	if(globalVariable2_1== 23){
		ret=func2_24();
		return ret;
	}else{
			return 0;
	}
}
void call2_22(){
	globalVariable2_1 += 1;
}
int func2_22(){
	int ret=0;
	if(globalVariable2_1== 22){
		ret=func2_23();
		return ret;
	}else{
			return 0;
	}
}
void call2_21(){
	globalVariable2_1 += 1;
}
int func2_21(){
	int ret=0;
	if(globalVariable2_1== 21){
		ret=func2_22();
		return ret;
	}else{
			return 0;
	}
}
void call2_20(){
	globalVariable2_1 += 1;
}
int func2_20(){
	int ret=0;
	if(globalVariable2_1== 20){
		ret=func2_21();
		return ret;
	}else{
			return 0;
	}
}
void call2_19(){
	globalVariable2_1 += 1;
}
int func2_19(){
	int ret=0;
	if(globalVariable2_1== 19){
		ret=func2_20();
		return ret;
	}else{
			return 0;
	}
}
void call2_18(){
	globalVariable2_1 += 1;
}
int func2_18(){
	int ret=0;
	if(globalVariable2_1== 18){
		ret=func2_19();
		return ret;
	}else{
			return 0;
	}
}
void call2_17(){
	globalVariable2_1 += 1;
}
int func2_17(){
	int ret=0;
	if(globalVariable2_1== 17){
		ret=func2_18();
		return ret;
	}else{
			return 0;
	}
}
void call2_16(){
	globalVariable2_1 += 1;
}
int func2_16(){
	int ret=0;
	if(globalVariable2_1== 16){
		ret=func2_17();
		return ret;
	}else{
			return 0;
	}
}
void call2_15(){
	globalVariable2_1 += 1;
}
int func2_15(){
	int ret=0;
	if(globalVariable2_1== 15){
		ret=func2_16();
		return ret;
	}else{
			return 0;
	}
}
void call2_14(){
	globalVariable2_1 += 1;
}
int func2_14(){
	int ret=0;
	if(globalVariable2_1== 14){
		ret=func2_15();
		return ret;
	}else{
			return 0;
	}
}
void call2_13(){
	globalVariable2_1 += 1;
}
int func2_13(){
	int ret=0;
	if(globalVariable2_1== 13){
		ret=func2_14();
		return ret;
	}else{
			return 0;
	}
}
void call2_12(){
	globalVariable2_1 += 1;
}
int func2_12(){
	int ret=0;
	if(globalVariable2_1== 12){
		ret=func2_13();
		return ret;
	}else{
			return 0;
	}
}
void call2_11(){
	globalVariable2_1 += 1;
}
int func2_11(){
	int ret=0;
	if(globalVariable2_1== 11){
		ret=func2_12();
		return ret;
	}else{
			return 0;
	}
}
void call2_10(){
	globalVariable2_1 += 1;
}
int func2_10(){
	int ret=0;
	if(globalVariable2_1== 10){
		ret=func2_11();
		return ret;
	}else{
			return 0;
	}
}
void call2_9(){
	globalVariable2_1 += 1;
}
int func2_9(){
	int ret=0;
	if(globalVariable2_1== 9){
		ret=func2_10();
		return ret;
	}else{
			return 0;
	}
}
void call2_8(){
	globalVariable2_1 += 1;
}
int func2_8(){
	int ret=0;
	if(globalVariable2_1== 8){
		ret=func2_9();
		return ret;
	}else{
			return 0;
	}
}
void call2_7(){
	globalVariable2_1 += 1;
}
int func2_7(){
	int ret=0;
	if(globalVariable2_1== 7){
		ret=func2_8();
		return ret;
	}else{
			return 0;
	}
}
void call2_6(){
	globalVariable2_1 += 1;
}
int func2_6(){
	int ret=0;
	if(globalVariable2_1== 6){
		ret=func2_7();
		return ret;
	}else{
			return 0;
	}
}
void call2_5(){
	globalVariable2_1 += 1;
}
int func2_5(){
	int ret=0;
	if(globalVariable2_1== 5){
		ret=func2_6();
		return ret;
	}else{
			return 0;
	}
}
void call2_4(){
	globalVariable2_1 += 1;
}
int func2_4(){
	int ret=0;
	if(globalVariable2_1== 4){
		ret=func2_5();
		return ret;
	}else{
			return 0;
	}
}
void call2_3(){
	globalVariable2_1 += 1;
}
int func2_3(){
	int ret=0;
	if(globalVariable2_1== 3){
		ret=func2_4();
		return ret;
	}else{
			return 0;
	}
}
void call2_2(){
	globalVariable2_1 += 1;
}
int func2_2(){
	int ret=0;
	if(globalVariable2_1== 2){
		ret=func2_3();
		return ret;
	}else{
			return 0;
	}
}

void call2_1(){
	globalVariable2_1 += 1;
}

int func2_1(){
#if !defined(sparc_sun_solaris2_4) &&  !defined(rs6000_ibm_aix4_1) &&!defined(i386_unknown_linux2_0) &&!defined(rs6000_ibm_aix5_1)

    printf("Skipped test #2 (instrument many simple function calls and save the world)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[2] = TRUE;

#else
	int ret = 0;

	if(globalVariable2_1 == 1){

		ret = func2_2();
		if(ret == 1){
			printf("Passed Test #2 (instrument many simple function calls and save the world)\n");
			passedTest[2] = TRUE;
		}else{
			printf("Failed Test #2 (instrument many simple function calls and save the world)\n");
		}
		return ret;

	}else{
		printf("Failed Test #2 (instrument many simple function calls and save the world)\n");
		return 0;
	}
#endif
}


void call1_1(){

	globalVariable1_1 = 42;
}


int func1_1(){
#if !defined(sparc_sun_solaris2_4) &&  !defined(rs6000_ibm_aix4_1) &&!defined(i386_unknown_linux2_0) &&!defined(rs6000_ibm_aix5_1)

    printf("Skipped test #1 (instrument one simple function call and save the world)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[1] = TRUE;

#else

	if(globalVariable1_1 == 42) {

		printf("Passed Test #1 (instrument one simple function call and save the world)\n");
		passedTest[1] = TRUE;

		return 1;
	}else{
		printf("Failed Test #1 (instrument one simple function call and save the world)\n");
		return 0;
	}
#endif
}

/*
 * Start of Test #3
 */
void func3_1() {
#if !defined(sparc_sun_solaris2_4) &&!defined(i386_unknown_linux2_0) &&!defined(rs6000_ibm_aix5_1) && !defined(rs6000_ibm_aix4_1) 
/* 
	&&  !defined(rs6000_ibm_aix4_1) this fails on aix from the test case but the
	mutated binary works fine when it is run by hand 
*/

    printf("Skipped test #3 (instrument four parameter function call and save the world)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[3] = TRUE;

#else

 dprintf("func3_1 () called\n"); 
#endif
}

void call3_1(int arg1, int arg2, char *arg3, void *arg4)
{
    assert(TEST_PTR_SIZE == sizeof(void *));

    if ((arg1 == 1) && (arg2 == 2) && (!strcmp(arg3, "testString3_1")) &&
	(arg4 == TEST_PTR)) {
	printf("Passed test #3 (four parameter function)\n");
	passedTest[3] = TRUE;
    } else {
	printf("**Failed** test #3 (four parameter function)\n");
	if (arg1 != 1)
	    printf("    arg1 = %d, should be 1\n", arg1);
	if (arg2 != 2)
	    printf("    arg2 = %d, should be 2\n", arg2);
	if (strcmp(arg3, "testString3_1"))
	    printf("    arg3 = %s, should be \"testString3_1\"\n", arg3);
	if (arg4 != TEST_PTR)
	    printf("    arg4 = 0x%p, should be 0x%p\n", arg4, TEST_PTR);
    }
}

/*
 * Test #4 - read/write a variable in the mutatee
 */
void func4_1()
{
#if !defined(sparc_sun_solaris2_4) &&!defined(i386_unknown_linux2_0) &&  !defined(rs6000_ibm_aix4_1) 
/* 
	&&  !defined(rs6000_ibm_aix4_1) this fails on aix from the test case but the
	mutated binary works fine when it is run by hand 
*/

    printf("Skipped test #4 (read/write a variable in the mutatee and save the world)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[4] = TRUE;

#else


    if (globalVariable4_1 == 17) {
    	printf("Passed test #4 (read/write a variable in the mutatee)\n");
		passedTest[4] = TRUE;
    } else {
		printf("**Failed test #4 (read/write a variable in the mutatee)\n");
		if (globalVariable4_1 == 42)
	    	printf("    globalVariable4_1 still contains 42 (probably it was not written to)\n");
		else
		    printf("    globalVariable4_1 contained %d, not 17 as expected\n",
			    globalVariable4_1);
    }
#endif
}

void func5_1(){
#if !defined(sparc_sun_solaris2_4) &&  !defined(i386_unknown_linux2_0) &&!defined(rs6000_ibm_aix4_1)

    printf("Skipped test #5 (use loadLibrary and save the world)\n");
    printf("\t- not implemented on this platform\n");
    passedTest[5] = TRUE;

#else

#if defined(rs6000_ibm_aix4_1)
	/* 	This code checks to see if the library has been loaded. I cannot
		get AIX to produce a shared library that updates a variable exported
		by the main executable in an init (binitfini) routine.
	*/
	char buffer[4096];
	struct ld_info *ldinfo;
	int found = 0;
	loadquery(L_GETINFO, (void*)buffer, 4096);

	ldinfo = (struct ld_info*) (& (buffer[0]));

	do{
		if( ldinfo){
			if(ldinfo->ldinfo_filename && strstr(ldinfo->ldinfo_filename, "libLoadMe")){
				found = 1;
			}

			ldinfo = (struct ld_info*)(((char*)ldinfo)+ldinfo->ldinfo_next);
		}
		
	
	}while(ldinfo && !found);
	

	if(found){	
#else
	if(globalVariable5_1 == 99){
#endif
		/* init in libLoadMe.so should set globalVariable5_1 */

		printf("Passed test #5 (use loadLibrary)\n");
		passedTest[5] = TRUE;
	}else{
		printf("**Failed test #5 (use loadLibrary) %d\n",globalVariable5_1);
	}
		
#endif
}

void runTests()
{
    int j;

    for (j=0; j <= MAX_TEST; j++) {
	passedTest [j] = FALSE;
    }

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();
    if (runTest[5]) func5_1();
    if (runTest[6]) func6_1();
		
}


int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
    unsigned int i, j;
    unsigned int testsFailed = 0;
    int useAttach = FALSE;
#ifndef i386_unknown_nt4_0
    int pfd;
#endif

    for (j=0; j <= MAX_TEST; j++) {
	runTest [j] = FALSE;
    }

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
	    if (++i >= argc) {
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	    }
	    pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-runall")) {
            dprintf("selecting all tests\n");
            for (j=1; j <= MAX_TEST; j++) runTest[j] = TRUE;
        } else if (!strcmp(argv[i], "-run")) {
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        dprintf("selecting test %d\n", testId);
                        runTest[testId] = TRUE;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    /* end of test list */
		    break;
                }
            }
            i=j-1;
		} else {
            fprintf(stderr, "%s\n", USAGE);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0],
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

    /* actually invoke the tests.  This function is implemented in two
     *   different locations (one for C and one for Fortran). Look in
     *     test1.mutatee.c and test1.mutateeFortC.c for the code.
     */
    runTests();

    /* See how we did running the tests. */
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) {
	    testsFailed++;
	}
    }


    fflush(stdout);
    dprintf("Mutatee %s terminating.\n", argv[0]);
    exit(testsFailed ? 127 :1); /* 1 is success! 127 is failure*/ 
}
