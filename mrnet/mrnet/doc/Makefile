XSLSTYLE=/scratch/darnold/local/share/docbook-xsl-1.68.1/html/docbook.xsl

UG.html: *.xml
	xsltproc --output $@ --catalogs  ${XSLSTYLE} UG.xml

clean:
	/bin/rm -f *~
