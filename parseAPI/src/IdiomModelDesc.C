#if defined(cap_stripped_binaries)


#include "ProbabilisticParser.h"
#include "util.h"

#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

#include <map>
#include <string>

using namespace std;
using namespace hd;
using namespace Dyninst;

IdiomModel::IdiomModel(string model_spec) {
#if defined(arch_x86) || defined(arch_x86_64) || defined(i386_unknown_nt4_0)
  #if defined(os_windows)
    if (!strcmp(model_spec.c_str(), "32-bit")) {
        bias = -10.374549;
	prob_threshold = 0.16076288925;

	Idiom i1;
	i1.w = 14.706479;
	i1.prefix = false;
	i1.terms.push_back(IdiomTerm(e_mov, x86::edi, x86::edi));
	i1.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	i1.terms.push_back(IdiomTerm(e_mov, x86::ebp, x86::esp));
	normal.addIdiom(i1);

	Idiom i2;
	i2.w = 12.585138;
	i2.prefix = false;
	i2.terms.push_back(IdiomTerm(e_mov, x86::edi, x86::edi));
	i2.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	normal.addIdiom(i2);

	Idiom i3;
	i3.w = 11.724242;
	i3.prefix = true;
	i3.terms.push_back(IdiomTerm(e_call, x86::eip, NOARG));
	i3.terms.push_back(IdiomTerm(e_mov, x86::eax, MEMARG));
	i3.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i3.terms.begin(), i3.terms.end());
	prefix.addIdiom(i3);

	Idiom i4;
	i4.w = 10.484159;
	i4.prefix = false;
	i4.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	i4.terms.push_back(IdiomTerm(e_mov, x86::ebp, x86::esp));
	normal.addIdiom(i4);

	Idiom i5;
	i5.w = 7.816502;
	i5.prefix = true;
	i5.terms.push_back(IdiomTerm(e_No_Entry, NOARG, NOARG));
	i5.terms.push_back(IdiomTerm(e_dec, x86::ecx, NOARG));
	i5.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i5.terms.begin(), i5.terms.end());
	prefix.addIdiom(i5);

	Idiom i6;
	i6.w = 7.656885;
	i6.prefix = true;
	i6.terms.push_back(IdiomTerm(e_add, MEMARG, x86::al));
	i6.terms.push_back(IdiomTerm(e_add, MEMARG, x86::al));
	i6.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i6.terms.begin(), i6.terms.end());
	prefix.addIdiom(i6);

	Idiom i7;
	i7.w = 7.568732;
	i7.prefix = false;
	i7.terms.push_back(IdiomTerm(e_cmp, MEMARG, IMMARG));
	i7.terms.push_back(IdiomTerm(e_je, x86::eip, NOARG));
	i7.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	normal.addIdiom(i7);

	Idiom i8;
	i8.w = 7.363702;
	i8.prefix = true;
	i8.terms.push_back(IdiomTerm(e_mov, x86::eax, x86::ecx));
	i8.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i8.terms.begin(), i8.terms.end());
	prefix.addIdiom(i8);

	Idiom i9;
	i9.w = 7.182773;
	i9.prefix = true;
	i9.terms.push_back(IdiomTerm(e_int3, NOARG, NOARG));
	reverse(i9.terms.begin(), i9.terms.end());
	prefix.addIdiom(i9);

	Idiom i10;
	i10.w = 6.983300;
	i10.prefix = false;
	i10.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	normal.addIdiom(i10);

	Idiom i11;
	i11.w = 6.922450;
	i11.prefix = false;
	i11.terms.push_back(IdiomTerm(e_mov, x86::eax, IMMARG));
	i11.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	normal.addIdiom(i11);

	Idiom i12;
	i12.w = 6.672104;
	i12.prefix = false;
	i12.terms.push_back(IdiomTerm(e_push, IMMARG, x86::esp));
	i12.terms.push_back(IdiomTerm(e_push, IMMARG, x86::esp));
	normal.addIdiom(i12);

	Idiom i13;
	i13.w = 6.241539;
	i13.prefix = true;
	i13.terms.push_back(IdiomTerm(e_add, MEMARG, x86::al));
	i13.terms.push_back(IdiomTerm(e_pop, x86::ecx, x86::esp));
	i13.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i13.terms.begin(), i13.terms.end());
	prefix.addIdiom(i13);

	Idiom i14;
	i14.w = 5.771418;
	i14.prefix = true;
	i14.terms.push_back(IdiomTerm(e_add, x86::esp, IMMARG));
	i14.terms.push_back(IdiomTerm(e_pop, x86::ebp, x86::esp));
	i14.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i14.terms.begin(), i14.terms.end());
	prefix.addIdiom(i14);

	Idiom i15;
	i15.w = 5.469180;
	i15.prefix = false;
	i15.terms.push_back(IdiomTerm(e_push, IMMARG, x86::esp));
	i15.terms.push_back(IdiomTerm(e_call, x86::eip, NOARG));
	normal.addIdiom(i15);

	Idiom i16;
	i16.w = 5.169010;
	i16.prefix = false;
	i16.terms.push_back(IdiomTerm(e_xor, x86::eax, x86::eax));
	i16.terms.push_back(WILDCARD_TERM);
	i16.terms.push_back(IdiomTerm(e_int3, NOARG, NOARG));
	normal.addIdiom(i16);

	Idiom i17;
	i17.w = 4.906925;
	i17.prefix = false;
	i17.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	normal.addIdiom(i17);

	Idiom i18;
	i18.w = 4.864221;
	i18.prefix = false;
	i18.terms.push_back(IdiomTerm(e_push, x86::ebx, x86::esp));
	normal.addIdiom(i18);

	Idiom i19;
	i19.w = 4.848379;
	i19.prefix = false;
	i19.terms.push_back(IdiomTerm(e_push, x86::ecx, x86::esp));
	i19.terms.push_back(IdiomTerm(e_push, x86::ebx, x86::esp));
	i19.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	normal.addIdiom(i19);

	Idiom i20;
	i20.w = 4.696929;
	i20.prefix = false;
	i20.terms.push_back(IdiomTerm(e_push, x86::ecx, x86::esp));
	normal.addIdiom(i20);

	Idiom i21;
	i21.w = 4.431254;
	i21.prefix = false;
	i21.terms.push_back(IdiomTerm(e_mov, x86::eax, MEMARG));
	normal.addIdiom(i21);

	Idiom i22;
	i22.w = 4.132493;
	i22.prefix = false;
	i22.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i22.terms.push_back(WILDCARD_TERM);
	i22.terms.push_back(IdiomTerm(e_int3, NOARG, NOARG));
	normal.addIdiom(i22);

	Idiom i23;
	i23.w = 3.996616;
	i23.prefix = false;
	i23.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	normal.addIdiom(i23);

	Idiom i24;
	i24.w = 3.967046;
	i24.prefix = false;
	i24.terms.push_back(IdiomTerm(e_mov, x86::ecx, MEMARG));
	normal.addIdiom(i24);

	Idiom i25;
	i25.w = 3.881104;
	i25.prefix = false;
	i25.terms.push_back(IdiomTerm(e_cmp, MEMARG, IMMARG));
	normal.addIdiom(i25);

	Idiom i26;
	i26.w = 3.739653;
	i26.prefix = false;
	i26.terms.push_back(IdiomTerm(e_push, x86::edi, x86::esp));
	normal.addIdiom(i26);

	Idiom i27;
	i27.w = 3.692273;
	i27.prefix = false;
	i27.terms.push_back(IdiomTerm(e_mov, x86::cl, MEMARG));
	normal.addIdiom(i27);

	Idiom i28;
	i28.w = 3.686196;
	i28.prefix = false;
	i28.terms.push_back(IdiomTerm(e_mov, x86::edx, MEMARG));
	normal.addIdiom(i28);

	Idiom i29;
	i29.w = 3.137101;
	i29.prefix = false;
	i29.terms.push_back(WILDCARD_TERM);
	i29.terms.push_back(IdiomTerm(e_cmp, MEMARG, x86::eax));
	normal.addIdiom(i29);

	Idiom i30;
	i30.w = 3.123769;
	i30.prefix = false;
	i30.terms.push_back(WILDCARD_TERM);
	i30.terms.push_back(WILDCARD_TERM);
	i30.terms.push_back(IdiomTerm(e_ja, x86::eip, NOARG));
	normal.addIdiom(i30);

	Idiom i31;
	i31.w = 2.796196;
	i31.prefix = false;
	i31.terms.push_back(IdiomTerm(e_cmp, x86::eax, IMMARG));
	normal.addIdiom(i31);

	Idiom i32;
	i32.w = 2.792813;
	i32.prefix = true;
	i32.terms.push_back(IdiomTerm(e_No_Entry, NOARG, NOARG));
	i32.terms.push_back(IdiomTerm(e_call, MEMARG, NOARG));
	reverse(i32.terms.begin(), i32.terms.end());
	prefix.addIdiom(i32);

	Idiom i33;
	i33.w = 1.549826;
	i33.prefix = false;
	i33.terms.push_back(WILDCARD_TERM);
	i33.terms.push_back(IdiomTerm(e_je, x86::eip, NOARG));
	normal.addIdiom(i33);

	Idiom i34;
	i34.w = 1.528525;
	i34.prefix = true;
	i34.terms.push_back(IdiomTerm(e_add, x86::esp, IMMARG));
	i34.terms.push_back(IdiomTerm(e_pop, x86::esi, x86::esp));
	i34.terms.push_back(WILDCARD_TERM);
	reverse(i34.terms.begin(), i34.terms.end());
	prefix.addIdiom(i34);

	Idiom i35;
	i35.w = 0.618346;
	i35.prefix = false;
	i35.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	i35.terms.push_back(WILDCARD_TERM);
	i35.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	normal.addIdiom(i35);

	Idiom i36;
	i36.w = -1.557146;
	i36.prefix = false;
	i36.terms.push_back(WILDCARD_TERM);
	i36.terms.push_back(WILDCARD_TERM);
	i36.terms.push_back(IdiomTerm(e_add, x86::esp, IMMARG));
	normal.addIdiom(i36);

	Idiom i37;
	i37.w = -3.533071;
	i37.prefix = true;
	i37.terms.push_back(IdiomTerm(e_jmp, x86::eip, NOARG));
	i37.terms.push_back(WILDCARD_TERM);
	i37.terms.push_back(IdiomTerm(e_jne, x86::eip, NOARG));
	reverse(i37.terms.begin(), i37.terms.end());
	prefix.addIdiom(i37);

	Idiom i38;
	i38.w = -3.700331;
	i38.prefix = true;
	i38.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	i38.terms.push_back(WILDCARD_TERM);
	reverse(i38.terms.begin(), i38.terms.end());
	prefix.addIdiom(i38);

	Idiom i39;
	i39.w = -3.984643;
	i39.prefix = true;
	i39.terms.push_back(IdiomTerm(e_dec, x86::ebp, NOARG));
	i39.terms.push_back(WILDCARD_TERM);
	reverse(i39.terms.begin(), i39.terms.end());
	prefix.addIdiom(i39);

	Idiom i40;
	i40.w = -4.757496;
	i40.prefix = true;
	i40.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::eax));
	reverse(i40.terms.begin(), i40.terms.end());
	prefix.addIdiom(i40);

	Idiom i41;
	i41.w = -5.819621;
	i41.prefix = true;
	i41.terms.push_back(IdiomTerm(e_add, MEMARG, x86::esi));
	i41.terms.push_back(WILDCARD_TERM);
	reverse(i41.terms.begin(), i41.terms.end());
	prefix.addIdiom(i41);

	Idiom i42;
	i42.w = -8.793439;
	i42.prefix = false;
	i42.terms.push_back(IdiomTerm(e_int3, NOARG, NOARG));
	i42.terms.push_back(IdiomTerm(e_int3, NOARG, NOARG));
	i42.terms.push_back(IdiomTerm(e_int3, NOARG, NOARG));
	normal.addIdiom(i42);

	Idiom i43;
	i43.w = -10.645611;
	i43.prefix = true;
	i43.terms.push_back(IdiomTerm(e_mov, x86::edi, x86::edi));
	reverse(i43.terms.begin(), i43.terms.end());
	prefix.addIdiom(i43);
    }	

  #else
    if (!strcmp(model_spec.c_str(), "32-bit")) {
        bias = -11.803098;
	prob_threshold = 0.56577812835;

	Idiom i1;
	i1.w = 17.263666;
	i1.prefix = true;
	i1.terms.push_back(IdiomTerm(e_call, x86::eip, NOARG));
	i1.terms.push_back(WILDCARD_TERM);
	i1.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	reverse(i1.terms.begin(), i1.terms.end());
	prefix.addIdiom(i1);

	Idiom i2;
	i2.w = 13.156334;
	i2.prefix = true;
	i2.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i2.terms.push_back(WILDCARD_TERM);
	i2.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	reverse(i2.terms.begin(), i2.terms.end());
	prefix.addIdiom(i2);

	Idiom i3;
	i3.w = 11.741889;
	i3.prefix = true;
	i3.terms.push_back(IdiomTerm(e_jmp, x86::eip, NOARG));
	i3.terms.push_back(WILDCARD_TERM);
	i3.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	reverse(i3.terms.begin(), i3.terms.end());
	prefix.addIdiom(i3);

	Idiom i4;
	i4.w = 11.111479;
	i4.prefix = false;
	i4.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	i4.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	normal.addIdiom(i4);

	Idiom i5;
	i5.w = 10.179057;
	i5.prefix = false;
	i5.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	i5.terms.push_back(IdiomTerm(e_mov, x86::ebp, x86::esp));
	normal.addIdiom(i5);

	Idiom i6;
	i6.w = 9.835183;
	i6.prefix = false;
	i6.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	i6.terms.push_back(IdiomTerm(e_push, x86::ebx, x86::esp));
	i6.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	normal.addIdiom(i6);

	Idiom i7;
	i7.w = 9.399355;
	i7.prefix = false;
	i7.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	i7.terms.push_back(IdiomTerm(e_push, x86::edi, x86::esp));
	normal.addIdiom(i7);

	Idiom i8;
	i8.w = 8.812445;
	i8.prefix = false;
	i8.terms.push_back(IdiomTerm(e_mov, x86::eax, IMMARG));
	i8.terms.push_back(WILDCARD_TERM);
	i8.terms.push_back(IdiomTerm(e_sar, x86::eax, IMMARG));
	normal.addIdiom(i8);

	Idiom i9;
	i9.w = 8.616524;
	i9.prefix = false;
	i9.terms.push_back(IdiomTerm(e_mov, x86::ebx, MEMARG));
	i9.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	normal.addIdiom(i9);

	Idiom i10;
	i10.w = 8.410117;
	i10.prefix = false;
	i10.terms.push_back(IdiomTerm(e_cmp, MEMARG, IMMARG));
	i10.terms.push_back(WILDCARD_TERM);
	i10.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	normal.addIdiom(i10);

	Idiom i11;
	i11.w = 8.361151;
	i11.prefix = true;
	i11.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i11.terms.begin(), i11.terms.end());
	prefix.addIdiom(i11);

	Idiom i12;
	i12.w = 8.255001;
	i12.prefix = true;
	i12.terms.push_back(IdiomTerm(e_add, x86::ecx, x86::ecx));
	i12.terms.push_back(WILDCARD_TERM);
	i12.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i12.terms.begin(), i12.terms.end());
	prefix.addIdiom(i12);

	Idiom i13;
	i13.w = 7.963622;
	i13.prefix = true;
	i13.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i13.terms.begin(), i13.terms.end());
	prefix.addIdiom(i13);

	Idiom i14;
	i14.w = 7.718187;
	i14.prefix = true;
	i14.terms.push_back(IdiomTerm(e_lea, x86::esi, x86::esi));
	reverse(i14.terms.begin(), i14.terms.end());
	prefix.addIdiom(i14);

	Idiom i15;
	i15.w = 7.199909;
	i15.prefix = false;
	i15.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	normal.addIdiom(i15);

	Idiom i16;
	i16.w = 7.155092;
	i16.prefix = true;
	i16.terms.push_back(IdiomTerm(e_inc, x86::esp, NOARG));
	i16.terms.push_back(IdiomTerm(e_and, x86::al, IMMARG));
	i16.terms.push_back(IdiomTerm(e_mov, x86::edx, MEMARG));
	reverse(i16.terms.begin(), i16.terms.end());
	prefix.addIdiom(i16);

	Idiom i17;
	i17.w = 7.129911;
	i17.prefix = false;
	i17.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	i17.terms.push_back(WILDCARD_TERM);
	i17.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	normal.addIdiom(i17);

	Idiom i18;
	i18.w = 6.504371;
	i18.prefix = true;
	i18.terms.push_back(IdiomTerm(e_mov, x86::eax, MEMARG));
	i18.terms.push_back(WILDCARD_TERM);
	i18.terms.push_back(IdiomTerm(e_mov, x86::ecx, MEMARG));
	reverse(i18.terms.begin(), i18.terms.end());
	prefix.addIdiom(i18);

	Idiom i19;
	i19.w = 6.003193;
	i19.prefix = false;
	i19.terms.push_back(IdiomTerm(e_push, x86::ebx, x86::esp));
	normal.addIdiom(i19);

	Idiom i20;
	i20.w = 5.968092;
	i20.prefix = false;
	i20.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	i20.terms.push_back(IdiomTerm(e_push, x86::edi, x86::esp));
	i20.terms.push_back(IdiomTerm(e_push, x86::esi, x86::esp));
	normal.addIdiom(i20);

	Idiom i21;
	i21.w = 5.564052;
	i21.prefix = false;
	i21.terms.push_back(IdiomTerm(e_push, x86::edi, x86::esp));
	normal.addIdiom(i21);

	Idiom i22;
	i22.w = 5.319274;
	i22.prefix = true;
	i22.terms.push_back(IdiomTerm(e_jmp, x86::eip, NOARG));
	reverse(i22.terms.begin(), i22.terms.end());
	prefix.addIdiom(i22);

	Idiom i23;
	i23.w = 5.225387;
	i23.prefix = false;
	i23.terms.push_back(IdiomTerm(e_mov, x86::edx, MEMARG));
	i23.terms.push_back(IdiomTerm(e_xor, x86::eax, x86::eax));
	normal.addIdiom(i23);

	Idiom i24;
	i24.w = 5.187184;
	i24.prefix = false;
	i24.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	normal.addIdiom(i24);

	Idiom i25;
	i25.w = 4.563738;
	i25.prefix = true;
	i25.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i25.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i25.terms.begin(), i25.terms.end());
	prefix.addIdiom(i25);

	Idiom i26;
	i26.w = 4.370586;
	i26.prefix = false;
	i26.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	i26.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::ebx));
	normal.addIdiom(i26);

	Idiom i27;
	i27.w = 4.122676;
	i27.prefix = true;
	i27.terms.push_back(IdiomTerm(e_pop, x86::esi, x86::esp));
	i27.terms.push_back(WILDCARD_TERM);
	i27.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i27.terms.begin(), i27.terms.end());
	prefix.addIdiom(i27);

	Idiom i28;
	i28.w = 3.877669;
	i28.prefix = false;
	i28.terms.push_back(WILDCARD_TERM);
	i28.terms.push_back(IdiomTerm(e_mov, x86::eax, MEMARG));
	i28.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	normal.addIdiom(i28);

	Idiom i29;
	i29.w = 3.705916;
	i29.prefix = true;
	i29.terms.push_back(IdiomTerm(e_pop, x86::ax, x86::esp));
	i29.terms.push_back(IdiomTerm(e_inc, x86::esp, NOARG));
	i29.terms.push_back(WILDCARD_TERM);
	reverse(i29.terms.begin(), i29.terms.end());
	prefix.addIdiom(i29);

	Idiom i30;
	i30.w = 2.818358;
	i30.prefix = true;
	i30.terms.push_back(IdiomTerm(e_add, x86::bl, x86::al));
	i30.terms.push_back(WILDCARD_TERM);
	i30.terms.push_back(WILDCARD_TERM);
	reverse(i30.terms.begin(), i30.terms.end());
	prefix.addIdiom(i30);

	Idiom i31;
	i31.w = 2.724302;
	i31.prefix = true;
	i31.terms.push_back(IdiomTerm(e_or, x86::bl, x86::al));
	i31.terms.push_back(WILDCARD_TERM);
	reverse(i31.terms.begin(), i31.terms.end());
	prefix.addIdiom(i31);

	Idiom i32;
	i32.w = 2.075361;
	i32.prefix = true;
	i32.terms.push_back(IdiomTerm(e_jmp, x86::eip, NOARG));
	i32.terms.push_back(IdiomTerm(e_lea, x86::esi, x86::esi));
	i32.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	reverse(i32.terms.begin(), i32.terms.end());
	prefix.addIdiom(i32);

	Idiom i33;
	i33.w = 1.855169;
	i33.prefix = false;
	i33.terms.push_back(WILDCARD_TERM);
	i33.terms.push_back(WILDCARD_TERM);
	i33.terms.push_back(IdiomTerm(e_push, x86::ebx, x86::esp));
	normal.addIdiom(i33);

	Idiom i34;
	i34.w = 1.539709;
	i34.prefix = true;
	i34.terms.push_back(IdiomTerm(e_pop, x86::ax, x86::esp));
	i34.terms.push_back(IdiomTerm(e_add, MEMARG, IMMARG));
	i34.terms.push_back(IdiomTerm(e_add, MEMARG, x86::al));
	reverse(i34.terms.begin(), i34.terms.end());
	prefix.addIdiom(i34);

	Idiom i35;
	i35.w = 1.126969;
	i35.prefix = false;
	i35.terms.push_back(IdiomTerm(e_sub, x86::esp, IMMARG));
	i35.terms.push_back(IdiomTerm(e_mov, x86::eax, MEMARG));
	i35.terms.push_back(IdiomTerm(e_mov, MEMARG, IMMARG));
	normal.addIdiom(i35);

	Idiom i36;
	i36.w = -3.429759;
	i36.prefix = true;
	i36.terms.push_back(IdiomTerm(e_rcr, MEMARG, IMMARG));
	i36.terms.push_back(WILDCARD_TERM);
	i36.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	reverse(i36.terms.begin(), i36.terms.end());
	prefix.addIdiom(i36);

	Idiom i37;
	i37.w = -4.326854;
	i37.prefix = false;
	i37.terms.push_back(WILDCARD_TERM);
	i37.terms.push_back(IdiomTerm(e_mov, x86::edi, x86::edx));
	normal.addIdiom(i37);

	Idiom i38;
	i38.w = -4.522070;
	i38.prefix = true;
	i38.terms.push_back(IdiomTerm(e_add, x86::esp, IMMARG));
	reverse(i38.terms.begin(), i38.terms.end());
	prefix.addIdiom(i38);

	Idiom i39;
	i39.w = -5.020421;
	i39.prefix = false;
	i39.terms.push_back(WILDCARD_TERM);
	i39.terms.push_back(IdiomTerm(e_push, x86::cx, x86::esp));
	normal.addIdiom(i39);

	Idiom i40;
	i40.w = -5.390782;
	i40.prefix = true;
	i40.terms.push_back(IdiomTerm(e_push, x86::ebp, x86::esp));
	i40.terms.push_back(WILDCARD_TERM);
	reverse(i40.terms.begin(), i40.terms.end());
	prefix.addIdiom(i40);

	Idiom i41;
	i41.w = -5.452389;
	i41.prefix = false;
	i41.terms.push_back(WILDCARD_TERM);
	i41.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	normal.addIdiom(i41);

	Idiom i42;
	i42.w = -5.622921;
	i42.prefix = false;
	i42.terms.push_back(WILDCARD_TERM);
	i42.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::ecx));
	normal.addIdiom(i42);

	Idiom i43;
	i43.w = -5.628140;
	i43.prefix = false;
	i43.terms.push_back(WILDCARD_TERM);
	i43.terms.push_back(WILDCARD_TERM);
	i43.terms.push_back(IdiomTerm(e_test, x86::edx, IMMARG));
	normal.addIdiom(i43);

	Idiom i44;
	i44.w = -5.675864;
	i44.prefix = true;
	i44.terms.push_back(IdiomTerm(e_and, x86::al, IMMARG));
	i44.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i44.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i44.terms.begin(), i44.terms.end());
	prefix.addIdiom(i44);

	Idiom i45;
	i45.w = -5.755232;
	i45.prefix = true;
	i45.terms.push_back(IdiomTerm(e_mov, x86::eax, x86::ebx));
	reverse(i45.terms.begin(), i45.terms.end());
	prefix.addIdiom(i45);

	Idiom i46;
	i46.w = -5.818181;
	i46.prefix = false;
	i46.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::eax));
	i46.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::eax));
	i46.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::eax));
	normal.addIdiom(i46);

	Idiom i47;
	i47.w = -6.009922;
	i47.prefix = false;
	i47.terms.push_back(IdiomTerm(e_mov, x86::ecx, MEMARG));
	i47.terms.push_back(WILDCARD_TERM);
	i47.terms.push_back(IdiomTerm(e_mov, MEMARG, x86::ecx));
	normal.addIdiom(i47);

	Idiom i48;
	i48.w = -6.046115;
	i48.prefix = true;
	i48.terms.push_back(IdiomTerm(e_dec, MEMARG, NOARG));
	i48.terms.push_back(WILDCARD_TERM);
	i48.terms.push_back(IdiomTerm(e_lea, x86::edi, x86::edi));
	reverse(i48.terms.begin(), i48.terms.end());
	prefix.addIdiom(i48);

	Idiom i49;
	i49.w = -6.180730;
	i49.prefix = false;
	i49.terms.push_back(IdiomTerm(e_sub, x86::ecx, IMMARG));
	normal.addIdiom(i49);

	Idiom i50;
	i50.w = -6.540841;
	i50.prefix = false;
	i50.terms.push_back(IdiomTerm(e_add, MEMARG, x86::al));
	normal.addIdiom(i50);

	Idiom i51;
	i51.w = -6.860705;
	i51.prefix = false;
	i51.terms.push_back(WILDCARD_TERM);
	i51.terms.push_back(WILDCARD_TERM);
	i51.terms.push_back(IdiomTerm(e_movq, x86::xmm0, MEMARG));
	normal.addIdiom(i51);

	Idiom i52;
	i52.w = -7.232735;
	i52.prefix = false;
	i52.terms.push_back(WILDCARD_TERM);
	i52.terms.push_back(IdiomTerm(e_movdqa, MEMARG, x86::xmm0));
	normal.addIdiom(i52);

	Idiom i53;
	i53.w = -8.169587;
	i53.prefix = true;
	i53.terms.push_back(IdiomTerm(e_add, MEMARG, x86::al));
	i53.terms.push_back(IdiomTerm(e_cmp, MEMARG, IMMARG));
	i53.terms.push_back(WILDCARD_TERM);
	reverse(i53.terms.begin(), i53.terms.end());
	prefix.addIdiom(i53);

	Idiom i54;
	i54.w = -8.490673;
	i54.prefix = true;
	i54.terms.push_back(IdiomTerm(e_mov, x86::ebx, x86::eax));
	reverse(i54.terms.begin(), i54.terms.end());
	prefix.addIdiom(i54);

	Idiom i55;
	i55.w = -8.526088;
	i55.prefix = true;
	i55.terms.push_back(IdiomTerm(e_je, x86::eip, NOARG));
	reverse(i55.terms.begin(), i55.terms.end());
	prefix.addIdiom(i55);

	Idiom i56;
	i56.w = -11.258578;
	i56.prefix = false;
	i56.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	normal.addIdiom(i56);
    }

    if (!strcmp(model_spec.c_str(), "64-bit")) {
        bias = -7.400460;
	prob_threshold = 0.5604427516;

	Idiom i1;
	i1.w = 14.852382;
	i1.prefix = false;
	i1.terms.push_back(IdiomTerm(e_push, x86_64::rbp, x86_64::rsp));
	i1.terms.push_back(IdiomTerm(e_mov, x86_64::rbp, x86_64::rsp));
	normal.addIdiom(i1);

	Idiom i2;
	i2.w = 13.101528;
	i2.prefix = true;
	i2.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i2.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i2.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i2.terms.begin(), i2.terms.end());
	prefix.addIdiom(i2);

	Idiom i3;
	i3.w = 12.550708;
	i3.prefix = true;
	i3.terms.push_back(IdiomTerm(e_pop, x86_64::rcx, x86_64::rsp));
	i3.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i3.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i3.terms.begin(), i3.terms.end());
	prefix.addIdiom(i3);

	Idiom i4;
	i4.w = 12.182198;
	i4.prefix = true;
	i4.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i4.terms.push_back(IdiomTerm(e_call, x86_64::rip, NOARG));
	i4.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i4.terms.begin(), i4.terms.end());
	prefix.addIdiom(i4);

	Idiom i5;
	i5.w = 11.952772;
	i5.prefix = false;
	i5.terms.push_back(IdiomTerm(e_push, x86_64::r15, x86_64::rsp));
	i5.terms.push_back(WILDCARD_TERM);
	i5.terms.push_back(IdiomTerm(e_push, x86_64::r14, x86_64::rsp));
	normal.addIdiom(i5);

	Idiom i6;
	i6.w = 11.948603;
	i6.prefix = false;
	i6.terms.push_back(IdiomTerm(e_push, x86_64::r12, x86_64::rsp));
	i6.terms.push_back(IdiomTerm(e_push, x86_64::r13, x86_64::rsp));
	normal.addIdiom(i6);

	Idiom i7;
	i7.w = 11.900729;
	i7.prefix = false;
	i7.terms.push_back(IdiomTerm(e_mov, x86_64::rax, IMMARG));
	i7.terms.push_back(IdiomTerm(e_push, x86_64::rbp, x86_64::rsp));
	i7.terms.push_back(IdiomTerm(e_sub, x86_64::rax, IMMARG));
	normal.addIdiom(i7);

	Idiom i8;
	i8.w = 11.854945;
	i8.prefix = false;
	i8.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::rbp));
	i8.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::r12));
	i8.terms.push_back(IdiomTerm(e_lea, x86_64::rbp, x86_64::rip));
	normal.addIdiom(i8);

	Idiom i9;
	i9.w = 11.854454;
	i9.prefix = false;
	i9.terms.push_back(IdiomTerm(e_push, x86_64::r13, x86_64::rsp));
	i9.terms.push_back(WILDCARD_TERM);
	i9.terms.push_back(IdiomTerm(e_push, x86_64::rsi, x86_64::rsp));
	normal.addIdiom(i9);

	Idiom i10;
	i10.w = 11.854041;
	i10.prefix = false;
	i10.terms.push_back(IdiomTerm(e_cmp, MEMARG, IMMARG));
	i10.terms.push_back(IdiomTerm(e_jne, x86_64::rip, NOARG));
	i10.terms.push_back(IdiomTerm(e_push, x86_64::rbp, x86_64::rsp));
	normal.addIdiom(i10);

	Idiom i11;
	i11.w = 11.814376;
	i11.prefix = false;
	i11.terms.push_back(IdiomTerm(e_push, x86_64::r12, x86_64::rsp));
	i11.terms.push_back(IdiomTerm(e_push, x86_64::rbx, x86_64::rsp));
	i11.terms.push_back(IdiomTerm(e_push, x86_64::rbp, x86_64::rsp));
	normal.addIdiom(i11);

	Idiom i12;
	i12.w = 11.747066;
	i12.prefix = true;
	i12.terms.push_back(IdiomTerm(e_add, x86_64::bl, x86_64::al));
	i12.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i12.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i12.terms.begin(), i12.terms.end());
	prefix.addIdiom(i12);

	Idiom i13;
	i13.w = 11.521780;
	i13.prefix = false;
	i13.terms.push_back(IdiomTerm(e_push, x86_64::r12, x86_64::rsp));
	i13.terms.push_back(WILDCARD_TERM);
	i13.terms.push_back(IdiomTerm(e_push, x86_64::rsi, x86_64::rsp));
	normal.addIdiom(i13);

	Idiom i14;
	i14.w = 11.481597;
	i14.prefix = false;
	i14.terms.push_back(IdiomTerm(e_xor, x86_64::ebp, x86_64::ebp));
	i14.terms.push_back(WILDCARD_TERM);
	i14.terms.push_back(IdiomTerm(e_pop, x86_64::rsi, x86_64::rsp));
	normal.addIdiom(i14);

	Idiom i15;
	i15.w = 11.247187;
	i15.prefix = false;
	i15.terms.push_back(IdiomTerm(e_push, x86_64::r12, x86_64::rsp));
	i15.terms.push_back(IdiomTerm(e_push, x86_64::r14, x86_64::rsp));
	normal.addIdiom(i15);

	Idiom i16;
	i16.w = 11.177761;
	i16.prefix = true;
	i16.terms.push_back(IdiomTerm(e_setbe, x86_64::al, NOARG));
	i16.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i16.terms.begin(), i16.terms.end());
	prefix.addIdiom(i16);

	Idiom i17;
	i17.w = 11.137729;
	i17.prefix = false;
	i17.terms.push_back(IdiomTerm(e_sub, x86_64::rsp, IMMARG));
	i17.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::r9));
	normal.addIdiom(i17);

	Idiom i18;
	i18.w = 11.016895;
	i18.prefix = true;
	i18.terms.push_back(IdiomTerm(e_mov, x86_64::rsi, x86_64::rdi));
	i18.terms.push_back(WILDCARD_TERM);
	i18.terms.push_back(IdiomTerm(e_jmp, x86_64::rip, NOARG));
	reverse(i18.terms.begin(), i18.terms.end());
	prefix.addIdiom(i18);

	Idiom i19;
	i19.w = 10.908163;
	i19.prefix = false;
	i19.terms.push_back(IdiomTerm(e_push, x86_64::r15, x86_64::rsp));
	i19.terms.push_back(IdiomTerm(e_push, x86_64::r14, x86_64::rsp));
	normal.addIdiom(i19);

	Idiom i20;
	i20.w = 10.462039;
	i20.prefix = true;
	i20.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::esi));
	i20.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i20.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i20.terms.begin(), i20.terms.end());
	prefix.addIdiom(i20);

	Idiom i21;
	i21.w = 10.194427;
	i21.prefix = true;
	i21.terms.push_back(IdiomTerm(e_jmp, x86_64::rip, NOARG));
	i21.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i21.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i21.terms.begin(), i21.terms.end());
	prefix.addIdiom(i21);

	Idiom i22;
	i22.w = 10.156846;
	i22.prefix = true;
	i22.terms.push_back(IdiomTerm(e_mov, MEMARG, IMMARG));
	i22.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i22.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i22.terms.begin(), i22.terms.end());
	prefix.addIdiom(i22);

	Idiom i23;
	i23.w = 9.803962;
	i23.prefix = false;
	i23.terms.push_back(IdiomTerm(e_sub, x86_64::rsp, IMMARG));
	i23.terms.push_back(IdiomTerm(e_xor, x86_64::r9d, x86_64::r9d));
	normal.addIdiom(i23);

	Idiom i24;
	i24.w = 9.749547;
	i24.prefix = false;
	i24.terms.push_back(IdiomTerm(e_push, x86_64::rbx, x86_64::rsp));
	i24.terms.push_back(IdiomTerm(e_mov, x86_64::rbx, x86_64::rdi));
	i24.terms.push_back(IdiomTerm(e_call, x86_64::rip, NOARG));
	normal.addIdiom(i24);

	Idiom i25;
	i25.w = 9.428662;
	i25.prefix = false;
	i25.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::rbx));
	i25.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::rbp));
	normal.addIdiom(i25);

	Idiom i26;
	i26.w = 9.409167;
	i26.prefix = true;
	i26.terms.push_back(IdiomTerm(e_shr, x86_64::eax, IMMARG));
	i26.terms.push_back(WILDCARD_TERM);
	i26.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i26.terms.begin(), i26.terms.end());
	prefix.addIdiom(i26);

	Idiom i27;
	i27.w = 9.368111;
	i27.prefix = true;
	i27.terms.push_back(IdiomTerm(e_add, MEMARG, x86_64::ebx));
	i27.terms.push_back(WILDCARD_TERM);
	i27.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i27.terms.begin(), i27.terms.end());
	prefix.addIdiom(i27);

	Idiom i28;
	i28.w = 9.352001;
	i28.prefix = true;
	i28.terms.push_back(IdiomTerm(e_dec, MEMARG, NOARG));
	i28.terms.push_back(IdiomTerm(e_No_Entry, NOARG, NOARG));
	i28.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i28.terms.begin(), i28.terms.end());
	prefix.addIdiom(i28);

	Idiom i29;
	i29.w = 9.170271;
	i29.prefix = true;
	i29.terms.push_back(IdiomTerm(e_mov, x86_64::eax, MEMARG));
	i29.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i29.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i29.terms.begin(), i29.terms.end());
	prefix.addIdiom(i29);

	Idiom i30;
	i30.w = 8.991899;
	i30.prefix = false;
	i30.terms.push_back(IdiomTerm(e_sub, x86_64::rsp, IMMARG));
	i30.terms.push_back(IdiomTerm(e_mov, x86_64::rdx, IMMARG));
	i30.terms.push_back(IdiomTerm(e_mov, x86_64::rsi, IMMARG));
	normal.addIdiom(i30);

	Idiom i31;
	i31.w = 8.803897;
	i31.prefix = true;
	i31.terms.push_back(IdiomTerm(e_pop, x86_64::r12, x86_64::rsp));
	i31.terms.push_back(WILDCARD_TERM);
	i31.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i31.terms.begin(), i31.terms.end());
	prefix.addIdiom(i31);

	Idiom i32;
	i32.w = 8.573753;
	i32.prefix = true;
	i32.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::esi));
	i32.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i32.terms.begin(), i32.terms.end());
	prefix.addIdiom(i32);

	Idiom i33;
	i33.w = 8.555044;
	i33.prefix = true;
	i33.terms.push_back(IdiomTerm(e_add, x86_64::eax, x86_64::edx));
	i33.terms.push_back(WILDCARD_TERM);
	i33.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i33.terms.begin(), i33.terms.end());
	prefix.addIdiom(i33);

	Idiom i34;
	i34.w = 8.468515;
	i34.prefix = false;
	i34.terms.push_back(IdiomTerm(e_push, x86_64::rax, x86_64::rsp));
	i34.terms.push_back(IdiomTerm(e_push, x86_64::rdx, x86_64::rsp));
	normal.addIdiom(i34);

	Idiom i35;
	i35.w = 8.464544;
	i35.prefix = true;
	i35.terms.push_back(IdiomTerm(e_mov, MEMARG, x86_64::rdi));
	i35.terms.push_back(WILDCARD_TERM);
	i35.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i35.terms.begin(), i35.terms.end());
	prefix.addIdiom(i35);

	Idiom i36;
	i36.w = 8.383780;
	i36.prefix = true;
	i36.terms.push_back(IdiomTerm(e_je, x86_64::rip, NOARG));
	i36.terms.push_back(IdiomTerm(e_call, x86_64::rip, NOARG));
	i36.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i36.terms.begin(), i36.terms.end());
	prefix.addIdiom(i36);

	Idiom i37;
	i37.w = 8.348538;
	i37.prefix = true;
	i37.terms.push_back(IdiomTerm(e_and, x86_64::al, IMMARG));
	i37.terms.push_back(IdiomTerm(e_add, x86_64::rsp, IMMARG));
	i37.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i37.terms.begin(), i37.terms.end());
	prefix.addIdiom(i37);

	Idiom i38;
	i38.w = 8.345892;
	i38.prefix = true;
	i38.terms.push_back(IdiomTerm(e_xor, x86_64::eax, x86_64::eax));
	i38.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i38.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i38.terms.begin(), i38.terms.end());
	prefix.addIdiom(i38);

	Idiom i39;
	i39.w = 8.015981;
	i39.prefix = false;
	i39.terms.push_back(IdiomTerm(e_sub, x86_64::rsp, IMMARG));
	i39.terms.push_back(WILDCARD_TERM);
	i39.terms.push_back(IdiomTerm(e_test, x86_64::rax, x86_64::rax));
	normal.addIdiom(i39);

	Idiom i40;
	i40.w = 7.947397;
	i40.prefix = true;
	i40.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i40.terms.push_back(WILDCARD_TERM);
	i40.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	reverse(i40.terms.begin(), i40.terms.end());
	prefix.addIdiom(i40);

	Idiom i41;
	i41.w = 7.946556;
	i41.prefix = true;
	i41.terms.push_back(IdiomTerm(e_add, MEMARG, x86_64::al));
	i41.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	i41.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i41.terms.begin(), i41.terms.end());
	prefix.addIdiom(i41);

	Idiom i42;
	i42.w = 7.937924;
	i42.prefix = true;
	i42.terms.push_back(IdiomTerm(e_rcr, MEMARG, IMMARG));
	i42.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i42.terms.begin(), i42.terms.end());
	prefix.addIdiom(i42);

	Idiom i43;
	i43.w = 7.573627;
	i43.prefix = true;
	i43.terms.push_back(IdiomTerm(e_mov, x86_64::rax, IMMARG));
	i43.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i43.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	reverse(i43.terms.begin(), i43.terms.end());
	prefix.addIdiom(i43);

	Idiom i44;
	i44.w = 7.290136;
	i44.prefix = false;
	i44.terms.push_back(IdiomTerm(e_push, x86_64::r15, x86_64::rsp));
	i44.terms.push_back(WILDCARD_TERM);
	i44.terms.push_back(IdiomTerm(e_push, x86_64::rsi, x86_64::rsp));
	normal.addIdiom(i44);

	Idiom i45;
	i45.w = 4.767534;
	i45.prefix = false;
	i45.terms.push_back(IdiomTerm(e_sub, x86_64::rsp, IMMARG));
	i45.terms.push_back(WILDCARD_TERM);
	i45.terms.push_back(IdiomTerm(e_test, x86_64::rdi, x86_64::rdi));
	normal.addIdiom(i45);

	Idiom i46;
	i46.w = -4.140649;
	i46.prefix = true;
	i46.terms.push_back(IdiomTerm(e_and, x86_64::al, IMMARG));
	i46.terms.push_back(WILDCARD_TERM);
	reverse(i46.terms.begin(), i46.terms.end());
	prefix.addIdiom(i46);

	Idiom i47;
	i47.w = -4.339725;
	i47.prefix = false;
	i47.terms.push_back(IdiomTerm(e_xor, x86_64::edi, x86_64::edi));
	normal.addIdiom(i47);

	Idiom i48;
	i48.w = -6.111701;
	i48.prefix = true;
	i48.terms.push_back(IdiomTerm(e_mov, x86_64::rbp, MEMARG));
	i48.terms.push_back(WILDCARD_TERM);
	reverse(i48.terms.begin(), i48.terms.end());
	prefix.addIdiom(i48);

	Idiom i49;
	i49.w = -6.592325;
	i49.prefix = true;
	i49.terms.push_back(IdiomTerm(e_je, x86_64::rip, NOARG));
	reverse(i49.terms.begin(), i49.terms.end());
	prefix.addIdiom(i49);

	Idiom i50;
	i50.w = -6.962922;
	i50.prefix = true;
	i50.terms.push_back(IdiomTerm(e_ret_near, MEMARG, NOARG));
	i50.terms.push_back(IdiomTerm(e_push, x86_64::rbp, x86_64::rsp));
	i50.terms.push_back(IdiomTerm(e_mov, x86_64::rbp, x86_64::rsp));
	reverse(i50.terms.begin(), i50.terms.end());
	prefix.addIdiom(i50);

	Idiom i51;
	i51.w = -11.891171;
	i51.prefix = true;
	i51.terms.push_back(IdiomTerm(e_jne, x86_64::rip, NOARG));
	reverse(i51.terms.begin(), i51.terms.end());
	prefix.addIdiom(i51);

	Idiom i52;
	i52.w = -14.770056;
	i52.prefix = false;
	i52.terms.push_back(IdiomTerm(e_nop, NOARG, NOARG));
	normal.addIdiom(i52);

    }    
  #endif
#endif
}

#endif
