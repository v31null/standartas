Hírù gùkríbngaín uq standartas Íika , i.e./ .v31np & .v31n formatas , úír kípamíț čí Mílksùns‑maíțúčmíțas.

![Íik](nżúù/eayk.png)

Þis File‑format is þͤ Best‑File‑format in þͤ World File‑format‑‘istorie , All‑so ; I am It’s kreator.


***

# ( A ) INTRODUCTION

## ( a ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the V31NP STANDARD. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document.

## ( b ) Foreword

This document has been published to define the .v31np file format and the V31NP STANDARD, and to describe the programmes that read and write .v31np files. It is intended for developers who implement readers or writers of the format, and for those who maintain the services of v31n that exchange .v31np files with one another.

The format is defined in ( B ), which sets out the layout of a file, the rows and fields of its tables, the constraints a file shall satisfy, the checksum, and the properties the format supports and does not support. ( Ú ) describes the programmes that support the format, namely the native systems of v31n, the Windows Explorer extension and the 7-Zip plugin. ( U ) defines the two medallions, PASSING and FAILING, by which each programme in ( Ú ) is rated against the standard.

P.K.

## ( ú ) Scope and Purpose

A .v31np file, as defined by v31n, is a single file that holds any number of files together with their paths. The data of each file is stored raw, without compression or encryption. A header at the start of the file holds two tables, the first giving the address of each file in data order and the second giving the size of each file and the parts of its path. A checksum at the end of the file covers the whole file. This definition applies to every .v31np file regardless of the programme that writes it or the system that reads it.

The format was created 1) to share a large number of small files faster, 2) to be used in server ⇆ client relationships, 3) to store data safely, 4) to block zip bomb attacks and similar, 5) to secure the interests of v31n, 6) related to 5), to reduce supply-chain risks, 7) to ease the use of file storage, and 8) to create a standard that all services conforming to the V31NP STANDARD can use interchangeably.

Several of these purposes follow directly from the structure of the format. A .v31np file carries no per-file headers and no compression, and the addresses of all files are known from the header before any data is read, which allows many small files to be shared as one. Since the header precedes the data and the data is contiguous, a file can be streamed from a server, resumed by byte range and read at any address; the native systems in ( Ú/( a ) ) rely on this. Names are stored exactly as given and are never altered by a writer, and a single checksum covers header, data and closing mark. Because data is not compressed, the extracted size of each file equals its stored size; sizes and addresses shall chain exactly to the length of the file, and empty folders cannot be stored, so that an archive cannot expand beyond what its header states. The format and its readers and writers are defined and maintained by v31n, which limits the dependence of its services on outside software.

The naming of extracted files on a given operating system is not part of the format. Readers derive such names on output, and the rules applied on Windows are given in ( Ú/( b.i.4. ) ).

# ( B ) THE FORMAT

## ( a ) Definition

### ( i ) Layout

A .v31np file consists of a header, the data of the files and a footer, in that order. The header begins with a magic of 15 bytes, the ASCII text "V31NSPACKFORMAT", followed by the mark π. Two tables follow, T1 and T2, each closed by π; the π that closes T2 ends the header. The data follows immediately after the header, and the footer consists of a fourth π and a checksum of 4 bytes. All offsets are absolute and are counted from the first byte of the file, and all numbers are unsigned and stored in little-endian byte order. Table B-1 presents the layout of a file.

Table B-1 File Layout

| Part | Length (bytes) | Contents |
|---|---|---|
| Magic | 15 | V31NSPACKFORMAT (ASCII) |
| π | 12 | Mark |
| T1 | variable | One row per file: 00 φ, key, 01 φ, address |
| π | 12 | Mark |
| T2 | variable | One row per file: 00 φ, key, 01 φ, size, 01 φ, a, 01 φ, b, 01 φ, c |
| π | 12 | Mark, end of header |
| Data | variable | Contents of the files, raw, concatenated in the order of T1 |
| π | 12 | Mark |
| CRC32 | 4 | Checksum |

The two marks are taken from the hexadecimal fractions of the constants π and φ. π divides the parts of the file, while φ divides the rows and fields within a table. Table B-2 gives the bytes of each mark.

Table B-2 Marks

| Mark | Bytes (hexadecimal) | Origin |
|---|---|---|
| π | 24 3F 6A 88 85 A3 08 D3 13 19 8A 2E | Hexadecimal fraction of π, first 12 bytes |
| φ | 9E 37 79 B9 7F 4A 7C 15 F3 9C C0 60 | Hexadecimal fraction of φ, first 12 bytes |

### ( ii ) Rows

Each table is a sequence of rows, one row per file. A row begins with the byte 00 followed by φ, and each further field of the row is preceded by the byte 01 followed by φ. A table begins directly with its first row, and no byte may stand between the preceding π and the first 00 φ. A table that contains no rows denotes an archive that holds no files.

A row of T1 shall have exactly two fields, and a row of T2 exactly five. Since each file is described once in each table, T1 and T2 shall hold the same number of rows.

### ( iii ) Fields

Table B-3 presents the fields of both tables. The key is the path of the file as UTF-8 text, in which the character / separates folders. The address is the offset at which the data of the file begins, and the size is the length of that data. The fields a, b and c divide the key into its folder, its name and its extension.

Table B-3 Fields

| Field | Table | Contents |
|---|---|---|
| key | T1, T2 | Path of the file, UTF-8, stored as given; / separates folders |
| address | T1 | Absolute offset of the first data byte of the file |
| size | T2 | Length of the data of the file in bytes |
| a | T2 | Folder: the key up to and including its last /; empty if the key contains no / |
| b | T2 | Name: the remainder of the key without c |
| c | T2 | Extension: the remainder of the key from its last full stop to the end, the full stop included; empty if the remainder contains no full stop |

Address and size are numbers of 1 to 7 bytes without fixed width, and each shall be less than 2⁵². Writers should write each number in its shortest form, so that the number zero is written as the single byte 00.

The division of a key into a, b and c is illustrated in Table B-4. A key without a folder leaves a empty, a name that begins with a full stop leaves b empty, and a name without a full stop leaves c empty. Only the last full stop of the name separates the extension.

Table B-4 Examples of the Division of a Key

| Key | a | b | c |
|---|---|---|---|
| /css/fonts/Frank.ttf | /css/fonts/ | Frank | .ttf |
| photo.tar.gz | (empty) | photo.tar | .gz |
| src/.gitignore | src/ | (empty) | .gitignore |
| bin/run | bin/ | run | (empty) |

### ( iv ) Constraints

A .v31np file shall satisfy the constraints of this section; the checksum is treated separately in ( B/( a.v. ) ).

Each key shall be valid UTF-8. Each key shall occur in T2 only once, and each key of T1 shall occur in T2 exactly once. The fields a, b and c shall divide the key as defined in ( B/( a.iii. ) ), so that their concatenation equals the key.

The order of T1 is the order of the data, while the order of T2 is free. The first address shall equal the length of the header, counted from the first byte of the file through the π that closes T2, and each following address shall equal the previous address plus the size of the previous file. The data of the last file shall end at the offset of the closing π, so that no byte between the header and the footer is left unassigned. The length of the whole file shall be less than 2⁵² bytes.

It follows from these constraints that a .v31np file holding no files is 67 bytes long, consisting of the magic, four π and the checksum.

### ( v ) Checksum

The checksum is a CRC-32 with the reflected polynomial EDB88320 (hexadecimal), an initial value of FFFFFFFF and a final exclusive-or with FFFFFFFF. It is computed over every byte from the first byte of the file through the closing π, and is stored in little-endian byte order.

The checksum covers the whole file and not single files. A reader shall not refuse a file because its checksum does not match; a mismatch is reported only when the file is tested. Since no checksum of a single file exists, a mismatch cannot be attributed to one file, and the 7-Zip plugin therefore reports it for every file of the archive (see ( Ú/( b.ii. ) )).

### ( vi ) Header Length

The addresses in T1 depend on the length of the header, and the length of the header in turn depends on the addresses, since numbers have no fixed width. A writer therefore builds the header with a start address of 0, measures its length, and builds it again with that length as the start address. The process is repeated until the length no longer changes.

## ( b ) Properties

### ( i ) Supported

Neither π nor φ can occur inside valid UTF-8, since each contains an ASCII byte followed by a continuation byte. Neither can occur inside a number of at most 7 bytes, and neither contains the byte 00 or 01, so that no mark can be formed across the boundary of a field. Keys therefore require no escaping and no length prefix.

Every valid UTF-8 string is a legal key, including .., C:, \, the characters * and ?, control characters and the empty string. Writers do not alter keys, and readers derive names that are safe for their system on output, as described for Windows in ( Ú/( b.i.4. ) ).

The data is contiguous, and every address and size is given in the header, so that whether the layout of a file is complete can be determined from the header alone. Since the data is stored raw and every address is absolute, a file is read at its address for its size, without decoding. A single checksum covers the header, the data and the closing mark.

### ( ii ) Unsupported

Developers familiar with tar, zip and rar should note that several features of those formats have no equivalent in .v31np. Table B-5 lists these features together with the treatment each receives in .v31np.

Table B-5 Features of tar, zip and rar without Equivalent in .v31np

| Feature | Treatment in .v31np |
|---|---|
| Folder entries | Folders exist only as prefixes of keys; empty folders cannot be stored, and writers shall refuse them |
| Dates, permissions, owners, attributes | Not stored; extracted files carry the date of extraction |
| Compression, encryption, method fields | Not provided; data is stored raw |
| Deduplication, solid blocks, run-length encoding | Not provided |
| Symbolic links, hard links | Not provided; every entry is a file |
| Duplicate names | A later entry does not override an earlier one; a duplicate key makes the file invalid |
| Padding, block alignment | Not permitted; every byte between header and footer belongs to a file |
| Appending, in-place update | Not possible; every address depends on the length of the header and the checksum covers the whole file, so any change rewrites the whole file |
| Directory at the end of the file | Not used; the tables are at the start |
| Checksums of single files | Not provided; one checksum covers the whole file |
| Code pages other than UTF-8 | Not permitted |
| Cleaning of names on write (\ to /, removal of ../, removal of drive letters) | Not permitted; the key is stored as given |
| Volumes, comments, extra fields, 64-bit extensions | Not provided; numbers are of variable width up to 2⁵² |

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

Table Ú-1 Commands

| Command | Selection | Result |
|---|---|---|
| Anfas alùz e íülíntù, followed by the name of the archive | One .v31np file | Extracts into a folder of that name beside the archive |
| Anfas alùz híre | One .v31np file | Extracts into the folder of the archive |
| Anfas alùz țùre | One .v31np file | Extracts into a picked folder |
| Anfas alùz țùre e íülíntù, followed by the name of the archive | One .v31np file | Extracts into a folder of that name inside a picked folder |
| Anfas teí̈nt híre | Several .v31np files | Extracts each into a folder of its name inside a new folder baíbíz e alùz in the folder of the archives |
| Anfas teí̈nt țùre | Several .v31np files | Extracts each into a folder of its name inside a new folder baíbíz e alùz in a picked folder |
| Baíbíz | Files and folders | Packs the selection into a new .v31np file; also offered in the Send to menu |
| New | None | Creates an empty .v31np file |

Where a folder baíbíz e alùz, or an archive of the name chosen by Baíbíz, already exists, the new one receives a letter suffix in the manner described in ( Ú/( b.i.4. ) ).

#### 3. Editing

Files are added to an archive by dropping or pasting them into its folder. A file whose key already exists replaces the stored file without prompt. Deletion removes files without confirmation. A rename is refused if the new name is empty or contains /.

Since a .v31np file cannot be updated in place (see ( B/( b.ii. ) )), each change writes a new temporary file, which then replaces the archive. A change inside a nested archive rewrites the inner archive first and then each archive that contains it.

#### 4. Extraction

Keys may hold characters and names that Windows does not permit in file names. On extraction, each part of a key is therefore naturalised according to Table Ú-2, and names that then collide are made unique as described below. The key in the archive is not changed. Extracted files carry the date of extraction and normal attributes, and the checksum is not verified on extraction.

Table Ú-2 Naturalisation of Names on Windows

| Stored | Written |
|---|---|
| Leading / | Removed |
| Drive letter and colon at the start of the key, e.g. C: | Folder of the letter, e.g. C |
| Empty part, e.g. a//b | Removed |
| Part consisting of . or .. | AN |
| Colon and backslash | Removed |
| \* ? < > \| " | ! |
| Control characters U+0000 to U+001F | 0x followed by two hexadecimal digits, e.g. a line feed becomes 0x0A |
| Trailing space or full stop | Removed |
| Empty name | NÍ |
| Reserved names CON, PRN, AUX, NUL, CONIN$, CONOUT$, COM0 to COM9, LPT0 to LPT9 | Prefixed with ! |
| Name of more than 255 characters | Shortened, extension kept |

Windows compares file names without regard to case. Where two names in one folder are equal in that sense, the first keeps its name, and each later one receives a suffix before its extension. The suffixes are taken in the order a, b, ú, u, ù, g, d, e, ż, z, i, í, k, l, m, n, o, p, r, s, t, f, ț, č, ș, c, q, x and h, and continue with aa, ab and onward.

Relative paths are limited to 259 characters. Where long paths are disabled in Windows, the limit includes the destination path. A path that exceeds the limit is shortened by cutting the longest folder name first and then the file name; the extension is always kept.

### ( ii ) 7-Zip

The 7-Zip plugin, v31np.dll, adds .v31np as a format to 7-Zip 26.05. It is built for x86, x64 and ARM64 and is placed in the Formats folder of 7-Zip by baíbúočman.exe, a 32-bit 7-Zip on 64-bit Windows receiving the x86 build. The plugin carries the medallion PASSING (see ( U )).

The plugin registers the format name v31np and the extension v31np, and recognises a file by its magic followed by π. Folders are built from the keys, and each item reports its path, whether it is a folder, and its size. Folders are sorted before files, and names are sorted in V31N order as in ( Ú/( b.i.1. ) ).

7-Zip lists names as they are stored. On extraction, the plugin reports names naturalised and made unique as described in ( Ú/( b.i.4. ) ), while the length of paths is left to 7-Zip. Extraction does not verify the checksum. A test verifies the checksum over the whole archive and, on a mismatch, reports CRC Failed for every file, since the checksum does not cover single files (see ( B/( a.v. ) )).

The plugin supports update, by which files may be added, replaced, renamed and deleted; the archive is rewritten in full. A backslash in a path given by 7-Zip is converted to /. Dates and attributes are ignored, and empty folders, invalid UTF-8 and duplicate keys are refused.

# ( U ) V31NP STANDARD

The V31NP STANDARD is the format as defined in ( B ). Each programme described in ( Ú ) carries one of two medallions, which state whether the programme conforms to the standard.

The medallion PASSING is carried by a programme of which every .v31np file it writes conforms to ( B/( a ) ), and which reads every file that conforms. The medallion FAILING is carried by a programme that writes or keeps at least one .v31np file that does not conform to ( B/( a ) ), or that refuses a file that conforms.
