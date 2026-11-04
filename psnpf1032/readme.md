<table>
<thead>
<tr><th colspan="6">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td colspan="3">1. Report No.<br>PS-N/PF-1032 R.1</td><td colspan="3">2. Report Date<br>2026/11/04</td></tr>
<tr><td colspan="6">3. Title<br>V31NP NETWORK STANDARD</td></tr>
<tr><td colspan="6">4. Author(s)<br>P.K. — 619851078911877 , kokain@v31null.de</td></tr>
<tr><td colspan="3">a. Performing Organization Name(s) and Address(es)<br>Bureau for Onion & Over of þͤ Services <i>of</i> þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio — fileprtejlf5mpnrgs5isoknpg6zmidwvc2jtpotepkfeocx4zltm2ad.onion , psnpf1032@v31null.de</td><td colspan="3">b. Sponsoring/Monitoring Agency Name(s) and Address(es)<br>Mbrlí úír Entípatùíțù fùțbíz — v31null.de , mef@v31null.de<br>Services for þe populace General et Private Services for serving of images from Lokal disk — imgsw.v31null.de<br>Kokain organization — v31n.de , kokain@v31null.de<br>Services for þe populace General et Private Musik applikation of Per.‑Gen.‑portal nullpunkts as ver. 2 for General use — musik.v31null.de<br>þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio — file.v31null.de</td></tr>
<tr><td colspan="6">6. Abstract<br>This document sets the V31NP NETWORK STANDARD: the exchange of .v31np files between a server and a client in as few requests as possible, the request by which a client states the files it holds, the codes of the answers, the answer by which a client patches its copy, the order in which an archive is rebuilt, the index entry by which a client checks every file and keeps the header fields that the server sends with it, the list and the levels in which a client takes the files it needs, and the byte ranges of an archive and of each file in it.</td></tr>
<tr><td colspan="3" rowspan="2">7. Key Words<br>V31NP NETWORK STANDARD, .v31np, network, server, client, stub, sync check, V31NPIELALAIMIT, 267 Re-sync, 268 List, CRC-32, index, level, byte range, V31NBAIMITIF, SHA-256, Ed25519, header field, Tor</td><td colspan="3">8. Distribution Statement</td></tr>
<tr><td>a. Report<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>b. Abstract<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>ú. This page<br>A; Approved for public release: distribution is unlimited, no restrictions.</td></tr>
<tr><td colspan="3">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td><td colspan="3">10. №ⁱ OF PP.<br>20</td></tr>
</tbody>
</table>

# ( A ) INTRODUCTION

## ( a ) Table of Contents

- [( A ) INTRODUCTION](#aintroduction)
  - [( b ) List of Tables](#blist-of-tables)
  - [( ú ) Notice](#únotice)
  - [( u ) Foreword](#uforeword)
  - [( ù ) Tasked With](#ùtasked-with)
  - [( g ) Scope and Purpose](#gscope-and-purpose)
  - [( d ) Notation](#dnotation)
- [( B ) THE EXCHANGE](#bthe-exchange)
  - [( a ) Architecture](#aarchitecture)
  - [( b ) Procedures](#bprocedures)
  - [( ú ) Status Codes](#ústatus-codes)
  - [( u ) The Index Entry](#uthe-index-entry)
  - [( ù ) Integrity and Authenticity](#ùintegrity-and-authenticity)
  - [( g ) Levels](#glevels)
  - [( d ) Byte Ranges](#dbyte-ranges)
- [( Ú ) THE REQUEST](#úthe-request)
- [( U ) THE 267 ANSWER](#uthe-267-answer)
- [( Ù ) ORDER OF THE REBUILT ARCHIVE](#ùorder-of-the-rebuilt-archive)
- [( G ) ANSWERS PER QUERY](#ganswers-per-query)
  - [( a ) The Answer](#athe-answer)
  - [( b ) Sync of a Query](#bsync-of-a-query)
  - [( ú ) The Index](#úthe-index)
  - [( u ) The Drop-in — INFORMATIVE](#uthe-drop-ininformative)
- [( D ) FULFILMENT](#dfulfilment)
- [( E ) GLOSSARY](#eglossary)
- [( Ż ) REFERENCES](#żreferences)

## ( b ) List of Tables

- [Table A-1 Tasks](#table-a-1)
- [Table A-2 Requests of Non-Governmental Organisations](#table-a-2)
- [Table B-1 Procedures](#table-b-1)
- [Table B-2 Status Codes](#table-b-2)
- [Table B-3 Lines of the Index Entry](#table-b-3)
- [Table B-4 Levels](#table-b-4)
- [Table Ú-1 Layout of a Request](#table-ú-1)
- [Table U-1 Layout of a 267 Answer](#table-u-1)
- [Table U-2 Entries of a 267 File](#table-u-2)
- [Table Ù-1 Order of Folders and Files](#table-ù-1)
- [Table Ù-2 Order of Names in a Tie](#table-ù-2)
- [Table G-1 Lines of the Index](#table-g-1)
- [Table D-1 Receipt](#table-d-1)

## ( ú ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the V31NP NETWORK STANDARD. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document. v31n is not related to non official platforms, services, people, & ‘oc genus omne in any way or form. Textual, programmatic, and media content published across these services is made available under the terms of the v31n license. No statutory copyright, proprietary monopoly, or reservation of rights subsists in these works. Any person is granted full, unconditional, and irrevocable permission to access, copy, modify, adapt, distribute, or incorporate all materials for any purpose, without formality, fee, or prior authorization. The constitutive Latin charter and legal terms of this declaration are recorded in the system registry and resp. places.

## ( u ) Foreword

This document has been published to define the V31NP NETWORK STANDARD, the exchange of .v31np files between a server and a client. It applies to every person dealing with Prinzian systems and with native systems of v31n.

The exchange is defined in § ( B ). Its part ( a ) sets out the architecture, its part ( b ) the procedures between a client and a server, its part ( ú ) the status codes of the answers, its part ( u ) the index entry that every .v31np file of the exchange holds, its part ( ù ) the integrity and authenticity of the files, its part ( g ) the levels in which a client takes the files, and its part ( d ) the byte ranges. § ( Ú ) sets out the request by which a client states the files it holds and those it wants, and § ( U ) the answer by which a server sends only what differs. § ( Ù ) sets out the order in which an archive is rebuilt. § ( G ) sets out the answers per query: the answer, the sync of a query, the index and, informatively, the drop-in. § ( D ) sets out the fulfilment of the tasks and requests of § ( A/( ù ) ). § ( E ) gives the terms used in this document, and § ( Ż ) the documents to which it refers.

P.K.

## ( ù ) Tasked With

This document is tasked with the requests of the agencies in Table A-1, and takes up the requests of non-governmental organisations in Table A-2. Their fulfilment is set out in § ( D ).

<table id="table-a-1">
<caption>Table A-1 Tasks</caption>
<thead>
<tr><th>Requester</th><th>Task</th></tr>
</thead>
<tbody>
<tr><td>Services for þe populace General et Private Services for serving of images from Lokal disk — imgsw.v31null.de</td><td>Delivery of many images in the answer to a single request, also where that request is made to a page that the server generates, as with PHP</td></tr>
<tr><td>Mbrlí úír Entípatùíțù fùțbíz — v31null.de</td><td>Security of the exchange: the integrity and authenticity of every file exchanged, and the protections that the server sets for its pages, in force for every file answered from an archive</td></tr>
<tr><td>þe General populace Ministeria custodiae filaciorum ( Secura ) , aßervatio — fileprtejlf5mpnrgs5isoknpg6zmidwvc2jtpotepkfeocx4zltm2ad.onion</td><td>Availability of the exchange over Tor, through onion services, also for clients that keep no storage between sessions</td></tr>
<tr><td>Services for þe populace General et Private Musik applikation of Per.‑Gen.‑portal nullpunkts as ver. 2 for General use</td><td>Serving of static files, as audio, video, images, fonts, styles and scripts, from one archive kept by the client, also in parts by byte range</td></tr>
</tbody>
</table>

<table id="table-a-2">
<caption>Table A-2 Requests of Non-Governmental Organisations</caption>
<thead>
<tr><th>Requester</th><th>Request</th></tr>
</thead>
<tbody>
<tr><td>Kokain organization — v31n.de , kokain@v31null.de</td><td>Serving of the exchange by a Node.js server; references written as usual, whether relative, from the root of the site or to other sites, working without change; and a ready-to-use function of the Bureau, dropped into a site and used by a single call</td></tr>
</tbody>
</table>

## ( g ) Scope and Purpose

The V31NP NETWORK STANDARD, as defined by v31n, applies to every exchange of .v31np files between a server and a client over a network. A site is loaded in as few requests as possible.

The .v31np file is defined in PS-N/PF-1031, and every request and answer of this document that carries files is a .v31np file as defined there. This document defines the formats and the meaning of the codes. How a server answers, and which further checks it runs, is chosen by its administrator.

## ( d ) Notation

Bytes are written in hexadecimal, two digits to a byte, as in 00 and 01. π and φ denote the marks of PS-N/PF-1031, Table B-2. Numbers are stored in little-endian byte order, the least significant byte first.

A reference to a part of this document gives the sign § followed by the division and the part, as in § ( B/( a ) ), which denotes ( a ) of ( B ). Tables are numbered by division, as in Table B-1 and Table U-2.

The requirements of this document are stated with shall, and each binds unless this document states otherwise. Should denotes a recommendation, which does not bind, and may denotes a permission. A part marked INFORMATIVE gives guidance and binds no one.

# ( B ) THE EXCHANGE

## ( a ) Architecture

The page that a client opens first is a stub. The stub shall carry the name and the URL of its archive, and may carry the CRC-32 of that archive. It carries them as the meta elements v31np-archive, v31np-url and v31np-crc, or as the attributes data-archive, data-url and data-crc of its script element, and a client reads whichever is present. The stub performs the handshake before the content of the site is loaded. After the handshake, every file of the site comes out of one .v31np file.

Every key of the exchange begins with /, except the empty key and the key v31np index.txt.

The client keeps its copy of the archive locally. A browser keeps it in its Indexed Database (W3C, Indexed Database API), each file in parts of 4 194 304 bytes, so that a byte range reads only its parts; it computes the CRC-32 and the SHA-256 of a file while it stores it. A programme that is not a browser, as one that serves a local host to its own view, keeps its copy as one .v31np file. From the list of § ( B/( b ) ) it writes that file at its full size, with the header built from the rows of the index entry and the footer with the CRC-32 of the archive, writes every file that it receives at its address, and notes beside the file which entries are present. It reads a file at its address. Once every entry is present, the file is a complete .v31np file (PS-N/PF-1031).

Pages that a server generates, as with PHP, are served by plain HTTP, or as an answer per query of § ( G ), which holds the executed page together with the files to which it refers. References to images and other files are written as usual. A browser that runs JavaScript should answer them by a Service Worker (W3C, Service Workers) from the stored archive by key, with the header fields of § ( B/( u ) ); otherwise they are requested from the server.

Every .v31np file of the exchange is sent with the content type application/v31np. No .v31np file of the exchange, and no file answered by its key, shall be sent with the field Content-Encoding.

This document applies to the exchange over a network. Programmes that present .v31np files on a system, as set out in PS-N/PF-1031, § ( Ú ), are not bound by it.

## ( b ) Procedures

Table B-1 presents the procedures between a client and a server.

<table id="table-b-1">
<caption>Table B-1 Procedures</caption>
<thead>
<tr><th>Procedure</th><th>Request</th><th>Answer</th></tr>
</thead>
<tbody>
<tr><td>Handshake</td><td>The client opens the stub</td><td>The stub</td></tr>
<tr><td>Sync check</td><td>The client sends its request of § ( Ú ) by POST to the URL of the archive</td><td>One of the codes of Table B-2</td></tr>
<tr><td>Re-sync</td><td>Part of the sync check</td><td>A 267 answer of § ( U ), from which the client patches its copy</td></tr>
<tr><td>List</td><td>Part of the sync check</td><td>268 with the index entry of the archive</td></tr>
<tr><td>Wanted files</td><td>The client names the keys that it wants in its request of § ( Ú )</td><td>A 267 answer of § ( U ) that holds those files</td></tr>
<tr><td>Byte ranges</td><td>The client requests the URL of the archive, or of a file in it, by GET with the field Range</td><td>206 with the ranges, as set out in § ( B/( d ) )</td></tr>
<tr><td>Full file</td><td>The client requests the URL of the archive by GET</td><td>200 with the full .v31np file</td></tr>
</tbody>
</table>

The client sends its request on every visit, also where the CRC-32 in the stub equals that of its copy. The server computes the CRC-32 of its archive and of each file in it once, when the archive is built, and afterwards only reads them.

The procedures run as follows:

1. On a first visit, the client holds no files and sends an empty request. The server answers with the full file (200), points to a pre-built file (303), or gives the list (268).
2. On a return visit where the copy is in sync, the server answers 304, and nothing follows.
3. On a return visit where the copy is out of sync, the server answers with only what differs (267), with the full file (200), or with the new list (268), as the server decides.
4. A server may point every client to a pre-built file (303), which it writes once.
5. A client may request the full .v31np file. A server should give the list, and then the files by wanted keys (§ ( Ú )) and by byte ranges (§ ( B/( d ) )).

A client shall send its request for a copy only to the origin of the archive of that copy. A client may send an empty request in place of the files that it holds. Where a full file breaks off, the client may request the rest by the field Range with the field If-Range, or request the full file again.

The list is the index entry of the archive (§ ( B/( u ) )). A server that gives the list compares only the CRC-32 of the index entry in the request with that of its own: where they are equal it answers 304, and otherwise 268 with its index entry. From the rows of the index entry the client computes the address of every file: the header of the archive is built from the keys and the sizes in the order of § ( Ù ), and the files follow in that order. The CRC-32 of the archive computed from the rows equals the field ETag of the 268 answer.

## ( ú ) Status Codes

Table B-2 presents the status codes of the answers and their meaning in this document. The codes other than 267 and 268 are those of RFC 9110. 267 and 268 are sent over HTTP/1.1 with the reason phrases Re-sync and List, in ASCII; HTTP/2 and HTTP/3 carry the number alone.

<table id="table-b-2">
<caption>Table B-2 Status Codes</caption>
<thead>
<tr><th>Code</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>200</td><td>The full file follows</td></tr>
<tr><td>206</td><td>The byte ranges asked follow, as set out in § ( B/( d ) )</td></tr>
<tr><td>267 Re-sync</td><td>Only what differs, or the files wanted, follow, as a 267 answer of § ( U )</td></tr>
<tr><td>268 List</td><td>Only the index entry of the archive follows, as a .v31np file that holds that single entry</td></tr>
<tr><td>303</td><td>The pre-built file named in the field Location answers the request</td></tr>
<tr><td>304</td><td>The copy is in sync, and nothing follows</td></tr>
<tr><td>400</td><td>The field Range is malformed</td></tr>
<tr><td>404</td><td>The archive, or the file named by the query key=, does not exist</td></tr>
<tr><td>405</td><td>The method is neither GET, HEAD nor POST, or, for a file requested by its key, neither GET nor HEAD</td></tr>
<tr><td>413</td><td>The request is larger than the server accepts</td></tr>
<tr><td>414</td><td>The URL of a file requested by its key is longer than the server accepts</td></tr>
<tr><td>415</td><td>A request by POST to the URL of the archive does not carry the content type application/v31np</td></tr>
<tr><td>416</td><td>No range of the field Range can be satisfied</td></tr>
<tr><td>422</td><td>The request is broken, as by a wrong magic or a wrong checksum</td></tr>
</tbody>
</table>

The field ETag shall carry the CRC-32 in 8 hexadecimal digits in upper case within quotation marks, as a strong validator (RFC 9110). A server should send the field Cache-Control with no-store with 267, with 268, and with every answer to a sync check. On 401, 403, 404, a code of the class 5xx, or a code that it does not know, a client shall keep its copy and show from it. A server may answer 413 and 414 by limits of its own, and a client may refuse an answer larger than it can store.

A client in a browser sends its sync check with the cache mode no-store of the Fetch Standard. A 303 is followed by the browser itself; the client reads the final answer, whose URL names the pre-built file.

## ( u ) The Index Entry

Every archive of the exchange, and every answer per query of § ( G ), holds one entry under the key v31np index.txt, the index entry. The server builds it from its index of § ( G/( ú ) ) when it builds the archive or the answer, for the files of that archive or answer only. The index entry begins with the magic V31NBAIMITIF followed by 3 bytes 00, 15 bytes in all, and is otherwise written as the index: UTF-8 text, lines ending with a carriage return and a line feed, fields separated by a tab, text fields escaped as in PS-N/PF-1031, Table G-3, and numbers written in decimal. Table B-3 presents the lines of the index entry.

<table id="table-b-3">
<caption>Table B-3 Lines of the Index Entry</caption>
<thead>
<tr><th>Line</th><th>Fields</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>Magic</td><td>V31NBAIMITIF, 00 00 00</td><td>The first line</td></tr>
<tr><td>Row</td><td>I, key, size, CRC-32, SHA-256, level</td><td>One file of the archive or answer: its key, its size in bytes, its CRC-32 in 8 hexadecimal digits in upper case, where the server signs its SHA-256 in 64 hexadecimal digits in upper case, and its level of § ( B/( g ) ) where one is written. The executed page of an answer per query has its row under the empty key</td></tr>
<tr><td>Header field</td><td>H, key, name, value</td><td>One header field (RFC 9110) that the server sends with the file of the key when that file is requested by itself</td></tr>
<tr><td>Signature</td><td>S, signature</td><td>Where the server signs: the Ed25519 signature (RFC 8032) over every byte of the index entry before this line, in 128 hexadecimal digits in upper case. The last line</td></tr>
</tbody>
</table>

The rows are written in the order of § ( Ù ), each key once, and the header fields follow the rows, in the order of their keys. The index entry has no row of its own.

A programme recognises the index entry by its magic. It should not care about it, and may not show it.

Where a client answers a request for a file from its copy, it answers with the header fields of its key, beside the content type, the length and the byte ranges of the file.

## ( ù ) Integrity and Authenticity

The transport gives the integrity and authenticity of the exchange: HTTPS, or the onion address of an onion service of Tor. CRC-32 serves the sync check.

A server may add to its index entries the SHA-256 (FIPS 180-4) of every file and an Ed25519 signature (RFC 8032), as set out in Table B-3. Its public key reaches the client by other means than the archive and the stub, as the client chooses.

A client that holds the public key of the server checks its copy before it stores it, whether received whole or patched by a 267 answer: the signature of the index entry, and for every other file its size, its CRC-32 and its SHA-256 against its row, and that every file has a row. Where a check fails, or the index entry or its signature is missing, the client requests the full file once; where that fails too, it stores nothing of the answer.

A client shall drop every key that does not begin with /, and every key with a segment . or .., except the empty key and the key v31np index.txt, and store the rest of the answer.

## ( g ) Levels

Every file of an archive belongs to one of six levels, which a client takes in their order. The level of a file is written as the last field of its row in the index (§ ( G/( ú ) )) and in the index entry (§ ( B/( u ) )). A file whose row has no level belongs to level Ù. Table B-4 presents the levels.

<table id="table-b-4">
<caption>Table B-4 Levels</caption>
<thead>
<tr><th>Level</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>S</td><td>Essentials: the files that a page needs to show itself</td></tr>
<tr><td>A</td><td>Additional</td></tr>
<tr><td>B, Ú, U, Ù</td><td>The rest, in this order</td></tr>
</tbody>
</table>

A page that answers per query (§ ( G )) answers with the executed page and the files of level S, and with the files of further levels where the server chooses. The client then takes the files of level A, by one request of wanted keys. It takes the files of the levels B to Ù when it requests them, or when the server sends them.

## ( d ) Byte Ranges

A server answers byte ranges (RFC 9110) of an archive at the URL of the archive, and of a file in it at the URL of the archive with the query key=, followed by the key of the file. The key in the query shall be written in UTF-8 and percent-encoded (RFC 3986), and the server decodes it once. A file requested by its key is answered as a file of its own: with its content type, the header fields of its key (§ ( B/( u ) )), its CRC-32 in the field ETag, and its own byte ranges. Audio and video are played from it.

1. A request of one range is answered 206 with the field Content-Range.
2. A request of several ranges is answered by one 206 answer of the content type multipart/byteranges, whose boundary is v31np- followed by the CRC-32 of the archive or of the file; each part carries its own fields Content-Type and Content-Range.
3. A range that cannot be satisfied is dropped; where none is left, the answer is 416.
4. A malformed field Range is answered 400.
5. Where the field If-Range does not match the field ETag, the whole archive or file is answered (200).

A client that answers byte ranges from its copy answers them in the same way.

# ( Ú ) THE REQUEST

The request is a .v31np file that holds a single entry, whose key is empty. That entry is the request file, which begins with the magic V31NPIELALAIMIT, 15 bytes of ASCII. Table Ú-1 presents the layout of a request.

<table id="table-ú-1">
<caption>Table Ú-1 Layout of a Request</caption>
<thead>
<tr><th>Part</th><th>Length (bytes)</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><th colspan="3">Header of the .v31np file</th></tr>
<tr><td>Magic</td><td>15</td><td>V31NSPACKFORMAT (ASCII)</td></tr>
<tr><td>π, T1, π, T2, π</td><td>variable</td><td>One row, for the empty key</td></tr>
<tr><th colspan="3">Request file</th></tr>
<tr><td>Magic</td><td>15</td><td>V31NPIELALAIMIT (ASCII)</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>T1</td><td>variable</td><td>One row per file of the copy: key, address</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>T2</td><td>variable</td><td>One row per file of the copy: key, size 8, a, b, c</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>CRC-32 of the files</td><td>8 per file</td><td>One CRC-32 per file, in the order of T1, in hexadecimal digits (ASCII)</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>4</td><td>Checksum of the request file</td></tr>
<tr><th colspan="3">Footer of the .v31np file</th></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>4</td><td>Checksum</td></tr>
</tbody>
</table>

The header of the request file is binary and is built as the header of a .v31np file (PS-N/PF-1031, § ( B/( a ) )). It lists the keys of the files of the copy, and the keys that the client wants, in the order of § ( Ù ). Each entry has the size 8, and the addresses chain through the CRC-32 text of the request file.

Each CRC-32 is written as the number, in 8 hexadecimal digits in upper case, the most significant digit first, without separators. The checksum of the request file is computed as the checksum of a .v31np file (PS-N/PF-1031, § ( B/( a.v. ) )), over every byte of the request file before it.

A key that the client wants and does not hold is written with the CRC-32 text 00000000.

An empty request lists no files: its T1, its T2 and its CRC-32 text are empty.

# ( U ) THE 267 ANSWER

The 267 answer is a .v31np file that holds a single entry, whose key is empty. That entry is the 267 file, which begins with the magic V31N267 followed by 8 bytes 00, 15 bytes in all; the bytes 00 are reserved. Table U-1 presents the layout of a 267 answer.

<table id="table-u-1">
<caption>Table U-1 Layout of a 267 Answer</caption>
<thead>
<tr><th>Part</th><th>Length (bytes)</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><th colspan="3">Header of the .v31np file</th></tr>
<tr><td>Magic</td><td>15</td><td>V31NSPACKFORMAT (ASCII)</td></tr>
<tr><td>π, T1, π, T2, π</td><td>variable</td><td>One row, for the empty key</td></tr>
<tr><th colspan="3">267 file</th></tr>
<tr><td>Magic</td><td>15</td><td>V31N267 (ASCII), 00 00 00 00 00 00 00 00</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>T1</td><td>variable</td><td>One row per changed key: key, address</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>T2</td><td>variable</td><td>One row per changed key: key, size, a, b, c</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>Entries</td><td>variable</td><td>One entry per changed key, in the order of T1, as in Table U-2</td></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>4</td><td>Checksum of the 267 file</td></tr>
<tr><th colspan="3">Footer of the .v31np file</th></tr>
<tr><td>π</td><td>12</td><td>Mark</td></tr>
<tr><td>CRC32</td><td>4</td><td>Checksum</td></tr>
</tbody>
</table>

Every key of the 267 file is in its T1 and in its T2. A key of the copy that is not in the 267 file stays as it is. The checksum of the 267 file is computed as that of the request file (§ ( Ú )).

Every entry ends in one flag byte. Table U-2 presents the entries and what the client does with each.

<table id="table-u-2">
<caption>Table U-2 Entries of a 267 File</caption>
<thead>
<tr><th>Entry</th><th>Data</th><th>Size</th><th>The client</th></tr>
</thead>
<tbody>
<tr><td>Deleted</td><td>01</td><td>1</td><td>Deletes the key</td></tr>
<tr><td>Changed or new</td><td>The bytes of the file, then 00</td><td>Size of the file + 1</td><td>Replaces the file with the bytes before the flag</td></tr>
<tr><td>Now empty</td><td>00</td><td>1</td><td>Replaces the file with an empty file</td></tr>
</tbody>
</table>

A server that gives the list (§ ( B/( b ) )) answers a request of wanted keys with one 267 answer that holds every wanted file as changed or new. A wanted key that is not in the archive comes as deleted. Where the index entry of the request is not the current one, the current index entry comes in the same answer.

The server sends the CRC-32 of its whole archive in the field ETag (RFC 9110). After patching, the client may compare the CRC-32 of its rebuilt archive with it, and on a mismatch it requests the full file.

# ( Ù ) ORDER OF THE REBUILT ARCHIVE

The client rebuilds its archive in the order in which the server builds its own, as follows:

1. Keys are split at / and compared part by part, and at each level folders come before files.
2. Within a level, names are compared in alphabetical order, and a tie is broken, as set out in PS/G-1104, § ( B/( b ) ).

The lower case of the folding of PS/G-1104 is the simple lowercase mapping of Unicode 18.0, field 13 of UnicodeData.txt.

Table Ù-1 presents keys in this order, and Table Ù-2 names that are equal after folding.

<table id="table-ù-1">
<caption>Table Ù-1 Order of Folders and Files</caption>
<thead>
<tr><th>Place</th><th>Key</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>/m/img/a.png</td></tr>
<tr><td>2</td><td>/m/img/b.png</td></tr>
<tr><td>3</td><td>/m/a.png</td></tr>
<tr><td>4</td><td>/m/b.png</td></tr>
<tr><td>5</td><td>/z.png</td></tr>
</tbody>
</table>

<table id="table-ù-2">
<caption>Table Ù-2 Order of Names in a Tie</caption>
<thead>
<tr><th>Place</th><th>Name</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>A.png</td></tr>
<tr><td>2</td><td>a.png</td></tr>
<tr><td>3</td><td>AA.png</td></tr>
<tr><td>4</td><td>AAa.png</td></tr>
<tr><td>5</td><td>aaaa.png</td></tr>
<tr><td>6</td><td>ú.png</td></tr>
<tr><td>7</td><td>V.png</td></tr>
<tr><td>8</td><td>v.png</td></tr>
</tbody>
</table>

# ( G ) ANSWERS PER QUERY

## ( a ) The Answer

A page that a server generates may answer with a .v31np file that holds the page as executed together with the files to which it refers. The entry with the empty key holds the executed page, and every other entry holds a file to which the page refers, under the reference of the page, resolved against the URL of the page and written from the root of the host. The server builds the answer when it is requested, from the files as they are stored, through the index of § ( G/( ú ) ), with its entries in the order of § ( Ù ). The server sends the CRC-32 of the whole answer in the field ETag (RFC 9110). The answer holds the files of level S, and of further levels where the server chooses (§ ( B/( g ) )).

The page that a browser opens is a shell that holds the script. The shell carries the name and the URL of the archive as the meta elements v31np-archive and v31np-url, as a stub does. The script requests the answer and shows the executed page, every reference of the page and of its scripts taken from the stored files, so that every request of the page goes to the URL of the archive. A file that the client does not hold is taken by its key (§ ( B/( d ) )). What the shell shows before the answer has arrived is the choice of the administrator.

The page answers itself, at its own URL. A request by POST with the content type application/v31np, or by GET or HEAD with application/v31np in the field Accept, is answered with the answer per query; every other request is answered with the shell. The requests of the client carry these fields. Other requests of the page, as forms, reach it unchanged. Such a page should send the field Vary with Accept.

## ( b ) Sync of a Query

A client keeps the answer of each query as a copy of its own, named by the URL of the query. It sends the request of § ( Ú ) for that copy by POST to the URL of the query, an empty request where it holds no copy, and the server answers with the codes of Table B-2: 200 with the full answer, 304 where the copy is in sync, and 267 with what differs, as set out in § ( U ). A key of the copy that the answer no longer holds is deleted. A GET of the URL of the query with application/v31np in the field Accept answers with the full answer.

## ( ú ) The Index

The server finds each file of an answer per query, and of an archive that it builds, through its index, the file v31np index.txt, without searching the folders in which the files are stored. The index of a site lies in the root folder of the site. A file that is not in the index is not served. The index is UTF-8 text. Its lines end with a carriage return and a line feed, a reader also accepts a line feed alone, and empty lines are ignored. The fields of a line are separated by a tab, text fields are escaped as in PS-N/PF-1031, Table G-3, and numbers are written in decimal. Table G-1 presents the lines of the index.

<table id="table-g-1">
<caption>Table G-1 Lines of the Index</caption>
<thead>
<tr><th>Line</th><th>Fields</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>Magic</td><td>V31NBAIMITIF, 00 00 00</td><td>The first line that is not empty, as in the index entry of § ( B/( u ) )</td></tr>
<tr><td>Root</td><td>root</td><td>The next line that is not empty: the folder of the files, relative to the folder of the index</td></tr>
<tr><td>Row</td><td>I, key, size, CRC-32, path, time, SHA-256, level</td><td>One file: its key, its size in bytes, its CRC-32 in 8 hexadecimal digits in upper case, its path relative to the root, the time of its last write as in the journal of PS-N/PF-1031, § ( G/( b.vi. ) ), where the server signs its SHA-256 in 64 hexadecimal digits in upper case, and its level of § ( B/( g ) ) where one is written</td></tr>
<tr><td>Header field</td><td>H, key, name, value</td><td>One header field (RFC 9110) that the server sends with the file of the key when that file is requested by itself</td></tr>
</tbody>
</table>

The rows are written in the order of § ( Ù ), each key once, and the header fields follow the rows. No line divides the rows from the header fields; the first field of a line names its kind. Where the size of a file, or the time of its last write counted in seconds, differs from its row, the server computes the CRC-32 of the file again, and its SHA-256 where it signs, and writes the row anew. The index is written whole, to a temporary file in the same folder that then replaces it.

## ( u ) The Drop-in — INFORMATIVE

The Bureau gives the exchange as one folder, v31np, which a site places in the root folder of its host, and which serves a PHP server and a Node.js server alike. The archive covers the whole host, so that references written relative to the page, from the root of the site, or to other sites work without change. The site names its archive. One command, of PHP or of Node.js, builds the archive, the index of § ( G/( ú ) ) and the script of the folder; both build the same bytes. The command takes the files of the types of pages, styles, scripts, images, fonts, audio and video, and further types that the administrator names, and of those only the files that the rules of the server do not deny, and none in a folder named node_modules.

A site loads its pages in one of two forms:

1. A page carries in its head a script element whose source is /v31np/v31np.js and whose attributes data-archive and data-url name the archive and its URL; the page is otherwise unchanged. The script stops the reading of the page by the browser, takes the list (§ ( B/( b ) )), takes by one request of wanted keys (§ ( Ú )) every file named by the markup of the page that the client does not hold, then the files named by its styles, and shows the page with every such reference taken from the stored files. Audio and video are taken by their keys (§ ( B/( d ) )). Links, forms and frames keep their URLs. A meta element with the attribute http-equiv Content-Security-Policy (W3C, Content Security Policy Level 3) and the content default-src * data: blob: 'unsafe-inline' 'unsafe-eval', placed before the script, keeps the browser from fetching the files named by the markup before the script runs. Where the script comes from another origin, as from the Bureau, the URL of data-url is read against the URL of the page. Where the server signs, the attribute data-key of the script element gives its public key in 64 hexadecimal digits. The attribute data-query names, separated by spaces, the URLs of the site whose answers come per query (§ ( G/( a ) )). The client asks them with the request of § ( Ú ), gives the executed answer to the page, and keeps every other file of the answer under its key, which is the URL, query included, by which the page asks for that file, written from the root of the host.
2. A page answers per query (§ ( G/( a ) )) by a single call at its top: v31np with the name of the archive in PHP, and v31np.page with the request, the answer, the name of the archive and the page in Node.js.

A Node.js server serves the exchange by the module of the folder, called with the request and the answer as a handler, or used as middleware of Express. It answers the URLs of the archive, of its pre-built file and of the script, and refuses every other URL of the folder.

A URL named in data-query answers per query by the call v31np_query at its top, with the index of its files and a function that gives the keys of the files from its output. Every other request receives its usual answer.

An existing site takes the drop-in as follows:

1. The script element goes into the head of the page as the browser receives it. Where one file of the site includes another to build the page, it goes into the file that writes the head.
2. Files outside the root folder of the host, and files that a script of the site serves under a query, come with the answer of the URL that lists them, each under the URL by which the page asks for it. The index of such files lies beside them, and its rows keep the key apart from the path of the file.
3. Settings of the server that apply to every script of the site, as a file prepended to each, apply to the scripts of the folder v31np too. A setting that names a file by a relative path names it there by its full path.
4. Links, forms, the requests of the page and its scripts work as before; only the transfer of the files changes. A list of the site comes as one answer, and the files it names are then taken from the stored files.

# ( D ) FULFILMENT

Table D-1 presents the state of the tasks of Table A-1 and of the requests of Table A-2, and the parts of this document that fulfil them.

<table id="table-d-1">
<caption>Table D-1 Receipt</caption>
<thead>
<tr><th>Requester</th><th>Task</th><th>State</th><th>See</th></tr>
</thead>
<tbody>
<tr><td>imgsw</td><td>Many images, one request</td><td>Done</td><td>§§ ( B/( a ) ) & ( G )</td></tr>
<tr><td>MEF</td><td>Security of the exchange</td><td>Done</td><td>§§ ( B/( u ) )〜( B/( ù ) ) & ( G/( ú ) )</td></tr>
<tr><td>Secura</td><td>Availability over Tor</td><td>Done</td><td>Format itself</td></tr>
<tr><td>m2</td><td>Static files, byte ranges</td><td>Done</td><td>§§ ( B/( a ) )〜( B/( u ) ) , ( B/( g ) )〜( B/( d ) ) , ( Ú )〜( U ) , ( G/( a ) ) , & ( G/( ú ) )</td></tr>
<tr><td>Kokain</td><td>Node.js, references, drop-in</td><td>Done</td><td>§§ ( G/( a ) ) & ( G/( u ) )</td></tr>
</tbody>
</table>

# ( E ) GLOSSARY

<table>
<tbody>
<tr><td>267 answer</td><td>The answer by which a server sends only what differs; see § ( U ).</td></tr>
<tr><td>Answer per query</td><td>A .v31np file that holds a page as executed together with the files to which it refers; see § ( G ).</td></tr>
<tr><td>Byte range</td><td>A part of an archive or of a file in it, named in the field Range (RFC 9110); see § ( B/( d ) ).</td></tr>
<tr><td>Copy</td><td>The archive that a client keeps locally.</td></tr>
<tr><td>Executed page</td><td>The output of a page that a server generates, as sent to a client.</td></tr>
<tr><td>Handshake</td><td>The opening of the stub, before the content of the site is loaded.</td></tr>
<tr><td>Header field</td><td>A field of the header of an HTTP answer (RFC 9110), as CSP, HSTS, X-Frame-Options, X-Content-Type-Options and Referrer-Policy.</td></tr>
<tr><td>Index</td><td>The file through which a server finds the files of its answers per query and of the archives it builds; see § ( G/( ú ) ).</td></tr>
<tr><td>Index entry</td><td>The entry v31np index.txt of an archive or an answer per query, which lists its files, their header fields and the signature; see § ( B/( u ) ).</td></tr>
<tr><td>Level</td><td>One of the six levels S, A, B, Ú, U and Ù, in which a client takes the files of an archive; see § ( B/( g ) ).</td></tr>
<tr><td>List</td><td>The index entry of an archive, sent alone with 268; see § ( B/( b ) ).</td></tr>
<tr><td>Pre-built file</td><td>A file that a server writes once, and to which it points clients with 303.</td></tr>
<tr><td>Request</td><td>The .v31np file by which a client states the files it holds and those it wants; see § ( Ú ).</td></tr>
<tr><td>Shell</td><td>The page that a browser opens for a page that answers per query, holding the script that requests the answer; see § ( G/( a ) ).</td></tr>
<tr><td>Stub</td><td>The page that a client opens first, carrying the name and the URL of the archive.</td></tr>
<tr><td>Sync check</td><td>The request of a client and the answer of the server by which the copy is compared with the archive of the server; see § ( B/( b ) ).</td></tr>
<tr><td>Wanted key</td><td>A key that a client names in its request with the CRC-32 text 00000000; see § ( Ú ).</td></tr>
</tbody>
</table>

# ( Ż ) REFERENCES

Internet Engineering Task Force (IETF) (2005) "RFC 3986: Uniform Resource Identifier (URI): Generic Syntax".

Internet Engineering Task Force (IETF) (2022) "RFC 9110: HTTP Semantics".

Internet Research Task Force (IRTF) (2017) "RFC 8032: Edwards-Curve Digital Signature Algorithm (EdDSA)".

National Institute of Standards and Technology (NIST) (2015) "FIPS 180-4: Secure Hash Standard (SHS)".

Unicode Consortium (2026) "The Unicode Standard, Version 18.0".

W3C (n.d.) "Content Security Policy Level 3".

W3C (n.d.) "Indexed Database API".

W3C (n.d.) "Service Workers".

WHATWG (n.d.) "Fetch Standard".

PS-N/PF-1031

PS/G-1104
