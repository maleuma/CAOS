;*****************************************
;**	CAOS 4.7	ROM F		**
;**					**
;**	Adresse:  F000h bis FDFFh	**
;**					**
;**	letzte Aenderung: 30.03.2019	**
;*****************************************

;	ORG	0F000H
				;				Adresse
	JP	PWRON		; >> POWER ON <<		F000
	JR	PV1		; DEFB nr			F003
OUTT1:	DB	0	; CRT
	JR	PV2		; (ARGC)=nr			F006
ZEI2:	DB	2	; UOUT1
	JR	PV3		; E=nr				F009
ZEI3:	DB	3	; UOUT2
	JR	PV4		; E=nr, IRM e/a			F00C
INTT1:	DB	4	; KBD
	JP	RCALL		; DEFW offset			F00F
	JP	BYE		; JUMP-Einsprung		F012
	JP	PV5		; E=nr, IRM/Stack		F015
	JP	IRMON		;				F018
	JP	IRMOF		;				F01B
	JP	PV6		; (ARGC)=nr			F01E
	JP	PV7		; DEVICE-Aufruf, DEFB nr.	F021
				; PV7 ab CAOS 4.7 verfuegbar
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
	EX	(SP),HL
	RET			; UP anspringen

PV3:	SCF			; VR: F
;	JR	PV21
	DB	0EDH		; ED A0 = NOP (Trick aus CAOS 3.4 uebernommern)
PV2:	AND	A		; VR: F
PV21:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	HL,POP3		; Returnadresse
	EX	(SP),HL		; auf Stack legen
	PUSH	HL
	PUSH	AF
	PUSH	DE
	JR	C,PV22		; falls PV3
	LD	A,(ARGC)	; sonst E
	LD	E,A		; beschaffen
	JR	PV22
;
; IRM ein und Stack auf (SYSP) setzen
; aktuellen SP in IY merken
; VR:	BC, F
;
IRMON:	POP	BC		; Rueckkehradresse
	PUSH	IY		; Inhalt von IY auf Anwenderstack legen
	LD	IY,0		; SP in IY merken
	ADD	IY,SP		; (kein UP veraendert IY waehrend IRM ON!)
	DI
	LD	(IX+11),A	; Inhalt von Register A merken
	IN	A,(PIOAD)
	OR	00100100b	; IRM+LED ein
	OUT	(PIOAD),A
	LD	SP,(SYSP)	; System-SP verwenden
	EI
	LD	A,(IX+11)	; Register A regenerieren
	PUSH	BC
	RET			; wie JP (BC)
;
; IRM ausschalten und Stack auf IY setzen
; VR:	BC
;
IRMOF:	POP	BC		; analog IRMON
	LD	(IX+11),A	; Inhalt von Register A merken
	IN	A,(PIOAD)
	RES	2,A		; IRM+LED aus
	RES	5,A		; (F unveraendert)
	DI
	OUT	(PIOAD),A
	LD	SP,IY		; gemerkten Anwender-SP regenerieren
	EI
	LD	A,(IX+11)	; Register A regenerieren
	POP	IY		; Inhalt von IY von Anwenderstack holen
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
;_____________________________________________________________
;							**23**
; Ausgabe einer Zeichenkette
; Zeichektette hinter dem CALL, Ende mit 00h
; PE/PA: -
; VR: AF

OSTR:	EX	(SP),HL
	CALL	ZKOUT		; Zeichenkette ausgeben
	EX	(SP),HL
	RET

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

;SYS7F:	DW	7F7FH		; Dieses Menuewort ist ab CAOS 4.6 nicht
;	DB	'SYSTEM'	; mehr erforderlich - macht SWITCH mit!
;	DB	11H
ESC6:	CALL	CON
	CALL	SYSTC		; Systemcheck (interne Module 0-5)
ESC61:	LD	DE,(CURSO)	; aktuelle Cursorposition mitnehmen
	JR	MCOFF

; Module lesen und Schalten
MODUL2:	LD	L,2		; 2=USER-ROM
MODUSW:	LD	A,2		; Modul schalten
;_____________________________________________________________
;							**26**
; Module lesen und Schalten
; PE:	A=0 oder 1	Lesen
;	A=2 oder >2	Schalten
;	L	Steckplatz
;	D	Steuerbyte
; PA:	H	Modultyp (Strukturbyte)
;	D	Modulsteuerbyte
; VR:	AF,H,BC

MODU:	CALL	CON
	CALL	MODUC
MCOFF:	JP	COFF

ESCD:	CALL	CON		; DEVICE-Umschaltung
	CALL	ESCDD
	JR	ESC61

; Betriebssystem wechseln:

JUMP7F:	DW	7F7FH
	DB	'JUMP'
	DB	3
	LD	A,L		; Steckplatz
;_____________________________________________________________
;							**27**
; Sprung in ein neues Betriebssystem
; PE:	A	Modulsteckplatz

if SYSROM
JUMP:	cp	4
	jr	c,JUMPE		; JUMP 0 bis JUMP 3
else
JUMP:	OR	A
	JR	Z,BYE		; JUMP 0 entspricht RESET
endif
	LD	B,A
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
	LD	A,0FDH		; Ausgabe FD (Adresse C000, schreibgeschuetzt)
	LD	L,B
	LD	(HL),A		; Eintrag
	DI
	OUT	(C),A		; einschalten
	IN	A,(PIOAD)
	AND	01111110b	; interne ROMs abschalten
	JP	NCAOS		; Sprung ueber IRM
if SYSROM
;
; JUMP-Erweiterung: Wechsel der CAOS-System-Ebene
;
JUMPE:	rlca			; Bit 1 und 0 nach 
	rlca			; Bit 3 und 2 rotieren
	ld	h,a
	xor	(ix+4)
	and	0f3h		; Bit 2-3 ruecksetzen
	xor	h		; CAOS-Bank waehlen
	di			; Interrupts global sperren
	ld	(ix+4),a	; CAOS-Ebene eintragen
	ld	a,PORT2
	ld	(NCAOS+1),a	; nicht CAOS abschalten, Ebene wechseln!
	JP	NCAOS		; Sprung in neues Systm ueber IRM
endif

; Einschalt-Initialisierung:

PWRON:	LD	A,0E3H		; RAM4 WR einschalten, CAOS-Bank 0
	OUT	(PORT2),A	; und CAOS-C ein
	JP	PWRONC		; Programm im ROM-C weiter abarbeiten ...
;
; Tasten-RESET und JUMP-Einsprung:
;
BYE:	LD	SP,STACK	; System-Stack setzen
if SYSROM
	LD	A,(01F4h)	; IX+4 auch wenn IX noch nicht definiert ist
	OR	0E3H		; RAM4 WR einschalten, CAOS-Bank erhalten
else
	LD	A,0E3H		; RAM4 WR einschalten, CAOS-Bank 0
endif
	OUT	(PORT2),A	; und CAOS-C ein
	CALL	SYSI		; Systeminitialisierung
PWR4:	CALL	V24INI		; M003 suchen und initialisieren
	CALL	COFF		; CAOS-C ausschalten (Standard)
	LD	BC,880H		; auf Steckplatz 8
	IN	A,(C)		; ROM-Modul mit Strukturbyte
	DEC	A		; 01 vorhanden?
	JR	NZ,PWR6		; nein!
	LD	A,41H		; 43H bis CAOS 4.6, also mit Schreibfreigabe
	OUT	(C),A		; R/O einschalten auf Adresse 4000H
	LD	(MODST+8),A	; Schaltzustand eintragen
	LD	A,(IX+4)
	AND	0FCH		; internen RAM4 ausschalten
	LD	(IX+4),A
	OUT	(PORT2),A
	JP	4000H		; Adresse 4000h im Modul anspringen
	;
PWR6:	LD	B,0FCH
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	NZ,PWR7		; nein
;	OUT	(C),A		; einschalten mit Steuerbyte A7H
; Befehl kann entfallen, da die Kopplung fuer die Treibersuche bereits ein ist!
	LD	BC,UROK		; 0B3F3H
	IN	A,(C)		; Betriebsart ermitteln
	DEC	A
	JR	Z,PWR7		; 1=CP/M-Betriebsart erkannt
	CP	4
	JR	Z,PWR7		; 5=CAOS-Betriebsart erkannt
	CALL	OSTR
	DB	0CH,LF		; Ansonsten: Autostart mit JUMP FC
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
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Epilogverwaltung:		(bereits in CAOS 2.5 von Frank Klemm realisiert)
; -----------------
; Epiloge 0 ... 1F
;
; Bit:  4      3      2      1      0
;	I      I      I	     I	    \
;  0: mindes-  -------+-------	   IRM 
;      tens	  Anzahl der
;  1:  genau	   Argumente	  on/off
;
; wenn Epilog 1F -> dann keine Auswer-
; tung der Argumente, DE Zeiger auf Args
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Menue Display
;
	DW	7F7FH
	DB	'MENU'
	DB	1
	POP	HL		; Stack clear
MEN0:	CALL	OSTR
	DB	0CH,LF
	DB	'* KC-CAOS 4.7 ',0
	CALL	DEVANZ		; aktuelles Device anzeigen
	CALL	OSTR
	DB	' *',0
;_____________________________________________________________
;							**46**
; Menue anzeigen und Uebergang zur Kommandoeingabe
; PE:	(IX+9)	Prologbyte
;	ARGN=1	ARG1 auswerten
;	ARG1=0	MENU normal anzeigen
;	ARG1=1	versteckte Menueworte mit anzeigen
;	ARG1=2	Adressen mit anzeigen
;	ARGN=3	versteckte Menueworte und Adressen anzeigen
; PA/VR: -

MENU:	LD	HL,(ARGN)
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
	LD	A,(IX+9)	; Prologbyte
MEN4:	CPIR
	JP	PO,LOOP2	; ausgesucht
	CPI			; 2mal nacheinander?
	JP	PO,LOOP2	; ausgesucht
	JR	NZ,MEN4		; nicht 2mal
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
;_____________________________________________________________
;							**12**
; Eingabeschleife:

LOOP:	IN	A,(PIOAD)
	AND	11011111b	; LED aus
	OR	00000100b	; IRM ein
	OUT	(PIOAD),A
	CALL	COFF
	LD	A,CR		; Zeilenanfang
LOOP1:	CALL	OPRCHR		; Prompt ausgeben
LOOP2:	CALL	INPUT
	JR	LOOP

; Menuekommandoroutine

INPUT:	CALL	INLIN		; Eingabe
	JR	NC,INPT1	; mit Enter abgeschlossen
	LD	A,0BH		; bei BRK
	CALL	CRT		; Cursor hoch
	LD	A,2		; und
	JP	CRT		; Zeile loeschen
	;
INPT1:	INC	DE		; Zeichen nach Prompt
	LD	A,(DE)
	AND	0DFH		; leer?
	RET	Z
	LD	A,(MODST+2)	; Schaltzustand USER-ROM
	LD	(ARGC),A	; CAOS 4.6: hier uebergeben an USER-Programme
	CALL	MSUCH		; Suche ab C000H
	JR	C,INPT5		; gefunden
	LD	A,(MODST+5)
	AND	1
	JR	NZ,INPT2	; CAOS-C war bereits an
	CALL	CON
	LD	B,20H		; ansonsten 2000h Bytes
	CALL	MS1		; durchsuchen im CAOS-ROM-C
	JR	C,INPT5		; gefunden
INPT2:	PUSH	DE
if SYSROM
	LD	D,0C9H		; 1. Ebene (BASIC nicht mit durchsuchen)
else
	LD	D,0D1H		; 1. Ebene (BASIC nicht mit durchsuchen)
endif
INPT3:	CALL	MODUL2		; USER-ROM-C Ebene ein
	RESIXA	7,4		; RES 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-ROM-C aus!
	EX	DE,HL
	EX	(SP),HL		; DE=Suchkette
	EX	DE,HL
	LD	B,20H		; Suchlaenge 2000h Bytes
	CALL	MS1		; Menuewort in USER-ROM suchen
	JR	C,INPT4		; gefunden
	EX	DE,HL
	EX	(SP),HL		; DE=Schaltzustand
	EX	DE,HL
if SYSROM
	LD	A,8		; Offset zur naechsten Ebene: C1, C9, D1 ... F9
else
	LD	A,10H		; Offset zur naechsten Ebene: C1, D1, E1, F1
endif
	ADD	A,D		; naechste Ebene berechnen
	LD	D,A
	JR	NC,INPT3	; naechste Ebene
	LD	DE,(ARGC-1)	; CAOS 4.6: D = USERC-Schaltzustand vor Suche
	CALL	MODUL2		; USER-ROMC wie zuvor
	POP	DE
	JR	ERR1		; invalid command
	;
INPT4:	POP	AF		; Schaltzustand USER-ROM-C vergessen
	XOR	A
	LD	(MODST+5),A	; CAOS-C ausschalten
INPT5:	LD	A,(HL)		; Epilog
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
	JR	Z,INPT6
	BIT	4,C
	JR	NZ,ERR2		; zu viele arg
INPT6:	BIT	0,C
	CALL	LARG		; Arg's laden
	RET	NZ		; mit IRM on
	JP	SCROFF		; mit IRM off

ERR1:	CALL	OSTR
	DB	'Kommando?',7,0
	JR	ERRCR
	;
ERR2:	CALL	OSTR
	DB	'Zu viele',0
	JR	ERRA
	;
ERR3:	CALL	OSTR
	DB	'Zu wenig',0
	JR	ERRA
	;
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
;_____________________________________________________________
;							**1D**
; Zeichenkette/Menuewort suchen
; PE:	A	Prologbyte (bei CAOS: 7FH)
;	BC	Laege des Suchbereiches
;	DE	Anfang der Vergleichskette
;	HL	Anfang des Suchbereichs
; PA:	DE	Ende+1 Vergleichskette (wenn gefunden)
;	HL	Ende+1 gefundene Kette
;	CY=1	Kette gefunden
; VR:	AF,BC,DE,HL

ZSUCH:	CPIR
	SCF
	CCF			; CY=0
	RET	PO		; Prolog nicht gefunden
	CPI
	JR	NZ,ZSUCH	; nur 1x gefunden
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
;_____________________________________________________________
;							**13**
; Normalein- und Ausgabe: CRT und KBD
; PE:	-
; PA:	HL	alter Ausgabezeiger
; VR:	HL

NORM:	CALL	NIN		; Normaleingabe
;_____________________________________________________________
;							**20**
; Setzen eines neuen Output Kanals

NOUT:	LD	HL,OUTT1
;_____________________________________________________________
;							**1E**
SOUT:	PUSH	HL
	LD	HL,(OUTAB)	; bisheriger Zeiger
	EX	(SP),HL
	LD	(OUTAB),HL	; neuer Zeiger
	POP	HL
	RET
;_____________________________________________________________
;							**21**
; Setzen eines neuen Input Kanals

NIN:	LD	HL,INTT1
;_____________________________________________________________
;							**1F**
SIN:	PUSH	HL
	LD	HL,(INTAB)	; bisheriger Zeiger
	EX	(SP),HL
	LD	(INTAB),HL	; neuer Zeiger
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
;_____________________________________________________________
;							**1A**
; 16-Bit-Wert hexadezimal anzeigen und ein Leerzeichen danach

HLHX:	LD	A,H
	CALL	AHEX
ALSPC:	LD	A,L
AHSPC:	CALL	AHEX
;_____________________________________________________________
;							**2B**
; Leerzeichen anzeigen

SPACE:	LD	A,' '		; Leerzeichen
	JR	OCHR
;_____________________________________________________________
;							**2D**
; Cursor in Home-Position

HOME:	LD	A,10H		; Home-Code
	JR	OCHR
;_____________________________________________________________
;							**1C**
; 8-Bit-Wert hexadezimal anzeigen

AHEX:	PUSH	AF
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
;_____________________________________________________________
;							**24**
; Ausgabe eines Zeichens ueber def. Kanal

OCHR:	PUSH	HL
	LD	HL,(OUTAB)	; Ausgabekanal
INTA1:	PUSH	DE		; siehe auch PV1 (sehr aehnlich)
	PUSH	BC
	LD	E,(HL)		; UP-Nr. der Routine
	LD	D,0
	LD	HL,(SUTAB)
	PUSH	AF
	ADD	HL,DE
	ADD	HL,DE
	POP	AF
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; DE = Adresse der Routine
	EX	DE,HL
	; hier CAOS-C ausschalten
	CALL	JPHL		; Routine aufrufen
	; hier CAOS-C wieder einschalten wie er war
POP3:	POP	BC		; auch fuer PV2
	POP	DE		; bis PV6!
	POP	HL
	RET
;_____________________________________________________________
;							**16**
; Eingabe eines Zeichens ueber def. Kanal
;
INTB:	PUSH	HL
	LD	HL,(INTAB)	; Eingabekanal
	JR	INTA1
;_____________________________________________________________
;							**04**
; Eingabe eines Zeichens mit Cursoreinblendung
;
KBD:	PUSH	HL
	PUSH	DE
	PUSH	BC
KBD1:	BIT	6,(IX+8)	; F-Taste aktiv?
	JR	NZ,KBD9		; ja
KBD2:	LD	E,0
KBD3:	CALL	PV1
	DB	25h		; CUCP - blinken
	INC	E		; e/a-Merker
	LD	B,15		; F00h mal
KBD4:	CALL	KBDZ		; abfragen
	JR	C,KBD5
	DEC	BC
	LD	A,B
	OR	C
	JR	NZ,KBD4		; warten,
	JR	KBD3		; dann Cursor wieder weg usw.
	;
KBD5:	BIT	0,E		; gerade ein?
	JR	Z,KBD5a
	CALL	PV1		; Cursor wegnehmen
	DB	25h		; CUCP
KBD5a:	LD	D,A
	BIT	5,(IX+8)	; Click ein?
	CALL	NZ,CLIK		; Click-Ton erzeugen
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
	LD	HL,FTASTE	; 0B900H
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
;_____________________________________________________________
;							**17**
; Eingabe einer Zeile mit Funktion aller Cursortasten,
; Abschluss mit <ENTER> oder Abbruch <BRK>

INLIN:	CALL	INTB
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
	LD	A,(WINLG+1)	; Anzahl Zeilen
	LD	D,A		; D muss dann in letzte Zeile zeigen
INLI2:	DEC	D		; Zeile drueber
	PUSH	HL
	CALL	DABR
	EX	DE,HL
	POP	HL		; PA: DE
	POP	AF		; PA: CY=BRK
NOOP:	RET
;
; Sprung zu einer Adresse mit Registeruebergabe
; Syntax: %go ADR [HL] [DE] [BC] [A]
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
;_____________________________________________________________
;							**19**
; ERROR anzeigen

ERRM:	CALL	OSTR
	DB	'ERROR',7,0
;_____________________________________________________________
;							**2C**
; Neue Zeile mit CR,LF
; PA:	A=0
; VR:	AF
;
CRLF:	CALL	OSTR
	DB	CR,LF,0
	RET
;_____________________________________________________________
;							**1B**
; Zwei 16-Bit-Werte hexadezimal anzeigen und
; ein Leerzeichen danach

HLDE:	CALL	HLDE1		; 1x aufrufen, dann reinlaufen
HLDE1:	CALL	HLHX		; HL anzeigen
	EX	DE,HL		; HL <-> DE tauschen
	RET
;_____________________________________________________________
;							**18**
; Erfassung einer eingegebenen Hex-Zahl
; PE:	DE	Adresse im VRAM
; PA:	(NUMNX)	Anzahl der HEX-Zeichen (0-4)
;	(NUMVX)	Wert der erfassten Zahl
;	CY=1	Fehler
;
RHEX0:	INC	DE
RHEX:	LD	A,(DE)
	CP	' '		; Trennzeichen
	JR	Z,RHEX0		; uebergehen
	XOR	A
	LD	HL,NUMVX+1
	LD	(HL),A
	DEC	HL		; HL=NUMVX
	LD	(HL),A
	DEC	HL		; HL=NUMNX
	LD	(HL),A		; alles auf 0 setzen
; Erfassen der ASCII-Zeichen
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
	CP	10
	RET	C
	CP	16
	CCF
	RET	C
RH2:	INC	DE		; naechstes Zeichen
	INC	(HL)		; NUMNX erhoehen
	INC	HL
	RLD			; Tetrade
	INC	HL
	RLD			; durchschieben
	DEC	HL
	DEC	HL
	JR	Z,RH1		; RLD setzt auch Z-Flag!
	DEC	DE
	SCF			; Fehler: Zahl zu gross
	RET
;_____________________________________________________________
;							**22**
; Bis zu 10 Argumente erfassen
; PE:	DE	Adresse im VRAM
; PA:	(ARGN) und (ARG1) bis (ARG9)
;	CY=1	Fehler
;
GARG:	LD	BC,ARGN
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
	LD	(BC),A		; LOW(ARG) eintragen
	INC	HL
	INC	BC
	LD	A,(HL)		; HIGH(NUMVX)
	LD	(BC),A		; HIGH(ARG) eintragen
	LD	L,LOW(ARGN)	; HL=ARGN
	INC	(HL)
	LD	A,(HL)
	ADD	A,-11		; 10 Argumente?
	JR	NC,GARG1
	RET			; Fehler: zu viele Argumente

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
;_____________________________________________________________
;							**43**
; E/A-Kanal initialisieren
; PE:	HL	Tabelle mit Initialisierungsdaten
;		1. Byte: E/A-Adresse
;		2. Byte: Anzahl
;		danach die Initialisierungsbytes
; VR:	F, HL

INIEA:	PUSH	BC
	LD	C,(HL)		; erstes Byte: Portadresse
	INC	HL
	LD	B,(HL)		; zweites Byte: Anzahl
	INC	HL
	OTIR			; ... Bytes
	POP	BC
	RET
;
; CAOS-interne Initialisierung meherer E/A-Kanaele ohne DI/EI
; PE:	HL	Initialisierungsdaten mit vorangestelltem Byte fuer Anzahl

INIMEX:	LD	D,(HL)		; Anzahl Kanaele
	INC	HL
INIMX1:	CALL	INIEA
	DEC	D
	JR	NZ,INIMX1
	RET
;_____________________________________________________________
;							**44**
; Initialisierung mehrerer E/A-Kanaele
; PE:	HL	Tabelle mit Initialisierungsdaten
;	D	Anzahl der Kanaele
; VR:	F, D, HL

INIME:	DI
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
;_____________________________________________________________
;							**36**
; Datei speichern

SAVE:	CALL	CLC		; Kassettenpuffer loeschen
;	LD	DE,CASS		; DE wird von CLC uebergeben
	PUSH	HL		; Dateiname
	LD	BC,11
	LDIR			; Name in Puffer kopieren
	XOR	A
	LD	(DE),A		; Ende-0 anhaengen
	LD	HL,ARGN		; alle Argumente inklusive ARGN
	LD	E,10H
	LD	C,15H
	LDIR			; in den Puffer kopieren
	CALL	KPUFF		; Standard-Puffer setzen
	POP	HL		; Dateiname
	CALL	PV7		; Vorblock ausgeben
	DB	2		; ISRO
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
	LD	BC,160		; kurzer Vorton
	JR	C,SAV5		; BRK -> Datei schliessen
	ld	de,128		; Blocklaenge
	sbc	hl,de		; Ende erreicht?
	jr	z,SAV5		; ja, genau das Blockende
	jr	c,SAV5		; ja, letzter Block mit weniger Bytes
	push	hl		; restliche Anzahl
	CALL	PV7		; Datenblock ausgeben
	DB	0		; MBO
	JR	NC,SAV1		; kein Fehler
	pop	hl		; restliche Byte-Anzahl vom Stack nehmen
	;
SAV5:	CALL	PV7		; Ausgabe Endeblock 0FFH
	DB	3		; CSRO
	CALL	CLC		; CAOS 4.6: Puffer loeschen nicht mehr in CSRO
	CALL	CRLF		; CAOS 4.6: Zeilenvorschub nicht mehr in CSRO
	RET
;_____________________________________________________________
;							**15**
; Laden der Argumente in Register
; PA:	HL, DE, BC, A mit Daten der Argumente
;
LARG:	LD	BC,(ARG3)
	LD	DE,(ARG2)
	LD	HL,(ARG1)
	LD	A,(ARGN)
	RET

; Lesen eines Blockes mit Ausschrift Blocknummer bzw. Name
;
; PE:	(IX+3)	zu lesender Block
; VR:	AF,BC	(Block<>1) ( 00>  )
;	AF,BC,HL (Block=1) ( Name )
; PA:	CY=1	BRK gedrueckt nach Lesefehler oder Fehler bei Device nicht TAPE

RDBLK:	INC	(IX+3)		; erwarteter Block
LUP1:	CALL	PV7
	DB	1		; MBI
LUP2:	JR	NC,LUP3		; kein Fehler aufgetreten

	CALL	DEV1		; Kassette?
	SCF			; Fehlerflag wieder setzen
	RET	NZ		; Abbruch bei Fehler wenn kein TAPE

	CALL	OSTR		; bei Kassette nach Wiederholung fragen:
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

	LD	HL,CASS		; Dateiname in Kassettenpuffer
	LD	B,11
LUP7:	LD	A,(HL)		; auf gueltige Zeichen testen
	OR	A
	JR	Z,LUP8		; 00h ist OK
	CP	20h
	JR	C,LUP10		; <20h also raus
	CP	7Fh
	JR	C,LUP8		; 20h..7Eh ist OK
	CP	0D3h
	JR	C,LUP10		; 7Fh..D2h raus
	CP	0D6h		; (D3..D5 sind BASIC-Dateitypen)
	JR	NC,LUP10	; >D5h also raus
LUP8:	INC	L
	DJNZ	LUP7
	LD	L,0		; wieder zurueck auf Anfang Kassettenpuffer
	LD	B,11
LUP9:	LD	A,(HL)		; aus Kassettenpuffer anzeigen
	AND	7Fh		; hohe Zeichen fuer BASIC-Dateiname
	INC	HL
	OR	A
	CALL	NZ,CRT		; nur echte Zeichen anzeigen
	DJNZ	LUP9
	CALL	SPACE		; Leerzeichen zwischen Dateiname + Adressen
LUP10:	AND	A		; kein Fehler
	RET

NOMC:	CALL	OSTR
	DB	'Keine MC-Datei!',0
CLJP:	CALL	CSRI		; Datei schliessen
	SCF			; Fehlerflag setzen
	RET

VERI7F:	DW	7F7FH
	DB	'VERIFY',1
;_____________________________________________________________
;							**11**
; Kassettendatei kontrollieren

VERIF:	CALL	DEV1		; Kassette?
	RET	NZ		; nur bei TAPE sinnvoll
	LD	(IX+7),A	; A=0
	JR	LOAD1

LOAD7F:	DW	7F7FH
	DB	'LOAD',1
	CALL	DEV1		; Kassette?
	CALL	NZ,NAME		; Dateiname abfragen falls nicht TAPE
	RET	C		; mit BRK abgebrochen
;_____________________________________________________________
;							**10**
; Datei einlesen

LOAD:	LD	(IX+7),1	; Daten lesen
	LD	A,(ARGN)
	CP	2
	JR	C,LOAD1		; Autostart?
	SET	1,(IX+7)	; unterdruecken
LOAD1:	CALL	KPUFF		; Standardpuffer setzen
	CALL	PV7		; HL = Dateiname
	DB	4		; ISRI (Vorblock einlesen)
	JR	NC,LOAD0	; kein Fehler aufgetreten
	CALL	DEV1		; Kassette?
	SCF			; Fehlerflag setzen
	RET	NZ		; Abbruch wenn nicht Kassette
LOAD0:	CALL	LUP2		; Blockinhalt auswerten (Name anzeigen usw.)
LOAD2:	JR	C,CLJP		; BRK -> CSRI -> Fehler
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
	JR	NC,NOMC		; Fehler: kein gueltiger CAOS-Vorblock
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
	ADD	HL,BC		; Startadresse auch umrechnen
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
	JR	C,LOAD2		; BRK -> CSRI -> Fehler melden
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
; CAOS 4.7:
; Bei VERIFY wird mit Blocknummer FF der Vergleich beendet
;
LOAD11:	INC	(IX+2)		; Blocknummer FF?
	JR	Z,LOAD12	; ja, bei VERIFY Einlesen beenden
	JR	LOAD6
	;
LOAD9:	LDIR			; gelesenen Block kopieren
LOAD10:	POP	HL
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
	CALL	PV7		; Datei schliessen
	DB	5		; CSRI
	RET	C		; Fehler
	LD	A,(IX+7)
	AND	3
	DEC	A
	RET	NZ		; kein Start oder Verify
	LD	A,(IX+7)
	AND	1CH
	CP	0CH
	LD	(IX+7),0	; Befehl fehlte in CAOS 4.3!
	CCF			; kein Fehler
	RET	NC		; keine Startadresse
	LD	HL,(ARG3)	; Startadresse
	JP	(HL)		; anspringen

	DW	7F7FH
	DB	'COLOR'
	DB	1
	CALL	CON
	JP	COLRC
;_____________________________________________________________
;							**0F**
COLR:	CALL	CON
	JP	SETCO
;_____________________________________________________________
;							**42**
CSTBT:	PUSH	HL
	LD	HL,STBT
	SET	3,(HL)
	CALL	OCHR		; Steuerzeichen darstellen
	RES	3,(HL)
	POP	HL
	RET

	DW	7F7FH
	DB	'DISPLAY'
	DB	3
;_____________________________________________________________
;							**3B**
DISP:	CALL	CON
	CALL	DISPC
	JR	JCOFF

	DW	7F7FH
	DB	'MODIFY'
	DB	3
;_____________________________________________________________
;							**2E**
MODI:	CALL	CON
	CALL	MODIC
JCOFF:	JP	COFF

WIND7F:	DW	7F7FH
	DB	'WINDOW'
	DB	1
	CALL	CON
	CALL	WINDC
	JR	JCOFF
;_____________________________________________________________
;							**3C**
; PE:	A	Fensternummer 0..9
;	HL	Fensteranfang
;	DE	Fenstrergroesse
; PA:	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
WININ:	CALL	CON
	CALL	WININC
	JR	JCOFF
;_____________________________________________________________
;							**3D**
; PE:	A	Fensternummer 0..9
; PA:	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
WINAK:	CALL	CON
	CALL	WINAKC
	JR	JCOFF
;
KEY7F:	DW	7F7FH
	DB	'KEY'
	DB	1
	CP	1
	JR	NC,KEYL
;_____________________________________________________________
;							**3A**
KEYLI:	CALL	CON
	CALL	KEYLIC		; Keylist
	JR	JCOFF

KEYL:	LD	A,L		; Nummer
;_____________________________________________________________
;							**39**
KEY:	CALL	CON
	CALL	KEYC		; Taste belegen
	JR	JCOFF
;_____________________________________________________________
;							**25**
; Cursor komplementieren

CUCP:	PUSH	DE
	PUSH	AF
	PUSH	HL
	CALL	PADR0		; Adresse im VRAM berechnen
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
;_____________________________________________________________
;							**30**
; Punkt setzen
; PE:	(HOR)	Horizontalkoordinate
;	(VERT)	Vertikalkoordinate
;	(FARB)	Bildpunktfarbe
; PA:	CY=1	Punkt auáerhalb (Fehler)
; VR:	AF
;
PUSE:	CALL	CON
	CALL	PUSEC		; Punkt setzen
	JR	COF
;_____________________________________________________________
;							**2F**
; Punkt loeschen
; PE:	(HOR)	Horizontalkoordinate
;	(VERT)	Vertikalkoordinate
;	(FARB)	Bildpunktfarbe
; PA:	CY=1	Punkt auáerhalb (Fehler)
;	Z=1	Punkt war nicht gesetzt
;	A	Farbbyte
; VR:	AF
;
PUDE:	CALL	CON
	CALL	PUDEC		; Punkt loeschen
	JR	COF
;_____________________________________________________________
;							**3F**
; Kreis zeichnen
; PE:	(ARG1)	X-Koordinate Mittelpunkt
;	(ARG2)	Y-Koordinate Mittelpunkt
;	(ARG3)	Radius
;	(FARB)	Farbe des Kreises
; PA:	-
; VR:	AF, BC, DE, HL, BC', DE', HL'

CIRCL:	CALL	CON
	CALL	CIRCLC		; Kreis
	JR	COF
;_____________________________________________________________
;							**3E**
; Linie zeichnen
; PE:	(ARG1)	X-Koordinate-Anfang
;	(ARG2)	Y-Koordinate-Anfang
;	(ARG3)	X-Koordinate-Ende
;	(ARG4)	Y-Koordinate-Ende
;	(FARB)	Farbe der Linie
; PA:	-
; VR:	AF, BC, DE, HL, AF', BC', DE', HL'

LINE:	CALL	CON
	CALL	LINEC		; Linie
COF:	JP	COFF
;_____________________________________________________________
;							**37**
; Byteweise Eingabe:

MBIN:	CALL	CON		; CAOS-ROM C ein
	CALL	MBIC		; Byte einlesen
	JR	MBERR
;_____________________________________________________________
;							**38**
; Byteweise Ausgabe:

MBOUT:	CALL	CON		; CAOS-ROM C ein
	CALL	MBOC		; Byte ausgeben
MBERR:	CALL	COFF		; CAOS-ROM C aus
	RET	NC		; kein Fehler
;	JP	JIOERR		; Sprung zu Fehlerroutine in BASIC-ROM

; BASIC: Sprung zu ?IO ERROR
;JIOERR:	CALL	COFF
BD2:	LD	HL,(IOERR)	; Adresse aus IRM holen
	CALL	IRMOF		; IRM aus
	JP	(HL)		; Sprung in BASIC-ROM

; BASIC-I/O-Verteiler
; PE:	E	Steuerbyte
;	Bit 0..2 Kanalauswahl
;		00 = Eingabe Kanal 0 = Tastatur
;		01 = Ausgabe Kanal 0 = Bildschirm
;		02 = Eingabe Kanal 1 = Kassette
;		03 = Ausgabe Kanal 1 = Kassette
;		04 = Eingabe Kanal 2 = Anwenderkanal 1
;		05 = Ausgabe Kanal 2 = Anwenderkanal 1
;		06 = Eingabe Kanal 3 = Anwenderkanal 2
;		07 = Ausgabe Kanal 3 = Anwenderkanal 2
;	Bit 3	Init = Datei oeffnen
;	Bit 4	BIT8
;	Bit 5	Ruecksprung zu CAOS (BYE)
;	Bit 6	Close = Datei schliessen
;	Bit 7	Abfrage Tastaturstatus
;	HL	Zeiger, bei Init beginnt Dateiname 2 Byte danach
;	A	Datenbyte bei Ausgabe
; PA:	A	Datenbyte bei Eingabe

BASPV:	PUSH	HL
	PUSH	BC
	CALL	IRMON
	BIT	5,E
	JR	NZ,BASBYE	; BYE
	PUSH	DE		; E = Steuerbyte
	INC	HL
	INC	HL
	BIT	7,E
	JR	NZ,BSA2		; Tastaturstatus
	PUSH	HL
	LD	A,E		; Kanalnummer
	AND	7		; 0-3
	LD	HL,BUPTAB	; Tabelle der Unterprogramme
	ADD	A,L
	LD	L,A
	LD	A,D		; Parameter
	LD	D,E		; Steuerbyte
	LD	E,(HL)		; UP-Nummer
	POP	HL		; Dateiname
	CALL	PV3		; Aufruf (UP-Nr. in E)
	POP	DE		; E = Steuerbyte
	LD	D,A		; D = Ausgabeparameter
	LD	A,E		; Steuerbyte
	AND	4FH		; Kanalnummer und Close-Bit ausfiltern
	XOR	43H		; Close bei Kassettenausgabe?
	JR	NZ,BSA1
	CALL	DEV1		; DEVICE=Kassette?
	JR	NZ,BSA1		; VERIFY nur bei TAPE ausfuehren
	CALL	OSTR
	DB	'VERIFY ?(Y):',0
	CALL	KBD
	CALL	CSTBT		; anzeigen, was eingegeben wurde
	PUSH	AF
	CALL	CRLF		; auf neue Zeile wechseln
	POP	AF
	CP	'Y'
	CALL	Z,VERIF		; bei "Y" VERIFY ausfuehren
BSA1:	LD	A,D		; Ausgabeparameter
	RES	3,E		; Init ruecksetzen
	CALL	IRMOF
	POP	BC
	POP	HL
	RET

BSA2:	CALL	KBDS		; Tastaturabfrage
	POP	DE
	LD	D,A		; Zeichencode in D zurueck geben
	JR	NC,BSA1		; keine Taste gedrueckt
	RES	7,E		; Rueckmeldung, dass Taste gedrueckt wurde
	JR	BSA1

BASBYE:	LD	A,(DATFLG)	; geschuetzt?
	AND	A
	JR	Z,BASB1
	CALL	IRMOF
	CALL	NEW1		; NEW
	CALL	IRMON
BASB1:	LD	D,0
	CALL	MODUL2		; BASIC-ROM abschalten
	JP	LOOP

ZKOUT0:	CALL	OCHR
;_____________________________________________________________
;							**45**
ZKOUT:	LD	A,(HL)
	INC	HL
	AND	A
	JR	NZ,ZKOUT0
	RET
;_____________________________________________________________
;							**35**
; Tonausgabe
;
; (ARG1) => HL	L = Tonhoehe 1 (ZK fuer CTC 0, 0=kein Ton)
;		H = Vorteiler 1 (0, 1)
; (ARG2) => DE	E = Tonhoehe 2 (ZK fuer CTC 1, 0=kein Ton)
;		D = Vorteiler 2 (0, 1)
; (ARG3) => BC	C = Lautstaerke (0 ... 1FH)
;		B = Tondauer (0 ... FFH) (in 20 ms-Schritten)
;		    bzw. 0 = Dauerton)

TON:	CALL	LARG		; Argumente laden
TON1:	BIT	1,(IX+8)	; noch alter Ton?
	JR	NZ,TON1		; warten
TON2:	LD	A,C		; Lautstaerke
	AND	00011110b	; maskieren
	XOR	10011111b	; Komplement da nullaktiv
	LD	C,A
	LD	A,B
	AND	A		; Dauerton?
	DI
	JR	Z,TON3		; ja, keine CTC starten
	RES	7,C		; nein, Blinken aus
	SET	1,(IX+8)	; neuer Ton
	LD	A,11000111b	; EI, Zaehler 50Hz
	OUT	(CTC2),A	; CTC2
	LD	A,B		; Dauer
	OUT	(CTC2),A
TON3:	IN	A,(PIOBD)
	AND	01100000b	; RAM8 maskieren
	OR	C
	OUT	(PIOBD),A	; Lautstaerke ausgeben
	EI
	LD	C,CTC0
	CALL	TON4		; Tonkanal 1 starten
	INC	C		; CTC1
	EX	DE,HL		; Tonkanal 2 starten
TON4:	LD	A,L		; Zeitkonstante
	AND	A		; Ton?
	LD	L,3		; CTC Stop
	JR	Z,TON6		; kein Ton
	LD	L,A		; Zeitkonstante wieder in L
	LD	A,00111000b	; Steuerbyte "Zeitgeber" vor 3x RRA
	SRL	H		; Bit 0 (Vorteiler) nach CY
	RRA
	RRA			; Vorteiler nach Bit 5 rotieren
	RRA
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
	DI			; Befehl ab CAOS 4.3 zusaetzlich
	AND	(IX+1)
	XOR	C
OUT84:	LD	(IX+1),A
	OUT	(PORT1),A
	EI			; Befehl ab CAOS 4.3 zusaetzlich
	RET

ESC7:	LD	C,00000100b	; Invers-Bit
XORSTB:	LD	HL,STBT
	LD	A,(HL)
	XOR	C
	LD	(HL),A
	RET

ESC9:	DI			; Befehl ab CAOS 4.3 zusaetzlich
	LD	A,(IX+1)	; VR: A
	XOR	2
	JR	OUT84		; BC unveraendert

ESCA:	LD	HL,STBT
	RES	6,(HL)		; HRG aus
ESCAB:	LD	BC,0FF08h
	CALL	AXO84
	AND	C
	ADD	A,B		; CY = Bit 3
	DI			; Befehl ab CAOS 4.5 zusaetzlich
	IN	A,(PIOBD)
	RLA			; als Bit 0 einschieben
	RRCA			; auf Bit 7 rotieren
	OUT	(PIOBD),A
	EI			; Befehl ab CAOS 4.5 zusaetzlich
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
;_____________________________________________________________
;							**47**
V24OUT:	CALL	CON
	CALL	LSTC
COF1:	JP	COFF

; Reaktion auf ShCLR:

HCPGM:	PUSH	AF
	LD	A,(IX+4)	; CAOS-C merken
	PUSH	AF
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-C on
	CALL	HCPGMC
	POP	AF
	LD	(IX+4),A	; wie vorher
	OUT	(PORT2),A
	POP	AF
	RET

; Duplexroutine

V24D7F:	DW	7F7FH
	DB	'V24DUP'
	DB	1
;_____________________________________________________________
;							**48**
V24DUP:	CALL	CON		; CAOS-C on
	CALL	V24DC
	CALL	C,ERRM		; Fehler
	JR	COF1		; CAOS-C off

; Druckroutine mit Protokoll

ECHO:	CALL	CRT
PRINT:	BIT	7,(IX+4)
	JR	NZ,PR1		; CAOS-C on!
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
	;
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
	 OUT	(PORT2),A	; CAOS-C einschalten
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
	 OUT	(PORT2),A	; CAOS-C wie vor Interrupt
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
	OUT	(PORT2),A	; CAOS-C ausschalten
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
	OUT	(PORT2),A	; CAOS-C einschalten
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
	OUT	(PORT2),A	; CAOS-C ausschalten
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
	JR	JCON

; Datenbyte holen (fuer DISPC, MODIC, FSAVE, view)
LDAME:	CALL	COFF		; CAOS-C aus
	LD	A,(MODST+1)
	AND	1		; IRM aus?
	CALL	Z,SCROFF
	LD	A,(HL)		; Byte holen
JCON:	CALL	SCRON

; CAOS-ROM-C einschalten:
CON:	PUSH	AF
	SETIXA	7,4		; SET 7,(IX+4),A
	JR	CSW
;_____________________________________________________________
;							**31**
; CAOS-Arbeitsbereich verlagern
; PE:	A	HW-Teil der IX-Arbeitszellen

SIXD:	CALL	CON
	CALL	SIXC
;	JP	COFF		; reinlaufen...

; CAOS-ROM-C ausschalten falls erforderlich:
; VR:	-
;
COFF:	PUSH	AF
	LD	A,(MODST+5)	; Schaltzustand Moduladresse 5
	AND	1
	JR	NZ,CNSW		; bleibt ein!
	RESIXA	7,4		; RES 7,(IX+4),A
CSW:	OUT	(PORT2),A
CNSW:	POP	AF
	RET
;_____________________________________________________________
;							**41**
; Berechnung des Produktes zweier 8-Bit-Zahlen
; PE:	D, C	Faktoren (8 Bit)
; PA:	BA	Produkt (16 Bit)
; VR:	AF,HL,DE,B

MULT:	CALL	CON
	CALL	MULC		; BA=D*C
	JR	COFF
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
	CP	8
	JP	NC,ERRM		; n>8
	CALL	SETDEV		; Treiber einstellen
	JP	C,ERRM		; Treiber nicht aktiv
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
;_____________________________________________________________
;							**49**
; Geraetetreiber auswaehlen, abfragen oder anzeigen
; PE:	A 	0..7	- Auswahl Geraetetreiber Nr. 0-7
;		8	- aktuellen Treiber abfragen
;		9-FEH	- aktuellen Treibername anzeigen
;		FFH	- Auflisten aller Treiber
; PA:	CY=1	ausgew„hlter Treiber nicht aktiv
;	bei CY=0	HL = Zeiger auf Treibername
;			Z=1 - Kassettentreiber
; VR:	AF,BC,DE,HL

SETDEV:	CALL	CON
	CALL	SET_DD		; Routine im ROM C/D aufrufen
SETDC:	JP	COFF

; Verzeichnis anzeigen:

	DW	7F7FH
	DB	'DIR '
	DB	1FH		; Textargument zulaessig (Maske in DE)
	CALL	PV7
	DB	8
	RET

; Laufwerk wechseln:

	DW	7F7FH
	DB	'CD '		; ersetzt bisheriges Menuewort DRIVE
	DB	1FH		; Textargument zulaessig (Verzeichnis)
	CALL	PV7
	DB	9
	RET

; Datei loeschen:

	DW	7F7FH
	DB	'ERA '
	DB	1FH		; Textargument zulaessig (Dateiname)
	CALL	PV7
	DB	10
	RET

; Datei umbenennen:

	DW	7F7FH
	DB	'REN '
	DB	1FH		; Textargument zulaessig (2 Dateinamen)
	CALL	PV7
	DB	11
	RET
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
; Steuercodetabelle:
;
CRTTAB:	DW	NOOP	; 00
	DW	CLR	; 01
	DW	CLLN	; 02
	DW	NOOP	; 03=BRK
	DW	NOOP	; 04=Sh-BRK
	DW	ESC0	; 05=TAB
	DW	NOOP	; 06 (bei CAOS 3.3 invers)
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
	TOKEN	CSRLIN		; F4 bis hierher in CAOS 3.1 bis 4.4
	TOKEN	DEVICE		; F5 (DRIVE in CAOS 4.5, 4.6 und HCBASIC.COM)
	TOKEN	FILES		; F6
	TOKEN	CHDIR		; F7 bis hierher in CAOS 4.7
	DB	80H		; Ende-Kennung
;
; zugehoerige Sprungtabelle fuer BASIC-Token
;
TOKJP:	DW	INKEY
	DW	JOYST
	DW	STRING
	DW	INSTR
TADR5:	DW	RENUM
	DW	DELETE
	DW	PAUSE
	DW	BBEEP
	DW	WINDOW
	DW	ARGVL1		; BORDER
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
	DW	BDEV		; Speichergeraet einstellen
	DW	FILES		; Verzeichnis anzeigen
	DW	CHDIR		; Verzeichnis wechseln
TOKJPE:

	ABSFILL	0FDF0h,<ROM-F-Ende>

; High-Byte der Adresse muss konstant sein!
; je 1 Byte fuer Eingabe + Ausgabe pro Kanal

BUPTAB:	DB	16H, 24H	; INTB, OCHR	Kanel #0 - Konsole
	DB	37H, 38H	; MBIN, MBOUT	Kanal #1 - Kassette
	DB	06H, 02H	; USIN1, UOT1	Kanal #2 - Anwenderkanal 2
	DB	07H, 03H	; USIN2, UOT2	Kanal #3 - Anwenderkanal 3

; Adresse FDF8h fest - wird von anderen Programmen (z.B. TEMO) miz genutzt!

BITTAB:	DB	80H,40H,20H,10H,8,4,2,1
