Hírù gùkríbngaín uq standartas Íika , i.e./ .v31np & .v31n formatas , úír kípamíț čí Mílksùns‑maíțúčmíțas.

![Íik](nżúù/eayk.png)

Þis File‑format is þͤ Best‑File‑format in þͤ World File‑format‑‘istorie , All‑so ; I am It’s kreator.


***

<table>
<thead>
<tr><th colspan="6">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td colspan="3">1. Report No.<br>PS-N/PF-1031 R.4</td><td colspan="3">2. Report Date<br>2026/11/03</td></tr>
<tr><td colspan="6">3. Title<br>V31NP STANDARD</td></tr>
<tr><td colspan="6">4. Author(s)<br>P.K. — 619851078911877 , kokain@v31null.de</td></tr>
<tr><td colspan="3">a. Performing Organization Name(s) and Address(es)<br>Bureau for Onion & Over of þͤ Services <i>of</i> þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio — fileprtejlf5mpnrgs5isoknpg6zmidwvc2jtpotepkfeocx4zltm2ad.onion , psnpf1031@v31null.de</td><td colspan="3">b. Sponsoring/Monitoring Agency Name(s) and Address(es)<br>Mbrlí úír Entípatùíțù fùțbíz — v31null.de , mef@v31null.de</td></tr>
<tr><td colspan="6">6. Abstract<br>This document defines the .v31np file format and the V31NP STANDARD. A .v31np file holds any number of files together with their paths, stored raw behind a header of two tables and closed by a single CRC-32 over the whole file. The document sets out the layout, fields, constraints and properties of the format, describes the programmes that support it, namely the native systems of v31n, the Windows Explorer extension and the 7-Zip plugin, rates each against the standard, sets out considerations for developers concerning storage, compression, nested archives, metadata, the marks, the checksum and networking, and sets out the behaviour required of programmes that present, extract and edit .v31np files on any system, including editing through a shadow and its journal.</td></tr>
<tr><td colspan="3" rowspan="2">7. Key Words<br>.v31np, V31NP STANDARD, file format, archive, header, π, φ, CRC-32, UTF-8, byte range, naturalisation, V31N order, shadow, journal, repack, Windows Explorer, 7-Zip</td><td colspan="3">8. Distribution Statement</td></tr>
<tr><td>a. Report<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>b. Abstract<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>ú. This page<br>A; Approved for public release: distribution is unlimited, no restrictions.</td></tr>
<tr><td colspan="3">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td><td colspan="3">10. №ⁱ OF PP.<br>76</td></tr>
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
- [( Ú ) SUPPORT](#úsupport)
  - [( a ) Windows](#awindows)
    - [( i ) Native Systems](#inative-systems)
      - [1. Services for þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio](#1services-for-þe-general-populace-ministeria-custodiae-filaciorum-secura--aßervatio)
      - [2. Services for þe populace General et Private Musik applikation of Per.‑Gen.‑portal nullpunkts as ver. 2 for General use](#2services-for-þe-populace-general-et-private-musik-applikation-of-pergenportal-nullpunkts-as-ver-2for-general-use)
      - [3. V31null Hub](#3v31null-hub)
    - [( ii ) External Third Parties](#iiexternal-third-parties)
      - [1. Windows Explorer](#1windows-explorer)
        - [a. Browsing](#abrowsing)
        - [b. Commands](#bcommands)
        - [ú. Editing](#úediting)
        - [u. Extraction](#uextraction)
        - [ù. Interfaces of the Shell](#ùinterfaces-of-the-shell)
      - [2. 7-Zip](#27-zip)
  - [( b ) GNU/Linux](#bgnulinux)
    - [( i ) External Third Parties](#iexternal-third-parties)
      - [1. Files](#1files)
        - [a. Browsing](#abrowsing-1)
        - [b. Commands](#bcommands-1)
        - [ú. Editing](#úediting-1)
        - [u. Extraction](#uextraction-1)
        - [ù. Interfaces of the System](#ùinterfaces-of-the-system)
      - [2. The Command v31np](#2the-command-v31np)
    - [( ii ) Packages](#iipackages)
  - [( ú ) MacOS](#úmacos)
- [( U ) V31NP STANDARD](#uv31np-standard)
- [( Ù ) CONSIDERATIONS](#ùconsiderations)
  - [( a ) Introduction](#aintroduction-1)
  - [( b ) Rewriting and Storage](#brewriting-and-storage)
  - [( ú ) Compression](#úcompression)
  - [( u ) Nested Archives](#unested-archives)
  - [( ù ) Metadata](#ùmetadata)
  - [( g ) The Marks π and φ](#gthe-marks-π-and-φ)
  - [( d ) The Checksum](#dthe-checksum)
  - [( e ) Networking](#enetworking)
- [( G ) CONFORMING BEHAVIOUR](#gconforming-behaviour)
  - [( a ) Introduction](#aintroduction-2)
  - [( b ) Inside One System](#binside-one-system)
    - [( i ) Browsing](#ibrowsing)
    - [( ii ) Extraction](#iiextraction)
    - [( iii ) Commands](#iiicommands)
    - [( iv ) Editing](#ivediting)
    - [( v ) The Shadow](#vthe-shadow)
    - [( vi ) The Journal](#vithe-journal)
    - [( vii ) Changes](#viichanges)
    - [( viii ) The Repack](#viiithe-repack)
    - [( ix ) Stale Journals and Concurrency](#ixstale-journals-and-concurrency)
    - [( x ) Readers](#xreaders)
  - [( ú ) Across Systems](#úacross-systems)
    - [( i ) Plates](#iplates)
    - [( ii ) Mounted Devices](#iimounted-devices)
    - [( iii ) Networks](#iiinetworks)
      - [1. Shared Roads](#1shared-roads)
      - [2. The Post and the Signal](#2the-post-and-the-signal)
      - [3. The Officer](#3the-officer)
      - [4. The Clock of the Officer](#4the-clock-of-the-officer)
      - [5. Tickets, the Queue and the Lane](#5tickets-the-queue-and-the-lane)
      - [6. One Change, One Car](#6one-change-one-car)
      - [7. Leased Folders](#7leased-folders)
      - [8. Breakdowns](#8breakdowns)
      - [9. Custody](#9custody)
      - [10. Readers on a Shared Road](#10readers-on-a-shared-road)
  - [( u ) Edge Cases and Limitations](#uedge-cases-and-limitations)
- [( D ) GLOSSARY](#dglossary)
- [( E ) REFERENCES](#ereferences)

## ( b ) List of Figures

- [Figure B-1 File Layout](#figure-b-1)
- [Figure B-2 Rows of T1 and T2](#figure-b-2)
- [Figure B-3 Header Length](#figure-b-3)
- [Figure Ú-1 Paths by which Explorer Reaches the Extension](#figure-ú-1)
- [Figure Ú-2 Report of a Move to the Source](#figure-ú-2)
- [Figure Ú-3 Removal Requested by the Copy Engine](#figure-ú-3)
- [Figure Ú-4 Writing a File through IStorage](#figure-ú-4)
- [Figure Ú-5 Creating an Item with Ípmùk](#figure-ú-5)
- [Figure Ú-6 Opening an Archive in 7-Zip](#figure-ú-6)
- [Figure Ù-1 Rewriting on Change](#figure-ù-1)
- [Figure Ù-2 Reading a File](#figure-ù-2)
- [Figure Ù-3 Download and Comparison between Client and Server](#figure-ù-3)
- [Figure Ù-4 Selective Download](#figure-ù-4)
- [Figure Ù-5 Checking a Copy](#figure-ù-5)
- [Figure G-1 Folding of a Name](#figure-g-1)
- [Figure G-2 Comparison of Two Names](#figure-g-2)
- [Figure G-3 Naming of Extracted Files](#figure-g-3)
- [Figure G-4 Life of a Change](#figure-g-4)
- [Figure G-5 Layout of a Journal](#figure-g-5)
- [Figure G-6 Rows and Their Content](#figure-g-6)
- [Figure G-7 Deletion of the Last File of a Folder](#figure-g-7)
- [Figure G-8 Renaming of a Folder Holding a Nested Archive](#figure-g-8)
- [Figure G-9 Views, Wait and Repack](#figure-g-9)
- [Figure G-10 The Repack](#figure-g-10)
- [Figure G-11 Order of Writing Nested Archives](#figure-g-11)
- [Figure G-12 Check of a Journal](#figure-g-12)
- [Figure G-13 Two Programmes Changing One Archive](#figure-g-13)
- [Figure G-14 Listing a Level through the Journal](#figure-g-14)
- [Figure G-15 A Device Carried between Machines](#figure-g-15)
- [Figure G-16 The Post, the Signal and the Officer](#figure-g-16)
- [Figure G-17 The Queue and the Lane over Time](#figure-g-17)
- [Figure G-18 The Drive of a Car](#figure-g-18)
- [Figure G-19 A Breakdown and an Override](#figure-g-19)

## ( ú ) List of Tables

- [Table B-1 File Layout](#table-b-1)
- [Table B-2 Marks](#table-b-2)
- [Table B-3 Bytes of a File Holding One File](#table-b-3)
- [Table B-4 Fields](#table-b-4)
- [Table B-5 Examples of the Division of a Key](#table-b-5)
- [Table B-6 Bytes of a File Holding No Files](#table-b-6)
- [Table B-7 Features of tar, zip and rar without Equivalent in .v31np](#table-b-7)
- [Table Ú-1 Commands](#table-ú-1)
- [Table Ú-2 Naturalisation of Names on Windows](#table-ú-2)
- [Table Ú-3 Attributes of Items](#table-ú-3)
- [Table Ú-4 Data Received by the Drop Target](#table-ú-4)
- [Table Ú-5 Methods of ITransferSource](#table-ú-5)
- [Table Ú-6 Naturalisation of Names on GNU/Linux](#table-ú-6)
- [Table U-1 Medallions](#table-u-1)
- [Table G-1 Ranks of V31N Order](#table-g-1)
- [Table G-2 Letter Suffixes](#table-g-2)
- [Table G-3 Commands](#table-g-3)
- [Table G-4 Escapes in the Text of a Journal](#table-g-4)
- [Table G-5 Lines of a Journal](#table-g-5)
- [Table G-6 Kinds of Rows](#table-g-6)
- [Table G-7 Changes and the Rows They Write](#table-g-7)
- [Table G-8 Edge Cases](#table-g-8)
- [Table G-9 Limitations](#table-g-9)
- [Table G-10 Lines of the Officer](#table-g-10)
- [Table G-11 Events of the Officer](#table-g-11)
- [Table G-12 Lines of a Record of Custody](#table-g-12)

## ( u ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the V31NP STANDARD. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document. v31n is not related to non official platforms, services, people, & ‘oc genus omne in any way or form. Textual, programmatic, and media content published across these services is made available under the terms of the v31n license. No statutory copyright, proprietary monopoly, or reservation of rights subsists in these works. Any person is granted full, unconditional, and irrevocable permission to access, copy, modify, adapt, distribute, or incorporate all materials for any purpose, without formality, fee, or prior authorization. The constitutive Latin charter and legal terms of this declaration are recorded in the system registry and resp. places.

## ( ù ) Foreword

This document has been published to define the .v31np file format and the V31NP STANDARD, and to describe the programmes that read and write .v31np files. It is intended for developers who implement readers or writers of the format, and for those who maintain the services of v31n that exchange .v31np files with one another.

The format is defined in § ( B ), which sets out the layout of a file, the rows and fields of its tables, the constraints a file shall satisfy, the checksum, and the properties the format supports and does not support. § ( Ú ) describes the programmes that support the format, namely the native systems of v31n, the Windows Explorer extension, the 7-Zip plugin and the programmes for GNU/Linux. § ( U ) defines the two medallions, PASSING and FAILING, by which each programme in § ( Ú ) is rated against the standard. § ( Ù ) sets out the considerations that follow from the design of the format, concerning rewriting and storage, compression, nested archives, metadata, the marks, the checksum and networking. § ( G ) sets out the behaviour required of programmes that present, extract and edit .v31np files, on any operating system, including the editing of an archive through a shadow and its journal, and the rules of the road by which several machines share one archive on a mounted device or a network. § ( D ) gives the terms used in this document, and § ( E ) the documents to which it refers.

P.K.

## ( g ) Scope and Purpose

A .v31np file, as defined by v31n, is a single file that holds any number of files together with their paths. The data of each file is stored raw, without compression or encryption. A header at the start of the file holds two tables, the first giving the address of each file in data order and the second giving the size of each file and the parts of its path. A checksum at the end of the file covers the whole file. This definition applies to every .v31np file regardless of the programme that writes it or the system that reads it.

The format was created 1) to share a large number of small files faster, 2) to be used in server ⇆ client relationships, 3) to store data safely, 4) to block zip bomb attacks and similar, 5) to secure the interests of v31n, 6) related to 5), to reduce supply-chain risks, 7) to ease the use of file storage, and 8) to create a standard that all services conforming to the V31NP STANDARD can use interchangeably.

Several of these purposes follow directly from the structure of the format. A .v31np file carries no per-file headers and no compression, and the addresses of all files are known from the header before any data is read, which allows many small files to be shared as one. Since the header precedes the data and the data is contiguous, a file can be streamed from a server, resumed by byte range and read at any address; the native systems in § ( Ú/( a.i. ) ) rely on this. Names are stored exactly as given and are never altered by a writer, and a single checksum covers header, data and closing mark. Because data is not compressed, the extracted size of each file equals its stored size; sizes and addresses shall chain exactly to the length of the file, and empty folders cannot be stored, so that an archive cannot expand beyond what its header states. The format and its readers and writers are defined and maintained by v31n, which limits the dependence of its services on outside software.

The naming of extracted files on a given operating system is not part of the format. Readers derive such names on output as set out in § ( G/( b.ii. ) ), and the rules applied on Windows are given in § ( Ú/( a.ii.1.u. ) ).

## ( d ) Notation

Bytes are written in hexadecimal, two digits to a byte, separated by spaces, as in 24 3F. Offsets, lengths, addresses and sizes are written in decimal; where a stored number is shown, its bytes are followed by its decimal value, as in B7 (183). Numbers are stored in little-endian byte order, the least significant byte first. Powers of two are written as 2⁵². In figures, ␣ denotes a space.

π and φ denote the marks of Table B-2 and not the constants from which they are taken. 00 φ and 01 φ denote the byte 00 or 01 followed by φ. Keys and names of files and programmes are written as they are stored or given, as in docs/note.txt or v31np.dll.

A reference to a part of this document gives the sign § followed by the division and the part, as in § ( B/( a.iii. ) ), which denotes ( iii ) of ( a ) of ( B ). Tables and figures are numbered by division, as in Table B-3 and Figure Ù-4.

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

A row of T1 shall have exactly two fields, and a row of T2 exactly five. Since each file is described once in each table, T1 and T2 shall hold the same number of rows. Figure B-2 illustrates a row of each table for the file of Table B-3.

<table id="table-b-3">
<caption>Table B-3 Bytes of a File Holding One File</caption>
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

<p id="figure-b-2" align="center"><img src="nżúù/b-2.svg" alt="Figure B-2 Rows of T1 and T2"></p>

Figure B-2 Rows of T1 and T2

### ( iii ) Fields

Table B-4 presents the fields of both tables. The key is the path of the file as UTF-8 text, in which the character / separates folders. The address is the offset at which the data of the file begins, and the size is the length of that data. The fields a, b and c divide the key into its folder, its name and its extension.

<table id="table-b-4">
<caption>Table B-4 Fields</caption>
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

The division of a key into a, b and c is illustrated in Table B-5. A key without a folder leaves a empty, a name that begins with a full stop leaves b empty, and a name without a full stop leaves c empty. Only the last full stop of the name separates the extension.

<table id="table-b-5">
<caption>Table B-5 Examples of the Division of a Key</caption>
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

A .v31np file shall satisfy the constraints of this section; the checksum is treated separately in § ( B/( a.v. ) ).

Each key shall be valid UTF-8. Each key shall occur in T2 only once, and each key of T1 shall occur in T2 exactly once. The fields a, b and c shall divide the key as defined in § ( B/( a.iii. ) ), so that their concatenation equals the key.

The order of T1 is the order of the data, while the order of T2 is free. The first address shall equal the length of the header, counted from the first byte of the file through the π that closes T2, and each following address shall equal the previous address plus the size of the previous file. The data of the last file shall end at the offset of the closing π, so that no byte between the header and the footer is left unassigned. The length of the whole file shall be less than 2⁵² bytes.

It follows from these constraints that a .v31np file holding no files is 67 bytes long, consisting of the magic, four π and the checksum. Table B-6 presents the bytes of such a file. Table B-3 presents the bytes of a file holding a single file, of which the key is docs/note.txt and the data the five bytes of the ASCII text Hello.

<table id="table-b-6">
<caption>Table B-6 Bytes of a File Holding No Files</caption>
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

### ( v ) Checksum

The checksum is a CRC-32 with the reflected polynomial EDB88320 (hexadecimal), an initial value of FFFFFFFF and a final exclusive-or with FFFFFFFF. It is computed over every byte from the first byte of the file through the closing π, and is stored in little-endian byte order.

The checksum covers the whole file and not single files. A reader shall not refuse a file because its checksum does not match; a mismatch is reported only when the file is tested. Since no checksum of a single file exists, a mismatch cannot be attributed to one file, and the 7-Zip plugin therefore reports it for every file of the archive (see § ( Ú/( a.ii.2. ) )).

### ( vi ) Header Length

The addresses in T1 depend on the length of the header, and the length of the header in turn depends on the addresses, since numbers have no fixed width. A writer therefore builds the header with a start address of 0, measures its length, and builds it again with that length as the start address. The process is repeated until the length no longer changes. Figure B-3 shows the process for the file of Table B-3 and for a file whose start address grows from one byte to two.

<p id="figure-b-3" align="center"><img src="nżúù/b-3.svg" alt="Figure B-3 Header Length"></p>

Figure B-3 Header Length

## ( b ) Properties

### ( i ) Supported

Neither π nor φ can occur inside valid UTF-8, since each contains an ASCII byte followed by a continuation byte. Neither can occur inside a number of at most 7 bytes, and neither contains the byte 00 or 01, so that no mark can be formed across the boundary of a field. Keys therefore require no escaping and no length prefix.

Every valid UTF-8 string is a legal key, including .., C:, \, the characters * and ?, control characters and the empty string. Writers do not alter keys, and readers derive names that are safe for their system on output, as described for Windows in § ( Ú/( a.ii.1.u. ) ).

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

# ( Ú ) SUPPORT

## ( a ) Windows

### ( i ) Native Systems

The native systems are the services of v31n that read or write .v31np files as part of their own operation.

#### 1. Services for þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio

Secura reads and writes .v31np files in the browser, in assets/js/app.js, and writes them on the server, in api/pack.php. It carries the medallion PASSING (see § ( U )).

A downloaded file whose name ends in .v31np is unpacked after decryption and shown as a folder, and .v31np files nested inside it are expanded in turn. A file that does not parse as .v31np is saved unchanged. The command Down‑load all packs the current listing into a new file named after the identifier of the file, with the extension .v31np; duplicate keys receive a letter suffix. The folder view sorts folders and file names in V31N order.

Secura also uses .v31np to transport stored chunks from server to client. The server streams the requested chunks as a .v31np file, in which each key is the SHA-256 of a chunk written as 64 hexadecimal digits, and each entry consists of an initialisation vector of 12 bytes followed by the ciphertext. The reader checks each key against its entry. An interrupted download resumes by HTTP byte range, and a checksum mismatch causes one retry and then failure. A single request holds 1 to 17000 distinct keys.

#### 2. Services for þe populace General et Private Musik applikation of Per.‑Gen.‑portal nullpunkts as ver. 2 for General use

M2 writes m/m2.v31np through m/m2-refresh.php, which runs from m/m2.php. It carries the medallion PASSING (see § ( U )).

The file bundles the static assets of the application, namely audio, video, images, fonts, style sheets, scripts and STL models from the folders /m/m, /m/img, /m/js/three, /css/fonts and /m/css/cursors, together with the files /img/logomonochrome.ico and /m/serv/npfp.png. Keys are URL paths with a leading /, in which each part is percent-encoded, and are sorted by byte order.

The file is rebuilt when m2list.json is out of date. It is written to m2.v31np.tmp and then renamed, so that a reader never sees a partly written file. It is served only to requests that carry the header X-V31null-Hub with the value 1, and is read by the V31null Hub (see § ( Ú/( a.i.3. ) )).

#### 3. V31null Hub

The V31null Hub reads /m/m2.v31np from its mirrors in Arc.cs. It fetches the footer and the header first and then each file by HTTP byte range. Every key shall be listed in m2list.json and shall map to a safe local path, and keys with empty parts or parts consisting of . or .. are refused. Each file is checked against its fingerprint in m2list.json. The checksum is verified after the whole file has been read, and on a mismatch the download is retried, up to five attempts. Demo.cs writes m2.v31np for the demonstration server.

The hub writes two files that do not conform to § ( B/( a ) ), and therefore carries the medallion FAILING (see § ( U )). Arc.cs keeps %AppData%\M2\Archive\m2.v31np, which holds the header and the footer without the data. shared/Vlog.cs writes vp.v31np, which consists of the magic, π, a single table of rows of six fields (Id, Kind, Path, Dur, Bytes and Date), π, π and the checksum, and has neither T1 nor T2.

### ( ii ) External Third Parties

#### 1. Windows Explorer

The Windows Explorer extension, v31npshell.dll, is a shell namespace extension that allows Explorer to open, edit, create and extract .v31np files as folders. It is built for x86, x64 and ARM64 and is installed by baíbúočman.exe. The extension carries the medallion PASSING (see § ( U )).

The extension carries out the behaviour of § ( G ) on Windows. This part sets out the names it gives to that behaviour and the means by which it is achieved, so that a developer may reproduce the extension on Windows without its source.

##### a. Browsing

A .v31np file, and a .v31np file stored inside another, opens in Explorer as a folder, presented and sorted as set out in § ( G/( b.i. ) ). The view shows the columns name, type and size. The file type is named Baíbízù format ekta kokain and carries the icon nzuu.ico. Other programmes may give the type another name or a translation of it, such as Kokain's Storage-format.

Thumbnails are taken from the thumbnail handler registered for the type of the file. Where none exists, the image is decoded with the Windows Imaging Component, and where that fails, embedded cover art is used. Decoding runs outside Explorer, in dllhost, so that a faulty file cannot affect Explorer itself. Video frames are not shown.

A file opened from inside an archive is first copied to %TEMP%\v31np\{GUID}\ and opened from there. Copies older than one month are removed.

##### b. Commands

The extension offers the functions of § ( G/( b.iii. ) ) under the names of Table Ú-1. They appear in the context menu of .v31np files, in the context menu and the Send to menu of other files and folders, in the New menu, and inside archives in the context menus of items and of the empty spot. In the table, the name of an archive means its file name without .v31np. Other programmes may give these commands, and the folder baíbíz e alùz, other names or translations of them.

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
<tr><td rowspan="2">Baíbíz</td><td>Files and folders on disk</td><td>Packs the selection into a new .v31np file beside it; also offered in the Send to menu</td></tr>
<tr><td>Files and folders inside an archive</td><td>Moves the selection into a new .v31np file in the same folder; an entry of the context menu of items</td></tr>
<tr><td>New</td><td>None, on disk</td><td>Creates an empty .v31np file</td></tr>
<tr><td>Ípmùk</td><td>None, inside an archive</td><td>Submenu of the empty spot offering the New list of Windows; the new item is opened for naming</td></tr>
</tbody>
</table>

Where the destination of an extraction already holds a name, the item receives a suffix of Table G-2 and folders are merged, as set out in § ( Ú/( a.ii.1.u. ) ); no dialog is shown.

Inside archives, the entry Share of Windows is hidden, since it fails on items that are not files on disk.

##### ú. Editing

Editing follows § ( G/( b.iv. ) ), and every change passes through the shadow of § ( G/( b.v. ) ). Files and folders are added, copied and moved by drag and drop, by Cut, Copy and Paste, and by dropping them onto the icon of a nested archive, and they leave an archive for the disk in the same manner. A drop within the same archive moves by default and any other drop copies; Ctrl forces a copy and Shift a move. A copy into the folder it comes from, by Copy and Paste or by a drag with Ctrl onto the same folder, receives a suffix, as eayk.png becomes eayka.png, and a folder photos so copied becomes a new folder photosa holding a full copy; an item added under a name already taken receives a suffix in the same manner (see § ( G/( b.iv. ) )). An original cut or moved in from the disk is hidden until the repack. On a drag or a paste out onto the disk, Explorer writes the files itself and asks in its own dialogs where a name is taken; on a move, the items that reached the disk are then removed from the archive, as by Delete, and items skipped in those dialogs stay. The Delete key and Delete on the context menu delete without confirmation, and Rename renames.

The journal carries the hidden attribute, and temporary files are kept in %TEMP%\v31np. The extension counts the open views of each archive within the Explorer process, and repacks the archive 1.5 seconds after the last of them has closed, unless a view has opened again. The interfaces of Windows through which all of this is achieved are described in § ( Ú/( a.ii.1.ù. ) ).

A file opened from inside an archive is saved back as set out in § ( G/( b.iv. ) ). Its copy in %TEMP%\v31np\{GUID}\ is watched for as long as Explorer runs, and each save goes back into the archive and shows there at once with its new size.

An archive on a network share, or inside a folder that the PC itself shares, lies on a shared road (see § ( G/( ú.iii.1. ) )). There, each change is written into the archive at once, and an open view refreshes itself when another machine changes the archive. Where the holder of the lane has missed its renewals and only this PC waits, the extension asks in a message box captioned with the file name of the archive, with the buttons Yes and No, naming the holder by its papers, as in Still waiting for pingu-346656037 (Ubuntu 26.04.1 LTS), which holds this archive. Go ahead anyway?

##### u. Extraction

Keys may hold characters and names that Windows does not permit in file names. On extraction, each part of a key is therefore naturalised according to Table Ú-2, which gives the rules of § ( G/( b.ii. ) ) for Windows, and names that then collide are made unique as described below. The key in the archive is not changed. Extracted files carry the date of extraction and normal attributes, and the checksum is not verified on extraction.

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

Windows compares file names without regard to case. Where two names in one folder are equal in that sense, the first keeps its name, and each later one receives a suffix of Table G-2 before its extension.

The commands of Table Ú-1 never replace what the destination holds. A name already taken there, by the same comparison, receives a suffix in the same manner, as E:\eayk.png becomes E:\eayka.png; a folder that exists at the destination is merged, and the clashing files inside it receive suffixes, as docs\note.txt becomes docs\notea.txt. An existing destination folder is used as it is, and no dialog is shown. The means are described in § ( Ú/( a.ii.1.ù. ) ).

Where long paths are disabled in Windows, the commands of Table Ú-1 shorten an item whose destination path and name together exceed 259 characters, by cutting the longest folder name first and then the file name, the extension always being kept and room being left for a suffix; where long paths are enabled, nothing is shortened. Items dragged or pasted out of an archive are offered with relative paths of at most 259 characters, shortened in the same manner, whether or not long paths are enabled.

##### ù. Interfaces of the Shell

Explorer reaches the extension along three paths, shown in Figure Ú-1. Commands given on an item or on the empty spot, among them the Delete key, Delete, Rename, Baíbíz and Paste, arrive through the context menus. Drag and drop, and Paste onto a folder, arrive through the drop target. Other programmes and scripts that drive IFileOperation arrive through the copy engine, which also serves the drop target for foreign items that offer no contents of their own, and which the extraction commands of Table Ú-1 drive themselves. In the tests made for this document, every action taken in Explorer itself inside an archive took one of the first two paths; the copy engine was met only from other programmes and from the extraction commands.

<p id="figure-ú-1" align="center"><img src="nżúù/ú-1.svg" alt="Figure Ú-1 Paths by which Explorer Reaches the Extension"></p>

Figure Ú-1 Paths by which Explorer Reaches the Extension

The folder of an archive implements IShellFolder2 and IPersistFolder3. Its view is created with SHCreateShellFolderView, and an IShellFolderViewCB is handed to it in SFV_CREATE for one purpose: the release of that callback marks the closing of the view, by which the views of § ( G/( b.viii. ) ) are counted. An enumeration requested with SHCONTF_STORAGE (0x800) shall return folders and files alike. Table Ú-3 presents the attributes reported for items. A drop onto the icon of a nested archive requires SFGAO_DROPTARGET on the item and an IDropTarget for it from GetUIObjectOf, directed at the nested archive.

<table id="table-ú-3">
<caption>Table Ú-3 Attributes of Items</caption>
<thead>
<tr><th>Item</th><th>Attributes (SFGAO_)</th></tr>
</thead>
<tbody>
<tr><td>File</td><td>STREAM, CANCOPY, CANMOVE, CANRENAME, CANDELETE</td></tr>
<tr><td>Nested .v31np</td><td>As a file, with FOLDER and DROPTARGET</td></tr>
<tr><td>Folder</td><td>FOLDER, HASSUBFOLDER, DROPTARGET, CANCOPY, CANMOVE, CANRENAME, CANDELETE</td></tr>
<tr><td>The folder itself, no item selected</td><td>FOLDER, HASSUBFOLDER, DROPTARGET</td></tr>
</tbody>
</table>

The context menu of items is built with SHCreateDefaultContextMenu and a callback. Delete arrives as DFM_CMD_DELETE, with the verb delete, and Paste as DFM_CMD_PASTE; neither passes through the copy engine, and neither asks. Explorer may invoke an entry by its canonical verb, taken from GetCommandString, instead of by its menu identifier, and both shall be handled; a verb that is not recognised falls into the default path of the menu, which opens copies of the files in their applications and reports an error for each. Baíbíz carries the verb v31np.pack. Entries with the verbs Windows.ModernShare and Windows.Share are removed inside archives.

Table Ú-4 presents the data the drop target receives and its treatment, and Figure Ú-2 shows how a move is reported to its source. The last row of the table and the lower part of the figure give the reverse case, a move out of an archive onto the disk, which the drop target of Explorer reports to the extension. Moves from disk and between archives of the extension are optimised moves: the extension does the work itself and reports CFSTR_PERFORMEDDROPEFFECT as DROPEFFECT_NONE, so that the source keeps its files, which are deleted only by the repack. Until then, each original moved in at the top of the move is hidden with FILE_ATTRIBUTE_HIDDEN and recorded as a row H, and each change of the attribute is announced through SHChangeNotify with SHCNE_ATTRIBUTES and SHCNE_UPDATEDIR on the folder that holds the original; where Explorer is set to show hidden files, the original is still shown, faded. CFSTR_PASTESUCCEEDED with DROPEFFECT_MOVE shall never be reported for sources on disk, since the clipboard object of Windows takes it as the order to delete the originals at once, before the repack has read them. Foreign virtual sources, such as zip folders and devices, are unoptimised moves: the extension reports DROPEFFECT_MOVE, on Paste also CFSTR_PASTESUCCEEDED, and the source deletes its items. On Paste, a move is recognised by CFSTR_PREFERREDDROPEFFECT. From every source, an item whose name is already taken receives a suffix as set out in § ( G/( b.iv. ) ).

<table id="table-ú-4">
<caption>Table Ú-4 Data Received by the Drop Target</caption>
<thead>
<tr><th>Source</th><th>Formats</th><th>Treatment</th><th>Reported on a move</th></tr>
</thead>
<tbody>
<tr><td>Disk</td><td>CF_HDROP</td><td>Rows F; on a move rows M, R for each folder, and H for each original hidden</td><td>CFSTR_PERFORMEDDROPEFFECT: DROPEFFECT_NONE</td></tr>
<tr><td>Archives of the extension</td><td>CFSTR_SHELLIDLIST, resolved to archive, level and path</td><td>Rows moved, or copied out as in Table G-7</td><td>CFSTR_PERFORMEDDROPEFFECT: DROPEFFECT_NONE</td></tr>
<tr><td>Zip folders, devices and other virtual sources</td><td>CFSTR_SHELLIDLIST with CFSTR_FILEDESCRIPTORW and CFSTR_FILECONTENTS, the contents as IStream or HGLOBAL</td><td>Contents copied to rows T, folders recorded as rows D</td><td>CFSTR_PERFORMEDDROPEFFECT: DROPEFFECT_MOVE; on Paste also CFSTR_PASTESUCCEEDED: DROPEFFECT_MOVE</td></tr>
<tr><td>Other shell items</td><td>CFSTR_SHELLIDLIST alone</td><td>Handed to IFileOperation</td><td>Left to the copy engine</td></tr>
<tr><td>Out of an archive onto the disk, received by the drop target of Explorer</td><td>CFSTR_FILEDESCRIPTORW and CFSTR_FILECONTENTS, offered by the data object of the extension</td><td>Items read to the end removed through the shadow, as by Delete; the rest kept</td><td>Reported to the extension through SetData: CFSTR_PERFORMEDDROPEFFECT: DROPEFFECT_MOVE; on Paste also CFSTR_PASTESUCCEEDED: DROPEFFECT_MOVE</td></tr>
</tbody>
</table>

<p id="figure-ú-2" align="center"><img src="nżúù/ú-2.svg" alt="Figure Ú-2 Report of a Move to the Source"></p>

Figure Ú-2 Report of a Move to the Source

Items copied or dragged out of an archive are offered as CFSTR_FILEDESCRIPTORW and CFSTR_FILECONTENTS by a data object wrapped with SHCreateDataObject. The drop target of Explorer on the disk takes these formats for copies and for moves alike: it names the files itself from cFileName, shows its own dialogs Copy File and Confirm Folder Replace where a name is taken, and does not use the source interface described below, so that the suffix of § ( G/( b.iv. ) ) does not apply there. The formats are offered nonetheless, since programmes that accept only virtual files, such as web browsers receiving an upload, would otherwise receive nothing. The data object is never told its destination, and FILEDESCRIPTORW has no means to rename an item on a clash. After a move, the drop target reports it to the data object with CFSTR_PERFORMEDDROPEFFECT set to DROPEFFECT_MOVE, and on a paste also with CFSTR_PASTESUCCEEDED; the data object of the extension, behind SHCreateDataObject, receives the report through SetData, and the extension then removes the moved items from the archive through the shadow, by the same path as the Delete key. Only items that reached the disk are removed: a file counts as written once Windows has read its CFSTR_FILECONTENTS stream to the end, and an empty file once its stream has been requested; a folder is removed only when all its files were read, and otherwise only its read files are removed, the rest staying with the folder. Items skipped in the dialogs of Windows, or not written since the operation was cancelled, stay in the archive. A copy reports no move and removes nothing.

The extraction commands of Table Ú-1 hand the items of the archive to the copy engine themselves, through IFileOperation::CopyItems onto the destination folder, with the window of Explorer as owner. The engine asks GetDefaultDestinationName of the source interface for each item, from the top down, with the real destination folder, and receives the names of § ( Ú/( a.ii.1.u. ) ); for a folder that lands on a folder it receives the same name, merges the two, and asks again for each item inside. No dialog is shown.

The copy engine finds its source through IShellFolder::CreateViewObject with ITransferSource, not through BindToObject. Table Ú-5 presents the methods of ITransferSource. OpenItem returns S_FALSE for files and folders, which tells the engine to open them itself: it then enters folders through BindToObject with IStorage and reads files through BindToObject with IStream, as it does for the zip folder of Windows. The resources of IShellItemResources are not used: the resource that holds the content of a file has no documented identifier, the folders of Windows list none, and an engine given resources copies every file with 0 bytes.

<table id="table-ú-5">
<caption>Table Ú-5 Methods of ITransferSource</caption>
<thead>
<tr><th>Method</th><th>Behaviour</th></tr>
</thead>
<tbody>
<tr><td>OpenItem</td><td>Returns S_FALSE for files and folders</td></tr>
<tr><td>RemoveItem</td><td>As shown in Figure Ú-3; an item already gone returns S_OK</td></tr>
<tr><td>RecycleItem</td><td>As RemoveItem, since an archive has no Recycle Bin</td></tr>
<tr><td>MoveItem</td><td>E_NOTIMPL; the engine then copies and calls RemoveItem</td></tr>
<tr><td>RenameItem</td><td>Renames and returns the new IShellItem</td></tr>
<tr><td>GetDefaultDestinationName</td><td>Returns the name naturalised for Windows; where the destination already holds it, the name with a suffix of Table G-2, except for a folder landing on a folder, which keeps its name and is merged; shortened as in § ( Ú/( a.ii.1.u. ) ) where long paths are disabled</td></tr>
<tr><td>LinkItem, ApplyPropertiesToItem</td><td>E_NOTIMPL</td></tr>
<tr><td>Advise, Unadvise, SetProperties, EnterFolder, LeaveFolder</td><td>S_OK</td></tr>
</tbody>
</table>

The engine asks for the removal of a folder before it touches the items inside it, as shown in Figure Ú-3. On a delete, where the flag TSF_MOVE_AS_COPY_DELETE (0x400) is not set, the extension removes the folder with all it holds and returns COPYENGINE_S_DONT_PROCESS_CHILDREN; with S_OK, the engine would look for the items just removed and fail with 0x80070002. On a move, where the flag is set, the items have not yet been copied: the extension returns S_OK, marks the folder and keeps everything, and removes the folder once its last item has moved out. A folder removed at once loses every file of the move, and an answer of ERROR_DIR_NOT_EMPTY makes the engine abort. A folder that is emptied in any other way remains as a row D, since a folder that vanishes with its last item breaks the engine as well.

<p id="figure-ú-3" align="center"><img src="nżúù/ú-3.svg" alt="Figure Ú-3 Removal Requested by the Copy Engine"></p>

Figure Ú-3 Removal Requested by the Copy Engine

As a destination, the extension offers no ITransferDestination, for the reason given above; the engine then writes through IStorage, which the folder of an archive provides through QueryInterface and BindToObject. Figure Ú-4 shows the writing of a file. CreateStream(name) shall list the new item at once, as a row T on an empty temporary file; Commit and Stat follow, and Stat on the stream shall report the name of the element and not the name of the temporary file. The engine releases the stream without writing to it, finds the item with ParseDisplayName, and binds it with IStream three times, with the modes 0, 0x20 and 0x22, the last being STGM_READWRITE with STGM_SHARE_DENY_WRITE. The content arrives through the last bind, so that a bind in a mode that permits writing shall hand out a writable stream on the same temporary file. A folder is created with CreateStorage(name), which records a row D; Commit and Stat follow, ParseDisplayName shall find the new empty folder at once, and BindToObject with IStorage opens it for the files that follow. DestroyElement, RenameElement and MoveElementTo are supported as well. Names already taken in writes through IStorage are left to the clash handling of Windows, as permitted in § ( G/( b.iv. ) ).

<p id="figure-ú-4" align="center"><img src="nżúù/ú-4.svg" alt="Figure Ú-4 Writing a File through IStorage"></p>

Figure Ú-4 Writing a File through IStorage

IFileOperation directed at the root of an archive fails with 0x8007007B before the extension is asked, since to Windows the root is a file; the zip folder of Windows fails in the same way. Drop and Paste into the root pass through the drop target and are not affected. The Delete key of Explorer does not use the copy engine and asks nothing, while programmes that delete through IFileOperation receive the confirmation that Windows shows for items that cannot be recycled, which an extension cannot suppress.

Windows does not show its New menu inside a namespace extension, even where a key Background\shellex\ContextMenuHandlers\New is handed to SHCreateDefaultContextMenu. The extension therefore hosts the New menu of Windows itself, CNewMenu, {D969A300-E7FF-11D0-A93B-00A0C90F2719}, as shown in Figure Ú-5. It mirrors the names of the folder of the archive into a private temporary folder in %TEMP%\v31np as empty placeholders, so that Windows numbers new names correctly, initialises the New menu on that folder, lets it fill the menu, and renames its popup to Ípmùk. Once Windows has created an item there, the extension moves it into the archive and opens its name field through IShellBrowser, QueryActiveShellView and SelectItem with SVSI_EDIT. New Shortcut writes a placeholder .lnk, which is not empty, and starts a wizard that completes it later; a new .lnk is therefore taken only once it has been changed, renamed or removed, and the folder is watched for ten minutes.

<p id="figure-ú-5" align="center"><img src="nżúù/ú-5.svg" alt="Figure Ú-5 Creating an Item with Ípmùk"></p>

Figure Ú-5 Creating an Item with Ípmùk

The lock of § ( G/( b.ix. ) ) is a named mutex, Local\v31np- followed by the 64-bit FNV-1a hash of the path of the archive, in upper case and UTF-16, as 16 hexadecimal digits. The journal is written to its name with .tmp appended, moved over the journal with MoveFileEx, MOVEFILE_REPLACE_EXISTING and MOVEFILE_WRITE_THROUGH, and given FILE_ATTRIBUTE_HIDDEN. The extension writes only the plain name, and looks for the journal under both names of § ( G/( b.vi. ) ): where one exists, it is used; where both hold the same bytes, the plain one is used; where they differ, the one that can be read is used, and where both or neither can be read, the one with the later ftLastWriteTime. The repack and the discard of a stale journal delete only the journal that was used. The time in its header is the ftLastWriteTime of the archive. The repack writes the new archive to a temporary file beside it and replaces the archive through SetFileInformationByHandle with FileRenameInfoEx, FILE_RENAME_FLAG_REPLACE_IF_EXISTS and FILE_RENAME_FLAG_POSIX_SEMANTICS, so that readers holding the old file open are not disturbed; where that fails, MoveFileEx is used. After each change and after the repack, SHChangeNotify with SHCNE_UPDATEITEM refreshes the views.

The plate of the PC is its DNS host name, taken from GetComputerNameExW with ComputerNameDnsHostname, a hyphen, and the number of § ( G/( ú.i. ) ) drawn from the value MachineGuid of HKLM\SOFTWARE\Microsoft\Cryptography, read in the 64-bit view of the registry and written in lower case. The temporary files that the extension knows as its own are %TEMP%\v31np\ followed by a GUID in braces and .tmp; no other path of a row T is ever deleted.

A path lies on a shared road where GetDriveTypeW reports DRIVE_REMOTE for its volume, taken with GetVolumePathNameW, or where it lies inside the folder named by the entry Path= of a share listed under HKLM\SYSTEM\CurrentControlSet\Services\LanmanServer\Shares, whose values are of the type REG_MULTI_SZ. The key can be read without administrative rights, and the administrative shares, such as C$, are not listed there.

The post is opened with CreateFileW, OPEN_ALWAYS, FILE_SHARE_READ, FILE_SHARE_WRITE and FILE_ATTRIBUTE_HIDDEN. The signal is taken with LockFileEx, LOCKFILE_EXCLUSIVE_LOCK and LOCKFILE_FAIL_IMMEDIATELY, on one byte at the offset 2⁴⁰ given in the OVERLAPPED structure, and released with UnlockFileEx; a machine waiting for green tries again every 25 milliseconds. The time of the officer is taken after FlushFileBuffers on the post, as LastWriteTime of FILE_BASIC_INFO through GetFileInformationByHandleEx, so that it is the time the file server has set.

On a shared road, the archive is not kept open. Once its length and time are read, its handle is closed, and every read opens the archive, compares GetFileSizeEx and GetFileTime with those of the listing, reads and closes it. A file server reached through SMB refuses to replace a file while any machine holds a handle to it, even one opened with FILE_SHARE_DELETE. FileRenameInfoEx with FILE_RENAME_FLAG_POSIX_SEMANTICS fails there with ERROR_INVALID_PARAMETER, and the extension then replaces the archive through FileRenameInfo with ReplaceIfExists, trying again up to 40 times at intervals of 250 milliseconds while a reader holds it open; the journal is written with the same patience. An open view of an archive on a shared road watches the folder of the archive with FindFirstChangeNotificationW, checks the length and time of the archive every 2 seconds, and refreshes itself when they change.

A copy opened from inside an archive is watched with FindFirstChangeNotificationW on its folder, for names, sizes and write times. A change is taken one second after it is announced, where the size or the last-write time of the copy differs from that of the last save, and a copy that cannot yet be read is tried again every 2 seconds. Whether the entry was changed elsewhere is decided by the 64-bit FNV-1a hash of its content, compared with the hash of the copy when it was opened and with that of the last save. After a save, SHChangeNotify with SHCNE_UPDATEITEM shows the new size at once.

#### 2. 7-Zip

The 7-Zip plugin, v31np.dll, adds .v31np as a format to 7-Zip 26.05. It is built for x86, x64 and ARM64 and is placed in the Formats folder of 7-Zip by baíbúočman.exe, a 32-bit 7-Zip on 64-bit Windows receiving the x86 build. The plugin carries the medallion PASSING (see § ( U )).

The plugin registers the format name v31np and the extension v31np, and recognises a file by its magic followed by π. Folders are built from the keys, and each item reports its path, whether it is a folder, and its size. Folders are sorted before files, and names are sorted in V31N order as in § ( G/( b.i. ) ).

7-Zip lists names as they are stored. On extraction, the plugin reports names naturalised and made unique as described in § ( Ú/( a.ii.1.u. ) ), while the length of paths is left to 7-Zip. Extraction does not verify the checksum. A test verifies the checksum over the whole archive and, on a mismatch, reports CRC Failed for every file, since the checksum does not cover single files (see § ( B/( a.v. ) )).

The plugin reads the journal of § ( G/( b.vi. ) ) through IArchiveOpenVolumeCallback, as shown in Figure Ú-6. It takes the name and the last-write time of the archive from GetProperty with kpidName and kpidMTime, and opens the journal under both names through GetStream, choosing between them as the extension does (see § ( Ú/( a.ii.1.ù. ) )), the write time of each being taken from IStreamGetProps::GetProps on its stream; where no time is given, the plain name prevails on a tie. Where the journal is readable and matches the archive, 7-Zip lists and extracts the archive with its pending changes, among them files linked on disk, empty folders, and nested archives built from their sections, as set out in § ( G/( b.x. ) ). The plugin does not discard a stale journal; it reads the archive as stored.

<p id="figure-ú-6" align="center"><img src="nżúù/ú-6.svg" alt="Figure Ú-6 Opening an Archive in 7-Zip"></p>

Figure Ú-6 Opening an Archive in 7-Zip

The plugin supports update when no journal exists, by which files may be added, replaced, renamed and deleted; the archive is rewritten in full. While a journal exists, update is refused, since 7-Zip always writes a whole new archive itself and cannot write into the shadow. A backslash in a path given by 7-Zip is converted to /. Dates and attributes are ignored, and empty folders, invalid UTF-8 and duplicate keys are refused.

## ( b ) GNU/Linux

On GNU/Linux, .v31np files are supported by v31np-mount, which presents an archive as a mounted file system, by its integration into Files, the file manager of GNOME, and by the command v31np. The programmes are built for amd64 and are installed by the packages of § ( Ú/( b.ii. ) ). They carry out the behaviour of § ( G ) on GNU/Linux. This part sets out the names they give to that behaviour and the means by which it is achieved.

### ( i ) External Third Parties

#### 1. Files

##### a. Browsing

A .v31np file opens in Files on a double click. The archive is mounted at ~/V31NP/, followed by its file name, as ~/V31NP/photos.v31np, and Files shows it as it shows a USB stick: with an entry in its sidebar, a button to eject it, and a path that begins at the name of the archive. Where that name is taken by another open archive, the folder receives a suffix of Table G-2, and an archive that is already open is shown again from its mount. The mount ends when it is ejected, or by itself when no window and no programme has used it for ten minutes; its folder, and ~/V31NP where it is left empty, are then removed. Archives nested inside it are presented as folders in turn, and names are presented as they are stored, sorted by Files in its own order. Only names that the system cannot show are presented as in Table Ú-6, namely a part consisting of . or .., an empty part and a name of more than 255 bytes; where such a name then meets a stored name, the stored name keeps it.

The file type is application/x-v31np, recognised by the extension .v31np and by the magic followed by $, and named Baíbízù format ekta kokain, or Kokain's Storage-format in English. It carries the icon nzuu.ico.

##### b. Commands

The context menu of Files offers the functions of § ( G/( b.iii. ) ) under the names of Table Ú-1: the four extraction commands on one archive, Anfas teí̈nt híre and Anfas teí̈nt țùre on several, and Baíbíz on any other selection. A folder is picked in the dialog of zenity. Where an extraction fails, the message names the archive, as in photos.v31np could not be extracted. Where packing fails, it names the new archive and the reason, as in photos.v31np could not be made, followed by a colon and the reason; where the selection holds no files, the message is Nothing to pack: the selection holds no files, and the .v31np format stores no empty folders.

##### ú. Editing

A mounted archive is edited in Files, or by any programme, as a folder. On a local disk, changes pass through the shadow of § ( G/( b.v. ) ), and the archive is repacked when it is unmounted, or once it has not been used for 1.5 seconds. On a shared road, every change is one car (see § ( G/( ú.iii.6. ) )); a file that is being written drives when it is closed, so that until then it is visible only on the machine that writes it. The journal and the post are hidden through the file .hidden of their folder, which Files reads, and carry no full stop before their names.

##### u. Extraction

Extraction follows § ( G/( b.ii. ) ). GNU/Linux permits every character in a name except / and the null character, and names are compared with regard to case, so that A.txt and a.txt are two names. On extraction, each part of a key is therefore changed only as presented in Table Ú-6, and names that are then equal receive a suffix of Table G-2. A name already taken at the destination is never replaced: a file receives a suffix, and a folder is merged. No dialog is shown.

<table id="table-ú-6">
<caption>Table Ú-6 Naturalisation of Names on GNU/Linux</caption>
<thead>
<tr><th>Stored</th><th>Written</th></tr>
</thead>
<tbody>
<tr><td>Part consisting of . or ..</td><td>AN</td></tr>
<tr><td>Control characters U+0000 to U+001F, U+007F and U+0080 to U+009F</td><td>0x followed by two hexadecimal digits, e.g. a line feed becomes 0x0A</td></tr>
<tr><td>Empty name</td><td>NÍ</td></tr>
<tr><td>Name of more than 255 bytes</td><td>Shortened, extension kept</td></tr>
</tbody>
</table>

##### ù. Interfaces of the System

v31np-mount is a file system in user space, built on FUSE 3. It is started as v31np-mount, followed by --idle= and a number of seconds after which an unused mount ends, by --tidy where the folder of the mount is to be removed after it, by the archive and by the folder of the mount. A double click runs v31np-open, which chooses the folder under ~/V31NP, mounts the archive and opens it in Files. The context menu is a native extension of Files, built with libnautilus-extension-4, which runs v31np-menu.

The plate of the machine is its host name from gethostname, a hyphen, and the number of § ( G/( ú.i. ) ) drawn from the first line of /etc/machine-id, written in lower case. The temporary files that the programmes know as their own lie in the folder v31np under $XDG_CACHE_HOME, or under ~/.cache where it is not set, and are named with 32 hexadecimal digits and .tmp.

A path lies on a shared road where statfs reports the file system of its folder as CIFS, SMB, SMB2, NFS or Ceph, or where a mount of the type fuse.gvfsd-fuse, fuse.sshfs or fuse.rclone holds it, as /proc/self/mounts shows. A path also lies on a shared road where its folder lies inside a folder that the machine shares: a share of /etc/samba/smb.conf, the home folder where [homes] is shared, a usershare of Samba, or an export of /etc/exports. The post is created with O_CREAT and O_EXCL and then listed in .hidden. The signal is taken with fcntl and F_OFD_SETLK, a lock of the open file description, on one byte at the offset 2⁴⁰, so that two programmes, or two threads of one, never share a signal by mistake. On a shared road, the archive is not kept open between reads, as on Windows.

#### 2. The Command v31np

The command v31np serves scripts and the context menu. It is run as follows:

1. v31np pack, the archive and a folder: packs the folder into a new archive.
2. v31np make, a new archive and one or more paths: packs the paths, and never replaces an existing file.
3. v31np name, a folder and a name: gives the first name not taken in the folder, with a suffix of Table G-2 where needed.
4. v31np list and an archive: lists its keys and sizes.
5. v31np test and an archive: verifies it, the checksum included.
6. v31np extract, an archive and a folder: extracts into the folder, as set out in § ( Ú/( b.i.1.u. ) ).

### ( ii ) Packages

The programmes are distributed as packages built by installer/build.sh: a package for Debian and Ubuntu, v31np_1.0.0_amd64.deb, described as V31np format for GNU/Linux and maintained by rel. authorities @ &lt;psnpf1031@v31null.de&gt;, a package for Fedora and other systems that use RPM, and an archive .tar.gz with the script install.sh for every other system. The packages install the programmes, the file type and its icon, and the extension of Files for all users; install.sh installs them for one user, and with --menu also the extension of Files, for which administrative rights are required.

## ( ú ) MacOS

No programme supporting .v31np exists for MacOS. Responsibility for this lies solely with Apple.

v31n has already gone a long way to serve every system. The format is defined byte for byte in § ( B ), the behaviour required of programmes on any system is set out in § ( G ), and the means by which a file manager presents an archive as a folder are described in § ( Ú/( a.ii.1. ) ), so that a developer may reproduce them without the source. Apple has not taken a single step in return. It has placed three obstacles before the format.

First, the licence of MacOS permits it to run only on hardware made by Apple. v31n does not develop for a system that cannot be run without buying the hardware of its maker. Should Apple wish its users to be served, it may send v31n a development kit.

Second, MacOS blocks any programme that Apple has not signed and notarised, and Apple signs only for a yearly fee. v31n gives its programmes away without fee (see § ( A/( u ) )) and shall not pay a toll for the right to do so.

Third, Finder offers no interface by which a file may be opened as a folder, as Windows Explorer does through shell namespace extensions. The only means of presenting files in Finder is a File Provider extension, which again requires the signature of Apple.

In all public services, standards, and programmes of ours; v31n demonstrated restraint and goodwill, publishing open specifications so that all parties might coexist on equal terms. Instead of constructive engagement, Apple chose the path of artificial sanctions, proprietary diktat, and closed architecture. We were given no alternative.

Should Apple ask, v31n will provide every particular it requires. 

We will not knock on closed doors, nor will we pay tribute. We remain open to civilized dialogue, but strictly on the basis of equality and mutual respect. But make no mistake: any attempts to impose proprietary hegemony will be met with symmetrical, uncompromising disregard.

# ( U ) V31NP STANDARD

The V31NP STANDARD is the format as defined in § ( B ). Each programme described in § ( Ú ) carries one of two medallions, which state whether the programme conforms to the standard.

The medallion PASSING is carried by a programme of which every .v31np file it writes conforms to § ( B/( a ) ), and which reads every file that conforms. The medallion FAILING is carried by a programme that writes or keeps at least one .v31np file that does not conform to § ( B/( a ) ), or that refuses a file that conforms. Medallions are awarded by v31n after review, by its own decision, and are informational. A programme is rated against § ( G ) only for the functions it offers; a programme that only writes .v31np files, for example, is not expected to present or edit them. Table U-1 presents the medallion carried by each programme described in § ( Ú ).

<table id="table-u-1">
<caption>Table U-1 Medallions</caption>
<thead>
<tr><th>Programme</th><th>Section</th><th>Medallion</th></tr>
</thead>
<tbody>
<tr><th colspan="3">Native Systems</th></tr>
<tr><td>Secura</td><td>§ ( Ú/( a.i.1. ) )</td><td>PASSING</td></tr>
<tr><td>M2</td><td>§ ( Ú/( a.i.2. ) )</td><td>PASSING</td></tr>
<tr><td>V31null Hub</td><td>§ ( Ú/( a.i.3. ) )</td><td>FAILING</td></tr>
<tr><th colspan="3">External Third Parties</th></tr>
<tr><td>Windows Explorer extension</td><td>§ ( Ú/( a.ii.1. ) )</td><td>PASSING</td></tr>
<tr><td>7-Zip plugin</td><td>§ ( Ú/( a.ii.2. ) )</td><td>PASSING</td></tr>
</tbody>
</table>

# ( Ù ) CONSIDERATIONS

## ( a ) Introduction

The properties of the format described in § ( B ) carry costs as well as benefits. These costs are deliberate and follow from the purposes set out in § ( A/( g ) ). The format leaves the use of these properties to the developer, who is responsible for the design of the files written and should exercise judgment in applying the format to a given purpose. The considerations of this division are provided to that end.

## ( b ) Rewriting and Storage

Since every address depends on the length of the header and the checksum covers the whole file, a .v31np file cannot be updated in place (see § ( B/( b.ii. ) )). Any change, whether a file is added, replaced, renamed or deleted, writes the whole file again. A file of 50 GB is therefore written again in full, all 50 GB, even when a single byte or a single key changes. The 7-Zip plugin and M2 write the new file beside the old one for each change and then replace it, so that free space at least equal to the length of the new file is required for the duration of the change. The Windows Explorer extension records changes in a shadow and writes the archive once, when the user leaves it (see § ( G/( b.viii. ) )), so that any number of changes costs a single rewrite; the free space is required at that moment, and the journal and temporary files hold the pending content until then. Figure Ù-1 shows the sequence of a change to a file inside a nested archive.

<p id="figure-ù-1" align="center"><img src="nżúù/ù-1.svg" alt="Figure Ù-1 Rewriting on Change"></p>

Figure Ù-1 Rewriting on Change

Large files that are rewritten repeatedly tend to become fragmented, as the new copy is placed in whatever free space the drive holds at the time. On a hard disk drive, fragmentation slows reading. On a solid-state drive, fragmentation has little effect on reading, but each rewrite writes the full length of the file again.

This cost is offset by the absolute addresses of the format, by which any file is read at its address without decoding, and a file is streamed, resumed and read by byte range. The format is therefore best suited to files that are written once and read many times. Files that change often should be kept apart from files that rarely change, so that archives that are edited remain small.

## ( ú ) Compression

The data of each file is stored raw (see § ( B/( b.ii. ) )). The extracted size of each file therefore equals its stored size, which blocks zip bombs, and each file can be read at its address without decoding. The absence of compression is offset by this direct read access and by the use of the format for general storage.

Most files shared today are already compressed by their own formats, such as images, audio and video, and compressing them again gains little. The developer should therefore optimise the input before packing it, choosing for each file a format that is already compressed where size matters.

## ( u ) Nested Archives

Packing files into a zip or another compressed archive and then storing that archive as a single file in .v31np is strongly discouraged. The header then describes only the outer file, so that the addresses and sizes of the files inside it are not known, they cannot be read by byte range, and their extracted size is no longer bounded by the header. The benefits of the format, and its protection against zip bombs, then no longer apply to those files.

A .v31np file may be stored inside another, and keeps these properties, since its data is raw as well. A change to a nested archive, however, rewrites each archive that contains it (see § ( G/( b.viii. ) )); the Windows Explorer extension does so once for each visit of the archive.

## ( ù ) Metadata

Dates, permissions, owners and attributes are not stored (see Table B-7). The omission is intended: .v31np is a format for storing and sharing the contents of files. Where an application requires such information, it should store it as a file inside the archive, in a format of its own choosing.

## ( g ) The Marks π and φ

It may be supposed that a reader which searches for π and φ could find them in the wrong place, for example inside the data of a file. A reader searches for the marks only within the header. The magic is followed by π, the next π closes T1, and the next closes T2. As shown in § ( B/( b.i. ) ), neither mark can occur inside a key or a number, so that the first π found after the rows of a table is always the π that closes it, and the same holds for φ within the rows.

The data is not searched. Once the header has been read, each file is read at its address for its size, and the footer is read at its fixed position in the last 16 bytes of the file. A π or a φ inside the data of a file, as in any .v31np file stored inside another, is therefore harmless, and a reader has no need to search beyond the header. Figure Ù-2 illustrates what a reader searches and what it reads.

<p id="figure-ù-2" align="center"><img src="nżúù/ù-2.svg" alt="Figure Ù-2 Reading a File"></p>

Figure Ù-2 Reading a File

## ( d ) The Checksum

The checksum is a quick check of the whole file. It does not establish the authenticity of a file, nor who wrote it, nor that it has not been altered on purpose. The format was created for exchange between two parties, and the checksum allows them to compare their copies of a file, in the manner of a client asking a server whether its file matches the server's.

The checksum comes last, so that a client downloading a file can check the last bytes it holds. Where the closing π and the checksum are missing, the download is incomplete, and the client resumes it by byte range from the length it already holds. Figure Ù-3 shows the exchange between a client and a server.

<p id="figure-ù-3" align="center"><img src="nżúù/ù-3.svg" alt="Figure Ù-3 Download and Comparison between Client and Server"></p>

Figure Ù-3 Download and Comparison between Client and Server

A mismatch indicates that the file differs from the file that was written, without showing which file inside it differs (see § ( B/( a.v. ) )). The file should then be requested again from its original owner where that is possible. Where every file inside can be confirmed correct by other means, such as against the copy of the owner or against fingerprints, the archive may be rewritten with a new checksum.

## ( e ) Networking

Since the header precedes the data and gives the address and size of every file, a client need not fetch a whole .v31np file. It may fetch the header alone, offer the files it lists, and fetch each chosen file by byte range from its address for its size. Figure Ù-4 shows this exchange.

<p id="figure-ù-4" align="center"><img src="nżúù/ù-4.svg" alt="Figure Ù-4 Selective Download"></p>

Figure Ù-4 Selective Download

A client that already holds a copy may check whether it is current without fetching the file again. A local copy may carry a checksum that matches its own bytes while its data is stale, so that the copy is compared with that of the server rather than with itself. The server sends its header and its checksum, and the client compares them with those of its copy.

Where they differ, the format allows two courses, and the developer chooses between them. The client may be given agency: it compares the two headers and requests by byte range the files that are missing or of another size, and the whole file where the headers match but the checksums differ. Alternatively, the server may decide, answering that the whole file will be sent since it is small, or that the client shall name the ranges it requires. Figure Ù-5 shows both courses.

<p id="figure-ù-5" align="center"><img src="nżúù/ù-5.svg" alt="Figure Ù-5 Checking a Copy"></p>

Figure Ù-5 Checking a Copy

# ( G ) CONFORMING BEHAVIOUR

## ( a ) Introduction

This division sets out the behaviour required of programmes that present, extract or edit .v31np files, on any operating system. Programmes on different systems shall behave alike, so that a user meets the same results everywhere, and an archive with pending changes passes between programmes unharmed. A programme is bound by a part of this division only where it offers the function concerned; a programme that only writes .v31np files, such as a packer in a browser, is not required to present or edit them.

Part ( b ) sets out the behaviour of programmes inside one system, where every programme that reaches an archive runs on the same machine. Part ( ú ) sets out the rules of the road for an archive that is reached by more than one machine, whether it is carried from one to another on a mounted device, such as a USB stick, or reached by several at once over a network. Since a journal written on one machine refers to the files of that machine, every journal carries the plate of the machine that wrote each of its rows (see § ( G/( ú.i. ) )). On a network, the changes of all machines are directed by the officer of the archive, in the manner of a traffic police officer at a crossing, who admits one car at a time (see § ( G/( ú.iii. ) )).

The requirements of this division are stated with shall, and each binds unless this document states otherwise. Should denotes a recommendation, which does not bind, and may denotes a permission. A requirement binds the result and not the means. The names and wording of commands, the menus that carry them, the interfaces of the system through which they are carried out, and values such as the length of a wait are left to the programme, except the times of the road, which every machine shall share (see § ( G/( ú.iii.5. ) )). The means by which the Windows Explorer extension and the 7-Zip plugin meet these requirements are described in § ( Ú/( a.ii. ) ).

## ( b ) Inside One System

### ( i ) Browsing

A programme shall present a .v31np file as a folder, and a .v31np file stored inside another as a folder in turn. Folders exist as the prefixes of keys (see § ( B/( b.ii. ) )); in addition, empty folders recorded in a journal (see § ( G/( b.vi. ) )) shall be presented until the repack. Names shall be presented as they are stored in the key, since the rules of § ( G/( b.ii. ) ) apply only when files leave the archive. The size of each file shall be presented as stored, and the size of a nested archive with pending changes as the length it will have after the repack (see § ( G/( b.x. ) )). A file opened from inside an archive shall open in the application that the system associates with its type.

Folders should be listed before files, and the names within each group should be sorted in V31N order. Two names are compared first without their extensions, the extension being the part from the last full stop of the name where that full stop is not its first character; where the names are equal in that comparison, the whole names are compared. Before the comparison, each name is folded in five steps, shown in Figure G-1, as follows:

1. White space at the start is skipped, and where the name then begins with [ and a ] follows with at least one character between them, the text between the brackets replaces the name.
2. Punctuation and white space at the start are removed.
3. The name is decomposed to Unicode normalisation form D, each character followed by a combining diaeresis (U+0308) is written twice and the diaeresis dropped, and the result is composed again to form C, so that Ä becomes AA.
4. The sign № followed by digits, with or without white space between them, is written as № and the digits without leading zeros, padded with zeros to five digits.
5. The name is written in lower case.

<p id="figure-g-1" align="center"><img src="nżúù/g-1.svg" alt="Figure G-1 Folding of a Name"></p>

Figure G-1 Folding of a Name

The folded names are compared character by character, as shown in Figure G-2. Equal characters are passed over. Where both characters are listed in Table G-1, the one of lower rank comes first, and characters of equal rank are passed over as equal; the characters of each cell of the table are listed in the order of their places, which serve only to break a tie. A character listed in Table G-1 comes before one that is not, and two characters that are not listed are ordered by their code points. Where one name ends before a difference is found, the shorter comes first.

Two names that are still equal, as A.png and a.png, or ú.png and v.png, are not left to chance: the tie is broken in four steps, as follows:

1. At the first position where the folded names differ, first without their extensions and then whole, the character that stands earlier in its cell of Table G-1 comes first, so that ú comes before v, and v before в.
2. At the first position where one of the names as stored has a capital letter and the other has not, the name with the capital comes first, so that Ab comes before aB.
3. The name of fewer code points as stored comes first.
4. The names as stored are ordered by their code points, so that -a and _a, or №01 and № 1, keep a fixed order.

The names A.png, a.png, AA.png, AAa.png, aaaa.png, ú.png, V.png and v.png are thus sorted in the order given. Punctuation, white space and capital letters are those of one table of character classes, taken from Windows and used on every system, so that programmes on different systems fold and sort alike.

<table id="table-g-1">
<caption>Table G-1 Ranks of V31N Order</caption>
<thead>
<tr><th>Rank</th><th>Characters</th><th>Rank</th><th>Characters</th></tr>
</thead>
<tbody>
<tr><td>0 to 9</td><td>The digits 0 to 9, each of its own value</td><td>24</td><td>m м</td></tr>
<tr><td>10</td><td>a а</td><td>25</td><td>n н</td></tr>
<tr><td>11</td><td>b б</td><td>26</td><td>o о</td></tr>
<tr><td>12</td><td>ú v в</td><td>27</td><td>p п</td></tr>
<tr><td>13</td><td>u у</td><td>28</td><td>r р</td></tr>
<tr><td>14</td><td>ù w</td><td>29</td><td>s с</td></tr>
<tr><td>15</td><td>g г</td><td>30</td><td>t т</td></tr>
<tr><td>16</td><td>d д þ</td><td>31</td><td>f ф</td></tr>
<tr><td>17</td><td>e е э</td><td>32</td><td>ț ц</td></tr>
<tr><td>18</td><td>ż j ж</td><td>33</td><td>č ч</td></tr>
<tr><td>19</td><td>z з</td><td>34</td><td>ș ш</td></tr>
<tr><td>20</td><td>i и</td><td>35</td><td>c</td></tr>
<tr><td>21</td><td>í y й</td><td>36</td><td>q</td></tr>
<tr><td>22</td><td>k к</td><td>37</td><td>x</td></tr>
<tr><td>23</td><td>l л</td><td>38</td><td>h х ‘</td></tr>
</tbody>
</table>

<p id="figure-g-2" align="center"><img src="nżúù/g-2.svg" alt="Figure G-2 Comparison of Two Names"></p>

Figure G-2 Comparison of Two Names

### ( ii ) Extraction

A programme that writes the files of an archive to its system shall derive from each part of a key a name that the system permits, by rules that suit that system; the rules applied on Windows are given in Table Ú-2. Which parts, names and characters are changed is left to the rules of each system, but where a system replaces them it shall use the stand-ins of Table Ú-2, namely AN for a part consisting of . or .., NÍ for an empty name, ! in place of a character and before a reserved name, and 0x followed by two hexadecimal digits in place of a control character. The key in the archive shall not be changed, and no extension shall ever be added. Where two names in one folder are equal by the comparison of the system, the first shall keep its name, and each later one shall receive a suffix before its extension, which begins at the last full stop of the name. A name without a full stop shall receive the suffix at its end, and a name whose only full stop is its first character shall receive it at its start, so that .gitignore becomes a.gitignore. A name that is already taken at the destination, by the comparison of the system, shall not be replaced, and the programme should not ask: a file, and a folder landing on a file, shall receive a suffix in the same manner, while a folder landing on a folder of the same name shall be merged with it, so that only the clashing files inside it receive suffixes. Where the writing of the files is handed to the copy machinery of the system, names already taken are decided as set out in § ( G/( b.iv. ) ). A path that exceeds the length the system permits shall be shortened by the rules of that system, the extension being kept. Extracted files shall carry the date of extraction, and the checksum may be left unverified on extraction. Figure G-3 shows the naming of an extracted file.

<p id="figure-g-3" align="center"><img src="nżúù/g-3.svg" alt="Figure G-3 Naming of Extracted Files"></p>

Figure G-3 Naming of Extracted Files

The suffixes shall be the same on every system and shall be taken in the order of Table G-2. They are counted in the manner of the columns of a spreadsheet, with the 29 letters of the table as digits and no zero: after h follow aa to ah, then ba to bh, and so on to hh, after which aaa follows. The same suffixes shall be given to a new archive or folder whose name is already taken (see § ( G/( b.iii. ) )), to an item added to an archive under a name already taken, and to a copy made into the folder it comes from (see § ( G/( b.iv. ) )).

<table id="table-g-2">
<caption>Table G-2 Letter Suffixes</caption>
<thead>
<tr><th>Number of the later name</th><th>Suffix</th><th>Example for photo.jpg</th></tr>
</thead>
<tbody>
<tr><td>1 to 29</td><td>a, b, ú, u, ù, g, d, e, ż, z, i, í, k, l, m, n, o, p, r, s, t, f, ț, č, ș, c, q, x, h</td><td>photoa.jpg to photoh.jpg</td></tr>
<tr><td>30 to 58</td><td>aa to ah</td><td>photoaa.jpg to photoah.jpg</td></tr>
<tr><td>59 to 87</td><td>ba to bh</td><td>photoba.jpg to photobh.jpg</td></tr>
<tr><td>842 to 870</td><td>ha to hh</td><td>photoha.jpg to photohh.jpg</td></tr>
<tr><td>871</td><td>aaa</td><td>photoaaa.jpg</td></tr>
</tbody>
</table>

### ( iii ) Commands

A programme that integrates .v31np files into a file manager shall offer the functions of Table G-3. Their names, and the menus that carry them, are left to the programme; the names used on Windows are given in Table Ú-1. In the table, the name of an archive means its file name without .v31np. A programme may give the folder baíbíz e alùz another name or a translation of it. Where that folder, or an archive of the name chosen when packing, already exists, the new one shall receive a suffix of Table G-2.

<table id="table-g-3">
<caption>Table G-3 Commands</caption>
<thead>
<tr><th>Function</th><th>Selection</th><th>Result</th></tr>
</thead>
<tbody>
<tr><td>Extract into a folder of the name of the archive</td><td rowspan="4">One .v31np file</td><td>Files written into a new folder of that name beside the archive</td></tr>
<tr><td>Extract here</td><td>Files written into the folder of the archive</td></tr>
<tr><td>Extract into a picked folder</td><td>Files written into a folder picked by the user</td></tr>
<tr><td>Extract into a folder of the name of the archive inside a picked folder</td><td>Files written into a new folder of that name inside a folder picked by the user</td></tr>
<tr><td>Extract several here</td><td rowspan="2">Several .v31np files</td><td>Each archive extracted into a folder of its name inside a new folder baíbíz e alùz in the folder of the archives</td></tr>
<tr><td>Extract several into a picked folder</td><td>Each archive extracted into a folder of its name inside a new folder baíbíz e alùz in a folder picked by the user</td></tr>
<tr><td rowspan="2">Pack</td><td>Files and folders of the system</td><td>A new .v31np file beside the selection, which may be named after its first item, a file without its extension; the selection is copied</td></tr>
<tr><td>Files and folders inside an archive</td><td>A new .v31np file in the same folder, which may be named in the same manner; the selection is moved into it</td></tr>
<tr><td>Create an archive</td><td>None, in a folder of the system</td><td>An empty .v31np file, as in Table B-6</td></tr>
<tr><td>Create inside an archive</td><td>None, inside an archive</td><td>A new item of any type that the system can create, which may be made ready to be named at once</td></tr>
</tbody>
</table>

An extraction never replaces what the destination already holds; names already taken there receive a suffix of Table G-2, and folders are merged, as set out in § ( G/( b.ii. ) ).

### ( iv ) Editing

The functions that a programme that edits archives shall allow, inside an archive and inside every archive nested in it, are as follows:

1. Adding files and folders from the system, by copy and by move.
2. Copying and moving within one archive, between archives, between nested levels, and onto a nested archive.
3. Copying and moving out of an archive onto the system.
4. Renaming.
5. Deleting.
6. Creating new items.
7. Packing a selection into a new archive (see Table G-3).
8. Saving back a file opened from inside the archive.

A move out of an archive onto the system shall, where applicable, remove from the archive only the items that have reached the system, and shall remove them through the shadow in the manner of a delete. Items that were skipped or not written, as where the move is cancelled, stay in the archive.

Where a file opened from inside an archive opens from a copy on the system, a programme that edits archives shall watch such a copy for as long as it runs, and shall save each change of the copy back into the archive as one change, at the key from which the file was opened. Where that entry has meanwhile been renamed, moved or deleted, the file shall be saved at its original key, and so recreated there. Where the entry has meanwhile been changed by another programme or on another machine, so that it holds neither the content it held when the copy was opened nor the content of the last save, the save shall not replace it; it shall land beside it under a suffix of Table G-2, so that the work of neither is lost, and later saves of the same copy shall follow to the suffixed key. A save that cannot yet be read, as while the application still writes the copy, shall be tried again. A change saved back should be visible at once in every view of the archive.

An item added to an archive shall, where applicable, never replace an entry of the same name, and the programme should not ask. A clash is determined on the key, compared exactly as stored, level by level as the folder presents it, so that a leading / and empty parts of a key are disregarded. A file that lands on a file or on a folder of the same name shall receive a suffix of Table G-2, as eayk.png becomes eayka.png and then eaykb.png, and a folder that lands on a file shall receive a suffix and keep all it holds. A folder that lands on a folder of the same name shall be merged with it, and only the clashing files inside it shall receive suffixes; empty folders follow the same rules. These rules apply to every way in, namely copy and move from the system, from another archive or another level, from sources without files on the system, and to another folder of the same level. An item copied into the folder it comes from shall receive a suffix as well, and a folder so copied shall become a new folder of that name with the suffix, holding a full copy, and shall not be merged into itself; an item moved onto its own place is left as it is.

Where the writing of files, into an archive or out of it, is handed to the copy machinery of the system, that machinery decides on names already taken, and the rules of this part and of § ( G/( b.ii. ) ) on such names do not apply to that writing. On Windows, this is the case for a drag or a paste out of an archive onto the disk, and for programmes that write into an archive through the copy engine (see § ( Ú/( a.ii.1.ù. ) )).

The programme itself should not ask for confirmation before deleting; users are presumed to know what they do. A rename shall be refused where the new name is empty, contains / or is already used in the folder, and a folder shall not be moved into itself or into a folder inside it. A rename given through the calls of a mounted file system onto a name already used is the one exception: it shall replace the item of that name, since editors save a file by writing a temporary file and renaming it over the original.

Since a .v31np file cannot be updated in place (see § ( B/( b.ii. ) )), changes shall not be written to the archive one by one. Each change shall be recorded in a shadow, as set out in § ( G/( b.v. ) ), and the archive shall be written once, when the user leaves it. A change should be visible at once in every view of the archive, and shall be visible to every programme that reads through the journal.

### ( v ) The Shadow

The shadow is the state of an archive together with its pending changes. It consists of the archive on disk, a journal beside it (see § ( G/( b.vi. ) )), and the content to which the journal refers, namely files on the system that were added as links and temporary files holding content that has no place on the system. The archive itself shall not be touched until the repack (see § ( G/( b.viii. ) )). On a shared road, the shadow is not used, and each change is written into the archive at once (see § ( G/( ú.iii.1. ) )).

Every change shall run as shown in Figure G-4, as follows:

1. The programme takes a lock of the archive that excludes every other programme changing the same archive (see § ( G/( b.ix. ) )).
2. It reads the journal and discards it if it is stale.
3. It applies the change to the sections of the journal.
4. It writes the journal whole.
5. It releases the lock.

Views and readers then present the new state. When the last view of the archive closes and no view opens again within a short wait, the programme shall repack the archive.

<p id="figure-g-4" align="center"><img src="nżúù/g-4.svg" alt="Figure G-4 Life of a Change"></p>

Figure G-4 Life of a Change

A file added from the system shall not be copied. The journal shall refer to it, and it shall be read at the repack. A file moved in from the system shall stay where it is until the repack, after which it shall be deleted. Meanwhile the original shall not be presented on the system, where the system provides a means of hiding files: the programme shall hide each item moved in at the top of the move, a folder as a whole without the files inside it, and shall record it in the journal as a row H in the section of the level into which it was moved (see Table G-6). An original that was hidden already shall not be recorded and shall never be touched. After every change, a row H whose path is no longer referred to by any row M or R of the journal, since the item has been deleted inside the archive or moved on, shall be removed and its original shown again. Where the item is moved on to another archive, its row H shall be carried into the journal of that archive and removed from the source journal, and the original shall remain hidden; a move within one archive leaves the row H in place. A folder created inside an archive shall live in the journal until the repack, and shall be dropped then if it is still empty, since the format stores no empty folders.

### ( vi ) The Journal

The journal of an archive shall be a file in the same folder whose name is FUTALUZTU UERSIIA UUTI, a space, the file name of the archive without .v31np, and .v31np.txt; for photos.v31np it is FUTALUZTU UERSIIA UUTI photos.v31np.txt. The name shall be the same on every system. The journal shall be hidden where the system provides a means of hiding files; a full stop before the name is permitted for this purpose, and every programme shall look for the journal under both names. Where a journal exists under both names, a programme shall compare the two; where they are not the same, it shall use the one written most recently among those it can read.

The journal shall be UTF-8 text. Lines shall be written ending with a carriage return and a line feed, and a reader shall also accept a line feed alone; empty lines shall be ignored. The fields of a line shall be separated by a tab. Text fields shall be escaped as in Table G-4 and shall be valid UTF-8 once unescaped, and keys and paths shall not be empty. Numbers shall be written in decimal, without sign or separators, with at most 20 digits, and shall be less than 2⁶⁴. Table G-5 presents the lines of a journal, and Figure G-5 presents an example.

<table id="table-g-4">
<caption>Table G-4 Escapes in the Text of a Journal</caption>
<thead>
<tr><th>Written</th><th>Character</th></tr>
</thead>
<tbody>
<tr><td>\\</td><td>Backslash</td></tr>
<tr><td>\t</td><td>Tab</td></tr>
<tr><td>\n</td><td>Line feed</td></tr>
<tr><td>\r</td><td>Carriage return</td></tr>
<tr><td>\ followed by any other character</td><td>Not permitted; the journal is unreadable</td></tr>
</tbody>
</table>

<table id="table-g-5">
<caption>Table G-5 Lines of a Journal</caption>
<thead>
<tr><th>Line</th><th>Fields</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>Header</td><td>v31np shadow 2, length, time</td><td>The first line that is not empty. The length of the archive in bytes, and the time of its last write as the number of intervals of 100 nanoseconds since 1 January 1601 UTC</td></tr>
<tr><td>Officer</td><td>G, N, S, L, Q or W, then the fields of Table G-10</td><td>The lines of the officer, between the header and the first section (see § ( G/( ú.iii.3. ) ))</td></tr>
<tr><td>Section</td><td>@, then none or more keys</td><td>Opens the section of one level: no key for the archive itself, the key of a nested archive for the archive stored there, and further keys for archives nested deeper, each relative to the archive before it</td></tr>
<tr><td>Row</td><td>Kind, then the fields of Table G-6</td><td>One entry of the level of the section above it</td></tr>
</tbody>
</table>

<p id="figure-g-5" align="center"><img src="nżúù/g-5.svg" alt="Figure G-5 Layout of a Journal"></p>

Figure G-5 Layout of a Journal

The time in the header shall be the file time of Windows. A programme on another system shall convert the last-write time of its system to that form, so that a journal written on one system can be checked on another. Each level shall have at most one section. A section shall hold the complete list of entries of its level after all changes, and not a record of operations; a level without a section is as stored. A row before the first section, an officer line after it, an officer line other than Q and W given twice, an unknown kind, a wrong number of fields, a row with a path but without a plate, a number that is not valid or text that is not valid makes the whole journal unreadable. Table G-6 presents the kinds of rows. Paths in rows shall be absolute paths of the system that wrote the journal, and addresses in rows A shall be offsets within the archive of their level. Every row with a path shall end with the plate of the machine that wrote it (see § ( G/( ú.i. ) )).

A programme shall write the header v31np shadow 2. A journal headed v31np shadow 1, written before this revision, shall still be read: it holds no officer lines, its rows carry no plates, and they count as rows of the machine that reads it. The next change writes the journal again under the header v31np shadow 2, every row with a path then carrying the plate of that machine. Times in officer lines and in rows D are written in the storage form of PS/Í-1103, the clock of v31n, as in 20261101PM100101 (see § ( E )), and are taken in UTC+9, never in the local zone of a machine.

<table id="table-g-6">
<caption>Table G-6 Kinds of Rows</caption>
<thead>
<tr><th>Kind</th><th>Fields</th><th>Meaning</th><th>At the repack</th></tr>
</thead>
<tbody>
<tr><td>A</td><td>key, size, address</td><td>Data still in the archive of the level, at the address and of the size given</td><td>Copied from the archive</td></tr>
<tr><td>F</td><td>key, size, path, plate</td><td>A file on the system, linked; the size as it was when linked</td><td>Read as it is at the repack; left out if missing</td></tr>
<tr><td>M</td><td>key, size, path, plate</td><td>As F, but moved in from the system</td><td>As F; the file is deleted after the repack</td></tr>
<tr><td>T</td><td>key, size, path, plate</td><td>A temporary file of the programme, holding content that has no place on the system</td><td>Read; deleted after the repack</td></tr>
<tr><td>D</td><td>key; on a shared road key, plate, until</td><td>An empty folder; on a shared road, held by the machine of the plate until the time given (see § ( G/( ú.iii.7. ) ))</td><td>Left out</td></tr>
<tr><td>R</td><td>path, plate</td><td>A folder of the system moved in</td><td>Removed after the repack, with its empty subfolders, if it is empty</td></tr>
<tr><td>H</td><td>path, plate</td><td>An original on the system that the programme hid when it was moved in</td><td>Shown again after the repack if it still exists</td></tr>
</tbody>
</table>

The journal shall be written whole after every change, to a temporary file in the same folder that then replaces it, so that no programme ever reads a journal that is partly written. Temporary files of rows T are kept by the programme in a folder of its choosing. Figure G-6 shows the content to which each row of Figure G-5 refers.

<p id="figure-g-6" align="center"><img src="nżúù/g-6.svg" alt="Figure G-6 Rows and Their Content"></p>

Figure G-6 Rows and Their Content

### ( vii ) Changes

Table G-7 presents the rows that each change writes. A change shall not place an item at a key that is already taken; the item shall receive a suffix of Table G-2 or be merged, as set out in § ( G/( b.iv. ) ). Where a row is nonetheless placed at a taken key, as where the copy machinery of the system writes into an archive and so decides (see § ( G/( b.iv. ) )), it shall replace any stored row at that key, every row under it, and any stored row whose key is a folder above it, so that a file can replace a folder of the same name and a folder a file. A row D at or above the key of a new row shall be removed, since the folder is no longer empty.

<table id="table-g-7">
<caption>Table G-7 Changes and the Rows They Write</caption>
<thead>
<tr><th>Change</th><th>Rows</th></tr>
</thead>
<tbody>
<tr><td>Copy from the system</td><td>F for each file; folders are entered recursively, linked folders are not followed</td></tr>
<tr><td>Move from the system</td><td>M for each file, R for each folder moved, and H for each original hidden at the top of the move</td></tr>
<tr><td>Copy within one level</td><td>The rows copied under the new key; M copied as F</td></tr>
<tr><td>Move within one level, rename</td><td>The keys of the rows, and of the sections of nested archives under them, rewritten</td></tr>
<tr><td>Copy or move from another archive or another level</td><td>T for each row A, its data copied to a temporary file; T for each nested archive with a section, built into a temporary file; F for each row F; M for each row M on a move and F on a copy; a new T for each row T; D for each row D. On a move, the rows are then deleted from the source; on a move to another archive, the rows H of the items moved are carried with them</td></tr>
<tr><td>Content from a source without files on the system, such as another virtual folder or a device</td><td>T for each file, D for each folder</td></tr>
<tr><td>Create inside an archive</td><td>T for a file, D for a folder</td></tr>
<tr><td>Delete</td><td>The rows under the key and the sections of nested archives under it removed; D for the folder above, where it is left empty</td></tr>
<tr><td>Pack inside an archive</td><td>T for the new archive; the rows of the selection removed</td></tr>
</tbody>
</table>

A row M that is copied shall become F, so that the original is deleted only once, together with the row that moved it. Deleting the last entry of a folder shall leave the folder as a row D until the repack, as shown in Figure G-7. Renaming or moving a folder shall rewrite the keys of its rows and of the sections of nested archives within it, as shown in Figure G-8.

<p id="figure-g-7" align="center"><img src="nżúù/g-7.svg" alt="Figure G-7 Deletion of the Last File of a Folder"></p>

Figure G-7 Deletion of the Last File of a Folder

<p id="figure-g-8" align="center"><img src="nżúù/g-8.svg" alt="Figure G-8 Renaming of a Folder Holding a Nested Archive"></p>

Figure G-8 Renaming of a Folder Holding a Nested Archive

### ( viii ) The Repack

The repack writes the archive with all its pending changes. A programme shall repack an archive after the last of its views of the archive has closed and no view has opened again within a short wait, so that walking between the folders of an archive, which closes one view and opens another, does not cause a repack. Figure G-9 shows views, wait and repack over time. The repack may also be carried out by another programme that leaves the archive earlier; the result is the same, only sooner.

<p id="figure-g-9" align="center"><img src="nżúù/g-9.svg" alt="Figure G-9 Views, Wait and Repack"></p>

Figure G-9 Views, Wait and Repack

The repack shall run under the lock, as shown in Figure G-10. A stale journal shall be discarded, and a missing or unreadable journal leaves nothing to do. A journal that holds a foreign row, written on another machine or holding a path that the system cannot interpret, shall neither be repacked nor discarded, and shall be left for the machine that wrote it; a stale journal shall be discarded nonetheless, in the manner of § ( G/( ú.i. ) ). Otherwise, the nested archives that have sections shall be built first into temporary files, the innermost first, as shown in Figure G-11, and the archive shall then be written in full beside itself, under its file name, a full stop, 32 hexadecimal digits in lower case, and .tmp, in the manner of § ( Ù/( b ) ), and shall replace the old file. Linked files shall be read as they are at that moment, so that a changed file is packed changed and a missing file is left out. Rows D shall be left out.

<p id="figure-g-10" align="center"><img src="nżúù/g-10.svg" alt="Figure G-10 The Repack"></p>

Figure G-10 The Repack

<p id="figure-g-11" align="center"><img src="nżúù/g-11.svg" alt="Figure G-11 Order of Writing Nested Archives"></p>

Figure G-11 Order of Writing Nested Archives

Only after the new archive has replaced the old one shall the originals of rows M be deleted, the temporary files of rows T be deleted, the folders of rows R be removed if they are empty, the originals of rows H that still exist be shown again, and the journal that was used be deleted. A journal under the other name (see § ( G/( b.vi. ) )) shall be left; it no longer matches the archive, and is found stale and discarded when it is next read. Where writing or replacing fails, the new file shall be removed, and the journal and all pending content shall be kept. Where a programme ends before the repack, the journal shall remain, and the archive shall be repacked when it is next opened and left.

### ( ix ) Stale Journals and Concurrency

A journal is valid only for the archive against which it was written. Before using a journal, a programme shall compare the length and the last-write time of the archive with the header of the journal, as shown in Figure G-12. Where they differ, the archive has been changed by another tool since, the addresses of rows A no longer hold, and the journal is stale. A stale journal shall be discarded, except on a shared road (see § ( G/( ú.iii.1. ) )), under the lock, together with those temporary files of its rows T that the programme made itself, and every original of its rows H shall be shown again, unless the journal holds a foreign row, in which case only the journal file is removed (see § ( G/( ú.i. ) )); the archive on disk shall prevail, and its pending changes are lost. Only the journal that was used shall be discarded; a journal under the other name is left, and is checked in the same manner when it is next read. An unreadable journal shall be ignored, and the next change shall start a new journal from the archive on disk; originals hidden through its rows H remain hidden (see Table G-9).

<p id="figure-g-12" align="center"><img src="nżúù/g-12.svg" alt="Figure G-12 Check of a Journal"></p>

Figure G-12 Check of a Journal

Changes made by several programmes, or by several threads of one, shall be serialised by a lock for each archive that holds across programmes, as shown in Figure G-13. Each change shall read the journal under the lock, so that it builds on the changes made before it. A programme may keep a parsed journal in memory, provided that it detects a newer journal, for example by its length and last-write time.

<p id="figure-g-13" align="center"><img src="nżúù/g-13.svg" alt="Figure G-13 Two Programmes Changing One Archive"></p>

Figure G-13 Two Programmes Changing One Archive

### ( x ) Readers

A programme that reads an archive for which a journal exists shall present the archive with its pending changes. For each level, it shall take the entries from the section of that level where one exists, and from the archive otherwise, as shown in Figure G-14. A nested archive that has a section of its own shall be presented with the length it will have after the repack, computed from the journal, and its content, when it is read as a file, shall first be built into a temporary file.

<p id="figure-g-14" align="center"><img src="nżúù/g-14.svg" alt="Figure G-14 Listing a Level through the Journal"></p>

Figure G-14 Listing a Level through the Journal

A programme that can write .v31np files only by writing a whole new archive, and therefore cannot write into the shadow, shall refuse changes while a journal exists, and may read through it.

## ( ú ) Across Systems

An archive may be reached by more than one machine. It may be carried from one machine to another on a mounted device, or it may lie on a network, where several machines reach it at once. Each machine may run another system, with paths, names and a clock of its own. This part sets out how the rows of each machine are known by its plate, when a machine shall leave the journal of another untouched, and the rules of the road by which several machines change one archive on a network without loss.

### ( i ) Plates

Every machine shall carry a plate, by which its rows in a journal, its cars at the post of an archive and its records in the custody of an archive are known. The plate shall be the host name of the machine, a hyphen, and a number of at most nine digits drawn from the identifier of the machine, as in pingu-346656037. The number shall be the 64-bit FNV-1a hash of the identifier, written in lower case, taken modulo 10⁹. The plate is therefore the same for every programme on a machine and at every start, and differs between two machines that share a host name. The identifier used on each system is given in § ( Ú ).

Together with its plate, a machine shows its papers where this part requires it. The papers are the plate, and the name and version of the system of the machine, as in Ubuntu 26.04.1 LTS or Windows 10 Pro 10.0.19045.

A row is foreign to a machine where it carries the plate of another machine, or where it holds a path that the system of the machine cannot interpret, such as a path of another system. Rows A and D hold no path and are never foreign. A journal that holds at least one foreign row is read-only on that machine. The programme shall present the archive through it, as set out in § ( G/( b.x. ) ), but shall not open the files that foreign rows name, so that their entries are listed but cannot be read there. It shall neither change the archive, nor repack it, nor discard the journal, which waits for the machine that wrote it. A stale journal is the only exception: it shall be discarded nonetheless, but only the journal file itself shall be removed, and nothing it names, neither the temporary files of its rows T nor the originals hidden through its rows H, shall be touched.

A programme shall delete only the temporary files it has made itself. It shall know them by a folder of its own and a form of name of its own, and shall never delete the path of a row T that lies elsewhere or has another form, even where the row carries its own plate. The folders and forms used on each system are given in § ( Ú ).

### ( ii ) Mounted Devices

A mounted device, such as a USB stick, a memory card or an external disk, is a disk of the machine into which it is plugged. An archive on such a device lies off the road: it is edited as set out in part ( b ), with its journal beside it on the device. The device may, however, be pulled out while changes are pending, and plugged into another machine, as shown in Figure G-15. The journal then names files of the first machine, which the second cannot reach, and its rows carry the plate of the first.

<p id="figure-g-15" align="center"><img src="nżúù/g-15.svg" alt="Figure G-15 A Device Carried between Machines"></p>

Figure G-15 A Device Carried between Machines

On the second machine, the journal is read-only (see § ( G/( ú.i. ) )). The archive is presented with its pending changes, but the files linked from the first machine cannot be opened there, and no change can be made. When the device returns to the first machine, the journal is its own again, and the archive is repacked when it is next opened and left. Where the archive is changed on the second machine by a programme that does not follow this division, the journal becomes stale and is discarded, and its pending changes are lost (see § ( G/( b.ix. ) )).

Rules to follow before a device is pulled out are as follows:

1. Every archive on the device should be left in every programme, so that it is repacked and carries no journal.
2. Where a journal is still beside an archive, the device should be returned to the machine that wrote it before the archive is changed elsewhere.

### ( iii ) Networks

This part is the code of the road for archives on a network. Several machines may reach one archive at the same moment, and an archive cannot be changed in place (see § ( B/( b.ii. ) )), so that two machines writing at once would each destroy the work of the other. Every change is therefore a car. Cars meet at the post of the archive, where its officer admits one car at a time onto the lane, and every car that has driven leaves its record in the custody of the archive. The rules are written for programmes, but they are meant to be understood by everyone who meets them: the employee who saves a file on a share, the operator of the network that carries it, and the administrator of the server on which it lies.

#### 1. Shared Roads

An archive lies on a shared road where it is reached over a network, namely where it lies on a network share, and where it lies on a local disk inside a folder that its machine shares on the network. The machine that shares the folder shall treat a path inside it as a shared road, although the path is local to it, since other machines reach the same archive through the network at the same time. Every other archive lies off the road and is edited as set out in part ( b ). The means by which each system recognises a shared road are given in § ( Ú ).

The shadow of part ( b ) shall not be used on a shared road. A journal there could hold the files of only one machine, and a repack on leaving could come long after another machine had changed the archive, so that the stale journal would lose every change it held. On a shared road, each change shall instead be written into the archive at once, as one car (see § ( G/( ú.iii.6. ) )), and the journal shall hold only the officer and sections of rows A and D. A journal on a shared road that holds a row F, M, T, R or H is unreadable. A programme shall read the journal of a shared road before every request, since the officer and the leased folders exist only there. A stale journal on a shared road shall not be discarded: its sections shall be dropped and its officer kept, since the officer belongs to the road and not to the bytes of the archive. Once it holds an officer, the journal of a shared road shall remain beside the archive, as the post does.

#### 2. The Post and the Signal

Beside every archive on a shared road stands its post, a file whose name is NIEURFASMIT UUTI, a space, the file name of the archive without .v31np, and .v31np.lock; for photos.v31np it is NIEURFASMIT UUTI photos.v31np.lock. The name shall be the same on every system. The post shall be hidden where the system provides a means, though never by a full stop before its name. A programme creates the post when it is first needed. The post is permanent: no programme shall ever rename or delete it, since a car waiting at a post that had been removed and set up anew would wait at a signal that no other car can see.

The signal of the post is an exclusive lock of the single byte at offset 2⁴⁰ of the post, taken by the means that the system provides for locking a range of a file. The byte lies far beyond the end of the post, so that the lock never hinders the reading and writing of the post itself. The signal means as follows:

1. A green signal means that the machine holding it may visit the officer. Only one machine at a time holds a green signal.
2. A red signal means stop. Another machine is with the officer, and the machine shall wait until the signal turns green for it.

The signal shall be held only for a short visit to the officer, to read and write the journal, and never while an archive is written. While it holds a green signal, a machine shall write its papers at the start of the post, as plate=, the plate, a space, os=, and the name and version of its system, so that a waiting machine can learn who holds the signal. Having written them, it shall take the last-write time of the post, which the file server sets, as the time of the officer (see § ( G/( ú.iii.4. ) )).

#### 3. The Officer

The officer of an archive is kept in its journal, in the lines between the header and the first section, as presented in Table G-10. The officer knows the current shift, the next ticket to be given, whether the archive lies on a shared road, the car that holds the lane, the cars waiting in the queue, and the last 16 events, which Table G-11 presents. Every visit to the officer shall be made as shown in Figure G-16, as follows:

1. The car waits for a green signal and writes its papers.
2. It takes the time of the officer from the post.
3. It reads the journal and sweeps it: a lane, a car or a leased folder whose time has run out is struck out, and a lane so struck begins a new shift.
4. It does its business with the officer, as set out in the rules below.
5. It writes the journal whole, as set out in § ( G/( b.vi. ) ), and turns the signal red.

<p id="figure-g-16" align="center"><img src="nżúù/g-16.svg" alt="Figure G-16 The Post, the Signal and the Officer"></p>

Figure G-16 The Post, the Signal and the Officer

<table id="table-g-10">
<caption>Table G-10 Lines of the Officer</caption>
<thead>
<tr><th>Line</th><th>Fields</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>G</td><td>shift</td><td>The number of the current shift; it increases with every car waved on, every override and every lane that runs out</td></tr>
<tr><td>N</td><td>next</td><td>The ticket to be given to the next car that joins; at least 1</td></tr>
<tr><td>S</td><td>shared</td><td>1 where the archive lies on a shared road, 0 otherwise</td></tr>
<tr><td>L</td><td>ticket, plate, system, granted, until, shift</td><td>The car that holds the lane, the time it was waved on, the end of its lease and its shift; absent while the lane is free</td></tr>
<tr><td>Q</td><td>ticket, plate, system, joined, until, urgent, seen, checked, clock, then none or more keys</td><td>One car in the queue: the time it joined, the end of its lease, 1 if it is urgent, the shift it saw when it joined, the time of the officer at its last visit, its own clock at that visit, and the keys it intends to change</td></tr>
<tr><td>W</td><td>ticket, plate, system, time, event</td><td>One event of the log, the oldest first; at most 16 are kept</td></tr>
</tbody>
</table>

Lines G, N, S and L shall each be given at most once, and lines Q and W once for each car and each event. Times are written as set out in § ( G/( b.vi. ) ).

<table id="table-g-11">
<caption>Table G-11 Events of the Officer</caption>
<thead>
<tr><th>Event</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>wave</td><td>A car was waved onto the lane</td></tr>
<tr><td>leave</td><td>The holder left the lane after landing</td></tr>
<tr><td>expired</td><td>A lane or a car was struck out, since its lease had run out</td></tr>
<tr><td>withdraw</td><td>A car left the queue without driving</td></tr>
<tr><td>skew</td><td>A car was not waved on, since its clock is off</td></tr>
<tr><td>condemn</td><td>A clock is off, but all the clocks are off alike; the car keeps its turn</td></tr>
<tr><td>override</td><td>A car took the lane by override</td></tr>
<tr><td>overridden</td><td>The holder lost the lane by override</td></tr>
<tr><td>tag</td><td>A waiting car marked the holder as stuck</td></tr>
</tbody>
</table>

#### 4. The Clock of the Officer

The officer never trusts the clock of a driver. On a shared road, the time of the officer shall be the clock of the file server, read as the last-write time of the post after the papers have been written; off the road, it is the clock of the machine. Every time in the officer, in the leases of folders and in the records of custody shall be the time of the officer.

A car reports its own clock at every visit. The rules of the clock are as follows:

1. A car whose clock differs from the time of the officer by no more than 60 seconds is in order.
2. A car whose clock differs by more than 60 seconds shall not be waved on. The officer logs skew, and the car waits, so that a machine with a wrong clock cannot put false times into the records of an archive.
3. Where that car is alone in the queue, or where every other car is off as well and within 60 seconds of it, the fault lies with the clock of the officer and not with the cars. The officer logs condemn, a citation is written into the custody of the archive (see § ( G/( ú.iii.9. ) )), and the car keeps its turn, since the police are there for the people.

A car that is not waved on for its clock for the whole of its lease shall give up: it withdraws from the queue, and its change fails without changing anything. The clocks of the machines and of the file server should therefore be kept in step by the network.

#### 5. Tickets, the Queue and the Lane

A machine that wishes to change an archive on a shared road joins the queue as a car. At its visit it takes a ticket, the number N of the officer, which then increases by one, and it enters the queue with its papers, the time it joined, the end of its lease, the shift it saw, the time of the officer, its own clock and the keys it intends to change. A car may be marked urgent. A machine that already has a car in the queue keeps its ticket, and its visit renews that car. Every lease, whether of a car, of a lane or of a leased folder, shall run for 120 seconds from the visit that sets or renews it. A car shall visit the officer again and again while it waits, each visit renewing its lease; a car that does not return before its lease runs out is struck from the queue. A car that no longer wishes to drive shall withdraw.

The right of way at the post is as follows:

1. The car that holds the lane has the right of way ahead of every car in the queue.
2. An urgent car has the right of way ahead of every car that is not urgent.
3. Among the others, the car with the lower ticket has the right of way: first come, first served.
4. A car that is not waved on for its clock yields to every other car.
5. Even with the right of way, a car is waved on only at its own visit to the officer.

When the lane is free, the officer waves on the car that has the right of way. It leaves the queue and holds the lane, and a new shift begins, whose number the car keeps. While it holds the lane, the car shall renew its lease at a visit to the officer at least every 40 seconds. A renewal shall be refused where the lane no longer holds the ticket of the car in its shift. A lane whose lease has run out is struck out at the next visit of any car, and a new shift begins, so that a holder that returns late finds its shift gone. A car that finds its shift gone, whether its lease ran out or it was overridden, shall abandon its change. Figure G-17 shows a queue and its lane over time.

<p id="figure-g-17" align="center"><img src="nżúù/g-17.svg" alt="Figure G-17 The Queue and the Lane over Time"></p>

Figure G-17 The Queue and the Lane over Time

#### 6. One Change, One Car

Every change on a shared road is one car, whatever it holds: a copy of many files, a rename, a delete, an empty folder created, or a save of a file by an editor. A file that is being written into an archive drives once its writing is complete. Each car shall drive as shown in Figure G-18, as follows:

1. Join. The car joins the queue and visits the officer until it is waved on.
2. Look. Holding the lane, with the signal red, the car reads the archive and the journal as they are now. Where an entry that the change concerns has changed since the view from which the change was given, the change shall be refused and the view refreshed, so that nobody changes what they have not seen.
3. Build. The car applies the change by name to the archive as it is now; where its target has vanished meanwhile, the change fails as not found. It writes the new archive beside the old one, together with its record of custody, under the file name of the archive, a full stop, 32 hexadecimal digits in lower case, and .tmp. It then reads the new archive back and verifies it. While it builds, it renews its lease.
4. Land. With a green signal, the car checks that it still holds the lane in its shift, and that the archive is still the one it read, by its length and last-write time. It then replaces the archive with the new one, trying again for a while where a reader still holds the archive open. It writes the journal, with the officer after the car has left the lane and with sections of rows A and D for the levels that hold leased folders, and turns the signal red.

<p id="figure-g-18" align="center"><img src="nżúù/g-18.svg" alt="Figure G-18 The Drive of a Car"></p>

Figure G-18 The Drive of a Car

Where any step fails, or the car finds its shift gone, it shall remove its own temporary file, and the archive and the journal remain as they were. Nothing is lost but the change itself, which may be given again. A temporary file left beside an archive by a machine that stopped while it was building shall never be removed by a programme (see Table G-9).

#### 7. Leased Folders

Since the format stores no empty folders, an empty folder created on a shared road lives in the journal as a row D, with the plate of its creator and the end of its lease. The creator shall renew the lease at least every 40 seconds while it has the folder open, and every machine presents the folder meanwhile. The row shall be struck out when its lease runs out, or when an item lands under the folder, which then exists through its entries.

#### 8. Breakdowns

A holder that stops renewing its lane, as where its machine has frozen or has lost the network, keeps the lane until its lease runs out, and every other car waits. What a programme shall do while the holder appears broken down is shown in Figure G-19, and is as follows:

1. A holder that is alive and renews its lane shall be waited for without asking, however long it drives.
2. Where only two machines are concerned, the holder and one waiting car, and the holder has missed its renewals, so that no renewal of the lane has been seen for 80 seconds and less than 40 seconds of its lease remain, the programme of the waiting car shall ask its user whether to go ahead, naming the holder by its papers.
3. Where the user agrees, the car takes the lane at once by override. A new shift begins, the officer logs overridden for the holder and override for the car, and a record of custody override is written. The holder, on its return, finds its shift gone and abandons its change.
4. Where the user declines, or does not answer, nothing changes, and the lease of the holder runs out by itself.
5. Where more than two machines are concerned, no car shall override. A waiting car may instead tag the holder, which the officer logs as tag. A tag is a warning to the other cars and to the holder, and does not change the lane.

<p id="figure-g-19" align="center"><img src="nżúù/g-19.svg" alt="Figure G-19 A Breakdown and an Override"></p>

Figure G-19 A Breakdown and an Override

#### 9. Custody

Every car that changes an archive on a shared road shall leave a record of custody inside it, so that the archive itself carries the record of who changed it, when and how. Records are kept in the folder ALICEV31NP at the top of the level whose listing the change alters. Only that level receives the record; the archives around it, although they are written anew, receive none. The folder is flat and is not hidden, and it travels with its archive wherever the archive is copied or extracted. Each record shall be written as two files, as follows:

1. ALICEV31NP/, the plate, \, a number and .txt, as in ALICEV31NP/pingu-346656037\10.txt. The number counts the records of that machine in the archive, from 1.
2. ALICEV31NP/, the year, \, a number, a hyphen, the number of the first file and .txt, as in ALICEV31NP/2026\3-10.txt. The number counts the records of that year in the archive, from 1. This file keeps the order of events: its first line holds the key of the first file, and the record follows unchanged, so that a person or a programme can rebuild the chain of events, even where the records of two machines carry the same number.

In both keys, \ is an ordinary character of the name and not a separator. The year is the year of the time of the officer. Where the records of another archive are brought into an archive, they shall be kept under ALICEV31NP/, the file name of the archive they come from without .v31np, and /, with a suffix of Table G-2 where that name is taken.

A record shall be UTF-8 text in lines that each end with a carriage return and a line feed, as presented in Table G-12. Text in a record is escaped as in Table G-4.

<table id="table-g-12">
<caption>Table G-12 Lines of a Record of Custody</caption>
<thead>
<tr><th>Line</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>The plate of the machine</td></tr>
<tr><td>2</td><td>The name and version of its system</td></tr>
<tr><td>3</td><td>The time of the officer, as set out in § ( G/( b.vi. ) )</td></tr>
<tr><td>4</td><td>The shift</td></tr>
<tr><td>5</td><td>The ticket</td></tr>
<tr><td>6</td><td>The action: add, copy, move, rename, delete, override or citation</td></tr>
<tr><td>7 and on</td><td>The keys the change concerns, one to a line; for a rename, a move or a copy, each old or source key followed by its new key. For a citation, the warning WARNING - CLOCK AHEAD or WARNING - CLOCK BEHIND, a space, the difference in seconds followed by s, and, where every car was off alike, a space and ALL, as in WARNING - CLOCK BEHIND 75s</td></tr>
</tbody>
</table>

#### 10. Readers on a Shared Road

A programme that reads an archive on a shared road shall not hold it open between reads, since a file server may refuse to replace a file while any machine holds it open. For every read, it shall open the archive, check that its length and last-write time are those of the listing it presents, read and close it. Where they differ, the archive has been replaced, and the programme shall read its listing anew. A view of an archive on a shared road should be refreshed as soon as another machine changes it.

## ( u ) Edge Cases and Limitations

Table G-8 presents the edge cases of editing and the behaviour required in each. Table G-9 presents the limitations that follow from the design of the shadow and of the road; they are accepted.

<table id="table-g-8">
<caption>Table G-8 Edge Cases</caption>
<thead>
<tr><th>Case</th><th>Behaviour</th></tr>
</thead>
<tbody>
<tr><th colspan="2">Names already taken</th></tr>
<tr><td>A file is added under a key that exists</td><td>The incoming file receives a suffix of Table G-2; nothing is replaced</td></tr>
<tr><td>A file is added where a folder of the same name exists, or a folder where a file exists</td><td>The incoming item receives a suffix and keeps all it holds; the existing item is kept</td></tr>
<tr><td>A folder is added where a folder of the same name exists</td><td>Merged; only the clashing files inside receive suffixes</td></tr>
<tr><td>An item is copied into the folder it comes from</td><td>The copy receives a suffix; a folder becomes a new folder with the suffix, holding a full copy</td></tr>
<tr><td>An item is moved onto its own place</td><td>Left as it is</td></tr>
<tr><td>A name is taken at the destination of an extraction</td><td>A file, or a folder landing on a file, receives a suffix; a folder landing on a folder is merged</td></tr>
<tr><td>Files are written by the copy machinery of the system, into or out of an archive</td><td>That machinery decides on names already taken</td></tr>
<tr><td>A folder is moved into itself or into a folder inside it</td><td>Refused</td></tr>
<tr><td>A rename to an empty name, to a name containing /, or to a name used in the folder</td><td>Refused</td></tr>
<tr><td>A rename onto a name used in the folder, given through the calls of a mounted file system</td><td>The item of that name is replaced</td></tr>
<tr><td>The name for a packed archive or for baíbíz e alùz is taken</td><td>A suffix of Table G-2</td></tr>
<tr><th colspan="2">Folders</th></tr>
<tr><td>The last entry of a folder is deleted or moved out</td><td>The folder remains, empty, until the repack and then vanishes</td></tr>
<tr><td>A new folder is created and left empty</td><td>Dropped at the repack</td></tr>
<tr><td>A folder moved in from the system still holds files at the repack</td><td>It is kept</td></tr>
<tr><th colspan="2">Linked and moved files</th></tr>
<tr><td>A linked file is changed before the repack</td><td>Packed as it is at the repack</td></tr>
<tr><td>A linked file is missing at the repack</td><td>Its entry is left out</td></tr>
<tr><td>A file moved in is deleted inside the archive before the repack</td><td>It stays on the system and is shown again</td></tr>
<tr><td>An item moved in is moved on to another archive before the repack</td><td>Its row H goes with it; the original stays hidden until that archive is repacked</td></tr>
<tr><td>An original was hidden already when it was moved in</td><td>No row H is written; it is never touched</td></tr>
<tr><td>An item is moved out onto the system</td><td>Removed through the shadow at once, as by a delete; gone from the archive after the repack</td></tr>
<tr><td>An item moved out is skipped, or the move is cancelled</td><td>It stays in the archive</td></tr>
<tr><td>A file moved in is copied inside the archive</td><td>The copy links to it as F; the original is deleted once, at the repack, with the row that moved it</td></tr>
<tr><th colspan="2">The journal</th></tr>
<tr><td>The archive is changed by another tool while a journal exists</td><td>The journal is stale and discarded; the archive on disk prevails</td></tr>
<tr><td>A journal exists under both names and the two are not the same</td><td>The one written most recently among those the programme can read is used</td></tr>
<tr><td>The journal is unreadable</td><td>Ignored; the next change starts a new journal</td></tr>
<tr><td>A journal is deleted at the repack, or discarded, while one exists under the other name</td><td>Only the journal used is deleted; the other no longer matches and is discarded as stale when next read</td></tr>
<tr><td>The programme ends before the repack</td><td>The journal remains; the archive is repacked when next opened and left</td></tr>
<tr><td>Changes are made without any view of the archive</td><td>The journal remains until the archive is opened and left</td></tr>
<tr><td>A nested archive with a section is read as a file</td><td>Built first into a temporary file</td></tr>
<tr><th colspan="2">Saving back</th></tr>
<tr><td>The entry of an opened copy is renamed, moved or deleted before the copy is saved</td><td>The save lands at the original key and recreates the entry</td></tr>
<tr><td>The entry of an opened copy is changed elsewhere before the copy is saved</td><td>The save lands beside it with a suffix of Table G-2; later saves follow to that key</td></tr>
<tr><th colspan="2">Plates and mounted devices</th></tr>
<tr><td>A journal holds a row of another machine, or a path that the system cannot interpret</td><td>Read-only: presented, the files of foreign rows not opened, no change, repack or discard</td></tr>
<tr><td>A journal with a foreign row is stale</td><td>Only the journal file is removed; nothing it names is touched</td></tr>
<tr><td>A journal is headed v31np shadow 1</td><td>Read; its rows count as rows of this machine; written as v31np shadow 2 at the next change</td></tr>
<tr><td>A device is pulled out with a journal pending and plugged into another machine</td><td>Read-only there; repacked once it is back on the first machine and the archive is opened and left</td></tr>
<tr><th colspan="2">Shared roads</th></tr>
<tr><td>A journal on a shared road holds a row F, M, T, R or H</td><td>Unreadable</td></tr>
<tr><td>A journal on a shared road is stale</td><td>Its sections are dropped; its officer is kept</td></tr>
<tr><td>The target of a change has vanished when the car builds</td><td>The change fails as not found</td></tr>
<tr><td>An entry of the change has changed since the view from which it was given</td><td>Refused; the view is refreshed</td></tr>
<tr><td>A reader still holds the archive open when the car lands</td><td>The replacement is tried again for a while; where it still fails, the change fails and nothing is lost</td></tr>
<tr><td>The holder misses its renewals while one other car waits</td><td>The user of the waiting car is asked; on Yes, override and a record of custody</td></tr>
<tr><td>The holder misses its renewals while several cars wait</td><td>No override; cars may tag the holder, and the lane is free once its lease runs out</td></tr>
<tr><td>The clock of a car is off by more than 60 seconds</td><td>Not waved on; logged as skew</td></tr>
<tr><td>The clocks of all cars are off alike, or the car is alone</td><td>Logged as condemn, a citation written; the order is kept</td></tr>
<tr><td>An empty folder is created on a shared road</td><td>A row D with a lease; struck out when the lease runs out or an item lands under it</td></tr>
</tbody>
</table>

<table id="table-g-9">
<caption>Table G-9 Limitations</caption>
<thead>
<tr><th>Limitation</th><th>Consequence</th></tr>
</thead>
<tbody>
<tr><td>A programme repacks when its own last view closes</td><td>Another programme with a view of the archive, such as an Open dialog, may repack before Explorer has left it; the result is correct, only early</td></tr>
<tr><td>A programme that does not follow this division reads the archive as stored</td><td>Until the repack, it does not see the pending changes</td></tr>
<tr><td>A journal may be cached by its length and last-write time</td><td>Two writes of the journal by different programmes within one tick of the file time may hide the second from a programme until the next change</td></tr>
<tr><td>A stale journal is discarded</td><td>Its pending changes are lost; temporary files on another system remain</td></tr>
<tr><td>Temporary content lives outside the archive until the repack</td><td>A cleanup of temporary files before the repack loses it</td></tr>
<tr><td>A journal that cannot be read is ignored</td><td>Originals hidden through its rows H stay hidden on the system</td></tr>
<tr><td>The length of a nested archive with a section is computed from the sizes in the journal</td><td>It may differ from the result once a linked file has changed</td></tr>
<tr><td>On a shared road, every change rewrites the archive over the network</td><td>A change to a large archive takes as long as writing the whole archive to the server</td></tr>
<tr><td>On a shared road, an empty folder created and a save by an editor are a car each</td><td>Each of them rewrites the archive</td></tr>
<tr><td>Readers hold an archive on a shared road open only for each read</td><td>A reader that holds it open for longer makes a change fail once the retries are spent; nothing is lost</td></tr>
<tr><td>An archive on a shared road is replaced by every change</td><td>A file that is open from an archive presented as a mounted file system fails once another machine has replaced the archive</td></tr>
<tr><td>A file being written into an archive on a shared road drives when it is complete</td><td>Until then, it is visible only on the machine that writes it</td></tr>
<tr><td>The format stores no checksum per entry</td><td>An entry changed on another machine to the same size since the view is not noticed by the check</td></tr>
<tr><td>A machine may stop between replacing the archive and writing the journal</td><td>The queue and the leased folders are lost; the archive is whole and holds the change</td></tr>
<tr><td>A temporary file left beside an archive by a machine that stopped is never removed by a programme</td><td>It stays beside the archive</td></tr>
<tr><td>A journal with a foreign row is read-only</td><td>An archive carried to another machine with changes pending cannot be changed there until it is back on the first machine</td></tr>
</tbody>
</table>

# ( D ) GLOSSARY

<table>
<tbody>
<tr><td>7-Zip plugin</td><td>The programme v31np.dll, which adds .v31np as a format to 7-Zip; see § ( Ú/( a.ii.2. ) ).</td></tr>
<tr><td>a, b, c</td><td>The fields of T2 that divide a key into its folder, its name and its extension.</td></tr>
<tr><td>Address</td><td>The absolute offset of the first data byte of a file, given in T1.</td></tr>
<tr><td>Archive</td><td>A .v31np file.</td></tr>
<tr><td>ASCII</td><td>American Standard Code for Information Interchange.</td></tr>
<tr><td>Car</td><td>One change of one machine on a shared road, from joining the queue to landing; see § ( G/( ú.iii.6. ) ).</td></tr>
<tr><td>Checksum</td><td>The CRC-32 at the end of a file, computed over every byte before it.</td></tr>
<tr><td>CRC-32</td><td>Cyclic redundancy check of 32 bits, with the reflected polynomial EDB88320, an initial value of FFFFFFFF and a final exclusive-or with FFFFFFFF.</td></tr>
<tr><td>Custody</td><td>The records, kept in the folder ALICEV31NP inside an archive, of the cars that have changed it; see § ( G/( ú.iii.9. ) ).</td></tr>
<tr><td>Data</td><td>The contents of the files of an archive, stored raw and contiguous after the header.</td></tr>
<tr><td>FAILING</td><td>The medallion of a programme that writes or keeps a file that does not conform to § ( B/( a ) ), or that refuses a file that conforms.</td></tr>
<tr><td>FNV-1a</td><td>Fowler–Noll–Vo hash, variant 1a, of 64 bits.</td></tr>
<tr><td>Footer</td><td>The closing π and the checksum; the last 16 bytes of a file.</td></tr>
<tr><td>Foreign row</td><td>A row of a journal that carries the plate of another machine, or a path that the system cannot interpret; see § ( G/( ú.i. ) ).</td></tr>
<tr><td>Header</td><td>The magic, π, T1, π, T2 and π, in that order.</td></tr>
<tr><td>HTTP</td><td>Hypertext Transfer Protocol.</td></tr>
<tr><td>Journal</td><td>The file FUTALUZTU UERSIIA UUTI, followed by the name of an archive, that records the pending changes of the archive; see § ( G/( b.vi. ) ).</td></tr>
<tr><td>Key</td><td>The path of a file as UTF-8 text, in which / separates folders.</td></tr>
<tr><td>Lane</td><td>The right to change an archive on a shared road, held by one car at a time; see § ( G/( ú.iii.5. ) ).</td></tr>
<tr><td>Lease</td><td>The time until which a car, a lane or a leased folder keeps its place unless it is renewed.</td></tr>
<tr><td>Level</td><td>An archive or an archive nested in it, named in a journal by the chain of keys that leads to it.</td></tr>
<tr><td>Little-endian</td><td>The byte order in which the least significant byte of a number is stored first.</td></tr>
<tr><td>Magic</td><td>The 15 bytes at the start of a file, the ASCII text V31NSPACKFORMAT.</td></tr>
<tr><td>Mark</td><td>π or φ.</td></tr>
<tr><td>Medallion</td><td>The rating, PASSING or FAILING, by which a programme is rated against the V31NP STANDARD.</td></tr>
<tr><td>Mounted device</td><td>A removable disk, such as a USB stick, plugged into a machine; see § ( G/( ú.ii. ) ).</td></tr>
<tr><td>Native system</td><td>A service of v31n that reads or writes .v31np files as part of its own operation; see § ( Ú/( a.i. ) ).</td></tr>
<tr><td>Naturalisation</td><td>The derivation, on extraction, of names that are safe for the system written to; see Table Ú-2.</td></tr>
<tr><td>Officer</td><td>The lines of a journal that keep the queue, the lane and the log of an archive; see § ( G/( ú.iii.3. ) ).</td></tr>
<tr><td>Override</td><td>The taking of the lane from a holder that has missed its renewals, on the agreement of the user; see § ( G/( ú.iii.8. ) ).</td></tr>
<tr><td>Papers</td><td>The plate of a machine, with the name and version of its system.</td></tr>
<tr><td>PASSING</td><td>The medallion of a programme of which every .v31np file it writes conforms to § ( B/( a ) ), and which reads every file that conforms.</td></tr>
<tr><td>Plate</td><td>The name by which a machine is known: its host name, a hyphen and a number of at most nine digits; see § ( G/( ú.i. ) ).</td></tr>
<tr><td>Post</td><td>The file NIEURFASMIT UUTI, followed by the name of an archive, beside an archive on a shared road, which carries its signal; see § ( G/( ú.iii.2. ) ).</td></tr>
<tr><td>PS/Í-1103</td><td>The Time standard of v31n, in which the times of journals and records are written; see § ( E ).</td></tr>
<tr><td>Queue</td><td>The cars waiting at the post of an archive.</td></tr>
<tr><td>Repack</td><td>The single rewrite of an archive with all its pending changes, after the user has left it; see § ( G/( b.viii. ) ).</td></tr>
<tr><td>Row</td><td>The description of one file in a table, beginning with 00 φ; in a journal, a line describing one entry of a level, see Table G-6.</td></tr>
<tr><td>Section</td><td>The part of a journal that lists the entries of one level, opened by a line @.</td></tr>
<tr><td>Shadow</td><td>The state of an archive together with its pending changes, held in the journal and the content it refers to; see § ( G/( b.v. ) ).</td></tr>
<tr><td>Shared road</td><td>An archive reached over a network, on a share or inside a folder that its machine shares; see § ( G/( ú.iii.1. ) ).</td></tr>
<tr><td>Shift</td><td>The number of a holding of the lane, which changes whenever the lane passes to another car or runs out.</td></tr>
<tr><td>Signal</td><td>The lock of one byte of the post; green to the one machine that holds it, red to all others.</td></tr>
<tr><td>Size</td><td>The length of the data of a file in bytes, given in T2.</td></tr>
<tr><td>Stale journal</td><td>A journal whose header no longer matches the length and last-write time of its archive.</td></tr>
<tr><td>T1</td><td>The first table of the header, giving the address of each file in data order.</td></tr>
<tr><td>T2</td><td>The second table of the header, giving the size of each file and the division of its key.</td></tr>
<tr><td>Tag</td><td>A warning by a waiting car that the holder seems stuck; it does not change the lane.</td></tr>
<tr><td>Ticket</td><td>The number given to a car when it joins the queue, in the order of arrival.</td></tr>
<tr><td>UTF-8</td><td>Unicode Transformation Format, 8-bit; the encoding of every key.</td></tr>
<tr><td>V31N order</td><td>The order in which names are sorted, first by the name without its extension and then by the whole name, a tie being broken by place, case, length and code points; see § ( G/( b.i. ) ). Syn.: Alphabetical order.</td></tr>
<tr><td>V31NP STANDARD</td><td>The format as defined in § ( B ).</td></tr>
<tr><td>View</td><td>An open presentation of a folder of an archive in a file manager; the repack follows the closing of the last view.</td></tr>
<tr><td>Zip bomb</td><td>An archive of which the extracted size greatly exceeds the stored size.</td></tr>
<tr><td>π</td><td>The mark of 12 bytes taken from the hexadecimal fraction of π, which divides the parts of a file; see Table B-2.</td></tr>
<tr><td>φ</td><td>The mark of 12 bytes taken from the hexadecimal fraction of φ, which divides the rows and fields of a table; see Table B-2.</td></tr>
</tbody>
</table>

# ( E ) REFERENCES

International Organization for Standardization (ISO) (2002) "ISO/IEC 13239: Information technology – Telecommunications and information exchange between systems – High-level data link control (HDLC) procedures", Geneva.

International Telecommunication Union (ITU-T) (2002) "Recommendation V.42: Error-correcting procedures for DCEs using asynchronous-to-synchronous conversion", Geneva.

Internet Engineering Task Force (IETF) (2003) "RFC 3629: UTF-8, a transformation format of ISO 10646".

Internet Engineering Task Force (IETF) (2022) "RFC 9110: HTTP Semantics", section 14, Range Requests.

Microsoft Corporation (n.d.) "Naming Files, Paths, and Namespaces", Win32 documentation.

Pavlov, I. (2026) "7-Zip 26.05".

Unicode Consortium (n.d.) "Unicode Standard Annex #15: Unicode Normalization Forms".

PS/Í-1103
