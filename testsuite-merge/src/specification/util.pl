% count(+From, +To, ?Value)
% unifies Value with the integers between From and To, inclusive
count(F,T,F) :- F =< T.
count(F,T,X) :- F < T, G is F + 1, count(G, T, X).

% int_list(+From, +To, ?List) unifies List with a list of integers between
% From and To, inclusive.
int_list(From, To, List) :-
    findall(Int, count(From, To, Int), List).

% Eureka!  My generic list iterator!
% for_each(+List, +Variable, +Callable) unifies Variable with each element
% in List in turn, and then calls the term Callable with that binding, before
% backtracking, undoing the binding, and proceeding to the next element in the
% list.
for_each([], _, _).
for_each([Head | Tail], Variable, Callable) :-
    (
        Variable = Head,
        call(Callable),
        fail
    );
    for_each(Tail, Variable, Callable).

% I need a variant for_each that can do something different for the last
% element in a list.  Or a for_each_but_last.
% for_each/4
% for_each(+List, +Variable, +Callable1, +Callable2) unifies Variable with each
% element in List in turn, and calls the term Callable1 with that binding for
% all elements in List except for the last element.  for_each/4 calls Callable2
% with Variable bound to the last element in List, after all the other elements
% in the list have been handled.
for_each([], _, _, _).
for_each([Last], Variable, Callable1, Callable2) :-
	(
		Variable = Last,
		call(Callable2),
		fail
	);
	for_each([], Variable, Callable1, Callable2).
for_each([Head | Tail], Variable, Callable1, Callable2) :-
	Tail \= [],
	(
		Variable = Head,
		call(Callable1),
		fail
	);
	for_each(Tail, Variable, Callable1, Callable2).

% strip_extension/2
% strip_extension(+Atom, ?Stripped)
strip_extension(Atom, Stripped) :-
	findall(B, sub_atom(Atom, B, _, _, '.'), Bs),
	max_list(Bs, M),
	atom_chars(Atom, Achars),
	prefix(Aprefix, Achars),
	length(Aprefix, M),
	atom_chars(Stripped, Aprefix).

% extension/2
% extension(+Atom, ?Ext)
extension(Atom, Ext) :-
	findall(A, sub_atom(Atom, _, _, A, '.'), As),
	min_list(As, M1),
	M is M1 + 1,
	atom_chars(Atom, Achars),
	suffix(Asuffix, Achars),
	length(Asuffix, M),
	atom_chars(Ext, Asuffix).

% merge_lists/2
merge_lists([], []).
merge_lists([List], List).
merge_lists([List | Rest], Merged) :-
	merge_lists(Rest, M1),
	append(List, M1, Merged).
	

% replace_chars_str/4
% replace_char_str(Str1, Str2, +Char1, +Char2)
replace_chars_str(Str1, Str2, Char1, Char2) :-
	atom_chars(Str1, Str1l),
	replace_chars_list(Str1l, Str2l, Char1, Char2),
	atom_chars(Str2, Str2l).

replace_chars_list([], [], _, _).
replace_chars_list([C1 | T1], [C2 | T2], C1, C2) :-
	replace_chars_list(T1, T2, C1, C2).
replace_chars_list([H1 | T1], [H1 | T2], C1, C2) :-
	H1 \= C1,
	replace_chars_list(T1, T2, C1, C2).

% write_list(+List, +Separator, +Stream)
write_list([], _, _).
write_list([I], _, Stream) :-
    format(Stream, '~a', [I]).
write_list([H|T], Separator, Stream) :-
    T \= [],
    format(Stream, '~a~a', [H, Separator]),
    write_list(T, Separator, Stream).

% write_list(+List, +Prefix, +Separator, +Suffix, +Stream)
write_list([], _, _, _, _).
write_list([Item], Prefix, _, Suffix, Stream) :-
    format(Stream, '~a~a~a', [Prefix, Item, Suffix]).
write_list([Head | Tail], Prefix, Separator, Suffix, Stream) :-
    Tail \= [],
    format(Stream, '~a~a~a', [Prefix, Head, Separator]),
    write_list_(Tail, Separator, Suffix, Stream).
% Utility predicate used by write_list/5
write_list_([I], _, Suffix, Stream) :-
    format(Stream, '~a~a', [I, Suffix]).
write_list_([Head | Tail], Separator, Suffix, Stream) :-
    Tail \= [],
    format(Stream, '~a~a', [Head, Separator]),
    write_list_(Tail, Separator, Suffix, Stream).

% map doesn't work
% map(+List, -Variable, -Template, +Callable, ?Out)
% Collects the results (from Template) of calling Callable with Variable
% bound to each of the values in List
map([], _, _, _, []).
map([Head | Tail], Variable, Template, Callable, Out) :-
    (
        map(Tail, Variable, Template, Callable, Recursive_results),
        fail
    );
    Variable = Head,
    findall(Template, Callable, Results),
    append(Results, Recursive_results, Out).
        
    
% replace_suffix/4
% replace_suffix(+In, +Old, +New, ?Out)
% replace_suffix(?In, +Old, +New, +Out)
% Unifies Out with the atom In, after replacing the suffix Old in In with New
% If no matching suffix is found, the predicate fails.
% replace_suffix(+In, +Old, +New, ?Out)
replace_suffix(In, Old, New, Out) :-
    nonvar(In), nonvar(Old), nonvar(New),
    atom_length(Old, Old_length),
    sub_atom(In, Root_length, Old_length, 0, Old),
    sub_atom(In, 0, Root_length, Old_length, Root),
    atom_concat(Root, New, Out).
% replace_suffix(?In, +Old, +New, +Out)
replace_suffix(In, Old, New, Out) :-
    var(In), nonvar(Old), nonvar(New), nonvar(Out),
    replace_suffix(Out, New, Old, In).

removedups([], []).
%removedups([H|T], Result) :- member(H, T), removedups(T, Result), !.
removedups([H|T], [H|T1]) :- (member(H, T) -> removedups(T, [H | T1]); removedups(T, T1)).
