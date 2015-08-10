\begin{figure}
\centering
\tikzstyle{class} = [rectangle, draw, minimum width=4cm, minimum height=1.5em,
font=\ttfamily, node distance=1.5em]
\tikzstyle{top} = [rectangle, minimum width=4cm, draw, minimum height=1.5em,
fill=white, draw=white]
\tikzstyle{annotate} = [rectangle]
\tikzstyle{code} = [rectangle, font=\ttfamily]
\tikzstyle{line} = [draw, -latex']
\begin{tikzpicture}[
        every text node part/.style={align=left}
    ]

    % Label
    \node [annotate] (callstack) {\textbf{Call Stack}};

    % The callstack
    \node [top, below of=callstack] (dots) {...};
    \node [class, below of=dots] (a) {a};
    \node [class, below of=a] (b) {b};
    \node [class, below of=b] (mainRA) {X's RA(0x400000c)};
\node [class, below of=mainRA] (mainFP) {X's FP};
    \node [class, below of=mainFP] (c) {c};
    \node [class, below of=c] (fooRA) {main's RA(0x400010c)};
    \node [class, below of=fooRA] (fooFP) {main's FP};
    \node [class, below of=fooFP] (d) {d};
    \node [class, below of=d] (barRA) {foo's RA(0x400020c)};
    \node [class, below of=barRA] (barFP) {foo's FP};

    % Registers
    \node [annotate, below=1cm of barFP] (registers) {Registers};
    \node [class, below of=registers] (FramePointer) {Frame Pointer};
    \node [class, below of=FramePointer] (StackPointer) {StackPointer};
    \node [class, below of=StackPointer] (ProgramCounter) {Program Counter};

    % Left labels
    \node [code, left=7.5cm of dots,anchor=north] (main) {
        0x4000100\hspace{0.5cm}void main() \{\\
        0x4000104\hspace{1cm}int a;\\
        0x4000108\hspace{1cm}foo(0);\\
        0x400010c\hspace{1cm}...\\
        \hspace{2.2cm}\}
    };

    \node [code, above=2.5cm of main.west, anchor=west](callmain){
        0x4000000\hspace{0.5cm}\_\_X:\\
        0x400000x\hspace{1cm}...\\
        0x4000008\hspace{1cm}jmp main\\
        0x400000c\hspace{1cm}...\\
    };

    \node [code, above=1.5cm of callmain.west, anchor=west](codeTitle){
        pseudo line number\\and code\\
    };
    
    \node [code, below=3cm of main.west,anchor=west] (foo) {
        0x4000200\hspace{0.5cm}void foo(int b) \{\\
        0x4000204\hspace{1cm}int c;\\
        0x4000208\hspace{1cm}bar();\\
        0x400020c\hspace{1cm}...\\
        \hspace{2.2cm}\}
    };

    \node [code, below=3cm of foo.west,anchor=west] (bar) {
        0x4000300\hspace{0.5cm}void bar() \{\\
        0x4000304\hspace{1cm}int d;\\
        0x4000308\hspace{1cm}while(1);\\
        \hspace{2.2cm}\}
    };

    % Callstack->callstack labels
    \path [line] (mainFP.east) [bend right=20] to (dots.east);
    \path [line] (fooFP.east) [bend right=40] to (mainFP.east);
    \path [line] (barFP.east) [bend right=40] to (fooFP.east);
    \path [line] (FramePointer.east) [bend right=40] to (barFP.east);
    \path [line] (StackPointer.east) [bend right=40] to (barFP.east);

    % Right labels
    \node [code, left=2.5cm of dots,anchor=south] (mainFO) {main\textnormal{'s}\\ Frame Object};
    \node [code, below=.25em of mainFO] (mainFPlabel) {FP};
    \path [line] (mainFPlabel) -- (mainFP.west);
    \node [code, below=.25em of mainFPlabel] (mainSPlabel) {SP};
    \path [line] (mainSPlabel) -- (mainFP.west);
    \node [code, below=.25em of mainSPlabel] (mainRAlabel) {RA};
    \path [line, dashed] (mainRAlabel) -- (fooRA.west);

    \node [code, below=2cm of mainFO] (fooFO) {foo\textnormal{'s}\\ Frame Object};
    \node [code, below=.25em of fooFO] (fooFPlabel) {FP};
    \path [line] (fooFPlabel) -- (fooFP.west);
    \node [code, below=.25em of fooFPlabel] (fooSPlabel) {SP};
    \path [line] (fooSPlabel) -- (fooFP.west);
    \node [code, below=.25em of fooSPlabel] (fooRAlabel) {RA};
    \path [line, dashed] (fooRAlabel) -- (barRA.west);

    \node [code, below=2cm of fooFO] (barFO) {bar\textnormal{'s}\\ Frame Object};
    \node [code, below=.25em of barFO] (barFPlabel) {FP};
    \path [line, dashed] (barFPlabel) -- (FramePointer.west);
    \path [line] (barFPlabel) -- (barFP.west);
    \node [code, below=.25em of barFPlabel] (barSPlabel) {SP};
    \path [line, dashed] (barSPlabel) -- (StackPointer.west);
    \path [line] (barSPlabel) -- (barFP.west);
    \node [code, below=.25em of barSPlabel] (barRAlabel) {RA};
    \path [line, dashed] (barRAlabel) -- (ProgramCounter.west);

    % Callstack->source code labels
    \path [line] (mainRAlabel.west) -- ($(main.south east) + (-.75cm, 2em)$);
    \path [line] (fooRAlabel.west) -- ($(foo.south east) + (-1.5cm, 2em)$);
    \path [line] (barRAlabel.west) -- ($(bar.south east) + (0, 2em)$);


\end{tikzpicture}

\begin{tikzpicture}[]
    % Legend
    \node [annotate] (la1) {A};
    \node [annotate, right of=la1] (lb1) {B};
    \path [line] (la1) -- (lb1);
    \node [annotate, right=1cm of lb1] (l1) {A contains B's address};
    
    \node [annotate, below=.5cm of la1] (la2) {A};
    \node [annotate, right of=la2] (lb2) {B};
    \path [line, dashed] (la2) -- (lb2);
    \node [annotate, right=1cm of lb2] (l2) {A contains the contents of B};

    % Legend Box
    \node [rectangle, draw, fit=(la1)(lb1)(l1)(la2)(lb2)(l2)] (legendBox) {};
    \node [rectangle, label=left: \rotatebox{90}{LEGEND},fit=(legendBox)] {};
\end{tikzpicture}

\caption{Stack Frame and Frame Object Layout (ARMv8 Architecture)}
\label{fig:layout-armv8}

\end{figure}


