<table>
<thead>
<tr><th colspan="6">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td colspan="3">1. Report No.<br>PS‑N/PN-1117 R.4</td><td colspan="3">2. Report Date<br>2026/11/18</td></tr>
<tr><td colspan="6">3. Title<br>PRONAL NETWORK STANDARD</td></tr>
<tr><td colspan="6">4. Author(s)<br>P.K. — 619851078911877 , kokain@v31null.de</td></tr>
<tr><td colspan="3">a. Performing Organization Name(s) and Address(es)<br>Services for þe General populace Maß Tele‑kommunikation services namen as prono — prono.v31null.de</td><td colspan="3">b. Sponsoring/Monitoring Agency Name(s) and Address(es)<br>Mbrlí úír Entípatùíțù fùțbíz — v31null.de , mef@v31null.de</td></tr>
<tr><td colspan="6">6. Abstract<br>This document sets the PRONAL NETWORK STANDARD: what the pronal network is and which of its parts are pronal, the identifiers and the time by which its records are named, how its database is kept, how a pronal application authenticates through the V31N Portal, the requests by which an application talks with the pronal network, the pushes by which the pronal network talks back, the calls between users, the mesh in which the devices of users talk with each other — its links, its relay layer, its talk layer and its fold —, the limits of each part, and what an application shall and may support.</td></tr>
<tr><td colspan="3" rowspan="2">7. Key Words<br>PRONAL NETWORK STANDARD, pronal network, pronal application, Prono, V31N Portal, OIDC, bearer token, pin, min, big_date, psí1103r1, talk, NEGL, ENC2, Socket.IO, WebRTC, MLS, mesh, node, pronal://, QUIC, certificate, move, fold</td><td colspan="3">8. Distribution Statement</td></tr>
<tr><td>a. Report<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>b. Abstract<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>ú. This page<br>A; Approved for public release: distribution is unlimited, no restrictions.</td></tr>
<tr><td colspan="3">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td><td colspan="3">10. №ⁱ OF PP.<br>55</td></tr>
</tbody>
</table>

# ( A ) INTRODUCTION

## ( a ) Table of Contents

- [( A ) INTRODUCTION](#aintroduction)
  - [( b ) List of Tables](#blist-of-tables)
  - [( ú ) Notice](#únotice)
  - [( u ) Foreword](#uforeword)
  - [( ù ) Scope and Purpose](#ùscope-and-purpose)
  - [( g ) Notation](#gnotation)
- [( B ) THE PRONAL NETWORK](#bthe-pronal-network)
  - [( a ) Layers](#alayers)
  - [( b ) Parts of the Pronal Network](#bparts-of-the-pronal-network)
  - [( ú ) Admission](#úadmission)
  - [( u ) The Mesh](#uthe-mesh)
- [( Ú ) IDENTIFIERS AND TIME](#úidentifiers-and-time)
  - [( a ) Time](#atime)
  - [( b ) Identifiers](#bidentifiers)
  - [( ú ) Types of Talks](#útypes-of-talks)
- [( U ) THE DATABASE](#uthe-database)
  - [( a ) Records](#arecords)
  - [( b ) Deletion](#bdeletion)
  - [( ú ) Encryption](#úencryption)
  - [( u ) Search Index](#usearch-index)
  - [( ù ) Friendships and NEGL](#ùfriendships-and-negl)
  - [( g ) Attachments](#gattachments)
- [( Ù ) AUTHENTICATION](#ùauthentication)
  - [( a ) The Bearer Token](#athe-bearer-token)
  - [( b ) The V31N Portal — INFORMATIVE](#bthe-v31n-portalinformative)
  - [( ú ) The Certificate](#úthe-certificate)
- [( G ) THE REQUESTS](#gthe-requests)
  - [( a ) Form of a Request](#aform-of-a-request)
  - [( b ) Status Codes](#bstatus-codes)
  - [( ú ) Endpoints](#úendpoints)
  - [( u ) The Message](#uthe-message)
  - [( ù ) System Messages](#ùsystem-messages)
  - [( g ) The State](#gthe-state)
  - [( d ) Emendetur Blocks](#demendetur-blocks)
- [( D ) THE PUSHES](#dthe-pushes)
- [( E ) CALLS](#ecalls)
  - [( a ) Calls Between Two Users](#acalls-between-two-users)
  - [( b ) Calls in a Group](#bcalls-in-a-group)
  - [( ú ) Encryption of Calls in a Group](#úencryption-of-calls-in-a-group)
- [( Ż ) THE LINKS](#żthe-links)
  - [( a ) pronal://](#apronal)
  - [( b ) Peer Proof](#bpeer-proof)
  - [( ú ) Links on One Network](#úlinks-on-one-network)
  - [( u ) Links Without a Network](#ulinks-without-a-network)
  - [( ù ) Links Across the Internet](#ùlinks-across-the-internet)
  - [( g ) The Server Node](#gthe-server-node)
  - [( d ) The Node of a Device](#dthe-node-of-a-device)
- [( Z ) THE RELAY LAYER](#zthe-relay-layer)
  - [( a ) The Packet](#athe-packet)
  - [( b ) Carrying](#bcarrying)
  - [( ú ) Receipts](#úreceipts)
  - [( u ) Flood Control](#uflood-control)
  - [( ù ) Lookup and Friend Requests](#ùlookup-and-friend-requests)
  - [( g ) Bonds and Your Part — INFORMATIVE](#gbonds-and-your-partinformative)
- [( I ) THE TALK LAYER](#ithe-talk-layer)
  - [( a ) The Move](#athe-move)
  - [( b ) The Seal](#bthe-seal)
  - [( ú ) Time of a Node](#útime-of-a-node)
  - [( u ) Kinds of Moves](#ukinds-of-moves)
  - [( ù ) Hand-in, Receipts and Rulings](#ùhand-in-receipts-and-rulings)
  - [( g ) Other Rules](#gother-rules)
  - [( d ) Out of Sync](#dout-of-sync)
- [( Í ) THE FOLD](#íthe-fold)
  - [( a ) The State of a Talk](#athe-state-of-a-talk)
  - [( b ) Order and Effect](#border-and-effect)
  - [( ú ) Cases](#úcases)
- [( K ) LIMITS AND SUPPORT](#klimits-and-support)
  - [( a ) Limits](#alimits)
  - [( b ) Support](#bsupport)
- [( L ) PRONO — INFORMATIVE](#lpronoinformative)
- [( M ) GLOSSARY](#mglossary)
- [( N ) REFERENCES](#nreferences)

## ( b ) List of Tables

- [Table B-1 Parts of the Pronal Network](#table-b-1)
- [Table Ú-1 Identifiers](#table-ú-1)
- [Table Ú-2 Types of Talks](#table-ú-2)
- [Table U-1 Records of the Pronal Network](#table-u-1)
- [Table U-2 States of a Friendship](#table-u-2)
- [Table Ù-1 Members of a Certificate](#table-ù-1)
- [Table Ù-2 Signed Statements of the Pronal Network](#table-ù-2)
- [Table G-1 Status Codes](#table-g-1)
- [Table G-2 Endpoints](#table-g-2)
- [Table G-3 Fields of a Message](#table-g-3)
- [Table G-4 System Types](#table-g-4)
- [Table G-5 Members of the State](#table-g-5)
- [Table G-6 Members of a Talk in the State](#table-g-6)
- [Table D-1 Events from an Application](#table-d-1)
- [Table D-2 Events from the Pronal Network](#table-d-2)
- [Table E-1 Events of a Call Between Two Users](#table-e-1)
- [Table E-2 Actions of talks/sfu.php](#table-e-2)
- [Table E-3 Tracks](#table-e-3)
- [Table E-4 Events of a Call in a Group](#table-e-4)
- [Table E-5 Messages of gcall_mls](#table-e-5)
- [Table Ż-1 Messages of pronal://](#table-ż-1)
- [Table Ż-2 Messages of the Node of a Device](#table-ż-2)
- [Table Ż-3 State of the Node](#table-ż-3)
- [Table Ż-4 Messages over Bluetooth Low Energy](#table-ż-4)
- [Table Z-1 Layout of a Packet](#table-z-1)
- [Table I-1 Members of a Move](#table-i-1)
- [Table I-2 Kinds of Moves](#table-i-2)
- [Table Í-1 Cases of the Fold](#table-í-1)
- [Table K-1 Limits](#table-k-1)
- [Table K-2 Support](#table-k-2)

## ( ú ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the PRONAL NETWORK STANDARD. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document. v31n is not related to non official platforms, services, people, & ‘oc genus omne in any way or form. Textual, programmatic, and media content published across these services is made available under the terms of the v31n license. No statutory copyright, proprietary monopoly, or reservation of rights subsists in these works. Any person is granted full, unconditional, and irrevocable permission to access, copy, modify, adapt, distribute, or incorporate all materials for any purpose, without formality, fee, or prior authorization. The constitutive Latin charter and legal terms of this declaration are recorded in the system registry and resp. places.

## ( u ) Foreword

This document has been published to define the PRONAL NETWORK STANDARD, the telecommunications database and system of Prinz and the way in which an application talks with it. It applies to every person dealing with Prinzian systems and with native systems of v31n.

The network is defined in § ( B ). Its part ( a ) sets out the layers, its part ( b ) the parts of the pronal network, its part ( ú ) the admission of an application, and its part ( u ) the mesh. § ( Ú ) sets out the time and the identifiers by which records are named, and the types of talks. § ( U ) sets out how the database is kept: its records, deletion, encryption, the search index, friendships and NEGL, and attachments. § ( Ù ) sets out the bearer token by which an application authenticates, describes, informatively, the SSO login of the V31N Portal, and sets out the certificate by which a node is known to other nodes. § ( G ) sets out the requests: their form, the status codes, the endpoints, the message, the system messages, the state and Emendetur blocks. § ( D ) sets out the pushes, and § ( E ) the calls and their encryption. § ( Ż ) sets out the links between nodes, § ( Z ) the relay layer that carries sealed packets, § ( I ) the talk layer that makes and seals moves, and § ( Í ) the fold by which every node computes the state of a talk. § ( K ) sets out the limits and what an application shall and may support. § ( L ) describes, informatively, how Prono applies this document. § ( M ) gives the terms used in this document, and § ( N ) the documents to which it refers.

P.K.

## ( ù ) Scope and Purpose

The PRONAL NETWORK STANDARD, as defined by v31n, applies to the pronal network and to every pronal application. Users of different applications talk with each other as if they used one. A move made in the mesh reaches the users of applications that are not nodes when it reaches the pronal network.

This document defines what the pronal network keeps, what an application sends, and what the pronal network answers and pushes, and what nodes send to each other. How an application looks, and how it shows what it receives, is chosen by its developer.

## ( g ) Notation

A reference to a part of this document gives the sign § followed by the division and the part, as in § ( B/( a ) ), which denotes ( a ) of ( B ). Tables are numbered by division, as in Table B-1 and Table G-2.

Names of records, fields, actions and events are written as the pronal network uses them, as identification_numero_talkes. Everything built for this document bears these names.

The requirements of this document are stated with shall, and each binds unless this document states otherwise. Should denotes a recommendation, which does not bind, and may denotes a permission. A part marked INFORMATIVE gives guidance and binds no one. The pronal network is described as it is; shall, should and may bind applications.

# ( B ) THE PRONAL NETWORK

## ( a ) Layers

This document names four layers, each distinct:

1. Prinzian denotes the origin, the jurisdiction and the standards of v31n. the pronal network and the V31N Portal are both Prinzian, and each is a service of its own.
2. The V31N Portal (portal.v31null.de) is the service of sign-on and eID. The pronal network uses it for the login of users. Its notices, rules and desks apply to the Portal alone.
3. The pronal network is the telecommunications database and system: the users, the talks, the stored messages and their keys, the search index, the friendships and NEGL, the attachments and the calls. The pronal network answers at pronal.v31null.de.
4. A pronal application is any programme, official or third-party, that talks with the pronal network and authenticates through the V31N Portal. Prono is one pronal application.

## ( b ) Parts of the Pronal Network

The pronal network holds what a user needs to talk with a user of another application. Table B-1 presents its parts.

<table id="table-b-1">
<caption>Table B-1 Parts of the Pronal Network</caption>
<thead>
<tr><th>Part</th><th>See</th></tr>
</thead>
<tbody>
<tr><td>Messages: send, get, edit, delete, like, reply, read, search</td><td>§ ( G/( u ) )</td></tr>
<tr><td>Direct messages</td><td>Table Ú-2</td></tr>
<tr><td>Groups, their members and their owner</td><td>§ ( Ú/( ú ) )</td></tr>
<tr><td>Friendships and NEGL</td><td>§ ( U/( ù ) )</td></tr>
<tr><td>Name of a user: name_useres and seperator_useres</td><td>Table U-1</td></tr>
<tr><td>Current avatar of a user and current icon of a group</td><td>§ ( G/( g ) )</td></tr>
<tr><td>Status of a user: online or offline</td><td>§ ( D )</td></tr>
<tr><td>Attachments, and files uploaded by users that are shown as embeds</td><td>§ ( U/( g ) )</td></tr>
<tr><td>Calls</td><td>§ ( E )</td></tr>
<tr><td>The mesh: certificates, moves and their hand-in, rulings and checkpoints</td><td>§ ( B/( u ) )</td></tr>
</tbody>
</table>

## ( ú ) Admission

An application is admitted to the pronal network by MEF, on request to mef@v31null.de. Admission gives the application its own client of the V31N Portal (§ ( Ù )), names the origins from which a browser application runs, and names, for an Android application, its package and the SHA-256 digest of its signing certificate. The pronal network signs the list of admitted applications (Table Ù-2), and nodes pass it on to each other (§ ( Ż/( a ) )). The pronal network answers browsers from the origins of admitted applications alone. The pronal network accepts the tokens of admitted clients alone, and only while their admission stands.

## ( u ) The Mesh

The devices of users talk with each other in a mesh, so that users talk while the pronal network or the internet is out of reach, the load on the pronal network is lessened, and devices carry each other's traffic. Every device that joins adds routes, copies and capacity.

1. A node is a pronal application that speaks pronal:// (§ ( Ż )). A node is a native application; a browser application is not a node. A device that cannot link across the internet is no node (§ ( Ż/( ù ) )).
2. A node is known to other nodes by its certificate (§ ( Ù/( ú ) )). Every node holds a certificate, and so a login through the V31N Portal and a pin.
3. A node has two layers. The relay layer (§ ( Z )) carries sealed packets and knows nothing of talks. The talk layer (§ ( I ), § ( Í )) makes, seals, opens and folds moves. A relay-only node runs the relay layer alone.
4. Inside a talk, a node acts without asking the pronal network first. It checks each move against this document itself, applies it, and tells the pronal network later, when the pronal network is in reach (§ ( I/( ù ) )). Nodes take on tasks of the pronal network.
5. The pronal network is the referee. It records every move, and rules where a move breaks this document; its ruling stands. The V31N Portal and the database of the pronal network are the absolute, from MEF.
6. What a node has witnessed cannot be changed by another node (§ ( Í/( b ) )). This binds the nodes, not the pronal network.
7. The mesh carries talks of type 0 and 1 (Table Ú-2). The requests of § ( G ) and the pushes of § ( D ) stay whole, and applications that are not nodes keep working.

Nothing in the mesh is mined, counted as a token, voted on or forked. Each number of the mesh is measured, taken from a limit of this document, or signed by the pronal network, except one: the window of 7 days (Table K-1), which is a choice.

# ( Ú ) IDENTIFIERS AND TIME

## ( a ) Time

Every date and time of the pronal network is a sort key of psí1103r1 (PS/Í-1103), sixteen characters long, as in 20260612A0001001, and is written big_date. The pronal network stores and compares sort keys as text, and their text order is their order in time. Every record of the pronal network uses this clock. A node counts its time as set out in § ( I/( ú ) ).

## ( b ) Identifiers

Table Ú-1 presents the identifiers of the pronal network.

<table id="table-ú-1">
<caption>Table Ú-1 Identifiers</caption>
<thead>
<tr><th>Name</th><th>Form</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>pin</td><td>15 digits</td><td>The Platformwise Identification № of a user, and the only true identifier of a user. Compared whole, with =.</td></tr>
<tr><td>pinpriv</td><td>15 digits</td><td>The private key of a login. The pronal network keeps it to itself.</td></tr>
<tr><td>System pin</td><td>000000000000000</td><td>Reserved for the pronal network itself. The pronal network refuses every request that names it as a sender, a friend, a member or a target.</td></tr>
<tr><td>Orphan pin</td><td>392000000001627</td><td>The pin shown in place of every orphaned account (§ ( U/( b ) )).</td></tr>
<tr><td>prigotov</td><td>5 characters of A–Z, a–z, 0–9</td><td>The random part of a min.</td></tr>
<tr><td>min</td><td>big_date + pin + prigotov, 36 characters</td><td>The Message Identification №. Immutable. It begins with big_date, so min sort in order of time. A move is named by a min (§ ( I/( a ) )).</td></tr>
<tr><td>identification_numero_talkes</td><td>Integer, 1 000 000 000 to 9 999 999 999 999 999</td><td>The identifier of a talk, chosen at random by the pronal network, or by the node that makes the talk in the mesh (§ ( I/( g ) )). Sent as text.</td></tr>
<tr><td>uuid</td><td>Up to 64 characters</td><td>The identifier of an attachment. A node makes it as 64 hexadecimal digits at random.</td></tr>
<tr><td>Device key</td><td>P-256 key pair</td><td>The key of a node for one pinpriv (§ ( Ù/( ú ) )). It signs alone; every key agreement uses a throwaway key that the device key signs. Its public half is written as an uncompressed point of 65 bytes.</td></tr>
<tr><td>Address</td><td>pronal:// + 64 hexadecimal digits</td><td>The address of a node for one device key: the SHA-256 of the public half of the device key, in lower case.</td></tr>
<tr><td>serial</td><td>Integer, counted up by the pronal network</td><td>The identifier of a certificate.</td></tr>
<tr><td>tag</td><td>16 bytes</td><td>The opaque name of a talk and a day on a packet (§ ( I/( b ) )).</td></tr>
<tr><td>arrival</td><td>Integer, counted up per talk</td><td>The arrival number: the order in which the messages and moves of a talk arrived, at the pronal network or at a node.</td></tr>
</tbody>
</table>

An application may make the min of its own message; otherwise the pronal network makes it. The pronal network accepts a min sent through § ( G ) only where its pin is that of the sender and its big_date lies within 60 seconds of the time of the pronal network. A node makes the min of every move it makes, and the pronal network accepts a move from the mesh by § ( I/( ú ) ).

The pronal network and every node compare a min byte by byte, never by a collation.

Where the pronal network sends the pin of an orphaned account to an application, it sends the orphan pin in its place. An address and a tag carry no pin.

## ( ú ) Types of Talks

The pronal network holds every conversation as a talk, whose type is held in type_talkes. Table Ú-2 presents the types.

<table id="table-ú-2">
<caption>Table Ú-2 Types of Talks</caption>
<thead>
<tr><th>type_talkes</th><th>Talk</th><th>Membership and read state</th></tr>
</thead>
<tbody>
<tr><td>0</td><td>Direct messages between two users</td><td>direct_talk_map: the smaller pin in pin_smaller, the larger in pin_larger; read state in luimme_smaller and luimme_larger</td></tr>
<tr><td>1</td><td>Group</td><td>group_talk_members, with luimme per member</td></tr>
</tbody>
</table>

Two users share one talk of type 0 at most. Every member of a group may send, add members, rename the group and leave it. The owner of a group is the member whose row holds the owner's own pin in added_by_pin. The owner alone may remove a member or hand the ownership to another member. The name of a group is the channel_name of its talk.

The read state luimme holds the min of the last message that the user has seen, and beside it the arrival number of that message, which is raised and never lowered. Every message from others whose arrival number is greater is unread.

# ( U ) THE DATABASE

## ( a ) Records

The pronal network keeps one database, and applications reach it through § ( G ), § ( D ) and hand-in (§ ( I/( ù ) )) alone. Table U-1 presents the records of the pronal network.

<table id="table-u-1">
<caption>Table U-1 Records of the Pronal Network</caption>
<thead>
<tr><th>Record</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>users</td><td>The user: pin, pinpriv, name_useres, seperator_useres, big_date, orphan, burnt.</td></tr>
<tr><td>portal_links</td><td>The link of a subject of the V31N Portal (sub) to a pin.</td></tr>
<tr><td>talks</td><td>identification_numero_talkes, type_talkes, channel_name, big_date.</td></tr>
<tr><td>direct_talk_map</td><td>The talk of type 0 of each pair of users, and their read state.</td></tr>
<tr><td>group_talk_members</td><td>The members of each group, added_by_pin, big_date and luimme.</td></tr>
<tr><td>messages</td><td>min, identification_numero_talkes, sends, content, system_type, big_date, modify, big_date_modifyes, poistettu, palvelimeltasaapuvanvelvoittavanviestintyyliluettelo, reply_min, liktcount, arrival.</td></tr>
<tr><td>message_likes</td><td>The count of likes of each user on a message.</td></tr>
<tr><td>message_mentions</td><td>The users named in a message by @ and their pin.</td></tr>
<tr><td>message_keys</td><td>The wrapped data key of each talk and day (§ ( U/( ú ) )).</td></tr>
<tr><td>search_keys, search_tokens</td><td>The search index (§ ( U/( u ) )).</td></tr>
<tr><td>attachments</td><td>The attachments of a message (§ ( U/( g ) )), with the hash of each file.</td></tr>
<tr><td>friendships</td><td>Friendships and NEGL (§ ( U/( ù ) )).</td></tr>
</tbody>
</table>

For the mesh, the pronal network also keeps:

1. the certificates it issued, by serial, and the list of certificates ended early (§ ( Ù/( ú ) ));
2. the moves handed in, each as its original bytes with its signature, so that every ruling can be proved;
3. the rulings and the checkpoints of each talk (§ ( I/( ù ) ));
4. the secret of each talk from which its day keys are derived (§ ( I/( b ) ));
5. the arrival number of each talk, and beside each luimme the arrival number of the message it names.

Every column that holds or compares a min holds it in a binary collation.

## ( b ) Deletion

A deleted message stays in the database with poistettu = 1. The pronal network treats its content as void, and its answers carry messages with poistettu = 0.

The deletion of an account orphans it: the row of the user is scrubbed, and the pin is kept so that the records that name it stay whole. Every orphaned pin is shown as the orphan pin (§ ( Ú/( b ) )).

A deletion holds on conforming applications alone; copies already passed on stay where they are. In the mesh, every min holds the pin of its maker, so the orphaning of an account is likewise a request that conforming applications obey.

## ( ú ) Encryption

The pronal network encrypts the content of every message at rest, as ENC2. The encryption is held by the server: the pronal network keeps the keys and reads the content.

1. Each talk has one data key per day, the day being the first eight characters of the min. The data key is 32 random bytes.
2. The data key is wrapped by a master key with AES-256-GCM and the additional data made of DEK, the id of the data key, the talk and the day, joined by |, and kept in message_keys.
3. The content is sealed with the data key with AES-256-GCM, a random nonce of 12 bytes and the additional data made of MSG, the id of the data key and the min, joined by |. It is stored as the byte 01, the text ENC2:, the id of the data key, a colon, and the base64 of the nonce, the tag and the ciphertext.

The min is bound into the seal, so a content opens in its own message alone.

The content of a move handed in is stored as ENC2 in the same way. The data keys and master keys of ENC2 never leave the pronal network; the day keys of the mesh (§ ( I/( b ) )) are separate keys.

## ( u ) Search Index

The pronal network searches by hashes of words. Each talk has a search key, wrapped like a data key and kept in search_keys. The words of a message are folded to Unicode normalisation form KC and to lower case, cut to 64 characters, and each is kept as the first 16 bytes of its HMAC-SHA-256 under the search key, in search_tokens. A search folds and hashes its words the same way. The words of a move handed in are indexed the same way.

## ( ù ) Friendships and NEGL

Each pair of users has one row in friendships at most, with the smaller pin in asker_user and the larger in askt_user. The pin that asked is held in requester. Table U-2 presents the states.

<table id="table-u-2">
<caption>Table U-2 States of a Friendship</caption>
<thead>
<tr><th>status</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>TBD</td><td>Asked, awaiting an answer</td></tr>
<tr><td>AVAIL</td><td>Friends</td></tr>
<tr><td>NEG</td><td>Refused</td></tr>
<tr><td>NEGL</td><td>Blocked</td></tr>
</tbody>
</table>

NEGL binds both users, whichever of them blocked. It bars them from sending to each other, asking each other for friendship, calling each other and sharing a group. The pronal network enforces it on every request and on every move handed in. Every node enforces it on every move, by the NEGL it knows from the state. A NEGL set on a node applies there at once, and reaches the pronal network by users.php?action=block when the pronal network is in reach; the pronal network's record of it binds everyone.

## ( g ) Attachments

An attachment belongs to a message of its sender and is uploaded after that message, which is sent with defer and pushed by publish (Table G-2). It is kept unencrypted in /img/filesuserses/users/, in the folder of the sender's pin, in its folder uploads/. The file is named by its uuid, file_sequence and prigotov, joined by _, with the extension of its original name in lower case. A video has a poster beside it, named the same with _poster added and the extension jpg. The url and the poster of an attachment (Table G-3) are these paths on pronal.v31null.de, with the orphan pin in place of the pin of an orphaned sender. The pronal network takes the type of a file from its bytes. Images, video and audio are media, and The pronal network accepts these media types: image/png, image/jpeg, image/gif, image/webp, image/avif, image/bmp, video/mp4, video/webm, video/ogg, audio/mpeg, audio/mp3, audio/ogg, audio/wav, audio/x-wav, audio/webm, audio/mp4, audio/aac and audio/flac. An image wider or higher than 1920 pixels is scaled down to fit.

The pronal network keeps the SHA-256 of each file in hash. In the mesh:

1. The sending node does what the pronal network does above before it hashes a file: it takes the type from the bytes, refuses types outside those above, scales an image wider or higher than 1920 pixels down to fit, and keeps to Table K-1.
2. The node names the file by a uuid it makes at random (Table Ú-1) and gives its hash in the send move (Table I-1). The poster of a video is a second file with its own hash, made by the sending node.
3. The file is sealed under the seal key of its day and carried in a packet of its own (§ ( I/( b ) )); relays check it by the hash of its sealed bytes.
4. The pronal network stores a conforming file as it was handed in and does not encode it again, so that its bytes keep their hash. The pronal network pushes the message to applications that are not nodes once every file it names has been handed in.
5. The url stays a path on pronal.v31null.de. A node that holds the file serves it first.

# ( Ù ) AUTHENTICATION

## ( a ) The Bearer Token

An application authenticates the user through the V31N Portal, by OpenID Connect with PKCE, as its own client admitted by MEF (§ ( B/( ú ) )):

1. The application signs the user in at the V31N Portal and receives an ID token for its client.
2. The application sends the ID token by POST to auth/token.php of the pronal network.
3. The pronal network checks the signature of the token against the keys of the V31N Portal, that its issuer is the V31N Portal, that its audience is an admitted client, and that it is still valid. It finds the pin of the subject in portal_links. It answers 401 where a check fails or the subject lacks a linked pin.
4. The pronal network answers with token, the bearer token, and expires, the big_date of its expiry.
5. The application sends the bearer token in the field Authorization, as Bearer and the token, with every request of § ( G ) but mesh/reach.php, which takes none.

The pronal network keeps only the hash of a bearer token. A bearer token ends at its expiry, at auth/logout.php, at auth/logout_all.php for all tokens of the user, and when the account is burnt or orphaned. An application shall keep the bearer token to itself and send it to pronal.v31null.de alone. Other nodes receive the certificate, never the token.

The V31N Portal tells the pronal network, by a signed push, when the access of a client ends, when the user logs out at the V31N Portal, and when the account ends. The pronal network then ends the bearer tokens of that user for that client, or all of them. The push is a POST to auth/portal_push.php whose body is a JSON object with kind (client, logout or account), sub and client_id, and whose field X-Pronal-Signature holds the HMAC-SHA256 of the body, in hexadecimal, under a secret that the V31N Portal and the pronal network share.

## ( b ) The V31N Portal — INFORMATIVE

The SSO login belongs to the V31N Portal, the service of sign-on of v31null. Every user signs in, and signs up, on the SSO page of the application at the V31N Portal. The V31N Portal gives the application a refresh token that lasts 90 days and is renewed at each use, so the user signs in once.

## ( ú ) The Certificate

A node holds one device key for each pinpriv that uses it on the device. The device key shall be drawn inside a key store of the device and shall never leave it: on Android, Android Keystore; on Windows, the TPM, or, where the PC has none, the software key store with the key marked as not exportable. Applications of one account on one device share its device key and its node; two accounts on one device have two device keys and two addresses. The device key is bound to the pinpriv, never made from it, and the pinpriv appears in no certificate, packet, move or address. The server node alone keeps its device key otherwise (§ ( Ż/( g ) )).

A node sends the public half of its device key, with its bearer token and a key proof, to mesh/certificate.php. The key proof is a statement signed by the device key itself, with kind key_proof, key and big_date; The pronal network refuses it where its big_date lies more than 60 seconds from the time of the pronal network. The pronal network answers with a certificate, whose members Table Ù-1 presents, and with the statement of the online key that signed it.

<table id="table-ù-1">
<caption>Table Ù-1 Members of a Certificate</caption>
<thead>
<tr><th>Member</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>kind</td><td>certificate</td></tr>
<tr><td>serial</td><td>The serial of the certificate</td></tr>
<tr><td>key</td><td>The base64 of the public half of the device key</td></tr>
<tr><td>pin</td><td>The pin of the user</td></tr>
<tr><td>client</td><td>The client of the V31N Portal to which the bearer token belongs</td></tr>
<tr><td>from</td><td>The big_date of issue</td></tr>
<tr><td>until</td><td>from, added 7 days by psí1103r1</td></tr>
</tbody>
</table>

1. The pronal network issues one certificate per bearer token. A certificate ends at until, or when its bearer token ends (§ ( Ù/( a ) )), so the pushes of the V31N Portal and the end of an admission end certificates too.
2. Each contact with the pronal network may renew the certificate, but the pronal network issues a new certificate only to a node that has taken the current rulings on the talks of its user (§ ( I/( ù ) )).
3. To check a move, a node checks its certificate against the keys of the pronal network, reads the pin and checks that the pin is a member of the talk. It needs no list of the devices of a user.

The pronal network signs with an online key, a P-256 key pair. The public half of the online key is signed, with an end date, by the root key of the pronal network. The root key is kept on no server, and its public half ships inside every node. The online key is kept as the master keys of ENC2 are kept, outside the database. Wherever a certificate is given — in the answer of mesh/certificate.php, in a move, in a receipt and in the peer proof — the statement of the online key that signed it is given with it, so that a node out of reach of the pronal network can check a certificate signed by a newer online key.

Every signature of this document is ECDSA over P-256 with SHA-256 (FIPS 186-5), written as r and s of 32 bytes each, 64 bytes in that order. Every signed statement is the bytes of a JSON object in UTF-8, given as a JSON object with two members: signed, the base64 of those bytes, and sig, the base64 of the signature. A reader checks sig over the bytes as received before it parses them, and nobody encodes the bytes again. Every signed statement names what it is in its member kind: online_key, certificate, key_proof, beacon, list, pronal_receipt, ruling, checkpoint, name_card, receipt (§ ( Z/( ú ) )), peer_proof (§ ( Ż/( b ) )), radio_proof (§ ( Ż/( u ) )), local_ping (§ ( Ż/( d ) )), reach (§ ( Ż/( g ) )), admitted (§ ( B/( ú ) )), agreement_key (§ ( I/( b ) )), or a kind of Table I-2 for a move. A reader refuses a statement whose kind is not the one it expects. Table Ù-2 presents the signed statements of the pronal network.

<table id="table-ù-2">
<caption>Table Ù-2 Signed Statements of the Pronal Network</caption>
<thead>
<tr><th>Statement</th><th>Members</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>Online key</td><td>kind, key, until; signed by the root key</td><td>The key by which the pronal network signs, and the end of its use</td></tr>
<tr><td>Certificate</td><td>Table Ù-1</td><td>A device key bound to a pin</td></tr>
<tr><td>Beacon</td><td>kind, big_date, list</td><td>The time of the pronal network and the number of the current list, made on request at every contact</td></tr>
<tr><td>List</td><td>kind, number, part, parts, serials</td><td>The serials of certificates ended before their until. A serial leaves the list once its until has passed, so the list holds no more than 7 days of endings. A list longer than a control message (Table K-1) is given in parts, each signed with its number and its part.</td></tr>
<tr><td>Pronal network's receipt</td><td>kind, big_date, hashes</td><td>The hashes of the packets that the pronal network now holds (§ ( I/( ù ) )). It names no id, since an id holds a pin and receipts are passed on to strangers.</td></tr>
<tr><td>Ruling</td><td>kind, number, talk, id, cause, big_date; to join two talks, into in place of talk</td><td>A move struck, and why; or a talk joined into another (§ ( I/( ù ) ))</td></tr>
<tr><td>Checkpoint</td><td>kind, talk, id, members, owner, name, big_date</td><td>The state of a talk after the move id (§ ( Í/( a ) ))</td></tr>
<tr><td>Name card</td><td>kind, pin, name_useres, seperator_useres, big_date</td><td>The name of a user at a time (§ ( Z/( ù ) ))</td></tr>
<tr><td>Reach</td><td>kind, fresh, big_date; from the server node, seen; from a peripheral, reached and cert</td><td>The answer to a tester of § ( Ż/( g ) )</td></tr>
<tr><td>Admitted applications</td><td>kind, number, big_date, apps, each with client, origins, and android, a list of package and sha256, the hexadecimal SHA-256 of its signing certificate</td><td>The applications admitted to the pronal network (§ ( B/( ú ) )), numbered upward at every change</td></tr>
</tbody>
</table>

In every handshake two nodes compare the numbers of their lists, and the node with the lower number takes the higher list. They do the same with their lists of admitted applications, by admitted (Table Ż-1). A node refuses a certificate whose serial stands on the list it holds. A node that the list has not reached trusts an ended certificate until its until; that is the cost of the window.

# ( G ) THE REQUESTS

## ( a ) Form of a Request

Every request is made over HTTPS to https://pronal.v31null.de/API1/ and the path of Table G-2. API1 names the first revision of the requests. Where an endpoint takes an action, the action is the query parameter action. A request that reads is made by GET with its fields in the query. A request that writes is made by POST with its fields as application/x-www-form-urlencoded, or as multipart/form-data where it carries a file.

Every answer is JSON in UTF-8. An error is answered with its status code and an object whose member error holds the text of the error.

## ( b ) Status Codes

Table G-1 presents the status codes of the answers and their meaning in this document. They are those of RFC 9110.

<table id="table-g-1">
<caption>Table G-1 Status Codes</caption>
<thead>
<tr><th>Code</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>200</td><td>Done</td></tr>
<tr><td>400</td><td>A field is missing or out of its limits</td></tr>
<tr><td>401</td><td>The bearer token is missing, wrong or ended</td></tr>
<tr><td>403</td><td>The user lacks the right, as under NEGL or outside the talk</td></tr>
<tr><td>404</td><td>The user, talk or message is unknown</td></tr>
<tr><td>405</td><td>The method differs from Table G-2</td></tr>
<tr><td>413</td><td>The file exceeds its limit</td></tr>
<tr><td>429</td><td>Too many requests</td></tr>
<tr><td>502</td><td>A service behind the pronal network failed</td></tr>
</tbody>
</table>

## ( ú ) Endpoints

Table G-2 presents the endpoints of the pronal network.

<table id="table-g-2">
<caption>Table G-2 Endpoints</caption>
<thead>
<tr><th>Path</th><th>Method</th><th>Fields</th><th>Does</th></tr>
</thead>
<tbody>
<tr><td>auth/token.php</td><td>POST</td><td>id_token</td><td>Gives a bearer token (§ ( Ù ))</td></tr>
<tr><td>auth/logout.php</td><td>POST</td><td></td><td>Ends this bearer token</td></tr>
<tr><td>auth/logout_all.php</td><td>POST</td><td></td><td>Ends every bearer token of the user</td></tr>
<tr><td>state.php</td><td>GET</td><td></td><td>Gives the state (§ ( G/( g ) ))</td></tr>
<tr><td>messages.php?action=send</td><td>POST</td><td>talk_id, content, min, reply_min, style, nonce, defer</td><td>Sends a message; with defer 1 it is pushed at publish</td></tr>
<tr><td>messages.php?action=publish</td><td>POST</td><td>min</td><td>Pushes a deferred message once its attachments are uploaded</td></tr>
<tr><td>messages.php?action=get</td><td>GET</td><td>talk_id, limit, before, since</td><td>Gives the latest messages of a talk, in order of time</td></tr>
<tr><td>messages.php?action=get_one</td><td>GET</td><td>min</td><td>Gives one message, with edit_content (§ ( G/( d ) ))</td></tr>
<tr><td>messages.php?action=get_around</td><td>GET</td><td>talk_id, min</td><td>Gives the messages before and after one message</td></tr>
<tr><td>messages.php?action=edit</td><td>POST</td><td>min, content</td><td>Edits a message of the user</td></tr>
<tr><td>messages.php?action=delete</td><td>POST</td><td>min</td><td>Deletes a message of the user</td></tr>
<tr><td>messages.php?action=like</td><td>POST</td><td>min, delta (1 or −1)</td><td>1 likes a message once more; −1 takes back all likes of the user on it</td></tr>
<tr><td>messages.php?action=get_liked</td><td>GET</td><td>talk_id</td><td>Gives the liked messages of a talk</td></tr>
<tr><td>messages.php?action=read</td><td>POST</td><td>talk_id, min</td><td>Sets luimme</td></tr>
<tr><td>messages.php?action=search</td><td>GET</td><td>talk_id, q, from, has, mentions, before</td><td>Searches a talk (§ ( U/( u ) ))</td></tr>
<tr><td>talks/create_dm.php</td><td>POST</td><td>target_pin</td><td>Gives the talk of type 0 with a user, and makes it on first use</td></tr>
<tr><td>talks/create_group.php</td><td>POST</td><td>name, members</td><td>Makes a group; the maker is its owner</td></tr>
<tr><td>talks/get_my_groups.php</td><td>GET</td><td></td><td>Gives the groups of the user</td></tr>
<tr><td>talks/get_members.php</td><td>GET</td><td>talk_id</td><td>Gives the members of a group and its owner</td></tr>
<tr><td>talks/add_members.php</td><td>POST</td><td>talk_id, pins</td><td>Adds members to a group, or makes a group from a talk of type 0</td></tr>
<tr><td>talks/remove_member.php</td><td>POST</td><td>talk_id, target_pin</td><td>Removes a member; owner only</td></tr>
<tr><td>talks/set_owner.php</td><td>POST</td><td>talk_id, target_pin</td><td>Hands the ownership to a member; owner only</td></tr>
<tr><td>talks/update_group.php</td><td>POST</td><td>talk_id, name</td><td>Renames a group</td></tr>
<tr><td>talks/leave.php</td><td>POST</td><td>talk_id</td><td>Leaves a group</td></tr>
<tr><td>users.php?action=me</td><td>GET</td><td></td><td>Gives the user's own pin, name, seperator and avatar</td></tr>
<tr><td>users.php?action=get</td><td>GET</td><td>pin</td><td>Gives the pin, name, seperator and avatar of a user</td></tr>
<tr><td>users.php?action=search</td><td>GET</td><td>q</td><td>Finds users by name</td></tr>
<tr><td>users.php?action=change_name</td><td>POST</td><td>name</td><td>Changes the name; draws a new seperator_useres where another user holds the same name and seperator</td></tr>
<tr><td>users.php?action=friends</td><td>GET</td><td></td><td>Gives the friendships of the user</td></tr>
<tr><td>users.php?action=mutual</td><td>GET</td><td>pin</td><td>Gives the friends that the user and that user share</td></tr>
<tr><td>users.php?action=friend_request</td><td>POST</td><td>pin</td><td>Asks a user for friendship</td></tr>
<tr><td>users.php?action=friend_request_by_name</td><td>POST</td><td>name, seperator</td><td>Asks a user for friendship by name</td></tr>
<tr><td>users.php?action=friend_response</td><td>POST</td><td>pin, accept (1 or 0)</td><td>Answers a request</td></tr>
<tr><td>users.php?action=retract_friend_request</td><td>POST</td><td>pin</td><td>Takes back a request</td></tr>
<tr><td>users.php?action=remove_friend</td><td>POST</td><td>pin</td><td>Ends a friendship</td></tr>
<tr><td>users.php?action=block</td><td>POST</td><td>pin</td><td>Sets NEGL</td></tr>
<tr><td>users.php?action=unblock</td><td>POST</td><td>pin</td><td>Lifts NEGL</td></tr>
<tr><td>users.php?action=socket_pass</td><td>POST</td><td></td><td>Gives a pass for § ( D )</td></tr>
<tr><td>attachments.php?action=upload</td><td>POST</td><td>talk_id, min, the file, width, height</td><td>Uploads an attachment to a message of the user</td></tr>
<tr><td>attachments.php?action=get</td><td>GET</td><td>min</td><td>Gives the attachments of a message</td></tr>
<tr><td>talks/ice.php</td><td>POST</td><td></td><td>Gives the ICE servers for a call, and the STUN servers of § ( Ż/( ù ) )</td></tr>
<tr><td>talks/sfu.php</td><td>POST</td><td>action (new, tracks, renegotiate, close), session, seal, body</td><td>Opens and runs a session of a call in a group (§ ( E/( b ) ))</td></tr>
<tr><td>talks/call_log.php</td><td>POST</td><td>peer_pin, dur</td><td>Records a call between two users as a system message</td></tr>
<tr><td>mesh/certificate.php</td><td>POST</td><td>key, proof, rulings</td><td>Gives a certificate for a device key (§ ( Ù/( ú ) ))</td></tr>
<tr><td>mesh/beacon.php</td><td>GET</td><td></td><td>Gives the beacon, the list and the online key (Table Ù-2)</td></tr>
<tr><td>mesh/day_keys.php</td><td>GET</td><td>talk_id</td><td>Gives the day keys of a talk from today through the window (§ ( I/( b ) ))</td></tr>
<tr><td>mesh/talk.php</td><td>POST</td><td>talk_id, type, day_keys</td><td>Hands the pronal network a talk made in the mesh and the day keys its maker drew (§ ( I/( g ) ))</td></tr>
<tr><td>mesh/hand_in.php</td><td>POST</td><td>packets, receipts</td><td>Takes packets (§ ( I/( ù ) )), and answers with the pronal network's receipt</td></tr>
<tr><td>mesh/moves.php</td><td>GET</td><td>talk_id, after</td><td>Gives the packets of the moves of a talk whose arrival number is greater than after, each with its arrival number</td></tr>
<tr><td>mesh/rulings.php</td><td>GET</td><td>after, talk_id</td><td>Gives the rulings numbered after after on the talks of the user, and the newest checkpoint of the talk</td></tr>
<tr><td>mesh/name_card.php</td><td>GET</td><td>pin</td><td>Gives the name card of a user (Table Ù-2)</td></tr>
<tr><td>mesh/admitted.php</td><td>GET</td><td></td><td>Gives the list of admitted applications (Table Ù-2)</td></tr>
<tr><td>mesh/reach.php</td><td>GET</td><td></td><td>Gives stun, two STUN servers for reach (§ ( Ż/( g ) )); takes no bearer token</td></tr>
<tr><td>mesh/reach.php</td><td>POST</td><td>address, port</td><td>Passes the public address and port of a tester to the server node, and gives address and port of its port for reach (§ ( Ż/( g ) )); takes no bearer token</td></tr>
</tbody>
</table>

The endpoints of mesh/ are for nodes. In mesh/certificate.php, key is the base64 of the public half of the device key, proof the key proof as signed (§ ( Ù/( ú ) )), and rulings the number of the newest ruling the node has taken. In mesh/talk.php, day_keys is a JSON object whose members are the days and whose values are the base64 of their day keys. In mesh/hand_in.php, packets is a file whose content is packets one after another, each in the layout of Table Z-1, and receipts is a JSON array of the receipts the node holds for them, each as signed.

## ( u ) The Message

Table G-3 presents the fields of a message as the pronal network gives it, in an answer and in a push.

<table id="table-g-3">
<caption>Table G-3 Fields of a Message</caption>
<thead>
<tr><th>Field</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>min</td><td>Its identifier</td></tr>
<tr><td>sender</td><td>pin, name and seperator of the sender, and avatar, the path of the current avatar</td></tr>
<tr><td>content</td><td>The text, decrypted</td></tr>
<tr><td>mentions</td><td>The users named by @ and their pin</td></tr>
<tr><td>emend</td><td>The pins of the editors of its Emendetur blocks, or null (§ ( G/( d ) ))</td></tr>
<tr><td>system_type, system</td><td>The type of a system message and its data (§ ( G/( ù ) ))</td></tr>
<tr><td>big_date</td><td>When it was sent</td></tr>
<tr><td>arrival</td><td>Its arrival number at the pronal network</td></tr>
<tr><td>modified, modified_at</td><td>Whether and when it was edited</td></tr>
<tr><td>likes</td><td>liktcount</td></tr>
<tr><td>style</td><td>The style: align 1 left, 2 centre, 3 right, and the colours by position</td></tr>
<tr><td>reply_to, reply_void</td><td>The min to which it replies, and whether that message is deleted</td></tr>
<tr><td>attachments</td><td>Per attachment: uuid, name, size, type, width, height, url, poster, and hash, the SHA-256 of the file in hexadecimal</td></tr>
</tbody>
</table>

A user is named in a message by @ and the 15 digits of the pin. A reply carries reply_min, and each application shows the quoted message in its own way. A user may like a message any number of times and every like counts; an unlike takes back all likes of the user on the message. The pronal network gives the count of likes alone.

## ( ù ) System Messages

The pronal network alone writes system messages into its record, those of system_type 1 and higher, and their content is the JSON of their data. Where a member causes a system message by a move made in the mesh, the pronal network writes it from the member's signed move; the right to cause it lies with the member (Table I-2). Table G-4 presents the types.

<table id="table-g-4">
<caption>Table G-4 System Types</caption>
<thead>
<tr><th>system_type</th><th>Event</th></tr>
</thead>
<tbody>
<tr><td>0</td><td>Message of a user</td></tr>
<tr><td>1</td><td>Member added</td></tr>
<tr><td>2</td><td>Member removed</td></tr>
<tr><td>3</td><td>Member left</td></tr>
<tr><td>4</td><td>Member joined</td></tr>
<tr><td>5</td><td>Call</td></tr>
<tr><td>6</td><td>Call missed</td></tr>
<tr><td>7</td><td>Group renamed</td></tr>
<tr><td>8</td><td>Group icon changed</td></tr>
<tr><td>9</td><td>Group made</td></tr>
<tr><td>10</td><td>Owner changed</td></tr>
<tr><td>11</td><td>Friendship</td></tr>
<tr><td>12</td><td>Like</td></tr>
</tbody>
</table>

## ( g ) The State

The state is what an application shows beside the talks: the user, the people the user meets, the friendships and the talks of the user. An application takes it whole from state.php and keeps it current by the push state_patch (§ ( D )). Table G-5 presents its members.

<table id="table-g-5">
<caption>Table G-5 Members of the State</caption>
<thead>
<tr><th>Member</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>rev</td><td>The revision of the state</td></tr>
<tr><td>me</td><td>The pin of the user</td></tr>
<tr><td>people</td><td>Per pin: name and sep, the name_useres and seperator_useres of that user, avatar, the path of the current avatar, and status, the status of § ( D )</td></tr>
<tr><td>friends</td><td>Per pin: status, as in Table U-2, and mine, true where the user asked</td></tr>
<tr><td>talks</td><td>Per identification_numero_talkes: the talk, as in Table G-6</td></tr>
</tbody>
</table>

<table id="table-g-6">
<caption>Table G-6 Members of a Talk in the State</caption>
<thead>
<tr><th>Member</th><th>Holds</th><th>Type</th></tr>
</thead>
<tbody>
<tr><td>type</td><td>0 or 1</td><td>0, 1</td></tr>
<tr><td>peer</td><td>The pin of the other user</td><td>0</td></tr>
<tr><td>can_send</td><td>Whether the user may send to the talk</td><td>0</td></tr>
<tr><td>name</td><td>The name of the group</td><td>1</td></tr>
<tr><td>icon</td><td>The path of the current icon of the group</td><td>1</td></tr>
<tr><td>members</td><td>The pins of the members, in the order in which they joined</td><td>1</td></tr>
<tr><td>created</td><td>The big_date of the talk</td><td>0, 1</td></tr>
<tr><td>last</td><td>The big_date of the latest message, or null</td><td>0, 1</td></tr>
<tr><td>unread</td><td>The count of unread messages from others (§ ( Ú/( ú ) ))</td><td>0, 1</td></tr>
<tr><td>mentions</td><td>The count of unread messages that name the user</td><td>0, 1</td></tr>
</tbody>
</table>

The pronal network keeps the current avatar of each user and the current icon of each group, and gives that one alone. An application may show them, and may keep earlier ones itself.

A push state_patch holds rev and ops. Each op holds p, a path, and v, the new value at that path; the value null removes what the path names. The paths are people. followed by a pin, friends. followed by a pin, talks. followed by an identification_numero_talkes, and reload, by which the pronal network tells the application to take the state again from state.php.

When a talk enters the state of a user, the pronal network joins every connection of that user to the room of the talk, and when the talk leaves the state, the pronal network takes them out of it.

## ( d ) Emendetur Blocks

The sender of a message may name users who may edit parts of it, by Emendetur blocks in its content. A block is written as these lines, in order:

1. the line [Emendetur;
2. for each editor, a line of @ and the 15 digits of the editor's pin, each pin once;
3. an empty line;
4. the body, of one line or more;
5. a line that begins with ], whose rest is the tail.

The pronal network gives in content the body and the tail of every block in its place, and in emend the pins of the editors of all blocks. messages.php?action=get_one gives the sender the whole content in edit_content, and gives an editor in edit_content the bodies of every block that names that editor, in order, joined by lines that begin with ]. The editor sends them back, changed, in the same form to messages.php?action=edit, and the pronal network puts each body into its block.

In the mesh, every member's node holds the whole content of a message, blocks included, and puts the bodies of an emend move into their blocks itself, by the rules above. An emend move names the version of the message against which it was made (Table I-1).

# ( D ) THE PUSHES

The pronal network pushes events to an application over Socket.IO, version 4, at https://pronal.v31null.de. The pronal network pushes every event as it happens, and applications receive events by these pushes. A node also receives moves from other nodes (§ ( I )).

1. The application takes a pass from users.php?action=socket_pass and, once connected, sends it in the event auth. A pass is good for one use. The pronal network answers auth_ok, or auth_error with invalid_token or too_many_connections.
2. The application joins the room of a talk with join_chat and the identification_numero_talkes, and leaves it with leave_chat. The pronal network admits only members of the talk.
3. The pronal network pushes the events of a talk only to the room of that talk, and the events of a user only to that user.
4. The pronal network also joins and leaves rooms for the user as talks enter and leave the state (§ ( G/( g ) )).
5. A move handed in is pushed by the events of Table D-2 as a request of § ( G ) is, and counts as a change for messages.php?action=get with since from the time it arrived.

The status of a user is a state. OFFLINE means offline. Every other value means online, and an application shows it as it likes. A user is online while at least one of their connections to the pronal network is open and their own state is other than OFFLINE; the pronal network then gives the state they set, or ONLINE, or AWAY after a minute without activity. The pronal network pushes every change of status by status_change to the friends of the user, to the users with whom the user shares a talk of type 0 or 1, and to the connections that watch the user, and gives the status in the state (§ ( G/( g ) )).

Tables D-1 and D-2 present the events of the pronal network.

<table id="table-d-1">
<caption>Table D-1 Events from an Application</caption>
<thead>
<tr><th>Event</th><th>Data</th><th>Does</th></tr>
</thead>
<tbody>
<tr><td>auth</td><td>The pass</td><td>Authenticates the connection</td></tr>
<tr><td>join_chat</td><td>The identification_numero_talkes alone</td><td>Joins the room of a talk</td></tr>
<tr><td>leave_chat</td><td>The identification_numero_talkes alone</td><td>Leaves it</td></tr>
<tr><td>user_typing</td><td>chatId, isTyping</td><td>Tells the room that the user types</td></tr>
<tr><td>set_own_state</td><td>state: ONLINE, AWAY, DND, OFFLINE, or null to clear it</td><td>Sets the user's own state</td></tr>
<tr><td>presence</td><td>status: AWAY or OFFLINE while the user is inactive, any other value while active</td><td>Tells the pronal network whether the user is active</td></tr>
<tr><td>watch_presence, unwatch_presence</td><td>pin</td><td>Starts or stops watching the status of a user, up to 50 at a time</td></tr>
<tr><td>mesh_signal</td><td>to, address, endpoint, dial</td><td>Passes the address of a node, and the public addresses and ports at which it is seen when it listens (endpoint) and when it dials (dial), to the nodes of the user to, where the two users share a talk and no NEGL stands between them, or where either is the system pin (§ ( Ż/( ù ) ), § ( Ż/( g ) ))</td></tr>
<tr><td>The events of § ( E )</td><td></td><td>Calls</td></tr>
</tbody>
</table>

<table id="table-d-2">
<caption>Table D-2 Events from the Pronal Network</caption>
<thead>
<tr><th>Event</th><th>Data</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>auth_ok, auth_error</td><td></td><td>The answer to auth</td></tr>
<tr><td>message_received</td><td>The message (Table G-3)</td><td>A new message</td></tr>
<tr><td>message_edited</td><td>talk_id, min, modified_at, content, mentions, emend</td><td>A message was edited</td></tr>
<tr><td>message_deleted</td><td>talk_id, min</td><td>A message was deleted</td></tr>
<tr><td>message_liked</td><td>talk_id, min, likes</td><td>The count of likes changed</td></tr>
<tr><td>status_change</td><td>pin, status</td><td>The status of a user changed</td></tr>
<tr><td>typing_indicator</td><td>chatId, user, isTyping</td><td>A user types</td></tr>
<tr><td>typing_stop</td><td>chatId, user</td><td>A user stopped, or 5 seconds passed</td></tr>
<tr><td>group_update_required</td><td>talk_id</td><td>The members, the owner or the name of a group changed</td></tr>
<tr><td>sync</td><td>action FRIEND_UPDATE, and data with from, pins and status</td><td>A friendship changed</td></tr>
<tr><td>sync</td><td>action READ, and data with talk_id, unread and mentions</td><td>The user read a talk on another device</td></tr>
<tr><td>sync</td><td>action DM_CREATED, and data with talk_id</td><td>The user opened a talk of type 0 on another device</td></tr>
<tr><td>state_patch</td><td>rev, ops</td><td>The state changed (§ ( G/( g ) ))</td></tr>
<tr><td>force_logout</td><td></td><td>The bearer token has ended</td></tr>
<tr><td>mesh_signal</td><td>from, address, endpoint, dial</td><td>A node of the user from tells its address and where it is seen when it listens and when it dials (§ ( Ż/( ù ) ))</td></tr>
<tr><td>mesh_near</td><td>addresses</td><td>The nodes of the user, or of a talk of the user, that share the public address of this connection (§ ( Ż/( ú ) ))</td></tr>
<tr><td>The events of § ( E )</td><td></td><td>Calls</td></tr>
</tbody>
</table>

# ( E ) CALLS

Calls run by WebRTC. The ICE servers come from talks/ice.php. The pronal network relays the signals of a call, and the media flow between the users, or through the selective forwarding unit in a group.

## ( a ) Calls Between Two Users

A call between two users runs peer to peer. Every signal names the other user in to, and the pronal network passes it on with the sender in from. A call rings for 45 seconds. NEGL bars it. cid and dev are tags of 8 to 32 hexadecimal digits for the call and the device. When it ends, the caller records it with talks/call_log.php. Table E-1 presents the events.

Two nodes with a live link (§ ( Ż )) may pass the events of Table E-1 over that link in place of the pronal network. The caller's node then records the call by a call move (Table I-2).

<table id="table-e-1">
<caption>Table E-1 Events of a Call Between Two Users</caption>
<thead>
<tr><th>Event</th><th>Data</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>call_offer</td><td>to, offer, cid, dev</td><td>Calls a user; pushed as from, offer, left, cid</td></tr>
<tr><td>call_answer</td><td>to, answer, dev</td><td>Takes the call</td></tr>
<tr><td>call_ice</td><td>to, candidate</td><td>An ICE candidate</td></tr>
<tr><td>call_reject, call_busy, call_cancel, call_end, call_leave</td><td>to, cid</td><td>Refuses, is busy, takes back, ends, leaves</td></tr>
<tr><td>call_renegotiate</td><td>to, desc</td><td>A new description during the call</td></tr>
<tr><td>call_media</td><td>to, screen</td><td>The shared screen changed</td></tr>
<tr><td>call_resume, call_resume_answer, call_resume_req</td><td>to, offer or answer, rs or rq</td><td>Resumes a broken call</td></tr>
<tr><td>call_handled</td><td></td><td>The call was taken on another device of the user</td></tr>
<tr><td>call_sync</td><td>dev</td><td>Asks for the state of the user's calls</td></tr>
<tr><td>call_unavailable</td><td>to</td><td>Pushed where the user is out of reach, as under NEGL</td></tr>
</tbody>
</table>

## ( b ) Calls in a Group

A call in a group runs through the selective forwarding unit of Cloudflare Realtime, version 1, which the pronal network reaches for the application through talks/sfu.php. The members of a talk of type 1 take part in its call. A call in a group needs the pronal network, and does not run over the mesh.

talks/sfu.php takes action, session, seal and body. body is the JSON text of the request to Cloudflare Realtime, up to 65 536 bytes, and the answer is that of Cloudflare Realtime, passed on whole. Where Cloudflare Realtime refuses, the pronal network answers 502 with its errorCode. session names a session that the user opened with new, and seal is the seal that the pronal network gave with it. Table E-2 presents the actions.

<table id="table-e-2">
<caption>Table E-2 Actions of talks/sfu.php</caption>
<thead>
<tr><th>action</th><th>body</th><th>Does</th></tr>
</thead>
<tbody>
<tr><td>new</td><td></td><td>Opens a session; the answer holds sessionId and seal</td></tr>
<tr><td>tracks</td><td>sessionDescription, with type offer and sdp, and tracks, each with location local, mid and trackName</td><td>Publishes tracks of the application</td></tr>
<tr><td>tracks</td><td>tracks, each with location remote, sessionId and trackName</td><td>Pulls tracks of another member; the answer holds the offer, which the application answers by renegotiate</td></tr>
<tr><td>renegotiate</td><td>sessionDescription, with type answer and sdp</td><td>Answers the offer of Cloudflare Realtime</td></tr>
<tr><td>close</td><td>tracks, each with mid; sessionDescription where the application gives one; force</td><td>Closes tracks</td></tr>
</tbody>
</table>

Each member publishes up to 8 tracks. A trackName is 1 to 32 letters, digits or hyphens. Video is VP8. Table E-3 presents the tracks.

<table id="table-e-3">
<caption>Table E-3 Tracks</caption>
<thead>
<tr><th>trackName</th><th>kind</th><th>Media</th></tr>
</thead>
<tbody>
<tr><td>mic</td><td>mic</td><td>Audio of the microphone</td></tr>
<tr><td>screen-v</td><td>screen</td><td>Video of the shared screen</td></tr>
<tr><td>screen-a</td><td>screenaudio</td><td>Audio of the shared screen</td></tr>
</tbody>
</table>

A member joins with gcall_join once its session is open and its tracks are published. It names its sessionId, of 8 to 128 characters of A–Z, a–z, 0–9, _ and -, its tracks, each with trackName and kind, its dev, and its e2ee: the pin of the user, a colon and 8 hexadecimal digits that the application draws for the call. The first member starts the call, and the pronal network rings the other members of the group for 45 seconds. After every change, the pronal network pushes gcall_state to the members of the group, with talk, starter, cid, elapsed in milliseconds, members, each with pin, sessionId, tracks, e2ee and dev, and ringing, the pins still rung. Each member pulls the tracks of the others by the sessionId and trackName given in gcall_state. Table E-4 presents the events.

<table id="table-e-4">
<caption>Table E-4 Events of a Call in a Group</caption>
<thead>
<tr><th>Event</th><th>Data</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>gcall_join</td><td>talk, sessionId, tracks, dev, e2ee</td><td>Joins the call of a group, or starts it</td></tr>
<tr><td>gcall_ring</td><td>talk, from, cid, name, left</td><td>Pushed to the members when a call starts</td></tr>
<tr><td>gcall_state</td><td>talk, starter, cid, elapsed, members, ringing, and rejoin where the device is already in the call</td><td>Pushed after every change</td></tr>
<tr><td>gcall_tracks</td><td>talk, tracks</td><td>The tracks of a member changed</td></tr>
<tr><td>gcall_leave, gcall_decline</td><td>talk</td><td>Leaves, or declines the ring</td></tr>
<tr><td>gcall_sync</td><td>dev</td><td>Asks for the calls of the user's groups; answered by gcall_state and, while rung, gcall_ring</td></tr>
<tr><td>gcall_mls_role</td><td>talk, create</td><td>Pushed after gcall_join (§ ( E/( ú ) ))</td></tr>
<tr><td>gcall_mls</td><td>talk, data</td><td>An MLS message, passed to the other members with from (§ ( E/( ú ) ))</td></tr>
<tr><td>gcall_end</td><td>talk, cid</td><td>Pushed when the last member leaves</td></tr>
</tbody>
</table>

## ( ú ) Encryption of Calls in a Group

The media of a call in a group are encrypted end to end by MLS (RFC 9420). The pronal network and Cloudflare carry them sealed.

The MLS group of a call uses the protocol version MLS 1.0 and the ciphersuite MLS_128_DHKEMX25519_AES128GCM_SHA256_Ed25519. The credential of each member is a basic credential whose identity is its e2ee, in UTF-8. The sender ratchet tolerates 500 messages out of order and a jump of 1000 generations at most. The ratchet tree travels beside the Welcome.

1. After gcall_join, the pronal network pushes gcall_mls_role with create. Where create is true, the member is the first in the call with an e2ee: it creates the MLS group and is its designated committer. Otherwise it shares its key package.
2. Members exchange MLS messages by gcall_mls. Its data is a JSON text of up to 262 144 characters, whose member type is one of those of Table E-5. The pronal network passes it to the other members of the call with from.
3. The designated committer adds every member that shared a key package: it sends the Welcome and the ratchet tree by sendMlsWelcome, and the Commit by sendMlsMessage. It removes every member whose e2ee leaves the members of gcall_state, and sends that Commit by sendMlsMessage. Every other member merges each Commit it receives.
4. A member that joined by a Welcome becomes the designated committer once every member who was in the group at its Welcome has left.

<table id="table-e-5">
<caption>Table E-5 Messages of gcall_mls</caption>
<thead>
<tr><th>type</th><th>Members</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>shareKeyPackage</td><td>keyPkg</td><td>The key package of the member</td></tr>
<tr><td>sendMlsWelcome</td><td>senderId, welcome, rtree</td><td>The Welcome and the ratchet tree</td></tr>
<tr><td>sendMlsMessage</td><td>senderId, msg</td><td>A Commit</td></tr>
</tbody>
</table>

keyPkg, welcome, rtree and msg are the base64 of their TLS serialization (RFC 9420). senderId is the e2ee of the designated committer.

Every encoded frame of a track, audio and video, is sealed in the application of its sender and opened in those of its receivers. Where the top bit of the first byte of the frame is 0, its first 10 bytes stay clear; otherwise its first byte stays clear. The rest is sealed as an MLS application message of the current epoch, and the frame sent is the clear part followed by the TLS serialization of that message. A receiver opens the message and puts its content after the clear part. A frame that is empty, too short, or fails to open is dropped.

The safety number of an epoch is the first 32 bytes of its epoch authenticator. An application that shows it shows its first 30 bytes as six groups of five digits, separated by spaces: each group is five bytes, read in order as a number in base 256, taken modulo 100 000 and written with five digits.

# ( Ż ) THE LINKS

## ( a ) pronal://

Nodes talk with each other by pronal://. A link of pronal:// is a connection of QUIC (RFC 9000), which carries TLS 1.3 (RFC 9001, RFC 8446); no other carrier is used. The application protocol named in the handshake is pronal1. A node listens on any free port, made known by § ( Ż/( ú ) ) and § ( Ż/( ù ) ).

The dialler opens one stream in both directions, the control stream. Every message on it is 1 byte of its type, 4 bytes of the length of its body, most significant byte first, and the body, which is JSON in UTF-8. Each packet goes on a one-way stream of its own, as one message of type 5 whose body is the packet in the layout of Table Z-1 followed by the JSON array of its receipts, or, while the receiver asks for work (§ ( Z/( u ) )), by a JSON object with receipts and value, so that a large file does not hold back the rest. A node accepts no message but proof until the peer proof of § ( Ż/( b ) ) has passed, save on the port for reach of the server node (§ ( Ż/( g ) )).

The pronal network speaks pronal:// through one node of its own, the server node (§ ( Ż/( g ) )), and is also reached by § ( G ) and § ( D ). Table Ż-1 presents the messages of pronal://.

<table id="table-ż-1">
<caption>Table Ż-1 Messages of pronal://</caption>
<thead>
<tr><th>Type</th><th>Message</th><th>Holds</th><th>See</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>proof</td><td>A fresh number, then the peer proof</td><td>§ ( Ż/( b ) )</td></tr>
<tr><td>2</td><td>list</td><td>The number of the newest list held; the list, where the other holds an older one</td><td>§ ( Ù/( ú ) )</td></tr>
<tr><td>3</td><td>seen</td><td>The address and port from which the other node is seen</td><td>§ ( Ż/( ù ) )</td></tr>
<tr><td>4</td><td>want</td><td>tags, the tags the node wants, and carry, whether it carries packets of others</td><td>§ ( Z/( b ) )</td></tr>
<tr><td>5</td><td>packet</td><td>A packet and its receipts, on a one-way stream of its own</td><td>§ ( Z/( a ) )</td></tr>
<tr><td>6</td><td>receipt</td><td>The pronal network's receipt</td><td>§ ( I/( ù ) )</td></tr>
<tr><td>7</td><td>busy</td><td>A seed and a number of bits</td><td>§ ( Z/( u ) )</td></tr>
<tr><td>8</td><td>ask</td><td>A lookup</td><td>§ ( Z/( ù ) )</td></tr>
<tr><td>9</td><td>answer</td><td>The answer to a lookup</td><td>§ ( Z/( ù ) )</td></tr>
<tr><td>10</td><td>befriend</td><td>A friend request, addressed to a node</td><td>§ ( Z/( ù ) )</td></tr>
<tr><td>11</td><td>keys</td><td>A request for day keys, a throwaway key signed by a device key, or day keys sealed to that throwaway key; addressed to a node</td><td>§ ( I/( b ) )</td></tr>
<tr><td>12</td><td>typing</td><td>talk, isTyping, on a live link alone</td><td>§ ( I/( g ) )</td></tr>
<tr><td>13</td><td>call</td><td>An event of Table E-1, on a live link alone</td><td>§ ( E/( a ) )</td></tr>
<tr><td>14</td><td>admitted</td><td>The number of the newest list of admitted applications held; the list, where the other holds an older one</td><td>§ ( B/( ú ) )</td></tr>
<tr><td>15</td><td>reach</td><td>From the tester, fresh; from the server node, the statement of kind reach</td><td>§ ( Ż/( g ) )</td></tr>
</tbody>
</table>

## ( b ) Peer Proof

TLS proves that each side holds the key of the certificate it shows; the peer proof binds that key to a device key and a pin.

1. Each node makes, at each start, one throwaway P-256 key and a self-signed X.509 certificate for it, in memory alone.
2. Both sides present their throwaway certificates in the TLS handshake; the listener requires the dialler's.
3. Each side sends a fresh number of 32 random bytes.
4. Each side signs with its device key the bytes of a JSON object with kind, peer_proof; role, dialler or listener; dialler, the hexadecimal SHA-256 of the dialler's throwaway certificate; listener, that of the listener's; fresh, the base64 of the fresh number of the other side; and cert, its own certificate as signed, with the statement of the online key. It sends these bytes with the signature.
5. Each side checks the signature; that role is the role of the other side; that dialler and listener are the hashes of the two throwaway certificates seen in this handshake; that fresh is the number it sent itself; and the certificate against the keys of the pronal network, its until, and the list. A proof that fails any check ends the connection.

A node in the middle shows each side a throwaway certificate of its own, so the two sides sign different hashes and the proof fails. The certificate, and so the pin, is inside what is signed, and the role and the order are fixed, so a proof cannot be turned round or passed on.

## ( ú ) Links on One Network

Two nodes on one network talk over their addresses on that network, and their traffic never leaves the router. They find each other by DNS-SD over multicast DNS (RFC 6762, RFC 6763), under the service type _pronal._udp with the first 32 hexadecimal digits of the address of the node as the name of the instance, or by mesh_near, by which the pronal network tells nodes of one user or of one talk that they share a public address. Where a link on one network fails, as behind a VPN, a node should fall back to its other links without telling the user.

## ( u ) Links Without a Network

Nodes with Bluetooth Low Energy and Wi-Fi Direct find each other by Bluetooth Low Energy and carry their traffic over Wi-Fi Direct, with QUIC on top.

1. The advert carries the service identifier of the pronal network, 92bf1dfd-bfd3-4e48-8cf7-e92801a7a9a1, and, as service data under that identifier, a value of 4 random bytes. The node draws a new value and starts its advert anew every 10 minutes. Nothing in the advert is fixed.
2. A node that finds an advert connects to it only where its own value, read as a number most significant byte first, is lower than the value found, and connects to one value once at most. The connecting node is the central, the advertising node the peripheral. A tester of § ( Ż/( g ) ) connects to any advert, as central.
3. The peripheral serves a GATT service under the service identifier with one characteristic, 92bf1dfd-bfd3-4e48-8cf7-e92801a7a9a2, which the central writes and whose indications the peripheral sends. Every message is 1 byte of its type, 4 bytes of the length of its body, most significant byte first, and the body, which is JSON in UTF-8, as on pronal:// (§ ( Ż/( a ) )), cut into pieces of the size the connection allows and sent in order. A body holds 16 384 bytes at most. Table Ż-4 presents the messages.
4. Each side sends key, with point, the base64 of a throwaway P-256 public key drawn for this connection alone, in the uncompressed form of 65 bytes, and hosting, true where it hosts a Wi-Fi Direct group. On holding both keys, each side sends proof, a signed statement of kind radio_proof by its device key, with role, central or peripheral; central and peripheral, the point of the central and of the peripheral; and cert, its certificate as signed, with the statement of the online key. Each side checks the proof of the other as a peer proof is checked (§ ( Ż/( b ) )), and ends the connection where the proof fails or where it already holds a link to that address.
5. Both derive a shared secret by ECDH of the two throwaway keys. The host is the side whose hosting is true, or, where both or neither are, the side with the lower address. The host sends group, with nonce, the base64 of 12 random bytes, and sealed, the base64 of the bytes of a JSON object with name, the network name of its group; passphrase, its passphrase; address, its IPv4 address in the group; and port, the port of QUIC on which it listens there; sealed with AES-256-GCM under the SHA-256 of the shared secret, with that nonce, no additional data, and the tag of 16 bytes after the sealed bytes.
6. The joiner joins the group by its name and passphrase, with no dialog, and dials the link of § ( Ż/( a ) ) to address and port. The connection of Bluetooth Low Energy then ends.

A host keeps its group while a link runs over it, and leaves it 60 seconds after its last link ends. A stranger who learns the passphrase reaches only the port of QUIC, and fails the peer proof.

<table id="table-ż-4">
<caption>Table Ż-4 Messages over Bluetooth Low Energy</caption>
<thead>
<tr><th>Type</th><th>Message</th><th>From</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>key</td><td>Both</td><td>point, hosting</td></tr>
<tr><td>2</td><td>proof</td><td>Both</td><td>The statement of kind radio_proof</td></tr>
<tr><td>3</td><td>group</td><td>Host</td><td>nonce, sealed</td></tr>
<tr><td>4</td><td>reach</td><td>Tester; peripheral</td><td>From the tester, fresh; from the peripheral, the statement of kind reach</td></tr>
</tbody>
</table>

## ( ù ) Links Across the Internet

No node needs an open port.

1. Before it listens, a node asks two STUN servers (RFC 8489) of talks/ice.php for its public address and port, from the port on which it will listen and from the port from which it will dial. Where the two servers see one port at two public ports, the router gives a new port for each destination.
2. Two nodes swap their addresses and the public addresses and ports at which they are seen, by mesh_signal.
3. The node with the lower address listens first: it sends UDP datagrams from its listener port to the public dialling address of the other, so that its own router admits the other, and then listens on that port. The other dials from its dialling port to the public listening address of the first. Where no connection opens, the two change roles.
4. While the pronal network is out of reach, connected nodes tell each other, by seen, the address and port at which they see each other.
5. A device that cannot link this way, such as one whose router gives a new port for each destination, is no node; its application reaches the pronal network by § ( G ) and § ( D ).

A connection that is dialled cannot share the port of a listener, so a node listens on one port and dials from another. Since the listener sends first, its router and its firewall admit the other, and no port is opened and no rule of the firewall is made.

Behind a VPN, a node is excluded from the VPN by split tunnelling, or it cannot link and is no node. On a desktop the node therefore runs as a process of its own, which the user can select; on Android the application is excluded. Without a VPN nothing is needed.

## ( g ) The Server Node

The server node is the node of the pronal network, run by MEF beside the database of the pronal network, so that the database speaks pronal://. It keeps its device key, a P-256 key pair, in a file of MEF beside the keys of the pronal network, so that it can know itself and be restored; this is the one exception to § ( Ù/( ú ) ). It links as every node does (§ ( Ż/( ù ) )); it needs no open port. It opens a fresh port for reach for each tester (items 3 to 5).

1. The certificate of the server node names the system pin (Table Ú-1). The pronal network issues a certificate naming the system pin to the key of the server node alone.
2. The public half of the device key of the server node is: `BN/VuTWK/9ELVBRLsUD0OJ5SodGEveLeoo17DgKqqg+FnxGH6HD4pdErsczR6GVAFvD3XLSRbO0oV2AG/ARnpRY=`. Every node holds it. A node that meets a certificate naming the system pin for any other key refuses it.
3. Any device, logged in or not, may test whether it reaches pronal://; it is then the tester. The server node learns the public address and port of that port as in § ( Ż/( ù ) ), item 1.
4. Across the internet, the tester takes two STUN servers by GET of mesh/reach.php and asks them, from the port from which it will dial, for its public address and port. It sends these by POST to mesh/reach.php. The pronal network passes them to the server node, which sends UDP datagrams from its port for reach to them, so that its router admits the tester; the pronal network then answers with address and port, the public address and port of that port. The tester dials them by QUIC with the application protocol pronal1, the listener requiring no certificate of the dialler, and sends on the control stream reach, with fresh, the base64 of 32 random bytes. The server node answers reach with a statement signed by its device key, with kind reach; fresh, as received; seen, the address and port from which it sees the tester; and big_date. It then ends the connection. It answers no other message on that port and ends the connection at any other. The tester checks the signature against the key of item 2 and that fresh is its own; where both hold, it reaches pronal:// across the internet. Where no connection opens, it does not.
5. By radio, the tester finds an advert of § ( Ż/( u ) ) and sends reach (Table Ż-4), with fresh. The peripheral answers reach with a statement signed by its device key, with kind reach; fresh, as received; reached, whether it holds a link to the server node; big_date; and cert, its certificate as signed, with the statement of the online key. It then ends the connection. The tester checks the statement as a peer proof is checked (§ ( Ż/( b ) )); where it holds, the tester reaches pronal:// by radio, and, where reached is true, the server node through it.

## ( d ) The Node of a Device

Every pronal application of one account on one device uses the one node of that account (§ ( Ù/( ú ) )), whichever application started it. The node is a programme of its own, so that it can be excluded from a VPN by split tunnelling (§ ( Ż/( ù ) )), and every application reaches it as set out here.

1. The node keeps its files in the folder pronal in the local data of the user: %LOCALAPPDATA%\pronal\ on Windows and ~/.local/share/pronal/ on GNU/Linux. The folder shall be open to that user alone. The programme of the node lies in it, as node.exe on Windows and node on GNU/Linux. The pronal network offers the programme of the node for Windows at https://pronal.v31null.de/psnpn1117r2_node/, as node.exe and msquic.dll, beside psnpn1117r2_manifest.json, a JSON object with version, date, a sort key of psí1103r1, and files, each with file, bytes and sha256, the SHA-256 of the file in hexadecimal. An application that puts the programme into the folder shall check every file against the manifest first. On Android, the node is in an application, and other applications reach it through a service of the system (§ ( L )); the rules below hold inside it.
2. The node is started with sub, the subject of its login at the V31N Portal, by which it takes its bearer token (§ ( Ù/( a ) )). While it runs, it holds a lock of the operating system on the file named by sub followed by .lock. The operating system frees the lock when the node ends, however it ends. A node that cannot take the lock ends at once, so that one node runs per account.
3. The node writes into the folder a .v31np file (PS-N/PF-1031) named by its pin followed by .v31np. It holds two files, node and settings. node is a JSON object with port, the TCP port on 127.0.0.1 on which the node listens; key, the base64 of the public half of its device key; secret, the base64 of 32 random bytes drawn at each start; and sub. settings is a JSON object of the settings of Table Ż-3. The node alone writes the file.
4. An application talks with the node over TCP at 127.0.0.1 and port. Each message is 1 byte of its type, 4 bytes of the length of its body, most significant byte first, and the body, which is JSON in UTF-8, as on pronal:// (§ ( Ż/( a ) )). Table Ż-2 presents the messages.
5. The application first sends ping with fresh, the base64 of 32 random bytes. The node answers with a signed statement of kind local_ping, with fresh, signed by its device key. The application checks the signature against key and fresh against its own. Where the port does not answer, or the answer fails a check, no node is alive, whatever programme now holds the port: the application starts the programme of the node with sub, and tries again once the node has written its file. An application that finds no file of its pin starts no node.
6. After ping, the application sends hello with secret. The node closes every connection whose secret differs, and every connection that sends any message but ping before hello.
7. Any application of the account may change the settings by set. The node obeys the settings last set, whichever application set them, and pushes node to every connected application whenever its state changes.
8. On Android, the application that holds the node exports a bound service under the action pronal.psnpn1117r2.NODE, reached by a Messenger. Each message carries what, the type of Table Ż-2, and a bundle with pin, the pin of the account, and body, the JSON as text; the answer goes to replyTo with the same what. ping and hello are not used. The service reads the uid of each message, finds its package and the SHA-256 of its signing certificate, and answers only a caller named in the newest list of admitted applications it holds (Table Ù-2), which it takes from mesh/admitted.php or from its neighbours by admitted (Table Ż-1), and keeps. The pushes of the node go to every replyTo that has sent get.
9. A caller on Android sends proof and is answered by a statement of kind key_proof signed by the device key of the pin; with it and its own bearer token the caller obtains a certificate from mesh/certificate.php, and gives it to the node by cert. The node answers sign, send, relay and status for that caller only while it holds a certificate given by that caller which is usable, names the pin of the message, names the key of that pin, and names the client admitted for the package of the caller.

<table id="table-ż-2">
<caption>Table Ż-2 Messages of the Node of a Device</caption>
<thead>
<tr><th>Type</th><th>Message</th><th>From</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>ping</td><td>Application</td><td>fresh; answered by the statement of kind local_ping</td></tr>
<tr><td>2</td><td>hello</td><td>Application</td><td>secret; answered by ok, true</td></tr>
<tr><td>3</td><td>get</td><td>Application</td><td>Nothing; answered by the state of Table Ż-3</td></tr>
<tr><td>4</td><td>set</td><td>Application</td><td>Any of the settings of Table Ż-3; answered by the state</td></tr>
<tr><td>5</td><td>node</td><td>Node</td><td>The state of Table Ż-3, pushed at every change</td></tr>
<tr><td>6</td><td>status</td><td>Application</td><td>Nothing; answered by address, pin, anchored, now, the time of the node (§ ( I/( ú ) )), beacon, list, the number of the list held, cert, the certificate with the statement of the online key, reached, whether the pronal network answered, signal, whether § ( D ) is connected, seen (§ ( Ż/( ù ) )), prev, the hash of the last move made with the device key, order (Table Ż-3), and peers, each with pin, address, lan, carry and wants, the tags it wants</td></tr>
<tr><td>7</td><td>sign</td><td>Application</td><td>data, the base64 of the bytes to sign; answered by sig, the base64 of the signature by the device key</td></tr>
<tr><td>8</td><td>send</td><td>Application</td><td>packet, the base64 of a packet of the user (Table Z-1); to, the pins of the members of its talk; move, the hexadecimal SHA-256 of the bytes of the move, which becomes prev; and need, true where the node shall take the packet only while a neighbour wants its tag or carries; answered by hash, or null where it was not taken, and willing, the count of such neighbours. The node holds it, passes it on, and hands it in (§ ( I/( ù ) ))</td></tr>
<tr><td>9</td><td>want</td><td>Application</td><td>tags, the base64 of the tags of the talks of the user; answered by ok</td></tr>
<tr><td>10</td><td>file</td><td>Application</td><td>hash; answered by packet, the base64 of the packet the node holds by that hash, or null</td></tr>
<tr><td>11</td><td>rulings</td><td>Application</td><td>number, the newest ruling the application has taken (§ ( Ù/( ú ) )); answered by ok</td></tr>
<tr><td>12</td><td>prompt</td><td>Application</td><td>title, text, now and later, the words of the prompt of § ( I/( d ) ); answered by now, true where the user chose to be corrected now</td></tr>
<tr><td>13</td><td>relay</td><td>Application</td><td>type, a type of Table Ż-1 from 8 to 13, body, and pin for typing and call; answered by ok</td></tr>
<tr><td>14</td><td>packet</td><td>Node</td><td>packet and receipts, pushed when a packet of a wanted tag arrives</td></tr>
<tr><td>15</td><td>receipt</td><td>Node</td><td>receipt, the pronal network's receipt, pushed when it arrives</td></tr>
<tr><td>16</td><td>relayed</td><td>Node</td><td>type, pin, address and body, pushed for a message of Table Ż-1 from 8 to 13 that reached the node</td></tr>
<tr><td>17</td><td>proof</td><td>Application on Android</td><td>Nothing; answered by the statement of kind key_proof, with key, the base64 of the public half of the device key, and big_date, the time of the node</td></tr>
<tr><td>18</td><td>cert</td><td>Application on Android</td><td>cert, the certificate the caller obtained for the device key, with the statement of the online key; answered by ok</td></tr>
</tbody>
</table>

<table id="table-ż-3">
<caption>Table Ż-3 State of the Node</caption>
<thead>
<tr><th>Member</th><th>Holds</th><th>Setting</th><th>At first start</th></tr>
</thead>
<tbody>
<tr><td>link</td><td>true where the node can link across the internet (§ ( Ż/( ù ) ))</td><td>No</td><td></td></tr>
<tr><td>has_radio</td><td>true where the device has Bluetooth Low Energy and Wi-Fi Direct (§ ( Ż/( u ) ))</td><td>No</td><td></td></tr>
<tr><td>free</td><td>The measured free space of the device, in bytes</td><td>No</td><td></td></tr>
<tr><td>carry</td><td>true where the node carries packets of others (§ ( Z/( b ) ))</td><td>Yes</td><td>true</td></tr>
<tr><td>cap</td><td>The bound of the storage of carried packets, in bytes, at most free</td><td>Yes</td><td>free</td></tr>
<tr><td>lan</td><td>true where the node links on one network (§ ( Ż/( ú ) ))</td><td>Yes</td><td>true</td></tr>
<tr><td>radio</td><td>true where the node links without a network (§ ( Ż/( u ) ))</td><td>Yes</td><td>false</td></tr>
<tr><td>internet</td><td>true where the node links across the internet (§ ( Ż/( ù ) ))</td><td>Yes</td><td>true</td></tr>
<tr><td>order</td><td>For talks of type 0 and 1. api1: the application sends by § ( G ), and the mesh carries a move only while the pronal network is out of reach. mesh: the node sends the move to the nodes of the talk first, and the move is handed in when the pronal network is in reach (§ ( I/( ù ) )).</td><td>Yes</td><td>mesh</td></tr>
</tbody>
</table>

An application whose node runs shall, while order is mesh, give each write of § ( G ) that has a kind in Table I-2 to the node as a move with need, and use § ( G ) only where the node does not take it, so that the traffic of its users runs between their own devices first. It shall use § ( G ) first only while the user has set order to api1.

# ( Z ) THE RELAY LAYER

## ( a ) The Packet

A packet is a sealed move, or a sealed file, as relays see it. Table Z-1 presents its layout.

<table id="table-z-1">
<caption>Table Z-1 Layout of a Packet</caption>
<thead>
<tr><th>Part</th><th>Bytes</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>tag</td><td>16</td><td>The tag of the talk and day (§ ( I/( b ) ))</td></tr>
<tr><td>day</td><td>8</td><td>The day, as the eight characters of the first part of the id</td></tr>
<tr><td>size</td><td>4</td><td>The count of sealed bytes, most significant byte first</td></tr>
<tr><td>hash</td><td>32</td><td>The SHA-256 of the sealed bytes</td></tr>
<tr><td>sealed</td><td>size</td><td>The sealed bytes</td></tr>
</tbody>
</table>

A relay sees a tag, a day and a size: no pin, no talk, no sender.

## ( b ) Carrying

1. A node carries packets for any node that holds a valid certificate, bonded or not. It cannot read them.
2. A node tells its neighbours, by want, the tags it wants and, by carry, whether it carries packets of others. It passes each packet of its own user to every neighbour, and each packet it carries to the neighbours that want its tag. Packets are routed directly by tag, not through circuits of several hops.
3. A node holds a packet until its receiver shows up, and drops it on the pronal network's receipt naming its hash, when its day leaves the window, or, under pressure of storage, those of the weakest bond first (§ ( Z/( g ) )).
4. The storage of carried packets is bounded by the measured free space of the device; the user may lower the bound. A node may take packets and never carry any.

## ( ú ) Receipts

A node that takes a packet of a talk it is not in may sign a receipt: a signed statement whose members are kind, receipt; tag; hash; big_date, the node's own time (§ ( I/( ú ) )); anchored; and cert, its certificate as signed, with the statement of the online key. A relay may witness what it cannot read. Receipts travel with the packet they witness, and the node that made a receipt is its witness. A node signs no receipt for a packet of its own talks, since its certificate would show which talk its pin is in; it witnesses those by a witness move (Table I-2). A receipt made by an unanchored node does not count.

## ( u ) Flood Control

A node limits each neighbour from which it takes packets, not each author: the author is hidden inside the seal, and the neighbour is known from the peer proof. A flood dies at the first honest hop.

A node whose queue from a neighbour grows faster than it drains answers busy, with seed, the base64 of random bytes changed at least every 60 seconds, and bits, a number of bits from 0 to 24. The neighbour then finds value, a whole number below 2⁵³, such that the SHA-256 of the bytes of the seed, the 32 bytes of its own address, the 32 bytes of the hash of the packet and the 8 bytes of the value, most significant byte first, begins with that number of zero bits, and sends the value with the packet (Table Ż-1). The node refuses every packet from that neighbour whose value fails against its current seed or the seed before it. The number of bits rises by one while the queue grows, and falls by one as it drains; a busy with bits 0 ends the puzzle.

## ( ù ) Lookup and Friend Requests

An application finds a user by name through users.php?action=search, or, where the pronal network is out of reach or the application so chooses, by ask, sent plainly to its neighbours. A node passes each ask on once, by its id, and drops repeats; § ( Z/( u ) ) holds. A node that knows the user answers. The answer carries the name card of the user, signed by the pronal network, and the certificate of a device of the user, with the statement of the online key; a node can forge neither. Of two name cards, the one with the later big_date stands. An orphaned account is answered with the orphan pin, or not at all. The address of a node of a pin is asked and answered the same way.

A friend request may travel by befriend through other nodes to the node of its target, signed by the device key of the asker. It reaches the pronal network as users.php?action=friend_request when the asker's node is in reach of the pronal network.

## ( g ) Bonds and Your Part — INFORMATIVE

A node keeps its user's own talks and their attachments, and sealed copies of the traffic of the people its user is bonded with; the stronger the bond, the more is kept. A bond is what two devices did for each other: their meetings and the packets they delivered. It is counted on the device, works without the pronal network, and is never shared. No bond decays by a constant; when the store is full, the copies of the weakest bond go first.

# ( I ) THE TALK LAYER

## ( a ) The Move

A move is one action in a talk, made and signed by a node. It is the bytes of a JSON object in UTF-8, made once by the acting node. It travels, is stored and is handed in as exactly those bytes, with its signature, and nobody encodes it again. To check a move, a node checks the signature over the bytes as received, then parses them. The hash of a move is the SHA-256 of its bytes.

A parser refuses a move in which a member is named twice or a number has a fraction. identification_numero_talkes is text in a move, never a number. Table I-1 presents the members of a move.

<table id="table-i-1">
<caption>Table I-1 Members of a Move</caption>
<thead>
<tr><th>Member</th><th>Holds</th></tr>
</thead>
<tbody>
<tr><td>id</td><td>A min made by the acting node: its big_date, the pin of the actor, a prigotov. For a sent message, the min of the message.</td></tr>
<tr><td>kind</td><td>A kind of Table I-2</td></tr>
<tr><td>talk</td><td>The identification_numero_talkes of the talk</td></tr>
<tr><td>target</td><td>The min on which the move acts, or the pin on which it acts</td></tr>
<tr><td>body</td><td>What the endpoint of Table I-2 takes: content, style, reply_min, and attachments, each with uuid, name, size, type, width, height and hash; a name; a count of likes; a duration</td></tr>
<tr><td>version</td><td>For an emend: the id of the last edit of the target that it saw, or the id of the send</td></tr>
<tr><td>prev</td><td>The hexadecimal SHA-256 of the previous move made with the same device key, or null</td></tr>
<tr><td>beacon</td><td>The newest beacon the node holds, as signed</td></tr>
<tr><td>anchored</td><td>true where the node counts its time from its anchor (§ ( I/( ú ) ))</td></tr>
<tr><td>cert</td><td>The certificate of the device key, as signed, with the statement of the online key</td></tr>
</tbody>
</table>

The pin in the id shall be the pin of the certificate, and the day in the id shall be the day of the packet. For a message sent through § ( G ), the pronal network makes the move and signs it with its online key, naming the pin; nodes accept the signature of the pronal network for any pin.

## ( b ) The Seal

1. The pronal network holds one secret of 32 random bytes per talk, which never leaves it. The day key of a day is the HMAC-SHA-256 under the secret of DAY, the talk and the day, joined by |, the day being the first eight characters of the id, as in ENC2. The pronal network gives each member, by mesh/day_keys.php, the day keys from today through the window. A removed member is given no further day keys, so what it can open ends with the window.
2. From the day key two keys are derived: the seal key, the HMAC-SHA-256 under the day key of SEAL, and the tag key, the HMAC-SHA-256 under the day key of TAG.
3. The 64 bytes of the signature followed by the bytes of the move are sealed with AES-256-GCM under the seal key, with the additional data made of MOVE, the talk and the day, joined by |. The nonce is 12 bytes: the first 4 bytes of the SHA-256 of the public half of the device key, or, for the pronal network, of its online key, then 8 random bytes. The sealed bytes are the nonce, the tag of GCM and the ciphertext. A file is sealed the same way, with FILE in place of MOVE and without a signature; the signed send move names its hash.
4. The tag of a packet is the first 16 bytes of the HMAC-SHA-256 under the tag key of the talk and the day, joined by |. A talk has one tag per day.
5. The pronal network seals its rulings and checkpoints as a move is sealed, its signature followed by the bytes of the statement, under the day of their big_date, and they travel as packets under the tag of their talk.

The id is not bound into the additional data, since a receiver learns it only on opening; it is bound by the signature. Every move is sealed to the day key, which the members' nodes and the pronal network hold, so relays cannot read it. The pronal network opens every move it is handed, stores its content as ENC2 and indexes it, as for a message sent through § ( G ); the mesh is not encrypted end to end against the pronal network.

A removed member holds the day keys through the window and can open the packets it catches in that time.

A member added while the pronal network is out of reach is given the day keys by the node that added it, by keys (Table Ż-1), carried to its address as befriend is:

1. The adding node finds the address of a node of the new member (§ ( Z/( ù ) )) and sends it keys, asking for a key.
2. That node draws a throwaway P-256 key and answers with its public half in a statement signed by its device key, with kind agreement_key, key, big_date and cert.
3. The adding node checks the statement, draws a throwaway P-256 key of its own, derives a shared secret by ECDH between the two throwaway keys, and seals the day keys with AES-256-GCM under the SHA-256 of the shared secret. It sends keys holding its throwaway public key and the sealed bytes.

Each keys message is a JSON object with id, 32 hexadecimal digits at random, to, the address it is for, and from, the address of its sender. The request holds ask, the identification_numero_talkes. The answer holds reply, the id of the request, and statement, the agreement_key as signed. The day keys hold reply, key, the base64 of the throwaway public key of the adding node as an uncompressed point, and sealed, the base64 of 12 random bytes followed by the AES-256-GCM ciphertext and tag of a JSON object with talk, type, name, members and day_keys, whose members are the days and whose values are the base64 of their day keys.

The device key never takes part in a key agreement.

## ( ú ) Time of a Node

1. A node counts its time from its anchor: the big_date of its newest beacon, added by psí1103r1, in seconds, the advance of a counter of the device that runs through sleep. The anchor, with the counter and the wall clock at that moment, is kept on the device.
2. Where the counter went backwards, or the wall clock moved against the counter, the node is unanchored: it takes its time from the wall clock and states anchored false in its moves, until its next beacon or the time of a connected node.
3. The id of a move shall not be earlier than its beacon.
4. A move whose id lies more than 60 seconds (Table K-1) after the big_date of its first receipt from an anchored node is refused.
5. A move under a certificate that has ended is accepted only with a receipt from an anchored node made before the certificate ended.
6. prev chains the moves of each device key, so that no move can be slipped in between two that were already shown.

A node can thus misstate the time of a move only within a stretch in which nobody saw it, and never to before a move of its own that somebody saw.

## ( u ) Kinds of Moves

Table I-2 presents the kinds of moves, the endpoint of § ( G ) that each takes the place of, who may make each, and what a node checks. A node applies every move at once, as final, unless a ruling strikes it. A member's node shall make a witness move for every remove and owner move of another pin that it receives. Read state is not carried in the mesh; it goes through messages.php?action=read alone.

<table id="table-i-2">
<caption>Table I-2 Kinds of Moves</caption>
<thead>
<tr><th>kind</th><th>In place of</th><th>Who may</th><th>A node checks</th></tr>
</thead>
<tbody>
<tr><td>make</td><td>talks/create_dm.php, talks/create_group.php</td><td>A user</td><td>The identification_numero_talkes is not held; for type 0, no NEGL between the two pins; the name of a group within Table K-1</td></tr>
<tr><td>send</td><td>messages.php?action=send</td><td>A member</td><td>Membership at that place in the fold; the content, the style and the attachments within Table K-1; the rate of § ( Í/( b ) )</td></tr>
<tr><td>edit</td><td>messages.php?action=edit</td><td>The sender</td><td>The pin of the signer is the pin in the target</td></tr>
<tr><td>emend</td><td>messages.php?action=edit by an editor</td><td>A named editor</td><td>The pin of the signer stands in that Emendetur block of the target; version is the current version; each body goes into its block (§ ( G/( d ) ))</td></tr>
<tr><td>delete</td><td>messages.php?action=delete</td><td>The sender</td><td>The pin of the signer is the pin in the target</td></tr>
<tr><td>like</td><td>messages.php?action=like</td><td>A member</td><td>The body carries the count of likes of the pin on the message after the move. Per pin and message, the move with the greatest id stands</td></tr>
<tr><td>add</td><td>talks/add_members.php</td><td>A member</td><td>The signer is a member; no NEGL known between the added pin and a member</td></tr>
<tr><td>remove</td><td>talks/remove_member.php</td><td>The owner</td><td>The signer is the owner at that place in the fold</td></tr>
<tr><td>leave</td><td>talks/leave.php</td><td>A member</td><td>The signer is a member</td></tr>
<tr><td>owner</td><td>talks/set_owner.php</td><td>The owner</td><td>The signer is the owner at that place in the fold; the target is a member</td></tr>
<tr><td>rename</td><td>talks/update_group.php</td><td>A member</td><td>The name within Table K-1</td></tr>
<tr><td>call</td><td>talks/call_log.php</td><td>The caller</td><td>The talk is of type 0</td></tr>
<tr><td>witness</td><td></td><td>A member other than the actor</td><td>The target is a remove or owner move; the signer is a member; anchored is true</td></tr>
</tbody>
</table>

## ( ù ) Hand-in, Receipts and Rulings

A node shall tell the pronal network every move it makes, when the pronal network is in reach. A talk never lives in the mesh alone.

1. Hand-in: any node gives the pronal network, by mesh/hand_in.php, the packets it holds, its own or carried, with their receipts. The pronal network opens each with the day key of its talk, checks it as a node does, and adds it to its record. A move whose id the pronal network already holds is passed over, so many nodes may hand in one packet. The pronal network limits hand-in by size and by node, not per message. A packet whose tag names no talk that the pronal network knows, as of a talk made in the mesh whose maker has not yet reached the pronal network, is held until the talk is handed in, and at most until its day leaves the window.
2. The pronal network's receipt: the pronal network answers with a signed statement of the hashes of the packets that it now holds. Nodes drop their carried copies of those packets and pass the receipt on.
3. The record: the pronal network stores the content as ENC2 under the day of the id, makes the search tokens and the mentions, gives the move its arrival number, and pushes the events of Table D-2. To an application that is not a node, a message made in the mesh is as any other.
4. Ruling: where a move breaks this document, the pronal network signs a ruling naming its id and the cause. Rulings are numbered by the pronal network across all talks, and travel sealed under the tag of their talk (§ ( I/( b ) )) and by mesh/rulings.php. A ruling may instead join two talks: it names id, the make move of the talk joined, and into, the identification_numero_talkes of the talk into which it is joined. The moves keep their bytes, and nodes fold the moves of both as one talk under into, in order of id. A node that takes a ruling skips that move in its fold from then on.
5. Checkpoint: after rulings, the pronal network signs a new checkpoint of the talk.

## ( g ) Other Rules

1. Unread goes by arrival (§ ( Ú/( ú ) )): every node and the pronal network number the messages of a talk as they arrive. A message that arrives late is unread, whatever its min.
2. A talk may be made while the pronal network is out of reach, where the nodes reach each other. The making node chooses the identification_numero_talkes at random, draws a day key of 32 random bytes for each day from today through the window, and gives them to the members. It hands the talk and those day keys to the pronal network by mesh/talk.php when the pronal network is in reach. The pronal network adopts the day keys for their days, draws the secret of the talk, and derives the day keys of later days from it, so the secret never leaves the pronal network. Where the identifier is already taken, the pronal network chooses a new one and joins the talk into it by a ruling (§ ( I/( ù ) )); the moves keep their bytes.
3. Typing travels only on a live link and is never stored. Status is not carried.
4. How long an application keeps talks on the device is its own choice.

## ( d ) Out of Sync

A node that reaches the pronal network and learns that rulings or a checkpoint would change something the user has already been shown — a message struck, the members or the owner different, an order changed — shall raise a prompt. Ordinary catching up raises none.

1. The prompt is one of the operating system, not drawn inside the application.
2. It offers to be corrected now, or to continue and be corrected later. Its words are chosen by the developer of each application.
3. To be corrected now, the node takes the rulings and the checkpoint and folds again at once. To continue, it keeps its own fold.
4. The user may ignore the prompt. The node shall connect back to the pronal network. The pronal network renews a certificate only for a node that has taken the current rulings (§ ( Ù/( ú ) )), so ignoring lasts at most the window.

An application that is not a node holds no fold of its own and raises no such prompt.

# ( Í ) THE FOLD

## ( a ) The State of a Talk

The state of a talk — its members, its owner, its name, and every message as it now reads — is computed, not stored as truth. A node starts from the newest checkpoint of the talk that the pronal network signed, takes every move after it in order (§ ( Í/( b ) )), checks each against the state so far, and applies it or skips it. This is the fold. Nodes that hold the same moves and receipts reach the same state, byte for byte.

A move that arrives late may change what was already shown. An application shall fold again and redraw, not only add at the end.

## ( b ) Order and Effect

Moves are taken in order of their ids, the 36 characters compared byte by byte, never by a collation.

A move that strikes others, remove or owner, takes effect at the earliest big_date among its counting receipts, not at its id, and is ordered by that big_date in place of the big_date of its id. A receipt counts only where it is a witness move of a member of the talk other than the actor, made by an anchored node, at the big_date of its id; or where it is the pronal network's receipt naming the hash of the move's packet, at its big_date. A striking move without a counting receipt is not yet applied. Moves that came before its effect stand. Nodes that hold different receipts may differ until the receipts reach them.

A send is skipped where the five kept sends of the same pin to the same talk before it all lie within the 5 seconds before its id (Table K-1).

Content is counted in code points, and names in bytes of UTF-8. Text is compared as bytes, without normalisation.

## ( ú ) Cases

Table Í-1 presents the cases that every fold shall decide alike.

<table id="table-í-1">
<caption>Table Í-1 Cases of the Fold</caption>
<thead>
<tr><th>Case</th><th>Rule</th></tr>
</thead>
<tbody>
<tr><td>Two moves of one big_date</td><td>Ordered by the pin, then by the prigotov, as the bytes of the id</td></tr>
<tr><td>Two talks of type 0 for one pair of users, each made apart</td><td>The talk whose make move has the lower id stands; the pronal network joins the other into it by a ruling, and the moves of both are folded as one talk</td></tr>
<tr><td>Two different moves with one id</td><td>Possible from one pin alone. The one whose bytes have the lower SHA-256 is kept; the other is skipped.</td></tr>
<tr><td>A target not yet held</td><td>Not applied in this fold, but kept. When the target arrives, the node folds again and applies it.</td></tr>
<tr><td>A target whose id is not below the id of the move</td><td>Refused</td></tr>
<tr><td>An id more than 60 seconds ahead of the time of the folding node</td><td>Kept aside, not skipped</td></tr>
<tr><td>An edit after a delete</td><td>Skipped; a delete ends a message</td></tr>
<tr><td>Two edits</td><td>Both applied in order; the later id stands</td></tr>
<tr><td>An emend against an older version</td><td>Skipped</td></tr>
<tr><td>The owner hands over, then removes the new owner</td><td>The removal is skipped: the signer is no longer owner</td></tr>
<tr><td>The owner removes a member, then hands over to that member</td><td>The hand-over is skipped: the target is no member</td></tr>
<tr><td>The two cases above from two nodes of the owner, apart</td><td>Ordered by effect (§ ( Í/( b ) )), then by id</td></tr>
<tr><td>A like and an unlike from two nodes of one pin</td><td>Per pin and message, the move with the greatest id stands. The count of likes of the message is the sum of the counts of the standing moves.</td></tr>
<tr><td>The add of a member; the remove of a non-member</td><td>Skipped, with no system message</td></tr>
<tr><td>A send by a pin between its removal and its adding again</td><td>Skipped</td></tr>
<tr><td>The pin in the id differs from the pin of the certificate</td><td>Refused</td></tr>
<tr><td>The day in the id differs from the day of the packet</td><td>Refused</td></tr>
<tr><td>Content of more than 4 000 code points</td><td>Refused</td></tr>
<tr><td>A move older than the checkpoint</td><td>Not folded by a node. Handed to the pronal network, which places it and signs a new checkpoint.</td></tr>
<tr><td>The owner leaves</td><td>The owner leaves as any member does, with the system message of a member who left; no other member becomes owner, as with talks/leave.php</td></tr>
</tbody>
</table>

# ( K ) LIMITS AND SUPPORT

## ( a ) Limits

Table K-1 presents the limits of the pronal network. The pronal network accepts what lies within them.

<table id="table-k-1">
<caption>Table K-1 Limits</caption>
<thead>
<tr><th>Item</th><th>Minimum</th><th>Maximum</th></tr>
</thead>
<tbody>
<tr><td>Content of a message</td><td>0 characters</td><td>4 000 characters</td></tr>
<tr><td>Style of a message</td><td></td><td>2 048 bytes</td></tr>
<tr><td>Requests to auth/token.php from one address</td><td></td><td>30 in 60 seconds</td></tr>
<tr><td>Messages of a user to one talk</td><td></td><td>5 in 5 seconds; for moves, counted by their ids</td></tr>
<tr><td>Publish of a user</td><td></td><td>10 in 5 seconds</td></tr>
<tr><td>Deviation of a min sent through § ( G )</td><td></td><td>60 seconds</td></tr>
<tr><td>Deviation of the id of a move from its first receipt</td><td></td><td>60 seconds</td></tr>
<tr><td>The window: life of a certificate, and of the day keys given ahead</td><td></td><td>7 days</td></tr>
<tr><td>Packet</td><td></td><td>10 485 848 bytes: the 60 bytes before the sealed bytes, and a file of 10 MiB sealed</td></tr>
<tr><td>Body of a control message of pronal://</td><td></td><td>262 144 bytes</td></tr>
<tr><td>Messages per get</td><td>1</td><td>100; 50 by default</td></tr>
<tr><td>Messages on each side in get_around</td><td></td><td>50</td></tr>
<tr><td>Name of a user</td><td>2 bytes</td><td>32 bytes</td></tr>
<tr><td>seperator_useres</td><td>1000</td><td>9999</td></tr>
<tr><td>Name of a group</td><td>1 byte</td><td>32 bytes</td></tr>
<tr><td>Query of users.php?action=search</td><td>2 bytes</td><td></td></tr>
<tr><td>Attachments per message</td><td></td><td>10</td></tr>
<tr><td>Size of a media attachment</td><td></td><td>10 MiB</td></tr>
<tr><td>Size of another attachment</td><td></td><td>5 MiB</td></tr>
<tr><td>Side of an image</td><td></td><td>1920 pixels, scaled down beyond</td></tr>
<tr><td>Ring of a call</td><td></td><td>45 seconds</td></tr>
<tr><td>Connections of a user to § ( D )</td><td></td><td>20</td></tr>
<tr><td>body of talks/sfu.php</td><td></td><td>65 536 bytes</td></tr>
<tr><td>Requests of a user to talks/sfu.php</td><td></td><td>240 in 60 seconds</td></tr>
<tr><td>Tracks of a member in a call in a group</td><td></td><td>8</td></tr>
<tr><td>data of gcall_mls</td><td></td><td>262 144 characters</td></tr>
</tbody>
</table>

## ( b ) Support

Table K-2 presents what an application shall support, at the least, and what it may support beyond that. An application that supports a part shall support it as this document sets out.

<table id="table-k-2">
<caption>Table K-2 Support</caption>
<thead>
<tr><th>Part</th><th>Application</th></tr>
</thead>
<tbody>
<tr><td>Authentication (§ ( Ù ))</td><td>Shall</td></tr>
<tr><td>Talks of type 0: send, get, read</td><td>Shall</td></tr>
<tr><td>Pushes: message_received, message_edited, message_deleted</td><td>Shall</td></tr>
<tr><td>Deleted messages treated as void</td><td>Shall</td></tr>
<tr><td>System messages shown or passed over</td><td>Shall</td></tr>
<tr><td>Orphan pin shown as given</td><td>Shall</td></tr>
<tr><td>Block and unblock</td><td>Shall</td></tr>
<tr><td>Edit, delete, reply, like</td><td>May</td></tr>
<tr><td>Groups and their owner</td><td>May</td></tr>
<tr><td>Friendships</td><td>May</td></tr>
<tr><td>Search</td><td>May</td></tr>
<tr><td>Attachments</td><td>May</td></tr>
<tr><td>Typing</td><td>May</td></tr>
<tr><td>Calls</td><td>May</td></tr>
<tr><td>Change of name</td><td>May</td></tr>
<tr><td>Avatars and icons shown</td><td>May</td></tr>
<tr><td>Status</td><td>May</td></tr>
<tr><td>The mesh: being a node (§ ( B/( u ) ), § ( Ż ) to § ( Í ))</td><td>May</td></tr>
<tr><td>For a node: hand-in of its moves and connecting back to the pronal network (§ ( I/( ù ) ), § ( I/( d ) ))</td><td>Shall</td></tr>
<tr><td>For a node: the prompt of § ( I/( d ) )</td><td>Shall</td></tr>
</tbody>
</table>

# ( L ) PRONO — INFORMATIVE

Prono is a pronal application. Prono makes the min of its messages itself, so that a message shows at once and its push is matched to it by the nonce. Prono shows the owner of a group.

On Windows, the node of Prono is node.exe of § ( Ż/( d ) ), which v31nhub puts into %LOCALAPPDATA%\pronal\ and keeps up to date. v31nhub sits in the tray, starts with Windows, and starts the node once the user has signed in at the V31N Portal through it; Prono desktop started without v31nhub starts the node itself. The client of v31nhub at the V31N Portal is admitted to the pronal network, and the node reaches the pronal network with the bearer token of that client: v31nhub keeps the refresh token of the user beside the node, sealed for the Windows user by DPAPI, and the node renews it. Prono desktop and other pronal applications on the PC use the node through § ( Ż/( d ) ). Prono shows the settings of Table Ż-3 last in its settings while link is true, and hides them otherwise. The page reaches the node by the messages of Table Ż-2. Where order is mesh, Prono makes each write of § ( G ) that has a kind in Table I-2 as a move and gives it to the node with need, and uses § ( G ) where no neighbour takes it; where order is api1, it uses § ( G ), and gives the move to the node only when the pronal network does not answer. Where the node holds no key for the pin of the page, the page works by § ( G ) alone. v31nhub targets .NET Framework 4.8; its HTTPS stays at TLS 1.2 or later.

On Android, the node is in the foreground service of the Prono APK, which gains the type connectedDevice. The node reaches the pronal network with the bearer token of the client of Prono, which it asks of the Prono server with the session of the page, as the page does. Other pronal applications on the phone use it through the bound service of § ( Ż/( d ) ), items 8 and 9. The node uses no Play services. The same messages as across the pipe cross the bridge between the page and the APK.

The talk layer of Prono is written once, in the JavaScript of the page, and the shells of Windows and Android carry it. The page never signs: the key stays in the store, and the page verifies with crypto.subtle.verify. Android opens a move natively to show a notification while the page is not running, and that code agrees with the JavaScript exactly.

QUIC is msquic on both platforms: on Windows its OpenSSL build, Microsoft.Native.Quic.MsQuic.OpenSSL, called from .NET Framework 4.8 by P/Invoke; on Android built from source with the NDK, for arm64-v8a and x86_64, from github.com/vpnhood/VpnHood.Net.Quic.MsQuic.AndroidNative, and called through JNI. msquic cannot use a key held in the TPM or in Android Keystore, hence the throwaway key of § ( Ż/( b ) ). The listener sets QUIC_CREDENTIAL_FLAG_REQUIRE_CLIENT_AUTHENTICATION, the certificate of the other side arrives by QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED as DER, and the throwaway credential is given as PKCS#12 in memory.

The device key is held, on Windows, in the TPM through the Microsoft Platform Crypto Provider of CNG, or else in the Microsoft Software Key Storage Provider; on Android, in Android Keystore. The counter of § ( I/( ú ) ) is QueryInterruptTime or GetTickCount64 on Windows, and SystemClock.elapsedRealtime on Android.

On Windows, DNS-SD is DnsServiceRegister and DnsServiceBrowse of dnsapi.dll, with the service registered under the PC's own name; on Android, NsdManager. The first time node.exe listens, Windows asks the user by its own prompt. Bluetooth is used on Android alone: Android 12 and later ask for BLUETOOTH_SCAN, declared neverForLocation, BLUETOOTH_ADVERTISE and BLUETOOTH_CONNECT; Android 13 and later ask for NEARBY_WIFI_DEVICES; Android 11 and 12 need ACCESS_FINE_LOCATION, with location switched on, for Wi-Fi Direct, and Android 11 also for the scan. The scan is filtered by the service identifier and restarted at a steady pace in its low-power mode. Wi-Fi Direct is joined by WifiP2pConfig.Builder with the name and passphrase of the group, with no dialog.

Carried packets are kept in one file per day, with an index beside it; moves of the user's own talks are kept in the store of the application, one file per talk. Prono keeps talks on the device as PLATFORMS.md states. The prompt of § ( I/( d ) ) is a TaskDialog of Win32 raised by node.exe on Windows and an AlertDialog of the system on Android, and its words are plain English. Port mapping by UPnP, NAT-PMP or PCP on desktops is a later addition.

# ( M ) GLOSSARY

<table>
<tbody>
<tr><td>Anchor</td><td>The beacon from which a node counts its time; see § ( I/( ú ) ).</td></tr>
<tr><td>Arrival number</td><td>The order in which the messages and moves of a talk arrived; see Table Ú-1.</td></tr>
<tr><td>Beacon</td><td>The pronal network's signed statement of its time and of the number of the current list; see Table Ù-2.</td></tr>
<tr><td>Bearer token</td><td>The token that the pronal network gives an application for a user, sent with every request; see § ( Ù ).</td></tr>
<tr><td>Certificate</td><td>The pronal network's signed statement binding a device key to a pin; see § ( Ù/( ú ) ).</td></tr>
<tr><td>Checkpoint</td><td>The pronal network's signed state of a talk at a point; see § ( Í/( a ) ).</td></tr>
<tr><td>Day key</td><td>The key of one talk for one day, held by the nodes of its members and by the pronal network; see § ( I/( b ) ).</td></tr>
<tr><td>Designated committer</td><td>The member of the MLS group of a call who adds and removes members; see § ( E/( ú ) ).</td></tr>
<tr><td>Device key</td><td>A P-256 key pair held in the key store of a device, one per pinpriv; see § ( Ù/( ú ) ).</td></tr>
<tr><td>Emendetur block</td><td>A part of a message that the users it names may edit; see § ( G/( d ) ).</td></tr>
<tr><td>ENC2</td><td>The encryption at rest of the content of messages; see § ( U/( ú ) ).</td></tr>
<tr><td>Fold</td><td>The computing of the state of a talk from a checkpoint and the moves after it; see § ( Í ).</td></tr>
<tr><td>Hand-in</td><td>The giving of packets to the pronal network by a node; see § ( I/( ù ) ).</td></tr>
<tr><td>List</td><td>The pronal network's signed, numbered list of certificates ended early; see Table Ù-2.</td></tr>
<tr><td>Move</td><td>One signed action in a talk; see § ( I/( a ) ).</td></tr>
<tr><td>Name card</td><td>The pronal network's signed statement of the name of a user at a time; see Table Ù-2.</td></tr>
<tr><td>NEGL</td><td>The state of a friendship in which one user has blocked the other; it binds both; see § ( U/( ù ) ).</td></tr>
<tr><td>Node</td><td>A pronal application that speaks pronal://; see § ( B/( u ) ).</td></tr>
<tr><td>Orphan pin</td><td>The pin 392000000001627, shown in place of every orphaned account.</td></tr>
<tr><td>Owner</td><td>The member of a group who may remove members and hand on the ownership; see § ( Ú/( ú ) ).</td></tr>
<tr><td>Packet</td><td>A sealed move or file as relays see it; see § ( Z/( a ) ).</td></tr>
<tr><td>Pass</td><td>A single-use secret by which an application authenticates to § ( D ).</td></tr>
<tr><td>Pronal network</td><td>The telecommunications database and system of Prinz; see § ( B/( a ) ).</td></tr>
<tr><td>Pronal application</td><td>A programme that talks with the pronal network and authenticates through the V31N Portal.</td></tr>
<tr><td>Pronal network's receipt</td><td>The pronal network's signed statement of the hashes of the packets it now holds; see § ( I/( ù ) ).</td></tr>
<tr><td>Prono</td><td>The Services for þe General populace Maß Tele‑kommunikation services namen as prono, a pronal application.</td></tr>
<tr><td>Receipt</td><td>A node's signed statement that it held a packet of a talk it is not in, at a time; see § ( Z/( ú ) ).</td></tr>
<tr><td>Relay layer</td><td>The layer of a node that carries packets and knows nothing of talks; see § ( Z ).</td></tr>
<tr><td>Room</td><td>The group of connections of § ( D ) that receive the events of one talk.</td></tr>
<tr><td>Ruling</td><td>The pronal network's signed statement that a move is struck, with the cause; see § ( I/( ù ) ).</td></tr>
<tr><td>Safety number</td><td>The first 32 bytes of the epoch authenticator of a call in a group, by which users compare their keys; see § ( E/( ú ) ).</td></tr>
<tr><td>Status</td><td>Whether a user is online or offline, as a state of which OFFLINE alone means offline; see § ( D ).</td></tr>
<tr><td>State</td><td>The user, the people the user meets, the friendships and the talks, as an application shows them; see § ( G/( g ) ).</td></tr>
<tr><td>Tag</td><td>The opaque name of a talk and a day on a packet; see § ( I/( b ) ).</td></tr>
<tr><td>Talk</td><td>A conversation of the pronal network, of a type of Table Ú-2.</td></tr>
<tr><td>Talk layer</td><td>The layer of a node that makes, seals, opens and folds moves; see § ( I ).</td></tr>
<tr><td>V31N Portal</td><td>The service of sign-on and eID at portal.v31null.de.</td></tr>
<tr><td>Window</td><td>7 days: how long a certificate lasts without its node seeing the pronal network; see Table K-1.</td></tr>
<tr><td>Witness</td><td>The node that made a receipt or a witness move; see § ( Z/( ú ) ) and Table I-2.</td></tr>
</tbody>
</table>

# ( N ) REFERENCES

Bluetooth SIG (n.d.) "Bluetooth Core Specification".

Cloudflare (n.d.) "Realtime SFU API".

Internet Engineering Task Force (IETF) (2015) "RFC 7636: Proof Key for Code Exchange by OAuth Public Clients".

Internet Engineering Task Force (IETF) (2011) "RFC 6455: The WebSocket Protocol".

Internet Engineering Task Force (IETF) (2016) "RFC 7741: RTP Payload Format for VP8 Video".

Internet Engineering Task Force (IETF) (2022) "RFC 9110: HTTP Semantics".

Internet Engineering Task Force (IETF) (2023) "RFC 9420: The Messaging Layer Security (MLS) Protocol".

Internet Engineering Task Force (IETF) (2013) "RFC 6762: Multicast DNS".

Internet Engineering Task Force (IETF) (2013) "RFC 6763: DNS-Based Service Discovery".

Internet Engineering Task Force (IETF) (2018) "RFC 8446: The Transport Layer Security (TLS) Protocol Version 1.3".

Internet Engineering Task Force (IETF) (2020) "RFC 8489: Session Traversal Utilities for NAT (STUN)".

Internet Engineering Task Force (IETF) (2021) "RFC 9000: QUIC: A UDP-Based Multiplexed and Secure Transport".

Internet Engineering Task Force (IETF) (2021) "RFC 9001: Using TLS to Secure QUIC".

Microsoft (n.d.) "MsQuic: Cross-platform, C implementation of the IETF QUIC protocol".

National Institute of Standards and Technology (NIST) (2007) "SP 800-38D: Recommendation for Block Cipher Modes of Operation: Galois/Counter Mode (GCM) and GMAC".

National Institute of Standards and Technology (NIST) (2023) "FIPS 186-5: Digital Signature Standard (DSS)".

OpenID Foundation (2014) "OpenID Connect Core 1.0".

Unicode Consortium (2026) "The Unicode Standard, Version 18.0".

VpnHood (n.d.) "VpnHood.Net.Quic.MsQuic.AndroidNative".

W3C (n.d.) "WebRTC: Real-Time Communication in Browsers".

Wi-Fi Alliance (n.d.) "Wi-Fi Peer-to-Peer (P2P) Technical Specification".

PS/Í-1103
