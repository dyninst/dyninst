TEX=/s/texlive-2010/bin/pdflatex
MAIN=patchapi
DEP=patchapi.tex \
	section/1_intro.tex \
	section/2_abs.tex \
	section/3_example.tex \
	section/4_api.tex \
	section/5_dyninst.tex \

$(MAIN).pdf: $(DEP)
	$(TEX) $(MAIN)
	$(TEX) $(MAIN)

clean:
	@rm -f *.dvi $(MAIN).pdf *.log *.aux *.ent *.bak *.bbl *.blg *.out *.toc

read:
	@xpdf patchapi.pdf
