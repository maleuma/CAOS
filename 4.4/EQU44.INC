;*********************************
;**				**
;**	CAOS 4.4   (c) ML-Soft	**
;**				**
;*********************************
;**				**
;**	Mario Leubner		**
;**	Dorfstr. 3		**
;**	09236 Claussnitz	**
;**	---------------		**
;**				**
;**	von:	13.04.1994	**
;**	bis:	09.02.2003	**
;**				**
;*********************************
 
;Neu in CAOS 4.4 (gegen}ber 4.3):
;
;fehlenden Befehl LD (IX+7),0 in
;Routine CSRI wieder eingesetzt.
;
;Fehler PWRON initialisiert Kassetten-
;puffer nicht wurde behoben.
;
;Scannermodul in Modul-Liste definiert.
;
;BASIC:	CLOSE ohne ge|ffneten Kanal
;	korrigiert.
;
;V.24-Software verbessert:
;	A800 enth{lt jetzt Modulsteckpl.
;	des ersten V.24-Moduls,
;	(Empfangsinterrupt schaltet
;	das Modul bei Bedarf zu und ab)
;	korrigierte Interruptroutinen
;	bez}glich ESC-T und ESC-U:
;	definierter SPeicherzustand
;	IRM ist EIN, CAOS-C ist AUS
 
;Hardware:	(KC85/5)
;
;256 KByte interner RAM (RAM0,RAM4,RAM8)
; 64 KByte IRM (2 unabh{ngige Bilder)
; 16 KByte CAOS-ROM C000-FFFF
; 32 KByte USER-ROM C000-DFFF (4 Ebenen)
;
;---------------------------------------
;
;**	Speicher}bersicht	**
 
;Adr.	Label:
;C000	KCTAB	IRM-Defaulttabelle
;C04F	PWRONC	Power-ON Initialisierung
;C086	SYSI	SYSI, Portinittabellen
;C124	SYSTC	SYSTEM-Check
;C217	MODULC	MODUL-Steuerung, Liste
;C46E	KEYC	KEY, KEYLIST
;C564	SQRC	SQR, MULT, COLOR
;C5B5	WINDC	WINDOW, WINAK, WININ
;C63E	LINEC	LINE, PUSE, CIRCLE, PUDE
;C7EE	DPMEMO	DISPLAY, MODIFY
;C8B4	CENC	CEN, V24OUT, V24-Suche
;CA5C	ZIBM	CAOS->IBM-Codewandeltab.
;CA6A	HCPGMC	ShCLR-Reaktionsprogramm
;CAA9	COPIES	Hardcopyroutinen
;CD5E	V24DC	V24-Duplex, V24-Init
;CE1C	MBOC	MBOUT, MBIN
;CF7E	VIEW7F	Speichercheck VIEW
;CFC0	PRI7F	Druckerinit PRINT
;CFF7	FREIC	bis CFFF frei *)
;
;---------------------------------------
;
;Adr.	Label:
;D000	JDISC	DISK-Verteiler, UP's
;D0CC	BASEX	Disktreiber f}r BASIC
;D1CE	FLOA7F	FLOAD, FSAVE
;D2E6	DIR7F	DIR, STAT
;D3B4	REN7F	REN, ERA, SETRO, SETWR
;D42D	DRIV7F	DRIVE, TYPE, DUMP, INIT
;D61D	HELP7F	Versionsanzeige
;D644	FREID	bis DAB7 frei *)
;DAB8	Zusatztab. IBM-Zeichen *)
;DB00	IBM-Zeichensatz 00..1F *)
;DC00	IBM-Zeichensatz 80..FF *)
;
;---------------------------------------
;
;Adr.	Label:
;E000		RESET, BASIC-Verteiler
;E052	PADR0	CRT-Treiber, PADR, DABR
;E407	ISRC3	KBD-Treiber mit BRKT
;E4B6	ISRI1	TAPE-Treiber
;E65B	BEXP1	BASIC-Extensionen
;EDDE	L3TB	ESC-Tabelle
;EDF8	FREIE	bis EDFE frei *)
;EDFF	CAOS-Versionsnummer
;EE00	Zeichenbildtab. Gro~buchst. *)
;
;---------------------------------------
;
;Adr.	Label:
;F000		PWRON, Programmverteiler
;F0D2	SWI7F	SWITCH, MODUL, JUMP
;F1B5	MENU7F	Kommandointerpreter MENU
;F33D	WAIT	WAIT, OSTR, INTB, OCHR
;F39B	KBD	KBD, INLIN, go
;F469	HLHX	Hexzahlen-Verarbeitung
;F502	SIXD	SIXD, INIEA, INIME
;F528	SAVE7F	SAVE, VERIFY, LOAD
;F705	COL7F	COLOR, CSTBT
;F722	DISP7F	DISPLAY, MODIFY
;F768	WIND7F	WINDOW, KEY
;F7A5	CUCP	CUCP, Grafikroutinen
;F7F3	MBOUT	MBOUT,MBIN, SYSTEM
;F81F	BASPV	BASIC-I/O-Verteiler
;F8A7	ZKOUT	ZKOUT, TON, ESC-Prog's
;F957	V24O7F	V24OUT, V24DUP, CEN
;F9B1	ECHO	ECHO-Funktion
;FA81	ISRT	ISR SIO-B (Tastatur...)
;FB6A	CON	ROMC-Schalter, MULT, SQR
;FBA6	ZEI0	I/O-UP-Nummer-Tabelle
;FBAE	SUTB	UP-Tabbelle f}r SUTAB
;FC40	KTAB	Tastencodetabelle
;FCC0	ISRTAB	Interrupttabelle
;FCCE	CRTTAB	Steuerfunktionstabelle
;FD0E	TOKTAB	BASIC-Token
;FDB8	TOKJP	Sprungtab. BASIC-Token
;FDF6	FREIF	frei *)
;FDF8	Bittabelle f}r Punktroutinen
;FE00	Zeichenbildtab. Kleinbuchst. *)
;
;---------------------------------------
;*) nicht in diesem Listing aufgef}hrt
 
 
;**	EQU-Liste		**
 
LF	EQU	0AH
CR	EQU	0DH
ESC	EQU	1BH
 
V24PL	EQU	0A800H	;Steckplatz M003
 
CASS	EQU	0B700H	;Kassettenpuffer
ARGC	EQU	0B780H	;Argument-Nr.
ARGN	EQU	0B781H	;AnzahlArgumente
ARG1	EQU	0B782H	;Argument 1
ARG2	EQU	0B784H	;   "	  2
ARG3	EQU	0B786H	;   "	  3
ARG4	EQU	0B788H	;   "	  4
ARG5	EQU	0B78AH	;   "	  5
ARG10	EQU	0B794H	;   "	 10
NUMNX	EQU	0B796H	;Anzahl Zeichen
NUMVX	EQU	0B797H	;Wert HEX-Zahl
HCADR	EQU	0B799H	;Hardcopy-Aufruf
WINNR	EQU	0B79BH	;Fenster-Nr.
WINON	EQU	0B79CH	;Fenster-Anfang
WINLG	EQU	0B79EH	;Fenster-Gr|~e
CURSO	EQU	0B7A0H	;Cursor-Position
STBT	EQU	0B7A2H	;BS-Steuerbyte
COLOR	EQU	0B7A3H	;Farbe
WEND	EQU	0B7A4H	;Adr. Scrolling
CCTL0	EQU	0B7A6H	;Zeichentab. 0
CCTL1	EQU	0B7A8H	;   "	     1
CCTL2	EQU	0B7AAH	;   "	     2
CCTL3	EQU	0B7ACH	;   "	     3
SYSP	EQU	0B7AEH	;System Stackp.
SUTAB	EQU	0B7B0H	;UP-Tabelle
CTAB	EQU	0B7B2H	;CRT-Steuerprog.
BJUMP	EQU	0B7B4H	;BASIC-Sprung
OUTAB	EQU	0B7B9H	;Zeiger Ausgabe
INTAB	EQU	0B7BBH	;Zeiger Eingabe
UOUT1	EQU	0B7BDH	;Sprung OUT#2
UIN1	EQU	0B7C0H	;Sprung IN#2
UOUT2	EQU	0B7C3H	;Sprung OUT#3
UIN2	EQU	0B7C6H	;Sprung IN#3
IOERR	EQU	0B7C9H	;BASIC-Error
VRAM	EQU	0B7CBH	;Adr. Video-RAM
ZOTAB	EQU	0B7CDH
ZWEND	EQU	0B7CFH	;WEND-Merker
FTAST	EQU	0B7D1H	;F-Tastenpointer
HOR	EQU	0B7D3H	;Horizontalpos.
VERT	EQU	0B7D5H	;Vertikalpos.
FARB	EQU	0B7D6H	;Grafikfarbe
MIXIT	EQU	0B7D7H	;IX-Merker
VORTN	EQU	0B7D8H	;Vortonl{nge
DTPTR	EQU	0B7DAH	;CASS-Pointer
CTCMD	EQU	0B7DBH	;CTC2-Modus
BLINK	EQU	0B7DCH	;Blinkfrequenz
L3TAB	EQU	0B7DDH	;ESC-Tabelle
L3SIZ	EQU	0B7DFH	;Anzahl ESC's
COUNT	EQU	0B7E0H	;Tastatur-Repeat
HCPZ	EQU	0B7E1H	;Druckersteuerb.
INTV1	EQU	0B7E2H	;Druckerinit.
INTV1L	EQU	0B7E4H	;  "  -L{nge
INTV2	EQU	0B7E5H	;Duplexinit.
INTV2L	EQU	0B7E7H	;  "  -L{nge
HCPZ2	EQU	0B7E8H	;Duplexsteuerb.
OFILT	EQU	0B7E9H	;Druckroutine
PROMPT	EQU	0B7ECH	;Systemprompt
LINTYP	EQU	0B7EDH	;Linientyp
CUMUST	EQU	0B7EEH	;^Cursormuster

MODST	EQU	0B800H	;Modulsteuerwortspeicher

WNDFN	EQU	0B99CH	;Fensterspeicher
