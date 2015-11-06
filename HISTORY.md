XWRT 3.4 (5.11.2007)
-PPMVC replaced with PPMd var J1
-added support for LZMA and PPMd back-end compression on Linux

XWRT 3.2 (25.10.2007)
-FastPAQ8 replaced with lpaq6 (compression level 10-14)

XWRT 3.1 (05.06.2007)
-improved support for XML files encoded in UTF-8
-dictionary is compressed using front compression
-added little-endian/big-endian Unicode (UCS-2) support
-non-textual files are compressed/stored without using a filter
-64-bit compiler support

XML-WRT 3.0 (14.09.2006)
-internal PPMVC and FastPAQ8 compression

XML-WRT 2.0 (14.06.2006)
-internal zlib and LZMA compression
-input XML file is split into containers depend on start-tags and end-tags and content under the same tag is sent to the same container
-container for dates in format 1980-02-31 and 01-MAR-1920
-container for times in format 11:30pm
-container for numbers from 1900 to 2155 (years)
-container for pages in format "x-y", where y-x<256, eg. "120-148", "1480-1600"
-container for numbers in format "x-y", eg. "1234-0", "87-623"
-container for two digits after period, eg. "102.00", "12.01"
-container for numbers from 0.0 to 24.9 (one digit after period), eg. "12.0", "9.9"
-urls (statring from "http:"), e-mails (x@y.z), "&uuml;" added to dynamic dictionary

XML-WRT 1.0 (27.03.2006)
-first public release
