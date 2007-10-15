:- include('util.pl').

% test_init/1
% test_init(+Platform)
% Performs some initialization: sets the global variable platform_name to
% its argument and sets up a global variable with the object suffix for this
% platform.  Need to change the object suffix thing to be a term rather than
% a gvar.
test_init(Platform) :- nonvar(Platform),
                       platform(Platform),
                       g_assign(platform_name, Platform).

% current_platform/1
% current_platform(?Platform)
% This predicate is true if Platform can be unified with the current platform
% name, as previously specified by test_init/1
current_platform(Platform) :- g_read(platform_name, Platform).

% Simple tuple rules.  I'll need more complex rules to generate the output that
% actually goes to the makefile generator: it will need to have the correct
% strings to be inserted into the makefiles rather than the internal symbols

% platform_tuple/5
% Maps a platform name to the filename conventions and auxilliary compilers
% for that platform
% Platform, ObjSuffix, LibPrefix, and LibSuffix are strings
% AuxCompilers is a map from language names to compiler names
platform_tuple(Platform, ObjSuffix, LibPrefix, LibSuffix, AuxCompilers,
	           ABIs) :-
    platform(_, _, _, Platform),
    object_suffix(Platform, ObjSuffix),
    library_prefix(Platform, LibPrefix),
    library_suffix(Platform, LibSuffix),
    % AuxCompilers defaults to an empty map if none were specified
    (
        \+ aux_compiler_for_platform(Platform, _, _) -> AuxCompilers = [];
        findall([L, C],
                 aux_compiler_for_platform(Platform, L, C),
                 AuxCompilers)
    ),
	findall(A, platform_abi(Platform, A), ABIs_t),
	sort(ABIs_t, ABIs).

compiler_tuple(Name, Executable, DefString, Platforms, PresenceVar, OptStrings,
               ParmStrings, Languages, StdFlags, ABIFlags) :-
    findall(C, (mutatee_comp(C); mutator_comp(C)), Cs),
    sort(Cs, Cs_q), !,
    member(Name, Cs_q),
    findall(P, compiler_platform(Name, P), Platforms_n),
    sort(Platforms_n, Platforms),
    compiler_s(Name, Executable),
    compiler_define_string(Name, DefString),
    findall([L, S], compiler_opt_trans(Name, L, S), OptStrings),
    findall([P, S], compiler_parm_trans(Name, P, S), ParmStrings),
    findall(L, comp_lang(Name, L), Languages),
    % Mutatee link options defaults to empty
    (
        \+ mutatee_link_options(Name, _) -> LinkFlagsStr = '';
        mutatee_link_options(Name, LinkFlagsStr)
    ),
    % Standard flags string defaults to empty
    (
        \+ comp_std_flags_str(Name, _) -> StdFlagsStr = '';
        comp_std_flags_str(Name, StdFlagsStr)
    ),
    % Mutatee flags string defaults to empty
    (
        \+ comp_mutatee_flags_str(Name, _), MutFlagsStr = '';
        comp_mutatee_flags_str(Name, MutFlagsStr)
    ),
    StdFlags = [StdFlagsStr, MutFlagsStr, LinkFlagsStr],
	% Need ABIFlags
	% [Platform, ABI, ABIFlagStr]
	findall([P, A, F], compiler_platform_abi_s(Name, P, A, F), ABIFlags),
	(
		\+ compiler_presence_def(Name, _) -> PresenceVar = 'true';
		compiler_presence_def(Name, PresenceVar)
	),
	true.

% Tuples for language specifications.
% Right now it just maps language names to filename extensions
language_tuple(Language, Extensions) :-
    lang(Language),
    findall(E, lang_ext(Language, E), Es),
    sort(Es, Extensions).

mutator_tuple(Name, Sources, Libraries, Platform, Compiler) :-
    mutator(Name, Sources),
    mutator_requires_libs(Name, Explicit_libs),
    all_mutators_require_libs(Implicit_libs),
    findall(L, (member(L, Explicit_libs); member(L, Implicit_libs)), All_libs),
    % BUG(?) This doesn't maintain order of libraries, if link order matters..
    sort(All_libs, Libraries),
    test_platform(Name, Platform),
    mcomp_plat(Compiler, Platform).

mutator_tuple_s([Name, Sources, Libraries, Platform, Compiler],
                [Name_s, Sources_s, Libraries_s, Platform_s, Compiler_s]) :-
    mutator_tuple(Name, Sources, Libraries, Platform, Compiler),
    Name = Name_s,
    Sources = Sources_s,
    Libraries = Libraries_s,
    Platform = Platform_s,
    compiler_s(Compiler, Compiler_s).

% This clause generates duplicates which are removed by the sort/2 call in
% write_tuples.
mutatee_tuple(Name, PreprocessedSources, RawSources, Libraries, Platform,
              ABI, Compiler, Optimization_level, Groupable) :-
	test(TestName, _, Name),
	test_platform(TestName, Platform),
	test_platform_abi(TestName, Platform, ABI),
    % This mutatee is groupable if any of the tests that use it are groupable
    % FIXME This is assuming a one-to-one relation between mutators and
    % mutatees.  This should still work as long as the mutatee is only used
    % in either groupable tests or non-groupable tests.
    (
		groupable_test(TestName) -> Groupable = 'true';
		Groupable = 'false'
    ),
    mutatee(Name, PreprocessedSources, S2),
    forall_mutatees(S3),
    % Merge the source lists S2 and S3
    append(S2, S3, S4), sort(S4, RawSources),
    mutatee_requires_libs(Name, Libraries),
    compiler_for_mutatee(Name, Compiler),
    compiler_platform(Compiler, Platform),
    (
        \+ optimization_for_mutatee(Name, _, _) ->
            compiler_opt_trans(Compiler, Optimization_level, _);
        (optimization_for_mutatee(Name, Compiler, Optimization_level),
     	 compiler_opt_trans(Compiler, Optimization_level, _))
    ).

% This one handles peers
mutatee_tuple(Name, PreprocessedSources, RawSources, Libraries, Platform,
              ABI, Compiler, Optimization_level, Groupable) :-
    mutatee(M1, _, _),
	test(TestName, _, M1),
    test_platform(TestName, Platform),
    mutatee_peer(M1, Name),
    mutatee(Name, PreprocessedSources, RS1),
    forall_mutatees(RS2),
    append(RS1, RS2, RS3),
    sort(RS3, RawSources),
    mutatee_requires_libs(Name, Libraries),
    compiler_for_mutatee(Name, Compiler),
    compiler_platform(Compiler, Platform),
    (
        \+ optimization_for_mutatee(Name, _, _) ->
            compiler_opt_trans(Compiler, Optimization_level, _);
        (optimization_for_mutatee(Name, Compiler, Optimization_level),
	     compiler_opt_trans(Compiler, Optimization_level, _))
    ),
	test_platform_abi(TestName, Platform, ABI),
    % FIXME This is assuming a one-to-one relation between mutators and
    % mutatees.  This should still work as long as the mutatee is only used
    % in either groupable tests or non-groupable tests.
    (
        test(_, Mr, M1),
        (
            groupable_test(Mr) -> Groupable = 'true';
            Groupable = 'false'
        )
    ).

% For mutatees, I should be using a (mutatee, compiler, optimization) tuple
% instead of a simple mutatee name
% test_tuple
% Map a test name to its mutator and mutatee, along with ?
test_tuple(Name, Mutator, Mutatee, Platform, Groupable) :-
    test(Name, Mutator, Mutatee),
    test_platform(Name, Platform),
    (groupable_test(Name) -> Groupable = true; Groupable = false).

% Provide tuples for run groups
rungroup_tuple(Mutatee, Compiler, Optimization, RunMode, StartState,
               Groupable, Tests, Platform, ABI) :-
    mutatee(Mutatee, _, _),
    compiler_for_mutatee(Mutatee, Compiler),
    compiler_platform(Compiler, Platform),
    (
        \+ optimization_for_mutatee(Mutatee, _, _) ->
            compiler_opt_trans(Compiler, Optimization, _);
        (optimization_for_mutatee(Mutatee, Compiler, Optimization),
	 compiler_opt_trans(Compiler, Optimization, _))
    ),
	platform_abi(Platform, ABI),
    member(RunMode, ['createProcess', 'useAttach']),
    member(StartState, ['stopped', 'running', 'selfstart']),
    member(Groupable, ['true', 'false']),
    findall(T, (test(T, _, Mutatee),
	            test_platform(T, Platform),
				test_platform_abi(T, Platform, ABI),
			    test_runmode(T, RunMode), test_start_state(T, StartState),
			    ((groupable_test(T), Groupable = true);
			     (\+ groupable_test(T), Groupable = false))),
            Ts),
    sort(Ts, Ts_q),
    Ts_q \= [],
    Tests = Ts_q.

exception_tuple(Filename, ExceptionType, ParameterNames, Parameters) :-
    spec_exception(Filename, ExceptionType, Parameters),
    spec_exception_type(ExceptionType, ParameterCount, ParameterNames),
    length(ParameterNames, ParameterCount),
    length(Parameters, ParameterCount).

% write_tuples/1
% write_tuples(Filename)
% Writes compiler, mutator, mutatee, and test tuple lists for the platform
% Platform to the file specified in Filename
write_tuples(Filename, Platform) :-
    nonvar(Filename), nonvar(Platform), % Sanity check
    open(Filename, write, Stream),
    findall([P, OS, LP, LS, AC, As], platform_tuple(P, OS, LP, LS, AC, As),
	        Platforms),
    write_term(Stream, Platforms, [quoted(true)]),
    write(Stream, '\n'),
    findall([L, E], language_tuple(L, E), Languages),
    write_term(Stream, Languages, [quoted(true)]),
    write(Stream, '\n'),
    findall([N, E, D, P, PV, O, PS, L, F, As],
	        compiler_tuple(N, E, D, P, PV, O, PS, L, F, As),
            Compilers),
    write_term(Stream, Compilers, [quoted(true)]),
    write(Stream, '\n'),
    findall([N, S, L, Platform, C],
            mutator_tuple(N, S, L, Platform, C), Mutators),
    write_term(Stream, Mutators, [quoted(true)]),
    write(Stream, '\n'),
    findall([N, PS, RS, L, Platform, A, C, O, G],
            mutatee_tuple(N, PS, RS, L, Platform, A, C, O, G), Mutatees_t),
	sort(Mutatees_t, Mutatees),
    write_term(Stream, Mutatees, [quoted(true)]),
    write(Stream, '\n'),
    findall([T, Mr, Me, G],
            test_tuple(T, Mr, Me, Platform, G),
            Tests),
    write_term(Stream, Tests, [quoted(true)]),
    write(Stream, '\n'),
    findall([M, C, O, R, S, G, T, A],
            rungroup_tuple(M, C, O, R, S, G, T, Platform, A),
            RunGroups),
    write_term(Stream, RunGroups, [quoted(true)]),
    write(Stream, '\n'),
%     findall([ET, PC, PL], spec_exception_type(ET, PC, PL), ExceptionTypes),
%     write_term(Stream, ExceptionTypes, [quoted(true)]),
%     write(Stream, '\n'),
%     findall([F, ET, PN, Ps], exception_tuple(F, ET, PN, Ps), Exceptions),
%     write_term(Stream, Exceptions, [quoted(true)]),
%     write(Stream, '\n'),
    findall([M, C, S, D, F], spec_object_file(M, C, S, D, F), ObjectFiles),
	write_term(Stream, ObjectFiles, [quoted(true)]),
	write(Stream, '\n'),
    close(Stream), !.
