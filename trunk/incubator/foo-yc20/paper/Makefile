
all: lac2010_yc20.pdf

lac2010_yc20.pdf: *.tex *.bib
	pdflatex lac2010_yc20
	bibtex lac2010_yc20
	pdflatex lac2010_yc20
	pdflatex lac2010_yc20

clean:
	rm -f *.aux *.bbl *.blg *.log *~

