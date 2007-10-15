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
                  mutator_mutatee/2, comp_mut/2, compiler_platform/2,
                  mcomp_plat/2, test_runmode/2, comp_std_flags_str/2,
                  comp_mutatee_flags_str/2, test_runs_everywhere/1,
                  mutatee_special_make_str/2, mutatee_special_requires/2,
                  test_name/1, groupable_test/1, test_platform/2,
                  compiler_for_mutatee/2, test_start_state/2,
                  compiler_define_string/2, compiler_s/2,
                  mutatee_link_options/2, mutatee_peer/2,
                  compiler_parm_trans/3, test/3, test_description/2,
                  optimization_for_mutatee/3, spec_exception/3,
                  spec_object_file/5, fortran_c_component/1,
				  whitelist/1, parameter/1, parameter_values/2,
				  mutatee_abi/1, platform_abi/2,
				  compiler_platform_abi_s/4, test_platform_abi/3,
				  restricted_amd64_abi/1, compiler_presence_def/2]).

%%%%%%%%%%
%
% SAFE TO EDIT
% It is safe to add new clauses and edit the clauses in the section below
%
%%%%%%%%%%

% Specify the name of the mutator and the source files it uses
%mutator('test.sample', ['test.sample.C']).
% Specify which mutatee this mutator uses
%mutator_mutatee('test.sample', 'test.sample.mutatee').
% Specify which platforms the test runs on
%test_plat('test.sample', Plat) :- platform(Plat).

% For a new mutatee, specify the source files it uses
%mutatee('test.sample.mutatee', ['test.sample.mutatee.c']).
% Specify which compilers the mutatee is compiled with
% (this mutatee is compiled with any C/C++ compiler)
%comp_mut('test.sample.mutatee', Compiler) :-
%    mutatee_comp(Compiler), comp_lang(Compiler, 'c').
% If there are constraints on the runtime modes (attach/create, etc.) specify
% them (this sample only runs in attach mode)
%mutatee_runtime('test.sample.mutatee', 'useAttach').

% Some tests require shared libraries as well as mutatees
%mutator('test.requireslib', ['test.requireslib.C']).
% We need to specify that 'test.requireslib' requires the library 'requireslib'
%mutator_library('test.requireslib', 'requireslib').
% And then we need to provide a definition of the library and the source files
% it reqires if we haven't already done so.
% NOTE TO SELF:
% So we want Windows to compile the following file to 'requireslib.dll' and
% Unix to compile it to 'librequireslib.so'
%library('requireslib', ['requireslib.C']).

% Mutatees can require that they're linked with certain libraries.  Here's
% how to specify that
%mutatee_requires_libs('test.threaded.mutatee', ['pthread']).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Below are specifications for the standard Dyninst test suite
%
% DO NOT EDIT ANYTHING BELOW THIS MARK UNLESS YOU'RE SURE YOU KNOW
% WHAT YOU'RE DOING
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% This is a dummy mutatee definition that's used by some tests that check that
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

% Example test
% test_name('test_skeleton').
% test_runs_everywhere('test_skeleton').
% groupable_test('test_skeleton').
% group_only_test('test_skeleton').
% mutator('test_skeleton', ['test_skeleton.C']).
% mutatee('test_skeleton', ['test_skeleton_mutatee.c']).
% compiler_for_mutatee('test_skeleton', Compiler) :-
%     comp_lang(Compiler, 'c').
% mutator_mutatee('test_skeleton', 'test_skeleton').
% test_runmode('test_skeleton', 'both').
% test_start_state('test_skeleton', 'stopped').

test_name('test1_1').
test_description('test1_1', 'instrument with zero-arg function call').
test_runs_everywhere('test1_1').
groupable_test('test1_1').
mutator('test1_1', ['test1_1.C']).
mutatee('test1_1', ['test1_1_mutatee.c']).
compiler_for_mutatee('test1_1', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_1', 'test1_1').
test_runmode('test1_1', 'both').
test_start_state('test1_1', 'stopped').

test_name('test1_2').
test_description('test1_2', 'instrument with four-arg function call').
test_runs_everywhere('test1_2').
groupable_test('test1_2').
mutator('test1_2', ['test1_2.C']).
mutatee('test1_2', ['test1_2_mutatee.c']).
compiler_for_mutatee('test1_2', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_2', 'test1_2').
test_runmode('test1_2', 'both').
test_start_state('test1_2', 'stopped').

test_name('test1_3').
test_description('test1_3', 'passing variables to a function').
test_runs_everywhere('test1_3').
groupable_test('test1_3').
mutator('test1_3', ['test1_3.C']).
mutatee('test1_3', ['test1_3_mutatee.c']).
compiler_for_mutatee('test1_3', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_3', 'test1_3').
test_runmode('test1_3', 'both').
test_start_state('test1_3', 'stopped').

test_name('test1_4').
test_description('test1_4', 'instrument with a sequence').
test_runs_everywhere('test1_4').
groupable_test('test1_4').
mutator('test1_4', ['test1_4.C']).
mutatee('test1_4', ['test1_4_mutatee.c']).
compiler_for_mutatee('test1_4', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_4', 'test1_4').
test_runmode('test1_4', 'both').
test_start_state('test1_4', 'stopped').

test_name('test1_5').
test_description('test1_5', 'instrument with if clause (no else)').
test_runs_everywhere('test1_5').
groupable_test('test1_5').
mutator('test1_5', ['test1_5.C']).
mutatee('test1_5', ['test1_5_mutatee.c']).
compiler_for_mutatee('test1_5', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_5', 'test1_5').
test_runmode('test1_5', 'both').
test_start_state('test1_5', 'stopped').

test_name('test1_6').
test_description('test1_6', 'arithmetic operators').
test_runs_everywhere('test1_6').
groupable_test('test1_6').
mutator('test1_6', ['test1_6.C']).
mutatee('test1_6', ['test1_6_mutatee.c']).
compiler_for_mutatee('test1_6', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_6', 'test1_6').
test_runmode('test1_6', 'both').
test_start_state('test1_6', 'stopped').

test_name('test1_7').
test_description('test1_7', 'relational operators').
test_runs_everywhere('test1_7').
groupable_test('test1_7').
mutator('test1_7', ['test1_7.C']).
mutatee('test1_7', ['test1_7_mutatee.c']).
compiler_for_mutatee('test1_7', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_7', 'test1_7').
test_runmode('test1_7', 'both').
test_start_state('test1_7', 'stopped').

test_name('test1_8').
test_description('test1_8', 'verify that registers are preserved across inserted expression').
test_runs_everywhere('test1_8').
groupable_test('test1_8').
mutator('test1_8', ['test1_8.C']).
mutatee('test1_8', ['test1_8_mutatee.c']).
compiler_for_mutatee('test1_8', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_8', 'test1_8').
test_runmode('test1_8', 'both').
test_start_state('test1_8', 'stopped').

test_name('test1_9').
test_description('test1_9', 'verify that registers are preserved across inserted function call').
test_runs_everywhere('test1_9').
groupable_test('test1_9').
mutator('test1_9', ['test1_9.C']).
mutatee('test1_9', ['test1_9_mutatee.c']).
compiler_for_mutatee('test1_9', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_9', 'test1_9').
test_runmode('test1_9', 'both').
test_start_state('test1_9', 'stopped').

test_name('test1_10').
test_description('test1_10', 'inserted snippet order').
test_runs_everywhere('test1_10').
groupable_test('test1_10').
mutator('test1_10', ['test1_10.C']).
mutatee('test1_10', ['test1_10_mutatee.c']).
compiler_for_mutatee('test1_10', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_10', 'test1_10').
test_runmode('test1_10', 'both').
test_start_state('test1_10', 'stopped').

test_name('test1_11').
test_description('test1_11', 'insert snippets at entry, exit, and call points').
test_runs_everywhere('test1_11').
groupable_test('test1_11').
mutator('test1_11', ['test1_11.C']).
mutatee('test1_11', ['test1_11_mutatee.c']).
compiler_for_mutatee('test1_11', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_11', 'test1_11').
test_runmode('test1_11', 'both').
test_start_state('test1_11', 'stopped').

test_name('test1_12').
test_description('test1_12', 'insert/remove and malloc/free').
test_runs_everywhere('test1_12').
mutator('test1_12', ['test1_12.C']).
mutatee('test1_12', ['test1_12_mutatee.c']).
% test1_12's mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_12', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_12', 'test1_12').
test_runmode('test1_12', 'both').
test_start_state('test1_12', 'stopped').

test_name('test1_13').
test_description('test1_13', 'paramExpr,retExpr,nullExpr').
test_runs_everywhere('test1_13').
groupable_test('test1_13').
mutator('test1_13', ['test1_13.C']).
mutatee('test1_13', ['test1_13_mutatee.c']).
compiler_for_mutatee('test1_13', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_13', 'test1_13').
test_runmode('test1_13', 'both').
test_start_state('test1_13', 'stopped').

test_name('test1_14').
test_description('test1_14', 'Replace/Remove Function Call').
test_runs_everywhere('test1_14').
groupable_test('test1_14').
mutator('test1_14', ['test1_14.C']).
mutatee('test1_14', ['test1_14_mutatee.c']).
% test1_14's mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_14', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_14', 'test1_14').
test_runmode('test1_14', 'both').
test_start_state('test1_14', 'stopped').

test_name('test1_15').
test_description('test1_15', 'setMutationsActive').
test_runs_everywhere('test1_15').
mutator('test1_15', ['test1_15.C']).
mutatee('test1_15', ['test1_15_mutatee.c']).
% test1_15's mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_15', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_15', 'test1_15').
test_runmode('test1_15', 'both').
test_start_state('test1_15', 'stopped').

test_name('test1_16').
test_description('test1_16', 'If else').
test_runs_everywhere('test1_16').
groupable_test('test1_16').
mutator('test1_16', ['test1_16.C']).
mutatee('test1_16', ['test1_16_mutatee.c']).
% test1_16's mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_16', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_16', 'test1_16').
test_runmode('test1_16', 'both').
test_start_state('test1_16', 'stopped').

test_name('test1_17').
test_description('test1_17', 'Verifies that instrumentation inserted at exit point doesn\'t clobber return value').
test_runs_everywhere('test1_17').
groupable_test('test1_17').
mutator('test1_17', ['test1_17.C']).
mutatee('test1_17', ['test1_17_mutatee.c']).
% test1_17's mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_17', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_17', 'test1_17').
test_runmode('test1_17', 'both').
test_start_state('test1_17', 'stopped').

test_name('test1_18').
test_description('test1_18', 'Read/Write a variable in the mutatee').
test_runs_everywhere('test1_18').
groupable_test('test1_18').
mutator('test1_18', ['test1_18.C']).
mutatee('test1_18', ['test1_18_mutatee.c']).
% test1_18's mutatee can be compiled with any C or Fortran compiler
compiler_for_mutatee('test1_18', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_18', 'test1_18').
test_runmode('test1_18', 'both').
test_start_state('test1_18', 'stopped').

test_name('test1_19').
test_description('test1_19', 'oneTimeCode').
test_runs_everywhere('test1_19').
mutator('test1_19', ['test1_19.C']).
mutatee('test1_19', ['test1_19_mutatee.c']).
% test1_19's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_19', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_19', 'test1_19').
test_runmode('test1_19', 'both').
test_start_state('test1_19', 'stopped').

test_name('test1_20').
test_description('test1_20', 'Instrumentation at arbitrary points').
test_runs_everywhere('test1_20').
groupable_test('test1_20').
mutator('test1_20', ['test1_20.C']).
mutatee('test1_20', ['test1_20_mutatee.c']).
% test1_20's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_20', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_20', 'test1_20').
test_runmode('test1_20', 'both').
test_start_state('test1_20', 'stopped').

test_name('test1_21').
test_description('test1_21', 'findFunction in module').
test_runs_everywhere('test1_21').
groupable_test('test1_21').
mutator('test1_21', ['test1_21.C']).
mutatee('test1_21', ['test1_21_mutatee.c']).
% test1_21's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_21', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_21', 'test1_21').
test_runmode('test1_21', 'both').
test_start_state('test1_21', 'stopped').

test_name('test1_22').
test_description('test1_22', 'Replace Function').
test_runs_everywhere('test1_22').
groupable_test('test1_22').
mutator('test1_22', ['test1_22.C']).
mutatee('test1_22', ['test1_22_mutatee.c']).
% test1_22's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_22', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_22', 'test1_22').
mutatee_requires_libs('test1_22', ['dl']).
test_runmode('test1_22', 'both').
test_start_state('test1_22', 'stopped').

test_name('test1_23').
test_description('test1_23', 'Local Variables').
test_runs_everywhere('test1_23').
groupable_test('test1_23').
mutator('test1_23', ['test1_23.C']).
mutatee('test1_23', ['test1_23_mutatee.c']).
% test1_23's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_23', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_23', 'test1_23').
test_runmode('test1_23', 'both').
test_start_state('test1_23', 'stopped').

test_name('test1_24').
test_description('test1_24', 'Array Variables').
test_runs_everywhere('test1_24').
groupable_test('test1_24').
mutator('test1_24', ['test1_24.C']).
mutatee('test1_24', ['test1_24_mutatee.c']).
% test1_24's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_24', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_24', 'test1_24').
test_runmode('test1_24', 'both').
test_start_state('test1_24', 'stopped').

test_name('test1_25').
test_description('test1_25', 'Unary Operators').
test_runs_everywhere('test1_25').
groupable_test('test1_25').
mutator('test1_25', ['test1_25.C']).
mutatee('test1_25', ['test1_25_mutatee.c']).
% test1_25's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_25', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_25', 'test1_25').
test_runmode('test1_25', 'both').
test_start_state('test1_25', 'stopped').

test_name('test1_26').
test_description('test1_26', 'Struct Elements').
test_runs_everywhere('test1_26').
groupable_test('test1_26').
mutator('test1_26', ['test1_26.C']).
mutatee('test1_26', ['test1_26_mutatee.c']).
% test1_26's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_26', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_26', 'test1_26').
test_runmode('test1_26', 'both').
test_start_state('test1_26', 'stopped').

test_name('test1_27').
test_description('test1_27', 'Type compatibility').
test_runs_everywhere('test1_27').
groupable_test('test1_27').
mutator('test1_27', ['test1_27.C']).
mutatee('test1_27', ['test1_27_mutatee.c']).
% test1_27's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_27', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_27', 'test1_27').
test_runmode('test1_27', 'both').
test_start_state('test1_27', 'stopped').

test_name('test1_28').
test_description('test1_28', 'User Defined Fields').
test_runs_everywhere('test1_28').
groupable_test('test1_28').
mutator('test1_28', ['test1_28.C']).
mutatee('test1_28', ['test1_28_mutatee.c']).
% test1_28's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_28', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_28', 'test1_28').
test_runmode('test1_28', 'both').
test_start_state('test1_28', 'stopped').

test_name('test1_29').
test_description('test1_29', 'BPatch_srcObj class').
test_runs_everywhere('test1_29').
groupable_test('test1_29').
mutator('test1_29', ['test1_29.C']).
mutatee('test1_29', ['test1_29_mutatee.c']).
% test1_29's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_29', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_29', 'test1_29').
test_runmode('test1_29', 'both').
test_start_state('test1_29', 'stopped').

test_name('test1_30').
test_description('test1_30', 'Line Information').
test_runs_everywhere('test1_30').
groupable_test('test1_30').
mutator('test1_30', ['test1_30.C']).
mutatee('test1_30', ['test1_30_mutatee.c']).
% test1_30's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_30', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_30', 'test1_30').
test_runmode('test1_30', 'both').
test_start_state('test1_30', 'stopped').

test_name('test1_31').
test_description('test1_31', 'Non-Recursive Base Tramp').
test_runs_everywhere('test1_31').
groupable_test('test1_31').
mutator('test1_31', ['test1_31.C']).
mutatee('test1_31', ['test1_31_mutatee.c']).
% test1_31's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_31', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_31', 'test1_31').
test_runmode('test1_31', 'both').
test_start_state('test1_31', 'stopped').

test_name('test1_32').
test_description('test1_32', 'Recursive Base Tramp').
test_runs_everywhere('test1_32').
groupable_test('test1_32').
mutator('test1_32', ['test1_32.C']).
mutatee('test1_32', ['test1_32_mutatee.c']).
% test1_32's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_32', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_32', 'test1_32').
test_runmode('test1_32', 'both').
test_start_state('test1_32', 'stopped').

test_name('test1_33').
test_description('test1_33', 'Control Flow Graphs').
test_runs_everywhere('test1_33').
groupable_test('test1_33').
mutator('test1_33', ['test1_33.C']).
mutatee('test1_33', ['test1_33_mutatee.c']).
% test1_33's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_33', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_33', 'test1_33').
test_runmode('test1_33', 'both').
test_start_state('test1_33', 'stopped').

test_name('test1_34').
test_description('test1_34', 'Loop Information').
test_runs_everywhere('test1_34').
groupable_test('test1_34').
mutator('test1_34', ['test1_34.C']).
mutatee('test1_34', ['test1_34_mutatee.c']).
% test1_34's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_34', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_34', 'test1_34').
test_runmode('test1_34', 'both').
test_start_state('test1_34', 'stopped').

test_name('test1_35').
test_description('test1_35', 'Function Relocation').
test_platform('test1_35', 'i386-unknown-linux2.4').
test_platform('test1_35', 'i386-unknown-linux2.6').
test_platform('test1_35', 'x86_64-unknown-linux2.4').
test_platform('test1_35', 'sparc-sun-solaris2.8').
test_platform('test1_35', 'sparc-sun-solaris2.9').
groupable_test('test1_35').
mutator('test1_35', ['test1_35.C']).
% FIXME(?) the following clause wants current_platform/1 to show the platform
% we're compiling specs for, not the platform we're running on.
mutatee('test1_35', ['test1_35_mutatee.c'], Sources) :-
    current_platform(Plat), platform(Arch, OS, _, Plat),
    (
        (Arch = 'sparc', OS = 'solaris') ->
            Sources = ['call35_1_sparc_solaris.s'];
        (Arch = 'x86_64', OS = 'linux') ->
            Sources = ['call35_1_x86_64_linux.s'];
        (Arch = 'i386', OS = 'linux') ->
            Sources = ['call35_1_x86_linux.s'];
        (Arch = 'i386', OS = 'solaris') ->
            Sources = ['call35_1_x86_solaris.s'];
        Sources = ['call35_1.c']
    ).
% test1_35's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_35', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_35', 'test1_35').
test_runmode('test1_35', 'both').
test_start_state('test1_35', 'stopped').

test_name('test1_36').
test_description('test1_36', 'Callsite Parameter Referencing').
test_runs_everywhere('test1_36').
groupable_test('test1_36').
mutator('test1_36', ['test1_36.C']).
mutatee('test1_36', ['test1_36_mutatee.c']).
% test1_36's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_36', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_36', 'test1_36').
test_runmode('test1_36', 'both').
test_start_state('test1_36', 'stopped').

test_name('test1_37').
test_description('test1_37', 'Instrument Loops').
test_runs_everywhere('test1_37').
groupable_test('test1_37').
mutator('test1_37', ['test1_37.C']).
mutatee('test1_37', ['test1_37_mutatee.c']).
% test1_37's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_37', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_37', 'test1_37').
test_runmode('test1_37', 'both').
test_start_state('test1_37', 'stopped').

% FIXME I don't think test1_38 runs on all platforms
test_name('test1_38').
test_description('test1_38', 'CFG Loop Callee Tree').
test_runs_everywhere('test1_38').
groupable_test('test1_38').
mutator('test1_38', ['test1_38.C']).
mutatee('test1_38', ['test1_38_mutatee.c']).
% test1_38's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_38', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_38', 'test1_38').
test_runmode('test1_38', 'both').
test_start_state('test1_38', 'stopped').

test_name('test1_39').
test_description('test1_39', 'Regex search').
% test1_39 doesn't run on Windows
test_platform('test1_39', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
groupable_test('test1_39').
mutator('test1_39', ['test1_39.C']).
mutatee('test1_39', ['test1_39_mutatee.c']).
% test1_39's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_39', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_39', 'test1_39').
test_runmode('test1_39', 'both').
test_start_state('test1_39', 'stopped').

% FIXME I don't think test1_40 runs on all platforms
test_name('test1_40').
test_description('test1_40', 'Verify that we can monitor call sites').
test_runs_everywhere('test1_40').
groupable_test('test1_40').
mutator('test1_40', ['test1_40.C']).
mutatee('test1_40', ['test1_40_mutatee.c']).
% test1_40's mutatee can be compiled with any C compiler or Fortran compiler
compiler_for_mutatee('test1_40', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test1_40', 'test1_40').
test_runmode('test1_40', 'both').
test_start_state('test1_40', 'stopped').

test_name('test1_41').
test_description('test1_41', 'Tests whether we lose line information running a mutatee twice').
% test1_41 doesn't run on Windows
test_platform('test1_41', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test1_41', ['test1_41.C']).
mutatee('test1_41', ['test1_41_mutatee.c']).
compiler_for_mutatee('test1_41', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test1_41', 'test1_41').
test_runmode('test1_41', 'createProcess').
test_start_state('test1_41', 'selfstart').

test_name('test2_1').
test_description('test2_1', 'Run an executable that does not exist').
test_runs_everywhere('test2_1').
mutator('test2_1', ['test2_1.C']).
test('test2_1', 'test2_1', none).
test_runmode('test2_1', 'createProcess').
test_start_state('test2_1', 'selfstart').

test_name('test2_2').
test_description('test2_2', 'Try to run a createProcess on a file that is not an executable file').
test_runs_everywhere('test2_2').
mutator('test2_2', ['test2_2.C']).
test('test2_2', 'test2_2', none).
test_runmode('test2_2', 'createProcess').
test_start_state('test2_2', 'selfstart').

test_name('test2_3').
test_description('test2_3', 'Attatch to an invalid pid').
test_runs_everywhere('test2_3').
mutator('test2_3', ['test2_3.C']).
test('test2_3', 'test2_3', none).
test_runmode('test2_3', 'useAttach').
test_start_state('test2_3', 'selfstart').

test_name('test2_4').
test_description('test2_4', 'Attach to a protected pid').
test_runs_everywhere('test2_4').
mutator('test2_4', ['test2_4.C']).
test('test2_4', 'test2_4', none).
test_runmode('test2_4', 'useAttach').
test_start_state('test2_4', 'selfstart').

test_name('test2_5').
test_description('test2_5', 'Look up nonexistent function').
test_runs_everywhere('test2_5').
groupable_test('test2_5').
mutator('test2_5', ['test2_5.C']).
mutatee('test2_5', ['test2_5_mutatee.c']).
compiler_for_mutatee('test2_5', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test2_5', 'test2_5').
test_runmode('test2_5', 'both').
test_start_state('test2_5', 'stopped').

test_name('test2_6').
test_description('test2_6', 'Load a dynamically linked library from the mutatee').
% test2_6 runs on a somewhat constrained set of platforms
test_platform('test2_6', Platform) :-
    (
        platform('sparc', 'solaris', _, Platform);
        platform('i386', 'linux', _, Platform);
        platform('x86_64', 'linux', _, Platform);
        platform('power', 'aix', _, Platform);
        platform('ia64', 'linux', _, Platform)
    ).
mutator('test2_6', ['test2_6.C']).
mutatee('test2_6', ['test2_6_mutatee.c']).
compiler_for_mutatee('test2_6', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_requires_libs('test2_6', ['dl']).
mutator_mutatee('test2_6', 'test2_6').
test_runmode('test2_6', 'both').
test_start_state('test2_6', 'stopped').

test_name('test2_7').
test_description('test2_7', '').
% test2_7 runs on Solaris, Linux, AIX, and Windows
test_platform('test2_7', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['solaris', 'linux', 'aix', 'windows']).
mutator('test2_7', ['test2_7.C']).
mutatee('test2_7', ['test2_7_mutatee.c']).
compiler_for_mutatee('test2_7', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test2_7', 'test2_7').
test_runmode('test2_7', 'both').
test_start_state('test2_7', 'stopped').
groupable_test('test2_7').

test_name('test2_8').
test_runs_everywhere('test2_8').
mutator('test2_8', ['test2_8.C']).
mutatee('test2_8', ['test2_8_mutatee.c']).
compiler_for_mutatee('test2_8', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test2_8', 'test2_8').
test_runmode('test2_8', 'both').
test_start_state('test2_8', 'stopped').

test_name('test2_9').
% test2_9 only runs on Sparc Solaris
test_platform('test2_9', Platform) :-
    platform('sparc', 'solaris', _, Platform).
mutator('test2_9', ['test2_9.C']).
mutatee('test2_9', ['test2_9_mutatee.c']).
compiler_for_mutatee('test2_9', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test2_9', 'test2_9').
test_runmode('test2_9', 'both'). % Is this correct?
test_start_state('test2_9', 'stopped').

test_name('test2_10').
% test2_10 runs on everything but Windows
test_platform('test2_10', Platform) :-
    platform(Platform), \+ platform(_, 'windows', _, Platform).
mutator('test2_10', ['test2_10.C']).
mutatee('test2_10', ['test2_10_mutatee.c']).
compiler_for_mutatee('test2_10', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test2_10', 'test2_10').
test_runmode('test2_10', 'both').
test_start_state('test2_10', 'stopped').
groupable_test('test2_10').

test_name('test2_11').
test_runs_everywhere('test2_11').
mutator('test2_11', ['test2_11.C']).
mutatee('test2_11', ['test2_11_mutatee.c']).
compiler_for_mutatee('test2_11', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test2_11', 'test2_11').
test_runmode('test2_11', 'both').
test_start_state('test2_11', 'stopped').
groupable_test('test2_11').

test_name('test2_12').
test_runs_everywhere('test2_12').
mutator('test2_12', ['test2_12.C']).
mutatee('test2_12', ['test2_12_mutatee.c']).
compiler_for_mutatee('test2_12', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test2_12', 'test2_12').
test_runmode('test2_12', 'both').
test_start_state('test2_12', 'stopped').
groupable_test('test2_12').

test_name('test2_13').
% test2_13 doesn't run on Alpha, but we're not supporting Alpha any more, so we
% don't need to bother checking for it
test_runs_everywhere('test2_13').
mutator('test2_13', ['test2_13.C']).
mutatee('test2_13', ['test2_13_mutatee.c']).
compiler_for_mutatee('test2_13', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test2_13', 'test2_13').
test_runmode('test2_13', 'both').
test_start_state('test2_13', 'stopped').
groupable_test('test2_13').

test_name('test2_14').
test_runs_everywhere('test2_14').
mutator('test2_14', ['test2_14.C']).
mutatee('test2_14', ['test2_14_mutatee.c']).
compiler_for_mutatee('test2_14', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test2_14', 'test2_14').
test_runmode('test2_14', 'both').
test_start_state('test2_14', 'stopped').

test_name('test3_1').
test_runs_everywhere('test3_1').
mutator('test3_1', ['test3_1.C']).
mutatee('test3_1', ['test3_1_mutatee.c']).
compiler_for_mutatee('test3_1', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test3_1', 'test3_1').
test_runmode('test3_1', 'createProcess').
test_start_state('test3_1', 'selfstart').

test_name('test3_2').
test_runs_everywhere('test3_2').
mutator('test3_2', ['test3_2.C']).
mutatee('test3_2', ['test3_2_mutatee.c']).
compiler_for_mutatee('test3_2', C) :- comp_lang(C, 'c').
mutator_mutatee('test3_2', 'test3_2').
test_runmode('test3_2', 'createProcess').
test_start_state('test3_2', 'selfstart').

test_name('test3_3').
test_runs_everywhere('test3_3').
mutator('test3_3', ['test3_3.C']).
mutatee('test3_3', ['test3_3_mutatee.c']).
compiler_for_mutatee('test3_3', C) :- comp_lang(C, 'c').
mutator_mutatee('test3_3', 'test3_3').
test_runmode('test3_3', 'createProcess').
test_start_state('test3_3', 'selfstart').

test_name('test3_4').
test_runs_everywhere('test3_4').
mutator('test3_4', ['test3_4.C']).
mutatee('test3_4', ['test3_4_mutatee.c']).
compiler_for_mutatee('test3_4', C) :- comp_lang(C, 'c').
mutator_mutatee('test3_4', 'test3_4').
test_runmode('test3_4', 'createProcess').
test_start_state('test3_4', 'selfstart').

test_name('test3_5').
test_runs_everywhere('test3_5').
mutator('test3_5', ['test3_5.C']).
mutatee('test3_5', ['test3_5_mutatee.c']).
compiler_for_mutatee('test3_5', C) :- comp_lang(C, 'c').
mutator_mutatee('test3_5', 'test3_5').
test_runmode('test3_5', 'createProcess').
test_start_state('test3_5', 'selfstart').

test_name('test3_6').
test_description('test3_6', 'simultaneous multiple-process management - terminate (fork)').
% test3_6 doesn't run on Windows
test_platform('test3_6', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test3_6', ['test3_6.C']).
mutatee('test3_6', ['test3_6_mutatee.c']).
compiler_for_mutatee('test3_6', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test3_6', 'test3_6').
test_runmode('test3_6', 'createProcess').
test_start_state('test3_6', 'selfstart').

test_name('test3_7').
test_runs_everywhere('test3_7').
mutator('test3_7', ['test3_7.C']).
mutatee('test3_7', ['test3_7_mutatee.c']).
compiler_for_mutatee('test3_7', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test3_7', 'test3_7').
test_runmode('test3_7', 'createProcess').
test_start_state('test3_7', 'selfstart').

test_name('test4_1').
% Mutator claims test doesn't run on Alpha or Windows, but there were no
% checks to make sure it wasn't running on Windows..
test_runs_everywhere('test4_1').
mutator('test4_1', ['test4_1.C']).
mutatee('test4_1', ['test4_1_mutatee.c']).
compiler_for_mutatee('test4_1', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test4_1', 'test4_1').
test_runmode('test4_1', 'createProcess').
test_start_state('test4_1', 'selfstart').

test_name('test4_2').
% test4_2 doesn't run on Windows
test_platform('test4_2', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test4_2', ['test4_2.C']).
mutatee('test4_2', ['test4_2_mutatee.c']).
compiler_for_mutatee('test4_2', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test4_2', 'test4_2').
test_runmode('test4_2', 'createProcess').
test_start_state('test4_2', 'selfstart').

test_name('test4_3').
% test4_3 doesn't run on Windows
test_platform('test4_3', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test4_3', ['test4_3.C']).
mutatee('test4_3', ['test4_3_mutatee.c']).
compiler_for_mutatee('test4_3', Compiler) :- comp_lang(Compiler, 'c').
mutator_mutatee('test4_3', 'test4_3').
test_runmode('test4_3', 'createProcess').
test_start_state('test4_3', 'selfstart').
mutatee('test4_3b', ['test4_3b_mutatee.c']).
mutatee_peer('test4_3', 'test4_3b').
compiler_for_mutatee('test4_3b', Compiler) :- comp_lang(Compiler, 'c').

test_name('test4_4').
% test4_4 doesn't run on Windows
test_platform('test4_4', Platform) :-
    platform(_, OS, _, Platform), OS \= 'windows'.
mutator('test4_4', ['test4_4.C']).
mutatee('test4_4', ['test4_4_mutatee.c']).
compiler_for_mutatee('test4_4', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test4_4', 'test4_4').
test_runmode('test4_4', 'createProcess').
test_start_state('test4_4', 'selfstart').
mutatee('test4_4b', ['test4_4b_mutatee.c']).
mutatee_peer('test4_4', 'test4_4b').
compiler_for_mutatee('test4_4b', Compiler) :-
    comp_lang(Compiler, 'c').

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
    ((Platform = 'x86_64-unknown-linux2.4',
      restricted_amd64_abi(Test)) -> ABI \= 32;
	 true).

test_name('test5_1').
% test5_1 only runs on Linux, Solaris, and Windows
test_platform('test5_1', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_1', ['test5_1.C']).
mutatee('test5_1', ['test5_1_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_1', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_1', 'test5_1').
test_runmode('test5_1', 'createProcess').
test_start_state('test5_1', 'stopped').
groupable_test('test5_1').
restricted_amd64_abi('test5_1').

test_name('test5_2').
% test5_2 only runs on Linux, Solaris, and Windows
test_platform('test5_2', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_2', ['test5_2.C']).
mutatee('test5_2', ['test5_2_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_2', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_2', 'test5_2').
test_runmode('test5_2', 'createProcess').
test_start_state('test5_2', 'stopped').
groupable_test('test5_2').
restricted_amd64_abi('test5_2').

test_name('test5_3').
test_runs_everywhere('test5_3').
mutator('test5_3', ['test5_3.C']).
mutatee('test5_3', ['test5_3_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_3', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_3', 'test5_3').
test_runmode('test5_3', 'createProcess').
test_start_state('test5_3', 'stopped').
groupable_test('test5_3').
restricted_amd64_abi('test5_3').

test_name('test5_4').
% test5_4 only runs on Linux, Solaris, and Windows
test_platform('test5_4', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_4', ['test5_4.C']).
mutatee('test5_4', ['test5_4_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_4', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_4', 'test5_4').
test_runmode('test5_4', 'createProcess').
test_start_state('test5_4', 'stopped').
groupable_test('test5_4').
restricted_amd64_abi('test5_4').

test_name('test5_5').
% test5_5 only runs on Linux, Solaris, and Windows
test_platform('test5_5', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_5', ['test5_5.C']).
mutatee('test5_5', ['test5_5_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_5', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_5', 'test5_5').
test_runmode('test5_5', 'createProcess').
test_start_state('test5_5', 'stopped').
groupable_test('test5_5').
restricted_amd64_abi('test5_5').

test_name('test5_6').
% test5_6 only runs on x86 Linux
test_platform('test5_6', Platform) :-
    platform('i386', 'linux', _, Platform).
mutator('test5_6', ['test5_6.C']).
mutatee('test5_6', ['test5_6_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_6', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_6', 'test5_6').
test_runmode('test5_6', 'createProcess').
test_start_state('test5_6', 'stopped').
groupable_test('test5_6').
restricted_amd64_abi('test5_6').

test_name('test5_7').
% test5_7 only runs on Linux, Solaris, and Windows
test_platform('test5_7', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_7', ['test5_7.C']).
mutatee('test5_7', ['test5_7_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_7', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_7', 'test5_7').
test_runmode('test5_7', 'createProcess').
test_start_state('test5_7', 'stopped').
groupable_test('test5_7').
restricted_amd64_abi('test5_7').

test_name('test5_8').
% test5_8 only runs on Linux, Solaris, and Windows
test_platform('test5_8', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_8', ['test5_8.C']).
mutatee('test5_8', ['test5_8_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_8', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_8', 'test5_8').
test_runmode('test5_8', 'createProcess').
test_start_state('test5_8', 'stopped').
groupable_test('test5_8').
restricted_amd64_abi('test5_8').

test_name('test5_9').
% test5_9 only runs on Linus, Solaris, and Windows
test_platform('test5_9', Platform) :-
    platform(_, OS, _, Platform),
    member(OS, ['linux', 'solaris', 'windows']).
mutator('test5_9', ['test5_9.C']).
mutatee('test5_9', ['test5_9_mutatee.C'], ['cpp_test.C']).
compiler_for_mutatee('test5_9', Compiler) :-
    comp_lang(Compiler, 'c++').
mutator_mutatee('test5_9', 'test5_9').
test_runmode('test5_9', 'createProcess').
test_start_state('test5_9', 'stopped').
groupable_test('test5_9').
restricted_amd64_abi('test5_9').

% Convenience rule for mapping platforms to the asm sources for test_mem
test_mem_mutatee_aux(P, Aux) :-
    (
        platform('sparc', 'solaris', _, P) -> Aux = ['test_mem_util.c',
                                                     'test6LS-sparc.S'];
        platform('power', 'aix', _, P) -> Aux = ['test_mem_util.c',
                                                 'test6LS-power.s'];
        platform('i386', 'linux', _, P) -> Aux = ['test_mem_util.c',
                                                  'test6LS-x86.asm'];
        platform('i386', 'windows', _, P) -> Aux = ['test_mem_util.c',
                                                    'test6LS-masm.asm'];
        platform('ia64', 'linux', _, P) -> Aux = ['test_mem_util.c',
                                                  'test6LS-ia64.s'];
        platform('x86_64', 'linux', _, P) -> Aux = ['test_mem_util.c',
                                                    'test6LS-x86_64.s'];
        platform('power', 'linux', _, P) -> Aux = ['test_mem_util.c',
                                                   'test6LS-powerpc.S']
    ).

% Convenience rule for checking platforms for test_mem_*
test_mem_platform(Platform) :-
    (
        platform('sparc', 'solaris', _, Platform);
        platform('power', 'aix', _, Platform);
        platform('i386', 'linux', _, Platform);
        platform('i386', 'windows', _, Platform);
        platform('ia64', 'linux', _, Platform);
        platform('x86_64', 'linux', _, Platform)
    ).


test_name('test_mem_1').
% test_mem_1 runs on some specific platforms (asm code)
test_platform('test_mem_1', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_1', ['test_mem_1.C']).
mutatee('test_mem_1', ['test_mem_1_mutatee.c'], Aux) :-
    current_platform(P),
    test_mem_mutatee_aux(P, Aux).
compiler_for_mutatee('test_mem_1', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_1', 'test_mem_1').
test_runmode('test_mem_1', 'createProcess').
test_start_state('test_mem_1', 'stopped').
groupable_test('test_mem_1').
restricted_amd64_abi('test_mem_1').

test_name('test_mem_2').
% test_mem_2 runs on specified platforms (assembly code)
test_platform('test_mem_2', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_2', ['test_mem_2.C']).
mutatee('test_mem_2', ['test_mem_2_mutatee.c'], Aux) :-
    current_platform(Plat),
    test_mem_mutatee_aux(Plat, Aux).
compiler_for_mutatee('test_mem_2', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_2', 'test_mem_2').
test_runmode('test_mem_2', 'createProcess').
test_start_state('test_mem_2', 'stopped').
groupable_test('test_mem_2').
restricted_amd64_abi('test_mem_2').

test_name('test_mem_3').
test_platform('test_mem_3', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_3', ['test_mem_3.C']).
mutatee('test_mem_3', ['test_mem_3_mutatee.c'], Aux) :-
    current_platform(Plat),
    test_mem_mutatee_aux(Plat, Aux).
compiler_for_mutatee('test_mem_3', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_3', 'test_mem_3').
test_runmode('test_mem_3', 'createProcess').
test_start_state('test_mem_3', 'stopped').
groupable_test('test_mem_3').
restricted_amd64_abi('test_mem_3').

test_name('test_mem_4').
test_platform('test_mem_4', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_4', ['test_mem_4.C']).
mutatee('test_mem_4', ['test_mem_4_mutatee.c'], Aux) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_4', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_4', 'test_mem_4').
test_runmode('test_mem_4', 'createProcess').
test_start_state('test_mem_4', 'stopped').
groupable_test('test_mem_4').
restricted_amd64_abi('test_mem_4').

test_name('test_mem_5').
test_platform('test_mem_5', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_5', ['test_mem_5.C']).
mutatee('test_mem_5', ['test_mem_5_mutatee.c'], Aux) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_5', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_5', 'test_mem_5').
test_runmode('test_mem_5', 'createProcess').
test_start_state('test_mem_5', 'stopped').
groupable_test('test_mem_5').
restricted_amd64_abi('test_mem_5').

test_name('test_mem_6').
test_platform('test_mem_6', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_6', ['test_mem_6.C']).
mutatee('test_mem_6', ['test_mem_6_mutatee.c'], Aux) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_6', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_6', 'test_mem_6').
test_runmode('test_mem_6', 'createProcess').
test_start_state('test_mem_6', 'stopped').
groupable_test('test_mem_6').
restricted_amd64_abi('test_mem_6').

test_name('test_mem_7').
test_platform('test_mem_7', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_7', ['test_mem_7.C']).
mutatee('test_mem_7', ['test_mem_7_mutatee.c'], Aux) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_7', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_7', 'test_mem_7').
test_runmode('test_mem_7', 'createProcess').
test_start_state('test_mem_7', 'stopped').
groupable_test('test_mem_7').
restricted_amd64_abi('test_mem_7').

test_name('test_mem_8').
test_platform('test_mem_8', Platform) :-
    test_mem_platform(Platform).
mutator('test_mem_8', ['test_mem_8.C']).
mutatee('test_mem_8', ['test_mem_8_mutatee.c'], Aux) :-
    current_platform(Platform),
    test_mem_mutatee_aux(Platform, Aux).
compiler_for_mutatee('test_mem_8', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_mem_8', 'test_mem_8').
test_runmode('test_mem_8', 'createProcess').
test_start_state('test_mem_8', 'stopped').
groupable_test('test_mem_8').
restricted_amd64_abi('test_mem_8').

test_name('test_fork_5'). % Formerly test7_1
% No fork() on Windows
test_platform('test_fork_5', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_5', ['test_fork_5.C']).
mutatee('test_fork_5', ['test_fork_5_mutatee.c']).
compiler_for_mutatee('test_fork_5', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_5', 'test_fork_5').
test_runmode('test_fork_5', 'createProcess').
test_start_state('test_fork_5', 'stopped').

test_name('test_fork_6'). % Formerly test7_2
% No fork() on Windows
test_platform('test_fork_6', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_6', ['test_fork_6.C']).
mutatee('test_fork_6', ['test_fork_6_mutatee.c']).
compiler_for_mutatee('test_fork_6', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_6', 'test_fork_6').
test_runmode('test_fork_6', 'createProcess').
test_start_state('test_fork_6', 'stopped').

test_name('test_fork_7'). % Formerly test7_3
% No fork() on Windows
test_platform('test_fork_7', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_7', ['test_fork_7.C']).
mutatee('test_fork_7', ['test_fork_7_mutatee.c']).
compiler_for_mutatee('test_fork_7', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_7', 'test_fork_7').
test_runmode('test_fork_7', 'createProcess').
test_start_state('test_fork_7', 'stopped').

test_name('test_fork_8'). % Formerly test7_4
% No fork() on Windows
test_platform('test_fork_8', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_8', ['test_fork_8.C']).
mutatee('test_fork_8', ['test_fork_8_mutatee.c']).
compiler_for_mutatee('test_fork_8', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_8', 'test_fork_8').
test_runmode('test_fork_8', 'createProcess').
test_start_state('test_fork_8', 'stopped').

test_name('test_fork_9'). % Formerly test7_5
% No fork() on Windows
test_platform('test_fork_9', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_9', ['test_fork_9.C']).
mutatee('test_fork_9', ['test_fork_9_mutatee.c']).
compiler_for_mutatee('test_fork_9', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_9', 'test_fork_9').
test_runmode('test_fork_9', 'createProcess').
test_start_state('test_fork_9', 'stopped').

test_name('test_fork_10'). % Formerly test7_6
% No fork() on Windows
test_platform('test_fork_10', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_10', ['test_fork_10.C']).
mutatee('test_fork_10', ['test_fork_10_mutatee.c']).
compiler_for_mutatee('test_fork_10', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_10', 'test_fork_10').
test_runmode('test_fork_10', 'createProcess').
test_start_state('test_fork_10', 'stopped').

test_name('test_fork_11'). % Formerly test7_7
% No fork() on Windows
test_platform('test_fork_11', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
mutator('test_fork_11', ['test_fork_11.C']).
mutatee('test_fork_11', ['test_fork_11_mutatee.c']).
compiler_for_mutatee('test_fork_11', Compiler) :-
    comp_lang(Compiler, 'c').
mutator_mutatee('test_fork_11', 'test_fork_11').
test_runmode('test_fork_11', 'createProcess').
test_start_state('test_fork_11', 'stopped').

mutator('test_fork_12', ['test_fork_12.C']).
mutatee('test_fork_12', ['test_fork_12_mutatee.c']).
compiler_for_mutatee('test_fork_12', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_fork_12', 'test_fork_12', 'test_fork_12'). % Formerly test7_8
% No fork() on Windows
test_platform('test_fork_12', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_fork_12', 'createProcess').
test_start_state('test_fork_12', 'stopped').

mutator('test_fork_13', ['test_fork_13.C']).
mutatee('test_fork_13', ['test_fork_13_mutatee.c']).
compiler_for_mutatee('test_fork_13', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_fork_13', 'test_fork_13', 'test_fork_13'). % Formerly test7_9
% No fork() on Windows
test_platform('test_fork_13', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_fork_13', 'createProcess').
test_start_state('test_fork_13', 'stopped').

% test_stack_1 (formerly test8_1)
mutator('test_stack_1', ['test_stack_1.C']).
mutatee('test_stack_1', ['test_stack_1_mutatee.c']).
compiler_for_mutatee('test_stack_1', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_stack_1', 'test_stack_1', 'test_stack_1').
test_description('test_stack_1', 'Basic getCallStack test').
test_runs_everywhere('test_stack_1').
test_runmode('test_stack_1', 'createProcess').
test_start_state('test_stack_1', 'stopped').

% test_stack_2 (formerly test8_2)
mutator('test_stack_2', ['test_stack_2.C']).
mutatee('test_stack_2', ['test_stack_2_mutatee.c']).
compiler_for_mutatee('test_stack_2', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_stack_2', 'test_stack_2', 'test_stack_2').
test_description('test_stack_2', 'Test getCallStack when the mutatee stops in a signal handler').
% test_stack_2 doesn't run on Windows
test_platform('test_stack_2', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_stack_2', 'createProcess').
test_start_state('test_stack_2', 'stopped').

% test_stack_3 (formerly test8_3)
mutator('test_stack_3', ['test_stack_3.C']).
mutatee('test_stack_3', ['test_stack_3_mutatee.c']).
compiler_for_mutatee('test_stack_3', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_stack_3', 'test_stack_3', 'test_stack_3').
test_description('test_stack_3', 'Test getCallStack through instrumentation').
test_runs_everywhere('test_stack_3').
test_runmode('test_stack_3', 'createProcess').
test_start_state('test_stack_3', 'stopped').

%mutator('test_stw_1', ['test_stw_1.C', 'test_stw.C']).
%mutatee('test_stw_1', ['test_stw_1_mutatee.c']).
%compiler_for_mutatee('test_stw_1', Compiler) :-
%    comp_lang(Compiler, 'c').
%test('test_stw_1', 'test_stw_1', 'test_stw_1').
%test_description('test_stw_1', 'instrument one simple function call and save the world').
% Save the World isn't implemented on all platforms
%test_platform('test_stw_1', Platform) :-
%    platform('sparc', 'solaris', _, Platform);
%    platform('power', 'aix', _, Platform);
%    platform('i386', 'linux', _, Platform).
%test_runmode('test_stw_1', 'createProcess').
%test_start_state('test_stw_1', 'selfstart').

% test10_* only run on Sparc.
test_platform('test_sparc_1', Platform) :-
    platform('sparc', 'solaris', _, Platform).

% test_callback_1 (formerly test12_2)
mutator('test_callback_1', ['test_callback_1.C']).
mutatee('test_callback_1', ['test_callback_1_mutatee.c']).
compiler_for_mutatee('test_callback_1', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_callback_1', 'test_callback_1', 'test_callback_1').
test_description('test_callback_1', 'dynamic callsite callbacks').
% Why doesn't this test run on Windows?
test_platform('test_callback_1', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_callback_1', 'createProcess').
test_start_state('test_callback_1', 'stopped').

% test_callback_2 (formerly test12_7)
mutator('test_callback_2', ['test_callback_2.C']).
mutatee('test_callback_2', ['test_callback_2_mutatee.c']).
compiler_for_mutatee('test_callback_2', Compiler) :-
    comp_lang(Compiler, 'c').
test('test_callback_2', 'test_callback_2', 'test_callback_2').
test_description('test_callback_2', 'user defined message callback -- st').
% Is there really any reason why this test *can't* run on Windows?
test_platform('test_callback_2', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_callback_2', 'createProcess').
test_start_state('test_callback_2', 'stopped').

% test_thread_1 (Formerly test12_1)
mutator('test_thread_1', ['test_thread_1.C']).
mutatee('test_thread_1', ['test_thread_1_mutatee.c'], ['test_thread.c']).
compiler_for_mutatee('test_thread_1', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_requires_libs('test_thread_1', ['dl', 'pthread']).
test('test_thread_1', 'test_thread_1', 'test_thread_1').
test_description('test_thread_1', 'rtlib spinlocks').
% test_thread_* doesn't run on Windows
test_platform('test_thread_1', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_1', 'createProcess').
test_start_state('test_thread_1', 'stopped').

% test_thread_2 (formerly test12_3)
mutator('test_thread_2', ['test_thread_2.C']).
mutatee('test_thread_2', ['test_thread_2_mutatee.c'], ['test_thread.c']).
compiler_for_mutatee('test_thread_2', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_requires_libs('test_thread_2', ['pthread']).
test('test_thread_2', 'test_thread_2', 'test_thread_2').
% Does this test fall under callback tests or thread tests?
test_description('test_thread_2', 'thread create callback').
test_platform('test_thread_2', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_2', 'createProcess').
test_start_state('test_thread_2', 'stopped').

% test_thread_3 (formerly test12_4)
mutator('test_thread_3', ['test_thread_3.C']).
mutatee('test_thread_3', ['test_thread_3_mutatee.c'], ['test_thread.c']).
compiler_for_mutatee('test_thread_3', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_requires_libs('test_thread_3', ['pthread']).
test('test_thread_3', 'test_thread_3', 'test_thread_3').
test_description('test_thread_3', 'thread create callback - doa').
test_platform('test_thread_3', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_3', 'createProcess').
test_start_state('test_thread_3', 'stopped').

% test_thread_4 (formerly test12_5)
% This test is disabled for all platforms via an #ifdef..
test_description('test_thread_4', 'thread exit callback').

% test_thread_5 (formerly test12_8)
mutator('test_thread_5', ['test_thread_5.C']).
mutatee('test_thread_5', ['test_thread_5_mutatee.c'], ['test_thread.c']).
compiler_for_mutatee('test_thread_5', Compiler) :-
    comp_lang(Compiler, 'c').
mutatee_requires_libs('test_thread_5', ['pthread']).
test('test_thread_5', 'test_thread_5', 'test_thread_5').
test_description('test_thread_5', 'user defined message callback -- mt').
test_platform('test_thread_5', Platform) :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.
test_runmode('test_thread_5', 'createProcess').
test_start_state('test_thread_5', 'stopped').

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
test_runmode('test_thread_6', 'both').
test_start_state('test_thread_6', 'selfstart').

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
	optimization_level(Opt).
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
test_runmode('test_thread_7', 'both').
test_start_state('test_thread_7', 'selfstart').

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
test_runmode('test_thread_8', 'both').
test_start_state('test_thread_8', 'selfstart').

% The Fortran tests

% convenience clause for C components of Fortran tests
spec_object_file(Object, 'gcc', [Source], [], ['-DSOLO_MUTATEE $(MUTATEE_G77_CFLAGS)']) :-
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
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_1F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_1F').
%spec_object_file('test1_1F_mutatee_solo_gcc_none', 'gcc',
%   ['test1_1F_solo_me.c'], [], ['$(MUTATEE_G77_CFLAGS)']).
% spec_exception('test1_1F_mutatee.c', 'mutatee_flags',
%          ['gcc', '$(MUTATEE_G77_CFLAGS)']).
% spec_exception('test1_1F_mutatee.c', 'mutatee_flags',
%          ['g++', '$(MUTATEE_G77_CFLAGS)']).
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_1F', 'test1_1', 'test1_1F').
test_description('test1_1F', 'instrument with zero-arg function call (Fortran)').
test_runs_everywhere('test1_1F').
test_runmode('test1_1F', 'both').
test_start_state('test1_1F', 'stopped').
groupable_test('test1_1F').

mutatee('test1_2F', ['test1_2F_mutatee.c'], ['test1_2F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_2F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_2F', Compiler, 'none') :-
    compiler_for_mutatee('test1_2F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_2F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_2F').
%spec_object_file('test1_2F_mutatee_solo_gcc_none', 'gcc',
%   ['test1_2F_solo_me.c'], [], ['$(MUTATEE_G77_CFLAGS)']).
% spec_exception('test1_2F_mutatee.c', 'mutatee_flags',
%          ['gcc', '$(MUTATEE_G77_CFLAGS)']).
% spec_exception('test1_2F_mutatee.c', 'mutatee_flags',
%          ['g++', '$(MUTATEE_G77_CFLAGS)']).
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_2F', 'test1_2', 'test1_2F').
test_description('test1_2F', 'instrument with four-arg function call (Fortran)').
test_runs_everywhere('test1_2F').
test_runmode('test1_2F', 'both').
test_start_state('test1_2F', 'stopped').
groupable_test('test1_2F').

mutatee('test1_3F', ['test1_3F_mutatee.c'], ['test1_3F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_3F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_3F', Compiler, 'none') :-
    compiler_for_mutatee('test1_3F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_3F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_3F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_3F', 'test1_3', 'test1_3F').
test_description('test1_3F', 'passing variables to a function (Fortran)').
test_runs_everywhere('test1_3F').
test_runmode('test1_3F', 'both').
test_start_state('test1_3F', 'stopped').
groupable_test('test1_3F').

mutatee('test1_4F', ['test1_4F_mutatee.c'], ['test1_4F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_4F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_4F', Compiler, 'none') :-
    compiler_for_mutatee('test1_4F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_4F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_4F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_4F', 'test1_4', 'test1_4F').
test_description('test1_4F', 'instrument with a sequence (Fortran)').
test_runs_everywhere('test1_4F').
test_runmode('test1_4F', 'both').
test_start_state('test1_4F', 'stopped').
groupable_test('test1_4F').

mutatee('test1_5F', ['test1_5F_mutatee.c'], ['test1_5F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_5F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_5F', Compiler, 'none') :-
    compiler_for_mutatee('test1_5F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_5F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_5F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_5F', 'test1_5', 'test1_5F').
test_description('test1_5F', 'instrument with if clause (no else) (Fortran)').
test_runs_everywhere('test1_5F').
test_runmode('test1_5F', 'both').
test_start_state('test1_5F', 'stopped').
groupable_test('test1_5F').

mutatee('test1_6F', ['test1_6F_mutatee.c'], ['test1_6F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_6F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_6F', Compiler, 'none') :-
    compiler_for_mutatee('test1_6F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_6F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_6F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_6F', 'test1_6', 'test1_6F').
test_description('test1_6F', 'arithmetic operators (Fortran)').
test_runs_everywhere('test1_6F').
test_runmode('test1_6F', 'both').
test_start_state('test1_6F', 'stopped').
groupable_test('test1_6F').

mutatee('test1_7F', ['test1_7F_mutatee.c'], ['test1_7F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_7F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_7F', Compiler, 'none') :-
    compiler_for_mutatee('test1_7F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_7F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_7F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_7F', 'test1_7', 'test1_7F').
test_description('test1_7F', 'relational operators (Fortran)').
test_runs_everywhere('test1_7F').
test_runmode('test1_7F', 'both').
test_start_state('test1_7F', 'stopped').
groupable_test('test1_7F').

mutatee('test1_8F', ['test1_8F_mutatee.c'], ['test1_8F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_8F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_8F', Compiler, 'none') :-
    compiler_for_mutatee('test1_8F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_8F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_8F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_8F', 'test1_8', 'test1_8F').
test_description('test1_8F', 'verify that registers are preserved across inserted expression (Fortran)').
test_runs_everywhere('test1_8F').
test_runmode('test1_8F', 'both').
test_start_state('test1_8F', 'stopped').
groupable_test('test1_8F').

mutatee('test1_9F', ['test1_9F_mutatee.c'], ['test1_9F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_9F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_9F', Compiler, 'none') :-
    compiler_for_mutatee('test1_9F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_9F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_9F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_9F', 'test1_9', 'test1_9F').
test_description('test1_9F', 'verify that registers are preserved across inserted function call (Fortran)').
test_runs_everywhere('test1_9F').
test_runmode('test1_9F', 'both').
test_start_state('test1_9F', 'stopped').
groupable_test('test1_9F').

mutatee('test1_10F', ['test1_10F_mutatee.c'], ['test1_10F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_10F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_10F', Compiler, 'none') :-
    compiler_for_mutatee('test1_10F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_10F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_10F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_10F', 'test1_10', 'test1_10F').
test_description('test1_10F', 'inserted snippet order (Fortran)').
test_runs_everywhere('test1_10F').
test_runmode('test1_10F', 'both').
test_start_state('test1_10F', 'stopped').
groupable_test('test1_10F').

mutatee('test1_11F', ['test1_11F_mutatee.c'], ['test1_11F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_11F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_11F', Compiler, 'none') :-
    compiler_for_mutatee('test1_11F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_11F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_11F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_11F', 'test1_11', 'test1_11F').
test_description('test1_11F', 'insert snippets at entry, exit, and call points (Fortran)').
test_runs_everywhere('test1_11F').
test_runmode('test1_11F', 'both').
test_start_state('test1_11F', 'stopped').
groupable_test('test1_11F').

mutatee('test1_12F', ['test1_12F_mutatee.c'], ['test1_12F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_12F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_12F', Compiler, 'none') :-
    compiler_for_mutatee('test1_12F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_12F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_12F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_12F', 'test1_12', 'test1_12F').
test_description('test1_12F', 'insert/remove and malloc/free (Fortran)').
test_runs_everywhere('test1_12F').
test_runmode('test1_12F', 'both').
test_start_state('test1_12F', 'stopped').

mutatee('test1_13F', ['test1_13F_mutatee.c'], ['test1_13F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_13F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_13F', Compiler, 'none') :-
    compiler_for_mutatee('test1_13F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_13F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_13F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_13F', 'test1_13', 'test1_13F').
test_description('test1_13F', 'paramExpr,retExpr,nullExpr (Fortran)').
test_runs_everywhere('test1_13F').
test_runmode('test1_13F', 'both').
test_start_state('test1_13F', 'stopped').
groupable_test('test1_13F').

mutatee('test1_14F', ['test1_14F_mutatee.c'], ['test1_14F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_14F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_14F', Compiler, 'none') :-
    compiler_for_mutatee('test1_14F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_14F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_14F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_14F', 'test1_14', 'test1_14F').
test_description('test1_14F', 'Replace/Remove Function Call (Fortran)').
test_runs_everywhere('test1_14F').
test_runmode('test1_14F', 'both').
test_start_state('test1_14F', 'stopped').
groupable_test('test1_14F').

mutatee('test1_15F', ['test1_15F_mutatee.c'], ['test1_15F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_15F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_15F', Compiler, 'none') :-
    compiler_for_mutatee('test1_15F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_15F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_15F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_15F', 'test1_15', 'test1_15F').
test_description('test1_15F', 'setMutationsActive (Fortran)').
test_runs_everywhere('test1_15F').
test_runmode('test1_15F', 'both').
test_start_state('test1_15F', 'stopped').

mutatee('test1_16F', ['test1_16F_mutatee.c'], ['test1_16F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_16F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_16F', Compiler, 'none') :-
    compiler_for_mutatee('test1_16F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_16F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_16F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_16F', 'test1_16', 'test1_16F').
test_description('test1_16F', 'If else (Fortran)').
test_runs_everywhere('test1_16F').
test_runmode('test1_16F', 'both').
test_start_state('test1_16F', 'stopped').
groupable_test('test1_16F').

mutatee('test1_17F', ['test1_17F_mutatee.c'], ['test1_17F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_17F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_17F', Compiler, 'none') :-
    compiler_for_mutatee('test1_17F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_17F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_17F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_17F', 'test1_17', 'test1_17F').
test_description('test1_17F', 'Verifies that instrumentation inserted at exit point doesn\'t clobber return value (Fortran)').
test_runs_everywhere('test1_17F').
test_runmode('test1_17F', 'both').
test_start_state('test1_17F', 'stopped').
groupable_test('test1_17F').

mutatee('test1_18F', ['test1_18F_mutatee.c'], ['test1_18F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_18F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_18F', Compiler, 'none') :-
    compiler_for_mutatee('test1_18F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_18F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_18F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_18F', 'test1_18', 'test1_18F').
test_description('test1_18F', 'Read/Write a variable in the mutatee (Fortran)').
test_runs_everywhere('test1_18F').
test_runmode('test1_18F', 'both').
test_start_state('test1_18F', 'stopped').
groupable_test('test1_18F').

mutatee('test1_19F', ['test1_19F_mutatee.c'], ['test1_19F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_19F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_19F', Compiler, 'none') :-
    compiler_for_mutatee('test1_19F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_19F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_19F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_19F', 'test1_19', 'test1_19F').
test_description('test1_19F', 'oneTimeCode (Fortran)').
test_runs_everywhere('test1_19F').
test_runmode('test1_19F', 'both').
test_start_state('test1_19F', 'stopped').

mutatee('test1_20F', ['test1_20F_mutatee.c'], ['test1_20F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_20F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_20F', Compiler, 'none') :-
    compiler_for_mutatee('test1_20F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_20F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_20F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_20F', 'test1_20', 'test1_20F').
test_description('test1_20F', 'Instrumentation at arbitrary points (Fortran)').
test_runs_everywhere('test1_20F').
test_runmode('test1_20F', 'both').
test_start_state('test1_20F', 'stopped').
groupable_test('test1_20F').

mutatee('test1_25F', ['test1_25F_mutatee.c'], ['test1_25F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_25F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_25F', Compiler, 'none') :-
    compiler_for_mutatee('test1_25F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_25F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_25F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_25F', 'test1_25', 'test1_25F').
test_description('test1_25F', 'Unary Operators (Fortran)').
test_runs_everywhere('test1_25F').
test_runmode('test1_25F', 'both').
test_start_state('test1_25F', 'stopped').
groupable_test('test1_25F').

mutatee('test1_29F', ['test1_29F_mutatee.c'], ['test1_29F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_29F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_29F', Compiler, 'none') :-
    compiler_for_mutatee('test1_29F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_29F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_29F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_29F', 'test1_29', 'test1_29F').
test_description('test1_29F', 'BPatch_srcObj class (Fortran)').
test_runs_everywhere('test1_29F').
test_runmode('test1_29F', 'both').
test_start_state('test1_29F', 'stopped').
groupable_test('test1_29F').

mutatee('test1_31F', ['test1_31F_mutatee.c'], ['test1_31F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_31F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_31F', Compiler, 'none') :-
    compiler_for_mutatee('test1_31F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_31F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_31F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_31F', 'test1_31', 'test1_31F').
test_description('test1_31F', 'Non-Recursive Base Tramp (Fortran)').
test_runs_everywhere('test1_31F').
test_runmode('test1_31F', 'both').
test_start_state('test1_31F', 'stopped').
groupable_test('test1_31F').

mutatee('test1_32F', ['test1_32F_mutatee.c'], ['test1_32F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_32F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_32F', Compiler, 'none') :-
    compiler_for_mutatee('test1_32F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_32F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_32F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_32F', 'test1_32', 'test1_32F').
test_description('test1_32F', 'Recursive Base Tramp (Fortran)').
test_runs_everywhere('test1_32F').
test_runmode('test1_32F', 'both').
test_start_state('test1_32F', 'stopped').
groupable_test('test1_32F').

mutatee('test1_34F', ['test1_34F_mutatee.c'], ['test1_34F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_34F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_34F', Compiler, 'none') :-
    compiler_for_mutatee('test1_34F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_34F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_34F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_34F', 'test1_34', 'test1_34F').
test_description('test1_34F', 'Loop Information (Fortran)').
test_runs_everywhere('test1_34F').
test_runmode('test1_34F', 'both').
test_start_state('test1_34F', 'stopped').
groupable_test('test1_34F').

mutatee('test1_36F', ['test1_36F_mutatee.c'], ['test1_36F_fortran.F']).
% TODO Make sure these are correct
compiler_for_mutatee('test1_36F', Compiler) :-
	comp_lang(Compiler, 'fortran').
optimization_for_mutatee('test1_36F', Compiler, 'none') :-
    compiler_for_mutatee('test1_36F', Compiler).
% The C language components of the Fortran tests are compiled with different
% options than the rest of the C files.  Regretably, this clause is very non-
% intuitive..  I'm doing a hack here around the fact that the Python component
% doesn't know that test1_36F_mutatee.c gets preprocessed and renamed.. This
% sucks and I need to figure out a better way to do it.
fortran_c_component('test1_36F').
% First try at a test that uses a one-to-many mutator-mutatee mapping
test('test1_36F', 'test1_36', 'test1_36F').
test_description('test1_36F', 'Callsite Parameter Referencing (Fortran)').
test_runs_everywhere('test1_36F').
test_runmode('test1_36F', 'both').
test_start_state('test1_36F', 'stopped').
groupable_test('test1_36F').

% test_start_state/2
% test_start_state(?Test, ?State) specifies that Test should be run with its
% mutatee in state State, with State in {stopped, running, selfstart}

% compiler_for_mutatee/2
% compiler_for_mutatee(?Testname, ?Compiler)
% Specifies that the mutatee for the test Testname can be compiled with the
% compiler Compiler.
% If nothing else is specified, a test's mutatee can be compiled with any C
% compiler
% Actually, I don't know how to specify this..
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
platform('sparc', 'solaris', 'solaris2.8', 'sparc-sun-solaris2.8').
platform('sparc', 'solaris', 'solaris2.9', 'sparc-sun-solaris2.9').
platform('i386', 'windows', 'nt4.0', 'i386-unknown-nt4.0').
platform('i386', 'windows', 'winXP', 'i386-unknown-winXP').
platform('power', 'aix', 'aix5.1', 'rs6000-ibm-aix5.1').
platform('alpha', 'osf', 'osf5.1', 'alpha-dec-osf5.1').
platform('ia64', 'linux', 'linux2.4', 'ia64-unknown-linux2.4').
platform('x86_64', 'linux', 'linux2.4', 'x86_64-unknown-linux2.4').
platform('power', 'linux', 'linux2.6', 'ppc64_linux').
platform('power', 'linux', 'linux2.6', 'ppc32_linux').

% Platform Defns
% platform/1
% Convenience thing.  Mostly used for verifying that a particular platform
% exists.
platform(P) :- platform(_, _, _, P).

% Mutatee ABI specifications
% We're going to try out the new implementation idea here
parameter('mutatee_abi').
parameter_values('mutatee_abi', Values) :-
	findall(V, mutatee_abi(V), Values_t),
	sort(Values_t, Values).
mutatee_abi(32).
mutatee_abi(64).

% Platform ABI support
% Testing out how this looks with whitelist clauses
whitelist([['platform', Platform], ['mutatee_abi', ABI]]) :-
	platform_abi(Platform, ABI).

% platform_abi/2
% All platforms support 32-bit mutatees except ia64
% FIXME Does ppc64 support 32-bit mutatees?
platform_abi(Platform, 32) :-
	platform(Arch, _, _, Platform),
	Arch \= 'ia64'.

% A smaller list of platforms with for 64-bit mutatees
platform_abi('ia64-unknown-linux2.4', 64).
platform_abi('x86_64-unknown-linux2.4', 64).
platform_abi('ppc64_linux', 64).

% object_suffix/2
object_suffix(Platform, Suffix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Suffix = '.obj';
        Suffix = '.o'
    ).

% library_prefix/2
library_prefix(Platform, Prefix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Prefix = '';
        Prefix = 'lib'
    ).

% library_suffix/2
library_suffix(Platform, Suffix) :-
    platform(_, OS, _, Platform),
    (
        OS = 'windows' -> Suffix = '.dll';
        Suffix = '.so'
    ).

% Platform Compilers Constraints
% gcc and g++ run on everything but Windows
compiler_platform('gcc', Plat) :- platform(_, OS, _, Plat), OS \= 'windows'.
compiler_platform('g++', Plat) :- platform(_, OS, _, Plat), OS \= 'windows'.
% g77 only runs on i386 Linux
compiler_platform('g77', 'i386-unknown-linux2.4').
compiler_platform('g77', 'i386-unknown-linux2.6').
% Visual C/C++ only runs on Windows
compiler_platform('VC', Plat) :- platform(_, OS, _, Plat), OS == 'windows'.
compiler_platform('VC++', Plat) :- platform(_, OS, _, Plat), OS == 'windows'.
% Portland Group compiler only runs on i386 Linux
compiler_platform('pgcc', Plat) :-
    platform(Arch, OS, _, Plat), Arch == 'i386', OS == 'linux'.
compiler_platform('pgCC', Plat) :-
    platform(Arch, OS, _, Plat), Arch == 'i386', OS == 'linux'.
% Alpha's native compilers are cc & cxx
compiler_platform('cc', 'alpha-dec-osf5.1').
compiler_platform('cxx', 'alpha-dec-osf5.1').
% AIX's native compilers are xlc & xlC
compiler_platform('xlc', 'rs6000-ibm-aix5.1').
compiler_platform('xlC', 'rs6000-ibm-aix5.1').
% Solaris's native compilers are cc & CC
compiler_platform('cc', Plat) :- platform(_, OS, _, Plat), OS = 'solaris'.
compiler_platform('CC', Plat) :- platform(_, OS, _, Plat), OS = 'solaris'.

% aux_compiler_for_platform/3
% aux_compiler_for_platform(?Platform, ?Language, ?Compiler)
% Specifies the default compiler to use for a given (platform, language) pair
% for compiling files where we're not trying to test the output of the compiler
aux_compiler_for_platform(Platform, 'c', 'gcc') :-
    platform(_, 'linux', _, Platform).
aux_compiler_for_platform(Platform, 'fortran', 'g77') :-
    platform('i386', 'linux', _, Platform).
aux_compiler_for_platform(Platform, 'nasm_asm', 'nasm') :-
    platform('i386', 'linux', _, Platform).
aux_compiler_for_platform(Platform, 'att_asm', 'gcc') :-
    platform(_, OS, _, Platform),
    OS \= 'windows'.

% mcomp_plat/2
% mcomp_plat(?Compiler, ?Platform)
% Specifies that Compiler is used to compile mutators on Platform.  Eventually
% we'll probably support more than one mutator compiler per platform.
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

% Language-file extension mappings
lang_ext('fortran', '.F').
lang_ext('c', '.c').
lang_ext('c++', '.C').
lang_ext('att_asm', '.s').
lang_ext('att_asm', '.S').
lang_ext('masm_asm', '.asm').
lang_ext('nasm_asm', '.asm').

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% COMPILER SPECIFICATIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Compiler/language constraints
comp_lang('g77', 'fortran').
comp_lang(Compiler, 'c') :-
    member(Compiler, ['gcc', 'pgcc', 'VC', 'cc', 'xlc']);
    member(Compiler, ['g++', 'pgCC', 'VC++', 'cxx', 'CC', 'xlC']).
comp_lang(Compiler, 'c++') :-
    member(Compiler, ['g++', 'pgCC', 'VC++', 'cxx', 'CC', 'xlC']).
comp_lang('gcc', 'att_asm').
comp_lang('masm', 'masm_asm').

% Mutatee Compiler Defns
% mutatee_comp(compiler name)
mutatee_comp('gcc').
mutatee_comp('g77').
mutatee_comp('g++').
mutatee_comp('pgcc').
mutatee_comp('pgCC').
mutatee_comp('VC').
mutatee_comp('VC++').
mutatee_comp('cc').
mutatee_comp('cxx').
mutatee_comp('CC').
mutatee_comp('xlc').
mutatee_comp('xlC').

% compiler_presence_def/2
% compiler_presence_def(Compiler, EnvironmentVariable)
% Before trying to build anything using Compiler, we need to check whether
% EnvironmentVariable is set.  If it is not set, then we don't have access
% to Compiler.
% FIXME There's got to be a better way to do this.
compiler_presence_def('pgcc', 'PGI').
compiler_presence_def('pgCC', 'PGI').

% Translations between compiler names and compiler #defines
compiler_define_string('gcc', 'gnu_cc').
compiler_define_string('g++', 'gnu_cxx').
compiler_define_string('pgcc', 'native_cc').
compiler_define_string('VC', 'native_cc').
compiler_define_string('cc', 'native_cc').
compiler_define_string('xlc', 'native_cc').
compiler_define_string('pgCC', 'native_cxx').
compiler_define_string('VC++', 'native_cxx').
compiler_define_string('cxx', 'native_cxx').
compiler_define_string('CC', 'native_cxx').
compiler_define_string('xlC', 'native_cxx').
compiler_define_string('g77', 'gnu_fc').

%%%%%%%%%%
% *_s relations translate various internal atoms into strings than are
% appropriate for writing out to makefiles, etc.
%%%%%%%%%%

% compiler_s/2 translates from the name of a compiler to the executable name
% HACK I think we're already using the executable names as our atoms
compiler_s(Comp, Comp) :- 
    findall(C, mutatee_comp(C), Me_comp),
    findall(C, mutator_comp(C), Mr_comp),
    findall(C, (member(C, Me_comp); member(C, Mr_comp)), All_comp),
    sort(All_comp, Comps),
    member(Comp, Comps).

% Translation for Optimization Level
%compiler_opt_trans(compiler name, symbolic name, compiler argument).
% FIXME(?) I think the Windows optimization strings should be with cap-O, not
% zero
% FIXME I'm also not sure that all these compilers default to no optimization
% FIXME This is adding -O? optimization flags to any compilers that aren't
% explicitly listed..
compiler_opt_trans(_, 'none', '').
compiler_opt_trans(Comp, 'low', '-O1') :- 
    member(Comp, ['gcc', 'g++', 'pgcc', 'pgCC', 'cc', 'CC', 'xlc', 'xlC',
                  'cxx', 'g77']).
compiler_opt_trans(Comp, 'low', '/O1') :- Comp == 'VC++'; Comp == 'VC'.
compiler_opt_trans(Comp, 'high', '-O2') :-
    member(Comp, ['gcc', 'g++', 'pgcc', 'pgCC', 'cc', 'CC', 'xlc', 'xlC',
                  'cxx', 'g77']).
compiler_opt_trans(Comp, 'high', '/O2') :- Comp == 'VC++'; Comp == 'VC'.
compiler_opt_trans(Comp, 'max', '-O3') :- Comp == 'gcc'; Comp == 'g++'; Comp == 'cc'; Comp == 'ccx';
                                          Comp == 'CC'.
compiler_opt_trans(Comp, 'max', '/Ox') :- Comp == 'VC++'; Comp == 'VC'.

% Translation for parameter flags
% partial_compile: compile to an object file rather than an executable
compiler_parm_trans(Comp, 'partial_compile', '-c') :-
    member(Comp, ['gcc', 'g++', 'pgcc', 'pgCC', 'cc', 'CC',
                  'xlc', 'xlC', 'cxx', 'g77']).

% Mutator compiler defns
mutator_comp('g++').
mutator_comp('pgCC').
mutator_comp('VC++').
mutator_comp('cxx').
mutator_comp('CC').
mutator_comp('xlC').

% Per-compiler link options for building mutatees
mutatee_link_options('gcc', '$(MUTATEE_LDFLAGS_GNU)').
mutatee_link_options('g++', '$(MUTATEE_LDFLAGS_GNU)').
mutatee_link_options(Native_cc, '$(MUTATEE_CFLAGS_NATIVE) $(MUTATEE_LDFLAGS_NATIVE)') :-
    member(Native_cc, ['cc', 'xlc', 'pgcc', 'VC']).
mutatee_link_options(Native_cxx, '$(MUTATEE_CXXFLAGS_NATIVE) $(MUTATEE_LDFLAGS_NATIVE)') :-
    member(Native_cxx, ['cxx', 'CC', 'xlC', 'pgCC', 'VC++']).

% Specify the standard flags for each compiler
comp_std_flags_str('gcc', '$(CFLAGS)').
comp_std_flags_str('g++', '$(CXXFLAGS)').
comp_std_flags_str('cc', '$(CFLAGS_NATIVE)').
comp_std_flags_str('xlc', '$(CFLAGS_NATIVE)').
comp_std_flags_str('pgcc', '$(CFLAGS_NATIVE)').
comp_std_flags_str('CC', '$(CXXFLAGS_NATIVE)').
% FIXME Make sure that these flags for cxx are correct, or tear out cxx (Alpha)
comp_std_flags_str('cxx', '$(CXXFLAGS_NATIVE)').
comp_std_flags_str('xlC', '$(CXXFLAGS_NATIVE)').
comp_std_flags_str('pgCC', '$(CXXFLAGS_NATIVE)').
% FIXME Tear out the '-DSOLO_MUTATEE' from these and make it its own thing
comp_mutatee_flags_str('gcc', '-DSOLO_MUTATEE $(MUTATEE_CFLAGS_GNU)').
comp_mutatee_flags_str('g++', '-DSOLO_MUTATEE $(MUTATEE_CXXFLAGS_GNU)').
comp_mutatee_flags_str('cc', '$(MUTATEE_CFLAGS_NATIVE)').
comp_mutatee_flags_str('xlc', '$(MUTATEE_CFLAGS_NATIVE)').
comp_mutatee_flags_str('pgcc', '-DSOLO_MUTATEE $(MUTATEE_CFLAGS_NATIVE)').
comp_mutatee_flags_str('CC', '$(MUTATEE_CXXFLAGS_NATIVE)').
% FIXME Make sure that these flags for cxx are correct, or tear out cxx (Alpha)
comp_mutatee_flags_str('cxx', '$(MUTATEE_CXXFLAGS_NATIVE)').
comp_mutatee_flags_str('xlC', '$(MUTATEE_CXXFLAGS_NATIVE)').
comp_mutatee_flags_str('pgCC', '-DSOLO_MUTATEE $(MUTATEE_CXXFLAGS_NATIVE)').
% FIXME What do I specify for the Windows compilers, if anything?
comp_std_flags_str('VC', '').
comp_std_flags_str('VC++', '').
comp_mutatee_flags_str('VC', '').
comp_mutatee_flags_str('VC++', '').

% g77 flags
comp_std_flags_str('g77', '-g').
comp_mutatee_flags_str('g77', '$(MUTATEE_G77_FFLAGS)').
mutatee_link_options('g77', '$(MUTATEE_G77_LDFLAGS)').

% NASM (for test_mem (formerly test6))
comp_lang('nasm', 'nasm_asm').
compiler_define_string('nasm', 'nasm').
compiler_platform('nasm', Platform) :-
    platform('i386', 'linux', _, Platform). % NASM runs on x86 Linux
comp_std_flags_str('nasm', '-f elf -dPLATFORM=$(PLATFORM)').
comp_mutatee_flags_str('nasm', '').
mutatee_link_options('nasm', '').
mutatee_comp('nasm'). % I think I want to reorganize so this isn't required
                      % for compilers that are only used for auxialliary files
compiler_parm_trans('nasm', 'partial_compile', '').

% Compiler Optimization Level Defns
optimization_level('none').
optimization_level('low').
optimization_level('high').
optimization_level('max').

% ABI defns for compilers
% compiler_platform_abi_s_default(FlagString)
% The flags string for a platform's default ABI (Do we want this?)
compiler_platform_abi_s_default('').
% compiler_platform_abi_s(Compiler, Platform, ABI, FlagString)
compiler_platform_abi_s(Compiler, Platform, ABI, '') :-
	mutatee_comp(Compiler),
	Compiler \= '',
	platform(Platform),
	compiler_platform(Compiler, Platform),
	mutatee_abi(ABI),
	platform_abi(Platform, ABI),
	\+ (member(Compiler, ['gcc', 'g++']), Platform = 'x86_64-unknown-linux2.4',
	    ABI = 32).
compiler_platform_abi_s(Compiler, 'x86_64-unknown-linux2.4', 32,
                        '-m32 -Di386_unknown_linux2_4 -Dm32_test') :-
	member(Compiler, ['gcc', 'g++']).


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
% from complaining that test_runs_everywhere isn't defined.
test_runs_everywhere(none).

% Test definitions

% test_name/1
% test_name(?Name)
% Specifies that Name is the name of a test.  This is kind of a useless clause
% right now.  I wanted to do something with it, but that never came together.
% I think that test/3 makes for a better implementation of what I was trying
% to do.

% test/3
% test(?Name, ?Mutator, ?Mutatee)
% Specifies that the test Name maps the mutator Mutator to the mutatee Mutatee.
% This (hopefully) allows many-to-many mappings to exist, and lets us assign
% a unique name to each mapping.
%
% The following is a convenience clause for backward compatibility.  It should
% let me change the internal uses of mutator_mutatee/2 to test/3 without
% needing to go back and edit all of the already-written test specs.
test(Name, Mutator, Mutatee) :-
    test_name(Name),
    mutator_mutatee(Name, Name),
    Mutator = Name,
    Mutatee = Name.

% Mutator Defns
% mutator/2
% mutator(+name, ?source list)

% Mutators can also require libraries to be linked with.
% All mutators need to be linked with dyninstAPI and iberty
% All mutators also need to be linked with testSuite, for the TestMutator
% superclass if nothing else.
all_mutators_require_libs(['dyninstAPI', 'iberty', 'testSuite']).
% Those with no specific requirements don't require specific libraries
mutator_requires_libs(Mutator, []) :-
    mutator(Mutator, _),
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

% I don't think there are any libraries that all mutatees need to be linked to
all_mutatees_require_libs([]).
% mutatee_requires_libs/2
% Allows the user to specify which libraries a mutatee needs to be linked with
% Any mutatees with no specified required libraries don't require any.
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

% Mutator,Mutatee Constraints
% mutator_mutatee/2
% mutator(?Mutator, ?Mutatee).

% Mutatee, Compliers Constraints
% Using compiler_for_mutatee(?Mutatee, ?Compiler) now

% optimization_for_mutatee/3
% optimization_for_mutatee(?Mutatee, ?Compiler, ?OptimizationLevel)
% Unifies a mutatee name with a valid compiler and optimization level for
% building an object file for the mutatee
% TODO Create some kind of default optimization_for_mutatee rule

% It makes more sense for run mode constraints to be by test rather than by
% mutatee.
% test_runmode/2
% test_runmode(?Test, ?Runmode)
% The atom 'both' as Runmode means the test can be run in either useAttach or createProcess mode.
test_runmode(Test, 'useAttach') :- test_runmode(Test, 'both').
test_runmode(Test, 'createProcess') :- test_runmode(Test, 'both').

% mutatee_peers/2
mutatee_peers(M, P) :-
    findall(N, mutatee_peer(M, N), Ps), sort(Ps, P).

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

% spec_object_file/4
% spec_object_file(?ObjectFile, ?Compiler, ?SourceList, ?DependencyList,
%                  ?FlagList)
% This clause should contain everything necessary to generate a makefile rule
% for an object file.  I don't want to do text processing in the Prolog
% component of the spec compiler, so we'll pass along a main source file string
% and let the Python component handle transforming that into an object file
% name
