default: quick_view_slides
	
all: report slides

quick_view: report
	xdg-open report.pdf

quick_view_slides: slides
	xdg-open slides.pdf

report:
	pdflatex report.tex

slides:
	pdflatex slides.tex

clean:
	$(RM) *.aux *.log *.snm *.toc *.out
