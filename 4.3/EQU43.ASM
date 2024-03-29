EQU43   ASM                                                                                                                      
;*********************************
;**				**
;**	CAOS 4.3   (c) ML-Soft	**
;**				**
;*********************************
;**				**
;**	Mario Leubner		**
;**	Chemnitztalstr. 25	**
;**	09236 MARKERSDORF	**
;**	-----------------	**
;**				**
;**	von:	13.04.94	**
;**	bis:	17.02.95	**
;**				**
;*********************************
 
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
;C04F	SYSI	SYSI, Portinittabellen
;C0EA	SYSTC	SYSTEM-Check
;C1DD	MODULC	MODUL-Steuerung, Liste
;C408	KEYC	KEY, KEYLIST
;C4FE	SQRC	SQR, MULT, COLOR
;C54F	WINDC	WINDOW, WINAK, WININ
;C5D8	LINEC	LINE, PUSE, CIRCLE, PUDE
;C788	DPMEMO	DISPLAY, MODIFY
;C84E	CENC	CEN, V24OUT, V24-Suche
;C9F6	ZIBM	CAOS->IBM-Codewandeltab.
;CA04	HCPGMC	ShCLR-Reaktionsprogramm
;CA43	COPIES	Hardcopyroutinen
;CCF8	V24DC	V24-Duplex, V24-Init
;CDB9	MBOC	MBOUT, MBIN
;CF1B	VIEW7F	Speichercheck VIEW
;CF5D	PRI7F	Druckerinit PRINT
;CF94 bis CFFF frei *)
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
;D61D bis DAB7 frei *)
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
;EDE4	L3TB	ESC-Tabelle
;EDFE frei *)
;EDFF	CAOS-Versionsnummer
;EE00	Zeichenbildtab. Gro~buchst. *)
;
;---------------------------------------
;
;Adr.	Label:
;F000		PWRON, Programmverteiler
;F0D2	SWI7F	SWITCH, MODUL, JUMP
;F1FE	MENU7F	Kommandointerpreter MENU
;F386	WAIT	WAIT, OSTR, INTB, OCHR
;F3E4	KBD	KBD, INLIN, go
;F4B2	HLHX	Hexzahlen-Verarbeitung
;F54B	SIXD	SIXD, INIEA, INIME
;F571	SAVE7F	SAVE, VERIFY, LOAD
;F74A	COL7F	COLOR, CSTBT
;F767	DISP7F	DISPLAY, MODIFY
;F7AD	WIND7F	WINDOW, KEY
;F7EA	CUCP	CUCP, Grafikroutinen
;F838	MBOUT	MBOUT,MBIN, SYSTEM
;F864	BASPV	BASIC-I/O-Verteiler
;F8EC	ZKOUT	ZKOUT, TON, ESC-Prog's
;F99C	V247F	V24OUT, V24DUP, CEN
;F9F6	ECHO	ECHO-Funktion
;FAC8	ISRSB	ISR SIO-B (Tastatur...)
;FB6B	CON	ROMC-Schalter, MULT, SQR
;FBA7	ZEI0	I/O-UP-Nummer-Tabelle
;FBAF	SUTB	UP-Tabbelle f}r SUTAB
;FC41	KTAB	Tastencodetabelle
;FCC1	ISRTAB	Interrupttabelle
;FCCF	CRTTAB	Steuerfunktionstabelle
;FD0F	TOKTAB	BASIC-Token
;FDB9	TOKJP	Sprungtab. BASIC-Token
;FDF7 frei *)
;FDF8	Bittabelle f}r Punktroutinen
;FE00	Zeichenbildtab. Kleinbuchst. *)
;
;---------------------------------------
;*) nicht in diesem Listing aufgef}hrt
 
 
;**	EQU-Liste		**
 
LF	EQU	0AH
CR	EQU	0DH
ESC	EQU	1BH
CASS	EQU	0B700H
ARGC	EQU	0B780H
ARGN	EQU	0B781H
ARG1	EQU	0B782H
ARG2	EQU	0B784H
ARG3	EQU	0B786H
ARG4	EQU	0B788H
ARG5	EQU	0B78AH
ARG10	EQU	0B794H
NUMNX	EQU	0B796H
NUMVX	EQU	0B797H
HCADR	EQU	0B799H
WINNR	EQU	0B79BH
WINON	EQU	0B79CH
WINLG	EQU	0B79EH
CURSO	EQU	0B7A0H
STBT	EQU	0B7A2H
COLOR	EQU	0B7A3H
WEND	EQU	0B7A4H
CCTL0	EQU	0B7A6H
CCTL1	EQU	0B7A8H
CCTL2	EQU	0B7AAH
CCTL3	EQU	0B7ACH
SYSP	EQU	0B7AEH
SUTAB	EQU	0B7B0H
CTAB	EQU	0B7B2H
BJUMP	EQU	0B7B4H
OUTAB	EQU	0B7B9H
INTAB	EQU	0B7BBH
UOUT1	EQU	0B7BDH
UIN1	EQU	0B7C0H
UOUT2	EQU	0B7C3H
UIN2	EQU	0B7C6H
IOERR	EQU	0B7C9H	;BASIC-Error
VRAM	EQU	0B7CBH
ZOTAB	EQU	0B7CDH
ZWEND	EQU	0B7CFH	;WEND-Merker
FTAST	EQU	0B7D1H	;F-Tastenpointer
HOR	EQU	0B7D3H	;Horizontalpos.
VERT	EQU	0B7D5H	;Vertikalpos.
FARB	EQU	0B7D6H	;Grafikfarbe
MIXIT	EQU	0B7D7H
VORTN	EQU	0B7D8H	;Vortonl{nge
DTPTR	EQU	0B7DAH	;CASS-Pointer
CTCMD	EQU	0B7DBH	;CTC2-Modus
BLINK	EQU	0B7DCH	;Blinkfrequenz
L3TAB	EQU	0B7DDH	;ESC-Tabelle
L3SIZ	EQU	0B7DFH
COUNT	EQU	0B7E0H	;Tastatur-Repeat
HCPZ	EQU	0B7E1H
INTV1	EQU	0B7E2H
INTV1L	EQU	0B7E4H
INTV2	EQU	0B7E5H
INTV2L	EQU	0B7E7H
HCPZ2	EQU	0B7E8H
OFILT	EQU	0B7E9H	;Druckroutine
PROMPT	EQU	0B7ECH	;Systemprompt
LINTYP	EQU	0B7EDH	;Linientyp
CUMUST	EQU	0B7EEH	;^Cursormuster
WNDFN	EQU	0B99CH	;Fensterspeicher
�@�t(�p6�H	;Systemprompt
LINTYP	EQU	0B7EDH	;Linientyp
CUMUST	EQU	0B7EEH	;^Cursormuster
WNDFN	EQU	0B99CH	;Fensterspeicher
�@�t(�p6�