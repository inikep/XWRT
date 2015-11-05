Introduction
-----------------

XWRT (XML-WRT) is an efficient XML/HTML compressor (actually it works well with all textual files). 
It transforms XML to more
compressible form and uses zlib (default), LZMA, PPMd, or lpaq6 as 
back-end compressor. This idea is based on well-known XML compressor - XMill.
Moreover, XML-WRT creates a semi-dynamic dictionary and replaces frequently 
used words with shorter codes. There are additional techniques to improve
compression ratio:
- word alphabet can consist of start tags (like '<tag>'), urls, e-mails
- special model for numbers encoding
- input XML file is split into containers
- there are special containers for dates, time, pages and fractional numbers
- end tags ('</tag>') are replaced with a single char
- end tags + EOL symbols can also be replaced with a single char
- spaceless words model
- very effective methods for white-space preserving
- quotes modeling ('="' and '">' replaced with a single char) 




Usage
-----------------

```
Usage: XWRT.exe [options] <file1> [file2] [file3] ...
 where <file> is a XML file or a XWRT compressed file (it's auto-detected)
             you can also use wildcards (e.g., "*.xml")
 GENERAL OPTIONS (which also set default additional options):
  -i = Delete input files
  -l0 = no compression (memory usage up to 16 MB)
  -l1 = zlib fast (memory usage 16+1 MB)
  -l2 = zlib normal (default, memory usage 16+1 MB)
  -l3 = zlib best (memory usage 16+1 MB)
  -l4 = LZMA dict size 64 KB (memory usage 16+9 MB for compression and 16+3 MB for decompression)
  -l5 = LZMA dict size 1 MB (memory usage 16+18 MB for compression and 16+3 MB for decompression)
  -l6 = LZMA dict size 8 MB (memory usage 16+84 MB for compression and 16+10 MB for decompression)
  -l7 = PPMd model size 16 MB (memory usage 16+20 MB)
  -l8 = PPMd model size 32 MB (memory usage 16+36 MB)
  -l9 = PPMd model size 64 MB (memory usage 16+70 MB)
  -l10 = lpaq6 model size 120 MB (memory usage 16+104 MB)
  -l11 = lpaq6 model size 214 MB (memory usage 16+198 MB)
  -l12 = lpaq6 model size 406 MB (memory usage 16+390 MB)
  -l13 = lpaq6 model size 790 MB (memory usage 16+774 MB)
  -l14 = lpaq6 model size 1560 MB (memory usage 16+1542 MB)

  -o = Force overwrite of output files
 ADDITIONAL OPTIONS:
  -bX = Set maximum buffer size while creating dynamic dictionary to X MB
  -c = Turn off containers (without number and word containers)
  +d = Turn on usage of the static dictionary (requires wrt-eng.dic,
       which is available at http://pskibinski.pl/research)
  -eX = Set maximum dictionary size to X words
  -fX = Set minimal word frequency to X
  -mX = Set maximum memory buffer size to X MB (default=8)
  -n = Turn off number containers
  -pX = Preprocess only (file_size/X) bytes in a first pass
  -s = Turn off spaces modeling option
  -t = Turn off "try shorter word" option
  -w = Turn off word containers
```


Used libraries
---------------------
```
zlib  (C) 1995-2005 Jean-loup Gailly and Mark Adler 
LZMA  (C) 1999-2006 Igor Pavlov
PPMd (C) 1997-2006 Dmitry Shkarin
lpaq6  (C) 2007 Matt Mahoney and Alexander Ratushnyak
```
