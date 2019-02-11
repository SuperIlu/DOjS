makeinfo --no-split grx2.tex
makeinfo --html --no-split grx2.tex
d2u grx*
texi2dvi grx2.tex
dvips grx2
d2u grx2.ps
texi2dvi --pdf grx2.tex
