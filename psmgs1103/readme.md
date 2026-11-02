<table>
<thead>
<tr><th colspan="6">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td colspan="3">1. Report No.<br>PS-MG/S-1103 R.1</td><td colspan="3">2. Report Date<br>2026/11/03</td></tr>
<tr><td colspan="6">3. Title<br>DEFAULT TRUST OF MACHINES</td></tr>
<tr><td colspan="6">4. Author(s)<br>P.K. — 619851078911877 , kokain@v31null.de</td></tr>
<tr><td colspan="3">a. Performing Organization Name(s) and Address(es)<br>Mbrlí úír Entípatùíțù fùțbíz — v31null.de , mef@v31null.de</td><td colspan="3">b. Sponsoring/Monitoring Agency Name(s) and Address(es)</td></tr>
<tr><td colspan="6">6. Abstract<br>This document sets the DEFAULT TRUST OF MACHINES: the six certificates of the certification authorities of v31n, the root CA and the subordinate CA of the chains RSA-4096, P-384 and ML-DSA-87, with which every Prinzian machine shall come installed, the address from which they are obtained, the support of OEMs and suppliers, and their installation on Windows, on GNU/Linux and on MacOS.</td></tr>
<tr><td colspan="3" rowspan="2">7. Key Words<br>DEFAULT TRUST OF MACHINES, Prinzian machine, certificate, certification authority, root CA, subordinate CA, RSA-4096, P-384, ML-DSA-87, Windows, GNU/Linux, NSS, Firefox</td><td colspan="3">8. Distribution Statement</td></tr>
<tr><td>a. Report<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>b. Abstract<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>ú. This page<br>A; Approved for public release: distribution is unlimited, no restrictions.</td></tr>
<tr><td colspan="3">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td><td colspan="3">10. №ⁱ OF PP.<br>6</td></tr>
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
- [( B ) DEFAULT TRUST](#bdefault-trust)
  - [( a ) Certificates](#acertificates)
  - [( b ) Source](#bsource)
  - [( ú ) OEMs and Suppliers](#úoems-and-suppliers)
- [( Ú ) INSTALLATION](#úinstallation)
  - [( a ) Windows](#awindows)
  - [( b ) GNU/Linux](#bgnulinux)
  - [( ú ) MacOS](#úmacos)

## ( b ) List of Tables

- [Table B-1 Certificates](#table-b-1)
- [Table Ú-1 Steps on GNU/Linux](#table-ú-1)

## ( ú ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the DEFAULT TRUST OF MACHINES. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document. v31n is not related to non official platforms, services, people, & ‘oc genus omne in any way or form. Textual, programmatic, and media content published across these services is made available under the terms of the v31n license. No statutory copyright, proprietary monopoly, or reservation of rights subsists in these works. Any person is granted full, unconditional, and irrevocable permission to access, copy, modify, adapt, distribute, or incorporate all materials for any purpose, without formality, fee, or prior authorization. The constitutive Latin charter and legal terms of this declaration are recorded in the system registry and resp. places.

## ( u ) Foreword

This document sets the DEFAULT TRUST OF MACHINES, the certification authorities of v31n that every Prinzian machine trusts from the moment it is delivered. It applies to every person dealing with Prinzian systems and with native systems of v31n.

The certificates, their source and the support of OEMs and suppliers are set out in § ( B ). § ( Ú ) sets out their installation on Windows, on GNU/Linux and on MacOS.

P.K.

## ( ù ) Scope and Purpose

A Prinzian machine trusts the certification authorities of v31n by default. This document names the certificates of those authorities, the address from which they are obtained, and the means by which they are installed.

## ( g ) Notation

Addresses, file names, paths and commands are written as they are typed, as in https://v31null.de/dss. OEM denotes an original equipment manufacturer. In commands, &lt;name&gt; stands for the name of a certificate file without its extension.

A reference to a part of this document gives the sign § followed by the division and the part, as in § ( Ú/( b ) ), which denotes ( b ) of ( Ú ). Tables are numbered by division, as in Table B-1.

The requirements of this document are stated with shall, and each binds unless this document states otherwise. Should denotes a recommendation, which does not bind, and may denotes a permission.

# ( B ) DEFAULT TRUST

## ( a ) Certificates

Every Prinzian machine shall come with the six certificates of Table B-1 installed and trusted as certification authorities. The certification authorities of v31n form three chains, RSA-4096, P-384 and ML-DSA-87, each of a root CA and a subordinate CA.

<table id="table-b-1">
<caption>Table B-1 Certificates</caption>
<thead>
<tr><th>Chain</th><th>Authority</th><th>File</th></tr>
</thead>
<tbody>
<tr><td rowspan="2">RSA-4096</td><td>Root CA</td><td>en_31941101v010501p_3.1--root_CA_RSA-4096.cer</td></tr>
<tr><td>Subordinate CA</td><td>en_31941101v010501p_3.1--subordinate_CA_RSA-4096.cer</td></tr>
<tr><td rowspan="2">P-384</td><td>Root CA</td><td>en_31941101v010501p_3.1--root_CA_P-384.cer</td></tr>
<tr><td>Subordinate CA</td><td>en_31941101v010501p_3.1--subordinate_CA_P-384.cer</td></tr>
<tr><td rowspan="2">ML-DSA-87</td><td>Root CA</td><td>en_31941101v010501p_3.1--root_CA_ML-DSA-87.cer</td></tr>
<tr><td>Subordinate CA</td><td>en_31941101v010501p_3.1--subordinate_CA_ML-DSA-87.cer</td></tr>
</tbody>
</table>

## ( b ) Source

The certificates shall be obtained from https://v31null.de/dss.

## ( ú ) OEMs and Suppliers

An OEM receives support for its machines only where every machine it delivers comes with the certificates of Table B-1 installed and trusted before it reaches the end user. The same holds for every intermediate supplier, distributor and reseller through whose hands a machine passes: support is given only where the certificates are installed and trusted when the machine leaves its hands. Every party in the chain of supply is taken into account. A machine that reaches the end user without the certificates is not supported, whichever party omitted or removed them.

# ( Ú ) INSTALLATION

## ( a ) Windows

On Windows, downloading the certificates from https://v31null.de/dss is enough.

## ( b ) GNU/Linux

On GNU/Linux, the commands of Table Ú-1 may be used on systems of the Debian family. They are run as root, in order, with the certificates of Table B-1 downloaded into one folder. They place the certificates in the store of the system, in the NSS database of the system and in the NSS database of each user, make Firefox take the certificates of the system, and point the programmes that read a bundle of certificates from the environment to the bundle of the system.

<table id="table-ú-1">
<caption>Table Ú-1 Steps on GNU/Linux</caption>
<thead>
<tr><th>Step</th><th>Command</th></tr>
</thead>
<tbody>
<tr><td>Folders</td><td>mkdir -p /usr/local/share/ca-certificates/v31null /etc/pki/nssdb /etc/firefox/policies</td></tr>
<tr><td>Conversion of each certificate</td><td>openssl x509 -in &lt;name&gt;.cer -inform DER -out /usr/local/share/ca-certificates/v31null/&lt;name&gt;.crt; for a certificate already in PEM, the same without -inform DER</td></tr>
<tr><td>Store of the system</td><td>update-ca-certificates</td></tr>
<tr><td>NSS database of the system</td><td>certutil -d sql:/etc/pki/nssdb -N --empty-password, where the database does not yet exist</td></tr>
<tr><td>Each certificate into the NSS database of the system</td><td>certutil -d sql:/etc/pki/nssdb -A -t "CT,C,C" -n &lt;name&gt; -i /usr/local/share/ca-certificates/v31null/&lt;name&gt;.crt</td></tr>
<tr><td>Each certificate into the NSS database of each user</td><td>certutil -d sql:$HOME/.pki/nssdb -A -t "CT,C,C" -n &lt;name&gt; -i /usr/local/share/ca-certificates/v31null/&lt;name&gt;.crt, then the folder $HOME/.pki returned to its user with chown -R</td></tr>
<tr><td>Firefox</td><td>/etc/firefox/policies/policies.json holding {"policies": {"Certificates": {"ImportEnterpriseRoots": true}}}</td></tr>
<tr><td>Environment</td><td>/etc/environment holding SSL_CERT_FILE, REQUESTS_CA_BUNDLE, CURL_CA_BUNDLE and NODE_EXTRA_CA_CERTS, each set to /etc/ssl/certs/ca-certificates.crt</td></tr>
</tbody>
</table>


## ( ú ) MacOS

MacOS is not supported. Responsibility for this lies solely with Apple.

Pursuant to the software license agreement of MacOS, the operating system is legally permitted to run exclusively on hardware designed and manufactured by Apple. Apple is consequently the sole OEM of every device executing MacOS, and is alone capable of fulfilling the requirements of § ( B/( ú ) ).

Furthermore, MacOS restricts default trust exclusively to certification authorities admitted under the Apple Root Store Program, under arbitrary terms, auditing regimes, and compliance diktats established by Apple alone. While v31n publishes its root and subordinate certificates openly at https://v31null.de/dss without fee or restriction for all systems, Apple has taken no reciprocal steps toward interoperability.

Support for MacOS shall be withheld until Apple delivers all hardware with the certificates of Table B-1 installed and trusted by default.
