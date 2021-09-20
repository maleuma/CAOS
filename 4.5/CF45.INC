;*****************************************
;**	CAOS 4.5	ROM F		**
;**					**
;**	Adresse:  F000h bis FDFFh	**
;**					**
;**	letzte Aenderung: 26.08.2010	**
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
JDISK:	CALL	CON
	CALL	JDISC		; DISK-Verteiler
	JP	COFF

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
AHOSTR:	CALL	AHEX		; RG A und folg. ZK
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
	JR	Z,ESC5
	CALL	CON
	CALL	SWIC		; lesen/schalten
	JR	MCOFF

ESC5:	CALL	CON
	CALL	MODULC		; Module listen
	JR	MCOFF

; Module lesen und Schalten
MODU:	CALL	CON		; **26**
	CALL	MODUC
MCOFF:	JP	COFF

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
JUM0:	PUSH	BC
	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
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
	DB	'* KC-CAOS 4.5 '
if beta
	DB	'beta '		; Beta-Version?
endif
	DB	'*',0
MENU:	LD	HL,(ARGN)	; **46**
	LD	E,0		; Parameter zum Testen in E bereithalten
	DEC	L		; ARGN=1?
	JR	NZ,men9		; nein, E=0 verwenden
	LD	E,H		; E=ARG1
men9:	LD	HL,0C000H
	LD	C,L
	LD	B,L		; BC=0
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
	CALL	CRT		; Zeile loeschen
	RET
INP1:	INC	DE		; Zeichen danach
	LD	A,(DE)
	AND	0DFH		; leer?
	RET	Z
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
	LD	A,(MODST+2)
	LD	E,A		; Schaltzustand USER-ROM-C merken
	LD	D,0D1H		; 1. Ebene (BASIC nicht mit durchsuchen)
INP3:	LD	A,2
	LD	L,A
	CALL	MODU		; USER-ROM-C Ebene ein
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
	LD	D,E
	LD	A,2
	LD	L,A
	CALL	MODU		; USER-ROMC wie zuvor
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
	DB	'Falsches Kommando',7,0
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
; Syntax: %go ADR
;
GO7F:	DW	7F7FH
	DB	'go'
	DB	3		; mindestens 1 Argument erforderlich
	JP	(HL)		; ohne Registeruebergabe

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

SAVE7F:	DW	7F7FH
	DB	'SAVE'
	DB	5
	CALL	OSTR
	DB	'Name :',0
	CALL	INLIN
	RET	C		; BRK
	LD	HL,6
	ADD	HL,DE		; Beginn Name
	;
SAVE:	LD	DE,CASS		; **36**
	LD	BC,11
	LDIR			; Name in Puffer kopieren
	XOR	A
	LD	(DE),A
	LD	HL,ARGN		; alle Argumente inklusive ARGN
	LD	E,10H
	LD	C,15H
	LDIR			; in den Puffer kopieren
	CALL	KPUFF		; Standard-Puffer setzen
	CALL	ISRO		; Vorblock ausgeben
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
	JP	C,CSROI		; BRK
	LD	BC,160		; kurzer Vorton
	ld	de,128		; Anzahl - 128
	sbc	hl,de		; Ende erreicht?
	jp	z,csro		; ja, genau das Blockende
	jp	c,csro		; ja, letzter Block mit weniger Bytes
	push	hl		; restliche Anzahl
	CALL	MBO		; Datenblock ausgeben
	JR	SAV1

LARG:	LD	BC,(ARG3)	; **15**
	LD	DE,(ARG2)
	LD	HL,(ARG1)
	LD	A,(ARGN)
	RET

; LOAD-Unterprogramm
; PA: CY=1 BRK gedrueckt
LUP:	INC	(IX+3)		; erwarteter Block
LUP1:	CALL	MBI
	JR	NC,LUP3
	CALL	OSTR
	DB	9,9,9,' ',0
	CALL	BNROST
	DB	' ?'		; fehlerhaft
	DB	1EH,0
	LD	A,(IX+3)
	DEC	A		; Block 01 erwartet?
	JR	Z,LUP1
	CALL	KBD		; nein
	CP	3		; BRK?
	SCF
	RET	Z
	CP	LF		; CUD?
	JR	NZ,LUP1
LUP3:	LD	A,(IX+2)
	LD	B,(IX+3)
	DEC	B		; Block 01 erwartet?
	JR	Z,LUP6
	INC	B
	CP	B		; richtige Blocknummer?
	JR	Z,LUP5
	INC	A		; Block FF?
	JR	Z,LUP5
LUP4:	CALL	BNROST
	DB	'*'		; falsche Blocknummer
	DB	19H,0
	JR	LUP1

LUP5:	CALL	BNROST
	DB	'> '		; korrekter Block
	DB	19H,0
	RET

LUP6:	DEC	A		; Block 01?
	JR	NZ,LUP4
	CALL	CRLF		; keine Blocknummer,
	LD	HL,CASS
	LD	B,11		; sondern den Namen
LUP7:	LD	A,(HL)		; ausgeben
	INC	HL
	CALL	CRT
	DJNZ	LUP7
	JP	SPACE

NOMC:	CALL	OSTR
	DB	'Keine MC-Datei!',0
CLJP:	JP	CSROI

VERI7F:	DW	7F7FH
	DB	'VERIFY'
	DB	1
VERIF:	LD	(IX+7),0	; **11**
	JR	LOAD1

LOAD7F:	DW	7F7FH
	DB	'LOAD'
	DB	1
LOAD:	LD	(IX+7),1	; **10**
	LD	A,(ARGN)
	CP	2
	JR	C,LOAD1		; Autostart?
	SET	1,(IX+7)	; unterdruecken
LOAD1:	CALL	ISRI1
	CALL	KPUFF		; Standard setzen
	LD	(IX+3),0
	CALL	LUP
LOAD2:	JR	C,CLJP		; BRK
	BIT	0,(IX+7)
	JR	Z,LOAD5		; Verify
	LD	L,10H
	LD	A,(HL)		; Anzahl der Argumente
	RLCA
	RLCA			; nach Bits 2..4
	XOR	(IX+7)
	AND	00011100b	; Bits 0..1 und 5..7 zuruecksetzen
	XOR	(IX+7)		; a = (a XOR b) XOR b fuer Bits 2..4
	LD	(IX+7),A
	LD	A,(HL)		; Anzahl der Argumente
	SUB	2
	CP	9		; MC?
	JR	NC,NOMC
	LD	BC,(CASS+15H)
	PUSH	BC
	LD	DE,(CASS+13H)
	LD	HL,(CASS+11H)
	LD	A,(ARGN)
	AND	A		; Ladeoffset?
	JR	Z,LOAD3		; nein
	LD	BC,(ARG1)
	ADD	HL,BC		; Anfangsadresse und Endadresse
	EX	DE,HL		; umrechnen
	ADD	HL,BC
	EX	DE,HL
	BIT	4,(IX+7)	; 4 bis 7 Argumente?
	JR	NZ,LOAD3
	EX	(SP),HL
	ADD	HL,BC		; Startadresse umrechnen
	EX	(SP),HL
LOAD3:	CALL	HLDE		; Anfangsadresse und Endadresse ausgeben
	LD	A,(CASS+10H)
	CP	3
	JR	C,LOAD4		; keine Startadresse
	EX	(SP),HL
	CALL	HLHX		; Startadresse ausgeben
	EX	(SP),HL
LOAD4:	POP	BC		; Startadresse merken
	LD	(ARG3),BC
LOAD5:	CALL	CRLF
	EX	DE,HL		; DE = Ladeadresse
	; Load-Hauptschleife
LOAD6:	CALL	LUP
	JR	C,LOAD2		; BRK
	BIT	0,(IX+7)
	JR	Z,LOAD11	; Verify
	PUSH	HL
	SBC	HL,DE
	LD	BC,128
	SBC	HL,BC
	ADD	HL,BC
	JR	NC,LOAD7
	LD	C,L		; restliche Anzahl
LOAD7:	LD	HL,CASS
	LD	A,(MODST+1)
	AND	1
	JR	NZ,LOAD9	; IRM ein
	LD	B,C
LOAD8:	LD	A,(HL)		; aus dem Puffer
	CALL	SCROFF
	LD	(DE),A		; in den RAM
	INC	HL
	INC	DE
	CALL	SCRON
	DJNZ	LOAD8
	JR	LOAD10
LOAD9:	LDIR			; kopieren
LOAD10:	POP	HL
LOAD11:	INC	(IX+2)		; Blocknummer FF?
	JR	NZ,LOAD6
	;
CSRI:	CALL	CSROI		; **0B**
	LD	A,(IX+7)
	AND	3
	DEC	A
	RET	NZ		; kein Start
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

; Sprung zu ?IO ERROR
JIOERR:	CALL	COFF
	LD	HL,(IOERR)
	CALL	IRMOF
	JP	(HL)

SYS7F:	DW	7F7FH
	DB	'SYSTEM'
	DB	11H
ESC6:	CALL	CON
	CALL	SYSTC
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
	LD	HL,(SUTAB)
	LD	DE,SUBAS
	AND	A
	SBC	HL,DE		; BASEX aktiv?
	JR	NZ,BASB2
	LD	HL,(SUSP)
	LD	(SUTAB),HL
BASB2:	JP	LOOP

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

ESC1:	LD	BC,0B200H
	JR	ESCVR

ESC2:	LD	BC,0AD05H
	JR	ESCVR

ESC3:	LD	BC,0AD04H
	JR	ESCVR

ESC4:	LD	BC,0B201H
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
LST7F:	DW	7F7Fh
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

; Druckroutine fuer M001:
;
; PE:	A	Zeichen
; VR:	F
;
M001PR:	PUSH	BC
	LD	C,05h
CBUSY1:	IN	B,(C)
	BIT	2,B		; BUSY?
	JR	NZ,CBUSY1
	LD	B,00000001b
	DI
	OUT	(04h),A		; Daten
	JR	CSTROB

; Druckroutine fuer M021:
;
; PE:	A	Zeichen
; VR:	F
;
M021PR:	PUSH	BC
	LD	C,90h
CBUSY2:	IN	B,(C)
	BIT	6,B		; BUSY?
	JR	NZ,CBUSY2
	LD	B,10000000b
	DI
	OUT	(91h),A		; Daten
CSTROB:	OUTC0			; OUT (C),0
	EI			; (erst nach naechstem Befehl)
	OUT	(C),B		; Strobe-Impuls
	POP	BC
	RET

; Druckroutine fuer V.24:
;
; PE:	A	Zeichen
; VR:	-
;
V24PR:	PUSH	BC
	PUSH	AF
	CALL	LCSO
V24PR0:	ADD	A,0Ah		; SIO-Steuerport
	LD	C,A
V24PR1:	IN	A,(C)
	AND	00000100b	; bereit?
	JR	NZ,V24PR2
	INC	A		; A=1
	CALL	WAIT		; nein: warten (wofuer?)
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

ISB1:	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
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
	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
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

; Geraetecode in Bits 2-4 (IX+8) eintragen
; PE:	A	Geraetecode 0...7
;		0 = Kassette
;		1 = Diskette
;		2 = USB (M052)
;		3 = Netzwerk (M052)
;		4-7 nicht benutzt
; VR:	AF

DEV_0:	XOR	A		; Kassette = Standardmedium
SETDEV:	RLCA
	RLCA			; nach Bits 2..4
	XOR	(IX+8)
	AND	00011100b	; Bits 0..1 und 5..7 zuruecksetzen
	XOR	(IX+8)		; a = (a XOR b) XOR b fuer Bits 2..4
	LD	(IX+8),A
	RET
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
; Unterprogrammtabelle:
;
SUTB:	DW	CRT	; 00
	DW	MBO	; 01
	DW	UOUT1	; 02
	DW	UOUT2	; 03
	DW	KBD	; 04
	DW	MBI	; 05
	DW	UIN1	; 06
	DW	UIN2	; 07
	DW	ISRO	; 08
	DW	CSRO	; 09
	DW	ISRI	; 0A
	DW	CSRI	; 0B
	DW	KBDS	; 0C
	DW	BYE	; 0D
	DW	KBDZ	; 0E
	DW	COLR	; 0F
	DW	LOAD	; 10
	DW	VERIF	; 11
	DW	LOOP	; 12
	DW	NORM	; 13
	DW	WAIT	; 14
	DW	LARG	; 15
	DW	INTB	; 16
	DW	INLIN	; 17
	DW	RHEX	; 18
	DW	ERRM	; 19
	DW	HLHX	; 1A
	DW	HLDE	; 1B
	DW	AHEX	; 1C
	DW	ZSUCH	; 1D
	DW	SOUT	; 1E
	DW	SIN	; 1F
	DW	NOUT	; 20
	DW	NIN	; 21
	DW	GARG	; 22
	DW	OSTR	; 23
	DW	OCHR	; 24
	DW	CUCP	; 25
	DW	MODU	; 26
	DW	JUMP	; 27
	DW	LDMA	; 28
	DW	LDAM	; 29
	DW	BRKT	; 2A
	DW	SPACE	; 2B
	DW	CRLF	; 2C
	DW	HOME	; 2D
	DW	MODI	; 2E
	DW	PUDE	; 2F
	DW	PUSE	; 30
	DW	SIXD	; 31
	DW	DABR	; 32
	DW	TCIF	; 33
	DW	PADR	; 34
	DW	TON	; 35
	DW	SAVE	; 36
	DW	MBIN	; 37
	DW	MBOUT	; 38
	DW	KEY	; 39
	DW	KEYLI	; 3A
	DW	DISP	; 3B
	DW	WININ	; 3C
	DW	WINAK	; 3D
	DW	LINE	; 3E
	DW	CIRCL	; 3F
	DW	SQR	; 40
	DW	MULT	; 41
	DW	CSTBT	; 42
	DW	INIEA	; 43
	DW	INIME	; 44
	DW	ZKOUT	; 45
	DW	MENU	; 46
	DW	V24OUT	; 47
	DW	V24DUP	; 48
;
; Tastaturcodetabelle:
;
KTAB:	DB	'Ww'
	DB	'Aa'
	DB	'2"'
	DB	8,19H		; CUL
	DB	10H,0CH		; HOME
	DB	'-='
	DB	0F2H,0F8H	; F2
	DB	'Yy'
	DB	'Ee'
	DB	'Ss'
	DB	'3#'
	DB	'^]'
	DB	1,0FH		; CLR
	DB	':*'
	DB	0F3H,0F9H	; F3
	DB	'Xx'
	DB	'Tt'
	DB	'Ff'
	DB	'5%'
	DB	'Pp'
	DB	1FH,2		; DEL
	DB	'0',40H
	DB	0F5H,0FBH	; F5
	DB	'Vv'
	DB	'Uu'
	DB	'Hh'
	DB	'7',27H
	DB	'Oo'
	DB	1AH,14H		; INS
	DB	'9)'
	DB	3,4		; BRK
	DB	'Nn'
	DB	'Ii'
	DB	'Jj'
	DB	'8('
	DB	' ['		; SPC
	DB	'Kk'
	DB	',<'
	DB	13H,ESC		; STOP
	DB	'Mm'
	DB	'Zz'
	DB	'Gg'
	DB	'6&'
	DB	1CH,1DH		; LIST/RUN
	DB	'Ll'
	DB	'.>'
	DB	0F6H,0FCH	; F6
	DB	'Bb'
	DB	'Rr'
	DB	'Dd'
	DB	'4$'
	DB	'_\'
	DB	'+;'
	DB	'/?'
	DB	0F4H,0FAH	; F4
	DB	'Cc'
	DB	'Qq'
	DB	16H,5		; CAPS
	DB	'1!'
	DB	LF,12H		; CUD
	DB	0BH,11H		; CUU
	DB	09H,18H		; CUR
	DB	0F1H,0F7H	; F1
	DB	CR,0EH		; ENTER
;
; Interrupttabelle:
;
ISRTAB:	DW	ISRJ	; (IX-16) M008 PIO A: Joystick
	DW	ISRSB	; (IX-14) M003 SIO B: V.24-Empfang
	DW	ISRPA	; (IX-12) PIO A: Kassette
	DW	ISRPB	; (IX-10) PIO B: Tastatur
	DW	IRET	; (IX-8)  CTC 0: frei (TEMO Schrittbetrieb)
	DW	ISRC1	; (IX-6)  CTC 1: Kassette
	DW	ISRC2	; (IX-4)  CTC 2: Tondauer
	DW	ISRC3	; (IX-2)  CTC 3: Tastatur
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
