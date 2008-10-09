XSLSTYLE=/s/docbook-xsl-1.68.0/common/xml/html/docbook.xsl

UG.html: *.xml
	xsltproc --output $@ --catalogs  ${XSLSTYLE} UG.xml

clean:
	/bin/rm -f *~
