
all: lac2010_faust_in_action.pdf

lac2010_faust_in_action.pdf: *.tex *.bib
	pdflatex lac2010_faust_in_action
	bibtex lac2010_faust_in_action
	pdflatex lac2010_faust_in_action
	pdflatex lac2010_faust_in_action

clean:
	rm -f *.aux *.bbl *.blg *.log *~

