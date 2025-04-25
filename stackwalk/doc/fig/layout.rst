\begin{figure}
\centering
\tikzstyle{class} = [rectangle, draw, minimum width=3cm, minimum height=1.5em,
font=\ttfamily, node distance=1.5em]
\tikzstyle{top} = [rectangle, minimum width=3cm, draw, minimum height=1.5em,
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
    \node [class, below of=b] (mainRA) {main's RA};
    \node [class, below of=mainRA] (mainFP) {main's FP};
    \node [class, below of=mainFP] (c) {C};
    \node [class, below of=c] (fooRA) {foo's RA};
    \node [class, below of=fooRA] (fooFP) {foo's FP};
    \node [class, below of=fooFP] (d) {d};

    % Registers
    \node [annotate, below=1cm of d] (registers) {Registers};
    \node [class, below of=registers] (FramePointer) {Frame Pointer};
    \node [class, below of=FramePointer] (ProgramCounter) {Program Counter};
    \node [class, below of=ProgramCounter] (StackPointer) {StackPointer};

    % Left labels
    \node [code, left=6cm of dots,anchor=north] (main) {void main() \{\\
        \hspace{1cm}int a;\\
        \hspace{1cm}foo(0);\\
        \hspace{1cm}...\\
        \}
    };
    
    \node [code, below=3cm of main.west,anchor=west] (foo) {void foo(int b) \{\\
        \hspace{1cm}int c;\\
        \hspace{1cm}bar();\\
        \hspace{1cm}...\\
        \}
    };

    \node [code, below=3cm of foo.west,anchor=west] (bar) {void bar() \{\\
        \hspace{1cm}int d;\\
        \hspace{1cm}while(1);\\
    \}
    };

    % Callstack->callstack labels
    \path [line] (mainFP.east) [bend right=20] to (dots.east);
    \path [line] (fooFP.east) [bend right=40] to (mainFP.east);
    \path [line] (FramePointer.east) [bend right=40] to (fooFP.east);
    \path [line] (StackPointer.east) [bend right=40] to (d.east);

    % Right labels
    \node [code, left=2.5cm of dots,anchor=south] (mainFO) {main\textnormal{'s}\\ Frame Object};
    \node [code, below=.25em of mainFO] (mainFPlabel) {FP};
    \path [line] (mainFPlabel) -- (a.west);
    \node [code, below=.25em of mainFPlabel] (mainRAlabel) {RA};
    \path [line, dashed] (mainRAlabel) -- (mainRA.west);
    \node [code, below=.25em of mainRAlabel] (mainSPlabel) {SP};
    \path [line] (mainSPlabel) -- (mainFP.west);

    \node [code, below=2.5cm of mainFO] (fooFO) {foo\textnormal{'s}\\ Frame Object};
    \node [code, below=.25em of fooFO] (fooFPlabel) {FP};
    \path [line] (fooFPlabel) -- (c.west);
    \node [code, below=.25em of fooFPlabel] (fooRAlabel) {RA};
    \path [line, dashed] (fooRAlabel) -- (fooRA.west);
    \node [code, below=.25em of fooRAlabel] (fooSPlabel) {SP};
    \path [line] (fooSPlabel) -- (fooFP.west);

    \node [code, below=2.5cm of fooFO] (barFO) {bar\textnormal{'s}\\ Frame Object};
    \node [code, below=.25em of barFO] (barFPlabel) {FP};
    \path [line, dashed] (barFPlabel) -- (FramePointer.west);
    \node [code, below=.25em of barFPlabel] (barRAlabel) {RA};
    \path [line, dashed] (barRAlabel) -- (ProgramCounter.west);
    \node [code, below=.25em of barRAlabel] (barSPlabel) {SP};
    \path [line, dashed] (barSPlabel) -- (StackPointer.west);

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

\caption{Stack Frame and Frame Object Layout (x86 Architecture)}
\label{fig:layout}

\end{figure}


