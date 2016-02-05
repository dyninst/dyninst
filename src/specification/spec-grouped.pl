% Dyninst test suite specification file

%%%%%%%%%%
%
% DO NOT EDIT ANYTHING UNTIL YOU REACH THE 'SAFE TO EDIT' MARK
%
%%%%%%%%%%

% Include some utility definitions
:- include('test.pl').

% Allow the user to specify new clauses as they want throughout this file
:- discontiguous([library/2, object_suffix/2, library_suffix/2,
                  library_prefix/2, test_plat/2, mutator/2,
                  mutator_requires_libs/2, mutator_comp/1, mutatee/2,
                  mutatee/3, mutatee_requires_libs/2, mutatee_comp/1, lang/1,
                  comp_lang/2, platform/4, compiler_opt_trans/3,
                  comp_mut/2, compiler_platform/2,
                  mcomp_plat/2, test_runmode/2, 
                  test_threadmode/2, test_processmode/2, threadmode/1, processmode/1,
                  mutatee_format/2, format_runmode/3, platform_format/2, 
                  compiler_format/2, test_exclude_format/2,
                  test_serializable/1, comp_std_flags_str/2,
                  comp_mutatee_flags_str/2, test_runs_everywhere/1,
                  mutatee_special_make_str/2, mutatee_special_requires/2,
                  groupable_test/1, test_platform/2,
                  compiler_for_mutatee/2, test_start_state/2,
                  compiler_define_string/2, compiler_s/2,
                  mutatee_link_options/2, mutatee_peer/2,
                  compiler_parm_trans/3, test/3, test_description/2,
                  optimization_for_mutatee/3, spec_exception/3,
                  spec_object_file/6, fortran_c_component/1,
                  whitelist/1, parameter/1, parameter_values/2,
                  mutatee_abi/1, platform_abi/2, module_pic/2,
                  mutatee_module/2, module_runmode_format/3,
                  compiler_platform_abi_s/5, test_platform_abi/3,
                  restricted_amd64_abi/1, compiler_presence_def/2,
                  restricted_abi_for_arch/3, insane/2, module/1,
                  compiler_static_link/3, compiler_dynamic_link/3,
                  compiler_platform_abi/3, tests_module/2, mutator_requires_libs/2, 
                  test_exclude_compiler/2, remote_platform/1, remote_runmode_mutator/2,
                  remote_runmode_mutatee/2, mutatee_launchtime/2, runmode_launch_params/5,
                  platform_module/2, mutatee_compiler_platform_exclude/2,
                  platform_mode/4, runmode_platform/3]).

%%%%%%%%%%
%
% SAFE TO EDIT
% It is safe to add new clauses and edit the clauses in the section below
%
%%%%%%%%%%

% Some tests require shared libraries as well as mutatees
%mutator('test_requireslib', ['test_requireslib.C']).
% We need to specify that 'test_requireslib' requires the library 'requireslib'
%mutator_library('test_requireslib', 'requireslib').
% And then we need to provide a definition of the library and the source files
% it reqires if we havent already done so.
% NOTE TO SELF:
% So we want Windows to compile the following file to 'requireslib.dll' and
% Unix to compile it to 'librequireslib.so'
%library('requireslib', ['requireslib.C']).

% Mutatees can require that they're linked with certain libraries.  Here's
% how to specify that
%mutatee_requires_libs('test_threaded_mutatee', ['pthread']).

module('dyninst').
module('symtab').
module('stackwalker').
module('instruction').
module('proccontrol').
module('patchapi').

platform_module(_, 'dyninst').
platform_module(_, 'symtab').
platform_module(P, 'instruction') :- platform('i386', _, _, P).
platform_module(P, 'instruction') :- platform('x86_64', _, _, P).
platform_module(P, 'instruction') :- platform('power32', _, _, P).
platform_module(P, 'instruction') :- platform('power64', _, _, P).
platform_module(P, 'proccontrol') :- platform(_, 'linux', _, P).
platform_module(P, 'proccontrol') :- platform(_, 'freebsd', _, P).
platform_module(P, 'proccontrol') :- platform(_, 'bluegene', _, P).
platform_module(P, 'proccontrol') :- platform(_, 'windows', _, P).
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Below are specifications for the standard Dyninst test suite
%
% DO NOT EDIT ANYTHING BELOW THIS MARK UNLESS YOU'RE SURE YOU KNOW
% WHAT YOU'RE DOING
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% This is a dummy mutatee definition thats used by some tests that check that
% we correctly identify error conditions
mutatee('none', []).
compiler_for_mutatee('none', '').
compiler_platform('', P) :- platform(P).
% Also a dummy compiler for our dummy mutatee
compiler_define_string('', '').
mutatee_comp('').
mutatee_link_options('', '').
comp_std_flags_str('', '').
comp_mutatee_flags_str('', '').

mutatee('dyninst_group_test', ['test1_1_mutatee.c', 
	'test1_2_mutatee.c',
	'test1_3_mutatee.c', 
	'test1_4_mutatee.c',
	'test1_5_mutatee.c', 
	'test1_6_mutatee.c',
	'test1_7_mutatee.c', 
	'test1_8_mutatee.c',
	'test1_9_mutatee.c', 
	'test1_10_mutatee.c',
	'test1_11_mutatee.c',
	'test1_13_mutatee.c',
	'test1_16_mutatee.c',
	'test1_17_mutatee.c',
	'test1_18_mutatee.c',
	'test1_20_mutatee.c',
	'test1_21_mutatee.c',
	'test1_22_mutatee.c',
	'snip_ref_shlib_var_mutatee.c',
	'snip_change_shlib_var_mutatee.c',
	'test1_23_mutatee.c',
	'test1_24_mutatee.c',
	'test1_25_mutatee.c',
	'test1_26_mutatee.c',
	'test1_27_mutatee.c',
	'test1_28_mutatee.c',
	'test1_30_mutatee.c',
	'test1_31_mutatee.c',
	'test1_32_mutatee.c',
	'test1_33_mutatee.c',
	'test1_34_mutatee.c',
	'test1_36_mutatee.c',
	'test1_37_mutatee.c',
	'test1_38_mutatee.c',
	'test1_39_mutatee.c',
	'test2_5_mutatee.c',
	'test2_7_mutatee.c',
	'test2_9_mutatee.c',
	'test2_11_mutatee.c',
	'test2_12_mutatee.c',
	'test2_13_mutatee.c',
   'test_write_param_mutatee.c'
    ]).
compiler_for_mutatee('dyninst_group_test', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_format('dyninst_group_test', 'staticMutatee').

mutatee('dyninst_cxx_group_test', ['test5_1_mutatee.C',
	'test5_2_mutatee.C',
	'test5_3_mutatee.C',
	'test5_4_mutatee.C',
	'test5_5_mutatee.C',
	'test5_6_mutatee.C',
	'test5_7_mutatee.C',
	'test5_8_mutatee.C',
	'test5_9_mutatee.C', 
        'cpp_test.C']).
compiler_for_mutatee('dyninst_cxx_group_test', Compiler) :-
    comp_lang(Compiler, 'c++').
mutatee_format('dyninst_cxx_group_test', 'staticMutatee').

mutatee('symtab_group_test', [
   'test_lookup_func_mutatee.c',
	'test_lookup_var_mutatee.c',
	'test_line_info_mutatee.c',
	'test_module_mutatee.c',
	'test_relocations_mutatee.c',
	'test_symtab_ser_funcs_mutatee.c',
	'test_ser_anno_mutatee.c',
	'test_type_info_mutatee.c',
        'test_anno_basic_types_mutatee.c',
        'test_add_symbols_mutatee.c'
   ]).
compiler_for_mutatee('symtab_group_test', Compiler) :-
    comp_lang(Compiler, 'c').
compiler_for_mutatee('symtab_cxx_group_test', Compiler) :-
    comp_lang(Compiler, 'c++').

test('test1_1', 'test1_1', 'dyninst_group_test').
test_description('test1_1', 'instrument with zero-arg function call').
test_runs_everywhere('test1_1').
groupable_test('test1_1').
mutator('test1_1', ['test1_1.C']).
test_runmode('test1_1', 'staticdynamic').
test_start_state('test1_1', 'stopped').
tests_module('test1_1', 'dyninst').

test('test1_2', 'test1_2', 'dyninst_group_test').
test_description('test1_2', 'instrument with four-arg function call').
test_runs_everywhere('test1_2').
groupable_test('test1_2').
mutator('test1_2', ['test1_2.C']).
mutatee('test1_2', ['test1_2_mutatee.c']).
compiler_for_mutatee('test1_2', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_2', 'staticdynamic').
test_start_state('test1_2', 'stopped').
tests_module('test1_2', 'dyninst').

test('test1_3', 'test1_3', 'dyninst_group_test').
test_description('test1_3', 'passing variables to a function').
test_runs_everywhere('test1_3').
groupable_test('test1_3').
mutator('test1_3', ['test1_3.C']).
test_runmode('test1_3', 'staticdynamic').
test_start_state('test1_3', 'stopped').
tests_module('test1_3', 'dyninst').

test('test1_4', 'test1_4', 'dyninst_group_test').
test_description('test1_4', 'instrument with a sequence').
test_runs_everywhere('test1_4').
groupable_test('test1_4').
mutator('test1_4', ['test1_4.C']).
test_runmode('test1_4', 'staticdynamic').
test_start_state('test1_4', 'stopped').
tests_module('test1_4', 'dyninst').

test('test1_5', 'test1_5', 'dyninst_group_test').
test_description('test1_5', 'instrument with if clause (no else)').
test_runs_everywhere('test1_5').
groupable_test('test1_5').
mutator('test1_5', ['test1_5.C']).
test_runmode('test1_5', 'staticdynamic').
test_start_state('test1_5', 'stopped').
tests_module('test1_5', 'dyninst').

test('test1_6', 'test1_6', 'dyninst_group_test').
test_description('test1_6', 'arithmetic operators').
test_runs_everywhere('test1_6').
groupable_test('test1_6').
mutator('test1_6', ['test1_6.C']).
test_runmode('test1_6', 'staticdynamic').
test_start_state('test1_6', 'stopped').
tests_module('test1_6', 'dyninst').

test('test1_7', 'test1_7', 'dyninst_group_test').
test_description('test1_7', 'relational operators').
test_runs_everywhere('test1_7').
groupable_test('test1_7').
mutator('test1_7', ['test1_7.C']).
test_runmode('test1_7', 'staticdynamic').
test_start_state('test1_7', 'stopped').
tests_module('test1_7', 'dyninst').

test('test1_8', 'test1_8', 'dyninst_group_test').
test_description('test1_8', 'verify that registers are preserved across inserted expression').
test_runs_everywhere('test1_8').
groupable_test('test1_8').
mutator('test1_8', ['test1_8.C']).
test_runmode('test1_8', 'staticdynamic').
test_start_state('test1_8', 'stopped').
tests_module('test1_8', 'dyninst').

test('test1_9', 'test1_9', 'dyninst_group_test').
test_description('test1_9', 'verify that registers are preserved across inserted function call').
test_runs_everywhere('test1_9').
groupable_test('test1_9').
mutator('test1_9', ['test1_9.C']).
test_runmode('test1_9', 'staticdynamic').
test_start_state('test1_9', 'stopped').
tests_module('test1_9', 'dyninst').

test('test1_10', 'test1_10', 'dyninst_group_test').
test_description('test1_10', 'inserted snippet order').
test_runs_everywhere('test1_10').
groupable_test('test1_10').
mutator('test1_10', ['test1_10.C']).
test_runmode('test1_10', 'staticdynamic').
test_start_state('test1_10', 'stopped').
tests_module('test1_10', 'dyninst').

test('test1_11', 'test1_11', 'dyninst_group_test').
test_description('test1_11', 'insert snippets at entry, exit, and call points').
test_runs_everywhere('test1_11').
groupable_test('test1_11').
mutator('test1_11', ['test1_11.C']).
test_runmode('test1_11', 'staticdynamic').
test_start_state('test1_11', 'stopped').
tests_module('test1_11', 'dyninst').

test('test1_12', 'test1_12', 'test1_12').
test_description('test1_12', 'insert/remove and malloc/free').
test_runs_everywhere('test1_12').
mutator('test1_12', ['test1_12.C']).
mutatee('test1_12', ['test1_12_mutatee.c']).
compiler_for_mutatee('test1_12', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_12', 'dynamic').
test_start_state('test1_12', 'stopped').
tests_module('test1_12', 'dyninst').

test('test1_13', 'test1_13', 'dyninst_group_test').
test_description('test1_13', 'paramExpr,retExpr,nullExpr').
test_runs_everywhere('test1_13').
groupable_test('test1_13').
mutator('test1_13', ['test1_13.C']).
test_runmode('test1_13', 'staticdynamic').
test_start_state('test1_13', 'stopped').
tests_module('test1_13', 'dyninst').

test('test1_14', 'test1_14', 'test1_14').
test_description('test1_14', 'Replace/Remove Function Call').
test_runs_everywhere('test1_14').
mutator('test1_14', ['test1_14.C']).
mutatee('test1_14', ['test1_14_mutatee.c']).
% test1_14s mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_14', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_14', 'staticdynamic').
test_start_state('test1_14', 'stopped').
tests_module('test1_14', 'dyninst').
groupable_test('test1_14').
mutatee_format('test1_14', 'staticMutatee').

% Removed 2AUG2012, nobody uses this functionality and it doesn't work. 
%test('test1_15', 'test1_15', 'test1_15').
%test_description('test1_15', 'setMutationsActive').
%test_runs_everywhere('test1_15').
%mutator('test1_15', ['test1_15.C']).
%mutatee('test1_15', ['test1_15_mutatee.c']).
%compiler_for_mutatee('test1_15', Compiler) :-
%   comp_lang(Compiler, 'c').
%test_runmode('test1_15', 'dynamic').
%test_start_state('test1_15', 'stopped').
%tests_module('test1_15', 'dyninst').

test('test1_16', 'test1_16', 'dyninst_group_test').
test_description('test1_16', 'If else').
test_runs_everywhere('test1_16').
groupable_test('test1_16').
mutator('test1_16', ['test1_16.C']).
test_runmode('test1_16', 'staticdynamic').
test_start_state('test1_16', 'stopped').
tests_module('test1_16', 'dyninst').

test('test1_17', 'test1_17', 'dyninst_group_test').
test_description('test1_17', 'Verifies that instrumentation inserted at exit point doesn\'t clobber return value').
test_runs_everywhere('test1_17').
groupable_test('test1_17').
mutator('test1_17', ['test1_17.C']).
test_runmode('test1_17', 'staticdynamic').
test_start_state('test1_17', 'stopped').
tests_module('test1_17', 'dyninst').

test('test1_18', 'test1_18', 'dyninst_group_test').
test_description('test1_18', 'Read/Write a variable in the mutatee').
test_runs_everywhere('test1_18').
groupable_test('test1_18').
mutator('test1_18', ['test1_18.C']).
test_runmode('test1_18', 'staticdynamic').
test_start_state('test1_18', 'stopped').
tests_module('test1_18', 'dyninst').

test('test1_19', 'test1_19', 'test1_19').
test_description('test1_19', 'oneTimeCode').
test_runs_everywhere('test1_19').
mutator('test1_19', ['test1_19.C']).
mutatee('test1_19', ['test1_19_mutatee.c']).
compiler_for_mutatee('test1_19', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_19', 'dynamic').
test_start_state('test1_19', 'stopped').
tests_module('test1_19', 'dyninst').

test('test1_20', 'test1_20', 'dyninst_group_test').
test_description('test1_20', 'Instrumentation at arbitrary points').
test_runs_everywhere('test1_20').
groupable_test('test1_20').
mutator('test1_20', ['test1_20.C']).
test_runmode('test1_20', 'staticdynamic').
test_start_state('test1_20', 'stopped').
tests_module('test1_20', 'dyninst').
mutator_requires_libs('test1_20', L) :-
        current_platform(P),
        platform(Arch, _, _, P),
        (Arch = 'i386' -> L = ['instructionAPI'];
         Arch = 'x86_64' -> L = ['instructionAPI'];
         Arch = 'power32' -> L = ['instructionAPI'];
         Arch = 'power64' -> L = ['instructionAPI'];
         Arch = 'powerpc' -> L = ['instructionAPI'];
         L = []
                ).

test('test1_21', 'test1_21', 'dyninst_group_test').
test_description('test1_21', 'findFunction in module').
test_runs_everywhere('test1_21').
groupable_test('test1_21').
mutator('test1_21', ['test1_21.C']).
test_runmode('test1_21', 'staticdynamic').
test_start_state('test1_21', 'stopped').
tests_module('test1_21', 'dyninst').

test('test1_22', 'test1_22', 'dyninst_group_test').
test_description('test1_22', 'Replace Function').
test_runs_everywhere('test1_22').
groupable_test('test1_22').
mutator('test1_22', ['test1_22.C']).
mutatee_requires_libs('dyninst_group_test', Libs) :-
    % FreeBSD doesn't have a libdl
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = [];
        Libs = ['dl']
    ).
test_runmode('test1_22', 'staticdynamic').
test_start_state('test1_22', 'stopped').
tests_module('test1_22', 'dyninst').
test_exclude_format('test1_22', 'staticMutatee').

test('snip_ref_shlib_var', 'snip_ref_shlib_var', 'dyninst_group_test').
test_description('snip_ref_shlib_var', 'Inst references variable in shared lib').
test_runs_everywhere('snip_ref_shlib_var').
groupable_test('snip_ref_shlib_var').
mutator('snip_ref_shlib_var', ['snip_ref_shlib_var.C']).
test_runmode('snip_ref_shlib_var', 'staticdynamic').
test_start_state('snip_ref_shlib_var', 'stopped').
tests_module('snip_ref_shlib_var', 'dyninst').

test('snip_change_shlib_var', 'snip_change_shlib_var', 'dyninst_group_test').
test_description('snip_change_shlib_var', 'Inst modifies variable in shared lib').
test_runs_everywhere('snip_change_shlib_var').
groupable_test('snip_change_shlib_var').
mutator('snip_change_shlib_var', ['snip_change_shlib_var.C']).
test_runmode('snip_change_shlib_var', 'staticdynamic').
test_start_state('snip_change_shlib_var', 'stopped').
tests_module('snip_change_shlib_var', 'dyninst').

% test_snip_remove
test('test_snip_remove', 'test_snip_remove', 'test_snip_remove').
test_description('test_snip_remove', 'Tests multiple snippet removal').
test_runs_everywhere('test_snip_remove').
groupable_test('test_snip_remove').
mutator('test_snip_remove', ['test_snip_remove.C']).
mutatee('test_snip_remove', ['test_snip_remove_mutatee.c']).
compiler_for_mutatee('test_snip_remove', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_snip_remove', 'dynamic').
test_start_state('test_snip_remove', 'stopped').
tests_module('test_snip_remove', 'dyninst').

% amd64_7_arg_call
test('amd64_7_arg_call', 'amd64_7_arg_call', 'amd64_7_arg_call').
test_description('amd64_7_arg_call', 'AMD64: verify that we can make calls using the stack and GPRs for parameter passing.').
    test_platform('amd64_7_arg_call', Platform) :-
    platform(Arch, _, _, Platform),
    member(Arch, ['x86_64']).
groupable_test('amd64_7_arg_call').
mutator('amd64_7_arg_call', ['amd64_7_arg_call.C']).
mutatee('amd64_7_arg_call', ['amd64_7_arg_call_mutatee.c']).
compiler_for_mutatee('amd64_7_arg_call', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('amd64_7_arg_call', 'staticdynamic').
test_start_state('amd64_7_arg_call', 'stopped').
tests_module('amd64_7_arg_call', 'dyninst').


test('init_fini_callback', 'init_fini_callback', 'init_fini_callback').
test_description('init_fini_callback', 'Adds callbacks for rewritten module on load/unload/entry/exit.').
% ELF platforms only
    test_platform('init_fini_callback', Platform) :-
    platform(Arch, OS, _, Platform),
    member(OS, ['linux', 'freebsd', 'bluegene']),
    member(Arch, ['i386', 'x86_64', 'power32', 'power64']).
mutator('init_fini_callback', ['init_fini_callback.C']).
mutatee('init_fini_callback', ['init_fini_callback_mutatee.c']).
mutatee_requires_libs('init_fini_callback', Libs) :-
    % FreeBSD doesn't have a libdl
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = [];
        Libs = ['dl']
    ).
compiler_for_mutatee('init_fini_callback', Compiler) :-
    comp_lang(Compiler, 'c').
groupable_test('init_fini_callback').
test_runmode('init_fini_callback', 'staticdynamic').
test_start_state('init_fini_callback', 'stopped').
tests_module('init_fini_callback', 'dyninst').

test('test1_23', 'test1_23', 'dyninst_group_test').
test_description('test1_23', 'Local Variables').
test_runs_everywhere('test1_23').
groupable_test('test1_23').
mutator('test1_23', ['test1_23.C']).
test_runmode('test1_23', 'staticdynamic').
test_start_state('test1_23', 'stopped').
tests_module('test1_23', 'dyninst').

test('test1_24', 'test1_24', 'dyninst_group_test').
test_description('test1_24', 'Array Variables').
test_runs_everywhere('test1_24').
groupable_test('test1_24').
mutator('test1_24', ['test1_24.C']).
test_runmode('test1_24', 'staticdynamic').
test_start_state('test1_24', 'stopped').
tests_module('test1_24', 'dyninst').

test('test1_25', 'test1_25', 'dyninst_group_test').
test_description('test1_25', 'Unary Operators').
test_runs_everywhere('test1_25').
groupable_test('test1_25').
mutator('test1_25', ['test1_25.C']).
test_runmode('test1_25', 'staticdynamic').
test_start_state('test1_25', 'stopped').
tests_module('test1_25', 'dyninst').

test('test1_26', 'test1_26', 'dyninst_group_test').
test_description('test1_26', 'Struct Elements').
test_runs_everywhere('test1_26').
groupable_test('test1_26').
mutator('test1_26', ['test1_26.C']).
test_runmode('test1_26', 'staticdynamic').
test_start_state('test1_26', 'stopped').
tests_module('test1_26', 'dyninst').

test('test1_27', 'test1_27', 'dyninst_group_test').
test_description('test1_27', 'Type compatibility').
test_runs_everywhere('test1_27').
groupable_test('test1_27').
mutator('test1_27', ['test1_27.C']).
test_runmode('test1_27', 'staticdynamic').
test_start_state('test1_27', 'stopped').
tests_module('test1_27', 'dyninst').

test('test1_28', 'test1_28', 'dyninst_group_test').
test_description('test1_28', 'User Defined Fields').
test_runs_everywhere('test1_28').
groupable_test('test1_28').
mutator('test1_28', ['test1_28.C']).
test_runmode('test1_28', 'staticdynamic').
test_start_state('test1_28', 'stopped').
tests_module('test1_28', 'dyninst').

test('test1_29', 'test1_29', 'test1_29').
test_description('test1_29', 'BPatch_srcObj class').
test_runs_everywhere('test1_29').
groupable_test('test1_29').
mutator('test1_29', ['test1_29.C']).
mutatee('test1_29', ['test1_29_mutatee.c']).
% test1_29s mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_29', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_29', 'staticdynamic').
test_start_state('test1_29', 'stopped').
tests_module('test1_29', 'dyninst').
mutatee_format('test1_29', 'staticMutatee').

test('test1_30', 'test1_30', 'dyninst_group_test').
test_description('test1_30', 'Line Information').
test_runs_everywhere('test1_30').
groupable_test('test1_30').
mutator('test1_30', ['test1_30.C']).
test_runmode('test1_30', 'staticdynamic').
test_start_state('test1_30', 'stopped').
tests_module('test1_30', 'dyninst').

test('test1_31', 'test1_31', 'dyninst_group_test').
test_description('test1_31', 'Non-Recursive Base Tramp').
test_runs_everywhere('test1_31').
groupable_test('test1_31').
mutator('test1_31', ['test1_31.C']).
test_runmode('test1_31', 'staticdynamic').
test_start_state('test1_31', 'stopped').
tests_module('test1_31', 'dyninst').

test('test1_32', 'test1_32', 'dyninst_group_test').
test_description('test1_32', 'Recursive Base Tramp').
test_runs_everywhere('test1_32').
groupable_test('test1_32').
mutator('test1_32', ['test1_32.C']).
test_runmode('test1_32', 'staticdynamic').
test_start_state('test1_32', 'stopped').
tests_module('test1_32', 'dyninst').

test('test1_33', 'test1_33', 'dyninst_group_test').
test_description('test1_33', 'Control Flow Graphs').
test_runs_everywhere('test1_33').
groupable_test('test1_33').
mutator('test1_33', ['test1_33.C']).
test_runmode('test1_33', 'staticdynamic').
test_start_state('test1_33', 'stopped').
tests_module('test1_33', 'dyninst').

test('test1_34', 'test1_34', 'dyninst_group_test').
test_description('test1_34', 'Loop Information').
test_runs_everywhere('test1_34').
groupable_test('test1_34').
mutator('test1_34', ['test1_34.C']).
test_runmode('test1_34', 'staticdynamic').
test_start_state('test1_34', 'stopped').
tests_module('test1_34', 'dyninst').

test('test1_35', 'test1_35', 'test1_35').
test_description('test1_35', 'Function Relocation').
test_platform('test1_35', 'i386-unknown-linux2.4').
test_platform('test1_35', 'i386-unknown-linux2.6').
test_platform('test1_35', 'x86_64-unknown-linux2.4').
test_platform('test1_35', 'i386-unknown-freebsd7.2').
test_platform('test1_35', 'amd64-unknown-freebsd7.2').
groupable_test('test1_35').
mutator('test1_35', ['test1_35.C']).
mutatee('test1_35', ['test1_35_mutatee.c', Sources]) :-
    current_platform(Plat), platform(Arch, OS, _, Plat),
    (
        (Arch = 'x86_64', OS = 'linux') ->
            Sources = 'call35_1_x86_64_linux.s';
        (Arch = 'i386', OS = 'linux') ->
            Sources = 'call35_1_x86_linux.s';
        (Arch = 'i386', OS = 'freebsd') ->
            Sources = 'call35_1_x86_linux.s';
        (Arch = 'x86_64', OS = 'freebsd') ->
            Sources = 'call35_1_x86_64_linux.s';
        Sources = 'call35_1.c'
    ).
% test1_35s mutatee can be compiled with any C compiler
compiler_for_mutatee('test1_35', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_35', 'staticdynamic').
test_start_state('test1_35', 'stopped').
restricted_amd64_abi('test1_35').
tests_module('test1_35', 'dyninst').
mutatee_format('test1_35', 'staticMutatee').

test('test1_36', 'test1_36', 'dyninst_group_test').
test_description('test1_36', 'Callsite Parameter Referencing').
test_runs_everywhere('test1_36').
groupable_test('test1_36').
mutator('test1_36', ['test1_36.C']).
test_runmode('test1_36', 'staticdynamic').
test_start_state('test1_36', 'stopped').
tests_module('test1_36', 'dyninst').

test('test1_37', 'test1_37', 'dyninst_group_test').
test_description('test1_37', 'Instrument Loops').
test_runs_everywhere('test1_37').
groupable_test('test1_37').
mutator('test1_37', ['test1_37.C']).
test_runmode('test1_37', 'staticdynamic').
test_start_state('test1_37', 'stopped').
tests_module('test1_37', 'dyninst').

% FIXME I dont think test1_38 runs on all platforms
test('test1_38', 'test1_38', 'dyninst_group_test').
test_description('test1_38', 'CFG Loop Callee Tree').
test_runs_everywhere('test1_38').
groupable_test('test1_38').
mutator('test1_38', ['test1_38.C']).
test_runmode('test1_38', 'staticdynamic').
test_start_state('test1_38', 'stopped').
tests_module('test1_38', 'dyninst').

test('test1_39', 'test1_39', 'dyninst_group_test').
test_description('test1_39', 'Regex search').
% test1_39 doesnt run on Windows
test_platform('test1_39', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
groupable_test('test1_39').
mutator('test1_39', ['test1_39.C']).
test_runmode('test1_39', 'staticdynamic').
test_start_state('test1_39', 'stopped').
tests_module('test1_39', 'dyninst').

test('test1_40', 'test1_40', 'test1_40').
test_description('test1_40', 'Verify that we can monitor call sites').
% test1_40 should not run on Windows or IA64 Linux
test_platform('test1_40', Platform) :-
        platform(Platform),
        \+ platform(_, 'windows', _, Platform),
        \+ platform(_, 'bluegene', _, Platform).
groupable_test('test1_40').
mutator('test1_40', ['test1_40.C']).
mutatee('test1_40', ['test1_40_mutatee.c']).
% test1_40s mutatee can be compiled with any C compiler except xlc/xlC
compiler_for_mutatee('test1_40', Compiler) :-
    comp_lang(Compiler, 'c'),
    \+ member(Compiler, ['xlc', 'xlC']).
test_runmode('test1_40', 'dynamic').
test_start_state('test1_40', 'stopped').
tests_module('test1_40', 'dyninst').

test('test1_41', 'test1_41', 'test1_41').
test_description('test1_41', 'Tests whether we lose line information running a mutatee twice').
% test1_41 doesnt run on Windows
test_platform('test1_41', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test1_41', ['test1_41.C']).
mutatee('test1_41', ['test1_41_mutatee.c']).
compiler_for_mutatee('test1_41', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test1_41', 'createProcess').
test_start_state('test1_41', 'selfstart').
tests_module('test1_41', 'dyninst').

test('test_write_param', 'test_write_param', 'dyninst_group_test').
test_description('test_write_param', 'writing to parameters').
test_platform('test_write_param', 'i386-unknown-linux2.4').
test_platform('test_write_param', 'x86_64-unknown-linux2.4').
test_platform('test_write_param', 'i386-unknown-nt4.0').
test_platform('test_write_param', 'rs6000-ibm-aix5.1').
test_platform('test_write_param', 'i386-unknown-freebsd7.2').
test_platform('test_write_param', 'amd64-unknown-freebsd7.2').
groupable_test('test_write_param').
mutator('test_write_param', ['test_write_param.C']).
test_runmode('test_write_param', 'staticdynamic').
test_start_state('test_write_param', 'stopped').
tests_module('test_write_param', 'dyninst').

test('test_pt_ls', 'test_pt_ls', none).
test_description('test_pt_ls', 'Run parseThat on ls').
% test_pt_ls doesnt run on Windowscd \paradyn
test_platform('test_pt_ls', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_pt_ls', ['test_pt_ls.C']).
test_runmode('test_pt_ls', 'staticdynamic').
test_start_state('test_pt_ls', 'selfstart').
tests_module('test_pt_ls', 'dyninst').

test('test2_1', 'test2_1', none).
test_description('test2_1', 'Run an executable that does not exist').
test_runs_everywhere('test2_1').
mutator('test2_1', ['test2_1.C']).
test_runmode('test2_1', 'createProcess').
test_start_state('test2_1', 'selfstart').
restricted_amd64_abi('test2_1').
tests_module('test2_1', 'dyninst').

test('test2_2', 'test2_2', none).
test_description('test2_2', 'Try to run a createProcess on a file that is not an executable file').
test_runs_everywhere('test2_2').
mutator('test2_2', ['test2_2.C']).
test_runmode('test2_2', 'createProcess').
test_start_state('test2_2', 'selfstart').
restricted_amd64_abi('test2_2').
tests_module('test2_2', 'dyninst').

test('test2_3', 'test2_3', none).
test_description('test2_3', 'Attatch to an invalid pid').
test_runs_everywhere('test2_3').
mutator('test2_3', ['test2_3.C']).
test_runmode('test2_3', 'useAttach').
test_start_state('test2_3', 'selfstart').
restricted_amd64_abi('test2_3').
tests_module('test2_3', 'dyninst').

test('test2_4', 'test2_4', none).
test_description('test2_4', 'Attach to a protected pid').
test_runs_everywhere('test2_4').
mutator('test2_4', ['test2_4.C']).
test_runmode('test2_4', 'useAttach').
test_start_state('test2_4', 'selfstart').
restricted_amd64_abi('test2_4').
tests_module('test2_4', 'dyninst').

test('test2_5', 'test2_5', 'dyninst_group_test').
test_description('test2_5', 'Look up nonexistent function').
test_runs_everywhere('test2_5').
groupable_test('test2_5').
mutator('test2_5', ['test2_5.C']).
test_runmode('test2_5', 'staticdynamic').
test_start_state('test2_5', 'stopped').
tests_module('test2_5', 'dyninst').

test('test2_6', 'test2_6', 'test2_6').
test_description('test2_6', 'Load a dynamically linked library from the mutatee').
% test2_6 doesnt run on Windows
test_platform('test2_6', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test2_6', ['test2_6.C']).
mutatee('test2_6', ['test2_6_mutatee.c']).
compiler_for_mutatee('test2_6', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_requires_libs('test2_6', Libs) :-
    % FreeBSD doesn't have a libdl
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = [];
        Libs = ['dl']
    ).
test_runmode('test2_6', 'dynamic').
test_start_state('test2_6', 'stopped').
tests_module('test2_6', 'dyninst').

test('test2_7', 'test2_7', 'dyninst_group_test').
test_description('test2_7', '').
% test2_7 runs on Solaris, Linux, AIX, and Windows
test_platform('test2_7', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'aix', 'windows', 'freebsd']).
mutator('test2_7', ['test2_7.C']).
test_runmode('test2_7', 'dynamic').
test_start_state('test2_7', 'stopped').
groupable_test('test2_7').
tests_module('test2_7', 'dyninst').

test('test2_8', 'test2_8', 'test2_8').
test_runs_everywhere('test2_8').
mutator('test2_8', ['test2_8.C']).
mutatee('test2_8', ['test2_8_mutatee.c']).
compiler_for_mutatee('test2_8', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test2_8', 'dynamic').
test_start_state('test2_8', 'stopped').
tests_module('test2_8', 'dyninst').

test('test2_11', 'test2_11', 'dyninst_group_test').
test_runs_everywhere('test2_11').
mutator('test2_11', ['test2_11.C']).
test_runmode('test2_11', 'dynamic').
test_start_state('test2_11', 'stopped').
groupable_test('test2_11').
tests_module('test2_11', 'dyninst').

test('test2_12', 'test2_12', 'dyninst_group_test').
test_runs_everywhere('test2_12').
mutator('test2_12', ['test2_12.C']).
test_runmode('test2_12', 'dynamic').
test_start_state('test2_12', 'stopped').
groupable_test('test2_12').
tests_module('test2_12', 'dyninst').

test('test2_13', 'test2_13', 'dyninst_group_test').
% test2_13 doesnt run on Alpha, but were not supporting Alpha any more, so we
% dont need to bother checking for it
test_runs_everywhere('test2_13').
mutator('test2_13', ['test2_13.C']).
test_runmode('test2_13', 'dynamic').
test_start_state('test2_13', 'stopped').
groupable_test('test2_13').
tests_module('test2_13', 'dyninst').

% test2_14 used getThreads(), which has been deprecated forever and is now gone.
test('test2_14', 'test2_14', 'test2_14').
test_runs_everywhere('test2_14').
mutator('test2_14', ['test2_14.C']).
mutatee('test2_14', ['test2_14_mutatee.c']).
compiler_for_mutatee('test2_14', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test2_14', 'dynamic').
test_start_state('test2_14', 'stopped').
tests_module('test2_14', 'dyninst').

test('test3_1', 'test3_1', 'test3_1').
test_runs_everywhere('test3_1').
mutator('test3_1', ['test3_1.C']).
mutatee('test3_1', ['test3_1_mutatee.c']).
compiler_for_mutatee('test3_1', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test3_1', 'createProcess').
test_start_state('test3_1', 'selfstart').
tests_module('test3_1', 'dyninst').

test('test3_2', 'test3_2', 'test3_2').
test_runs_everywhere('test3_2').
mutator('test3_2', ['test3_2.C']).
mutatee('test3_2', ['test3_2_mutatee.c']).
compiler_for_mutatee('test3_2', C) :- comp_lang(C, 'c').
test_runmode('test3_2', 'createProcess').
test_start_state('test3_2', 'selfstart').
tests_module('test3_2', 'dyninst').

test('test3_3', 'test3_3', 'test3_3').
test_runs_everywhere('test3_3').
mutator('test3_3', ['test3_3.C']).
mutatee('test3_3', ['test3_3_mutatee.c']).
compiler_for_mutatee('test3_3', C) :- comp_lang(C, 'c').
test_runmode('test3_3', 'createProcess').
test_start_state('test3_3', 'selfstart').
tests_module('test3_3', 'dyninst').

test('test3_4', 'test3_4', 'test3_4').
test_runs_everywhere('test3_4').
mutator('test3_4', ['test3_4.C']).
mutatee('test3_4', ['test3_4_mutatee.c']).
compiler_for_mutatee('test3_4', C) :- comp_lang(C, 'c').
test_runmode('test3_4', 'createProcess').
test_start_state('test3_4', 'selfstart').
tests_module('test3_4', 'dyninst').

test('test3_5', 'test3_5', 'test3_5').
test_platform('test3_5', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test3_5', ['test3_5.C']).
mutatee('test3_5', ['test3_5_mutatee.c']).
compiler_for_mutatee('test3_5', C) :- comp_lang(C, 'c').
test_runmode('test3_5', 'createProcess').
test_start_state('test3_5', 'selfstart').
tests_module('test3_5', 'dyninst').

test('test3_6', 'test3_6', 'test3_6').
test_description('test3_6', 'simultaneous multiple-process management - terminate (fork)').
% test3_6 doesnt run on Windows
test_platform('test3_6', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test3_6', ['test3_6.C']).
mutatee('test3_6', ['test3_6_mutatee.c']).
compiler_for_mutatee('test3_6', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test3_6', 'createProcess').
test_start_state('test3_6', 'selfstart').
tests_module('test3_6', 'dyninst').

test('test3_7', 'test3_7', 'test3_7').
test_runs_everywhere('test3_7').
mutator('test3_7', ['test3_7.C']).
mutatee('test3_7', ['test3_7_mutatee.c']).
compiler_for_mutatee('test3_7', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test3_7', 'createProcess').
test_start_state('test3_7', 'selfstart').
tests_module('test3_7', 'dyninst').

test('test4_1', 'test4_1', 'test4_1').
% Mutator claims test doesnt run on Alpha or Windows, but there were no
% checks to make sure it wasnt running on Windows..
test_runs_everywhere('test4_1').
mutator('test4_1', ['test4_1.C']).
mutatee('test4_1', ['test4_1_mutatee.c']).
compiler_for_mutatee('test4_1', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test4_1', 'createProcess').
test_start_state('test4_1', 'selfstart').
tests_module('test4_1', 'dyninst').

test('test4_2', 'test4_2', 'test4_2').
% test4_2 doesnt run on Windows
test_platform('test4_2', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test4_2', ['test4_2.C']).
mutatee('test4_2', ['test4_2_mutatee.c']).
compiler_for_mutatee('test4_2', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test4_2', 'createProcess').
test_start_state('test4_2', 'selfstart').
tests_module('test4_2', 'dyninst').

test('test4_3', 'test4_3', 'test4_3').
% test4_3 doesnt run on Windows
test_platform('test4_3', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test4_3', ['test4_3.C']).
mutatee('test4_3', ['test4_3_mutatee.c']).
compiler_for_mutatee('test4_3', Compiler) :- comp_lang(Compiler, 'c').
test_runmode('test4_3', 'createProcess').
test_start_state('test4_3', 'selfstart').
mutatee('test4_3b', ['test4_3b_mutatee.c']).
mutatee_peer('test4_3', 'test4_3b').
compiler_for_mutatee('test4_3b', Compiler) :- comp_lang(Compiler, 'c').
tests_module('test4_3', 'dyninst').

test('test4_4', 'test4_4', 'test4_4').
% test4_4 doesnt run on Windows
test_platform('test4_4', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test4_4', ['test4_4.C']).
mutatee('test4_4', ['test4_4_mutatee.c']).
compiler_for_mutatee('test4_4', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test4_4', 'createProcess').
test_start_state('test4_4', 'selfstart').
mutatee('test4_4b', ['test4_4b_mutatee.c']).
mutatee_peer('test4_4', 'test4_4b').
compiler_for_mutatee('test4_4b', Compiler) :-
    comp_lang(Compiler, 'c').
tests_module('test4_4', 'dyninst').

test('test5_1', 'test5_1', 'dyninst_cxx_group_test').
% test5_1 only runs on Linux, Solaris, and Windows
test_platform('test5_1', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_1', ['test5_1.C']).
test_runmode('test5_1', 'staticdynamic').
test_start_state('test5_1', 'stopped').
groupable_test('test5_1').
restricted_amd64_abi('test5_1').
tests_module('test5_1', 'dyninst').

test('test5_2', 'test5_2', 'dyninst_cxx_group_test').
% test5_2 only runs on Linux, Solaris, and Windows
test_platform('test5_2', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_2', ['test5_2.C']).
test_runmode('test5_2', 'staticdynamic').
test_start_state('test5_2', 'stopped').
groupable_test('test5_2').
restricted_amd64_abi('test5_2').
tests_module('test5_2', 'dyninst').

test('test5_3', 'test5_3', 'dyninst_cxx_group_test').
test_runs_everywhere('test5_3').
mutator('test5_3', ['test5_3.C']).
test_runmode('test5_3', 'staticdynamic').
test_start_state('test5_3', 'stopped').
groupable_test('test5_3').
restricted_amd64_abi('test5_3').
tests_module('test5_3', 'dyninst').

test('test5_4', 'test5_4', 'dyninst_cxx_group_test').
% test5_4 only runs on Linux, Solaris, and Windows
test_platform('test5_4', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_4', ['test5_4.C']).
test_runmode('test5_4', 'staticdynamic').
test_start_state('test5_4', 'stopped').
groupable_test('test5_4').
restricted_amd64_abi('test5_4').
tests_module('test5_4', 'dyninst').

test('test5_5', 'test5_5', 'dyninst_cxx_group_test').
% test5_5 only runs on Linux, Solaris, and Windows
test_platform('test5_5', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_5', ['test5_5.C']).
test_runmode('test5_5', 'staticdynamic').
test_start_state('test5_5', 'stopped').
groupable_test('test5_5').
restricted_amd64_abi('test5_5').
tests_module('test5_5', 'dyninst').

test('test5_6', 'test5_6', 'dyninst_cxx_group_test').
% test5_6 only runs on x86 Linux
test_platform('test5_6', Platform) :-
    platform('i386', OS, _, Platform),
    member(OS, ['linux', 'freebsd', 'bluegene']).
mutator('test5_6', ['test5_6.C']).
test_runmode('test5_6', 'staticdynamic').
test_start_state('test5_6', 'stopped').
groupable_test('test5_6').
restricted_amd64_abi('test5_6').
tests_module('test5_6', 'dyninst').

test('test5_7', 'test5_7', 'dyninst_cxx_group_test').
% test5_7 only runs on Linux, Solaris, and Windows
test_platform('test5_7', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_7', ['test5_7.C']).
test_runmode('test5_7', 'staticdynamic').
test_start_state('test5_7', 'stopped').
groupable_test('test5_7').
restricted_amd64_abi('test5_7').
tests_module('test5_7', 'dyninst').
test_exclude_compiler('test5_7', 'pgcxx').

test('test5_8', 'test5_8', 'dyninst_cxx_group_test').
% test5_8 only runs on Linux, Solaris, and Windows
test_platform('test5_8', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_8', ['test5_8.C']).
test_runmode('test5_8', 'staticdynamic').
test_start_state('test5_8', 'stopped').
groupable_test('test5_8').
restricted_amd64_abi('test5_8').
% pgcxx uses non-standard name mangling for templates
tests_module('test5_8', 'dyninst').

test('test5_9', 'test5_9', 'dyninst_cxx_group_test').
% test5_9 only runs on Linus, Solaris, and Windows
test_platform('test5_9', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'windows', 'aix', 'freebsd', 'bluegene']).
mutator('test5_9', ['test5_9.C']).
test_runmode('test5_9', 'staticdynamic').
test_start_state('test5_9', 'stopped').
groupable_test('test5_9').
restricted_amd64_abi('test5_9').
tests_module('test5_9', 'dyninst').
% pgcxx uses non-standard name mangling for templates
test_exclude_compiler('test5_9', 'pgcxx').

% Convenience rule for mapping platforms to the asm sources for test_mem
test_mem_mutatee_aux(P, Aux) :-
    (
        platform('power32', 'aix', _, P) -> Aux = 'test6LS-power.s';
        platform('i386', 'linux', _, P) -> Aux = 'test6LS-x86.asm';
        platform('i386', 'windows', _, P) -> Aux = 'test6LS-masm.asm';
        platform('x86_64', 'linux', _, P) -> Aux = 'test6LS-x86_64.s';
        platform('power32', 'linux', _, P) -> Aux = 'test6LS-powerpc.S';
        platform('power64', 'linux', _, P) -> Aux = 'test6LS-powerpc.S';
        platform('i386', 'freebsd', _, P) -> Aux = 'test6LS-x86.asm';
        platform('x86_64', 'freebsd', _, P) -> Aux = 'test6LS-x86_64.s'
    ).

% Convenience rule for checking platforms for test_mem_*
test_mem_platform(Platform) :-
        platform('power32', 'aix', _, Platform);
        platform('i386', 'linux', _, Platform);
        platform('i386', 'windows', _, Platform);
        platform('x86_64', 'linux', _, Platform);
        platform('i386', 'freebsd', _, Platform);
        platform('x86_64', 'freebsd', _, Platform).

spec_object_file(OFile, 'ibm_as', ['dyninst/test6LS-power.s'], [], [], []) :-
        current_platform(P),
        platform('power32', 'aix', _, P),
        member(OFile, ['test6LS-power_gcc_32_none', 'test6LS-power_gcc_32_low',
                       'test6LS-power_gcc_32_high', 'test6LS-power_gcc_32_max']).

% test_mem_1, formerly test6_1
test('test_mem_1', 'test_mem_1', 'test_mem_1').
% test_mem_1 runs on some specific platforms (asm code)
test_platform('test_mem_1', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_1', ['test_mem_1.C']).
mutatee('test_mem_1', ['test_mem_1_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(P),
    test_mem_mutatee_aux(P, Aux).
compiler_for_mutatee('test_mem_1', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_1', 'staticdynamic').
test_start_state('test_mem_1', 'stopped').
groupable_test('test_mem_1').
restricted_amd64_abi('test_mem_1').
tests_module('test_mem_1', 'dyninst').

% test_mem_2, formerly test6_2
test('test_mem_2', 'test_mem_2', 'test_mem_2').
% test_mem_2 runs on specified platforms (assembly code)
test_platform('test_mem_2', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_2', ['test_mem_2.C']).
mutatee('test_mem_2', ['test_mem_2_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Plat),
    test_mem_mutatee_aux(Plat, Aux).
compiler_for_mutatee('test_mem_2', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_2', 'staticdynamic').
test_start_state('test_mem_2', 'stopped').
groupable_test('test_mem_2').
restricted_amd64_abi('test_mem_2').
groupable_test('test_mem_2').
tests_module('test_mem_2', 'dyninst').

% test_mem_3, formerly test6_3
test('test_mem_3', 'test_mem_3', 'test_mem_3').
test_platform('test_mem_3', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_3', ['test_mem_3.C']).
mutatee('test_mem_3', ['test_mem_3_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Plat),
    test_mem_mutatee_aux(Plat, Aux).
compiler_for_mutatee('test_mem_3', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_3', 'staticdynamic').
test_start_state('test_mem_3', 'stopped').
groupable_test('test_mem_3').
restricted_amd64_abi('test_mem_3').
groupable_test('test_mem_3').
tests_module('test_mem_3', 'dyninst').

% test_mem_4, formerly test6_4
test('test_mem_4', 'test_mem_4', 'test_mem_4').
test_platform('test_mem_4', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_4', ['test_mem_4.C']).
mutatee('test_mem_4', ['test_mem_4_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_4', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_4', 'staticdynamic').
test_start_state('test_mem_4', 'stopped').
groupable_test('test_mem_4').
restricted_amd64_abi('test_mem_4').
groupable_test('test_mem_4').
tests_module('test_mem_4', 'dyninst').

% test_mem_5, formerly test6_5
test('test_mem_5', 'test_mem_5', 'test_mem_5').
test_platform('test_mem_5', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_5', ['test_mem_5.C']).
mutatee('test_mem_5', ['test_mem_5_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_5', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_5', 'staticdynamic').
test_start_state('test_mem_5', 'stopped').
restricted_amd64_abi('test_mem_5').
groupable_test('test_mem_5').
tests_module('test_mem_5', 'dyninst').

% test_mem_6, formerly test6_6
test('test_mem_6', 'test_mem_6', 'test_mem_6').
test_platform('test_mem_6', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_6', ['test_mem_6.C']).
mutatee('test_mem_6', ['test_mem_6_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_6', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_6', 'staticdynamic').
test_start_state('test_mem_6', 'stopped').
groupable_test('test_mem_6').
restricted_amd64_abi('test_mem_6').
tests_module('test_mem_6', 'dyninst').
groupable_test('test_mem_6').

% test_mem_7, formerly test6_7
test('test_mem_7', 'test_mem_7', 'test_mem_7').
test_platform('test_mem_7', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_7', ['test_mem_7.C']).
mutatee('test_mem_7', ['test_mem_7_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_7', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_7', 'staticdynamic').
test_start_state('test_mem_7', 'stopped').
groupable_test('test_mem_7').
restricted_amd64_abi('test_mem_7').
groupable_test('test_mem_7').
tests_module('test_mem_7', 'dyninst').

% test_mem_8, formerly test6_8
test('test_mem_8', 'test_mem_8', 'test_mem_8').
test_platform('test_mem_8', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_8', ['test_mem_8.C']).
mutatee('test_mem_8', ['test_mem_8_mutatee.c', 'test_mem_util.c', Aux]) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_8', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_mem_8', 'staticdynamic').
test_start_state('test_mem_8', 'stopped').
groupable_test('test_mem_8').
restricted_amd64_abi('test_mem_8').
tests_module('test_mem_8', 'dyninst').
groupable_test('test_mem_8').

test('test_fork_5', 'test_fork_5', 'test_fork_5'). % Formerly test7_1
% No fork() on Windows
test_platform('test_fork_5', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_5', ['test_fork_5.C']).
mutatee('test_fork_5', ['test_fork_5_mutatee.c']).
compiler_for_mutatee('test_fork_5', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_5', 'createProcess').
test_start_state('test_fork_5', 'stopped').
tests_module('test_fork_5', 'dyninst').

test('test_fork_6', 'test_fork_6', 'test_fork_6'). % Formerly test7_2
% No fork() on Windows
test_platform('test_fork_6', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_6', ['test_fork_6.C']).
mutatee('test_fork_6', ['test_fork_6_mutatee.c']).
compiler_for_mutatee('test_fork_6', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_6', 'createProcess').
test_start_state('test_fork_6', 'stopped').
tests_module('test_fork_6', 'dyninst').

test('test_fork_7', 'test_fork_7', 'test_fork_7'). % Formerly test7_3
% No fork() on Windows
test_platform('test_fork_7', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_7', ['test_fork_7.C']).
mutatee('test_fork_7', ['test_fork_7_mutatee.c']).
compiler_for_mutatee('test_fork_7', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_7', 'createProcess').
test_start_state('test_fork_7', 'stopped').
tests_module('test_fork_7', 'dyninst').

test('test_fork_8', 'test_fork_8', 'test_fork_8'). % Formerly test7_4
% No fork() on Windows
test_platform('test_fork_8', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_8', ['test_fork_8.C']).
mutatee('test_fork_8', ['test_fork_8_mutatee.c']).
compiler_for_mutatee('test_fork_8', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_8', 'createProcess').
test_start_state('test_fork_8', 'stopped').
tests_module('test_fork_8', 'dyninst').

test('test_fork_9', 'test_fork_9', 'test_fork_9'). % Formerly test7_5
% No fork() on Windows
test_platform('test_fork_9', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_9', ['test_fork_9.C']).
mutatee('test_fork_9', ['test_fork_9_mutatee.c']).
compiler_for_mutatee('test_fork_9', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_9', 'createProcess').
test_start_state('test_fork_9', 'stopped').
tests_module('test_fork_9', 'dyninst').

test('test_fork_10', 'test_fork_10', 'test_fork_10'). % Formerly test7_6
% No fork() on Windows
test_platform('test_fork_10', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_10', ['test_fork_10.C']).
mutatee('test_fork_10', ['test_fork_10_mutatee.c']).
compiler_for_mutatee('test_fork_10', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_10', 'createProcess').
test_start_state('test_fork_10', 'stopped').
tests_module('test_fork_10', 'dyninst').

test('test_fork_11', 'test_fork_11', 'test_fork_11'). % Formerly test7_7
% No fork() on Windows
test_platform('test_fork_11', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_11', ['test_fork_11.C']).
mutatee('test_fork_11', ['test_fork_11_mutatee.c']).
compiler_for_mutatee('test_fork_11', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_11', 'createProcess').
test_start_state('test_fork_11', 'stopped').
tests_module('test_fork_11', 'dyninst').

test('test_fork_12', 'test_fork_12', 'test_fork_12'). % Formerly test7_8
% No fork() on Windows
test_platform('test_fork_12', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_12', ['test_fork_12.C']).
mutatee('test_fork_12', ['test_fork_12_mutatee.c']).
compiler_for_mutatee('test_fork_12', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_12', 'createProcess').
test_start_state('test_fork_12', 'stopped').
tests_module('test_fork_12', 'dyninst').

test('test_fork_13', 'test_fork_13', 'test_fork_13'). % Formerly test7_9
% No fork() on Windows
test_platform('test_fork_13', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_13', ['test_fork_13.C']).
mutatee('test_fork_13', ['test_fork_13_mutatee.c']).
compiler_for_mutatee('test_fork_13', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_13', 'createProcess').
test_start_state('test_fork_13', 'stopped').
tests_module('test_fork_13', 'dyninst').

test('test_fork_14', 'test_fork_14', 'test_fork_14').
% No fork() on Windows
test_platform('test_fork_14', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_14', ['test_fork_14.C']).
mutatee('test_fork_14', ['test_fork_14_mutatee.c']).
compiler_for_mutatee('test_fork_14', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_fork_14', 'createProcess').
test_start_state('test_fork_14', 'stopped').
tests_module('test_fork_14', 'dyninst').

% test_stack_1 (formerly test8_1)
test('test_stack_1', 'test_stack_1', 'test_stack_1').
test_description('test_stack_1', 'Basic getCallStack test').
test_runs_everywhere('test_stack_1').
mutator('test_stack_1', ['test_stack_1.C']).
mutatee('test_stack_1', ['test_stack_1_mutatee.c']).
compiler_for_mutatee('test_stack_1', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_stack_1', 'createProcess').
test_start_state('test_stack_1', 'stopped').
tests_module('test_stack_1', 'dyninst').

% test_stack_2 (formerly test8_2)
test('test_stack_2', 'test_stack_2', 'test_stack_2').
test_description('test_stack_2', 'Test getCallStack when the mutatee stops in a signal handler').
% test_stack_2 doesnt run on Windows
test_platform('test_stack_2', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_stack_2', ['test_stack_2.C']).
mutatee('test_stack_2', ['test_stack_2_mutatee.c']).
compiler_for_mutatee('test_stack_2', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_stack_2', 'createProcess').
test_start_state('test_stack_2', 'stopped').
tests_module('test_stack_2', 'dyninst').

% test_stack_3 (formerly test8_3)
test('test_stack_3', 'test_stack_3', 'test_stack_3').
test_description('test_stack_3', 'Test getCallStack through instrumentation').
test_runs_everywhere('test_stack_3').
mutator('test_stack_3', ['test_stack_3.C']).
mutatee('test_stack_3', ['test_stack_3_mutatee.c']).
compiler_for_mutatee('test_stack_3', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_stack_3', 'createProcess').
test_start_state('test_stack_3', 'stopped').
tests_module('test_stack_3', 'dyninst').

% test_stack_4
test('test_stack_4', 'test_stack_4', 'test_stack_4').
test_description('test_stack_4', 'Test getCallStack through an entry-instrumented signal handler').
% test_stack_4 doesnt run on Windows
test_platform('test_stack_4', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_stack_4', ['test_stack_4.C']).
mutatee('test_stack_4', ['test_stack_4_mutatee.c']).
compiler_for_mutatee('test_stack_4', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_stack_4', 'createProcess').
test_start_state('test_stack_4', 'stopped').
tests_module('test_stack_4', 'dyninst').

% test_callback_1 (formerly test12_2)
test('test_callback_1', 'test_callback_1', 'test_callback_1').
test_description('test_callback_1', 'dynamic callsite callbacks').
% Why doesnt this test run on Windows?
test_platform('test_callback_1', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_callback_1', ['test_callback_1.C']).
mutatee('test_callback_1', ['test_callback_1_mutatee.c']).
compiler_for_mutatee('test_callback_1', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_callback_1', 'createProcess').
test_start_state('test_callback_1', 'stopped').
tests_module('test_callback_1', 'dyninst').

% test_callback_2 (formerly test12_7)
test('test_callback_2', 'test_callback_2', 'test_callback_2').
test_description('test_callback_2', 'user defined message callback -- st').
% Is there really any reason why this test *cant* run on Windows?
test_platform('test_callback_2', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_callback_2', ['test_callback_2.C']).
mutatee('test_callback_2', ['test_callback_2_mutatee.c']).
compiler_for_mutatee('test_callback_2', Compiler) :-
    comp_lang(Compiler, 'c').
test_runmode('test_callback_2', 'createProcess').
test_start_state('test_callback_2', 'stopped').
tests_module('test_callback_2', 'dyninst').

% test_thread_1 (Formerly test12_1)
test('test_thread_1', 'test_thread_1', 'test_thread_1').
test_description('test_thread_1', 'rtlib spinlocks').
% test_thread_* doesnt run on Windows
test_platform('test_thread_1', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_thread_1', ['test_thread_1.C']).
mutatee('test_thread_1', ['test_thread_1_mutatee.c','test_thread.c']).
compiler_for_mutatee('test_thread_1', Compiler) :-
    comp_lang(Compiler, 'c').
% Requires an additional library on Solaris
mutatee_requires_libs('test_thread_1', Libs) :-
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = ['pthread'];
        Libs = ['dl', 'pthread']
    ).
test_runmode('test_thread_1', 'createProcess').
test_start_state('test_thread_1', 'stopped').
tests_module('test_thread_1', 'dyninst').

% test_thread_2 (formerly test12_3)
mutator('test_thread_2', ['test_thread_2.C']).
mutatee('test_thread_2', ['test_thread_2_mutatee.c', 'test_thread.c']).
compiler_for_mutatee('test_thread_2', Compiler) :-
    comp_lang(Compiler, 'c').
% Requires an additional library on Solaris
mutatee_requires_libs('test_thread_2', Libs) :-
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = ['pthread'];
        Libs = ['dl', 'pthread']
    ).
test('test_thread_2', 'test_thread_2', 'test_thread_2').
% Does this test fall under callback tests or thread tests?
test_description('test_thread_2', 'thread create callback').
test_platform('test_thread_2', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_2', 'createProcess').
test_start_state('test_thread_2', 'stopped').
tests_module('test_thread_2', 'dyninst').

% test_thread_3 (formerly test12_4)
mutator('test_thread_3', ['test_thread_3.C']).
mutatee('test_thread_3', ['test_thread_3_mutatee.c', 'test_thread.c']).
compiler_for_mutatee('test_thread_3', Compiler) :-
    comp_lang(Compiler, 'c').
% Requires an additional library on Solaris
mutatee_requires_libs('test_thread_3', Libs) :-
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = ['pthread'];
        Libs = ['dl', 'pthread']
    ).
test('test_thread_3', 'test_thread_3', 'test_thread_3').
test_description('test_thread_3', 'thread create callback - doa').
test_platform('test_thread_3', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_3', 'createProcess').
test_start_state('test_thread_3', 'stopped').
tests_module('test_thread_3', 'dyninst').

% test_thread_5 (formerly test12_8)
mutator('test_thread_5', ['test_thread_5.C']).
mutatee('test_thread_5', ['test_thread_5_mutatee.c', 'test_thread.c']).
compiler_for_mutatee('test_thread_5', Compiler) :-
    comp_lang(Compiler, 'c').
% Requires an additional library on Solaris
mutatee_requires_libs('test_thread_5', Libs) :-
    current_platform(P),
    platform(_, OS, _, P),
    (
        OS = 'freebsd' -> Libs = ['pthread'];
        Libs = ['dl', 'pthread']
    ).
test('test_thread_5', 'test_thread_5', 'test_thread_5').
test_description('test_thread_5', 'user defined message callback -- mt').
test_platform('test_thread_5', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_5', 'createProcess').
test_start_state('test_thread_5', 'stopped').
tests_module('test_thread_5', 'dyninst').

% test_thread_6 (formerly test13_1)
mutator('test_thread_6', ['test_thread_6.C']).
mutatee('test_thread_6', ['test_thread_6_mutatee.c'], ['mutatee_util_mt.c']).
compiler_for_mutatee('test_thread_6', Compiler) :-
    comp_lang(Compiler, 'c').
% Mutatee needs to be linked with libpthread everywhere but on Windows
mutatee_requires_libs('test_thread_6', Libs) :-
    current_platform(Platform),
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Libs = [];
        Libs = ['pthread']
    ).
test('test_thread_6', 'test_thread_6', 'test_thread_6').
test_description('test_thread_6', 'thread create and destroy callbacks?').
test_runs_everywhere('test_thread_6').
test_runmode('test_thread_6', 'dynamic').
test_start_state('test_thread_6', 'delayedattach').
tests_module('test_thread_6', 'dyninst').

% test_thread_7 (formerly test14_1)
mutator('test_thread_7', ['test_thread_7.C']).
mutatee('test_thread_7', ['test_thread_7_mutatee.c'], ['mutatee_util_mt.c']).
compiler_for_mutatee('test_thread_7', Compiler) :-
    comp_lang(Compiler, 'c').
% gcc/g++ with max optimization do some weird recursive call unrolling that
% breaks this test
optimization_for_mutatee('test_thread_7', GNU, Opt) :-
    member(GNU, ['gcc', 'g++']),
    member(Opt, ['none', 'low', 'high']).
optimization_for_mutatee('test_thread_7', NonGNU, Opt) :-
    comp_lang(NonGNU, 'c'),
    \+ member(NonGNU, ['gcc', 'g++']),
    compiler_opt_trans(NonGNU, Opt, _).
% Mutatee needs to be linked with libpthread everywhere but on Windows
mutatee_requires_libs('test_thread_7', Libs) :-
    current_platform(Platform),
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Libs = [];
        Libs = ['pthread']
    ).
test('test_thread_7', 'test_thread_7', 'test_thread_7').
test_description('test_thread_7', 'multithreaded tramp guards').
test_runs_everywhere('test_thread_7').
test_runmode('test_thread_7', 'dynamic').
test_start_state('test_thread_7', 'delayedattach').
tests_module('test_thread_7', 'dyninst').

% test_thread_8 (formerly test15_1)
% This test tests both synchronous and asynchronous one time codes..
mutator('test_thread_8', ['test_thread_8.C']).
mutatee('test_thread_8', ['test_thread_8_mutatee.c'], ['mutatee_util_mt.c']).
compiler_for_mutatee('test_thread_8', Compiler) :-
    comp_lang(Compiler, 'c').
% Mutatee needs to be linked with libpthread everywhere but on Windows
mutatee_requires_libs('test_thread_8', Libs) :-
    current_platform(Platform),
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Libs = [];
        Libs = ['pthread']
    ).
test('test_thread_8', 'test_thread_8', 'test_thread_8').
test_description('test_thread_8', 'thread-specific one time codes').
test_runs_everywhere('test_thread_8').
test_runmode('test_thread_8', 'dynamic').
test_start_state('test_thread_8', 'delayedattach').
tests_module('test_thread_8', 'dyninst').

% The Fortran tests

% convenience clause for C components of Fortran tests
spec_object_file(Object, 'gcc', [], [Source], [], ['-DSOLO_MUTATEE ${MUTATEE_G77_CFLAGS) ']) :-
    fortran_c_component(Testname),
    atom_concat(Testname, '_mutatee_solo_gcc_none', Object),
    atom_concat(Testname, '_solo_me.c', Source).

mutatee('test1_1F', ['test1_1F_mutatee.c'], ['test1_1F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_1F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_1F', Compiler, 'none') :-
    compiler_for_mutatee('test1_1F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_1F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_1F').
%spec_object_file('test1_1F_mutatee_solo_gcc_none', 'gcc',
%   ['test1_1F_solo_me.c'], [], ['${MUTATEE_G77_CFLAGS}']).
% spec_exception('test1_1F_mutatee.c', 'mutatee_flags',
%          ['gcc', '${MUTATEE_G77_CFLAGS}']).
% spec_exception('test1_1F_mutatee.c', 'mutatee_flags',
%          ['g++', '${MUTATEE_G77_CFLAGS}']).
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_1F', 'test1_1', 'test1_1F').
test_description('test1_1F', 'instrument with zero-arg function call (Fortran)').
test_runs_everywhere('test1_1F').
test_runmode('test1_1F', 'dynamic').
test_start_state('test1_1F', 'stopped').
groupable_test('test1_1F').
tests_module('test1_1F', 'dyninst').

mutatee('test1_2F', ['test1_2F_mutatee.c'], ['test1_2F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_2F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_2F', Compiler, 'none') :-
    compiler_for_mutatee('test1_2F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_2F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_2F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_2F', 'test1_2', 'test1_2F').
test_description('test1_2F', 'instrument with four-arg function call (Fortran)').
test_runs_everywhere('test1_2F').
test_runmode('test1_2F', 'dynamic').
test_start_state('test1_2F', 'stopped').
groupable_test('test1_2F').
tests_module('test1_2F', 'dyninst').

mutatee('test1_3F', ['test1_3F_mutatee.c'], ['test1_3F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_3F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_3F', Compiler, 'none') :-
    compiler_for_mutatee('test1_3F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_3F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_3F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_3F', 'test1_3', 'test1_3F').
test_description('test1_3F', 'passing variables to a function (Fortran)').
test_runs_everywhere('test1_3F').
test_runmode('test1_3F', 'dynamic').
test_start_state('test1_3F', 'stopped').
groupable_test('test1_3F').
tests_module('test1_3F', 'dyninst').

mutatee('test1_4F', ['test1_4F_mutatee.c'], ['test1_4F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_4F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_4F', Compiler, 'none') :-
    compiler_for_mutatee('test1_4F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_4F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_4F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_4F', 'test1_4', 'test1_4F').
test_description('test1_4F', 'instrument with a sequence (Fortran)').
test_runs_everywhere('test1_4F').
test_runmode('test1_4F', 'dynamic').
test_start_state('test1_4F', 'stopped').
groupable_test('test1_4F').
tests_module('test1_4F', 'dyninst').

mutatee('test1_5F', ['test1_5F_mutatee.c'], ['test1_5F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_5F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_5F', Compiler, 'none') :-
    compiler_for_mutatee('test1_5F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_5F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_5F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_5F', 'test1_5', 'test1_5F').
test_description('test1_5F', 'instrument with if clause (no else) (Fortran)').
test_runs_everywhere('test1_5F').
test_runmode('test1_5F', 'dynamic').
test_start_state('test1_5F', 'stopped').
groupable_test('test1_5F').
tests_module('test1_5F', 'dyninst').

mutatee('test1_6F', ['test1_6F_mutatee.c'], ['test1_6F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_6F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_6F', Compiler, 'none') :-
    compiler_for_mutatee('test1_6F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_6F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_6F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_6F', 'test1_6', 'test1_6F').
test_description('test1_6F', 'arithmetic operators (Fortran)').
test_runs_everywhere('test1_6F').
test_runmode('test1_6F', 'dynamic').
test_start_state('test1_6F', 'stopped').
groupable_test('test1_6F').
tests_module('test1_6F', 'dyninst').

mutatee('test1_7F', ['test1_7F_mutatee.c'], ['test1_7F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_7F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_7F', Compiler, 'none') :-
    compiler_for_mutatee('test1_7F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_7F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_7F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_7F', 'test1_7', 'test1_7F').
test_description('test1_7F', 'relational operators (Fortran)').
test_runs_everywhere('test1_7F').
test_runmode('test1_7F', 'dynamic').
test_start_state('test1_7F', 'stopped').
groupable_test('test1_7F').
tests_module('test1_7F', 'dyninst').

mutatee('test1_8F', ['test1_8F_mutatee.c'], ['test1_8F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_8F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_8F', Compiler, 'none') :-
    compiler_for_mutatee('test1_8F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_8F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_8F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_8F', 'test1_8', 'test1_8F').
test_description('test1_8F', 'verify that registers are preserved across inserted expression (Fortran)').
test_runs_everywhere('test1_8F').
test_runmode('test1_8F', 'dynamic').
test_start_state('test1_8F', 'stopped').
groupable_test('test1_8F').
tests_module('test1_8F', 'dyninst').

mutatee('test1_9F', ['test1_9F_mutatee.c'], ['test1_9F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_9F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_9F', Compiler, 'none') :-
    compiler_for_mutatee('test1_9F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_9F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_9F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_9F', 'test1_9', 'test1_9F').
test_description('test1_9F', 'verify that registers are preserved across inserted function call (Fortran)').
test_runs_everywhere('test1_9F').
test_runmode('test1_9F', 'dynamic').
test_start_state('test1_9F', 'stopped').
groupable_test('test1_9F').
tests_module('test1_9F', 'dyninst').

mutatee('test1_10F', ['test1_10F_mutatee.c'], ['test1_10F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_10F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_10F', Compiler, 'none') :-
    compiler_for_mutatee('test1_10F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_10F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_10F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_10F', 'test1_10', 'test1_10F').
test_description('test1_10F', 'inserted snippet order (Fortran)').
test_runs_everywhere('test1_10F').
test_runmode('test1_10F', 'dynamic').
test_start_state('test1_10F', 'stopped').
groupable_test('test1_10F').
tests_module('test1_10F', 'dyninst').

mutatee('test1_11F', ['test1_11F_mutatee.c'], ['test1_11F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_11F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_11F', Compiler, 'none') :-
    compiler_for_mutatee('test1_11F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_11F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_11F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_11F', 'test1_11', 'test1_11F').
test_description('test1_11F', 'insert snippets at entry, exit, and call points (Fortran)').
test_runs_everywhere('test1_11F').
test_runmode('test1_11F', 'dynamic').
test_start_state('test1_11F', 'stopped').
groupable_test('test1_11F').
tests_module('test1_11F', 'dyninst').

mutatee('test1_12F', ['test1_12F_mutatee.c'], ['test1_12F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_12F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_12F', Compiler, 'none') :-
    compiler_for_mutatee('test1_12F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_12F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_12F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_12F', 'test1_12', 'test1_12F').
test_description('test1_12F', 'insert/remove and malloc/free (Fortran)').
test_runs_everywhere('test1_12F').
test_runmode('test1_12F', 'dynamic').
test_start_state('test1_12F', 'stopped').
tests_module('test1_12F', 'dyninst').

mutatee('test1_13F', ['test1_13F_mutatee.c'], ['test1_13F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_13F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_13F', Compiler, 'none') :-
    compiler_for_mutatee('test1_13F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_13F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_13F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_13F', 'test1_13', 'test1_13F').
test_description('test1_13F', 'paramExpr,retExpr,nullExpr (Fortran)').
test_runs_everywhere('test1_13F').
test_runmode('test1_13F', 'dynamic').
test_start_state('test1_13F', 'stopped').
groupable_test('test1_13F').
tests_module('test1_13F', 'dyninst').

mutatee('test1_14F', ['test1_14F_mutatee.c'], ['test1_14F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_14F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_14F', Compiler, 'none') :-
    compiler_for_mutatee('test1_14F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_14F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_14F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_14F', 'test1_14', 'test1_14F').
test_description('test1_14F', 'Replace/Remove Function Call (Fortran)').
test_runs_everywhere('test1_14F').
test_runmode('test1_14F', 'dynamic').
test_start_state('test1_14F', 'stopped').
groupable_test('test1_14F').
tests_module('test1_14F', 'dyninst').

%mutatee('test1_15F', ['test1_15F_mutatee.c'], ['test1_15F_fortran.F']).
% TODO Make sure these are correct
%compiler_for_mutatee('test1_15F', Compiler) :-
%    comp_lang(Compiler, 'fortran').
%optimization_for_mutatee('test1_15F', Compiler, 'none') :-
%    compiler_for_mutatee('test1_15F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_15F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
%fortran_c_component('test1_15F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
%test('test1_15F', 'test1_15', 'test1_15F').
%test_description('test1_15F', 'setMutationsActive (Fortran)').
%test_runs_everywhere('test1_15F').
%test_runmode('test1_15F', 'dynamic').
%test_start_state('test1_15F', 'stopped').
%tests_module('test1_15F', 'dyninst').

mutatee('test1_16F', ['test1_16F_mutatee.c'], ['test1_16F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_16F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_16F', Compiler, 'none') :-
    compiler_for_mutatee('test1_16F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_16F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_16F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_16F', 'test1_16', 'test1_16F').
test_description('test1_16F', 'If else (Fortran)').
test_runs_everywhere('test1_16F').
test_runmode('test1_16F', 'dynamic').
test_start_state('test1_16F', 'stopped').
groupable_test('test1_16F').
tests_module('test1_16F', 'dyninst').

mutatee('test1_17F', ['test1_17F_mutatee.c'], ['test1_17F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_17F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_17F', Compiler, 'none') :-
    compiler_for_mutatee('test1_17F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_17F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_17F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_17F', 'test1_17', 'test1_17F').
test_description('test1_17F', 'Verifies that instrumentation inserted at exit point doesn\'t clobber return value (Fortran)').
test_runs_everywhere('test1_17F').
test_runmode('test1_17F', 'dynamic').
test_start_state('test1_17F', 'stopped').
groupable_test('test1_17F').
tests_module('test1_17F', 'dyninst').

mutatee('test1_18F', ['test1_18F_mutatee.c'], ['test1_18F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_18F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_18F', Compiler, 'none') :-
    compiler_for_mutatee('test1_18F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_18F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_18F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_18F', 'test1_18', 'test1_18F').
test_description('test1_18F', 'Read/Write a variable in the mutatee (Fortran)').
test_runs_everywhere('test1_18F').
test_runmode('test1_18F', 'dynamic').
test_start_state('test1_18F', 'stopped').
groupable_test('test1_18F').
tests_module('test1_18F', 'dyninst').

mutatee('test1_19F', ['test1_19F_mutatee.c'], ['test1_19F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_19F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_19F', Compiler, 'none') :-
    compiler_for_mutatee('test1_19F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_19F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_19F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_19F', 'test1_19', 'test1_19F').
test_description('test1_19F', 'oneTimeCode (Fortran)').
test_runs_everywhere('test1_19F').
test_runmode('test1_19F', 'dynamic').
test_start_state('test1_19F', 'stopped').
tests_module('test1_19F', 'dyninst').

mutatee('test1_20F', ['test1_20F_mutatee.c'], ['test1_20F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_20F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_20F', Compiler, 'none') :-
    compiler_for_mutatee('test1_20F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_20F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_20F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_20F', 'test1_20', 'test1_20F').
test_description('test1_20F', 'Instrumentation at arbitrary points (Fortran)').
test_runs_everywhere('test1_20F').
test_runmode('test1_20F', 'dynamic').
test_start_state('test1_20F', 'stopped').
groupable_test('test1_20F').
tests_module('test1_20F', 'dyninst').

mutatee('test1_25F', ['test1_25F_mutatee.c'], ['test1_25F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_25F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_25F', Compiler, 'none') :-
    compiler_for_mutatee('test1_25F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_25F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_25F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_25F', 'test1_25', 'test1_25F').
test_description('test1_25F', 'Unary Operators (Fortran)').
test_runs_everywhere('test1_25F').
test_runmode('test1_25F', 'dynamic').
test_start_state('test1_25F', 'stopped').
groupable_test('test1_25F').
tests_module('test1_25F', 'dyninst').

mutatee('test1_29F', ['test1_29F_mutatee.c'], ['test1_29F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_29F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_29F', Compiler, 'none') :-
    compiler_for_mutatee('test1_29F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_29F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_29F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_29F', 'test1_29', 'test1_29F').
test_description('test1_29F', 'BPatch_srcObj class (Fortran)').
test_runs_everywhere('test1_29F').
test_runmode('test1_29F', 'dynamic').
test_start_state('test1_29F', 'stopped').
groupable_test('test1_29F').
tests_module('test1_29F', 'dyninst').

mutatee('test1_31F', ['test1_31F_mutatee.c'], ['test1_31F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_31F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_31F', Compiler, 'none') :-
    compiler_for_mutatee('test1_31F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_31F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_31F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_31F', 'test1_31', 'test1_31F').
test_description('test1_31F', 'Non-Recursive Base Tramp (Fortran)').
test_runs_everywhere('test1_31F').
test_runmode('test1_31F', 'dynamic').
test_start_state('test1_31F', 'stopped').
groupable_test('test1_31F').
tests_module('test1_31F', 'dyninst').

mutatee('test1_32F', ['test1_32F_mutatee.c'], ['test1_32F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_32F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_32F', Compiler, 'none') :-
    compiler_for_mutatee('test1_32F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_32F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_32F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_32F', 'test1_32', 'test1_32F').
test_description('test1_32F', 'Recursive Base Tramp (Fortran)').
test_runs_everywhere('test1_32F').
test_runmode('test1_32F', 'dynamic').
test_start_state('test1_32F', 'stopped').
groupable_test('test1_32F').
tests_module('test1_32F', 'dyninst').

mutatee('test1_34F', ['test1_34F_mutatee.c'], ['test1_34F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_34F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_34F', Compiler, 'none') :-
    compiler_for_mutatee('test1_34F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_34F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_34F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_34F', 'test1_34', 'test1_34F').
test_description('test1_34F', 'Loop Information (Fortran)').
test_runs_everywhere('test1_34F').
test_runmode('test1_34F', 'dynamic').
test_start_state('test1_34F', 'stopped').
groupable_test('test1_34F').
tests_module('test1_34F', 'dyninst').

mutatee('test1_36F', ['test1_36F_mutatee.c'], ['test1_36F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_36F', Compiler) :-
    comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_36F', Compiler, 'none') :-
    compiler_for_mutatee('test1_36F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  Im doing a hack here around the fact that the Python component
% doesnt know that test1_36F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_36F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_36F', 'test1_36', 'test1_36F').
test_description('test1_36F', 'Callsite Parameter Referencing (Fortran)').
test_runs_everywhere('test1_36F').
test_runmode('test1_36F', 'dynamic').
test_start_state('test1_36F', 'stopped').
groupable_test('test1_36F').
tests_module('test1_36F', 'dyninst').


% SymtabAPI tests

test('test_lookup_func', 'test_lookup_func', 'symtab_group_test').
test_description('test_lookup_func', 'Lookup a single function with SymtabAPI').
test_runs_everywhere('test_lookup_func').
groupable_test('test_lookup_func').
mutator('test_lookup_func', ['test_lookup_func.C']).
test_runmode('test_lookup_func', 'disk').
test_start_state('test_lookup_func', 'stopped').
tests_module('test_lookup_func', 'symtab').
% test_serializable('test_lookup_func').

test('test_lookup_var', 'test_lookup_var', 'symtab_group_test').
test_description('test_lookup_var', 'Lookup a single variable with SymtabAPI').
test_runs_everywhere('test_lookup_var').
groupable_test('test_lookup_var').
mutator('test_lookup_var', ['test_lookup_var.C']).
test_runmode('test_lookup_var', 'disk').
test_start_state('test_lookup_var', 'stopped').
tests_module('test_lookup_var', 'symtab').
% test_serializable('test_lookup_var').

test('test_add_symbols', 'test_add_symbols', 'symtab_group_test').
test_description('test_add_symbols', 'Use SymtabAPI to add symbols to a file').
groupable_test('test_add_symbols').
mutator('test_add_symbols', ['test_add_symbols.C']).
test_runmode('test_add_symbols', 'disk').
test_start_state('test_add_symbols', 'stopped').
tests_module('test_add_symbols', 'symtab').
test_platform('test_add_symbols', Platform) :- rewriteablePlatforms(Platform).
% test_serializable('test_add_symbols').

test('test_line_info', 'test_line_info', 'symtab_group_test').
test_description('test_line_info', 'SymtabAPI Line Information').
test_runs_everywhere('test_line_info').
groupable_test('test_line_info').
mutator('test_line_info', ['test_line_info.C']).
test_runmode('test_line_info', 'disk').
test_start_state('test_line_info', 'stopped').
tests_module('test_line_info', 'symtab').
% test_serializable('test_line_info').

test('test_module', 'test_module', 'symtab_group_test').
test_description('test_module', 'SymtabAPI Module detection & management').
test_runs_everywhere('test_module').
groupable_test('test_module').
mutator('test_module', ['test_module.C']).
test_runmode('test_module', 'disk').
test_start_state('test_module', 'stopped').
tests_module('test_module', 'symtab').
% test_serializable('test_module').

test('test_relocations', 'test_relocations', 'symtab_group_test').
test_description('test_relocations', 'SymtabAPI relocation table parsing').
test_platform('test_relocations', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'freebsd']).
groupable_test('test_relocations').
mutator('test_relocations', ['test_relocations.C']).
test_runmode('test_relocations', 'disk').
test_start_state('test_relocations', 'stopped').
tests_module('test_relocations', 'symtab').
mutatee_requires_libs('symtab_group_test', ['testA']).
% test_serializable('test_relocations').

test('test_type_info', 'test_type_info', 'symtab_group_test').
test_description('test_type_info', 'SymtabAPI Type Information').
test_runs_everywhere('test_type_info').
groupable_test('test_type_info').
mutator('test_type_info', ['test_type_info.C']).
test_runmode('test_type_info', 'disk').
test_start_state('test_type_info', 'stopped').
tests_module('test_type_info', 'symtab').
% test_serializable('test_type_info').

test('test_symtab_ser_funcs', 'test_symtab_ser_funcs', 'symtab_group_test').
test_description('test_symtab_ser_funcs', 'Base SymtabAPI seialization function sanity checks').
test_runs_everywhere('test_symtab_ser_funcs').
groupable_test('test_symtab_ser_funcs').
mutator('test_symtab_ser_funcs', ['test_symtab_ser_funcs.C']).
test_runmode('test_symtab_ser_funcs', 'disk').
test_start_state('test_symtab_ser_funcs', 'stopped').
tests_module('test_symtab_ser_funcs', 'symtab').

test('test_ser_anno', 'test_ser_anno', 'symtab_group_test').
test_description('test_ser_anno', 'Base SymtabAPI seialization function sanity checks').
test_runs_everywhere('test_ser_anno').
groupable_test('test_ser_anno').
mutator('test_ser_anno', ['test_ser_anno.C']).
test_runmode('test_ser_anno', 'disk').
test_start_state('test_ser_anno', 'stopped').
tests_module('test_ser_anno', 'symtab').

% should this really be groupable?
test('test_anno_basic_types', 'test_anno_basic_types', 'symtab_group_test').
test_description('test_anno_basic_types', 'Annotate objects with basic types').
test_runs_everywhere('test_anno_basic_types').
groupable_test('test_anno_basic_types').
mutator('test_anno_basic_types', ['test_anno_basic_types.C']).
test_runmode('test_anno_basic_types', 'disk').
test_start_state('test_anno_basic_types', 'stopped').
tests_module('test_anno_basic_types', 'symtab').

test('test_exception', 'test_exception', 'test_exception').
test_description('test_exception', 'SymtabAPI C++ Exception detection and sanity checks').
groupable_test('test_exception').
mutator('test_exception', ['test_exception.C']).
test_runmode('test_exception', 'disk').
% test_serializable('test_exception').
test_start_state('test_exception', 'stopped').
tests_module('test_exception', 'symtab').
test_platform('test_exception', 'i386-unknown-linux2.4').
test_platform('test_exception', 'i386-unknown-linux2.6').
test_platform('test_exception', 'x86_64-unknown-linux2.4').
mutatee('test_exception', ['test_exception_mutatee.C']).
compiler_for_mutatee('test_exception', Compiler) :-
   member(Compiler, ['g++', 'icpc']).

% instructionAPI tests
test('test_instruction_read_write', 'test_instruction_read_write', none).
test_description('test_instruction_read_write', 'Tests the read & write sets of instructions.').
test_platform('test_instruction_read_write', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('test_instruction_read_write', ['test_instruction_read_write.C']).
test_runmode('test_instruction_read_write', 'disk').
test_start_state('test_instruction_read_write', 'stopped').
tests_module('test_instruction_read_write', 'instruction').

test('test_instruction_farcall', 'test_instruction_farcall', none).
test_description('test_instruction_farcall', 'Tests decoding of far call instructions.').
test_platform('test_instruction_farcall', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('test_instruction_farcall', ['test_instruction_farcall.C']).
test_runmode('test_instruction_farcall', 'disk').
test_start_state('test_instruction_farcall', 'stopped').
tests_module('test_instruction_farcall', 'instruction').

test('test_instruction_bind_eval', 'test_instruction_bind_eval', none).
test_description('test_instruction_bind_eval', 'Tests bind and evaluation mechanism.').
test_platform('test_instruction_bind_eval', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('test_instruction_bind_eval', ['test_instruction_bind_eval.C']).
test_runmode('test_instruction_bind_eval', 'disk').
test_start_state('test_instruction_bind_eval', 'stopped').
tests_module('test_instruction_bind_eval', 'instruction').

test('test_instruction_profile', 'test_instruction_profile', none).
test_description('test_instruction_profile', 'Collect profiling data from decoding 1M bytes of random memory.').
test_platform('test_instruction_profile', Platform) :-
        platform(Platform),
        platform('i386', OS, _, Platform), OS \= 'windows';
        platform('x86_64', OS, _, Platform), OS \= 'windows'.
mutator('test_instruction_profile', ['test_instruction_profile.C']).
test_runmode('test_instruction_profile', 'disk').
test_start_state('test_instruction_profile', 'stopped').
tests_module('test_instruction_profile', 'instruction').
mutator_requires_libs('test_instruction_profile', ['symtabAPI']).

test('power_decode', 'power_decode', none).
test_description('power_decode', 'Tests the read & write sets of POWER instructions.').
test_platform('power_decode', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('power32', _, _, Platform);
        platform('power64', _, _, Platform);
        platform('powerpc', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('power_decode', ['power_decode.C']).
test_runmode('power_decode', 'disk').
test_start_state('power_decode', 'stopped').
tests_module('power_decode', 'instruction').

test('aarch64_decode', 'aarch64_decode', none).
test_description('aarch64_decode', 'Tests the read & write sets of AARCH64 instructions.').
test_platform('aarch64_decode', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('power32', _, _, Platform);
        platform('power64', _, _, Platform);
        platform('powerpc', _, _, Platform);
        platform('aarch64', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('aarch64_decode', ['aarch64_decode.C']).
test_runmode('aarch64_decode', 'disk').
test_start_state('aarch64_decode', 'stopped').
tests_module('aarch64_decode', 'instruction').

test('aarch64_cft', 'aarch64_cft', none).
test_description('aarch64_cft', 'Tests the read & write sets of AARCH64 instructions.').
test_platform('aarch64_cft', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('power32', _, _, Platform);
        platform('power64', _, _, Platform);
        platform('powerpc', _, _, Platform);
        platform('aarch64', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('aarch64_cft', ['aarch64_cft.C']).
test_runmode('aarch64_cft', 'disk').
test_start_state('aarch64_cft', 'stopped').
tests_module('aarch64_cft', 'instruction').

test('aarch64_decode_ldst', 'aarch64_decode_ldst', none).
test_description('aarch64_decode_ldst', 'Tests the read & write sets of AARCH64 load/store instructions.').
test_platform('aarch64_decode_ldst', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('power32', _, _, Platform);
        platform('power64', _, _, Platform);
        platform('powerpc', _, _, Platform);
        platform('aarch64', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('aarch64_decode_ldst', ['aarch64_decode_ldst.C']).
test_runmode('aarch64_decode_ldst', 'disk').
test_start_state('aarch64_decode_ldst', 'stopped').
tests_module('aarch64_decode_ldst', 'instruction').

test('aarch64_simd', 'aarch64_simd', none).
test_description('aarch64_simd', 'Tests the AARCH64 SIMD instructions.').
test_platform('aarch64_simd', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('power32', _, _, Platform);
        platform('power64', _, _, Platform);
        platform('powerpc', _, _, Platform);
        platform('aarch64', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('aarch64_simd', ['aarch64_simd.C']).
test_runmode('aarch64_simd', 'disk').
test_start_state('aarch64_simd', 'stopped').
tests_module('aarch64_simd', 'instruction').

test('power_cft', 'power_cft', none).
test_description('power_cft', 'Tests the control flow targets of POWER instructions.').
test_platform('power_cft', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('power32', _, _, Platform);
        platform('power64', _, _, Platform);
        platform('powerpc', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('power_cft', ['power_cft.C']).
test_runmode('power_cft', 'disk').
test_start_state('power_cft', 'stopped').
tests_module('power_cft', 'instruction').

test('fucompp', 'fucompp', none).
test_description('fucompp', 'Tests the fucompp instruction').
test_platform('fucompp', Platform) :-
        platform(Platform),
        platform('i386', _, _, Platform);
        platform('x86_64', _, _, Platform).
mutator('fucompp', ['fucompp.C']).
test_runmode('fucompp', 'createProcess').
test_start_state('fucompp', 'stopped').
tests_module('fucompp', 'instruction').


test('mov_size_details', 'mov_size_details', none).                                                 
test_description('mov_size_details', 'Tests the sizes of mov AST elements.').                          
test_platform('mov_size_details', Platform) :-                                                                 
        platform(Platform),                                                                                               
        platform('i386', _, _, Platform);                                                                                
        platform('x86_64', _, _, Platform).                                                                              
mutator('mov_size_details', ['mov_size_details.C']).                                                
test_runmode('mov_size_details', 'createProcess').                                                             
test_start_state('mov_size_details', 'stopped').                                                               
tests_module('mov_size_details', 'instruction').                                                             



% ProcessControlAPI Tests
pcPlatforms(P) :- platform(_, 'linux', _, P).
pcPlatforms(P) :- platform(_, 'windows', _, P).
pcPlatforms(P) :- platform('i386', 'freebsd', _,P).
pcPlatforms(P) :- platform('x86_64', 'freebsd', _,P).
pcPlatforms(P) :- platform(_, 'bluegene', _, P).

% ELF platforms
rewriteablePlatforms(P) :- platform(_, 'linux', _, P).
rewriteablePlatforms(P) :- platform(_, 'freebsd', _, P).
%rewriteablePlatforms(P) :- platform(_, 'bluegene', _, P).

pcMutateeLibs(Libs) :-
   current_platform(P),
   platform(_, OS, _, P),
   (
    OS = 'freebsd' -> Libs = ['pthread'];
    OS = 'windows' -> Libs = ['dl', 'pthread', 'ws2_32'];
    Libs = ['dl', 'pthread']
   ).

compiler_for_mutatee(Mutatee, Compiler) :-
    test(T, _, Mutatee),
    \+ member(Mutatee, ['pc_tls']),
    tests_module(T, 'proccontrol'),
    member(Compiler, ['gcc', 'g++', 'VC', 'VC++', 'bg_gcc', 'bg_g++', 'bgq_gcc', 'bgq_g++']).

mutatee_format(Mutatee, 'staticMutatee') :-
    test(T, _, Mutatee),
    \+ member(T, ['pc_library', 'pc_addlibrary', 'pc_fork_exec']),
    tests_module(T, 'proccontrol').

test('pc_launch', 'pc_launch', 'pc_launch').
test_description('pc_launch', 'Launch a process').
test_platform('pc_launch', Platform) :- pcPlatforms(Platform).
mutator('pc_launch', ['pc_launch.C']).
test_runmode('pc_launch', 'dynamic').
test_threadmode('pc_launch', 'Threading').
test_processmode('pc_launch', 'Processes').
test_start_state('pc_launch', 'selfattach').
tests_module('pc_launch', 'proccontrol').
mutatee('pc_launch', ['pc_launch_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_launch', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_launch', _, Opt) :- member(Opt, ['none']).

test('pc_thread_cont', 'pc_thread_cont', 'pc_thread_cont').
test_description('pc_thread_cont', 'Test process running').
test_platform('pc_thread_cont', Platform) :- 
   pcPlatforms(Platform),
   \+ platform(_, 'bluegene', _, Platform).
mutator('pc_thread_cont', ['pc_thread_cont.C']).
test_runmode('pc_thread_cont', 'dynamic').
test_threadmode('pc_thread_cont', 'Threading').
test_processmode('pc_thread_cont', 'Processes').
test_start_state('pc_thread_cont', 'selfattach').
tests_module('pc_thread_cont', 'proccontrol').
mutatee('pc_thread_cont', ['pc_thread_cont_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_thread_cont', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_thread_cont', _, Opt) :- member(Opt, ['none']).

test('pc_breakpoint', 'pc_breakpoint', 'pc_breakpoint').
test_description('pc_breakpoint', 'Test breakpoints').
test_platform('pc_breakpoint', Platform) :- pcPlatforms(Platform).
mutator('pc_breakpoint', ['pc_breakpoint.C']).
test_runmode('pc_breakpoint', 'dynamic').
test_threadmode('pc_breakpoint', 'Threading').
test_processmode('pc_breakpoint', 'Processes').
test_start_state('pc_breakpoint', 'selfattach').
tests_module('pc_breakpoint', 'proccontrol').
mutatee('pc_breakpoint', ['pc_breakpoint_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_breakpoint', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_breakpoint', _, Opt) :- member(Opt, ['none']).

test('pc_hw_breakpoint', 'pc_hw_breakpoint', 'pc_hw_breakpoint').
test_description('pc_hw_breakpoint', 'Test breakpoints').
test_platform('pc_hw_breakpoint', Platform) :- 
   platform(Arch, 'linux', _, Platform),
   member(Arch, ['x86_64', 'i386']).
mutator('pc_hw_breakpoint', ['pc_hw_breakpoint.C']).
test_runmode('pc_hw_breakpoint', 'dynamic').
test_threadmode('pc_hw_breakpoint', 'Threading').
test_processmode('pc_hw_breakpoint', 'Processes').
test_start_state('pc_hw_breakpoint', 'selfattach').
tests_module('pc_hw_breakpoint', 'proccontrol').
mutatee('pc_hw_breakpoint', ['pc_hw_breakpoint_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_hw_breakpoint', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_hw_breakpoint', _, Opt) :- member(Opt, ['none']).

test('pc_library', 'pc_library', 'pc_library').
test_description('pc_library', 'Library loads').
test_platform('pc_library', Platform) :- pcPlatforms(Platform).
mutator('pc_library', ['pc_library.C']).
test_runmode('pc_library', 'dynamic').
test_threadmode('pc_library', 'Threading').
test_processmode('pc_library', 'Processes').
test_start_state('pc_library', 'selfattach').
tests_module('pc_library', 'proccontrol').
mutatee('pc_library', ['pc_library_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_library', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_library', _, Opt) :- member(Opt, ['none']).

test('pc_addlibrary', 'pc_addlibrary', 'pc_addlibrary').
test_description('pc_addlibrary', 'Add Library').
test_platform('pc_addlibrary', Platform) :- pcPlatforms(Platform).
mutator('pc_addlibrary', ['pc_addlibrary.C']).
test_runmode('pc_addlibrary', 'dynamic').
test_threadmode('pc_addlibrary', 'Threading').
test_processmode('pc_addlibrary', 'Processes').
test_start_state('pc_addlibrary', 'selfattach').
tests_module('pc_addlibrary', 'proccontrol').
mutatee('pc_addlibrary', ['pc_addlibrary_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_addlibrary', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_addlibrary', _, Opt) :- member(Opt, ['none']).

test('pc_singlestep', 'pc_singlestep', 'pc_singlestep').
test_description('pc_singlestep', 'Single step').
test_platform('pc_singlestep', Platform) :- pcPlatforms(Platform).
mutator('pc_singlestep', ['pc_singlestep.C']).
test_runmode('pc_singlestep', 'dynamic').
test_threadmode('pc_singlestep', 'Threading').
test_processmode('pc_singlestep', 'Processes').
test_start_state('pc_singlestep', 'selfattach').
tests_module('pc_singlestep', 'proccontrol').
mutatee('pc_singlestep', ['pc_singlestep_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_singlestep', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_singlestep', _, Opt) :- member(Opt, ['none']).

test('pc_thread', 'pc_thread', 'pc_thread').
test_description('pc_thread', 'Thread Info').
test_platform('pc_thread', Platform) :- pcPlatforms(Platform).
mutator('pc_thread', ['pc_thread.C']).
test_runmode('pc_thread', 'dynamic').
test_threadmode('pc_thread', 'Threading').
test_processmode('pc_thread', 'Processes').
test_start_state('pc_thread', 'selfattach').
tests_module('pc_thread', 'proccontrol').
mutatee('pc_thread', ['pc_thread_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_thread', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_thread', _, Opt) :- member(Opt, ['none']).

test('pc_groups', 'pc_groups', 'pc_groups').
test_description('pc_groups', 'Group Operations').
test_platform('pc_groups', Platform) :- pcPlatforms(Platform).
mutator('pc_groups', ['pc_groups.C']).
test_runmode('pc_groups', 'dynamic').
test_threadmode('pc_groups', 'Threading').
test_processmode('pc_groups', 'Processes').
test_start_state('pc_groups', 'selfattach').
tests_module('pc_groups', 'proccontrol').
mutatee('pc_groups', ['pc_groups_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_groups', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_groups', _, Opt) :- member(Opt, ['none']).

test('pc_stat', 'pc_stat', 'pc_stat').
test_description('pc_stat', 'Operations done by STAT').
test_platform('pc_stat', Platform) :- pcPlatforms(Platform).
mutator('pc_stat', ['pc_stat.C']).
test_runmode('pc_stat', 'dynamic').
test_threadmode('pc_stat', 'Threading').
test_processmode('pc_stat', 'Processes').
test_start_state('pc_stat', 'selfattach').
tests_module('pc_stat', 'proccontrol').
mutatee('pc_stat', ['pc_stat_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_stat', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_stat', _, Opt) :- member(Opt, ['none']).

test('pc_fork', 'pc_fork', 'pc_fork').
test_description('pc_fork', 'Fork processes').
% FreeBSD doesn't provide fork events
test_platform('pc_fork', Platform) :- 
    pcPlatforms(Platform),
    platform(_, OS, _, Platform),
    member(OS, ['linux']).
mutator('pc_fork', ['pc_fork.C']).
test_runmode('pc_fork', 'dynamic').
test_threadmode('pc_fork', 'Threading').
test_processmode('pc_fork', 'Processes').
test_start_state('pc_fork', 'selfattach').
tests_module('pc_fork', 'proccontrol').
mutatee('pc_fork', ['pc_fork_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_fork', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_fork', _, Opt) :- member(Opt, ['none']).

test('pc_fork_exec', 'pc_fork_exec', 'pc_fork_exec').
test_description('pc_fork_exec', 'Fork exec processes').
test_platform('pc_fork_exec', Platform) :- 
    pcPlatforms(Platform),
    platform(_, OS, _, Platform),
    member(OS, ['linux']).
mutator('pc_fork_exec', ['pc_fork_exec.C']).
test_runmode('pc_fork_exec', 'dynamic').
test_threadmode('pc_fork_exec', 'Threading').
test_processmode('pc_fork_exec', 'Processes').
test_start_state('pc_fork_exec', 'selfattach').
tests_module('pc_fork_exec', 'proccontrol').
mutatee('pc_fork_exec', ['pc_fork_exec_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_fork_exec', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_fork_exec', _, Opt) :- member(Opt, ['none']).
mutatee('pc_exec_targ', ['pc_exec_targ_mutatee.c']).
mutatee_peer('pc_fork_exec', 'pc_exec_targ').
compiler_for_mutatee('pc_exec_targ', Compiler) :- comp_lang(Compiler, 'c').

test('pc_irpc', 'pc_irpc', 'pc_irpc').
test_description('pc_irpc', 'Run inferior RPCs').
test_platform('pc_irpc', Platform) :- pcPlatforms(Platform).
mutator('pc_irpc', ['pc_irpc.C']).
test_runmode('pc_irpc', 'dynamic').
test_threadmode('pc_irpc', 'Threading').
test_processmode('pc_irpc', 'Processes').
test_start_state('pc_irpc', 'selfattach').
tests_module('pc_irpc', 'proccontrol').
mutatee('pc_irpc', ['pc_irpc_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_irpc', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_irpc', _, Opt) :- member(Opt, ['none']).

test('pc_detach', 'pc_detach', 'pc_detach').
test_description('pc_detach', 'Detach from processes').
test_platform('pc_detach', Platform) :- pcPlatforms(Platform).
mutator('pc_detach', ['pc_detach.C']).
test_runmode('pc_detach', 'dynamic').
test_threadmode('pc_detach', 'Threading').
test_processmode('pc_detach', 'Processes').
test_start_state('pc_detach', 'selfattach').
tests_module('pc_detach', 'proccontrol').
mutatee('pc_detach', ['pc_detach_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_detach', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_detach', _, Opt) :- member(Opt, ['none']).

test('pc_temp_detach', 'pc_temp_detach', 'pc_temp_detach').
test_description('pc_temp_detach', 'Temoprarily detach from processes').
test_platform('pc_temp_detach', Platform) :- pcPlatforms(Platform),
   \+ platform(_, 'bluegene', _, Platform).
mutator('pc_temp_detach', ['pc_temp_detach.C']).
test_runmode('pc_temp_detach', 'dynamic').
test_threadmode('pc_temp_detach', 'Threading').
test_processmode('pc_temp_detach', 'Processes').
test_start_state('pc_temp_detach', 'selfattach').
tests_module('pc_temp_detach', 'proccontrol').
mutatee('pc_temp_detach', ['pc_temp_detach_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_temp_detach', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_temp_detach', _, Opt) :- member(Opt, ['none']).

test('pc_terminate', 'pc_terminate', 'pc_terminate').
test_description('pc_terminate', 'Detach from processes').
test_platform('pc_terminate', Platform) :- pcPlatforms(Platform).
mutator('pc_terminate', ['pc_terminate.C']).
test_runmode('pc_terminate', 'dynamic').
test_threadmode('pc_terminate', 'Threading').
test_processmode('pc_terminate', 'Processes').
test_start_state('pc_terminate', 'selfattach').
tests_module('pc_terminate', 'proccontrol').
mutatee('pc_terminate', ['pc_terminate_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_terminate', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_terminate', _, Opt) :- member(Opt, ['none']).

test('pc_terminate_stopped', 'pc_terminate_stopped', 'pc_terminate_stopped').
test_description('pc_terminate_stopped', 'Detach from processes').
test_platform('pc_terminate_stopped', Platform) :- pcPlatforms(Platform).
mutator('pc_terminate_stopped', ['pc_terminate_stopped.C']).
test_runmode('pc_terminate_stopped', 'dynamic').
test_threadmode('pc_terminate_stopped', 'Threading').
test_processmode('pc_terminate_stopped', 'Processes').
test_start_state('pc_terminate_stopped', 'selfattach').
tests_module('pc_terminate_stopped', 'proccontrol').
mutatee('pc_terminate_stopped', ['pc_terminate_stopped_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_terminate_stopped', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_terminate_stopped', _, Opt) :- member(Opt, ['none']).

test('pc_mem_perm', 'pc_mem_perm', 'pc_mem_perm').
test_description('pc_mem_perm', 'test operations on memory permission').
test_platform('pc_mem_perm', Platform) :- platform(_, 'windows', _, Platform).
mutator('pc_mem_perm', ['pc_mem_perm.C']).
test_runmode('pc_mem_perm', 'dynamic').
test_threadmode('pc_mem_perm', 'Threading').
test_processmode('pc_mem_perm', 'Processes').
test_start_state('pc_mem_perm', 'selfattach').
tests_module('pc_mem_perm', 'proccontrol').
mutatee('pc_mem_perm', ['pc_mem_perm_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_mem_perm', Libs) :- pcMutateeLibs(Libs).
optimization_for_mutatee('pc_mem_perm', _, Opt) :- member(Opt, ['none']).

test('pc_tls', 'pc_tls', 'pc_tls').
test_description('pc_tls', 'Read and write thread local variables').
test_platform('pc_tls', Platform) :- pcPlatforms(Platform),
   \+ platform(_, 'windows', _, Platform).
mutator('pc_tls', ['pc_tls.C']).
test_runmode('pc_tls', 'dynamic').
test_threadmode('pc_tls', 'Threading').
test_processmode('pc_tls', 'Processes').
test_start_state('pc_tls', 'selfattach').
tests_module('pc_tls', 'proccontrol').
mutatee('pc_tls', ['pc_tls_mutatee.c', 'pcontrol_mutatee_tools.c'], ['mutatee_util_mt.c']).
mutatee_requires_libs('pc_tls', Libs) :- pcMutateeLibs(Libs).
compiler_for_mutatee('pc_tls', Compiler) :-
    mutatee_comp(Compiler),
    comp_lang(Compiler, Language),
    member(Language, ['c']),
    \+ member(Compiler, ['pgcc', 'pgcxx']).




% test_start_state/2
% test_start_state(?Test, ?State) specifies that Test should be run with its
% mutatee in state State, with State in {stopped, running, selfstart, selfattach, delayedattach}

% compiler_for_mutatee/2
% compiler_for_mutatee(?Testname, ?Compiler)
% Specifies that the mutatee for the test Testname can be compiled with the
% compiler Compiler.
% If nothing else is specified, a tests mutatee can be compiled with any C
% compiler
% Actually, I dont know how to specify this..
% compiler_for_mutatee(Testname, Compiler) :- ...
    

% Specify libtestSuite
library('testSuite', ['test_lib.C',
                      'test_lib_soExecution.C',
                      'test_lib_mutateeStart.C',
                      'test_lib_test7.C',
                      'test_lib_test9.C',
                      'Process_data.C',
                      'ParameterDict.C',
                      'Callbacks.C',
                      'TestData.C',
                      'TestMutator.C']).
% All mutators require libtestSuite
% mutator_library(Mutator, 'testSuite') :- mutator(Mutator, _).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% PLATFORM SPECIFICATIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% platform/4
% platform(?Arch, ?OS_general, ?OS_specific, ?Platform)
platform('i386', 'linux', 'linux2.4', 'i386-unknown-linux2.4').
platform('i386', 'linux', 'linux2.6', 'i386-unknown-linux2.6').
platform('i386', 'windows', 'nt4.0', 'i386-unknown-nt4.0').
platform('i386', 'windows', 'winXP', 'i386-unknown-winXP').
platform('power32', 'aix', 'aix5.1', 'rs6000-ibm-aix5.1').
platform('power32', 'aix', 'aix5.2', 'rs6000-ibm-aix64-5.2').
platform('x86_64', 'linux', 'linux2.4', 'x86_64-unknown-linux2.4').
platform('x86_64', 'linux', 'cnl', 'x86_64_cnl').
platform('power64', 'linux', 'linux2.6', 'ppc64_linux').
platform('power32', 'linux', 'linux2.6', 'ppc32_linux').
platform('i386', 'freebsd', 'freebsd7.2', 'i386-unknown-freebsd7.2').
platform('x86_64', 'freebsd', 'freebsd7.2', 'amd64-unknown-freebsd7.2').
platform('power32', 'bluegene', 'bluegenep', 'ppc32_bgp_ion').
platform('power32', 'bluegene', 'bluegenel', 'ppc32_bgl_ion').
platform('power32', 'bluegene', 'bluegenep', 'ppc32_bgp').
platform('power64', 'bluegene', 'bluegeneq', 'ppc64_bgq_ion').
platform('aarch64', 'linux', 'linux', 'aarch64-unknown-linux').

% Platform Defns
% platform/1
% Convenience thing.  Mostly used for verifying that a particular platform
% exists.
platform(P) :- platform(_, _, _, P).

% Mutatee ABI specifications
% Were going to try out the new implementation idea here
% NOTE: The parameter_values / whitelise / blacklist system has not been
% implemented yet.
parameter('mutatee_abi').
parameter_values('mutatee_abi', Values) :-
    findall(V, mutatee_abi(V), Values_t),
    sort(Values_t, Values).
mutatee_abi(32).
mutatee_abi(64).

% platform_format (Platform, Format)
platform_format(P, 'dynamicMutatee') :- platform(_, _, S, P),
   S \= 'bluegenel'.
platform_format(P, 'staticMutatee') :- platform(_, O, _, P),
   O \= 'windows'.

% compiler_format (Compiler, Format)
compiler_format(_, 'dynamicMutatee').
% For the time being, static mutatees only built for GNU compilers
compiler_format('g++', 'staticMutatee').
compiler_format('gcc', 'staticMutatee').
compiler_format('gfortran', 'staticMutatee').

compiler_format('bg_gcc', 'staticMutatee').
compiler_format('bg_g++', 'staticMutatee').
compiler_format('bg_gfortran', 'staticMutatee').
compiler_format('bgq_gcc', 'staticMutatee').
compiler_format('bgq_g++', 'staticMutatee').
compiler_format('bgq_gfortran', 'staticMutatee').
% Also XLC for BG, people
compiler_format('bgxlc', 'staticMutatee').
compiler_format('bgxlc++', 'staticMutatee').

% format_runmode (Platform, RunMode, Format)
format_runmode(_, 'binary', 'staticMutatee').
format_runmode(_, 'binary', 'dynamicMutatee').
format_runmode(_, 'createProcess', 'dynamicMutatee').
format_runmode(_, 'useAttach', 'dynamicMutatee').
format_runmode(_, 'disk', 'dynamicMutatee').
format_runmode(_, 'createProcess', 'staticMutatee').
format_runmode(_, 'useAttach', 'staticMutatee').

module_runmode_format('dyninst', 'binary', 'staticMutatee').
module_runmode_format('dyninst', _, 'dynamicMutatee').
module_runmode_format('proccontrol', _, _).
module_runmode_format('instruction', _, _).
module_runmode_format('symtab', _, _).

% Platform ABI support
% Testing out how this looks with whitelist clauses
% NOTE: The parameter_values / whitelise / blacklist system has not been
% implemented yet.
whitelist([['platform', Platform], ['mutatee_abi', ABI]]) :-
    platform_abi(Platform, ABI).

% platform_abi/2
% All platforms support 32-bit mutatees except ia64, ppc64, and freebsd.
platform_abi(Platform, 32) :-
    platform(_, _, _, Platform),
    \+ member(Platform, ['amd64-unknown-freebsd7.2',
                         'ppc64_linux',
                         'ppc64_bgq_ion',
			 'aarch64-unknown-linux']).

% A smaller list of platforms with for 64-bit mutatees
platform_abi('x86_64-unknown-linux2.4', 64).
platform_abi('ppc64_linux', 64).
platform_abi('rs6000-ibm-aix64-5.2', 64).
platform_abi('x86_64_cnl', 64).
platform_abi('amd64-unknown-freebsd7.2', 64).
platform_abi('ppc64_bgq_ion', 64).
platform_abi('aarch64-unknown-linux', 64).

runmode_launch_params(Runmode, Platform, Mutator, Mutatee, Launchtime) :-
   runmode(Runmode),
   remote_runmode_mutator(Runmode, Rmutator),
   (
       \+ remote_platform(Platform) -> Mutator = 'local';
       remote_platform(Platform) -> Mutator = Rmutator
   ),
   remote_runmode_mutatee(Runmode, Rmutatee),
   (
       \+ remote_platform(Platform) -> Mutatee = 'local';
       remote_platform(Platform) -> Mutatee = Rmutatee
   ),
   mutatee_launchtime(Runmode, Launchtime).

remote_platform(P) :- 
   platform(_, OS, _, P),
   member(OS, ['bluegene']).

% Mutator and mutatee run remotely, test will launch mutatee
remote_runmode_mutator('createProcess', 'remote').
remote_runmode_mutatee('createProcess', 'remote').
mutatee_launchtime('createProcess', 'no_launch').

% Mutator and mutatee run remotely, launch mutatee before mutator runs
remote_runmode_mutator('useAttach', 'remote').
remote_runmode_mutatee('useAttach', 'remote').
mutatee_launchtime('useAttach', 'pre').

% Mutator runs locally, mutatee runs on BE.  Launch mutatee after test.
remote_runmode_mutator('binary', 'local').
remote_runmode_mutatee('binary', 'remote').
mutatee_launchtime('binary', 'post').

% Mutator runs locally, no mutatee.
remote_runmode_mutator('disk', 'local').
remote_runmode_mutatee('disk', 'not_run').
mutatee_launchtime('disk', 'no_launch').

% restricted_abi_for_arch(Test, Arch, ABI)
% Limits the test Test to only running with mutatees compiled to ABI on the
% architecture Arch

% restricted_amd64_abi(Test)
% Define restricted_amd64_abi as a convenience clause for
% restricted_abi_for_arch
restricted_abi_for_arch(Test, 'x86_64', 64) :-
        restricted_amd64_abi(Test).

% object_suffix/2
% Specifies the convention used for object files on a platform
object_suffix(Platform, Suffix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Suffix = '.obj';
        Suffix = '.o'
    ).

% executable_suffix/2
% Specifies the convention used for executable files on a platform
executable_suffix(Platform, Suffix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Suffix = '.exe';
        Suffix = ''
    ).

% library_prefix/2
% Specifies the convention used for shared library prefixes on a platform
library_prefix(Platform, Prefix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Prefix = '';
        Prefix = 'lib'
    ).

% library_suffix/2
% Specifies the convention used for shared library suffixes on a platform
library_suffix(Platform, Suffix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Suffix = '.dll';
        Suffix = '.so'
    ).

% Platform Compilers Constraints
% gcc and g++ run on everything but Windows
compiler_platform('gcc', Plat) :- platform(_, OS, _, Plat), OS \= 'windows', OS \= 'bluegene'.
compiler_platform('g++', Plat) :- platform(_, OS, _, Plat), OS \= 'windows', OS \= 'bluegene'.
% gfortran only runs on i386 Linux
compiler_platform('gfortran', 'i386-unknown-linux2.4').
compiler_platform('gfortran', 'i386-unknown-linux2.6').
% Visual C/C++ only runs on Windows
compiler_platform('VC', Plat) :- platform(_, OS, _, Plat), OS == 'windows'.
compiler_platform('VC++', Plat) :- platform(_, OS, _, Plat), OS == 'windows'.
% Portland Group compiler only runs on i386 Linux
compiler_platform('pgcc', Plat) :- platform(Arch, OS, _, Plat), Arch == 'i386', OS == 'linux'.
compiler_platform('pgcxx', Plat) :- platform(Arch, OS, _, Plat), Arch == 'i386', OS == 'linux'.
compiler_platform('pgcc', Plat) :- platform(Arch, OS, _, Plat), Arch == 'x86_64', OS == 'linux'.
compiler_platform('pgcxx', Plat) :- platform(Arch, OS, _, Plat), Arch == 'x86_64', OS == 'linux'.
% AIXs native compilers are xlc & xlC
compiler_platform('xlc', 'rs6000-ibm-aix5.1').
compiler_platform('xlC', 'rs6000-ibm-aix5.1').
% Intel cc on Linux/x86 and Linux/x86_64
compiler_platform('icc', Plat) :- 
    platform(Arch, OS, _, Plat), Arch == 'i386', OS == 'linux'.
compiler_platform('icpc', Plat) :-
    platform(Arch, OS, _, Plat), Arch == 'i386', OS == 'linux'.
compiler_platform('icc', Plat) :- 
    platform(Arch, OS, _, Plat), Arch == 'x86_64', OS == 'linux'.
compiler_platform('icpc', Plat) :-
    platform(Arch, OS, _, Plat), Arch == 'x86_64', OS == 'linux'.

% BlueGene gets its own versions of GNU compilers
compiler_platform('bg_gcc', Plat) :- platform(_, _, 'bluegenep', Plat).
compiler_platform('bg_g++', Plat) :- platform(_, _, 'bluegenep', Plat).
compiler_platform('bg_gfortran', Plat) :- platform(_, _, 'bluegenep', Plat).
compiler_platform('bgq_gcc', Plat) :- platform(_, _, 'bluegeneq', Plat).
compiler_platform('bgq_g++', Plat) :- platform(_, _, 'bluegeneq', Plat).
compiler_platform('bgq_gfortran', Plat) :- platform(_, _, 'bluegeneq', Plat).
mutatee_compiler_platform_exclude('gcc', Plat) :- platform(_, 'bluegene', _, Plat).
mutatee_compiler_platform_exclude('g++', Plat) :- platform(_, 'bluegene', _, Plat).
mutatee_compiler_platform_exclude('gfortran', Plat) :- platform(_, 'bluegene', _, Plat).

% Bluegene xlc ccompilers	 
compiler_platform('bgxlc', Plat) :- platform(_, 'bluegene', _, Plat).
compiler_platform('bgxlc++', Plat) :- platform(_, 'bluegene', _, Plat).

% linker/2
% linker(?Platform, ?Linker)
% specifies the default linker to use for a given (platform, language) pair
linker(Platform, Linker) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Linker = 'link';
        Linker = ''
    ).

% aux_compiler_for_platform/3
% aux_compiler_for_platform(?Platform, ?Language, ?Compiler)
% Specifies the default compiler to use for a given (platform, language) pair
% for compiling files where were not trying to test the output of the compiler
aux_compiler_for_platform(Platform, 'c', 'gcc') :-
    platform(_, 'linux', _, Platform).
aux_compiler_for_platform(Platform, 'fortran', 'gfortran') :-
    platform('i386', 'linux', _, Platform).
aux_compiler_for_platform(Platform, 'nasm_asm', 'nasm') :-
    platform('i386', OS, _, Platform),
    member(OS, ['freebsd', 'linux']).
aux_compiler_for_platform(Platform, 'masm_asm', 'masm') :-
    platform('i386', 'windows', _, Platform).
aux_compiler_for_platform(Platform, 'att_asm', 'gcc') :-
    platform(_, OS, _, Platform),
    % AIX is excluded because both att_asm and power_asm use the '.s' extension
    % and we cant have multiple compilers use the same extension on a platform
    \+ member(OS, ['windows', 'aix', 'bluegene']).
aux_compiler_for_platform(Platform, 'power_asm', 'ibm_as') :-
        platform('power32', 'aix', _, Platform).

% mcomp_plat/2
% mcomp_plat(?Compiler, ?Platform)
% Specifies that Compiler is used to compile mutators on Platform.  Eventually
% well probably support more than one mutator compiler per platform.
% Mutators are compiled with g++ on Unix
mcomp_plat('g++',Plat) :- platform(_, OS, _, Plat), OS \= 'windows'.
% Mutators are compiled with Visual C++ on Windows
mcomp_plat('VC++', Plat) :- platform(_, 'windows', _, Plat).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% LANGUAGE SPECIFICATIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Language Defns
lang('fortran').
lang('c').
lang('c++').
lang('att_asm').
lang('masm_asm').
lang('nasm_asm').
lang('power_asm').

% Language-file extension mappings
lang_ext('fortran', '.F').
lang_ext('c', '.c').
lang_ext('c++', '.C').
lang_ext('att_asm', '.s').
lang_ext('att_asm', '.S').
lang_ext('power_asm', '.s'). % On POWER/AIX
lang_ext('masm_asm', '.asm').
lang_ext('nasm_asm', '.asm').

% lang_ext sanity checking
% No platform should have compilers defined for multiple languages that share
% the same extension.  e.g. POWER/AIX must not have compilers defined for both
% att_asm and power_asm because they both use the '.s' extension for source
% files
insane('Too many compilers on platform P1 for extension P2',
       [Platform, Extension]) :-
    current_platform(Platform),
    compiler_platform(Compiler1, Platform),
    compiler_platform(Compiler2, Platform),
    lang_ext(Language1, Extension),
    lang_ext(Language2, Extension),
    Language1 \= Language2,
    comp_lang(Compiler1, Language1),
    comp_lang(Compiler2, Language2).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% COMPILER SPECIFICATIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Compiler/language constraints
comp_lang('gfortran', 'fortran').
comp_lang(Compiler, 'c') :-
    member(Compiler, ['gcc', 'pgcc', 'VC', 'xlc', 'icc', 'bg_gcc', 'bgq_gcc', 'bgxlc']);
    member(Compiler, ['g++', 'pgcxx', 'VC++', 'xlC', 'icpc', 'bg_g++', 'bgq_g++', 'bgxlc++']).
comp_lang(Compiler, 'c++') :-
    member(Compiler, ['g++', 'pgcxx', 'VC++', 'xlC', 'icpc', 'bg_g++', 'bgq_g++', 'bgxlc++']).
comp_lang('gcc', 'att_asm') :-
    % We dont use gcc for assembly files on AIX
    current_platform(Platform),
    \+ platform(_, 'aix', _, Platform).

% Mutatee Compiler Defns
% mutatee_comp(compiler name)
mutatee_comp('gcc').
mutatee_comp('gfortran').
mutatee_comp('g++').
mutatee_comp('pgcc').
mutatee_comp('pgcxx').
mutatee_comp('VC').
mutatee_comp('VC++').
mutatee_comp('xlc').
mutatee_comp('xlC').
mutatee_comp('icc').
mutatee_comp('icpc').
mutatee_comp('bg_gcc').
mutatee_comp('bg_g++').
mutatee_comp('bg_gfortran').
mutatee_comp('bgq_gcc').
mutatee_comp('bgq_g++').
mutatee_comp('bgq_gfortran').
mutatee_comp('bgxlc').
mutatee_comp('bgxlc++').

% compiler_presence_def/2
% compiler_presence_def(Compiler, EnvironmentVariable)
% Before trying to build anything using Compiler, we need to check whether
% EnvironmentVariable is set.  If it is not set, then we dont have access
% to Compiler.
% FIXME Theres got to be a better way to do this.
compiler_presence_def('pgcc', 'PGI').
compiler_presence_def('pgcxx', 'PGI').
compiler_presence_def('icc', 'ICC').
compiler_presence_def('icpc', 'ICC').
compiler_presence_def('xlc', 'XLC').
compiler_presence_def('xlC', 'XLC').

% Translations between compiler names and compiler #defines
compiler_define_string('gcc', 'gnu_cc').
compiler_define_string('g++', 'gnu_cxx').
compiler_define_string('pgcc', 'pg_cc').
compiler_define_string('VC', 'native_cc').
compiler_define_string('xlc', 'native_cc').
compiler_define_string('pgcxx', 'pg_cxx').
compiler_define_string('VC++', 'native_cc').
compiler_define_string('cxx', 'native_cxx').
compiler_define_string('xlC', 'native_cxx').
compiler_define_string('gfortran', 'gnu_fc').
compiler_define_string('icc', 'intel_cc').
compiler_define_string('icpc', 'intel_CC').
compiler_define_string_32('icc', 'intel_cc_32').
compiler_define_string_32('icpc', 'intel_CC_32').
compiler_define_string('bg_gcc', 'gnu_cc').
compiler_define_string('bg_g++', 'gnu_xx').
compiler_define_string('bg_gfortran', 'gnu_fc').
compiler_define_string('bgq_gcc', 'mpi_cc').
compiler_define_string('bgq_g++', 'mpi_cxx').
compiler_define_string('bgq_gfortran', 'gnu_fc').
compiler_define_string('bgxlc', 'mpi_xlc').
compiler_define_string('bgxlc++', 'mpi_xlcxx').

%%%%%%%%%%
% *_s relations translate various internal atoms into strings than are
% appropriate for writing out to makefiles, etc.
%%%%%%%%%%

% compiler_s/2 translates from the name of a compiler to the executable name
% HACK I think we're already using the executable names as our atoms
compiler_s(Comp, Comp) :-
    % The next line contains a list of compilers whose executable names are
    % different from the name of the compiler that is used in this
    % specification system.
    \+ member(Comp, ['ibm_as', 'masm', 'VC', 'VC++']),
    findall(C, mutatee_comp(C), Me_comp),
    findall(C, mutator_comp(C), Mr_comp),
    findall(C, (member(C, Me_comp); member(C, Mr_comp)), All_comp),
    sort(All_comp, Comps),
    member(Comp, Comps).
compiler_s('VC', 'cl').
compiler_s('VC++', 'cl').
compiler_s('icc', 'icc').
compiler_s('icpc', 'icpc').
compiler_s('bg_gcc', 'powerpc-bgp-linux-gcc').
compiler_s('bg_g++', 'powerpc-bgp-linux-g++').
compiler_s('bg_gfortran', 'powerpc-bgp-linux-gfortran').
compiler_s('bgq_gcc', 'mpicc').
compiler_s('bgq_g++', 'mpic++').
compiler_s('bgq_gfortran', 'mpif90').
compiler_s('pgcxx', 'pgCC').


% Translation for Optimization Level
%compiler_opt_trans(compiler name, symbolic name, compiler argument).
% FIXME(?) I think the Windows optimization strings should be with cap-O, not
% zero
% FIXME Im also not sure that all these compilers default to no optimization
compiler_opt_trans(Comp, 'none', '-g -O0') :- \+ member(Comp, ['VC++', 'VC']).
compiler_opt_trans(Comp, 'low', '-O1') :-
    member(Comp, ['gcc', 'g++', 'pgcc', 'pgcxx', 'gfortran', 'icc', 'icpc', 'bg_gcc', 'bg_g++', 'bg_gfortran', 'bgq_gcc', 'bgq_g++', 'bgq_gfortran']).
compiler_opt_trans(Comp, 'low', '/O1 /MD /Zi /DNDEBUG') :- Comp == 'VC++'; Comp == 'VC'.
compiler_opt_trans(IBM, 'low', '-O') :-
    member(IBM, ['xlc', 'xlC']).
compiler_opt_trans(Comp, 'high', '-O2') :-
    member(Comp, ['gcc', 'g++', 'pgcc', 'pgcxx', 'gfortran', 'icc', 'icpc', 'bg_gcc', 'bg_g++', 'bg_gfortran', 'bgq_gcc', 'bgq_g++', 'bgq_gfortran']).
compiler_opt_trans(Comp, 'high', '/O2 /MD /Zi /DNDEBUG') :- Comp == 'VC++'; Comp == 'VC'.
compiler_opt_trans(IBM, 'high', '-O3') :-
    member(IBM, ['xlc', 'xlC']).
compiler_opt_trans(Comp, 'max', '-O3') :-
    member(Comp, ['gcc', 'g++', 'icc', 'icpc', 'bg_gcc', 'bg_g++', 'bg_gfortran', 'bgq_gcc', 'bgq_g++', 'bgq_gfortran']).
compiler_opt_trans(IBM, 'max', '-O5') :-
    member(IBM, ['xlc', 'xlC']).
compiler_opt_trans(Comp, 'max', '/Ox /MD /Zi /DNDEBUG') :- Comp == 'VC++'; Comp == 'VC'.
compiler_opt_trans(Comp, 'none', '/Od /Zi /MDd /D_DEBUG') :- Comp == 'VC++'; Comp == 'VC'.

compiler_pic_trans(_, 'none', '').
compiler_pic_trans(Comp, 'pic', '-fPIC') :-
    member(Comp, ['gcc', 'g++', 'gfortran', 'icc', 'icpc', 'bg_gcc', 'bg_g++', 'bg_gfortran', 'bgq_gcc', 'bgq_g++', 'bgq_gfortran']).
compiler_pic_trans(Comp, 'pic', '-KPIC') :-
    member(Comp, ['pgcc', 'pgcxx']).
compiler_pic_trans(Comp, 'pic', '-qpic') :-
    member(Comp, ['bgxlc', 'bgxlc++']).
compiler_pic_trans(Comp, 'pic', '') :-
        member(Comp, ['VC++', 'VC']).

compiler_pic('g++', 'pic').
compiler_pic('gcc', 'pic').
compiler_pic('pgcxx', 'pic').
compiler_pic('pgcc', 'pic').
compiler_pic('icpc', 'pic').
compiler_pic('icc', 'pic').
compiler_pic('bgxlc', 'pic').
compiler_pic('bgxlc++', 'pic').
compiler_pic('gfortran', 'pic').
compiler_pic('bg_gcc', 'pic').
compiler_pic('bg_g++', 'pic').
compiler_pic('bg_gfortran', 'pic').
compiler_pic('bgq_gcc', 'pic').
compiler_pic('bgq_g++', 'pic').
compiler_pic('bgq_gfortran', 'pic').
compiler_pic(C, 'none') :-
        mutatee_comp(C).
        
% Ensure that we're only defining translations for compilers that exist
insane('P1 not defined as a compiler, but has optimization translation defined',
       [Compiler]) :-
    compiler_opt_trans(Compiler, _, _),
    \+ compiler_platform(Compiler, _).


% Translation for parameter flags
% partial_compile: compile to an object file rather than an executable
compiler_parm_trans(Comp, 'partial_compile', '-c') :-
    member(Comp, ['gcc', 'g++', 'pgcc', 'pgcxx', 
                  'xlc', 'xlC', 'gfortran', 'VC', 'VC++', 'icc', 'icpc',
                  'bg_gcc', 'bg_g++', 'bg_gfortran', 'bgxlc', 'bgxlc++',
                  'bgq_gcc', 'bgq_g++', 'bgq_gfortran']).

% Mutator compiler defns
mutator_comp('g++').
mutator_comp('pgcxx').
mutator_comp('VC++').
mutator_comp('xlC').
mutator_comp('bgxlc++').

% Per-compiler link options for building mutatees
mutatee_link_options(Gnu_family, '${MUTATEE_LDFLAGS_GNU}') :- member(Gnu_family, ['icc', 'gcc', 'g++', 'icpc', 'bg_gcc', 'bg_g++', 'bgq_gcc', 'bgq_g++']).
mutatee_link_options(Native_cc, '${MUTATEE_CFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}') :-
    member(Native_cc, ['xlc', 'pgcc']).
mutatee_link_options(Native_cxx, '${MUTATEE_CXXFLAGS_NATIVE} ${MUTATEE_LDFLAGS_NATIVE}') :-
    member(Native_cxx, ['xlC', 'pgcxx']).
mutatee_link_options('VC', '${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}').
mutatee_link_options('VC++', '${CMAKE_EXE_LINKER_FLAGS} ${MUTATEE_LDFLAGS_NATIVE}').
mutatee_link_options('bgxlc', '${MUTATEE_LDFLAGS_NATIVE}').
mutatee_link_options('bgxlc++', '${MUTATEE_LDFLAGS_NATIVE}').

% Static and dynamic linking
compiler_static_link('g++', P, '-static') :- platform(_,'linux', _, P).
compiler_static_link('gcc', P, '-static') :- platform(_,'linux', _, P).
compiler_static_link('g++', P, '-static') :- platform(_,'freebsd', _,P).
compiler_static_link('gcc', P, '-static') :- platform(_,'freebsd', _,P).
compiler_static_link('bg_g++', P, '-static') :- platform(_,'bluegene', 'bluegenep', P).
compiler_static_link('bg_gcc', P, '-static') :- platform(_,'bluegene', 'bluegenep', P).
compiler_static_link('bgq_g++', P, '-static') :- platform(_, _, 'bluegeneq', P).
compiler_static_link('bgq_gcc', P, '-static') :- platform(_, _, 'bluegeneq', P).

compiler_dynamic_link('bg_g++', P, '-dynamic -Wl,-export-dynamic') :- platform(_, _, 'bluegenep', P).
compiler_dynamic_link('bg_gcc', P, '-dynamic -Wl,-export-dynamic') :- platform(_, _, 'bluegenep', P).
compiler_dynamic_link('bgq_g++', P, '-dynamic -Wl,-export-dynamic') :- platform(_, _, 'bluegeneq', P).
compiler_dynamic_link('bgq_gcc', P, '-dynamic -Wl,-export-dynamic') :- platform(_, _, 'bluegeneq', P).
compiler_dynamic_link('bgxlc', P, '-qnostaticlink') :- platform(_, _, 'bluegeneq', P).
compiler_dynamic_link('bgxlc++', P, '-qnostaticlink') :- platform(_, _, 'bluegeneq', P).
compiler_dynamic_link('g++', _, '-Wl,-export-dynamic').
compiler_dynamic_link('gcc', _, '-Wl,-export-dynamic').
compiler_dynamic_link('icc', _, '-Xlinker -export-dynamic').
compiler_dynamic_link('icpc', _, '-Xlinker -export-dynamic').

% Specify the standard flags for each compiler
comp_std_flags_str('gcc', '${CFLAGS}').
comp_std_flags_str('g++', '${CXXFLAGS}').
comp_std_flags_str('xlc', '${CFLAGS_NATIVE}').
comp_std_flags_str('pgcc', '${CFLAGS_NATIVE}').
comp_std_flags_str('bgxlc', '${CFLAGS}').
comp_std_flags_str('bgxlc++', '${CXXFLAGS}').
% FIXME Make sure that these flags for cxx are correct, or tear out cxx (Alpha)
comp_std_flags_str('xlC', '${CXXFLAGS_NATIVE}').
comp_std_flags_str('pgcxx', '${CXXFLAGS_NATIVE}').
comp_std_flags_str('bg_gcc', '${CFLAGS}').
comp_std_flags_str('bg_g++', '${CXXFLAGS}').
comp_std_flags_str('bgq_gcc', '${CFLAGS}').
comp_std_flags_str('bgq_g++', '${CXXFLAGS}').

comp_mutatee_flags_str('gcc', '${MUTATEE_CFLAGS_GNU} ').
comp_mutatee_flags_str('g++', '${MUTATEE_CXXFLAGS_GNU} ').
comp_mutatee_flags_str('xlc', '${MUTATEE_CFLAGS_NATIVE} ').
comp_mutatee_flags_str('pgcc', '${MUTATEE_CFLAGS_NATIVE} ').
comp_mutatee_flags_str('bg_gcc', '${MUTATEE_CFLAGS_GNU} ').
comp_mutatee_flags_str('bg_g++', '${MUTATEE_CXXFLAGS_GNU} ').
comp_mutatee_flags_str('bgq_gcc', '${MUTATEE_CFLAGS_GNU} ').
comp_mutatee_flags_str('bgq_g++', '${MUTATEE_CXXFLAGS_GNU} ').
comp_mutatee_flags_str('bgxlc', '${CFLAGS}').
comp_mutatee_flags_str('bgxlc++', '${CXXFLAGS}').
% FIXME Make sure that these flags for cxx are correct, or tear out cxx (Alpha)
comp_mutatee_flags_str('xlC', '${MUTATEE_CXXFLAGS_NATIVE} ').
comp_mutatee_flags_str('pgcxx', '${MUTATEE_CXXFLAGS_NATIVE} ').
% FIXME What do I specify for the Windows compilers, if anything?
comp_std_flags_str('VC', '').
comp_std_flags_str('VC++', '').
comp_mutatee_flags_str('VC', '${MUTATEE_CFLAGS_NATIVE}').
comp_mutatee_flags_str('VC++', '${MUTATEE_CXXFLAGS_NATIVE}').
comp_std_flags_str('icc', '${CFLAGS}').
comp_std_flags_str('icpc', '${CXXFLAGS}').
comp_mutatee_flags_str('icc', '${MUTATEE_CFLAGS_GNU} ').
comp_mutatee_flags_str('icpc', ' ${MUTATEE_CXXFLAGS_GNU}  ').

% gfortran flags
comp_std_flags_str('gfortran', '-g').
comp_mutatee_flags_str('gfortran', '${MUTATEE_G77_FFLAGS}').
mutatee_link_options('gfortran', '${MUTATEE_G77_LDFLAGS}').

% NASM (for test_mem (formerly test6))
comp_lang('nasm', 'nasm_asm').
compiler_define_string('nasm', 'nasm').
compiler_platform('nasm', Platform) :-
    platform('i386', OS, _, Platform), % NASM runs on x86 Linux, FreeBSD
    member(OS, ['freebsd', 'linux']).
comp_std_flags_str('nasm', '-f elf -dPLATFORM=${PLATFORM}').
comp_mutatee_flags_str('nasm', '').
mutatee_link_options('nasm', '').
mutatee_comp('nasm'). % I think I want to reorganize so this isnt required
                      % for compilers that are only used for auxilliary files
compiler_parm_trans('nasm', 'partial_compile', '').

% ml (masm) for test_mem (WINDOWS)
comp_lang('masm', 'masm_asm').
compiler_s('masm', 'ml').
compiler_define_string('masm', 'masm').
compiler_platform('masm', Platform) :-
  platform('i386', 'windows', _, Platform).
comp_std_flags_str('masm', '-nologo').
comp_mutatee_flags_str('masm', '').
mutatee_link_options('masm', '').
mutatee_comp('masm').
compiler_parm_trans('masm', 'partial_compile', '-c').

% as for test_mem
comp_lang('ibm_as', 'power_asm').
compiler_s('ibm_as', 'as').
compiler_define_string('ibm_as', 'ibm_as').
compiler_platform('ibm_as', Platform) :-
    platform('power32', 'aix', _, Platform).
comp_std_flags_str('ibm_as', '').
comp_mutatee_flags_str('ibm_as', '').
mutatee_link_options('ibm_as', '').
mutatee_comp('ibm_as').
compiler_parm_trans('ibm_as', 'partial_compile', '').

% Compiler Optimization Level Defns
optimization_level('none').
optimization_level('low').
optimization_level('high').
optimization_level('max').

% ABI defns for compilers
% compiler_platform_abi_s_default(FlagString)
% The flags string for a platform's default ABI (Do we want this?)
compiler_platform_abi_s_default('').
% compiler_platform_abi_s(Compiler, Platform, ABI, FlagString, CompilerString)
compiler_platform_abi_s(Compiler, Platform, ABI, '', CompilerString) :-
    mutatee_comp(Compiler),
    Compiler \= '',
    platform(Platform),
    compiler_platform(Compiler, Platform),
    mutatee_abi(ABI),
    platform_abi(Platform, ABI),
    compiler_define_string(Compiler, CompilerString),
    \+ ((member(Compiler, ['gcc', 'g++', 'icc', 'icpc', 'pgcc', 'pgcxx']),
         Platform = 'x86_64-unknown-linux2.4',
         ABI = 32);
        (member(Compiler, ['gcc', 'g++']),
         Platform = 'ppc64_linux')).

compiler_platform_abi_s(Compiler, 'x86_64-unknown-linux2.4', 32,
                        '-m32 -Di386_unknown_linux2_4 -Dm32_test', CompilerString) :-
    member(Compiler, ['gcc', 'g++']),
    compiler_define_string(Compiler, CompilerString).
compiler_platform_abi_s(Compiler, 'x86_64-unknown-linux2.4', 32,
                        '-tp px -m32 -Di386_unknown_linux2_4 -Dm32_test', CompilerString) :-
    member(Compiler, ['pgcc', 'pgcxx']),
    compiler_define_string(Compiler, CompilerString).
compiler_platform_abi_s(Compiler, 'x86_64-unknown-linux2.4', 32,
                        '-Di386_unknown_linux2_4 -Dm32_test', CompilerString) :-
    member(Compiler, ['icc', 'icpc']),
    compiler_define_string_32(Compiler, CompilerString).
%
% PPC64 platform doesn't support 32-bit mutatees (yet).
%
%compiler_platform_abi_s(Compiler, 'ppc64_linux', 32,
%                        '-m32 -Dppc32_linux -Dm32_test') :-
%    member(Compiler, ['gcc', 'g++']).
compiler_platform_abi_s(Compiler, 'ppc64_linux', 64,
                        '-m64', CompilerString) :-
    member(Compiler, ['gcc', 'g++']),
    compiler_define_string(Compiler, CompilerString).

compiler_platform_abi(Compiler, Platform, ABI) :-
   mutatee_comp(Compiler),
   platform(Platform),
   compiler_platform(Compiler, Platform),
   mutatee_abi(ABI).
%   \+ (
%      member(Platform, ['x86_64-unknown-linux2.4']),
%      member(Compiler, ['icc', 'icpc', 'pgcc', 'pgcxx']),
%      member(ABI, [32])
%   ).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% TEST SPECIFICATION GLUE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Test / Platform constraints

% test_platform/2
test_platform(Testname, Platform) :-
    test_runs_everywhere(Testname),
    platform(Platform).

% test_runs_everywhere/1
% Specifies the tests that run on all platforms.
% 'none' is a special atom that no test should match that stops the compiler
% from complaining that test_runs_everywhere isnt defined.
test_runs_everywhere(none).

% Test definitions

% test/3
% test(?Name, ?Mutator, ?Mutatee)
% Specifies that the test Name maps the mutator Mutator to the mutatee Mutatee.
% This (hopefully) allows many-to-many mappings to exist, and lets us assign
% a unique name to each mapping.

% Mutator Defns
% mutator/2
% mutator(+name, ?source list)

% Mutators can also require libraries to be linked with.
% All mutators need to be linked with dyninstAPI and iberty
% All mutators also need to be linked with testSuite, for the TestMutator
% superclass if nothing else.
%
% Remove liberty from the prolog--this is determined by autoconf
all_mutators_require_libs(['testSuite']).


module_required_libs('dyninst', ['dyninstAPI']).
module_required_libs('symtab', ['symtabAPI']).
module_required_libs('stackwalker', ['stackwalkerAPI']).
module_required_libs('instruction', ['instructionAPI']).
module_required_libs('proccontrol', ['pcontrol']).

module_requires_libs_internal([], Output, Output).
module_requires_libs_internal([Module | Tail], Acc, Output) :-
	module_required_libs(Module, Libs),
	append(Libs, Acc, NewAcc),
	module_requires_libs_internal(Tail, NewAcc, Output).
module_requires_libs_internal(Module, Acc, Output) :-
	module_required_libs(Module, Libs),
	append(Libs, Acc, Output).

module_requires_libs(Modules, Output) :-
	module_requires_libs_internal(Modules, [], Libs),
	removedups(Libs, Output).

%module_requires_libs([Module | Rest], Output) :-
%	module_required_libs(Module, Libs),
%	\+ module_requires_libs(Rest, [Libs | Output]).
 
% Those with no specific requirements dont require specific libraries
mutator_requires_libs(Mutator, []) :-
	mutator(Mutator, _),
	test(_, Mutator, _),
	\+ mutator_requires_libs(Mutator, [_|_]).

% Mutatee Defns
% mutatee/2
% mutatee(?name, ?source_list)

% mutatee/3
% mutatee(?name, ?preprocessed_source, ?raw_source)
% Specifies a mutatee that uses some auxilliary source files.  The files in
% the preprocessed_source list have the the boilerplate transformation applied
% to them, while the files in the raw_source list are compiled as-is

% Mutatees specified with mutatee/2 have no raw source files.
mutatee(Name, PPSource, []) :- mutatee(Name, PPSource).

% forall_mutatees/1 defines a list of source files requires by all mutatees
% FIXME mutatee_util is hardcoded into the makefile generator.  I need to
% change that.
% FIXME Once I implement group mutatees, this will need to be fixed
forall_mutatees(['mutatee_util.c']).

% I dont think there are any libraries that all mutatees need to be linked to
all_mutatees_require_libs([]).
% mutatee_requires_libs/2
% Allows the user to specify which libraries a mutatee needs to be linked with
% Any mutatees with no specified required libraries dont require any.
mutatee_requires_libs(Mutatee, []) :-
    mutatee(Mutatee, _, _),
    \+ mutatee_requires_libs(Mutatee, [_|_]).
% The multithreaded mutatees are linked with pthreads
%mutatee_requires_libs('test12.mutatee', ['dl', 'pthread']).
%mutatee_requires_libs('test13.mutatee', ['pthread']).
%mutatee_requires_libs('test14.mutatee', ['pthread']).
%mutatee_requires_libs('test15.mutatee', ['pthread']).
% Test 9's mutatee is linked with something, depending on the platform it is
% run on.
% FIXME is libInstMe required on other platforms?
%mutatee_requires_libs('test9.mutatee', L) :-
%    current_platform(P),
%    (
%        P = 'i386-unknown-linux2.4' -> L = ['InstMe'];
%        L = []
%    ).

% Mutatee, Compliers Constraints
% Using compiler_for_mutatee(?Mutatee, ?Compiler) now

% optimization_for_mutatee/3
% optimization_for_mutatee(?Mutatee, ?Compiler, ?OptimizationLevel)
% Unifies a mutatee name with a valid compiler and optimization level for
% building an object file for the mutatee
% TODO Create some kind of default optimization_for_mutatee rule

% threadmode/1
threadmode('None').
threadmode('MultiThreaded').
threadmode('SingleThreaded').
test_threadmode(Test, 'SingleThreaded') :- test_threadmode(Test, 'Threading').
test_threadmode(Test, 'MultiThreaded') :- test_threadmode(Test, 'Threading').
test_threadmode(Test, 'None') :- tests_module(Test, Module),
   module(Module),
   \+ member(Module, ['proccontrol']).

processmode('None').
processmode('MultiProcess').
processmode('SingleProcess').
test_processmode(Test, 'SingleProcess') :- test_processmode(Test, 'Processes').
test_processmode(Test, 'MultiProcess') :- test_processmode(Test, 'Processes').
test_processmode(Test, 'None') :- tests_module(Test, Module),
   module(Module),
   \+ member(Module, ['proccontrol']).

bg_vn_exclude('VN', 'MultiThreaded').

% platform_mode is currently only used by BG/P to specify the modes
% the system can run in: Virtual, Dual, or SMP
platform_mode(P, M, RM, TM) :-
   current_platform(P),
   platform(_, 'bluegene', 'bluegenep', P),
   member(M, ['DUAL', 'VN', 'SMP']),
   member(RM, ['createProcess', 'useAttach', 'binary']),
   \+ bg_vn_exclude(M, TM).

platform_mode(P, 'NONE', 'disk', _) :-
   current_platform(P),
   platform(_, 'bluegene', 'bluegenep', P).

platform_mode(P, 'NONE', _, _) :- 
   current_platform(P),
   \+ platform(_, 'bluegene', 'bluegenep', P).   

% runmode/1
% runmode(+RunMode)
% Specifies the valid values for a test's run mode
runmode('createProcess').
runmode('useAttach').
runmode('binary').
runmode('disk').

% runmode('deserialize').

% mutaee_format/2
% mutatee_format(?Mutatee, ?Format)
% For now, all mutatees compiled dynamically
mutatee_format(_, 'dynamicMutatee').

% test_runmode/2
% test_runmode(?Test, ?Runmode)
% The atom 'dynamic' as Runmode means the test can be run in either useAttach
% or createProcess mode.
test_runmode(Test, 'useAttach') :- test_runmode(Test, 'dynamic').
test_runmode(Test, 'createProcess') :- test_runmode(Test, 'dynamic').
test_runmode(Test, 'binary') :- test_runmode(Test, 'staticdynamic').
test_runmode(Test, 'useAttach') :- test_runmode(Test, 'staticdynamic').
test_runmode(Test, 'createProcess') :- test_runmode(Test, 'staticdynamic').
test_runmode(Test, 'binary') :- test_runmode(Test, 'static').

% test_runmode(Test, 'deserialize') :- test_serializable(Test).

% runmode_platform/3
% runmode_platform(?Platform, ?Runmode, ?Module)
% This specifies what platforms support which runmodes, essentially
% specify binary rewriter support for Dyninst
runmode_platform(P, 'createProcess', 'dyninst') :- platform(_, S, _, P),
  S \= 'bluegene'.
runmode_platform(P, 'useAttach', 'dyninst') :- platform(_, S, _, P),
  S \= 'bluegene'.
runmode_platform(P, 'createProcess', 'proccontrol') :- platform(_, _, _, P).
runmode_platform(P, 'useAttach', 'proccontrol') :- platform(_, _, _, P).
runmode_platform(P, 'createProcess', 'instruction') :- platform(_, _, _, P).
runmode_platform(P, 'binary', _) :- platform('i386', 'linux', _, P).
runmode_platform(P, 'binary', _) :- platform('x86_64', 'linux', _, P).
runmode_platform(P, 'binary', _) :- platform('power32', 'linux', _, P).
runmode_platform(P, 'binary', _) :- platform('power64', 'linux', _, P).
runmode_platform(P, 'binary', _) :- platform('power32', 'bluegene', _, P).
runmode_platform(P, 'binary', _) :- platform('power64', 'bluegene', _, P).
runmode_platform(P, 'binary', _) :- platform('i386', 'freebsd', _, P).
runmode_platform(P, 'binary', _) :- platform('x86_64', 'freebsd', _,P).
% runmode_platform(P, 'binary', _) :- platform('i386', 'windows', _,P).
runmode_platform(P, 'disk', _) :- platform(_, _, _, P).
% runmode_platform(P, 'deserialize', _) :- platform(_, _, _, P).

% mutatee_peers/2
mutatee_peers(M, P) :-
    findall(N, mutatee_peer(M, N), Ps), sort(Ps, P).

mutatee_module('none', 'instruction').
mutatee_module('none', 'dyninst').
mutatee_module(Mutatee, Module) :-
    \+ member(Mutatee, ['none']),
    test(Name, _, Mutatee), !,
    tests_module(Name, Module).

module_pic(Module, _) :-
    \+ member(Module, ['proccontrol']).
module_pic('proccontrol', 'none').

%%%%%
% Playing around with how to specify that some tests only run in 64-bit mode
% on x86_64.
%%%%%
% blacklist([['test', 'test5_1'], ['platform', 'x86_64-unknown-linux2.4'],
%            ['mutatee_abi', '32']]).
%
% I like this one the best:
% blacklist([['test', Test], ['platform', 'x86_64-unknown-linux2.4'],
%            ['mutatee_abi', '32']]) :-
%     restricted_amd64_abi(Test).
%
% test_platform_abi(Test, Platform, ABI) :-
%     test_platform(Test, Platform),
%     platform_abi(Platform, ABI),
%     \+ (Platform = 'x86_64-unknown-linux2.4', ABI = 32)
%
test_platform_abi(Test, Platform, ABI) :-
    test_platform(Test, Platform), platform_abi(Platform, ABI),
        platform(Arch, _, _, Platform),
        (
        % If restricted_abi_for_arch is specified, follow its restrictions
            restricted_abi_for_arch(Test, Arch, ABI);
            % If restricted_abi_for_arch is not specified, there are no
            % restrictions
            \+ restricted_abi_for_arch(Test, Arch, _) -> true
        ).

% Sanity checking
% Define
% insane(Message) :- invalid condition.
% to specify that build should fail if invalid condition is true.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SPEC EXCEPTION GLUE
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% spec_exception_type/3
% spec_exception_type(?ExceptionType, ?ParamCount, ?Params)
% Specifies a new type of exception to a specification rule.  This is where we
% build a catalog of types of exceptions and their parameters

% mutatee_flags exception specifies an exception that overrides the
% standard mutatee flags string for a compiler when compiling a particular
% file.
% OVERRIDES comp_mutatee_flags_str/2
spec_exception_type('mutatee_flags', 2, ['compiler', 'flags']).

% spec_object_file/6
% spec_object_file(?ObjectFile, ?Compiler, ?SourceList, ?IntermediateSourceList
%                  ?DependencyList, ?FlagList)
% This clause should contain everything necessary to generate a makefile rule
% for an object file.  I dont want to do text processing in the Prolog
% component of the spec compiler, so we'll pass along a main source file string
% and let the Python component handle transforming that into an object file
% name

