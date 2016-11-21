% Dyninst test suite specification compiler: tuple generator

:- include('util.pl').

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% UTILITY FUNCTIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
current_platform(Platform) :-
    g_read(platform_name, Platform).

% Simple tuple rules.  I'll need more complex rules to generate the output that
% actually goes to the makefile generator: it will need to have the correct
% strings to be inserted into the makefiles rather than the internal symbols

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% PLATFORM TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% platform_tuple/8
% Maps a platform name to the filename conventions and auxilliary compilers
% for that platform
% Platform, ObjSuffix, ExecSuffix, LibPrefix, LibSuffix, and Linker are strings
% AuxCompilers is a map from language names to compiler names
platform_tuple(Platform, ObjSuffix, ExecSuffix,LibPrefix, LibSuffix,
               Linker, AuxCompilers, ABIs) :-
    platform(_, _, _, Platform),
    object_suffix(Platform, ObjSuffix),
    executable_suffix(Platform, ExecSuffix),
    library_prefix(Platform, LibPrefix),
    library_suffix(Platform, LibSuffix),
	% Linker defaults to an empty string if not specified
	(
		\+ linker(Platform, _) -> Linker = '';
		linker(Platform, Linker)
	),
    % AuxCompilers defaults to an empty map if none were specified
    (
        \+ aux_compiler_for_platform(Platform, _, _) -> AuxCompilers = [];
        findall([L, C],
                 aux_compiler_for_platform(Platform, L, C),
                 AuxCompilers)
    ),
    findall(A, platform_abi(Platform, A), ABIs_t),
    sort(ABIs_t, ABIs).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% COMPILER TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

compiler_tuple(Name, Executable, DefString, Platforms, PresenceVar, OptStrings, PicStrings,
               ParmStrings, Languages, StdFlags, ABIFlags, StaticLink, DynamicLink, Platform) :-
    % The next two lines ensure that we only return compilers that are used to
    % build at least one mutator or mutatee, and we don't return the same
    % compiler more than once.
    findall(C, (mutatee_comp(C); mutator_comp(C)), Cs),
    sort(Cs, Cs_q), !,
    member(Name, Cs_q),
    findall(P, compiler_platform(Name, P), Platforms_n),
    sort(Platforms_n, Platforms),
    compiler_s(Name, Executable),
    compiler_define_string(Name, DefString),
    findall([L, S], compiler_opt_trans(Name, L, S), OptStrings),
    findall([L, S], compiler_pic_trans(Name, L, S), PicStrings),
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
    % Mutatee static link flags, defaults to empty
    (
        \+ compiler_static_link(Name, Platform, _) -> StaticLink = '';
        compiler_static_link(Name, Platform, StaticLink)
    ),
    % Mutatee dynamic link flags, defaults to empty
    (
        \+ compiler_dynamic_link(Name, Platform, _) -> DynamicLink = '';
        compiler_dynamic_link(Name, Platform, DynamicLink)
    ),
    StdFlags = [StdFlagsStr, MutFlagsStr, LinkFlagsStr],
    % Need ABIFlags
    % [Platform, ABI, ABIFlagStr]
    findall([P, A, F, CS], compiler_platform_abi_s(Name, P, A, F, CS), ABIFlags),
    (
        \+ compiler_presence_def(Name, _) -> PresenceVar = 'true';
        compiler_presence_def(Name, PresenceVar)
    ),
    true.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% LANGUAGE TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Tuples for language specifications.
% Right now it just maps language names to filename extensions
language_tuple(Language, Extensions) :-
    lang(Language),
    findall(E, lang_ext(Language, E), Es),
    sort(Es, Extensions).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% MUTATOR TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

mutator_tuple(Name, Sources, Libraries, Platform, Compiler) :-
    % We don't want to produce duplicates here, so let's get a list of all
    % the appropriate mutators and limit Name to members of that list
    findall(M,
            (
                % We're getting a list of all the mutators that are used for
                % tests that run on this platform
                test(TestName, M, _),
                test_platform(TestName, Platform)
            ),
            Ms),
    sort(Ms, Ms_sorted), !, % We won't build the list again
    member(Name, Ms_sorted),
    mutator(Name, Sources),
    mutator_requires_libs(Name, Explicit_libs),
    all_mutators_require_libs(Implicit_libs),
    tests_module(Name, Modules),
    platform_module(Platform, Modules),
    module_requires_libs(Modules, Module_libs),
    findall(L, (member(L, Explicit_libs); member(L, Implicit_libs); member(L, Module_libs)), All_libs),
    % BUG(?) This doesn't maintain order of libraries, if link order matters..
    sort(All_libs, Libraries),
    mcomp_plat(Compiler, Platform).

% TODO Remove this term; it is not used
mutator_tuple_s([Name, Sources, Libraries, Platform, Compiler],
                [Name_s, Sources_s, Libraries_s, Platform_s, Compiler_s]) :-
    mutator_tuple(Name, Sources, Libraries, Platform, Compiler),
    Name = Name_s,
    Sources = Sources_s,
    Libraries = Libraries_s,
    Platform = Platform_s,
    compiler_s(Compiler, Compiler_s).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% MUTATEE TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% This clause generates duplicates which are removed by the sort/2 call in
% write_tuples.
mutatee_tuple(Name, PreprocessedSources, RawSources, Libraries, Platform,
              ABI, Compiler, Optimization_level, Groupable, Module, Format, PIC) :-
    test(TestName, _, Name),
    test_platform(TestName, Platform),
    test_platform_abi(TestName, Platform, ABI),
    tests_module(TestName, Module),
    platform_module(Platform, Module),
    % This mutatee is groupable if any of the tests that use it are groupable
    % FIXME This is assuming a one-to-one relation between mutators and
    % mutatees.  This should still work as long as the mutatee is only used
    % in either groupable tests or non-groupable tests.
    (
        groupable_test(TestName) -> Groupable = 'true';
        Groupable = 'false'
    ),
    mutatee(Name, PreprocessedSources, S2),
    mutatee_format(Name, Format),
    compiler_format(Compiler, Format),
    forall_mutatees(S3),
    % Merge the source lists S2 and S3
    append(S2, S3, S4), sort(S4, RawSources),
    mutatee_requires_libs(Name, Libraries),
    compiler_for_mutatee(Name, Compiler),
    compiler_platform(Compiler, Platform),
    \+ mutatee_compiler_platform_exclude(Compiler, Platform),
    compiler_pic(Compiler, PIC),
    (
        \+ optimization_for_mutatee(Name, _, _) ->
            compiler_opt_trans(Compiler, Optimization_level, _);
        (optimization_for_mutatee(Name, Compiler, Optimization_level),
         compiler_opt_trans(Compiler, Optimization_level, _))
    ),
    module_pic(Module, PIC).

% This one handles peers
mutatee_tuple(Name, PreprocessedSources, RawSources, Libraries, Platform,
              ABI, Compiler, Optimization_level, Groupable, Module, Format, PIC) :-
    mutatee(M1, _, _),
    test(TestName, _, M1),
    test_platform(TestName, Platform),
    mutatee_peer(M1, Name),
    mutatee(Name, PreprocessedSources, RS1),
    tests_module(TestName, Module),
    platform_module(Platform, Module),
    forall_mutatees(RS2),
    append(RS1, RS2, RS3),
    sort(RS3, RawSources),
    mutatee_requires_libs(Name, Libraries),
    compiler_for_mutatee(Name, Compiler),
    mutatee_format(Name, Format),
    compiler_format(Compiler, Format),
    compiler_platform(Compiler, Platform),
    \+ mutatee_compiler_platform_exclude(Compiler, Platform),
    compiler_pic(Compiler, PIC),
    (
        \+ optimization_for_mutatee(Name, _, _) ->
            compiler_opt_trans(Compiler, Optimization_level, _);
        (optimization_for_mutatee(Name, Compiler, Optimization_level),
         compiler_opt_trans(Compiler, Optimization_level, _))
    ),
    module_pic(Module, PIC),
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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% TEST AND RUNGROUP TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% For mutatees, I should be using a (mutatee, compiler, optimization) tuple
% instead of a simple mutatee name
% test_tuple
% Map a test name to its mutator and mutatee, along with ?
test_tuple(Name, Mutator, Mutatee, Platform, Groupable, Module) :-
    test(Name, Mutator, Mutatee),
    test_platform(Name, Platform),
    (groupable_test(Name) -> Groupable = true; Groupable = false),
	tests_module(Name, Module),
   platform_module(Platform, Module).

% Provide tuples for run groups
rungroup_tuple(Mutatee, Compiler, Optimization, RunMode, StartState,
              Groupable, Tests, Platform, ABI, ThreadMode, ProcessMode, Format,
              MutatorStart, MutateeStart, MutateeLaunchtime, PIC, PlatMode) :-
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
    compiler_platform_abi(Compiler, Platform, ABI),
    format_runmode(Platform, RunMode, Format),
    platform_format(Platform, Format),
    compiler_format(Compiler, Format),
    \+ mutatee_compiler_platform_exclude(Compiler, Platform),
    mutatee_format(Mutatee, Format),
    mutatee_module(Mutatee, Module),
    module_runmode_format(Module, RunMode, Format),
    compiler_pic(Compiler, PIC),
    module_pic(Module, PIC),
    % Enumerate / verify values for run-time options
    runmode(RunMode),
    runmode_launch_params(RunMode, Platform, MutatorStart, MutateeStart, MutateeLaunchtime),
    threadmode(ThreadMode),
    processmode(ProcessMode),
    platform_mode(Platform, PlatMode, RunMode, ThreadMode),
    member(StartState, ['stopped', 'running', 'selfstart', 'selfattach', 'delayedattach']),
    member(Groupable, ['true', 'false']),
    (
        % Rungroups for the 'none' mutatee should only contain a single test
        Mutatee = 'none' ->
            (
                test(T, _, Mutatee),
                test_platform(T, Platform),
                tests_module(T, Module),
                runmode_platform(Platform, RunMode, Module),
                platform_module(Platform, Module),
                test_platform_abi(T, Platform, ABI),
                test_runmode(T, RunMode),
                \+ test_exclude_format(T, Format),
                test_threadmode(T, ThreadMode),
                test_processmode(T, ProcessMode),
                test_start_state(T, StartState),
                ((groupable_test(T), Groupable = true);
                 (\+ groupable_test(T), Groupable = false)),
                Ts = [T]
            );
        % Rungroups for other mutatees may contain a number of tests
        findall(T, (test(T, _, Mutatee),
                    test_platform(T, Platform),
                    tests_module(T, Module),
                    platform_module(Platform, Module),
                    runmode_platform(Platform, RunMode, Module),
                    test_platform_abi(T, Platform, ABI),
                    test_runmode(T, RunMode), test_start_state(T, StartState),
                    \+ test_exclude_format(T, Format),
                    \+ test_exclude_compiler(T, Compiler),
                    test_threadmode(T, ThreadMode),
                    test_processmode(T, ProcessMode),
                    ((groupable_test(T), Groupable = true);
                     (\+ groupable_test(T), Groupable = false))),
                    Ts)
    ),
    sort(Ts, Ts_q),
    Ts_q \= [],
    Tests = Ts_q.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% EXCEPTION TUPLES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Exception tuples of this form are not used yet.  They are an avenue for
% future development
exception_tuple(Filename, ExceptionType, ParameterNames, Parameters) :-
    spec_exception(Filename, ExceptionType, Parameters),
    spec_exception_type(ExceptionType, ParameterCount, ParameterNames),
    length(ParameterNames, ParameterCount),
    length(Parameters, ParameterCount).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% TUPLE OUTPUT
%%%
%%% The write_tuples term is what gets called to generate tuples for the
%%% specification compiler.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% write_tuples/2
% write_tuples(Filename, Platform)
% Writes compiler, mutator, mutatee, and test tuple lists for the platform
% Platform to the file specified in Filename
write_tuples(Filename, Platform) :-
    nonvar(Filename), nonvar(Platform), % Sanity check
    % Check insane/2 statements.  If sanity checks fail, abort
    findall(Insanity, (insane(M, P), Insanity = [M, P]), Insanities),
    sort(Insanities, Insanities_uniq),
    \+ length(Insanities_uniq, 0) ->
            % At least one sanity check has failed.  Print all sanity checking
            % error messages and exit.
        (for_each(Insanities_uniq, [Fmt, Arg], format(Fmt, Arg)),
         halt(-1));
    % Sanity checks passed, so continue with the tuple generation
    open(Filename, write, Stream),
    findall([P, OS, ES, LP, LS, L, AC, As],
            platform_tuple(P, OS, ES, LP, LS, L, AC, As),
            Platforms),
    write_term(Stream, Platforms, [quoted(true)]),
    write(Stream, '\n'),
    findall([L, E], language_tuple(L, E), Languages),
    write_term(Stream, Languages, [quoted(true)]),
    write(Stream, '\n'),
    findall([N, E, D, P, PV, O, PIC, PS, L, F, As, SL, DL, Platform],
            compiler_tuple(N, E, D, P, PV, O, PIC, PS, L, F, As, SL, DL, Platform),
            Compilers),
    write_term(Stream, Compilers, [quoted(true)]),
    write(Stream, '\n'),
    setof([N, S, L, Platform, C],
            mutator_tuple(N, S, L, Platform, C), Mutators),
    write_term(Stream, Mutators, [quoted(true)]),
    write(Stream, '\n'),
    findall([N, PS, RS, L, Platform, A, C, O, G, MO, FO, PIC],
            mutatee_tuple(N, PS, RS, L, Platform, A, C, O, G, MO, FO, PIC), Mutatees_t),
    sort(Mutatees_t, Mutatees),
    write_term(Stream, Mutatees, [quoted(true)]),
    write(Stream, '\n'),
    findall([T, Mr, Me, G, Mo],
            test_tuple(T, Mr, Me, Platform, G, Mo),
            Tests),
    write_term(Stream, Tests, [quoted(true)]),
    write(Stream, '\n'),
    findall([M, C, O, R, S, G, T, A, H, P, F, Smr, Sme, Mrt, PIC, PlatMd],
            rungroup_tuple(M, C, O, R, S, G, T, Platform, A, H, P, F, Smr, Sme, Mrt, PIC, PlatMd),
            RunGroups),
    write_term(Stream, RunGroups, [quoted(true)]),
    write(Stream, '\n'),
    findall([M, C, S, IS, D, F], spec_object_file(M, C, S, IS, D, F), ObjectFiles),
    write_term(Stream, ObjectFiles, [quoted(true)]),
    write(Stream, '\n'),
    close(Stream), !.
