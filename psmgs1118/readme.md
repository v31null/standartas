<table>
<thead>
<tr><th colspan="6">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td colspan="3">1. Report No.<br>PS-MG/S-1118 R.1</td><td colspan="3">2. Report Date<br>2026/11/18</td></tr>
<tr><td colspan="6">3. Title<br>PRINZIAN TOP LEVEL DOMAIN</td></tr>
<tr><td colspan="6">4. Author(s)<br>P.K. — 619851078911877 , kokain@v31null.de</td></tr>
<tr><td colspan="3">a. Performing Organization Name(s) and Address(es)<br>Mbrlí úír Entípatùíțù fùțbíz — v31null.de , mef@v31null.de</td><td colspan="3">b. Sponsoring/Monitoring Agency Name(s) and Address(es)</td></tr>
<tr><td colspan="6">6. Abstract<br>This document sets the PRINZIAN TOP LEVEL DOMAIN pz: its addresses, its names and the keys from which they come, the registration of an address, the zone by which addresses are found and the copies by which it spreads, the certificates of the addresses and the authorities that issue them, the relay by which servers without an address of their own are reached, and the systems by which Windows, Firefox and GNU/Linux reach the domain.</td></tr>
<tr><td colspan="3" rowspan="2">7. Key Words<br>PRINZIAN TOP LEVEL DOMAIN, pz, address, key address, name, subdomain, P-384, HKDF-SHA-384, SHA-256, base32, registration, request, revision, zone, DNSSEC, copy, serial, ACME, tls-alpn-01, certificate, BISUGINMANU SERTIFIKAT UIR ALSERETUNS ASIFUEAS, BISUGINIARMANU SERTIFIKAT, Name Constraints, trusted list, relay, WebSocket, PROXY protocol, Windows, namespace provider, Firefox, GNU/Linux</td><td colspan="3">8. Distribution Statement</td></tr>
<tr><td>a. Report<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>b. Abstract<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>ú. This page<br>A; Approved for public release: distribution is unlimited, no restrictions.</td></tr>
<tr><td colspan="3">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td><td colspan="3">10. №ⁱ OF PP.<br>18</td></tr>
</tbody>
</table>

# ( A ) INTRODUCTION

## ( a ) Table of Contents

- [( A ) INTRODUCTION](#aintroduction)
  - [( b ) List of Tables](#blist-of-tables)
  - [( ú ) Notice](#únotice)
  - [( u ) Foreword](#uforeword)
  - [( ù ) Scope and Purpose](#ùscope-and-purpose)
  - [( g ) Notation](#gnotation)
- [( B ) THE DOMAIN](#bthe-domain)
  - [( a ) Addresses](#aaddresses)
  - [( b ) Names](#bnames)
  - [( ú ) Keys](#úkeys)
- [( Ú ) REGISTRATION](#úregistration)
  - [( a ) Procedure](#aprocedure)
  - [( b ) Holding of an Address](#bholding-of-an-address)
- [( U ) THE ZONE](#uthe-zone)
  - [( a ) Signing](#asigning)
  - [( b ) Entries](#bentries)
  - [( ú ) Copies](#úcopies)
- [( Ù ) CERTIFICATES](#ùcertificates)
  - [( a ) Authorities](#aauthorities)
  - [( b ) Issuance](#bissuance)
  - [( ú ) Trusted List](#útrusted-list)
- [( G ) THE RELAY](#gthe-relay)
- [( D ) SYSTEMS](#dsystems)
  - [( a ) Windows](#awindows)
  - [( b ) Firefox](#bfirefox)
  - [( ú ) GNU/Linux](#úgnulinux)
- [( E ) SETTING UP — INFORMATIVE](#esetting-upinformative)
  - [( a ) General](#ageneral)
  - [( b ) Users](#busers)
  - [( ú ) IT](#úit)
    - [( i ) System Administrators](#isystem-administrators)
    - [( ii ) Developers](#iidevelopers)
- [( Ż ) GLOSSARY](#żglossary)
- [( Z ) REFERENCES](#zreferences)

## ( b ) List of Tables

- [Table B-1 Symbols of the Keys](#table-b-1)
- [Table Ú-1 Steps of a Registration](#table-ú-1)
- [Table Ú-2 Fields of a Request](#table-ú-2)
- [Table U-1 Keys of the Zone](#table-u-1)
- [Table Ù-1 Authorities](#table-ù-1)

## ( ú ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the PRINZIAN TOP LEVEL DOMAIN. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document. v31n is not related to non official platforms, services, people, & ‘oc genus omne in any way or form. Textual, programmatic, and media content published across these services is made available under the terms of the v31n license. No statutory copyright, proprietary monopoly, or reservation of rights subsists in these works. Any person is granted full, unconditional, and irrevocable permission to access, copy, modify, adapt, distribute, or incorporate all materials for any purpose, without formality, fee, or prior authorization. The constitutive Latin charter and legal terms of this declaration are recorded in the system registry and resp. places.

## ( u ) Foreword

This document sets the PRINZIAN TOP LEVEL DOMAIN, the domain pz of v31n, and the way in which its addresses are registered, found and trusted. It applies to every person dealing with Prinzian systems and with native systems of v31n.

The domain is set out in § ( B ): its addresses, its names and the keys from which they come. § ( Ú ) sets out the registration of an address and the holding of it. § ( U ) sets out the zone: its signing, its entries and its copies. § ( Ù ) sets out the authorities that issue the certificates of the addresses, the issuance, and the entry in the trusted list of v31null. § ( G ) sets out the relay, and § ( D ) the systems by which Windows, Firefox and GNU/Linux reach the domain. § ( E ) gives, informatively, the example address of the domain and how users, IT and developers set up the domain. § ( Ż ) gives the terms used in this document, and § ( Z ) the documents to which it refers.

P.K.

## ( ù ) Scope and Purpose

The PRINZIAN TOP LEVEL DOMAIN, as defined by v31n, applies to every address under pz, to every person who registers one, and to every system that reaches one. An address under pz leads to its server, and the server proves itself by the key of the address.

This document defines how an address comes from a key, how it is registered, how it is found, and how its certificate is issued and trusted.

## ( g ) Notation

Addresses, names, file names and values are written as they are typed, as in dns.v31null.de. In formulas, ‖ denotes the joining of byte strings, mod the remainder of a division, and · the multiplication of a point of the curve by a number.

The requirements of this document are stated with shall, and each binds unless this document states otherwise. Should denotes a recommendation, which does not bind, and may denotes a permission. A part marked INFORMATIVE gives guidance and binds no one.

# ( B ) THE DOMAIN

## ( a ) Addresses

An address is reached as https:// followed by the address. Every registration has a key address, the label of § ( B/( ú ) ) followed by pz, as &lt;label&gt;.pz. An entry may also have a name and subdomains of § ( B/( b ) ).

## ( b ) Names

MEF gives names, on request to mef@v31null.de. A name is a label followed by pz, as &lt;name&gt;.pz, and a subdomain is a label followed by a name, as &lt;sub&gt;.&lt;name&gt;.pz. Each such label is of letters, digits and hyphens, without a hyphen at its start or end. Letters other than a to z are written as an internationalised domain name by UTS #46, without transitional processing, and the label stands in the zone in that form, beginning with xn--. A label of 52 characters, each of a to z or 2 to 7, is refused.

A name already held is not given. MEF decides a dispute over a name by the use of the name, the standards concerned and jurisprudence, and gives the reasons for its decision.

The holder of a name enters a subdomain by a change of § ( Ú/( b ) ) signed with the key of the name. The change names the key address of another active entry, and the subdomain leads to that entry. A subdomain stands in the zone only while its name and that entry stand.

## ( ú ) Keys

Every address comes from a key pair on the curve P-384. Its public key is also the key of the TLS server of the address. Table B-1 presents the symbols, and the key pair and the label are:

- k = HKDF-SHA-384 ( s , D ‖ c ) of 56 bytes, read as a number with its most significant byte first;
- d = ( k mod ( n − 1 ) ) + 1;
- Q = d · G;
- label = base32 ( SHA-256 ( Q ) ), in lower case, without padding, of 52 characters.

Each value of c gives another key pair and another label.

<table id="table-b-1">
<caption>Table B-1 Symbols of the Keys</caption>
<thead>
<tr><th>Symbol</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>s</td><td>The secret of the registrant, the input keying material of HKDF</td></tr>
<tr><td>D</td><td>The name or subdomain the key is made for, as &lt;name&gt;.pz or &lt;sub&gt;.&lt;name&gt;.pz, in lower case, or pz for a key without a name</td></tr>
<tr><td>c</td><td>A counter from 0, in four bytes, most significant byte first</td></tr>
<tr><td>n</td><td>The order of the base point of P-384</td></tr>
<tr><td>G</td><td>The base point of P-384</td></tr>
<tr><td>d</td><td>The private key</td></tr>
<tr><td>Q</td><td>The public key, hashed in its form as SubjectPublicKeyInfo in DER</td></tr>
</tbody>
</table>

HKDF-SHA-384 is HKDF with SHA-384, without salt. The step from k to d is that of key pair generation using extra random bits.

# ( Ú ) REGISTRATION

## ( a ) Procedure

A registration is made with v31null by the steps of Table Ú-1 and runs by itself.

<table id="table-ú-1">
<caption>Table Ú-1 Steps of a Registration</caption>
<thead>
<tr><th>Step</th><th>By</th><th>What</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>Registrant</td><td>Makes the key pair of § ( B/( ú ) ).</td></tr>
<tr><td>2</td><td>Registrant</td><td>Sends v31null the public key and the addresses of the server or the URL of its relay, as a JSON Web Signature of RFC 7515 of the algorithm ES384 of RFC 7518, signed with the private key and holding a nonce given by v31null.</td></tr>
<tr><td>3</td><td>v31null</td><td>Tests the public key by the ECC Full Public-Key Validation Routine, section 5.6.2.3.3 of SP 800-56A Rev. 3.</td></tr>
<tr><td>4</td><td>v31null</td><td>Connects to the server over TLS on port 443 with the server name &lt;label&gt;.pz, directly or through its relay; the server presents exactly that public key.</td></tr>
<tr><td>5</td><td>Server</td><td>Answers with 200 the request for /.well-known/psmgs1118r1_Ú.a--Token/&lt;token&gt;, where the token is of at least 128 random bits given by v31null.</td></tr>
<tr><td>6</td><td>v31null</td><td>Records the entry, publishes the request and logs the registration.</td></tr>
</tbody>
</table>

v31null gives the nonce and the token of a registration together, at https://dns.v31null.de/psmgs1118r1_Ú.a--Nonce, in the header fields Replay-Nonce and Psmgs1118r1-Token. Each is valid for 24 hours and is used once. The request of step 2 is sent to https://dns.v31null.de/psmgs1118r1_Ú.a--Registration, of media type application/jose+json; its protected header holds alg ES384, the public key as jwk of RFC 7517, the nonce and the URL, and its payload the fields of Table Ú-2, with addresses or relay, not both. It is answered once steps 3 to 5 are done. The addresses of a server are public IP addresses. v31null shall accept at most one request from each IP address each hour, and the registrant shall send it only once the server answers as in steps 4 and 5. A key whose entry was removed registers again with a higher revision. v31null publishes, at https://dns.v31null.de/psmgs1118r1_Ú.a--Request.&lt;label&gt;.json, of media type application/jose+json, the newest request or change of each entry that set its addresses or relay, or the change that removed it.

<table id="table-ú-2">
<caption>Table Ú-2 Fields of a Request</caption>
<thead>
<tr><th>Field</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>addresses</td><td>The IP addresses of the server, as a list</td></tr>
<tr><td>relay</td><td>The URL of the relay, beginning with wss://</td></tr>
<tr><td>revision</td><td>A number from 1, higher than that of every earlier request and change of the key</td></tr>
</tbody>
</table>

## ( b ) Holding of an Address

Registration is free. Each key holds one entry. A change to an entry is accepted only when signed with the private key of that entry, as a JSON Web Signature of the algorithm ES384 holding a nonce given by v31null.

MEF holds a name for a key without a server by recording an entry from the public key and the name, without addresses or relay and without the steps of Table Ú-1. A change of the entry sets its addresses or relay later.

A change is sent to https://dns.v31null.de/psmgs1118r1_Ú.b--Change. Its protected header holds the public key as jwk and the label as kid. Its payload holds a revision, higher than that of every earlier request and change of the key, and one of the following:

- addresses or relay, which replace the addresses or the relay;
- name, set to null, which drops the name;
- subdomain together with label, which enters the subdomain leading to the entry of that key address, or drops it when label is null;
- remove, set to true, which removes the entry.

Replaced addresses or a replaced relay are tested again by steps 4 and 5 of Table Ú-1. A removed entry leaves the zone and its name is free. v31null shall accept at most one change from each IP address each hour.

# ( U ) THE ZONE

## ( a ) Signing

The zone pz holds every entry of the domain. It is a zone of the DNS signed with DNSSEC by algorithm 14, ECDSAP384SHA384, with denial of existence by NSEC, and v31null alone signs it. Each signing raises its serial by one. The zone is signed each day, and each signature is valid for 30 days.

The apex names dns.v31null.de as the name server and as the primary master of the SOA record, and mef.v31null.de as the mailbox of the person responsible for the zone.

Table U-1 presents the keys of the zone.

<table id="table-u-1">
<caption>Table U-1 Keys of the Zone</caption>
<thead>
<tr><th>Key</th><th>Flags</th><th>Signs</th><th>Replaced</th></tr>
</thead>
<tbody>
<tr><td>KSK</td><td>257</td><td>The DNSKEY set</td><td>Each year</td></tr>
<tr><td>ZSK</td><td>256</td><td>Every other set</td><td>Every 90 days</td></tr>
</tbody>
</table>

BISUGINIARMANU SERTIFIKAT certifies the KSK by a certificate of 1 year, of subject C=PZ, O=MEF, CN=pz, with the subject alternative name dNSName pz and the extended key usage serverAuth.

## ( b ) Entries

An entry gives, for its key address, the address of the server, the URL of its relay, or, for an entry held by MEF without a server, neither. The name of an entry is a CNAME record to its key address, and each subdomain a CNAME record to the key address of the entry to which it leads. The key address holds the public key of the address as a TLSA record at _443._tcp.&lt;label&gt;.pz, of the usage DANE-EE, the selector SubjectPublicKeyInfo and the full key. An entry held without a server holds only its TLSA record and the CNAME record of its name. The key address of a server at an address of its own holds the IP addresses of the server as A and AAAA records, and the server is reached at that address on port 443. The key address of a server behind a relay holds the URL of the relay, beginning with wss://, as a URI record of RFC 7553 at _relay._tcp.&lt;label&gt;.pz, and the server is reached by § ( G ). The key address of a server behind a relay also holds, as A and AAAA records, the IP addresses of the bridge of v31null. The zone holds the same addresses of the bridge as A and AAAA records at _bridge._tcp.pz.

## ( ú ) Copies

The zone is published at dns.v31null.de over HTTPS and HTTP: the whole zone as psmgs1118r1_U.ú--Zone.zone in the master file format of RFC 1035, of media type text/dns, the certificate of its KSK as psmgs1118r1_U.a--KSK.cer, and the changes from each serial to the next as psmgs1118r1_U.ú--Delta.&lt;from&gt;.&lt;to&gt;.zone. A change gives, in the order of RFC 1995, the SOA record of the older serial, the records removed, the SOA record of the newer serial, and the records added. The 30 latest changes are published. The zone names the places that publish copies as URI records at _mirror._tcp.pz, each with the URL of the whole zone. Each such place publishes over HTTPS, at the root of its address and under the names of dns.v31null.de, the whole zone, the certificate of the KSK, and the changes and requests it holds. Every system of § ( D ) keeps a copy for itself and looks for a newer serial at the places of _mirror._tcp.pz every 12 hours.

A copy is tested whole by its ZONEMD record of RFC 8976, of the scheme SIMPLE and the hash SHA-384, and by its DNSSEC signatures, and holds when its KSK equals the key of the certificate of the KSK and that certificate chains to the root CA of Table Ù-1. Of two copies, the one with the higher serial holds. The entries of a copy are found while v31null is offline. A key address is found from the published request of its key: a holder fetches it from the places of _mirror._tcp.pz and tests it by its signature, by the label of its jwk and by its URL. It keeps the one of the highest revision and tests it again on use once 15 minutes have passed. For a key address, the request holds over the zone, and a removed entry leads nowhere. A name and a subdomain are found only from the zone, from the next signing.

# ( Ù ) CERTIFICATES

## ( a ) Authorities

The certificates of the addresses come from the chain of Table Ù-1, of keys on P-384.

<table id="table-ù-1">
<caption>Table Ù-1 Authorities</caption>
<thead>
<tr><th>Authority</th><th>Common Name</th><th>Name Constraints</th><th>Extended Key Usage</th><th>Path Length</th><th>Validity</th></tr>
</thead>
<tbody>
<tr><td>Root CA</td><td>BISUGINMANU SERTIFIKAT UIR ALSERETUNS ASIFUEAS</td><td>Permitted dNSName pz</td><td>serverAuth</td><td></td><td>15 years</td></tr>
<tr><td>Subordinate CA</td><td>BISUGINIARMANU SERTIFIKAT</td><td>Permitted dNSName pz</td><td>serverAuth</td><td>0</td><td>3 years</td></tr>
</tbody>
</table>

## ( b ) Issuance

BISUGINIARMANU SERTIFIKAT issues the certificate of an address by ACME, with the challenge tls-alpn-01 of RFC 8737 on port 443 of the address, and renews it by ACME. Each certificate of an address is valid for 90 days. It issues a certificate for a key address only for the public key of its active entry, and for a name or subdomain only for the public key of the entry in the zone to which it leads. One certificate holds the key address of a key and every name and subdomain leading to it. The unwrapper of § ( G ) obtains and renews the certificates of the servers behind it, and the bridge those of the servers it serves, once fewer than 30 days remain.

## ( ú ) Trusted List

The trusted list of v31null names the root CA and the subordinate CA of Table Ù-1, each as a service with the additional service information ForWebSiteAuthentication.

# ( G ) THE RELAY

A connection to a server behind a relay is a WebSocket to the relay, inside which the TLS connection with the server runs from end to end. The bridge of v31null receives the TLS connection on port 443, finds the relay of its server name in its copy of the zone, and passes the connection on through the WebSocket without opening it. On the server, the WebSocket is unwrapped and the TLS connection is passed on with the address of the visitor, as the relay reports it, by the PROXY protocol. The bytes of the TLS connection travel as binary messages of RFC 6455. The PROXY protocol is of version 2; where the relay reports no address of the visitor, its command is LOCAL. The test of step 4 of Table Ú-1 runs the same way. The bridge of v31null also serves servers of its own, and a connection with their server name ends at the bridge.

# ( D ) SYSTEMS

## ( a ) Windows

On Windows, the system of v31n for the domain installs the root CA of Table Ù-1 in the store of the system, keeps a copy of the zone, writes the A and AAAA records of every address of the domain into the hosts file of the system, and installs psmgs1118r1_D.a--Namespace.dll, a namespace provider of Winsock of 64 and of 32 bits. It answers every name ending in .pz by § ( U/( ú ) ), and a server behind a relay with the addresses of _bridge._tcp.pz. A task of the system renews the copy and the hosts file at start and every 12 hours. A server of the domain keeps its copy, answers from it, and directs every name ending in .pz to that answer by a rule of the Name Resolution Policy Table.

## ( b ) Firefox

Firefox reaches the domain with the policies.json of Table Ú-1 of PS-MG/S-1103, in /etc/firefox/policies on GNU/Linux and in the distribution folder of Firefox on Windows.

## ( ú ) GNU/Linux

On GNU/Linux, the root CA of Table Ù-1 is installed in the store of the system as in Table Ú-1 of PS-MG/S-1103, a copy of the zone is kept and answers, and systemd-resolved directs every name ending in .pz to that answer by the routing domain ~pz.

# ( E ) SETTING UP — INFORMATIVE

## ( a ) General

raremit.pz serves as the example address of the domain, as example.com does in the DNS by RFC 2606.

## ( b ) Users

On Windows, a user takes https://dns.v31null.de/psmgs1118r1_D.a--Hosts.exe and runs it once. It asks for the rights of an administrator and sets up the system of § ( D/( a ) ). Where Firefox is installed without a policies.json, it also sets up the policies.json of § ( D/( b ) ). On GNU/Linux, a user sets up the system of § ( D/( ú ) ). A key address is reached within 30 minutes of its registration or change. A name or subdomain is reached within 36 hours.

## ( ú ) IT

### ( i ) System Administrators

A server of the domain keeps running its web server, which serves &lt;label&gt;.pz with &lt;label&gt;.pz-key.pem and &lt;label&gt;.pz.pem, and, for a server behind a relay, the tunnel to which the URL of the relay leads and the unwrapper of § ( E/( ú.ii. ) ) step 5.

1. At start and every 12 hours the unwrapper prints one line for each address whose certificate it obtained or renewed: the address and the serial of the certificate, or the address and the error.
2. After writing a certificate, the unwrapper restarts the Apache service of XAMPP by httpd -k restart -n Apache2.4. Without the rights of an administrator the restart does not take effect, and the web server is restarted by hand.
3. The unwrapper is restarted after a registration, and after a signing that brings a new name or subdomain, so that the certificate holds them at once.
4. The newest request of an entry stands at https://dns.v31null.de/psmgs1118r1_Ú.a--Request.&lt;label&gt;.json, and the zone with its serial at https://dns.v31null.de/psmgs1118r1_U.ú--Zone.zone. A name or subdomain is in place once it stands in the zone.
5. Once the server answers as in steps 4 and 5 of Table Ú-1 at its new place, node psmgs1118r1_Ú.a--Registrant.js change --secret &lt;file&gt; --tokens &lt;folder&gt; --revision &lt;n&gt; --payload {"addresses":["&lt;ip&gt;"]} or --payload {"relay":"wss://&lt;host&gt;"} sends the new IP addresses or the new relay.
6. node psmgs1118r1_Ú.a--Registrant.js change --secret &lt;file&gt; --revision &lt;n&gt; --payload {"remove":true} removes the entry.
7. A server at an address of its own renews its certificate by the command of § ( E/( ú.ii. ) ) step 7 with --renew-days 30, and the web server frees port 443 while it runs. lego writes the certificate as &lt;folder&gt;/certificates/&lt;label&gt;.pz.crt, which the web server serves as &lt;label&gt;.pz.pem.

### ( ii ) Developers

A developer takes from https://dns.v31null.de/ the registrant tool psmgs1118r1_Ú.a--Registrant.js and, for a server behind a relay, into one folder the unwrapper psmgs1118r1_G--Unwrapper.js, psmgs1118r1_Ù.b--ACME.js, psmgs1118r1_U.ú--Copy.js, psmgs1118r1_Ù.a--Root_CA.cer and psmgs1118r1_Ù.a--Subordinate_CA.cer. Each runs on Node.js of version 24 without further packages. A server is set up as follows:

1. node psmgs1118r1_Ú.a--Registrant.js secret --out &lt;file&gt; makes the secret s of § ( B/( ú ) ) as 48 random bytes in a new file. Without the secret or the key file of step 2, no change of the entry can be signed.
2. node psmgs1118r1_Ú.a--Registrant.js key --secret &lt;file&gt; --out &lt;folder&gt; makes the key pair, writes it into the folder as &lt;label&gt;.pz-key.pem and prints the label. --name &lt;name&gt; makes the key for a name or subdomain, as D of Table B-1, and --counter &lt;c&gt; gives another key pair; both change only the key pair and its label, and the name itself is given by MEF in step 8.
3. openssl req -x509 -new -key &lt;label&gt;.pz-key.pem -subj /CN=&lt;label&gt;.pz -addext subjectAltName=DNS:&lt;label&gt;.pz -days 1 -out &lt;label&gt;.pz.pem makes the self-signed certificate that the server presents until its certificate of § ( Ù/( b ) ) is issued.
4. A server at an address of its own answers TLS on port 443 of its public IP addresses for the server name &lt;label&gt;.pz with these two files, and serves a folder for tokens under /.well-known/psmgs1118r1_Ú.a--Token/.
5. A server behind a relay takes a tunnel, as Cloudflare Tunnel or zrok, whose wss:// URL leads to a port on 127.0.0.1; that URL is the URL of the relay. node psmgs1118r1_G--Unwrapper.js &lt;port&gt; 127.0.0.1 &lt;port of the web server&gt; &lt;folder&gt;, where &lt;folder&gt; is the folder of step 2, listens on that port and passes each connection on to the web server by the PROXY protocol of version 2. The web server listens on a port of its own on 127.0.0.1 that reads the PROXY protocol, as by RemoteIPProxyProtocol On of Apache, which applies to the whole address and port; it serves &lt;label&gt;.pz with the key and the certificate from that folder, and the folder for tokens as in step 4.
6. node psmgs1118r1_Ú.a--Registrant.js register --secret &lt;file&gt; --tokens &lt;folder&gt; --revision 1 sends the request of Table Ú-1, together with either --addresses &lt;ip&gt;,&lt;ip&gt; or --relay wss://&lt;host&gt;. It fetches the nonce and the token, writes the token file into the folder for tokens and prints the answer. The key address is reached at once. --key &lt;file&gt; takes a key file in place of the secret.
7. Behind a relay, the unwrapper, once restarted, obtains the certificate of every &lt;label&gt;.pz-key.pem in its folder and writes it as &lt;label&gt;.pz.pem. A server at an address of its own makes a request of that key, openssl req -new -key &lt;label&gt;.pz-key.pem -subj /CN=&lt;label&gt;.pz -addext subjectAltName=DNS:&lt;label&gt;.pz -out &lt;label&gt;.pz.csr, and sends it with an ACME client of tls-alpn-01 to https://acme.v31null.de/acme/psmgs1118r1/directory, as lego of version 5 by lego run --server https://acme.v31null.de/acme/psmgs1118r1/directory --accept-tos --csr &lt;label&gt;.pz.csr --tls --path &lt;folder&gt;. The folder of --path is not the folder of step 2. The web server frees port 443 while the ACME client runs. A certificate for a key that the client makes itself is refused.
8. A name is asked of MEF at mef@v31null.de, giving the key address. The name reaches the zone at the next signing, and the certificate holds it from its next renewal.
9. The holder of a name enters a subdomain by node psmgs1118r1_Ú.a--Registrant.js change --secret &lt;file&gt; --name &lt;name&gt; --revision &lt;n&gt; --payload {"subdomain":"&lt;sub&gt;","label":"&lt;label&gt;"}, where &lt;label&gt; is the label of the entry to which the subdomain leads.

# ( Ż ) GLOSSARY

<table>
<tbody>
<tr><td>Address</td><td>A key address or a name under pz.</td></tr>
<tr><td>Copy</td><td>A copy of the zone, tested by its signatures; see § ( U/( ú ) ).</td></tr>
<tr><td>Entry</td><td>What the zone holds for one registration; see § ( U/( b ) ).</td></tr>
<tr><td>Key address</td><td>The label of a public key followed by pz; see § ( B/( a ) ).</td></tr>
<tr><td>Label</td><td>The 52 characters of base32 of the SHA-256 of a public key; see § ( B/( ú ) ).</td></tr>
<tr><td>MEF</td><td>Mbrlí úír Entípatùíțù fùțbíz, v31null.de.</td></tr>
<tr><td>Name</td><td>A name given by MEF followed by pz; see § ( B/( b ) ).</td></tr>
<tr><td>Registrant</td><td>The person who registers an address.</td></tr>
<tr><td>Relay</td><td>The host through which a server without an address of its own is reached; see § ( G ).</td></tr>
<tr><td>Request</td><td>The signed registration or change that sets the addresses or relay of an entry; see § ( Ú/( a ) ).</td></tr>
<tr><td>Serial</td><td>The number of a signing of the zone.</td></tr>
<tr><td>Subdomain</td><td>A label followed by a name, leading to another entry; see § ( B/( b ) ).</td></tr>
<tr><td>Zone</td><td>The zone pz, signed by v31null; see § ( U ).</td></tr>
</tbody>
</table>

# ( Z ) REFERENCES

European Telecommunications Standards Institute (ETSI) (2025) "TS 119 612 V2.4.1: Electronic Signatures and Trust Infrastructures (ESI); Trusted Lists".

HAProxy Technologies (n.d.) "The PROXY protocol".

Internet Engineering Task Force (IETF) (1987) "RFC 1035: Domain Names — Implementation and Specification".

Internet Engineering Task Force (IETF) (1996) "RFC 1995: Incremental Zone Transfer in DNS".

Internet Engineering Task Force (IETF) (1999) "RFC 2606: Reserved Top Level DNS Names".

Internet Engineering Task Force (IETF) (2003) "RFC 3596: DNS Extensions to Support IP Version 6".

Internet Engineering Task Force (IETF) (2005) "RFC 4027: Domain Name System Media Types".

Internet Engineering Task Force (IETF) (2005) "RFC 4033: DNS Security Introduction and Requirements".

Internet Engineering Task Force (IETF) (2005) "RFC 4034: Resource Records for the DNS Security Extensions".

Internet Engineering Task Force (IETF) (2005) "RFC 4035: Protocol Modifications for the DNS Security Extensions".

Internet Engineering Task Force (IETF) (2006) "RFC 4648: The Base16, Base32, and Base64 Data Encodings".

Internet Engineering Task Force (IETF) (2008) "RFC 5280: Internet X.509 Public Key Infrastructure Certificate and Certificate Revocation List (CRL) Profile".

Internet Engineering Task Force (IETF) (2010) "RFC 5869: HMAC-based Extract-and-Expand Key Derivation Function (HKDF)".

Internet Engineering Task Force (IETF) (2011) "RFC 6455: The WebSocket Protocol".

Internet Engineering Task Force (IETF) (2012) "RFC 6605: Elliptic Curve Digital Signature Algorithm (DSA) for DNSSEC".

Internet Engineering Task Force (IETF) (2012) "RFC 6698: The DNS-Based Authentication of Named Entities (DANE) Transport Layer Security (TLS) Protocol: TLSA".

Internet Engineering Task Force (IETF) (2015) "RFC 7515: JSON Web Signature (JWS)".

Internet Engineering Task Force (IETF) (2015) "RFC 7517: JSON Web Key (JWK)".

Internet Engineering Task Force (IETF) (2015) "RFC 7518: JSON Web Algorithms (JWA)".

Internet Engineering Task Force (IETF) (2015) "RFC 7553: The Uniform Resource Identifier (URI) DNS Resource Record".

Internet Engineering Task Force (IETF) (2018) "RFC 8446: The Transport Layer Security (TLS) Protocol Version 1.3".

Internet Engineering Task Force (IETF) (2019) "RFC 8555: Automatic Certificate Management Environment (ACME)".

Internet Engineering Task Force (IETF) (2020) "RFC 8737: Automated Certificate Management Environment (ACME) TLS Application-Layer Protocol Negotiation (ALPN) Challenge Extension".

Internet Engineering Task Force (IETF) (2021) "RFC 8976: Message Digest for DNS Zones".

Microsoft (n.d.) "Name Resolution Policy Table".

Microsoft (n.d.) "Namespace Service Providers".

Mozilla (n.d.) "Policy templates for Firefox".

National Institute of Standards and Technology (NIST) (2015) "FIPS 180-4: Secure Hash Standard (SHS)".

National Institute of Standards and Technology (NIST) (2018) "SP 800-56A Rev. 3: Recommendation for Pair-Wise Key-Establishment Schemes Using Discrete Logarithm Cryptography".

National Institute of Standards and Technology (NIST) (2023) "FIPS 186-5: Digital Signature Standard (DSS)".

National Institute of Standards and Technology (NIST) (2023) "SP 800-186: Recommendations for Discrete Logarithm-based Cryptography: Elliptic Curve Domain Parameters".

systemd (n.d.) "resolved.conf".

Unicode Consortium (n.d.) "Unicode Technical Standard #46: Unicode IDNA Compatibility Processing".

PS-B/P-1116

PS-MG/S-1103
