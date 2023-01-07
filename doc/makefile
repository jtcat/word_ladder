CSRCDIR= ..

CSRC= $(addprefix $(CSRCDIR)/,solution_word_ladder.c)

DOC=relatorio.pdf

TEXSRC= relatorio.tex

ENG= pdflatex

all: clean pdf

$(DOC):

pdf: $(DOC) $(TEXSRC) $(CSRC)
	$(ENG) -shell-escape $(TEXSRC)

clean:
	rm -f $(DOC) *.toc *.lof *.aux *.blg *.out *.xml *.bbl *.log *-blx.bib
