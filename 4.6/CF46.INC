;*****************************************
;**	CAOS 4.6	ROM F		**
;**					**
;**	Adresse:  F000h bis FDFFh	**
;**					**
;**	letzte Aenderung: 05.03.2016	**
;*****************************************

;	ORG	0F000H

	JP	PWRON
	JP	PV1		; DEFB nr
	JP	PV2		; (ARGC)=nr
	JP	PV3		; E=nr
	JP	PV4		; E=nr, IRM e/a
	JP	RCALL		; DEFW offset
	JP	BYE		; JUMP-Einsprung
	JP	PV5		; E=nr, IRM/Stack
	JP	IRMON
	JP	IRMOF
	JP	PV6		; (ARGC)=nr
JDISK:	RET			; DISK-Verteiler - entfernt in CAOS 4.6 !!!
;	CALL	CON
;	CALL	JDISC
;	JP	COFF

PV6:	PUSH	BC
	CALL	IRMON
	CALL	PV2
	JR	PV51

PV5:	PUSH	BC
	CALL	IRMON
	CALL	PV3
PV51:	CALL	IRMOF
	POP	BC
	RET

PV4:	CALL	SCRON
	CALL	PV3
SCROFF:	PUSH	AF
	IN	A,(PIOAD)
	AND	11111011b	; IRM aus
	OUT	(PIOAD),A
	POP	AF
	RET

SCRON:	PUSH	AF
	IN	A,(PIOAD)
	OR	00000100b	; IRM ein
	OUT	(PIOAD),A
	POP	AF
	RET

PV1:	DI
	PUSH	HL
	POP	HL
	POP	HL
	INC	HL		; nr uebergehen
	PUSH	HL
	DEC	HL		; (HL)=nr
	DEC	SP
	DEC	SP
	EI
	PUSH	AF
	PUSH	DE
	LD	E,(HL)
PV22:	LD	D,0
	LD	HL,(SUTAB)
	ADD	HL,DE
	ADD	HL,DE		; HL=Pos. in Tab
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	EX	DE,HL
	POP	DE
	POP	AF
EXRT:	EX	(SP),HL
	RET			; UP anspringen

PV2:	AND	A		; VR: F
PV21:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	HL,POP3		; Returnadresse
	EX	(SP),HL		; kellern
	PUSH	HL
	PUSH	AF
	PUSH	DE
	JR	C,PV22		; falls PV3
	LD	A,(ARGC)	; sonst E
	LD	E,A		; beschaffen
	JR	PV22

PV3:	SCF			; VR: F
	JR	PV21

IRMON:	POP	BC		; Rueckkehradresse
	PUSH	IY
	LD	IY,0		; SP in IY merken
	ADD	IY,SP		; (kein UP veraendert IY!)
	DI
	LD	(IX+11),A
	IN	A,(PIOAD)
	OR	00100100b	; IRM+LED ein
	OUT	(PIOAD),A
	LD	SP,(SYSP)
	EI
	LD	A,(IX+11)
	PUSH	BC
	RET			; wie JP (BC)

IRMOF:	POP	BC		; analog IRMON
	LD	(IX+11),A
	IN	A,(PIOAD)
	RES	2,A		; IRM+LED aus
	RES	5,A		; (F unveraendert)
	DI
	OUT	(PIOAD),A
	LD	SP,IY
	EI
	LD	A,(IX+11)
	POP	IY
	PUSH	BC
	RET

; Relativer UP-Aufruf ohne Registerzerstoerung:

RCALL:	EX	(SP),HL
	PUSH	DE
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; DE=Offset
	INC	HL
	EX	DE,HL
	ADD	HL,DE		; addieren
	EX	(SP),HL		; Zieladresse
	EX	DE,HL		; DE von Stack
	DI
	INC	SP
	INC	SP
	EX	(SP),HL		; HL von Stack
	DEC	SP
	DEC	SP
	EI
	RET			; Ziel anspringen

BNROST:	LD	A,(IX+2)	; Blocknummer
AHOSTR:	CALL	AHEX		; Register A und folgende Zeichenkette
OSTR:	EX	(SP),HL		; **23**
OSTR1:	LD	A,(HL)		; ausgeben
	INC	HL
	AND	A		; bis NUL-Byte
	JR	Z,EXRT
	CALL	OCHR
	JR	OSTR1

; Module listen, lesen, schalten:

SWI7F:	DW	7F7FH
	DB	'SWITCH'
	DB	1
	AND	A		; Argumente?
	JR	Z,MLIST
	CALL	CON
	CALL	SWIC		; lesen/schalten
	JR	MCOFF
	;
MLIST:	CALL	ESC6		; interne Module listen
	;
ESC5:	CALL	CON
	CALL	MODULC		; externe Module listen
	JR	ESC61

;SYS7F:	DW	7F7FH		; Dieses Menuewort ist in CAOS 4.6 nicht
;	DB	'SYSTEM'	; mehr erforderlich - macht SWITCH mit!
;	DB	11H
ESC6:	CALL	CON
	CALL	SYSTC		; Systemcheck (interne Module 0-5)
ESC61:	LD	DE,(CURSO)	; aktuelle Cursorposition mitnehmen
	JR	MCOFF

; Module lesen und Schalten
MODUL2:	LD	A,2
	LD	L,A
MODU:	CALL	CON		; **26**
	CALL	MODUC
MCOFF:	JP	COFF

ESCD:	CALL	CON		; DEVICE-Umschaltung
	CALL	ESCDD
	JR	ESC61

; Betriebssystem wechseln:

JUMP7F:	DW	7F7FH
	DB	'JUMP'
	DB	3
	LD	A,L
JUMP:	LD	B,A		; **27**
	LD	C,80H
	IN	A,(C)
	INC	A		; Kennbyte FF?
	JR	NZ,JUM0
	CALL	OSTR
	DB	'Kein Modul!'
	DB	7,CR,LF,0
	RET
	;
JUM0:	PUSH	BC
	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	B,7		; ab Platz 7!
JUM1:	INC	B
	JR	Z,JUM2		; bis FF
	IN	A,(C)
	CPL
	AND	70H		; Speichermodul?
	JR	NZ,JUM1
	OUT	(C),A		; aus
	LD	L,B
	LD	(HL),A
	JR	JUM1
	;
JUM2:	POP	BC
	LD	A,0FFH		; Ausgabe FF
	LD	L,B
	LD	(HL),A		; Eintrag
	DI
	OUT	(C),A		; einschalten
	IN	A,(PIOAD)
	AND	01111110b
	JP	BJUMP		; in IRM

; Einschalt-Initialisierung:

PWRON:	LD	A,0E3H		; RAM4 WR einschalten,
	OUT	(86H),A		; und CAOS-C ein
	JP	PWRONC		; Programm im ROM-C weiter abarbeiten ...
;
; Tasten-RESET:
;
BYE:	LD	SP,01C4H	; System-Stackpointer
	LD	A,0E3H		; RAM4 WR einschalten,
	OUT	(86H),A		; und CAOS-C ein
	CALL	SYSI		; Systeminitialisierung
PWR4:	CALL	V24INI		; M003 suchen und initialisieren
	CALL	COFF		; CAOS-C ausschalten (Standard)
	LD	BC,880H		; auf Steckplatz 8
	IN	A,(C)		; ROM-Modul mit Strukturbyte
	DEC	A		; 01 vorhanden?
	JR	NZ,PWR6		; nein!
	LD	A,43H
	OUT	(C),A		; einschalten auf Adresse 4000H
	LD	(MODST+8),A	; Schaltzustand eintragen
	LD	A,(IX+4)
	AND	0FCH		; internen RAM4 ausschalten
	LD	(IX+4),A
	OUT	(86H),A
	JP	4000H		; Adresse 4000h im Modul anspringen
	;
PWR6:	LD	B,0FCH
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	NZ,PWR7		; nein
	OUT	(C),A		; einschalten mit Steuerbyte A7H
	LD	BC,UROK		; 0B3F3H
	IN	A,(C)		; Betriebsart ermitteln
	DEC	A
	JR	Z,PWR7		; CP/M-Betriebsart erkannt
	CP	4
	JR	Z,PWR7		; CAOS-Betriebsart erkannt
	CALL	OSTR
	DB	0CH,LF
	DB	'Autostart Floppy'
	DB	CR,LF,0
	LD	B,0
PWR5:	CALL	BRKT		; BRK-Taste gedrueckt?
	JR	C,PWR7		; ja, dann kein Floppy-Autostart
	DJNZ	PWR5		; 256-mal abfragen
	LD	A,1
	LD	(ARGN),A	; 1 Parameter fuer JUMP FC vorgeben
	LD	A,0FCH
	CALL	JUMP		; Startversuch
PWR7:	XOR	A
	LD	(ARGN),A	; MENU ohne Parameter ausfuehren!
	JR	MEN0
;
	DW	7F7FH
	DB	'MENU'
	DB	1
	POP	HL		; Stack clear
MEN0:	CALL	OSTR
	DB	0CH,LF
	DB	'* KC-CAOS 4.6 ',0
	CALL	DEVANZ		; aktuelles Device anzeigen
	CALL	OSTR
	DB	' *',0
MENU:	LD	HL,(ARGN)	; **46**
	LD	E,0		; Parameter zum Testen in E bereithalten
	DEC	L		; ARGN=1?
	JR	NZ,men9		; nein, E=0 verwenden
	LD	E,H		; E=ARG1
men9:	LD	HL,0C000H	; Menuewortsuche immer ab C000H
	LD	C,L
	LD	B,L		; BC=0 (Suchlaenge)
MEN1:	CALL	CRLF
MEN2:	LD	A,2		; CLLN
	CALL	OPRCHR		; Prompt ausgeben
	CALL	BRKT
	JR	C,LOOP2		; Break
	LD	A,(IX+9)
MEN4:	CPIR
	JP	PO,LOOP2	; ausgesucht
	CPI
	JP	PO,LOOP2	; ausgesucht
	JR	NZ,MEN4		; 2mal!
MEN5:	LD	A,(HL)
	CP	20h		; Epilog (0..1Fh)?
	JR	C,MEN7
	bit	0,e		; versteckte Menueworte anzeigen?
	jr	z,men8		; nein
	CP	'z'+1		; <= 'z'?
	JR	NC,MEN2		; groesser als 'z'
	jr	men6
	;
men8:	CP	'0'		; >= '0' und
	JR	C,MEN2		; kleiner als '0'
	CP	'Z'+1		; <= 'Z'?
	JR	NC,MEN2		; groesser als 'Z'
	CP	'A'		; >= 'A' oder
	JR	NC,MEN6		; groesser als 'A'
	CP	':'+1		; <= ':'?
	JR	NC,MEN2		; groesser als ':'
MEN6:	CALL	OCHR
	INC	HL
	DEC	BC
	LD	A,C
	OR	B
	JR	NZ,MEN5
	LD	A,2		; CLLN
	JR	LOOP1		; ausgesucht
	;
MEN7:	bit	1,e
	jr	z,men1		; keine Adressen anzeigen
	ld	a,(winlg)
	cp	15		; Fensterbreite fuer Adressen ausreichend?
	jr	c,men1		; nein, dann nicht anzeigen!
	ld	a,11
	ld	(curso),a	; Spalte fuer Adressanzeige
	inc	hl
	call	hlhx		; Startadresse anzeigen
	dec	hl
	JR	MEN1

; Eingabeschleife:
LOOP:	IN	A,(PIOAD)	; **12**
	AND	11011111b	; LED aus
	OR	00000100b	; IRM ein
	OUT	(PIOAD),A
	CALL	COFF
	LD	A,CR		; Zeilenanfang
LOOP1:	CALL	OPRCHR		; Prompt ausgeben
LOOP2:	CALL	INPUT
	JR	LOOP

; Menuekommandoroutine
INPUT:	CALL	INLIN
	JR	NC,INP1
	LD	A,0BH		; bei BRK
	CALL	CRT		; Cursor hoch
	LD	A,2		; und
	JP	CRT		; Zeile loeschen
	;
INP1:	INC	DE		; Zeichen danach
	LD	A,(DE)
	AND	0DFH		; leer?
	RET	Z
	LD	A,(MODST+2)	; Schaltzustand USER-ROM
	LD	(ARGC),A	; CAOS 4.6: hier uebergeben an USER-Programme
	CALL	MSUCH		; Suche ab C000H
	JR	C,INP5		; gefunden
	LD	A,(MODST+5)
	AND	1
	JR	NZ,INP2		; CAOS-C war bereits an
	CALL	CON
	LD	B,20H		; ansonsten 2000h Bytes
	CALL	MS1		; durchsuchen im CAOS-ROM-C
	JR	C,INP5		; gefunden
INP2:	PUSH	DE
	LD	D,0D1H		; 1. Ebene (BASIC nicht mit durchsuchen)
INP3:	CALL	MODUL2		; USER-ROM-C Ebene ein
	RESIXA	7,4		; RES 7,(IX+4),A
	OUT	(86H),A		; CAOS-ROM-C aus!
	EX	DE,HL
	EX	(SP),HL		; DE=Suchkette
	EX	DE,HL
	LD	B,20H		; Suchlaenge 2000h Bytes
	CALL	MS1		; Menuewort in USER-ROM suchen
	JR	C,INP4		; gefunden
	EX	DE,HL
	EX	(SP),HL		; DE=Schaltzustand
	EX	DE,HL
	LD	A,10H		; Offset zur naechsten Ebene
	ADD	A,D		; ergibt: D1, E1, F1
	LD	D,A
	JR	NC,INP3		; naechste Ebene
	LD	DE,(ARGC-1)	; CAOS 4.6: D = USERC-Schaltzustand vor Suche
	CALL	MODUL2		; USER-ROMC wie zuvor
	POP	DE
	JR	ERR1		; invalid command
	;
INP4:	POP	AF		; Schaltzustand USER-ROM-C vergessen
	XOR	A
	LD	(MODST+5),A	; CAOS-C ausschalten
INP5:	LD	A,(HL)		; Epilog
	INC	HL
	PUSH	HL
	CP	1FH
	RET	Z		; ohne arg!
	PUSH	AF
	CALL	GARG		; Arg's erfassen
	POP	BC
	JR	C,ERR4		; bad args
	LD	A,(ARGN)
	LD	C,B
	RES	4,B
	SRA	B
	CP	B
	JR	C,ERR3		; zu wenig arg
	JR	Z,INP6
	BIT	4,C
	JR	NZ,ERR2		; zu viele arg
INP6:	BIT	0,C
	CALL	LARG		; Arg's laden
	RET	NZ		; mit IRM on
	JP	SCROFF		; mit IRM off

ERR1:	CALL	OSTR
	DB	'Kommando?',7,0
	JR	ERRCR
ERR2:	CALL	OSTR
	DB	'Zu viele',0
	JR	ERRA
ERR3:	CALL	OSTR
	DB	'Zu wenig',0
	JR	ERRA
ERR4:	CALL	OSTR
	DB	'Fehlerhafte',0
ERRA:	POP	HL		; Stack clear
	CALL	OSTR
	DB	' Argumente',7,0
ERRCR:	JP	CRLF

MSUCH:	LD	B,0
MS1:	LD	HL,0C000H	; Standard-Suchbeginn
	LD	C,L
ZS0:	LD	A,(IX+9)	; Prologbyte
ZSUCH:	CPIR			; **1D**
	SCF
	CCF			; CY=0
	RET	PO		; Prolog nicht gefunden
	CPI
	JR	NZ,ZSUCH
	PUSH	AF
	PUSH	DE		; Beginn der Vergleichskette retten
ZS1:	LD	A,(DE)		; Zeichen von Vergleichskette
	INC	DE
	CP	21H
	JR	C,ZS3		; Kettenende bei Vergleichskette erkannt
	BIT	5,(HL)		; Grossbuchstabe im Speicher?
	CALL	Z,UPCASE	; dann Zeichen von Vergleichskette upcasen
	CPI
	JR	Z,ZS1		; stimmt ueberein
ZS2:	POP	DE		; wieder zu Begenn der Vergleichskette
	POP	AF		; und verwendetes Prologbyte restaurieren
	JR	ZSUCH		; Suche fortsetzen
	;
ZS3:	LD	A,(HL)		; Zeichen aus Speicher weiter testen
	CP	20H		; bis Epilogbyte (0..1Fh) kommt
	JR	C,ZS4		; Epilog gefunden
;	JR	Z,ZS2		; SPC (war in CAOS 4.3 und 4.4 nicht zugelassen)
	CP	80H
	JR	NC,ZS2		; Zeichen ab 80H nicht zugelassen
	INC	HL
	DEC	BC		; Befehl am 11.04.2010 ergaenzt
	JR	ZS3
	;
ZS4:	POP	AF
	POP	AF
	SCF			; CY=1 fuer gefunden
	RET

WAIT:	LD	B,A		; **14**
	XOR	A
WAIT1:	CALL	WTUP
	DJNZ	WAIT1
	RET

WTUP:	DEC	A		; UP fuer WAIT
	RET	Z
	PUSH	AF
	POP	AF
	JR	WTUP

NORM:	CALL	NIN		; **13**
NOUT:	LD	HL,ZEI0		; **20**
SOUT:	PUSH	HL		; **1E**
	LD	HL,(OUTAB)
	EX	(SP),HL
	LD	(OUTAB),HL
	POP	HL
	RET

NIN:	LD	HL,ZEI4		; **21**
SIN:	PUSH	HL		; **1F**
	LD	HL,(INTAB)
	EX	(SP),HL
	LD	(INTAB),HL
	POP	HL
	RET

; Steuerzeichen und Prompt ausgeben
; PE: A=Steuerzeichen
OPRCHR:	CALL	CRT
	LD	A,(PROMPT)
	CP	' '
	JR	NC,OPRCH1
	LD	A,'%'		; Standard
OPRCH1:	JR	OCHR		; Prompt

HLHX:	LD	A,H		; **1A**
	CALL	AHEX
	LD	A,L
AHSPC:	CALL	AHEX
SPACE:	LD	A,20H		; **2B**
	JR	OCHR

HOME:	LD	A,10H		; **2D**
	JR	OCHR

AHEX:	PUSH	AF		; **1C**
	RRCA			; Tetradentausch
	RRCA
	RRCA
	RRCA
	CALL	AHEX0		; erst aufrufen,
	POP	AF		; dann reinlaufen
AHEX0:	AND	0FH		; Maske
	ADD	A,90H		; uebliche
	DAA			; Routine
	ADC	A,40H		; fuer die
	DAA			; Hexausgabe
	;
OCHR:	PUSH	HL		; **24**
	LD	HL,(OUTAB)
INTA1:	PUSH	DE		; siehe auch PV1
	PUSH	BC		; (sehr aehnlich)
	LD	E,(HL)
	LD	D,0
	LD	HL,(SUTAB)
	PUSH	AF
	ADD	HL,DE
	ADD	HL,DE
	POP	AF
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	EX	DE,HL
	CALL	JPHL
POP3:	POP	BC		; auch fuer PV2
	POP	DE		; bis PV6!
	POP	HL
	RET

INTB:	PUSH	HL		; **16**
	LD	HL,(INTAB)
	JR	INTA1

KBD:	PUSH	HL		; **04**
	PUSH	DE
	PUSH	BC
KBD1:	BIT	6,(IX+8)	; F-Taste aktiv?
	JR	NZ,KBD9		; ja
KBD2:	LD	E,0
KBD3:	CALL	CUCP		; blinken
	INC	E		; e/a-Merker
	LD	B,0FH		; F00h mal
KBD4:	CALL	KBDZ		; abfragen
	JR	C,KBD5
	DEC	BC
	LD	A,B
	OR	C
	JR	NZ,KBD4		; dann Cursor
	JR	KBD3		; wieder weg usw.
	;
KBD5:	BIT	0,E		; gerade ein?
	CALL	NZ,CUCP		; ja - wegnehmen
	LD	D,A
	BIT	5,(IX+8)	; Click ein?
	CALL	NZ,CLIK
	LD	A,D
	LD	HL,STBT
	BIT	4,(HL)		; ESC aktiv?
	JR	Z,KBD6
	LD	DE,(CURSO)
	CALL	CRT1		; ESC ausfuehren
	LD	(CURSO),DE
	JR	KBD2
	;
KBD6:	CP	ESC
	JR	NZ,KBD7
	SET	4,(HL)		; ESC merken
KBD2J:	JR	KBD2
	;
KBD7:	CP	0F1H		; F-Taste?
	JR	C,POP3
	AND	0FH
	LD	B,A
	LD	HL,0B900H
KBD8:	LD	A,(HL)
	INC	L
	JR	Z,KBD2		; Speicherende
	AND	A
	JR	NZ,KBD8		; Position im Puffer
	DJNZ	KBD8		; auszaehlen
	LD	A,L
	CP	9CH
	JR	NC,KBD2		; Speicherende
	SET	6,(IX+8)	; F-Taste aktiv
	JR	KBD10
	;
KBD9:	CALL	BRKT		; Abbruch?
	JR	C,KBD12
	LD	HL,(FTAST)	; Zeiger auf
KBD10:	LD	A,(HL)		; naechstes CHR
	CP	ESC
	JR	NZ,KBD11
	INC	HL
	LD	(FTAST),HL
	LD	HL,STBT
	SET	4,(HL)		; ESC merken
KBD1J:	JR	KBD1
	;
KBD11:	INC	HL
	LD	(FTAST),HL	; Zeiger retten
	AND	A		; NUL-Ende?
	JR	NZ,KBD13
KBD12:	RES	6,(IX+8)	; ja - inaktiv
	JR	KBD2J
	;
KBD13:	LD	HL,STBT
	BIT	4,(HL)
	JP	Z,POP3
	LD	DE,(CURSO)
	CALL	CRT1		; ESC ausfuehren
	LD	(CURSO),DE
	JR	KBD1J

INLIN:	CALL	INTB		; **17**
	CALL	OCHR
	CP	3		; BRK?
	SCF
	JR	Z,INLI1
	CP	CR		; ENTER?
	JR	NZ,INLIN
INLI1:	PUSH	AF
	CALL	CRLF		; PA: A=0
	LD	DE,(CURSO)
	CP	D		; Zeile 1 durch PAGE-Mode?
	JR	NZ,INLI2
	LD	A,(WINLG+1)
	LD	D,A		; D korrigieren
INLI2:	DEC	D		; Zeile drueber
	PUSH	HL
	CALL	DABR
	EX	DE,HL
	POP	HL		; PA: DE
	POP	AF		; PA: CY=BRK
NOOP:	RET
;
; Sprung zu einer Adresse mit Registeruebergabe
; Syntax: %go ADR [HL] [DE] [BC]
;
GO7F:	DW	7F7FH
	DB	'go'
	DB	3		; mindestens 1 Argument erforderlich
	PUSH	HL		; Adresse auf den Stack
	EX	DE,HL		; ARG2 von DE nach HL
	LD	D,B
	LD	E,C		; ARG3 von BC nach DE
	LD	BC,(ARG4)	; ARG4 nach BC
	LD	A,(ARG5)
	RET

ERRM:	CALL	OSTR		; **17**
	DB	'ERROR',7,0
	;
CRLF:	CALL	OSTR		; **2C**
	DB	CR,LF,0
	RET

HLDE:	CALL	HLDE1		; **1B**
HLDE1:	CALL	HLHX
	EX	DE,HL
	RET

RHEX0:	INC	DE
RHEX:	LD	A,(DE)		; **18**
	CP	' '		; Trennzeichen
	JR	Z,RHEX0		; uebergehen
	XOR	A
	LD	HL,NUMVX+1
	LD	(HL),A
	DEC	HL		; HL=NUMVX
	LD	(HL),A
	DEC	HL		; HL=NUMNX
	LD	(HL),A		; alles auf 0 setzen
RH1:	LD	A,(DE)
	OR	A		; Ende-Dummy?
	RET	Z
	CP	' '		; Ende-Space?
	RET	Z
	SUB	'0'
	RET	C
	CP	10
	JR	C,RH2
	SUB	7
	AND	11011111b	; klein -> gross
	CP	0Ah
	RET	C
	CP	10h
	CCF
	RET	C
RH2:	INC	DE
	INC	(HL)		; NUMNX erhoehen
	INC	HL
	RLD			; Tetrade
	INC	HL		; durchschieben
	RLD
	DEC	HL
	DEC	HL
	JR	Z,RH1		; RLD setzt auch Z-Flag!
	DEC	DE
	SCF			; Fehler: Zahl zu gross
	RET

; Bis zu 10 Argumente erfassen:
GARG:	LD	BC,ARGN		; **22**
	XOR	A
	LD	(BC),A		; (ARGN)=0
GARG1:	CALL	RHEX
	RET	C		; Fehler
	LD	A,(HL)
	OR	A		; Ende-Dummy?
	RET	Z
	INC	HL
	INC	BC
	LD	A,(HL)		; LOW(NUMVX)
	LD	(BC),A		; LOW(Argument)
	INC	HL
	INC	BC
	LD	A,(HL)		; HIGH(NUMVX)
	LD	(BC),A		; HIGH(Argument)
	LD	L,LOW(ARGN)	; HL=ARGN
	INC	(HL)
	LD	A,(HL)
	ADD	A,-11		; 10 Argumente?
	JR	NC,GARG1
	RET			; Fehler: zu viele

; Kleinbuchstaben in A in Grossbuchstaben umwandeln:
UPCASE:	CP	'a'
	RET	C
	CP	'z' + 1
	RET	NC
	AND	11011111b
	RET

KPUFF:	LD	(IX+5),LOW(CASS)
	LD	(IX+6),HIGH(CASS)
	RET

INIEA:	PUSH	BC		; **43**
	LD	C,(HL)		; erstes Byte: Portadresse
	INC	HL
	LD	B,(HL)		; zweites Byte: Anzahl
	INC	HL
	OTIR
	POP	BC
	RET

INIMEX:	LD	D,(HL)		; Anzahl Kanaele
	INC	HL
INIMX1:	CALL	INIEA
	DEC	D
	JR	NZ,INIMX1
	RET

INIME:	DI			;**44**
	CALL	INIMX1
	EI
	RET
;
; Dateiname abfragen
; PA:	HL	Zeiger auf den eingegebenen Namen
;	CY=1	mit BRK abgebrochen
;
NAME:	LD	HL,6
NAMHL:	CALL	OSTR
	DB	'Name :',0
	CALL	INLIN		; Dateiname abfragen
	RET	C		; BRK
	ADD	HL,DE		; Beginn Name
	RET

SAVE7F:	DW	7F7FH
	DB	'SAVE'
	DB	5
	CALL	NAME		; Dateiname abfragen
	RET	C		; mit BRK abgebrochen
	;
SAVE:	LD	DE,CASS		; **36**
	LD	BC,11
	LDIR			; Name in Puffer kopieren
	XOR	A
	LD	(DE),A		; Ende-0 anhaengen
	LD	HL,ARGN		; alle Argumente inklusive ARGN
	LD	E,10H
	LD	C,15H
	LDIR			; in den Puffer kopieren
	CALL	KPUFF		; Standard-Puffer setzen
	CALL	PV1		; Vorblock ausgeben
	DB	08H		; ISRO
	RET	C		; Fehler
	LD	HL,(ARG2)	; Endadresse+1
	LD	DE,(ARG1)	; Anfangsadresse
	AND	A
	SBC	HL,DE		; Anzahl Datenbytes
	push	hl		; Anzahl auf Stack 
SAV1:	LD	HL,(ARG1)	; Adresse fuer aktuellen Block
	LD	DE,CASS
	LD	BC,128
	LD	A,(MODST+1)
	AND	1
	JR	NZ,SAV3		; IRM ein
	LD	B,C
SAV2:	CALL	SCROFF
	LD	A,(HL)		; vom RAM
	CALL	SCRON
	LD	(DE),A		; in den Puffer
	INC	HL
	INC	DE
	DJNZ	SAV2
	JR	SAV4
SAV3:	LDIR			; kopieren
SAV4:	LD	(ARG1),HL	; Adresse fuer naechsten Block eintragen
	pop	hl		; Anzahl
	CALL	OSTR
	DB	2,0		; CLLN
	CALL	BNROST		; Blocknummer ausgeben
	DB	0
	CALL	BRKT
	JR	C,SAV5		; BRK -> Datei schliessen
	LD	BC,160		; kurzer Vorton
	ld	de,128		; Anzahl - 128
	sbc	hl,de		; Ende erreicht?
	jr	z,SAV5		; ja, genau das Blockende
	jr	c,SAV5		; ja, letzter Block mit weniger Bytes
	push	hl		; restliche Anzahl
	CALL	PV1		; Datenblock ausgeben
	DB	01H		; MBO
	JR	NC,SAV1		; kein Fehler
	pop	hl		; restliche Byte-Anzahl vom Stack nehmen
	;
SAV5:	CALL	PV1		; Ausgabe Endeblock 0FFH
	DB	09H		; CSRO
	CALL	CLC		; CAOS 4.6: Puffer loeschen nicht mehr in CSRO
	CALL	CRLF		; CAOS 4.6: Zeilenvorschub nicht mehr in CSRO
	RET

LARG:	LD	BC,(ARG3)	; **15**
	LD	DE,(ARG2)
	LD	HL,(ARG1)
	LD	A,(ARGN)
	RET

; Lesen eines Blockes von Kassette mit Ausschrift Blocknummer bzw. Name
;
; PE:	(IX+3)	zu lesender Block
; VR:	AF,BC	(Block<>1) ( 00>  )
;	AF,BC,HL (Block=1) ( Name )
; PA:	CY=1	BRK gedrueckt nach Lesefehler

RDBLK:	INC	(IX+3)		; erwarteter Block
LUP1:	CALL	PV1
	DB	05H		; MBI
LUP2:	JR	NC,LUP3		; kein Fehler aufgetreten
	CALL	OSTR
	DB	9,9,9,' ',0
	CALL	BNROST
	DB	' ?'		; Pruefsumme fehlerhaft
	DB	1EH,0
	LD	A,(IX+3)
	DEC	A		; Vorblock mit Nr. 01 erwartet?
	JR	Z,LUP1		; dann weiter versuchen...
	CALL	KBD		; Tastaturabfrage
	CP	3		; BRK?
	SCF
	RET	Z
	CP	LF		; CUD? (fehlerhaften Block uebernehmen?)
	JR	NZ,LUP1
LUP3:	LD	A,(IX+2)	; gelesener Block
	LD	B,(IX+3)	; zu lesender Block
	DEC	B		; Block 01?
	JR	Z,LUP6		; Block 1 soll gelesen werden
	INC	B
	CP	B		; richtige Blocknummer?
	JR	Z,LUP5
	INC	A		; Block FF?
	JR	Z,LUP5		; Endeblock immer als richtig werten
LUP4:	CALL	BNROST
	DB	'*',19H,0	; falsche Blocknummer
	JR	LUP1		; weiter versuchen
	;
LUP5:	CALL	BNROST
	DB	'> ',19H,0	; korrekter Block
	RET
	;
LUP6:	DEC	A		; Block 01?
	JR	NZ,LUP4		; nein -> falsche Blocknummer anzeigen
;	CALL	CRLF		; Zeilenvorschub vor Dateiname ???
	LD	HL,CASS		; Bei Vorblock keine Blocknummer,
	LD	B,11		; sondern den Name+Typ
LUP7:	LD	A,(HL)		; aus Kassettenpuffer anzeigen
	INC	HL
	CP	20h		; Steuerzeichen?
	CALL	nc,CRT		; nur anzeigen was groesser 20h ist!
	DJNZ	LUP7
	CALL	SPACE		; Leerzeichen zwischen Dateiname + Adressen
	AND	A		; kein Fehler
	RET

NOMC:	CALL	OSTR
	DB	'Keine MC-Datei!',0
CLJP:	CALL	PV1
	DB	0BH		; CSRI
	RET

VERI7F:	DW	7F7FH
	DB	'VERIFY'
	DB	1
VERIF:	LD	(IX+7),0	; **11**
	JR	LOAD1

LOAD7F:	DW	7F7FH
	DB	'LOAD'
	DB	1
	LD	A,(IX+8)
	AND	00011100b
	CALL	NZ,NAME		; Dateiname abfragen bei Device > 0
	RET	C		; mit BRK abgebrochen
LOAD:	LD	(IX+7),1	; **10**
	LD	A,(ARGN)
	CP	2
	JR	C,LOAD1		; Autostart?
	SET	1,(IX+7)	; unterdruecken
LOAD1:	CALL	KPUFF		; Standardpuffer setzen
	CALL	PV1		; HL = Dateiname
	DB	0AH		; ISRI (Vorblock einlesen)
	JR	NC,LOAD0	; kein Fehler aufgetreten
	LD	A,(IX+8)
	AND	00011100b
	RET	NZ		; Abbruch wenn DEVICE > 0
LOAD0:	CALL	LUP2		; Blockinhalt auswerten (Name anzeigen usw.)
LOAD2:	JR	C,CLJP		; BRK
	BIT	0,(IX+7)
	JR	Z,LOAD5		; Verify
	LD	L,10H
	LD	A,(HL)		; Anzahl der Argumente beim Abspeichern
	RLCA
	RLCA			; nach Bits 2..4
	XOR	(IX+7)
	AND	00011100b	; Bits 0..1 und 5..7 zuruecksetzen
	XOR	(IX+7)		; a = (a XOR b) XOR b fuer Bits 2..4
	LD	(IX+7),A	; in IX+7 merken fuer spaetere Auswertung
	LD	A,(HL)		; Anzahl der Argumente
	SUB	2
	CP	9		; MC?
	JR	NC,NOMC
	LD	HL,(CASS+15H)	; SADR
	PUSH	HL
	LD	DE,(CASS+13H)	; EADR+1
	LD	HL,(CASS+11H)	; AADR
	LD	A,(ARGN)
	AND	A		; Ladeoffset?
	JR	Z,LOAD3		; nein
	LD	BC,(ARG1)	; Offset
	ADD	HL,BC		; Anfangsadresse und Endadresse
	EX	DE,HL		; umrechnen
	ADD	HL,BC
	EX	DE,HL
	BIT	4,(IX+7)	; 4 bis 7 Argumente?
	JR	NZ,LOAD3
	EX	(SP),HL
	ADD	HL,BC		; Startadresse umrechnen
	EX	(SP),HL
LOAD3:	CALL	HLDE		; Anfangsadresse und Endadresse anzeigen
	LD	A,(CASS+10H)
	CP	3
	JR	C,LOAD4		; keine Startadresse
	EX	(SP),HL
	CALL	HLHX		; Startadresse anzeigen
	EX	(SP),HL
LOAD4:	POP	BC		; Startadresse merken
	LD	(ARG3),BC
LOAD5:	CALL	CRLF		; Zeilenvorschub nach Dateiname + Adressen
	EX	DE,HL		; DE = Ladeadresse
	; Load-Hauptschleife
LOAD6:	CALL	RDBLK		; 128-Byte-Block einlesen
	JR	C,LOAD2		; BRK -> CLJP
	BIT	0,(IX+7)
	JR	Z,LOAD11	; Verify
	PUSH	HL		; Endadresse
	SBC	HL,DE		; noch zu lesende Bytes
	LD	BC,128		; Blockgroesse 128 Byte
	SBC	HL,BC		; voller Block?
	ADD	HL,BC
	JR	NC,LOAD7	; ja
	LD	C,L		; restliche Anzahl
LOAD7:	LD	HL,CASS		; Standard-Kassettenpuffer
	LD	A,(MODST+1)
	AND	1
	JR	NZ,LOAD9	; IRM ist ein
	LD	B,C
LOAD8:	LD	A,(HL)		; Byteweise aus dem Puffer
	CALL	SCROFF
	LD	(DE),A		; in den RAM kopieren
	INC	HL
	INC	DE
	CALL	SCRON
	DJNZ	LOAD8
	JR	LOAD10
	;
LOAD9:	LDIR			; gelesenen Block kopieren
LOAD10:	POP	HL
LOAD11:
;	INC	(IX+2)		; Blocknummer FF?
;	JR	Z,LOAD12	; ja, Einlesen beenden
	SBC	HL,DE		; Endadresse erreicht?
	ADD	HL,DE		; EADR regenerieren (Z-Flag bleibt erhalten)
;
; CAOS 4.6:
; Damit das Einlesen auch bei Diskette funktioniert, muss zusaetzlich oder
; ausschliesslich (?) der Adressvergleich stattfinden! Da hier nur MC-Dateien
; gelesen werden, ist eigentlich keine Abfrage auf Blocknr. FF erforderlich
;

	JR	NZ,LOAD6
LOAD12:	CALL	CLC		; CAOS 4.6: Puffer loeschen nicht mehr in CSRI
	CALL	CRLF		; CAOS 4.6: Zeilenvorschub nicht mehr in CSRI
	;
	CALL	PV1		; Datei schliessen
	DB	0BH		; CSRI
	JP	C,CLJP		; Fehler
	LD	A,(IX+7)
	AND	3
	DEC	A
	RET	NZ		; kein Start oder Verify
	LD	A,(IX+7)
	AND	1CH
	CP	0CH
	LD	(IX+7),0	; Befehl fehlte in CAOS 4.3!
	RET	C		; keine Startadresse
	LD	HL,(ARG3)	; Startadresse
	JP	(HL)		; anspringen

	DW	7F7FH
	DB	'COLOR'
	DB	1
	CALL	CON
	JP	COLRC

COLR:	CALL	CON		; **0F**
	JP	SETCO

CSTBT:	PUSH	HL		; **42**
	LD	HL,STBT
	SET	3,(HL)
	CALL	OCHR		; Steuerzeichen
	RES	3,(HL)		; darstellen
	POP	HL
	RET

	DW	7F7FH
	DB	'DISPLAY'
	DB	3
DISP:	CALL	CON		; **3B**
	CALL	DISPC
	JR	JCOFF

	DW	7F7FH
	DB	'MODIFY'
	DB	3
MODI:	CALL	CON		; **2E**
	CALL	MODIC
JCOFF:	JP	COFF

WIND7F:	DW	7F7FH
	DB	'WINDOW'
	DB	1
	CALL	CON
	CALL	WINDC
	JR	JCOFF

WININ:	CALL	CON		; **3C**
	CALL	WININC
	JR	JCOFF

WINAK:	CALL	CON		; **3D**
	CALL	WINAKC
	JR	JCOFF
;
KEY7F:	DW	7F7FH
	DB	'KEY'
	DB	1
	CP	1
	JR	NC,KEYL
KEYLI:	CALL	CON		; **3A**
	CALL	KEYLIC		; Keylist
	JR	JCOFF

KEYL:	LD	A,L		; Nummer
KEY:	CALL	CON		; **39**
	CALL	KEYC
	JR	JCOFF

; Komplement des Cursors
CUCP:	PUSH	DE		; **25**
	PUSH	AF
	PUSH	HL
	CALL	PADR0
	JR	C,CUCP3		; ausserhalb
	PUSH	HL
	CALL	DABR
	LD	A,(HL)
	POP	HL
	AND	A		; Dummy?
	JR	NZ,CUCP1
	LD	A,6		; vorletzte
	OR	L		; Cursorzeile
	LD	L,A
	LD	A,(HL)
	XOR	7FH		; Strichcursor
	LD	(HL),A
	JR	CUCP3
	;
CUCP1:	PUSH	BC
	LD	DE,(CUMUST)	; Muster
	LD	B,8		; Zeichenhoehe
CUCP2:	LD	A,(DE)
	XOR	(HL)		; verknuepfen
	LD	(HL),A
	INC	DE
	INC	L
	DJNZ	CUCP2		; 8-mal
	POP	BC
CUCP3:	POP	HL
	POP	AF
	POP	DE
	RET

PUSE:	CALL	CON		; **30**
	CALL	PUSEC		; Punkt setzen
	JR	COF

PUDE:	CALL	CON		; **2F**
	CALL	PUDEC		; Punkt loeschen
	JR	COF

CIRCL:	CALL	CON		; **3F**
	CALL	CIRCLC		; Kreis
	JR	COF

LINE:	CALL	CON		; **3E**
	CALL	LINEC		; Linie
	JR	COF

MBOUT:	CALL	CON		; **38**
	CALL	MBOC		; Byte ausgeben
	JR	COF

MBIN:	CALL	CON		; **37**
	CALL	MBIC		; Byte einlesen
	JR	COF

; CAOS 4.6 - Floppy-Treiber (Sprung in ROM D):

FISRI:	CALL	CON		; ** 0A **
	CALL	FISRID		; Eingabedatei oeffnen
	JR	COF

FMBI:	CALL	CON		; ** 05 **
	CALL	FMBID		; Datenblock einlesen
	JR	COF

FCSRI:	CALL	CON		; ** 0B **
	CALL	FCSRID		; Eingabedatei schliessen
	JR	COF

FISRO:	CALL	CON		; ** 08 **
	CALL	FISROD		; Ausgabedatei oeffnen
	JR	COF

FMBO:	CALL	CON		; ** 01 **
	CALL	FMBOD		; Datenblock schreiben
	JR	COF

FCSRO:	CALL	CON		; ** 09 **
	CALL	FCSROD		; Ausgabedatei schliessen
	JR	COF

FMBIN:	CALL	CON		; ** 37 **
	CALL	FMBIND		; Datenbyte einlesen
	JR	COF

FMBOUT:	CALL	CON		; ** 38 **
	CALL	FMBOUD		; Datenbyte schreiben
COF:	JP	COFF

; BASIC-I/O-Verteiler
BASPV:	PUSH	HL
	PUSH	BC
	CALL	IRMON
	BIT	5,E
	JR	NZ,BASBYE	; BYE
	PUSH	DE
	INC	HL
	INC	HL
	BIT	7,E
	JR	NZ,BSA2
	PUSH	HL
	LD	A,E
	AND	7
	LD	HL,BUPTAB
	ADD	A,L
	LD	L,A
	LD	A,D		; Parameter
	LD	D,E		; Steuerbyte
	LD	E,(HL)		; UP-Nummer
	POP	HL		; Dateiname
	CALL	PV3		; Aufruf
	POP	DE
	LD	D,A		; Ausgabeparameter
	LD	A,E
	AND	4FH
	XOR	43H
	JR	NZ,BSA1
	LD	A,(IX+8)	; Device ausfiltern
	AND	00011100b
	JR	NZ,BSA1		; VERIFY nur bei 00=Tape ausfuehren
	CALL	OSTR
	DB	'VERIFY ?(Y):',0
	CALL	KBD
	CP	'Y'
	CALL	Z,VERIF
BSA1:	LD	A,D
	RES	3,E
	CALL	IRMOF
	POP	BC
	POP	HL
	RET

BSA2:	CALL	KBDS
	POP	DE
	LD	D,A
	JR	NC,BSA1
	RES	7,E
	JR	BSA1

BASBYE:	LD	A,(DATFLG)	; geschuetzt?
	AND	A
	JR	Z,BASB1
	CALL	IRMOF
	CALL	0C641H		; NEW
	CALL	IRMON
BASB1:	LD	D,0
	LD	A,2		; BASIC-ROM
	LD	L,A		; abschalten
	CALL	MODU
	JP	LOOP

ZKOUT0:	CALL	OCHR
ZKOUT:	LD	A,(HL)		; **45**
	INC	HL
	AND	A
	JR	NZ,ZKOUT0
	RET

TON:	CALL	LARG		; **35**
TON1:	BIT	1,(IX+8)	; noch alter Ton
	JR	NZ,TON1
TON2:	PUSH	BC
	LD	C,CTC0
	CALL	TON4		; starten
	INC	C		; CTC1
	EX	DE,HL
	CALL	TON4		; starten
	POP	BC
	LD	A,C		; Lautstaerke
	AND	00011110b	; maskieren
	XOR	10011111b	; Komplement da nullaktiv
	LD	C,A
	LD	A,B
	AND	A		; Dauerton?
	DI
	JR	Z,TON3
	RES	7,C		; nein, Blinkaus
	SET	1,(IX+8)	; neuer Ton
	LD	A,11000111b	; EI,Zaehler 50Hz
	OUT	(CTC2),A	; CTC2
	LD	A,B		; Dauer
	OUT	(CTC2),A
TON3:	IN	A,(PIOBD)
	AND	01100000b	; RAM8 maskieren
	OR	C
	OUT	(PIOBD),A	; Lautstaerke ausgeben
	EI
	RET

; CTC (C) programmieren mit HL:
TON4:	LD	A,L
	AND	A		; Ton?
	LD	L,00000011b
	JR	Z,TON6		; kein Ton
	LD	L,A
	LD	A,00000001b
	AND	H		; Vorteiler
	RRCA			; nach Bit 5 rotieren
	RRCA
	RRCA
	OR	00000111b	; Zeitgeber
	DI
	OUT	(C),A
	EI			; Interrupts wieder ein nach zweitem OUT
TON6:	OUT	(C),L
	RET

; ESC-Funktionen:

ESCPRG:	LD	HL,STBT		; ShSTOP
	SET	4,(HL)		; Kennung, dass ESC-Funktion folgt
	RET

ESC1:	LD	BC,VRAM0	; 0B200H
	JR	ESCVR

ESC2:	LD	BC,VRAM1+05H	; 0AD05H
	JR	ESCVR

ESC3:	LD	BC,VRAM1+04H	; 0AD04H
	JR	ESCVR

ESC4:	LD	BC,VRAM0+01H	; 0B201H
ESCVR:	LD	H,B
	LD	L,0
	LD	(VRAM),HL
	LD	B,0FAH
AXO84:	LD	A,B		; AND/XOR/OUT84
	DI
	AND	(IX+1)
	XOR	C
OUT84:	LD	(IX+1),A
	OUT	(84H),A
	EI
	RET

ESC7:	LD	C,00000100b	; Invers-Bit
XORSTB:	LD	HL,STBT
	LD	A,(HL)
	XOR	C
	LD	(HL),A
	RET

ESC9:	DI
	LD	A,(IX+1)	; VR: A
	XOR	2
	JR	OUT84		; BC unveraendert

ESCA:	LD	HL,STBT
	RES	6,(HL)		; HRG aus
ESCAB:	LD	BC,0FF08h
	CALL	AXO84
	AND	C
	ADD	A,B		; CY = Bit 3
	DI
	IN	A,(PIOBD)
	RLA			; als Bit 0 einschieben
	RRCA			; auf Bit 7 rotieren
	OUT	(PIOBD),A
	EI
	RET

ESCB:	LD	HL,STBT
	SET	6,(HL)		; HRG ein
	JR	ESCAB

ESCC:	LD	C,00100000b	; IBM <-> CAOS
	JR	XORSTB

; Druckerinitialisierung V.24 oder Centronics:
LST7F:	DW	7F7FH
	DB	'LSTDEV'
	DB	1
V24OUT:	CALL	CON		; **47**
	CALL	LSTC
COF1:	JP	COFF

; Reaktion auf ShCLR:
HCPGM:	PUSH	AF
	LD	A,(IX+4)	; CAOS-C merken
	PUSH	AF
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(86h),A		; CAOS-C on
	CALL	HCPGMC
	POP	AF
	LD	(IX+4),A	; wie vorher
	OUT	(86h),A
	POP	AF
	RET

; Duplexroutine
V24D7F:	DW	7F7FH
	DB	'V24DUP'
	DB	1
V24DUP:	CALL	CON		; **48**
	CALL	V24DC
	CALL	C,ERRM
	JR	COF1

; Druckroutine mit Protokoll
ECHO:	CALL	CRT
PRINT:	BIT	7,(IX+4)
	JR	NZ,PR1		; CAOSC on!
	PUSH	HL
	LD	HL,COFF		; nach PRINTC
	EX	(SP),HL		; wieder aus
	CALL	CON
PR1:	JP	PRINTC		; drucken
; Druckroutine fuer M001:			17 Byte
;
; PE:	A	Zeichen
; VR:	-

M001PR:	PUSH	BC
	PUSH	AF
	LD	C,05h		; M001 PIO Port B (Steuersignale)
CBUSY1:	IN	B,(C)
	BIT	2,B		; BUSY?
	JR	NZ,CBUSY1
	LD	B,00000001b	; Bit 0 = Strobe
	DI			; DI wegen Joysticktreiber
	OUT	(04h),A		; M001 PIO Port A (Daten)
	JR	CSTROB
 
; Druckroutine fuer M021:
;
; PE:	A	Zeichen
; VR:	-

M021PR:	PUSH	BC
	PUSH	AF
	LD	C,90h		; M021 PIO Port A (Steuersignale)
CBUSY2:	IN	B,(C)
	BIT	6,B		; BUSY?
	JR	NZ,CBUSY2
	LD	B,10000000b	; Bit 7 = Strobe
	DI			; DI wegen Joysticktreiber
	OUT	(91h),A		; M021 PIO Port B (Daten)
CSTROB:	XOR	A		; Strobe-Impuls aktiv
	OUT	(C),A		; war vorher ED 71 = OUT (C),0
	EI			; (erst nach naechstem Befehl)
	OUT	(C),B		; Strobe-Impuls passiv
	POP	AF
	POP	BC
	RET

; Druckroutine fuer V.24:
;
; PE:	A	Zeichen
; VR:	-

V24PR:	PUSH	BC
	PUSH	AF
	CALL	LCSO
V24PR0:	ADD	A,0Ah		; SIO-Steuerport
	LD	C,A
V24PR1:	IN	A,(C)
	AND	00000100b	; bereit?
	JR	NZ,V24PR2
	INC	A		; A=1
;	CALL	WAIT		; nein: warten (wofuer?)
	JR	V24PR1
V24PR2:	DEC	C		; SIO-Datenport
	DEC	C
	POP	AF
	OUT	(C),A		; Ausgabe
	POP	BC
	RET

; Ausgabe V.24-Duplex:
;
; PE:	A	Zeichen
; VR:	-
;
V24OT:	PUSH	BC
	PUSH	AF
	CALL	LCSOD		; Adresse holen
	JR	V24PR0

; Eingabe V.24-Duplex (Interrupt):
;
; PA:	A	Zeichen
;	CY	Status (1 = BRK, 0 = Zeichen gueltig)
; VR:	AF
;
V24I:	LD	A,11101110b	; Steuerbyte fuer Interrupt-Betrieb
	JR	V24IN0

; Eingabe V.24-Duplex (Polling):
;
; PA:	A	Zeichen
;	CY	Status (1 = BRK, 0 = Zeichen gueltig)
; VR:	AF
;
V24IN:	LD	A,(HCPZ2)
V24IN0:	PUSH	BC
	PUSH	DE
	LD	D,A		; Steuerbyte
	AND	00000100b	; Kanal A oder B?
	RRCA
	RRCA
	ADD	A,0Ah		; SIO-Steuerport
	LD	C,A
	LD	A,01101000b	; Sendeeinstellungen
	AND	D
	LD	E,A
V24IN1:	IN	A,(C)
	AND	00000001b	; CY=0, Daten abholbereit?
	LD	A,5		; WR5 auswaehlen
	DI			; Interrupts waehrend I/O-Sequenz sperren
	OUT	(C),A
	JR	NZ,V24IN2
	LD	A,10000010b	; DTR+RTS ein
	OR	E
	OUT	(C),A		; Senderfreigabe
	EI
	CALL	BRKT
	JR	NC,V24IN1
V24IN2:	OUT	(C),E		; DTR+RTS aus: Sender sperren
	EI
	JR	C,V24IN3
	DEC	C		; SIO-Datenport auswaehlen
	DEC	C
	IN	B,(C)
	LD	A,01111111b	; Maske fuer 7 Bit
	OR	D
	AND	B		; maskieren, CY=0
V24IN3:	POP	DE
	POP	BC
	RET

; SIO-Offset fuer V24OUT
; PA: A (SIO-Offset)
; VR: AF
LCSO:	LD	A,(HCPZ)
LCSO1:	AND	00000100b	; Kanal A oder B?
	RRCA
	RRCA
	RET

; SIO-Offset fuer V24DUP
LCSOD:	LD	A,(HCPZ2)
	JR	LCSO1

;
; ISR SIO B (Empfangsinterrupt)
; fuer MC-Load, MC-Start, Umschaltung auf Fremdtastatur
;
ISRSB:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	HL,V24PL	; Steckplatz M003
	CALL	IRMGET		; aus IRM lesen (Rueckkehr mit EI)
	LD	B,A		; Steckplatz M003 holen
	LD	C,80h
	LD	A,1
	OUT	(C),A		; Modul einschalten
	EXX			; auf Zweitregistersatz wechseln
	 PUSH	DE		; DE' sichern
	 PUSH	BC		; BC' sichern
	 IN	A,(PIOAD)
	 LD	B,A		; Schaltzustand IRM in Register B' merken
	 LD	C,(IX+4)	; Schaltzustand CAOS-C  in Reg. C' merken
	 SETIXA	7,4		; SET 7,(IX+4),A
	 OUT	(86h),A		; CAOS-C einschalten
	EXX
	LD	HL,V24POL
	DI
	IN	A,(9)		; empfangenes Zeichen von SIO abholen
	CALL	INIEA		; DTR off
	CALL	IRET		; EI; RETI

	CP	CR		; Fremdtastatur?
	JR	Z,ISB2
	CP	ESC		; MC-Programm?
	JR	NZ,ISB1

	CALL	V24I		; Folgezeichen nach ESC abholen
	SUB	'T'		; ESC-T ?
	JR	Z,ISB4		; -> MC laden
	DEC	A		; ESC-U ?
	JR	Z,ISB6		; -> MC starten

ISB1:	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	L,B		; Steckplatz
	CALL	IRMGET		; Schaltzustand vor ISR ermitteln
	LD	HL,V24INT	; SIO Kanal B wieder auf Interrupt einstellen
	LD	C,80h
	DI
	CALL	INIEA		; initialisieren
	OUT	(C),A		; M003-Schaltzustand wiederherstellen
	EI
	EXX
	 LD	A,C
	 LD	(IX+4),A
	 OUT	(86h),A		; CAOS-C wie vor Interrupt
	 POP	BC		; BC' regenerieren
	 POP	DE		; DE' regenerieren
	EXX
	JP	POP4		; POP	AF,BC,DE,HL   RET

ISB2:	DI
	LD	(IX-14),LOW(ISRT)	; neue Tastatur-ISR fuer SIO-B in
	LD	(IX-13),HIGH(ISRT)	; Interrupttabelle 01E2H+01E3H
	EI
	JR	ISB1
;
; MC laden (ESC-T, Adresse, Anzahl, Daten):
;
ISB4:	LD	HL,MODST+1	; IRM = internes Modul 1
	CALL	IRMGET		; Modulsteuerwort holen
	AND	1
	RLCA			; auf Bit 2 rotieren
	RLCA
	EXX			; auf Zweitregistersatz wechseln
	 LD	D,A
	 LD	A,01111011b	; USER-C aus
	 AND	B		; gemerkte Schaltzustaende
	 OR	D		; gewuenschter IRM-Schaltzustand
	 LD	D,A
	EXX
	RESIXA	7,4		; RES 7,(IX+4),A
	OUT	(86h),A		; CAOS-C ausschalten
	CALL	V24I
	LD	L,A		; HL = Adresse
	CALL	V24I
	LD	H,A
	CALL	V24I
	LD	E,A		; DE = Anzahl Bytes
	CALL	V24I
	LD	D,A
ISB5:	CALL	V24I
	JR	C,ISB8		; Break
	LD	C,A		; C = Datenbyte
	EXX
	 LD	A,D		; logischer IRM-Schaltzustand
	EXX
	DI
	OUT	(PIOAD),A	; USER-C aus und IRM schalten
	LD	(HL),C		; Daten in Speicher poken
	EXX
	 LD	A,B		; gemerkter IRM-Schaltzustand
	 OUT	(PIOAD),A	; USER-C und IRM wiederherstellen
	 EI
	EXX
	INC	HL
	DEC	DE		; Zaehler verringern
	LD	A,E
	OR	D		; bis Anzahl=0
	JR	NZ,ISB5
ISB8:	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(86h),A		; CAOS-C einschalten
	JR	ISB1
;
; MC starten (ESC-U, Adresse):
;
ISB6:	CALL	V24I
	LD	E,A		; DE = Startadresse
	CALL	V24I
	LD	D,A
	LD	HL,V24INT	; SIO Kanal B wieder auf Interrupt einstellen
	EXX
	 LD	A,B		; gemerkter IRM-Schaltzustand
	EXX
	OR	00000100b	; IRM einschalten
	DI
	CALL	INIEA		; uminitialisieren
	OUT	(PIOAD),A	; IRM schalten
	RESIXA	7,4		; RES 7,(IX+4),A
	OUT	(86h),A		; CAOS-C ausschalten
	LD	SP,(SYSP)
	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	L,B		; Steckplatz
	LD	A,(HL)		; M003-Schaltzustand vor ISR ermitteln
	OUT	(C),A		; und wiederherstellen
	EI
	LD	HL,LOOP		; Rueckkehradresse nach Programmaufruf
	PUSH	HL
	EX	DE,HL
	JP	(HL)		; Programm jetzt starten

; fuer MODIC, FLOAD
LDMAE:	CALL	COFF
	PUSH	AF
	LD	A,(MODST+1)
	AND	1		; IRM off?
	CALL	Z,SCROFF
	POP	AF
	LD	(HL),A		; Byte eintragen
	JR	CON

; Datenbyte holen (fuer DISPC, MODIC, FSAVE, view)
LDAME:	CALL	COFF		; CAOS-C aus
	LD	A,(MODST+1)
	AND	1		; IRM aus?
	CALL	Z,SCROFF
	LD	A,(HL)		; Byte holen
	CALL	SCRON
;	JP	CON		; reinlaufen...

; CAOS-ROM-C einschalten:
CON:	PUSH	AF
	SETIXA	7,4		; SET 7,(IX+4),A
	JR	CSW

SIXD:	CALL	CON		; **31**
	CALL	SIXC
;	JP	COFF		; reinlaufen...

; CAOS-ROM-C ausschalten falls erforderlich:
COFF:	PUSH	AF
	LD	A,(MODST+5)	; Schaltzustand Moduladresse 5
	AND	1
	JR	NZ,CNSW		; bleibt ein!
	RESIXA	7,4		; RES 7,(IX+4),A
CSW:	OUT	(86h),A
CNSW:	POP	AF
	RET

MULT:	CALL	CON		; **41**
	JP	MULC		; BA=D*C

SQR:	CALL	CON		; **40**
	JP	SQRC
;
; ISR fuer CTC2: Tonlaenge
;
ISRC2:	PUSH	AF
	PUSH	HL
	CALL	TOFF		; kehrt mit EI zurueck
	POP	HL
	POP	AF
	RETI
;
; Kassette ein/aus (CD im Tape-Mode)
;
MOT:	IN	A,(PIOAD)
	XOR	01000000b	; Motor ein/aus
	OUT	(PIOAD),A
	RET
;
; CAOS 4.6 - Geraeteumschaltung und Anzeige
; %DEVICE	Auflisten der installierten Treiber
; %DEVICE n	Umschaltung Geraet n (0..7)
;
	DW	7F7Fh
	DB	'DEVICE',1
	AND	A		; Parameter angegeben?
	JR	Z,DRLIST	; nein, Treiber auflisten
	LD	A,L
	CP	8		; falls n<8
	CALL	C,SETDEV	; Treiber einstellen
	CALL	DEVANZ		; Ergebnis anzeigen
	JP	CRLF

; Geraetecode in Bits 2-4 (IX+8) eintragen und Treiberadressen in SUTAB kopieren
; PE:	A	Geraetecode 0...7
;		0 = Kassette - Standard
;		1 = Diskette - D004/D008
;		2 = USB (M052)
;		3 = Netzwerk (M052)
;		4-7 noch nicht benutzt
;		8-FE	Name des aktuellen Treibers anzeigen
;		FF	alle Treiber auflisten
; PA:	CY=1	ausgewaehlter Treiber nicht aktiv
;	IX+8	Bit 2-4 entsprechend Treibernummer gesetzt
; VR:	AF,BC,DE,HL

DEVANZ:	LD	A,0FEh		; aktuellen Treiber anzeigen
	db	21h
DRLIST:	LD	A,0FFh		; Treiber auflisten
SETDEV:	CALL	CON		;					**49**
	CALL	SET_DD		; Routine im ROM C/D aufrufen
SETDC:	JP	COFF

; Geraetetreiber-Funktion aufrufen
; PE:	A	Funktionsnummer
;	A=0	DIR bzw. TAPELIST
;	A=1	DRIVE oder CD
;	A=2	ERA (Datei loeschen)
;	A=3	REN (Datei umbenennen)
;	DE	Dateiname, Maske...
; PA:		entsprechend Treiber
; VR:	AF,BC,DE,HL

DEVFKT:	CALL	CON		; Treiberfunktionen			**4A**
	CALL	FKT_DD		; Routine im ROM C/D aufrufen
	JR	SETDC
;
; Zeigertabelle fuer Unterprogramme
;
ZEI0:	DB	0	; CRT
	DB	38H	; MBOUT
ZEI2:	DB	2	; UOUT1
ZEI3:	DB	3	; UOUT2
ZEI4:	DB	4	; KBD
	DB	37H	; MBIN
	DB	6	; UIN1
	DB	7	; UIN2
;
; Tastaturcodetabelle:
;				 Taste		Scancode:
KTAB:	DB	'Ww'		;		00 01
	DB	'Aa'		;		02 03
	DB	'2"'		;		04 05
	DB	8,19H		; CUL		06 07
	DB	10H,0CH		; HOME		08 09
	DB	'-='		;		0A 0B
	DB	0F2H,0F8H	; F2		0C 0D
	DB	'Yy'		;		0E 0F	
	DB	'Ee'		;		10 11
	DB	'Ss'		;		12 13
	DB	'3#'		;		14 15
	DB	'^]'		;		16 17
	DB	1,0FH		; CLR		18 19
	DB	':*'		;		1A 1B
	DB	0F3H,0F9H	; F3		1C 1D
	DB	'Xx'		;		1E 1F
	DB	'Tt'		;		20 21
	DB	'Ff'		;		22 23
	DB	'5%'		;		24 25
	DB	'Pp'		;		26 27
	DB	1FH,2		; DEL		28 29
	DB	'0',40H		;		2A 2B
	DB	0F5H,0FBH	; F5		2C 2D
	DB	'Vv'		;		2E 2F
	DB	'Uu'		;		30 31
	DB	'Hh'		;		32 33
	DB	'7',27H		; 7 / Apostroph	34 35
	DB	'Oo'		;		36 37
	DB	1AH,14H		; INS		38 39
	DB	'9)'		;		3A 3B
	DB	3,4		; BRK		3C 3D
	DB	'Nn'		;		3E 3F
	DB	'Ii'		;		40 41
	DB	'Jj'		;		42 43
	DB	'8('		;		44 45
	DB	' ['		; SPC		46 47
	DB	'Kk'		;		48 49
	DB	',<'		;		4A 4B
	DB	13H,ESC		; STOP		4C 4D
	DB	'Mm'		;		4E 4F
	DB	'Zz'		;		50 51
	DB	'Gg'		;		52 53
	DB	'6&'		;		54 55
	DB	1CH,1DH		; LIST/RUN	56 57
	DB	'Ll'		;		58 59
	DB	'.>'		;		5A 5B
	DB	0F6H,0FCH	; F6		5C 5D
	DB	'Bb'		;		5E 5F
	DB	'Rr'		;		60 61
	DB	'Dd'		;		62 63
	DB	'4$'		;		64 65
	DB	'_\'		;		66 67
	DB	'+;'		;		68 69
	DB	'/?'		;		6A 6B
	DB	0F4H,0FAH	; F4		6C 6D
	DB	'Cc'		;		6E 6F
	DB	'Qq'		;		70 71
	DB	16H,5		; CAPS		72 73
	DB	'1!'		;		74 75
	DB	LF,12H		; CUD		76 77
	DB	0BH,11H		; CUU		78 79
	DB	9,18H		; CUR		7A 7B
	DB	0F1H,0F7H	; F1		7C 7D
	DB	CR,0EH		; ENTER		7E 7F
;
; Tabelle der ESC-Funktionen:
;
L3TB:	DW	ESC0		; Tabulator
	DW	ESC1		; Anzeige Bild 0	Zugriff Bild 0
	DW	ESC2		;	  Bild 1		Bild 1
	DW	ESC3		;	  Bild 0		Bild 1
	DW	ESC4		;	  Bild 1		Bild 0
	DW	ESC5		; MODUL
	DW	ESC6		; SYSTEM
	DW	ESC7		; PixInvers o/o
	DW	ESC8		; ColSwap o/o
	DW	ESC9		; Farbebene o/o
	DW	ESCA		; HiRes o/o
	DW	ESCB		; HRG o/o
	DW	ESCC		; IBM/CAOS
	DW	ESCD		; DEVICE-Umschaltung
;
; Steuercodetabelle:
;
CRTTAB:	DW	NOOP	; 00
	DW	CLR	; 01
	DW	CLLN	; 02
	DW	NOOP	; 03=BRK
	DW	NOOP	; 04=Sh-BRK
	DW	ESC0	; 05=TAB
	DW	NOOP	; 06
	DW	BEEP	; 07
	DW	CUL	; 08
	DW	CUR	; 09
	DW	CUD	; 0A
	DW	CUU	; 0B
	DW	CLS	; 0C
	DW	CBL	; 0D=Enter
	DW	NOOP	; 0E=Sh-Enter
	DW	HCOPY	; 0F
	DW	HOMEPG	; 10
	DW	PAGEM	; 11
	DW	SCROL	; 12
	DW	NOOP	; 13=STOP
	DW	CLICK	; 14
	DW	NOOP	; 15
	DW	NOOP	; 16=CAPS
	DW	NOOP	; 17
	DW	CEL	; 18
	DW	CBL	; 19
	DW	INS	; 1A
	DW	ESCPRG	; 1B
	DW	NOOP	; 1C=LIST
	DW	NOOP	; 1D=RUN
	DW	NL	; 1E=CONT
	DW	DEL	; 1F
;
; BASIC-Token	(Bit 7 im 1. Byte gesetzt!)
;
TOKTAB:	TOKEN	INKEY$		; D5
	TOKEN	JOYST		; D6
	TOKEN	STRING$		; D7
	TOKEN	INSTR		; D8
	TOKEN	RENUMBER	; D9
	TOKEN	DELETE		; DA
	TOKEN	PAUSE		; DB
	TOKEN	BEEP		; DC
	TOKEN	WINDOW		; DD
	TOKEN	BORDER		; DE
	TOKEN	INK		; DF
	TOKEN	PAPER		; E0
	TOKEN	AT		; E1
	TOKEN	COLOR		; E2
	TOKEN	SOUND		; E3
	TOKEN	PSET		; E4
	TOKEN	PRESET		; E5
	TOKEN	BLOAD		; E6
	TOKEN	VPEEK		; E7
	TOKEN	VPOKE		; E8
	TOKEN	LOCATE		; E9
	TOKEN	KEYLIST		; EA
	TOKEN	KEY		; EB
	TOKEN	SWITCH		; EC
	TOKEN	PTEST		; ED
	TOKEN	CLOSE		; EE
	TOKEN	OPEN		; EF
	TOKEN	RANDOMIZE	; F0
	TOKEN	VGET$		; F1
	TOKEN	LINE		; F2
	TOKEN	CIRCLE		; F3
	TOKEN	CSRLIN		; F4
	TOKEN	DRIVE		; F5
	DB	80H		; Ende-Kennung
;
; Sprungtabelle fuer BASIC-Token
;
TOKJP:	DW	INKEY
	DW	JOYST
	DW	STRING
	DW	INSTR
	DW	RENUM
	DW	DELETE
	DW	PAUSE
	DW	BBEEP
	DW	WINDOW
	DW	0D421H		; BORDER
	DW	INK
	DW	PAPER
	DW	SNER		; AT
	DW	BCOLOR
	DW	SOUND
	DW	PSET
	DW	PRESET
	DW	BLOAD
	DW	SNER		; VPEEK
	DW	VPOKE
	DW	LOCATE
	DW	KEYLIST
	DW	BKEY
	DW	SWITCH
	DW	SNER		; PTEST
	DW	CLOSE
	DW	OPEN
	DW	RANDOM
	DW	SNER		; VGET
	DW	BLINE
	DW	CIRCLE
	DW	SNER		; CSRLIN
	DW	BDRIVE		; Laufwerk wechseln

	ABSFILL	0FDF0h,<ROM-F-Ende>

; High-Byte der Adresse muss konstant sein!
BUPTAB:	DB	16H		; INTB #0
	DB	24H		; OCHR
	DB	37H		; MBIN #1
	DB	38H		; MBOUT
	DB	06H		; USIN1 #2
	DB	02H		; UOT1
	DB	07H		; USIN2	#3
	DB	03H		; UOT2

; Adresse FDF8h fest - wird von anderen Programmen (z.B. TEMO) miz genutzt!
BITTAB:	DB	80H,40H,20H,10H,8,4,2,1
