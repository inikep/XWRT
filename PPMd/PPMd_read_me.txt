        1. Short DESCRIPTION.
    PPMd program is  file-to-file compressor, it  is written for  embedding in
user  programs  mainly  and  it  is  not  intended  for  immediate  use. I was
interested in speed and performance  improvements of abstract PPM model  [1-6]
only, without tuning it to  particular data types, therefore compressor  works
good  enough  for  texts,  but  it  is  not  so  good for nonhomogeneous files
(executables) and for  noisy analog data  (sounds, pictures etc.).  Program is
very memory  consuming, You  can choose  balance between  execution speed  and
memory economy,  on one  hand, and  compression performance,  on another hand,
with the help of model order selection option (-o).
	Methods of restoration of model correctness at memory insufficiency:
    '-r0 - restart model  from scratch'.  This  method is not optimal  for any
type of data sources, but it works fast and efficient in average.
    '-r1 - cut off model'. This method is optimal for quasistationary  sources
when the period  of stationarity is  much larger than  period between cutoffs.
As a rule,  it gives better  results, but it  is slower than  '-r0' and it  is
unstable against  fragmentation of  memory heap  at high  model orders and low
memory.

        2. Distribution CONTENTS.
    read_me.txt        - this file;
    PPMd.h, PPMdType.h - header files;
    Coder.hpp, SubAlloc.hpp, Model.cpp, PPMd.cpp - code sources;
    makefile.imk       - makefile for Intel C/C++ compiler;
    makefile.mak       - makefile for Borland C/C++ compiler;
    makefile.L64.gmk   - makefile for GNU C/C++ compiler under Linux64;
    makefile.L64.imk   - makefile for Intel C/C++ compiler under Linux64;

        3. LEGAL stuff.
    You can not misattribute authorship on algorithm or code sources,  You can
not patent algorithm or its parts, all other things are allowed and welcomed.
    Dmitry Subbotin  and me  have authorship  rights on  code sources.  Dmitry
Subbotin owns authorship rights on  his variation of rangecoder algorithm  and
I own authorship rights  on my variation of  PPM algorithm. This variation  is
named  PPMII  (PPM  with  Information  Inheritance).  If  You  use  PPMd,  our
authorship must be mentioned somewhere in documentation on your program.
    PPMonstr  program  is  distributed  for  experiments and noncommercial use
only.

        4. DIFFERENCES between variants.
        Jun 13, 1999  var.A
    Initial release;
        Jun 30, 1999  var.B
    Arithmetic coder was changed to newer version;
    Simplified LOE was tested (Model1.cpp file);
    Some small improvements were done;
        Aug 22, 1999  var.C
    Rudimentary SEE was added;
    Some small improvements were done;
        Oct  6, 1999  var.D
    Inherited probabilities (IPs) were added;
    Memory requirements were reduced a bit;
    Small improvements were continued;
        Dec  3, 1999  var.E
    Program  name  was  changed  from  PPM,  escape  method D (PPMD) to PPM by
Dmitry (PPMd). Pronounce correctly! ;-)
    Bug in ARI_FLUSH_ENCODER was crushed;
    Model1.cpp file was removed from package due to LOE gives no gain;
        Apr  7, 2000  var.F(inal?)
    Michael   Schindler`s   rangecoder   implementation   was   replaced  with
'carryless rangecoder' by  Dmitry Subbotin.   Now, PPMd is  pure public domain
program;
    CC rate - 2.123/2.089bpb;
        Nov 26, 2000  var.G(rand final)
    Memory requirements were reduced;
    CC rate - 2.121/2.056bpb;
        Apr 21, 2001  var.H(ard run to final)
    Memory requirements were reduced a bit;
    CC rate - 2.104/2.041bpb;
        Apr 28, 2002  var.I(t is final too)
    CutOff and Freeze modes were added;
    References to papers were corrected;
    CC rate - 2.097/1.963bpb;
        Apr 30, 2002  var.I rev.1
    One  defect  in  PPMonstr  was  fixed,  this  revision  of PPMonstr is not
compatible with previous one;
        Feb ??, 2006  var.J(ust a final)
    Support for 64-bit processors and multi-threading was added;
    CC rate - 2.093/1.891bpb;

        5. REFERENCES.
    [1] Excellent  introductory  review  T.Bell,  I.H.Witten, J.G.Cleary
'MODELING FOR TEXT COMPRESSION'. Russian translation is placed at
http://www.compression.ru/download/articles/rev_univ/modeling.rar
    [2] Very descriptive M.R.Nelson`s COMP-2 program (PPMd is based on it).
COMP-2 is in http://www.compression.ru/download/articles/ppm/
nelson_1991_ddj_arithmetic_modeling_html.rar
    [3] P.G.Howard PhD thesis 'The Design and Analysis of Efficient Lossless
Data Compression Systems', is available in
http://www.compression.ru/download/articles/ppm/howard_phd_1993_pdf.rar
    [4] S.Bunton PhD thesis 'On-Line Stochastic Processes in Data
Compression', is available in
http://www.compression.ru/download/articles/ppm/bunton_phd_1996_pdf.rar

PPMII algorithm:
    [5] D.Shkarin 'Improving the efficiency of PPM algorithm', in Russian,
http://compression.graphicon.ru/download/articles/ppm/shkarin_ppi2001_pdf.rar
	[6] D.Shkarin 'PPM: one step to practicality', in English,
http://compression.graphicon.ru/download/articles/ppm/
shkarin_2002dcc_ppmii_pdf.rar

    AUTHOR SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL,  INCIDENTAL,
OR CONSEQUENTIAL  DAMAGES ARISING  OUT OF  ANY USE  OF THIS  SOFTWARE. YOU USE
THIS PROGRAM AT YOUR OWN RISK.

                                        Dancy compression!
                                        Dmitry Shkarin
                                        E-mail: dmitry.shkarin@mtu-net.ru
