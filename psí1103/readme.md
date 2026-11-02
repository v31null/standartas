<table>
<thead>
<tr><th colspan="6">Technical Report Documentation Page</th></tr>
</thead>
<tbody>
<tr><td colspan="3">1. Report No.<br>PS/Í-1103 R.1</td><td colspan="3">2. Report Date<br>2026/11/03</td></tr>
<tr><td colspan="6">3. Title<br>V0309S STANDARD</td></tr>
<tr><td colspan="6">4. Author(s)<br>P.K. — 619851078911877 , kokain@v31null.de</td></tr>
<tr><td colspan="3">a. Performing Organization Name(s) and Address(es)<br>Mbrlí úír Entípatùíțù fùțbíz — v31null.de , mef@v31null.de</td><td colspan="3">b. Sponsoring/Monitoring Agency Name(s) and Address(es)</td></tr>
<tr><td colspan="6">6. Abstract<br>This document is the implementation documentation of the law « JST shall be used. » and defines the V0309S STANDARD, the time keeping of v31n. It sets out what a date is, namely the calendar, the fields of the clock, the day of 86,400 labels and the true turn of the Earth within it, the last minute of the day and the extra seconds that follow from the stellar day and from the official leap seconds, the baseline of Japan Standard Time and the offsets taken from it, and the civil date. It sets out how dates are written, in the sort key and the original form, and which stamps shall not be accepted, and how dates are formatted for people, in the display line and the calendar of a month. It closes with considerations concerning databases, validation, precision and localisation.</td></tr>
<tr><td colspan="3" rowspan="2">7. Key Words<br>V0309S STANDARD, JST, time keeping, date, stamp, sort key, original form, display line, stellar day, leap minute, leap second, Japan Standard Time, offset, Julian Day Number, civil date</td><td colspan="3">8. Distribution Statement</td></tr>
<tr><td>a. Report<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>b. Abstract<br>A; Approved for public release: distribution is unlimited, no restrictions.</td><td>ú. This page<br>A; Approved for public release: distribution is unlimited, no restrictions.</td></tr>
<tr><td colspan="3">9. Security Classif. (of this document)<br><b>UNCLASSIFIED</b></td><td colspan="3">10. №ⁱ OF PP.<br>14</td></tr>
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
- [( B ) THE DATE](#bthe-date)
  - [( a ) The Calendar](#athe-calendar)
  - [( b ) Fields](#bfields)
  - [( ú ) The Day](#úthe-day)
  - [( u ) The Last Minute](#uthe-last-minute)
    - [( i ) The Stellar Second](#ithe-stellar-second)
    - [( ii ) The Official Leap Second](#iithe-official-leap-second)
  - [( ù ) Baseline and Offsets](#ùbaseline-and-offsets)
  - [( g ) The Civil Date](#gthe-civil-date)
- [( Ú ) WRITING DATES](#úwriting-dates)
  - [( a ) Forms](#aforms)
  - [( b ) The Sort Key](#bthe-sort-key)
  - [( ú ) The Original Form](#úthe-original-form)
  - [( u ) Stamps Not Accepted](#ustamps-not-accepted)
- [( U ) FORMATTING DATES](#uformatting-dates)
  - [( a ) The Display Line](#athe-display-line)
  - [( b ) The Calendar](#bthe-calendar)
- [( Ù ) CONSIDERATIONS](#ùconsiderations)
  - [( a ) Introduction](#aintroduction-1)
  - [( b ) Databases](#bdatabases)
  - [( ú ) Validation](#úvalidation)
  - [( u ) Precision](#uprecision)
  - [( ù ) Localisation](#ùlocalisation)
- [( G ) GLOSSARY](#gglossary)
- [( D ) REFERENCES](#dreferences)

## ( b ) List of Tables

- [Table B-1 Fields of the Clock](#table-b-1)
- [Table B-2 Readings of the Day](#table-b-2)
- [Table B-3 Length of the Last Minute](#table-b-3)
- [Table B-4 Days Carrying an Official Leap Second](#table-b-4)
- [Table B-5 Offset Strings](#table-b-5)
- [Table B-6 Civil Months](#table-b-6)
- [Table Ú-1 Forms of a Stamp](#table-ú-1)
- [Table Ú-2 Sort Keys and Their Order](#table-ú-2)
- [Table Ú-3 Stamps Not Accepted](#table-ú-3)
- [Table U-1 Display Lines](#table-u-1)
- [Table U-2 Calendar of June 2026](#table-u-2)
- [Table Ù-1 Columns of a Table of Events](#table-ù-1)

## ( ú ) Notice

The contents of this document reflect the views of the author, who is responsible for the facts and the accuracy of the data presented herein. This document sets the V0309S STANDARD. However, v31n does not act as a confirmation agency. The descriptions of programmes herein are provided for informational purposes, and developers are tasked with the conformance of their own programmes with this standard. V31null does not endorse products or manufacturers. Trade or manufacturer's names appear herein only because they are considered essential to the objective of this document. v31n is not related to non official platforms, services, people, & ‘oc genus omne in any way or form. Textual, programmatic, and media content published across these services is made available under the terms of the v31n license. No statutory copyright, proprietary monopoly, or reservation of rights subsists in these works. Any person is granted full, unconditional, and irrevocable permission to access, copy, modify, adapt, distribute, or incorporate all materials for any purpose, without formality, fee, or prior authorization. The constitutive Latin charter and legal terms of this declaration are recorded in the system registry and resp. places.

## ( u ) Foreword

This document is the implementation documentation of the law « JST shall be used. », on which it rests. It defines the V0309S STANDARD, the time keeping of v31n: what a date is, how a date is written, and how a date is formatted. It applies to every person dealing with Prinzian systems and with native systems of v31n.

What a date is is defined in § ( B ), which sets out the calendar, the fields of the clock, the day, its last minute, the baseline from which offsets are taken, and the civil date. § ( Ú ) sets out how dates are written, in the sort key and the original form, and which stamps shall not be accepted. § ( U ) sets out how dates are formatted for people, in the display line and the calendar of a month. § ( Ù ) sets out the considerations that follow from § ( B ) to § ( U ), concerning databases, validation, precision and localisation. § ( G ) gives the terms used in this document, and § ( D ) the documents to which it refers.

P.K.

## ( ù ) Scope and Purpose

Time, as kept by v31n, is taken from Japan Standard Time under the law « JST shall be used. » and written in the notation of this document. The day is counted in 86,400 labels and marks the moment at which the Earth has turned once against the fixed stars. The labels after that moment are padding.

This document covers modern dates, the years 1753 to 9999 of the Prinzo-gregorian calendar. Earlier and later years, the older styles of the calendar and other renderings of a date are outside this document.

## ( g ) Notation

Readings of the clock are written hours, minutes and seconds, separated by colons and followed by the meridiem, as in 00:01:01 AM. Stamps are written as they are stored, as in 20260612A0001001. Numbers of more than four digits are grouped by commas, as in 86,400.

A reference to a part of this document gives the sign § followed by the division and the part, as in § ( B/( u.i. ) ), which denotes ( i ) of ( u ) of ( B ). Tables are numbered by division, as in Table B-3 and Table Ú-1.

The requirements of this document are stated with shall, and each binds unless this document states otherwise. Should denotes a recommendation, which does not bind, and may denotes a permission.

# ( B ) THE DATE

## ( a ) The Calendar

A date is a day of the Prinzo-gregorian calendar, in which the Gregorian rule applies: a year divisible by 4 is a leap year, except a year divisible by 100 and not by 400. The years run from 1753 to 9999, each written in four digits.

Days are counted by the Julian Day Number, the count of days used in astronomy, which gives every day one whole number. The Julian Day Number is the count over which the stellar second of § ( B/( u.i. ) ) is taken, and the difference of two Julian Day Numbers is the number of days between two dates.

## ( b ) Fields

A date with its time consists of the year, the month and the day, and of the hour, the minute, the second and the meridiem. Table B-1 presents the fields and their values.

<table id="table-b-1">
<caption>Table B-1 Fields of the Clock</caption>
<thead>
<tr><th>Field</th><th>Values</th><th>Kind</th></tr>
</thead>
<tbody>
<tr><td>Year</td><td>1753 to 9999</td><td>Count of the calendar</td></tr>
<tr><td>Month</td><td>01 to 12</td><td>Ordinal</td></tr>
<tr><td>Day</td><td>01 to 31, as the month allows</td><td>Ordinal</td></tr>
<tr><td>Hour</td><td>00 to 11</td><td>Cardinal</td></tr>
<tr><td>Minute</td><td>01 to 60</td><td>Ordinal</td></tr>
<tr><td>Second</td><td>01 to 60; in the last minute of the day 01 to 240, 241 or 242</td><td>Ordinal</td></tr>
<tr><td>Meridiem</td><td>AM, PM</td><td>Ordinal</td></tr>
</tbody>
</table>

The meridiem, the minute and the second are ordinal labels. AM is the first half of the day and PM the second. The minute in progress is the first minute, and the second in progress is the first second, so that the labels run from 01 to 60 and the last label equals the count. The hour counts completed hours, a cardinal, so that during the first hour of each half of the day no hour is complete and the hour reads 00. The distinction between the ordinal and the cardinal number is that drawn by Cantor in Article I, §§ 1 and 7, and Article II, § 14, of the Beiträge zur Begründung der transfiniten Mengenlehre, and the count from 1 follows the natural numbers as set out by Peano, whose first axiom places 1 among them.

## ( ú ) The Day

A day begins at 00:01:01 AM, when no hour is complete and the first minute and the first second are in progress. Each hour holds 60 minutes of 60 seconds. The second after 11:60:60 AM is noon, read 00:01:01 PM, and the afternoon runs as the morning does, until the last minute of the day. Table B-2 presents the readings that mark the day.

<table id="table-b-2">
<caption>Table B-2 Readings of the Day</caption>
<thead>
<tr><th>Reading</th><th>Seconds since the start of the day</th><th>Meaning</th></tr>
</thead>
<tbody>
<tr><td>00:01:01 AM</td><td>1</td><td>First second of the day</td></tr>
<tr><td>11:60:60 AM</td><td>43,200</td><td>Last second before noon</td></tr>
<tr><td>00:01:01 PM</td><td>43,201</td><td>Noon</td></tr>
<tr><td>11:57:01 PM</td><td>86,161</td><td>First second of the last minute</td></tr>
<tr><td>11:57:04 PM</td><td>86,164</td><td>The Earth has turned once</td></tr>
<tr><td>11:57:240 PM</td><td>86,400</td><td>Last second of a day of 86,400 seconds</td></tr>
</tbody>
</table>

The Earth turns once against the fixed stars in the stellar day of 86,164.098903691 seconds; the true seconds of the day run out at 11:57:04 PM. The day holds 86,400 seconds. The last minute of the day, the 57ᵗʰ minute of hour 11 PM, holds the remaining 236 counts as padding and runs to 240. The second after the last second of the last minute is 00:01:01 AM of the next day.

## ( u ) The Last Minute

The last minute of the day holds 240 seconds. It holds one second more on a day that carries a stellar second, as set out in § ( B/( u.i. ) ), and one second more on a day that carries an official leap second, as set out in § ( B/( u.ii. ) ). The two stack. Table B-3 presents the length of the last minute in each case.

<table id="table-b-3">
<caption>Table B-3 Length of the Last Minute</caption>
<thead>
<tr><th>Stellar second</th><th>Official leap second</th><th>Last second of the day</th><th>Seconds in the day</th></tr>
</thead>
<tbody>
<tr><td>No</td><td>No</td><td>11:57:240 PM</td><td>86,400</td></tr>
<tr><td>Yes</td><td>No</td><td>11:57:241 PM</td><td>86,401</td></tr>
<tr><td>No</td><td>Yes</td><td>11:57:241 PM</td><td>86,401</td></tr>
<tr><td>Yes</td><td>Yes</td><td>11:57:242 PM</td><td>86,402</td></tr>
</tbody>
</table>

No system shall assume a fixed length for the last minute. A count of seconds across a day that carries an extra second holds more than 86,400 seconds. A field that holds the second shall hold three digits.

### ( i ) The Stellar Second

The stellar day exceeds 86,164 seconds by 0.098903691 of a second. It is counted from day to day, and on each day on which the count completes a whole second, the day carries one stellar second, so that one stellar second falls about every 10.1108 days and about 36.12 fall in a year. The count is taken over the Julian Day Number of the date, from day 0: the day with the Julian Day Number j carries a stellar second when the whole part of j × 0.098903691 exceeds the whole part of (j − 1) × 0.098903691. The gaps between such days are of 10 and of 11 days.

The sidereal day of 86,164.0905 seconds, measured against the equinox, shall not be used.

### ( ii ) The Official Leap Second

A day on which the International Earth Rotation and Reference Systems Service inserted a leap second into Coordinated Universal Time carries an official leap second. The leap second is inserted at the end of 30 June or 31 December in Coordinated Universal Time, which is 1 July or 1 January at the baseline, and the day of the baseline on which it falls carries it in its last minute. Leap seconds are announced by the service in its Bulletin C. Table B-4 presents the days that carry an official leap second, to the date of this document. On four of them a stellar second falls as well, 1 January 1977, 1 July 1994, 1 January 2009 and 1 January 2017, and these days end at 11:57:242 PM.

<table id="table-b-4">
<caption>Table B-4 Days Carrying an Official Leap Second</caption>
<thead>
<tr><th>Day</th><th>Years</th></tr>
</thead>
<tbody>
<tr><td>1 January</td><td>1973, 1974, 1975, 1976, 1977, 1978, 1979, 1980, 1988, 1990, 1991, 1996, 1999, 2006, 2009, 2017</td></tr>
<tr><td>1 July</td><td>1972, 1981, 1982, 1983, 1985, 1992, 1993, 1994, 1997, 2012, 2015</td></tr>
</tbody>
</table>

## ( ù ) Baseline and Offsets

The baseline is Japan Standard Time, kept at the Akashi Municipal Planetarium, as the law « JST shall be used. » requires. Every date is a date at the baseline unless an offset is given.

An offset is written as a string of hours relative to the baseline. A leading M denotes minus. A tail of the form F, numerator, TO, denominator adds a fraction of an hour. Table B-5 presents examples of offset strings.

<table id="table-b-5">
<caption>Table B-5 Offset Strings</caption>
<thead>
<tr><th>String</th><th>Offset from the baseline</th></tr>
</thead>
<tbody>
<tr><td>3</td><td>+3 hours</td></tr>
<tr><td>M3</td><td>−3 hours</td></tr>
<tr><td>3F1TO2</td><td>+3.5 hours</td></tr>
<tr><td>M1F1TO4</td><td>−1.25 hours</td></tr>
<tr><td>M7</td><td>−7 hours, Tel Aviv, standard time</td></tr>
<tr><td>M8</td><td>−8 hours, Berlin, standard time</td></tr>
<tr><td>M15</td><td>−15 hours, Chicago, standard time</td></tr>
</tbody>
</table>

## ( g ) The Civil Date

The civil date is an administrative date for official purposes. It is a single digit from 0 to 9 that names the tenth of the year in which a day falls. The 365 days of the year are divided into ten civil months of 36 or 37 days, and the tenth civil month carries the digit 0. 29 February is counted as 32 December, so that in a leap year the first nine civil months are unchanged and the tenth runs 38 days. The digit follows from the month and the day alone. A civil date names a span of days and not a day. Table B-6 presents the civil months.

<table id="table-b-6">
<caption>Table B-6 Civil Months</caption>
<thead>
<tr><th>Digit</th><th>Days</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>1 January to 5 February</td></tr>
<tr><td>2</td><td>6 February to 14 March</td></tr>
<tr><td>3</td><td>15 March to 19 April</td></tr>
<tr><td>4</td><td>20 April to 26 May</td></tr>
<tr><td>5</td><td>27 May to 1 July</td></tr>
<tr><td>6</td><td>2 July to 7 August</td></tr>
<tr><td>7</td><td>8 August to 12 September</td></tr>
<tr><td>8</td><td>13 September to 19 October</td></tr>
<tr><td>9</td><td>20 October to 24 November</td></tr>
<tr><td>0</td><td>25 November to 31 December, and 29 February</td></tr>
</tbody>
</table>

# ( Ú ) WRITING DATES

## ( a ) Forms

A stamp is a date with its time written as a string. A date is written in two forms, the sort key and the original form. Table Ú-1 presents the forms.

<table id="table-ú-1">
<caption>Table Ú-1 Forms of a Stamp</caption>
<thead>
<tr><th>Form</th><th>Layout</th><th>Example</th><th>Last minute</th></tr>
</thead>
<tbody>
<tr><td>Sort key</td><td>year, month, day, A or P, hour, minute, second of 3 digits</td><td>20260612A0001001</td><td>20260611P1157240</td></tr>
<tr><td>Original form</td><td>year, month, day, hour, minute, second, AM or PM</td><td>20260612000101AM</td><td>202606111157240PM</td></tr>
</tbody>
</table>

In both forms the year is four digits, and the month, the day, the hour and the minute are two digits each. The meridiem is required. Two stamps of the two forms name the same instant when they carry the same fields.

## ( b ) The Sort Key

The sort key is the form that shall be stored, indexed, sorted and sent between systems. It runs from the largest field to the smallest, with a meridiem of one letter, A or P, and a second of three digits, padded with zeros, so that every field holds the same position in every key. A sort key is always 16 characters long, the last minute of the day included.

Alphabetical order of sort keys is chronological order. Table Ú-2 presents sort keys in their alphabetical order, which is also their order in time.

<table id="table-ú-2">
<caption>Table Ú-2 Sort Keys and Their Order</caption>
<thead>
<tr><th>Sort key</th><th>Display line</th></tr>
</thead>
<tbody>
<tr><td>20260611P1157240</td><td>2026 06⁄11 11:57:240 PM</td></tr>
<tr><td>20260612A0001001</td><td>2026 06⁄12 00:01:01 AM</td></tr>
<tr><td>20260612A0931001</td><td>2026 06⁄12 09:31:01 AM</td></tr>
<tr><td>20260612P0031001</td><td>2026 06⁄12 00:31:01 PM</td></tr>
<tr><td>20260612P1157100</td><td>2026 06⁄12 11:57:100 PM</td></tr>
</tbody>
</table>

## ( ú ) The Original Form

The original form writes the date, then the time, then the meridiem of two letters, AM or PM. The second is two digits, and three digits in the last minute from its 100ᵗʰ second. The original form may be accepted as input. It shall not be used where stamps are compared or sorted: 20260612003101PM sorts ahead of 20260612093101AM, though it is the later instant.

## ( u ) Stamps Not Accepted

No system shall accept the stamps of Table Ú-3, and every system should refuse them. A system should not move such a stamp onto a nearby reading: a minute or a second of 00, in particular, should not be read as a step back across the boundary before it.

<table id="table-ú-3">
<caption>Table Ú-3 Stamps Not Accepted</caption>
<thead>
<tr><th>Stamp</th><th>Example</th></tr>
</thead>
<tbody>
<tr><td>A minute or a second of 00</td><td>20260612000100AM</td></tr>
<tr><td>A minute or a second above 60, outside the last minute</td><td>20260612A0061001</td></tr>
<tr><td>A second above the length of the last minute of that day</td><td>20260612P1157241</td></tr>
<tr><td>A date that does not exist in the calendar</td><td>20260230A0001001, 20210229A0001001</td></tr>
<tr><td>A stamp without a meridiem</td><td>20260612000101</td></tr>
<tr><td>A sort key with a second of fewer than three digits</td><td>20260612A000101</td></tr>
<tr><td>A display line given as a stamp</td><td>2026 06⁄12 00:01:01 AM</td></tr>
</tbody>
</table>

# ( U ) FORMATTING DATES

## ( a ) The Display Line

The display line is the form in which a date is shown to people. It writes the year, a space, the month and the day separated by the fraction slash ⁄, a space, the hour, the minute and the second separated by colons, a space, and the meridiem of two letters. The hour is shown as it is, from 00 to 11, and the second of the last minute is shown with three digits from its 100ᵗʰ second. Table U-1 presents display lines.

<table id="table-u-1">
<caption>Table U-1 Display Lines</caption>
<thead>
<tr><th>Sort key</th><th>Display line</th></tr>
</thead>
<tbody>
<tr><td>20260612A0001001</td><td>2026 06⁄12 00:01:01 AM</td></tr>
<tr><td>20260612A1160060</td><td>2026 06⁄12 11:60:60 AM</td></tr>
<tr><td>20260612P0001001</td><td>2026 06⁄12 00:01:01 PM</td></tr>
<tr><td>20260612P1157099</td><td>2026 06⁄12 11:57:99 PM</td></tr>
<tr><td>20260612P1157240</td><td>2026 06⁄12 11:57:240 PM</td></tr>
</tbody>
</table>

The display line is for people only. It should not be stored, nor sent to another system as a stamp.

## ( b ) The Calendar

Every month shall start from day 1 in the first column of its calendar, and its days shall follow one another without gaps, seven to a row. The calendar of a month is headed by the name of the month. The heading of each column names the weekday of the days below it, taken from days 1 to 7 of the month, so that the headings begin with the weekday of day 1. The last row is closed by empty cells. Table U-2 presents June 2026.

<table id="table-u-2">
<caption>Table U-2 Calendar of June 2026</caption>
<thead>
<tr><th colspan="7">June</th></tr>
<tr><th>Mon</th><th>Tue</th><th>Wed</th><th>Thu</th><th>Fri</th><th>Sat</th><th>Sun</th></tr>
</thead>
<tbody>
<tr><td>1</td><td>2</td><td>3</td><td>4</td><td>5</td><td>6</td><td>7</td></tr>
<tr><td>8</td><td>9</td><td>10</td><td>11</td><td>12</td><td>13</td><td>14</td></tr>
<tr><td>15</td><td>16</td><td>17</td><td>18</td><td>19</td><td>20</td><td>21</td></tr>
<tr><td>22</td><td>23</td><td>24</td><td>25</td><td>26</td><td>27</td><td>28</td></tr>
<tr><td>29</td><td>30</td><td colspan="5"></td></tr>
</tbody>
</table>

# ( Ù ) CONSIDERATIONS

## ( a ) Introduction

This division sets out what § ( B ) to § ( U ) require of systems that store, convert, validate and show dates.

## ( b ) Databases

Stamps shall be stored as sort keys in a text column. v31n stores sort keys in a column of VARCHAR(24). A native date or time column of a database shall not be used. Where the number of days between rows is needed, a column holding the Julian Day Number of the date may be written beside the sort key when the row is written. Table Ù-1 presents the columns of a table of events.

<table id="table-ù-1">
<caption>Table Ù-1 Columns of a Table of Events</caption>
<thead>
<tr><th>Column</th><th>Type</th><th>Contents</th></tr>
</thead>
<tbody>
<tr><td>id</td><td>INTEGER PRIMARY KEY</td><td>Row number</td></tr>
<tr><td>title</td><td>TEXT NOT NULL</td><td>Name of the event</td></tr>
<tr><td>stamp</td><td>VARCHAR(24) NOT NULL</td><td>Sort key</td></tr>
<tr><td>jdn</td><td>INTEGER</td><td>Julian Day Number of the date, where needed</td></tr>
</tbody>
</table>

ORDER BY stamp returns rows in time order. A range of dates is selected by the date digits as bounds. The number of days between two rows is the difference of their jdn columns.

Sort keys and stamps of another form should not be kept in one column. Data held in another form is converted once, row by row, into sort keys. Validation happens before the row is written.

## ( ú ) Validation

Input should be checked against § ( Ú/( u ) ) before it is accepted: minutes and seconds from 01 to 60, the seconds of the last minute up to the length of that day, hours from 00 to 11, and a date that exists in the calendar.

## ( u ) Precision

The second is the smallest unit of time. Two events within one second carry the same stamp, and the order between them is decided by the database, through its own locking, transactions and rules of conflict.

## ( ù ) Localisation

A stamp is stored at the baseline. A local reading shall not be stored. The offset of the user is applied where the user reads the date. The offset of a locality follows the law of its state. The offset shall not be fixed globally for a service whose users all live in one country. The mapping of a place to its offset string, and of a display line to another language, is made where the date is shown. A system should not apply daylight saving on its own judgment. Where the user chooses daylight saving, exactly one hour is added on top of the offset. Any other rule of a locality is expressed by the offset string.

# ( G ) GLOSSARY

<table>
<tbody>
<tr><td>Baseline</td><td>Japan Standard Time, kept at the Akashi Municipal Planetarium, from which every date and offset is taken.</td></tr>
<tr><td>Civil date</td><td>The digit from 0 to 9 naming the tenth of the year in which a day falls; see Table B-6.</td></tr>
<tr><td>Display line</td><td>The form in which a date is shown to people; not a stamp.</td></tr>
<tr><td>JST</td><td>Japan Standard Time; the baseline, under the law « JST shall be used. »</td></tr>
<tr><td>JDN</td><td>Julian Day Number; the count of days used in astronomy.</td></tr>
<tr><td>Last minute</td><td>The 57ᵗʰ minute of hour 11 PM, which holds 240, 241 or 242 seconds and closes the day.</td></tr>
<tr><td>Meridiem</td><td>AM or PM; in the sort key A or P.</td></tr>
<tr><td>Offset string</td><td>A string giving hours relative to the baseline; see Table B-5.</td></tr>
<tr><td>Official leap second</td><td>A second inserted into Coordinated Universal Time by the International Earth Rotation and Reference Systems Service; see Table B-4.</td></tr>
<tr><td>Original form</td><td>The form of a stamp with the meridiem at its end; accepted as input, not sorted.</td></tr>
<tr><td>Padding</td><td>The counts of the last minute after 11:57:04 PM.</td></tr>
<tr><td>Sidereal day</td><td>The rotation of the Earth against the equinox, 86,164.0905 seconds; shall not be used.</td></tr>
<tr><td>Sort key</td><td>The form of a stamp of which alphabetical order is chronological order; the form that is stored.</td></tr>
<tr><td>Stamp</td><td>A date with its time written as a string.</td></tr>
<tr><td>Stellar day</td><td>The rotation of the Earth against the fixed stars, 86,164.098903691 seconds.</td></tr>
<tr><td>Stellar second</td><td>The second added to the last minute on each day on which the fraction 0.098903691 of the stellar day, counted over the JDN, completes a whole second.</td></tr>
</tbody>
</table>

# ( D ) REFERENCES

« JST shall be used. »

Cantor, G. (1895) "Beiträge zur Begründung der transfiniten Mengenlehre", Article I, §§ 1 and 7, Mathematische Annalen 46, from p. 500, Göttinger Digitalisierungszentrum, https://gdz.sub.uni-goettingen.de/download/pdf/PPN235181684_0046/PPN235181684_0046.pdf.

Cantor, G. (1897) "Beiträge zur Begründung der transfiniten Mengenlehre", Article II, § 14, Mathematische Annalen 49.

Cantor, G. (1915) "Contributions to the Founding of the Theory of Transfinite Numbers", translated by P. E. B. Jourdain, Open Court, Chicago and London, archive.org, https://archive.org/stream/contributionstof00cant/contributionstof00cant_djvu.txt.

International Earth Rotation and Reference Systems Service (IERS) (n.d.) "Useful Constants".

International Earth Rotation and Reference Systems Service (IERS) (n.d.) "Bulletin C".

Peano, G. (1889) "Arithmetices principia, nova methodo exposita", Turin.
