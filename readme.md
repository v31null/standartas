Hírù gùkríbngaín uq standartas Íika , i.e./ .v31np & .v31n formatas , úír kípamíț čí Mílksùns‑maíțúčmíțas.

![Íik](nżúù/eayk.png)

Þis File‑format is þͤ Best‑File‑format in þͤ World File‑format‑‘istorie , All‑so ; I am It’s kreator.


***

<table>
<thead>
<tr><th colspan="2">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td>1. Report No.<br>PS-N/PF-1031 R.2</td><td>2. Report Date<br>2026/10/31</td></tr>
<tr><td colspan="2">3. Title<br>V31NP STANDARD</td></tr>
<tr><td>4. Author(s)<br>P.K.</td><td>5. Performing Organization<br>v31null, in its official capacity for all v31n services where applicable</td></tr>
<tr><td colspan="2">6. Abstract<br>This document defines the .v31np file format and the V31NP STANDARD. A .v31np file holds any number of files together with their paths, stored raw behind a header of two tables and closed by a single CRC-32 over the whole file. The document sets out the layout, fields, constraints and properties of the format, describes the programmes that support it, namely the native systems of v31n, the Windows Explorer extension and the 7-Zip plugin, rates each against the standard, and sets out considerations for developers concerning storage, compression, nested archives, metadata, the marks, the checksum and networking.</td></tr>
<tr><td>7. Key Words<br>.v31np, V31NP STANDARD, file format, archive, header, π, φ, CRC-32, UTF-8, byte range, naturalisation, Windows Explorer, 7-Zip</td><td>8. Distribution Statement<br>No restrictions.</td></tr>
<tr><td colspan="2">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td></tr>
</tbody>
</table>

# ( A ) INTRODUCTION

## ( a ) Table of Contents

- [( A ) INTRODUCTION](#aintroduction)
  - [( b ) List of Figures](#blist-of-figures)
  - [( ú ) List of Tables](#úlist-of-tables)
  - [( u ) Notice](#unotice)
  - [( ù ) Foreword](#ùforeword)
  - [( g ) Scope and Purpose](#gscope-and-purpose)
  - [( d ) Notation](#dnotation)
- [( B ) THE FORMAT](#bthe-format)
  - [( a ) Definition](#adefinition)
    - [( i ) Layout](#ilayout)
    - [( ii ) Rows](#iirows)
    - [( iii ) Fields](#iiifields)
    - [( iv ) Constraints](#ivconstraints)
    - [( v ) Checksum](#vchecksum)
    - [( vi ) Header Length](#viheader-length)
  - [( b ) Properties](#bproperties)
    - [( i ) Supported](#isupported)
    - [( ii ) Unsupported](#iiunsupported)
- [( Ú ) SUPPORTING PROGRAMMES](#úsupporting-programmes)
  - [( a ) Native Systems](#anative-systems)
    - [( i ) Services for þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio](#iservices-for-þe-general-populace-ministeria-custodiae-filaciorum-secura--aßervatio)
    - [( ii ) Services for þe populace General et Private Musik applikation of Per.‑Gen.‑portal nullpunkts as ver. 2 for General use](#iiservices-for-þe-populace-general-et-private-musik-applikation-of-pergenportal-nullpunkts-as-ver-2for-general-use)
    - [( iii ) V31null Hub](#iiiv31null-hub)
  - [( b ) External Third Parties](#bexternal-third-parties)
    - [( i ) Windows Explorer](#iwindows-explorer)
      - [1. Browsing](#1browsing)
      - [2. Commands](#2commands)
      - [3. Editing](#3editing)
      - [4. Extraction](#4extraction)
    - [( ii ) 7-Zip](#ii7-zip)
- [( U ) V31NP STANDARD](#uv31np-standard)
- [( Ù ) CONSIDERATIONS](#ùconsiderations)
  - [( a ) Rewriting and Storage](#arewriting-and-storage)
  - [( b ) Compression](#bcompression)
  - [( ú ) Nested Archives](#únested-archives)
  - [( u ) Metadata](#umetadata)
  - [( ù ) The Marks π and φ](#ùthe-marks-π-and-φ)
  - [( g ) The Checksum](#gthe-checksum)
  - [( d ) Networking](#dnetworking)
- [( G ) GLOSSARY](#gglossary)
- [( D ) REFERENCES](#dreferences)

## ( b ) List of Figures

- [Figure B-1 File Layout](#figure-b-1)
- [Figure B-2 Rows of T1 and T2](#figure-b-2)
- [Figure B-3 Header Length](#figure-b-3)
- [Figure Ù-1 Rewriting on Change](#figure-ù-1)
- [Figure Ù-2 Reading a File](#figure-ù-2)
- [Figure Ù-3 Download and Comparison between Client and Server](#figure-ù-3)
- [Figure Ù-4 Selective Download](#figure-ù-4)
- [Figure Ù-5 Checking a Copy](#figure-ù-5)

## ( ú ) List of Tables

- [Table B-1 File Layout](#table-b-1)
- [Table B-2 Marks](#table-b-2)
- [Table B-3 Fields](#table-b-3)
- [Table B-4 Examples of the Division of a Key](#table-b-4)
- [Table B-5 Bytes of a File Holding No Files](#table-b-5)
- [Table B-6 Bytes of a File Holding One File](#table-b-6)
- [Table B-7 Features of tar, zip and rar without Equivalent in .v31np](#table-b-7)
- [Table Ú-1 Commands](#table-ú-1)
- [Table Ú-2 Naturalisation of Names on Windows](#table-ú-2)
- [Table U-1 Medallions](#table-u-1)

## ( u ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the V31NP STANDARD. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document.

## ( ù ) Foreword

This document has been published to define the .v31np file format and the V31NP STANDARD, and to describe the programmes that read and write .v31np files. It is intended for developers who implement readers or writers of the format, and for those who maintain the services of v31n that exchange .v31np files with one another.

The format is defined in ( B ), which sets out the layout of a file, the rows and fields of its tables, the constraints a file shall satisfy, the checksum, and the properties the format supports and does not support. ( Ú ) describes the programmes that support the format, namely the native systems of v31n, the Windows Explorer extension and the 7-Zip plugin. ( U ) defines the two medallions, PASSING and FAILING, by which each programme in ( Ú ) is rated against the standard. ( Ù ) sets out the considerations that follow from the design of the format, concerning rewriting and storage, compression, nested archives, metadata, the marks, the checksum and networking. ( G ) gives the terms used in this document, and ( D ) the documents to which it refers.

P.K.

## ( g ) Scope and Purpose

A .v31np file, as defined by v31n, is a single file that holds any number of files together with their paths. The data of each file is stored raw, without compression or encryption. A header at the start of the file holds two tables, the first giving the address of each file in data order and the second giving the size of each file and the parts of its path. A checksum at the end of the file covers the whole file. This definition applies to every .v31np file regardless of the programme that writes it or the system that reads it.

The format was created 1) to share a large number of small files faster, 2) to be used in server ⇆ client relationships, 3) to store data safely, 4) to block zip bomb attacks and similar, 5) to secure the interests of v31n, 6) related to 5), to reduce supply-chain risks, 7) to ease the use of file storage, and 8) to create a standard that all services conforming to the V31NP STANDARD can use interchangeably.

Several of these purposes follow directly from the structure of the format. A .v31np file carries no per-file headers and no compression, and the addresses of all files are known from the header before any data is read, which allows many small files to be shared as one. Since the header precedes the data and the data is contiguous, a file can be streamed from a server, resumed by byte range and read at any address; the native systems in ( Ú/( a ) ) rely on this. Names are stored exactly as given and are never altered by a writer, and a single checksum covers header, data and closing mark. Because data is not compressed, the extracted size of each file equals its stored size; sizes and addresses shall chain exactly to the length of the file, and empty folders cannot be stored, so that an archive cannot expand beyond what its header states. The format and its readers and writers are defined and maintained by v31n, which limits the dependence of its services on outside software.

The naming of extracted files on a given operating system is not part of the format. Readers derive such names on output, and the rules applied on Windows are given in ( Ú/( b.i.4. ) ).

## ( d ) Notation

Bytes are written in hexadecimal, two digits to a byte, separated by spaces, as in 24 3F. Offsets, lengths, addresses and sizes are written in decimal; where a stored number is shown, its bytes are followed by its decimal value, as in B7 (183). Numbers are stored in little-endian byte order, the least significant byte first. Powers of two are written as 2⁵².

π and φ denote the marks of Table B-2 and not the constants from which they are taken. 00 φ and 01 φ denote the byte 00 or 01 followed by φ. Keys and names of files and programmes are written as they are stored or given, as in docs/note.txt or v31np.dll.

A reference to a part of this document gives the division followed by the part, as in ( B/( a.iii. ) ), which denotes ( iii ) of ( a ) of ( B ). Tables and figures are numbered by division, as in Table B-6 and Figure Ù-4.

# ( B ) THE FORMAT

## ( a ) Definition

### ( i ) Layout

A .v31np file consists of a header, the data of the files and a footer, in that order. The header begins with a magic of 15 bytes, the ASCII text "V31NSPACKFORMAT", followed by the mark π. Two tables follow, T1 and T2, each closed by π; the π that closes T2 ends the header. The data follows immediately after the header, and the footer consists of a fourth π and a checksum of 4 bytes. All offsets are absolute and are counted from the first byte of the file, and all numbers are unsigned and stored in little-endian byte order. Table B-1 presents the layout of a file, and Figure B-1 illustrates it.

<table id="table-b-1">
<caption>Table B-1 File Layout</caption>
<thead>
<tr><th>Part</th><th>Length (bytes)</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><th colspan="3">Header</th></tr>
<tr><td>Magic</td><td>15</td><td>V31NSPACKFORMAT (ASCII)</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>T1</td><td>variable</td><td>One row per file: 00 φ, key, 01 φ, address</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>T2</td><td>variable</td><td>One row per file: 00 φ, key, 01 φ, size, 01 φ, a, 01 φ, b, 01 φ, c</td></tr>
<tr><td>π</td><td>12</td><td>Mark, end of header</td></tr>
<tr><th colspan="3">Data</th></tr>
<tr><td>Data</td><td>variable</td><td>Contents of the files, raw, concatenated in the order of T1</td></tr>
<tr><th colspan="3">Footer</th></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>4</td><td>Checksum</td></tr>
</tbody>
</table>

<p id="figure-b-1" align="center"><img src="nżúù/b-1.svg" alt="Figure B-1 File Layout"></p>

Figure B-1 File Layout

The two marks are taken from the hexadecimal fractions of the constants π and φ. π divides the parts of the file, while φ divides the rows and fields within a table. Table B-2 gives the bytes of each mark.

<table id="table-b-2">
<caption>Table B-2 Marks</caption>
<thead>
<tr><th>Mark</th><th>Bytes (hexadecimal)</th><th>Origin</th></tr>
</thead>
<tbody>
<tr><td>π</td><td>24 3F 6A 88 85 A3 08 D3 13 19 8A 2E</td><td>Hexadecimal fraction of π, first 12 bytes</td></tr>
<tr><td>φ</td><td>9E 37 79 B9 7F 4A 7C 15 F3 9C C0 60</td><td>Hexadecimal fraction of φ, first 12 bytes</td></tr>
</tbody>
</table>

### ( ii ) Rows

Each table is a sequence of rows, one row per file. A row begins with the byte 00 followed by φ, and each further field of the row is preceded by the byte 01 followed by φ. A table begins directly with its first row, and no byte may stand between the preceding π and the first 00 φ. A table that contains no rows denotes an archive that holds no files.

A row of T1 shall have exactly two fields, and a row of T2 exactly five. Since each file is described once in each table, T1 and T2 shall hold the same number of rows. Figure B-2 illustrates a row of each table for the file of Table B-6.

<p id="figure-b-2" align="center"><img src="nżúù/b-2.svg" alt="Figure B-2 Rows of T1 and T2"></p>

Figure B-2 Rows of T1 and T2

### ( iii ) Fields

Table B-3 presents the fields of both tables. The key is the path of the file as UTF-8 text, in which the character / separates folders. The address is the offset at which the data of the file begins, and the size is the length of that data. The fields a, b and c divide the key into its folder, its name and its extension.

<table id="table-b-3">
<caption>Table B-3 Fields</caption>
<thead>
<tr><th>Field</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><th colspan="2">T1</th></tr>
<tr><td>key</td><td>Path of the file, UTF-8, stored as given; / separates folders</td></tr>
<tr><td>address</td><td>Absolute offset of the first data byte of the file</td></tr>
<tr><th colspan="2">T2</th></tr>
<tr><td>key</td><td>Path of the file, as in T1</td></tr>
<tr><td>size</td><td>Length of the data of the file in bytes</td></tr>
<tr><td>a</td><td>Folder: the key up to and including its last /; empty if the key contains no /</td></tr>
<tr><td>b</td><td>Name: the remainder of the key without c</td></tr>
<tr><td>c</td><td>Extension: the remainder of the key from its last full stop to the end, the full stop included; empty if the remainder contains no full stop</td></tr>
</tbody>
</table>

Address and size are numbers of 1 to 7 bytes without fixed width, and each shall be less than 2⁵². Writers should write each number in its shortest form, so that the number zero is written as the single byte 00.

The division of a key into a, b and c is illustrated in Table B-4. A key without a folder leaves a empty, a name that begins with a full stop leaves b empty, and a name without a full stop leaves c empty. Only the last full stop of the name separates the extension.

<table id="table-b-4">
<caption>Table B-4 Examples of the Division of a Key</caption>
<thead>
<tr><th rowspan="2">Key</th><th colspan="3">Division</th></tr>
<tr><th>a</th><th>b</th><th>c</th></tr>
</thead>
<tbody>
<tr><td>/css/fonts/Frank.ttf</td><td>/css/fonts/</td><td>Frank</td><td>.ttf</td></tr>
<tr><td>photo.tar.gz</td><td>(empty)</td><td>photo.tar</td><td>.gz</td></tr>
<tr><td>src/.gitignore</td><td>src/</td><td>(empty)</td><td>.gitignore</td></tr>
<tr><td>bin/run</td><td>bin/</td><td>run</td><td>(empty)</td></tr>
</tbody>
</table>

### ( iv ) Constraints

A .v31np file shall satisfy the constraints of this section; the checksum is treated separately in ( B/( a.v. ) ).

Each key shall be valid UTF-8. Each key shall occur in T2 only once, and each key of T1 shall occur in T2 exactly once. The fields a, b and c shall divide the key as defined in ( B/( a.iii. ) ), so that their concatenation equals the key.

The order of T1 is the order of the data, while the order of T2 is free. The first address shall equal the length of the header, counted from the first byte of the file through the π that closes T2, and each following address shall equal the previous address plus the size of the previous file. The data of the last file shall end at the offset of the closing π, so that no byte between the header and the footer is left unassigned. The length of the whole file shall be less than 2⁵² bytes.

It follows from these constraints that a .v31np file holding no files is 67 bytes long, consisting of the magic, four π and the checksum. Table B-5 presents the bytes of such a file. Table B-6 presents the bytes of a file holding a single file, of which the key is docs/note.txt and the data the five bytes of the ASCII text Hello.

<table id="table-b-5">
<caption>Table B-5 Bytes of a File Holding No Files</caption>
<thead>
<tr><th>Part</th><th>Offset</th><th>Bytes (hexadecimal)</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><th colspan="4">Header</th></tr>
<tr><td>Magic</td><td>0</td><td>56 33 31 4E 53 50 41 43 4B 46 4F 52 4D 41 54</td><td>Magic, V31NSPACKFORMAT</td></tr>
<tr><td>π</td><td>15</td><td>π</td><td>Mark</td></tr>
<tr><td>π</td><td>27</td><td>π</td><td>Mark, closes T1, which holds no rows</td></tr>
<tr><td>π</td><td>39</td><td>π</td><td>Mark, closes T2, which holds no rows; end of header</td></tr>
<tr><th colspan="4">Footer</th></tr>
<tr><td>π</td><td>51</td><td>π</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>63</td><td>46 AA 21 E5</td><td>Checksum</td></tr>
</tbody>
</table>

<table id="table-b-6">
<caption>Table B-6 Bytes of a File Holding One File</caption>
<thead>
<tr><th>Part</th><th>Offset</th><th>Bytes (hexadecimal)</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><th colspan="4">Header</th></tr>
<tr><td>Magic</td><td>0</td><td>56 33 31 4E 53 50 41 43 4B 46 4F 52 4D 41 54</td><td>Magic, V31NSPACKFORMAT</td></tr>
<tr><td>π</td><td>15</td><td>π</td><td>Mark</td></tr>
<tr><td rowspan="4">T1</td><td>27</td><td>00 φ</td><td>Start of the row</td></tr>
<tr><td>40</td><td>64 6F 63 73 2F 6E 6F 74 65 2E 74 78 74</td><td>Key, docs/note.txt</td></tr>
<tr><td>53</td><td>01 φ</td><td>Start of a field</td></tr>
<tr><td>66</td><td>B7</td><td>Address, 183, the length of the header</td></tr>
<tr><td>π</td><td>67</td><td>π</td><td>Mark, closes T1</td></tr>
<tr><td rowspan="10">T2</td><td>79</td><td>00 φ</td><td>Start of the row</td></tr>
<tr><td>92</td><td>64 6F 63 73 2F 6E 6F 74 65 2E 74 78 74</td><td>Key, docs/note.txt</td></tr>
<tr><td>105</td><td>01 φ</td><td>Start of a field</td></tr>
<tr><td>118</td><td>05</td><td>Size, 5</td></tr>
<tr><td>119</td><td>01 φ</td><td>Start of a field</td></tr>
<tr><td>132</td><td>64 6F 63 73 2F</td><td>a, docs/</td></tr>
<tr><td>137</td><td>01 φ</td><td>Start of a field</td></tr>
<tr><td>150</td><td>6E 6F 74 65</td><td>b, note</td></tr>
<tr><td>154</td><td>01 φ</td><td>Start of a field</td></tr>
<tr><td>167</td><td>2E 74 78 74</td><td>c, .txt</td></tr>
<tr><td>π</td><td>171</td><td>π</td><td>Mark, closes T2; end of header</td></tr>
<tr><th colspan="4">Data</th></tr>
<tr><td>Data</td><td>183</td><td>48 65 6C 6C 6F</td><td>Contents of docs/note.txt, the ASCII text Hello</td></tr>
<tr><th colspan="4">Footer</th></tr>
<tr><td>π</td><td>188</td><td>π</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>200</td><td>8D 37 9C 9C</td><td>Checksum</td></tr>
</tbody>
</table>

### ( v ) Checksum

The checksum is a CRC-32 with the reflected polynomial EDB88320 (hexadecimal), an initial value of FFFFFFFF and a final exclusive-or with FFFFFFFF. It is computed over every byte from the first byte of the file through the closing π, and is stored in little-endian byte order.

The checksum covers the whole file and not single files. A reader shall not refuse a file because its checksum does not match; a mismatch is reported only when the file is tested. Since no checksum of a single file exists, a mismatch cannot be attributed to one file, and the 7-Zip plugin therefore reports it for every file of the archive (see ( Ú/( b.ii. ) )).

### ( vi ) Header Length

The addresses in T1 depend on the length of the header, and the length of the header in turn depends on the addresses, since numbers have no fixed width. A writer therefore builds the header with a start address of 0, measures its length, and builds it again with that length as the start address. The process is repeated until the length no longer changes. Figure B-3 shows the process for the file of Table B-6 and for a file whose start address grows from one byte to two.

<p id="figure-b-3" align="center"><img src="nżúù/b-3.svg" alt="Figure B-3 Header Length"></p>

Figure B-3 Header Length

## ( b ) Properties

### ( i ) Supported

Neither π nor φ can occur inside valid UTF-8, since each contains an ASCII byte followed by a continuation byte. Neither can occur inside a number of at most 7 bytes, and neither contains the byte 00 or 01, so that no mark can be formed across the boundary of a field. Keys therefore require no escaping and no length prefix.

Every valid UTF-8 string is a legal key, including .., C:, \, the characters * and ?, control characters and the empty string. Writers do not alter keys, and readers derive names that are safe for their system on output, as described for Windows in ( Ú/( b.i.4. ) ).

The data is contiguous, and every address and size is given in the header, so that whether the layout of a file is complete can be determined from the header alone. Since the data is stored raw and every address is absolute, a file is read at its address for its size, without decoding. A single checksum covers the header, the data and the closing mark.

### ( ii ) Unsupported

Developers familiar with tar, zip and rar should note that several features of those formats have no equivalent in .v31np. Table B-7 lists these features together with the treatment each receives in .v31np.

<table id="table-b-7">
<caption>Table B-7 Features of tar, zip and rar without Equivalent in .v31np</caption>
<thead>
<tr><th>Feature</th><th>Treatment in .v31np</th></tr>
</thead>
<tbody>
<tr><td>Folder entries</td><td>Folders exist only as prefixes of keys; empty folders cannot be stored, and writers shall refuse them</td></tr>
<tr><td>Dates, permissions, owners, attributes</td><td>Not stored; extracted files carry the date of extraction</td></tr>
<tr><td>Compression, encryption, method fields</td><td>Not provided; data is stored raw</td></tr>
<tr><td>Deduplication, solid blocks, run-length encoding</td><td>Not provided</td></tr>
<tr><td>Symbolic links, hard links</td><td>Not provided; every entry is a file</td></tr>
<tr><td>Duplicate names</td><td>A later entry does not override an earlier one; a duplicate key makes the file invalid</td></tr>
<tr><td>Padding, block alignment</td><td>Not permitted; every byte between header and footer belongs to a file</td></tr>
<tr><td>Appending, in-place update</td><td>Not possible; every address depends on the length of the header and the checksum covers the whole file, so any change rewrites the whole file</td></tr>
<tr><td>Directory at the end of the file</td><td>Not used; the tables are at the start</td></tr>
<tr><td>Checksums of single files</td><td>Not provided; one checksum covers the whole file</td></tr>
<tr><td>Code pages other than UTF-8</td><td>Not permitted</td></tr>
<tr><td>Cleaning of names on write (\ to /, removal of ../, removal of drive letters)</td><td>Not permitted; the key is stored as given</td></tr>
<tr><td>Volumes, comments, extra fields, 64-bit extensions</td><td>Not provided; numbers are of variable width up to 2⁵²</td></tr>
</tbody>
</table>

# ( Ú ) SUPPORTING PROGRAMMES

## ( a ) Native Systems

The native systems are the services of v31n that read or write .v31np files as part of their own operation.

### ( i ) Services for þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio

Secura reads and writes .v31np files in the browser, in assets/js/app.js, and writes them on the server, in api/pack.php. It carries the medallion PASSING (see ( U )).

A downloaded file whose name ends in .v31np is unpacked after decryption and shown as a folder, and .v31np files nested inside it are expanded in turn. A file that does not parse as .v31np is saved unchanged. The command Down‑load all packs the current listing into a new file named after the identifier of the file, with the extension .v31np; duplicate keys receive a letter suffix. The folder view sorts folders and file names in V31N order.

Secura also uses .v31np to transport stored chunks from server to client. The server streams the requested chunks as a .v31np file, in which each key is the SHA-256 of a chunk written as 64 hexadecimal digits, and each entry consists of an initialisation vector of 12 bytes followed by the ciphertext. The reader checks each key against its entry. An interrupted download resumes by HTTP byte range, and a checksum mismatch causes one retry and then failure. A single request holds 1 to 17000 distinct keys.

### ( ii ) Services for þe populace General et Private Musik applikation of Per.‑Gen.‑portal nullpunkts as ver. 2 for General use

M2 writes m/m2.v31np through m/m2-refresh.php, which runs from m/m2.php. It carries the medallion PASSING (see ( U )).

The file bundles the static assets of the application, namely audio, video, images, fonts, style sheets, scripts and STL models from the folders /m/m, /m/img, /m/js/three, /css/fonts and /m/css/cursors, together with the files /img/logomonochrome.ico and /m/serv/npfp.png. Keys are URL paths with a leading /, in which each part is percent-encoded, and are sorted by byte order.

The file is rebuilt when m2list.json is out of date. It is written to m2.v31np.tmp and then renamed, so that a reader never sees a partly written file. It is served only to requests that carry the header X-V31null-Hub with the value 1, and is read by the V31null Hub (see ( Ú/( a.iii. ) )).

### ( iii ) V31null Hub

The V31null Hub reads /m/m2.v31np from its mirrors in Arc.cs. It fetches the footer and the header first and then each file by HTTP byte range. Every key shall be listed in m2list.json and shall map to a safe local path, and keys with empty parts or parts consisting of . or .. are refused. Each file is checked against its fingerprint in m2list.json. The checksum is verified after the whole file has been read, and on a mismatch the download is retried, up to five attempts. Demo.cs writes m2.v31np for the demonstration server.

The hub writes two files that do not conform to ( B/( a ) ), and therefore carries the medallion FAILING (see ( U )). Arc.cs keeps %AppData%\M2\Archive\m2.v31np, which holds the header and the footer without the data. shared/Vlog.cs writes vp.v31np, which consists of the magic, π, a single table of rows of six fields (Id, Kind, Path, Dur, Bytes and Date), π, π and the checksum, and has neither T1 nor T2.

## ( b ) External Third Parties

### ( i ) Windows Explorer

The Windows Explorer extension, v31npshell.dll, is a shell namespace extension that allows Explorer to open, edit, create and extract .v31np files as folders. It is built for x86, x64 and ARM64 and is installed by baíbúočman.exe. The extension carries the medallion PASSING (see ( U )).

#### 1. Browsing

A .v31np file opens in Explorer as a folder, and a .v31np file stored inside another opens as a folder in turn. The view shows the columns name, type and size. Names are shown as they are stored in the key, since the rules of ( Ú/( b.i.4. ) ) apply only when files leave the archive. Folders are sorted before files, and names are sorted in V31N order, first by the name without its extension and then by the whole name. The file type is named Baíbízù format ekta kokain and carries the icon nzuu.ico.

Thumbnails are taken from the thumbnail handler registered for the type of the file. Where none exists, the image is decoded with the Windows Imaging Component, and where that fails, embedded cover art is used. Decoding runs outside Explorer, in dllhost, so that a faulty file cannot affect Explorer itself. Video frames are not shown.

A file opened from inside an archive is first copied to %TEMP%\v31np\{GUID}\ and opened from there. Copies older than one month are removed.

#### 2. Commands

The extension adds commands to the context menu of .v31np files, to the context menu and the Send to menu of other files and folders, and to the New menu. Table Ú-1 presents these commands, the selection to which each applies, and the result. In the table, the name of an archive means its file name without .v31np.

<table id="table-ú-1">
<caption>Table Ú-1 Commands</caption>
<thead>
<tr><th>Command</th><th>Selection</th><th>Result</th></tr>
</thead>
<tbody>
<tr><td>Anfas alùz e íülíntù, followed by the name of the archive</td><td rowspan="4">One .v31np file</td><td>Extracts into a folder of that name beside the archive</td></tr>
<tr><td>Anfas alùz híre</td><td>Extracts into the folder of the archive</td></tr>
<tr><td>Anfas alùz țùre</td><td>Extracts into a picked folder</td></tr>
<tr><td>Anfas alùz țùre e íülíntù, followed by the name of the archive</td><td>Extracts into a folder of that name inside a picked folder</td></tr>
<tr><td>Anfas teí̈nt híre</td><td rowspan="2">Several .v31np files</td><td>Extracts each into a folder of its name inside a new folder baíbíz e alùz in the folder of the archives</td></tr>
<tr><td>Anfas teí̈nt țùre</td><td>Extracts each into a folder of its name inside a new folder baíbíz e alùz in a picked folder</td></tr>
<tr><td>Baíbíz</td><td>Files and folders</td><td>Packs the selection into a new .v31np file; also offered in the Send to menu</td></tr>
<tr><td>New</td><td>None</td><td>Creates an empty .v31np file</td></tr>
</tbody>
</table>

Where a folder baíbíz e alùz, or an archive of the name chosen by Baíbíz, already exists, the new one receives a letter suffix in the manner described in ( Ú/( b.i.4. ) ).

#### 3. Editing

Files are added to an archive by dropping or pasting them into its folder. A file whose key already exists replaces the stored file without prompt. Deletion removes files without confirmation. A rename is refused if the new name is empty or contains /.

Since a .v31np file cannot be updated in place (see ( B/( b.ii. ) )), each change writes a new temporary file, which then replaces the archive. A change inside a nested archive rewrites the inner archive first and then each archive that contains it.

#### 4. Extraction

Keys may hold characters and names that Windows does not permit in file names. On extraction, each part of a key is therefore naturalised according to Table Ú-2, and names that then collide are made unique as described below. The key in the archive is not changed. Extracted files carry the date of extraction and normal attributes, and the checksum is not verified on extraction.

<table id="table-ú-2">
<caption>Table Ú-2 Naturalisation of Names on Windows</caption>
<thead>
<tr><th>Stored</th><th>Written</th></tr>
</thead>
<tbody>
<tr><th colspan="2">Path</th></tr>
<tr><td>Leading /</td><td>Removed</td></tr>
<tr><td>Drive letter and colon at the start of the key, e.g. C:</td><td>Folder of the letter, e.g. C</td></tr>
<tr><td>Empty part, e.g. a//b</td><td>Removed</td></tr>
<tr><td>Part consisting of . or ..</td><td>AN</td></tr>
<tr><th colspan="2">Characters</th></tr>
<tr><td>Colon and backslash</td><td>Removed</td></tr>
<tr><td>* ? &lt; &gt; | "</td><td>!</td></tr>
<tr><td>Control characters U+0000 to U+001F</td><td>0x followed by two hexadecimal digits, e.g. a line feed becomes 0x0A</td></tr>
<tr><th colspan="2">Names</th></tr>
<tr><td>Trailing space or full stop</td><td>Removed</td></tr>
<tr><td>Empty name</td><td>NÍ</td></tr>
<tr><td>Reserved names CON, PRN, AUX, NUL, CONIN$, CONOUT$, COM0 to COM9, LPT0 to LPT9</td><td>Prefixed with !</td></tr>
<tr><td>Name of more than 255 characters</td><td>Shortened, extension kept</td></tr>
</tbody>
</table>

Windows compares file names without regard to case. Where two names in one folder are equal in that sense, the first keeps its name, and each later one receives a suffix before its extension. The suffixes are taken in the order a, b, ú, u, ù, g, d, e, ż, z, i, í, k, l, m, n, o, p, r, s, t, f, ț, č, ș, c, q, x and h, and continue with aa, ab and onward.

Relative paths are limited to 259 characters. Where long paths are disabled in Windows, the limit includes the destination path. A path that exceeds the limit is shortened by cutting the longest folder name first and then the file name; the extension is always kept.

### ( ii ) 7-Zip

The 7-Zip plugin, v31np.dll, adds .v31np as a format to 7-Zip 26.05. It is built for x86, x64 and ARM64 and is placed in the Formats folder of 7-Zip by baíbúočman.exe, a 32-bit 7-Zip on 64-bit Windows receiving the x86 build. The plugin carries the medallion PASSING (see ( U )).

The plugin registers the format name v31np and the extension v31np, and recognises a file by its magic followed by π. Folders are built from the keys, and each item reports its path, whether it is a folder, and its size. Folders are sorted before files, and names are sorted in V31N order as in ( Ú/( b.i.1. ) ).

7-Zip lists names as they are stored. On extraction, the plugin reports names naturalised and made unique as described in ( Ú/( b.i.4. ) ), while the length of paths is left to 7-Zip. Extraction does not verify the checksum. A test verifies the checksum over the whole archive and, on a mismatch, reports CRC Failed for every file, since the checksum does not cover single files (see ( B/( a.v. ) )).

The plugin supports update, by which files may be added, replaced, renamed and deleted; the archive is rewritten in full. A backslash in a path given by 7-Zip is converted to /. Dates and attributes are ignored, and empty folders, invalid UTF-8 and duplicate keys are refused.

# ( U ) V31NP STANDARD

The V31NP STANDARD is the format as defined in ( B ). Each programme described in ( Ú ) carries one of two medallions, which state whether the programme conforms to the standard.

The medallion PASSING is carried by a programme of which every .v31np file it writes conforms to ( B/( a ) ), and which reads every file that conforms. The medallion FAILING is carried by a programme that writes or keeps at least one .v31np file that does not conform to ( B/( a ) ), or that refuses a file that conforms. Table U-1 presents the medallion carried by each programme described in ( Ú ).

<table id="table-u-1">
<caption>Table U-1 Medallions</caption>
<thead>
<tr><th>Programme</th><th>Section</th><th>Medallion</th></tr>
</thead>
<tbody>
<tr><th colspan="3">Native Systems</th></tr>
<tr><td>Secura</td><td>( Ú/( a.i. ) )</td><td>PASSING</td></tr>
<tr><td>M2</td><td>( Ú/( a.ii. ) )</td><td>PASSING</td></tr>
<tr><td>V31null Hub</td><td>( Ú/( a.iii. ) )</td><td>FAILING</td></tr>
<tr><th colspan="3">External Third Parties</th></tr>
<tr><td>Windows Explorer extension</td><td>( Ú/( b.i. ) )</td><td>PASSING</td></tr>
<tr><td>7-Zip plugin</td><td>( Ú/( b.ii. ) )</td><td>PASSING</td></tr>
</tbody>
</table>

# ( Ù ) CONSIDERATIONS

The properties of the format described in ( B ) carry costs as well as benefits. These costs are deliberate and follow from the purposes set out in ( A/( g ) ). The format leaves the use of these properties to the developer, who is responsible for the design of the files written and should exercise judgment in applying the format to a given purpose. The considerations of this division are provided to that end.

## ( a ) Rewriting and Storage

Since every address depends on the length of the header and the checksum covers the whole file, a .v31np file cannot be updated in place (see ( B/( b.ii. ) )). Any change, whether a file is added, replaced, renamed or deleted, writes the whole file again. A file of 50 GB is therefore written again in full, all 50 GB, even when a single byte or a single key changes. The Windows Explorer extension, 7-Zip and M2 write the new file beside the old one and then replace it, so that free space at least equal to the length of the new file is required for the duration of the change. Figure Ù-1 shows the sequence of a change to a file inside a nested archive.

<p id="figure-ù-1" align="center"><img src="nżúù/ù-1.svg" alt="Figure Ù-1 Rewriting on Change"></p>

Figure Ù-1 Rewriting on Change

Large files that are rewritten repeatedly tend to become fragmented, as the new copy is placed in whatever free space the drive holds at the time. On a hard disk drive, fragmentation slows reading. On a solid-state drive, fragmentation has little effect on reading, but each rewrite writes the full length of the file again.

This cost is offset by the absolute addresses of the format, by which any file is read at its address without decoding, and a file is streamed, resumed and read by byte range. The format is therefore best suited to files that are written once and read many times. Files that change often should be kept apart from files that rarely change, so that archives that are edited remain small.

## ( b ) Compression

The data of each file is stored raw (see ( B/( b.ii. ) )). The extracted size of each file therefore equals its stored size, which blocks zip bombs, and each file can be read at its address without decoding. The absence of compression is offset by this direct read access and by the use of the format for general storage.

Most files shared today are already compressed by their own formats, such as images, audio and video, and compressing them again gains little. The developer should therefore optimise the input before packing it, choosing for each file a format that is already compressed where size matters.

## ( ú ) Nested Archives

Packing files into a zip or another compressed archive and then storing that archive as a single file in .v31np is strongly discouraged. The header then describes only the outer file, so that the addresses and sizes of the files inside it are not known, they cannot be read by byte range, and their extracted size is no longer bounded by the header. The benefits of the format, and its protection against zip bombs, then no longer apply to those files.

A .v31np file may be stored inside another, and keeps these properties, since its data is raw as well. A change to a nested archive, however, rewrites each archive that contains it (see ( Ú/( b.i.3. ) )).

## ( u ) Metadata

Dates, permissions, owners and attributes are not stored (see Table B-7). The omission is intended: .v31np is a format for storing and sharing the contents of files. Where an application requires such information, it should store it as a file inside the archive, in a format of its own choosing.

## ( ù ) The Marks π and φ

It may be supposed that a reader which searches for π and φ could find them in the wrong place, for example inside the data of a file. A reader searches for the marks only within the header. The magic is followed by π, the next π closes T1, and the next closes T2. As shown in ( B/( b.i. ) ), neither mark can occur inside a key or a number, so that the first π found after the rows of a table is always the π that closes it, and the same holds for φ within the rows.

The data is not searched. Once the header has been read, each file is read at its address for its size, and the footer is read at its fixed position in the last 16 bytes of the file. A π or a φ inside the data of a file, as in any .v31np file stored inside another, is therefore harmless, and a reader has no need to search beyond the header. Figure Ù-2 illustrates what a reader searches and what it reads.

<p id="figure-ù-2" align="center"><img src="nżúù/ù-2.svg" alt="Figure Ù-2 Reading a File"></p>

Figure Ù-2 Reading a File

## ( g ) The Checksum

The checksum is a quick check of the whole file. It does not establish the authenticity of a file, nor who wrote it, nor that it has not been altered on purpose. The format was created for exchange between two parties, and the checksum allows them to compare their copies of a file, in the manner of a client asking a server whether its file matches the server's.

The checksum comes last, so that a client downloading a file can check the last bytes it holds. Where the closing π and the checksum are missing, the download is incomplete, and the client resumes it by byte range from the length it already holds. Figure Ù-3 shows the exchange between a client and a server.

<p id="figure-ù-3" align="center"><img src="nżúù/ù-3.svg" alt="Figure Ù-3 Download and Comparison between Client and Server"></p>

Figure Ù-3 Download and Comparison between Client and Server

A mismatch indicates that the file differs from the file that was written, without showing which file inside it differs (see ( B/( a.v. ) )). The file should then be requested again from its original owner where that is possible. Where every file inside can be confirmed correct by other means, such as against the copy of the owner or against fingerprints, the archive may be rewritten with a new checksum.

## ( d ) Networking

Since the header precedes the data and gives the address and size of every file, a client need not fetch a whole .v31np file. It may fetch the header alone, offer the files it lists, and fetch each chosen file by byte range from its address for its size. Figure Ù-4 shows this exchange.

<p id="figure-ù-4" align="center"><img src="nżúù/ù-4.svg" alt="Figure Ù-4 Selective Download"></p>

Figure Ù-4 Selective Download

A client that already holds a copy may check whether it is current without fetching the file again. A local copy may carry a checksum that matches its own bytes while its data is stale, so that the copy is compared with that of the server rather than with itself. The server sends its header and its checksum, and the client compares them with those of its copy.

Where they differ, the format allows two courses, and the developer chooses between them. The client may be given agency: it compares the two headers and requests by byte range the files that are missing or of another size, and the whole file where the headers match but the checksums differ. Alternatively, the server may decide, answering that the whole file will be sent since it is small, or that the client shall name the ranges it requires. Figure Ù-5 shows both courses.

<p id="figure-ù-5" align="center"><img src="nżúù/ù-5.svg" alt="Figure Ù-5 Checking a Copy"></p>

Figure Ù-5 Checking a Copy

# ( G ) GLOSSARY

<table>
<tbody>
<tr><td>7-Zip plugin</td><td>The programme v31np.dll, which adds .v31np as a format to 7-Zip; see ( Ú/( b.ii. ) ).</td></tr>
<tr><td>a, b, c</td><td>The fields of T2 that divide a key into its folder, its name and its extension.</td></tr>
<tr><td>Address</td><td>The absolute offset of the first data byte of a file, given in T1.</td></tr>
<tr><td>Archive</td><td>A .v31np file.</td></tr>
<tr><td>ASCII</td><td>American Standard Code for Information Interchange.</td></tr>
<tr><td>Checksum</td><td>The CRC-32 at the end of a file, computed over every byte before it.</td></tr>
<tr><td>CRC-32</td><td>Cyclic redundancy check of 32 bits, with the reflected polynomial EDB88320, an initial value of FFFFFFFF and a final exclusive-or with FFFFFFFF.</td></tr>
<tr><td>Data</td><td>The contents of the files of an archive, stored raw and contiguous after the header.</td></tr>
<tr><td>FAILING</td><td>The medallion of a programme that writes or keeps a file that does not conform to ( B/( a ) ), or that refuses a file that conforms.</td></tr>
<tr><td>Footer</td><td>The closing π and the checksum; the last 16 bytes of a file.</td></tr>
<tr><td>Header</td><td>The magic, π, T1, π, T2 and π, in that order.</td></tr>
<tr><td>HTTP</td><td>Hypertext Transfer Protocol.</td></tr>
<tr><td>Key</td><td>The path of a file as UTF-8 text, in which / separates folders.</td></tr>
<tr><td>Little-endian</td><td>The byte order in which the least significant byte of a number is stored first.</td></tr>
<tr><td>Magic</td><td>The 15 bytes at the start of a file, the ASCII text V31NSPACKFORMAT.</td></tr>
<tr><td>Mark</td><td>π or φ.</td></tr>
<tr><td>Medallion</td><td>The rating, PASSING or FAILING, by which a programme is rated against the V31NP STANDARD.</td></tr>
<tr><td>Native system</td><td>A service of v31n that reads or writes .v31np files as part of its own operation; see ( Ú/( a ) ).</td></tr>
<tr><td>Naturalisation</td><td>The derivation, on extraction, of names that are safe for the system written to; see Table Ú-2.</td></tr>
<tr><td>PASSING</td><td>The medallion of a programme of which every .v31np file it writes conforms to ( B/( a ) ), and which reads every file that conforms.</td></tr>
<tr><td>Row</td><td>The description of one file in a table, beginning with 00 φ.</td></tr>
<tr><td>Size</td><td>The length of the data of a file in bytes, given in T2.</td></tr>
<tr><td>T1</td><td>The first table of the header, giving the address of each file in data order.</td></tr>
<tr><td>T2</td><td>The second table of the header, giving the size of each file and the division of its key.</td></tr>
<tr><td>UTF-8</td><td>Unicode Transformation Format, 8-bit; the encoding of every key.</td></tr>
<tr><td>V31N order</td><td>The order in which names are sorted, first by the name without its extension and then by the whole name; see ( Ú/( b.i.1. ) ). Syn.: Alphabetical order.</td></tr>
<tr><td>V31NP STANDARD</td><td>The format as defined in ( B ).</td></tr>
<tr><td>Zip bomb</td><td>An archive of which the extracted size greatly exceeds the stored size.</td></tr>
<tr><td>π</td><td>The mark of 12 bytes taken from the hexadecimal fraction of π, which divides the parts of a file; see Table B-2.</td></tr>
<tr><td>φ</td><td>The mark of 12 bytes taken from the hexadecimal fraction of φ, which divides the rows and fields of a table; see Table B-2.</td></tr>
</tbody>
</table>

# ( D ) REFERENCES

International Organization for Standardization (ISO) (2002) "ISO/IEC 13239: Information technology – Telecommunications and information exchange between systems – High-level data link control (HDLC) procedures", Geneva.

International Telecommunication Union (ITU-T) (2002) "Recommendation V.42: Error-correcting procedures for DCEs using asynchronous-to-synchronous conversion", Geneva.

Internet Engineering Task Force (IETF) (2003) "RFC 3629: UTF-8, a transformation format of ISO 10646".

Internet Engineering Task Force (IETF) (2022) "RFC 9110: HTTP Semantics", section 14, Range Requests.

Microsoft Corporation (n.d.) "Naming Files, Paths, and Namespaces", Win32 documentation.

Pavlov, I. (2026) "7-Zip 26.05".
