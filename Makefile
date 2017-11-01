all:
	pdflatex nlsr-docs
	bibtex nlsr-docs
	pdflatex nlsr-docs
	mv nlsr-docs.pdf NLSR-Developers-Guide.pdf
