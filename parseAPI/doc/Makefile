PDFLATEX=/s/texlive-2008/bin/pdflatex
BIBTEX=/s/texlive-2008/bin/bibtex
all:
	${PDFLATEX} parseapi
	${BIBTEX} parseapi
	${PDFLATEX} parseapi
	${PDFLATEX} parseapi
	acroread parseapi.pdf

clean:
	rm -f parseapi.aux parseapi.bbl parseapi.blg parseapi.dvi parseapi.log parseapi.pdf parseapi.ps
