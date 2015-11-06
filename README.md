Introduction
-----------------

XWRT (XML-WRT) is an efficient XML/HTML compressor (actually it works well with all textual files). 
It transforms XML to more compressible form and uses zlib (default), LZMA, PPMd, or lpaq6 as 
back-end compressor. This idea is based on well-known XML compressor: XMill.
Moreover XML-WRT creates a semi-dynamic dictionary and replaces frequently 
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


Comparision to other XML compressors
-------------------------

All files used for comparision can be downloaded from [Wratislavia XML Corpus]. Results are given in bpc (bits ber character). Tested with XWRT 3.1: 

|file       |gzip |XMill 0.9|zip|XWRT -l2 (gzip)|LZMA -a1|XWRT -l6 (LZMA)|PPMdJ -o8 -m64|XMill 0.9 PPMd|XMLPPM -l 9|SCMPPM -l 9|XWRT -l9 (PPM)|FastPAQ8 74 MB|XWRT -l11 (FastPAQ8)|
|-----------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
|dblp       |1.463|1.250|0.865|0.943|0.747|0.724|0.940|0.802|0.693|0.690|0.659|0.597|
|enwikibooks|2.339|2.295|1.742|1.686|1.504|1.565|1.838|1.621|1.621|1.481|1.357|1.269|
|enwikinews |2.248|2.198|1.597|1.462|1.301|1.291|1.746|1.379|1.398|1.202|1.172|1.090|
|lineitem   |0.721|0.380|0.276|0.421|0.243|0.359|0.270|0.261|0.242|0.243|0.236|0.226|
|Shakespeare|2.182|2.044|1.481|1.646|1.349|1.245|1.584|1.295|1.293|1.204|1.220|1.185|
|SwissProt  |0.985|0.619|0.475|0.478|0.388|0.490|0.477|0.416|0.417|0.363|0.395|0.313|
|uwm        |0.553|0.382|0.315|0.368|0.278|0.426|0.310|0.259|0.274|0.240|0.254|0.228|
|average    |1.499|1.310|0.964|1.001|0.830|0.871|1.024|0.862|0.848|0.775|0.756|0.701|

[Wratislavia XML Corpus]: http://pskibinski.pl/research/Wratislavia/



Usage
-----------------

```
Usage: XWRT.exe [options] <file1> [file2] [file3] ...
 where <file> is a XML file or a XWRT compressed file (it's auto-detected)
             you can also use wildcards (e.g., "*.xml")

GENERAL OPTIONS (which also set default additional options):
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
  -0 = preprocessed and uncompressed output optimized for further LZ77 compression
  -1 = preprocessed and uncompressed output optimized for further LZMA compression
  -2 = preprocessed and uncompressed output optimized for further PPM compression
  -3 = preprocessed and uncompressed output optimized for further PAQ compression

ADDITIONAL OPTIONS:
  -bX = Set maximum buffer size while creating dynamic dictionary to X MB
  -c = Turn off containers (without number and word containers)
  +d = Turn on usage of the static dictionary (requires wrt-eng.dic,
       which is available at http://pskibinski.pl/research)
  -eX = Set maximum dictionary size to X words
  -fX = Set minimal word frequency to X
  -i = Delete input files
  -mX = Set maximum memory buffer size to X MB (default=8)
  -n = Turn off number containers
  -o = Force overwrite of output files
  -pX = Preprocess only (file_size/X) bytes in a first pass
  -s = Turn off spaces modeling option
  -t = Turn off "try shorter word" option
  -w = Turn off word containers
```


Compilation
-------------------------
For Linux/Unix:
```
make BUILD_SYSTEM=linux
```

For Windows (MinGW)
```
make
```


Used libraries
---------------------
```
zlib  (C) 1995-2005 Jean-loup Gailly and Mark Adler 
LZMA  (C) 1999-2006 Igor Pavlov
PPMd (C) 1997-2006 Dmitry Shkarin
lpaq6  (C) 2007 Matt Mahoney and Alexander Ratushnyak
```
