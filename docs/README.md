# Dyninst manuals

Nine LaTeX manuals, one per component, built to PDF by the CMake build.

`v13.0.0_manuals/` holds the PDFs as released for 13.0.0.  They are a
snapshot, not a build product: nothing regenerates them, and a manual edited
after the release is not reflected there until someone rebuilds and replaces
them.  Build from source for anything current.

## Directory structure

```
docs/
  README.md                       this file
  CMakeLists.txt                  adds each manual's directory
  v13.0.0_manuals/                the PDFs as released for 13.0.0
    <module>.pdf
  common/manual-latex/            shared by every manual
    manual_commands.tex           preamble, \lstset and the \apient macros
    manual_frontpage.tex          title page
    paradyn_logo.pdf              one copy, found through \graphicspath
  <module>/manual-latex/
    CMakeLists.txt                dyninst_add_manual(<module>)
    <module>.tex                  the document; the name must match the directory
    *.tex                         its chapters, \input from <module>.tex
    API/*.tex                     one file per documented class
    examples/*.C                  sources the manual typesets, and compiles
    fig/                          figures, where a manual has them
```

The machinery is in `cmake/DyninstDocs.cmake`, which defines the two
functions a manual calls, and `cmake/DyninstRunLaTeX.cmake`, which runs
pdflatex to a fixed point and checks what it produced.

## Building

Needs `pdflatex`.  On Debian or Ubuntu:

```
apt install texlive-latex-base texlive-latex-recommended texlive-latex-extra \
            texlive-fonts-recommended texlive-pictures ghostscript
```

on Fedora:

```
dnf install texlive-collection-latexrecommended \
            texlive-collection-fontsrecommended \
            texlive-collection-pictures ghostscript
```

Ghostscript is optional; without it the page-boundary check is skipped.

```
cmake -S . -B build -DDYNINST_BUILD_DOCS=ON
cmake --build build --target docs
```

PDFs land beside their sources in the build tree, at
`build/docs/<module>/manual-latex/<module>.pdf`, and install to
`<prefix>/share/doc/Dyninst/`.

### Targets

| target                  | description |
|:------------------------|:------------|
| `docs`                  | every manual |
| `docs-install`          | every manual, then install them |
| `<module>.pdf`          | one manual |
| `<module>-doc-listings` | compile the example sources that manual typesets |

Every target exists whatever the options are set to, so any of them can be
asked for by name.  What the options decide is which ones an ordinary build
runs.

`docs` and `<module>.pdf` are in `all` only when `DYNINST_BUILD_DOCS` is on.
`docs-install` never is: ask for it by name.

`<module>-doc-listings` is in `all` when `DYNINST_DOCS_VALIDATE_LISTINGS` is
on, which it is by default, and whether or not `DYNINST_BUILD_DOCS` is set --
so an ordinary build with no LaTeX anywhere still compiles the examples.
The same option makes it a prerequisite of `<module>.pdf`, so asking for one
manual compiles that manual's examples first.  With the option off there is
no such edge, and `<module>.pdf` builds without them.

### What gets checked

Four things, governed separately.  `DYNINST_WARNINGS_AS_ERRORS` decides
whether any of them fails the target or is only printed.

| check | when |
|:------|:-----|
| warnings in the LaTeX log | always |
| overfull `\hbox`es wider than `MAX_OVERFULL_PT`, and every overfull `\vbox` | always |
| the finished PDF measured for ink past the paper's edge | when Ghostscript is found |
| the example sources compiled against the library's headers | `DYNINST_DOCS_VALIDATE_LISTINGS` |

### Options

| option | default | description |
|:-------|:--------|:------------|
| `DYNINST_BUILD_DOCS` | `OFF` | Put the manuals in `all`, so they build by default and `install` installs them.  Requires pdflatex: configuring fails if it is missing. |
| `DYNINST_DOCS_VALIDATE_LISTINGS` | `ON` | Compile the example sources a manual typesets, with the warning flags the library itself uses.  Puts `<module>-doc-listings` in `all`, whether or not `DYNINST_BUILD_DOCS` is set, and makes each a dependency of its `<module>.pdf`.  Off, nothing runs them on its own, but the targets remain and can be asked for by name. |
| `DYNINST_DOCS_FORCE_VALIDATE` | `OFF` | Require Ghostscript, so the page-boundary check cannot be skipped.  The check itself runs whenever Ghostscript is found; this makes a missing one a configure error rather than a silent omission. |
| `DYNINST_WARNINGS_AS_ERRORS` | `OFF` | Fail the target on a LaTeX warning or an example's compiler warning, rather than printing it.  Existing option, shared with the C++ build. |
| `DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS` | `OFF` | Report every warning and every box, ignoring `ALLOW_WARNINGS` and `MAX_OVERFULL_PT`.  Existing option, shared with the C++ build. |
| `DYNINST_DOCS_INSTALL_DIR` | `${CMAKE_INSTALL_DOCDIR}` | Where the manuals install, by default `<prefix>/share/doc/Dyninst`. |

## Adding a manual

Make `docs/<module>/manual-latex/` with `<module>.tex` in it.  The file name
has to match the directory name: `dyninst_add_manual` derives the target
`<module>.pdf` from it, and fails at configure time if it is not there.

Add `docs/<module>/manual-latex/CMakeLists.txt`:

```cmake
dyninst_add_manual(<module>)
dyninst_validate_listings(<module> SOURCES examples/Examples-check.C)
```

and a line in `docs/CMakeLists.txt`:

```cmake
add_subdirectory(<module>/manual-latex)
```

Nothing else: the shared preamble, the dependency scanning, the log checks
and the install rule all follow from `dyninst_add_manual`.  Dependencies are
over-approximated by globbing the manual's directory, so a new chapter needs
no build-system change, only an `\input`.

### Per-manual options

```
dyninst_add_manual(<module> [ALLOW_WARNINGS <regex>] [MAX_OVERFULL_PT <n>])
```

`ALLOW_WARNINGS` is a regular expression for log warnings this manual is
known to produce and cannot avoid.  Use it sparingly and say why; it is
matched unanchored against each warning, so a loose pattern hides more than
it was meant to.  `instructionAPI` is the only manual that sets one today.

`MAX_OVERFULL_PT` is the width past which an overfull box is reported,
defaulting to `72.27` — the margin at `margin=1in`, the point at which a line
stops protruding into the margin and starts running off the paper.

### Example sources

Anything a manual typesets with `\lstinputlisting` is also compiled, so an
example that stops matching the API it documents fails the build rather than
the reader's first attempt.  Pass those sources to `dyninst_validate_listings`.

An excerpt usually will not compile on its own, lacking the headers and the
handles the prose takes as given.  The convention is a harness beside it —
`examples/Examples-check.C` — that supplies the context and `#include`s each
excerpt in turn.  A harness is not itself typeset.  Handles the examples
assume should be function parameters rather than null locals: a compiler that
can see the null reports every call through it as `-Wnonnull`.
