;*****************************************
;**	CAOS 4.8	ROM F		**
;**					**
;**	Adresse:  F000h bis FDFFh	**
;**					**
;**	letzte Aenderung: 20.02.2023	**
;*****************************************

;	ORG	0F000H
				;			  feste Adresse
	JP	PWRON		; >> POWER ON <<		F000
	JP	PV1		; DEFB nr			F003
	JR	PV2		; (ARGC)=nr			F006
ZEI2:	DB	2	; UOUT1					F008
	JR	PV3		; E=nr				F009
ZEI3:	DB	3	; UOUT2					F00B
	JP	PV4		; E=nr, IRM e/a			F00C
	JP	RCALL		; DEFW offset			F00F
	JP	BYE		; JUMP-Einsprung		F012
	JR	PV5		; E=nr, IRM/Stack		F015
INTT1:	DB	4	; KBD					F017
	JP	IRMON		;				F018
	JP	IRMOF		;				F01B
	JR	PV6		; (ARGC)=nr			F01E
OUTT1:	DB	0	; CRT					F020
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
	DB	0EDH		; ED A7 = NOP (Trick aus CAOS 3.4 uebernommern)
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

;BNROST: LD	A,(IX+2)	; Blocknummer
AHOSTR:	CALL	AHEX		; Register A und folgende Zeichenkette
;_____________________________________________________________
;							**23**
; Ausgabe einer Zeichenkette
; Zeichektette hinter dem CALL, Ende mit 00h
; PE/PA: -
; VR:	AF

OSTR:	EX	(SP),HL
	CALL	ZKOUT		; Zeichenkette ausgeben
	EX	(SP),HL
	RET

; Module listen, lesen, schalten:

SWI7F:	DW	7F7FH
	DB	'SWITCH'
	DB	1
	AND	A		; Argumente?
	LD	BC,SWIC		; Module lesen/schalten
	JR	NZ,JROMC2	; im ROM-C ausfuehren

; Module auflisten
	CALL	ESC6		; interne Module listen
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
MCOFF:	JP	COFF

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

MODU:	LD	BC,MODUC
JROMC2:	JP	CLROMC		; im ROM-C ausfuehren

ESCD:	CALL	CON
	CALL	ESCDD		; DEVICE-Umschaltung
	JR	ESC61

ESCH:	CALL	CON
	CALL	HELP		; alle erreichbaren Menueworte auflisten
	LD	DE,(CURSO)	; aktuelle Cursorposition
	LD	E,0		; jedoch Spalte 0 setzen
	JR	MCOFF

; Betriebssystem wechseln:

JUMP7F:	DW	7F7FH
	DB	'JUMP'
	DB	3
	LD	A,L		; Steckplatz
;_____________________________________________________________
;							**27**
; Sprung in ein neues Betriebssystem
; PE:	A	Modulsteckplatz 8-FF
;		oder CAOS-ROM-Ebene 0-7

if SYSROM
;
; 13.06.2021:
; JUMP-Erweiterung: Wechsel der 8 CAOS-System-Ebenen
; JUMP 0 bis JUMP 7 springt in die verschiedenen Systeme
;
JUMP:	CP	8		; externes Modul ab Steckplatz 8?
	CALL	C,CON		; bei JUMP 0-7: ROM-C einschalten
	JP	C,JUMPC		; und weiter zu JUMP-Erweiterung im ROM-C
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
	LD	A,0FFH		; Ausgabe FF (Adresse C000)
	LD	L,B
	LD	(HL),A		; Eintrag
	DI
	OUT	(C),A		; einschalten
	IN	A,(PIOAD)
	AND	01111110b	; interne ROMs abschalten
	JP	NCAOS		; Sprung ueber IRM

; Einschalt-Initialisierung:

PWRON:	LD	A,0E3H		; RAM4 WR einschalten, CAOS-Bank 0
	OUT	(PORT2),A	; und CAOS-C ein
	JP	PWRONC		; Programm im ROM-C weiter abarbeiten ...
;
; Tasten-RESET und JUMP-Einsprung:
;
BYE:	LD	SP,STACK	; System-Stack setzen
	LD	A,0E3H		; RAM4 WR einschalten, CAOS-Bank 0
	OUT	(PORT2),A	; und CAOS-C ein
	CALL	SYSI		; Systeminitialisierung
PWR4:	CALL	V24INI		; M003 suchen und initialisieren
	CALL	COFF		; CAOS-C ausschalten (Standard)
	LD	BC,880H		; auf Steckplatz 8
	IN	A,(C)		; ROM-Modul mit Strukturbyte
	DEC	A		; 01 vorhanden?
	JR	NZ,PWR6		; nein!
	RES	6,(IX+8)	; Autostartmodul setzt INIT-Kommando zurueck
	LD	A,41H		; 43H bis CAOS 4.6, also mit Schreibfreigabe
	OUT	(C),A		; R/O einschalten auf Adresse 4000H
	LD	(MODST+8),A	; Schaltzustand eintragen
	LD	A,(IX+4)
	AND	0FCH		; internen RAM4 ausschalten
	CALL	OUT86		; (IX+4) und PORT2 einstellen
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
	JR	Z,PWR8		; 1=CP/M-Betriebsart erkannt
	CP	4
	JR	Z,PWR8		; 5=CAOS-Betriebsart erkannt
	CALL	OSTR
	DB	0CH,LF		; Ansonsten: Autostart mit JUMP FC
	DB	'Autostart Floppy'
	DB	CR,LF,0
	LD	B,0
PWR5:	CALL	BRKT		; BRK-Taste gedrueckt?
	JR	C,PWR8		; ja, dann kein Autostart
	DJNZ	PWR5		; 256-mal abfragen
	LD	A,1
	LD	(ARGN),A	; ein Parameter fuer JUMP FC vorgeben
	LD	A,0FCH
	CALL	JUMP		; Startversuch
;	JR	PWR8

; neu ab CAOS 4.8: Autostart INIT (z.B. bei USB)

PWR7:	CALL	DEV		; aktuelles Device in Bit 2-4
	INC	A		; Steckplatz
	LD	H,High(DevTab)
	LD	L,A
	LD	A,(HL)		; Steckplatz des aktuellen Device?
	CP	8		; externes Modul?
	JR	NC,PWR9		; ja
PWR8:	RES	6,(IX+8)	; Autostart (INIT) bei TAPE und DISK unterbinden
PWR9:	XOR	A
	LD	(ARGN),A	; MENU ohne Parameter ausfuehren!
	JR	MEN0
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Epilogverwaltung:		(bereits in CAOS 2.5 von Frank Klemm realisiert)
; -----------------		HEX/DEZ-Argumenta ab CAOS 4.8
; Epiloge 0 ... 1F
;
; Bit:  4      3      2      1      0
;	I      I      I	     I	    \
;  0 = hex     -------+-------	   IRM 
;  1 = dezimal   Anzahl der
;	         Argumente	  on/off
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
MEN0:	LD	BC,SHORT	; Kurzform CAOS-Titelzeile mit DEVICE anzeigen
	CALL	CLROMC		; im ROM-C ausfuehren
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
	JR	NZ,MEN1		; nein, E=0 verwenden
	LD	E,H		; E=ARG1
MEN1:	LD	HL,0C000H	; Menuewortsuche immer ab C000H
	LD	C,L
	LD	B,L		; BC=0 (Suchlaenge)
MEN2:	CALL	CRLF
MEN3:	LD	A,2		; CLLN
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
	JR	C,MEN8
	bit	0,e		; versteckte Menueworte anzeigen?
	jr	z,MEN6		; nein
	CP	'z'+1		; <= 'z'?
	JR	NC,MEN3		; groesser als 'z'
	jr	MEN7
	;
MEN6:	CP	'0'		; >= '0' und
	JR	C,MEN3		; kleiner als '0'
	CP	'Z'+1		; <= 'Z'?
	JR	NC,MEN3		; groesser als 'Z'
	CP	'A'		; >= 'A' oder
	JR	NC,MEN7		; groesser als 'A'
	CP	':'+1		; <= ':'?
	JR	NC,MEN3		; groesser als ':'
MEN7:	CALL	OCHR
	CPI			; INC HL, DEC BC
	JP	PE,MEN5		; BC>0, also weiter
	LD	A,2		; CLLN
	JR	LOOP1		; ausgesucht
	;
MEN8:	;LD	A,(CURSO)	; aktuelle Position
	;DEC	A		; leere Menuzeile?
	;JR	Z,MEN3		; diese unterdruecken
	bit	1,e
	jr	z,MEN2		; keine Adressen anzeigen
	ld	a,(winlg)
	cp	15		; Fensterbreite fuer Adressen ausreichend?
	jr	c,MEN2		; nein, dann nicht anzeigen!
	ld	a,11
	ld	(curso),a	; Spalte fuer Adressanzeige
	inc	hl
	call	hlhx		; Startadresse anzeigen
	dec	hl
	JR	MEN2
;
; 12.12.2020: Der Schutz von BASIC-Programmen wird ab CAOS 4.8 nicht mehr
; unterstuetzt. Das heisst, das Programm wird bei BYE nicht mehr geloescht.
;
BASBYE:
;	LD	A,(DATFLG)	; geschuetzt?
;	AND	A
;	JR	Z,BASB1
;	CALL	IRMOF
;	CALL	NEW1		; NEW
;	CALL	IRMON
;BASB1:
	LD	D,0
	CALL	MODUL2		; BASIC-ROM abschalten
;	JP	LOOP
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
CRT02:	LD	A,2		; und
	JP	CRT		; Zeile loeschen
	;
INPT1:	INC	DE		; Zeichen nach Prompt
	LD	A,(DE)
	AND	0DFH		; leer?
	RET	Z
	LD	A,(MODST+2)	; Schaltzustand USER-ROM
	LD	(ARGC),A	; CAOS 4.6: hier uebergeben an USER-Programme
	 ld	b,0		; gesamten Speicher durchsuchen
	 call	ms1		; ab C000H
	JR	C,INPT5		; gefunden
	 ld	hl,modst+5
	 bit	0,(hl)
	JR	NZ,INPT2	; CAOS-C war bereits an
	CALL	CON
	 call	msuch		; durchsuchen im CAOS-ROM-C
	JR	C,INPT5		; gefunden
INPT2:	PUSH	DE		; Suchkette
	LD	D,0D1H		; 1. Ebene (BASIC nicht mit durchsuchen)
INPT3:	CALL	MODUL2		; USER-ROM-C Ebene ein
	RESIXA	7,4		; RES 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-ROM-C aus!
	EX	DE,HL
	EX	(SP),HL		; DE=Suchkette, (SP)=SWITCH-Ebene
	EX	DE,HL
	 call	msuch		; Menuewort in USER-ROM suchen
	 ex	(sp),hl		; (SP)=Ende+1 der gefundenen Kette
	JR	C,INPT4		; gefunden
	LD	B,10H		; Offset zur naechsten Ebene: C1, D1, E1, F1
	 add	hl,bc		; naechste Ebene berechnen (C ist 0)
	 ex	de,hl		; DE=Schaltzustand
	 ex	(sp),hl		; (SP)=Suchkette		
	JR	NC,INPT3	; naechste Ebene
	LD	DE,(ARGC-1)	; CAOS 4.6: D = USERC-Schaltzustand vor Suche
	CALL	MODUL2		; USER-ROMC wie zuvor
	JR	ERR1		; invalid command
	;
INPT4:	 ld	hl,modst+5
	 ld	(hl),0		; CAOS-C ausschalten
	 pop	hl
INPT5:	INC	HL		; Epilogbyte uebergehen
	PUSH	HL		; Startadresse
	CP	1FH		; Textparameter?
	RET	Z		; ohne arg zu erfassen!
	PUSH	AF		; Epilogbyte
	LD	C,16		; Zahlenbasis hexadezimal
	AND	10h		; Dezimal-Argumente?
	JR	Z,INPT6
	LD	C,10		; Zahlenbasis dezimal
INPT6:	CALL	GARGC		; bis zu 10 Argumente erfassen
	POP	BC		; B=Epilogbyte
	JR	C,ERR4		; bad args
	LD	A,(ARGN)	; Anzahl Argumente
	LD	C,B
	RES	4,B		; Bit fuer Zahlenbasis ausblenden
	SRA	B
	CP	B
	JR	C,ERR3		; zu wenig arg
	BIT	0,C
	CALL	LARG		; Arg's laden
	RET	NZ		; mit IRM on
	JP	SCROFF		; mit IRM off

ERR1:	CALL	OSTR
	DB	'Kommando?',7,0
	JR	ERRCR
	;
ERR3:	CALL	OSTR
	DB	'Zu wenig',0
	JR	ERRA
	;
ERR4:	CALL	OSTR
	DB	'Fehlerhafte',0
ERRA:	CALL	OSTR
	DB	' Argumente',7,0
ERRCR:	 pop	hl		; Stack clear
	JP	CRLF

MSUCH:	 ld	b,20h		; 2000h Bytes
MS1:	LD	HL,0C000H	; Standard-Suchbeginn
ZS0:	LD	C,L		; C=0
	LD	A,(IX+9)	; aktuelles Prologbyte
;_____________________________________________________________
;							**1D**
; Zeichenkette/Menuewort suchen
; PE:	A	Prologbyte (bei CAOS: 7FH)
;	BC	Laege des Suchbereiches
;	DE	Anfang der Vergleichskette (Ende mit 0-20h)
;	HL	Anfang des Suchbereichs
; PA:	DE	Ende+1 Vergleichskette (wenn gefunden)
;	HL	Ende+1 gefundene Kette
;	CY=1	Kette gefunden
 ;	 dann  A=Epilogbyte 
 ;	       BC=Anfang der Vergleichskette
 ;	 sonst A=Prologbyte
 ;	       BC=0
 ;	       DE=Anfang der Vergleichskette
; VR:	AF,BC,DE,HL

ZSUCH:	 and	a		; CY=0
	CPIR			; (CY wird nicht veraendert)
	RET	PO		; Prolog nicht gefunden
	CPI
	JR	NZ,ZSUCH	; nur 1x gefunden
	 push	de		; Beginn der Vergleichskette retten
	 push	af		; Prologbyte
ZS1:	LD	A,(DE)		; Zeichen von Vergleichskette
	INC	DE
	CP	21H
	JR	C,ZS3		; Kettenende bei Vergleichskette erkannt
	BIT	5,(HL)		; Grossbuchstabe im Speicher?
	CALL	Z,UPCASE	; dann Zeichen von Vergleichskette upcasen
	CPI
	JR	Z,ZS1		; stimmt ueberein
ZS2:	 pop	af		; verwendetes Prologbyte restaurieren
	 pop	de		; und wieder zu Beginn der Vergleichskette
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
ZS4:	 pop	bc
	 pop	bc		; Anfang der Vergleichskette
	 ret			; CY ist 1 vom CP 20H
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
	JR	NC,OCHR
	LD	A,'%'		; Standard
	JR	OCHR		; Prompt
;_____________________________________________________________
;							**1B**
; Zwei 16-Bit-Werte hexadezimal anzeigen und
; ein Leerzeichen danach
; VR:	AF
HLDE:	CALL	HLDE1		; 1x aufrufen, dann reinlaufen
HLDE1:	CALL	HLHX		; HL anzeigen
	EX	DE,HL		; HL <-> DE tauschen
	RET
;_____________________________________________________________
;							**1A**
; 16-Bit-Wert hexadezimal anzeigen und ein Leerzeichen danach
; VR:	AF
HLHX:	LD	A,H
	CALL	AHEX
ALSPC:	LD	A,L
AHSPC:	CALL	AHEX
;_____________________________________________________________
;							**2B**
; Leerzeichen anzeigen
; VR:	AF
SPACE:	LD	A,' '		; Leerzeichen
	JR	OCHR
;_____________________________________________________________
;							**2D**
; Cursor in Home-Position
; VR:	AF
HOME:	LD	A,10H		; Home-Code
	JR	OCHR
;_____________________________________________________________
;							**1C**
; 8-Bit-Wert hexadezimal anzeigen
; VR:	AF
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
; VR:	AF
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
POP31:	JP	POP3		; auch fuer PV2 bis PV6!
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
	JR	C,POP31
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
	BIT	4,(HL)		; ESC aktiv?
	JP	Z,POP31
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
; Syntax: %go ADR [HL [DE [BC [A] ] ] ]
;
GO7F:	DW	7F7FH
	DB	'GO '
	DB	3		; mindestens 1 Argument erforderlich
	XOR	A
	LD	(ARGN),A
; alter Code, 4 Byte lang:
;	PUSH	HL		; Adresse auf den Stack
;	EX	DE,HL		; ARG2 von DE nach HL
;	LD	D,B
;	LD	E,C		; ARG3 von BC nach DE
; 28.10.2022: neuer Code von ZM.ASM uebernommen, 3 Byte lang
; BC(Arg3) => DE(Arg2) => HL(Arg1) => Stack
	PUSH	BC		; (SP)=(ARG3)
	EX	(SP),HL		; Stack = (ARG1), HL=(ARG3)
	EX	DE,HL		; DE = (ARG3), HL=(ARG2)
	LD	BC,(ARG4)	; BC = (ARG4)
	LD	A,(ARG5)	; A = (ARG5)
	RET			; JMP (ARG1)
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
;							**18**
; Erfassung einer eingegebenen Hex-Zahl
; PE:	DE	Zeichenkette
; PA:	DE	Ende der Zeichenkette
;	(NUMNX)	Laenge der Zeichenkette
;	(NUMVX)	umgewandelte Zahl
;	HL=NUMNX
;	CY=1	Fehler
; VR:	AF, DE, HL
;
RHEX:	PUSH	BC		; RHEX darf BC nicht veraendern
	LD	C,16		; Hexadezimalzahlen
	CALL	RZAHL
	POP	BC
	RET
;_____________________________________________________________
;							**4B**
; Erfassung einer eingegebenen Dezimal-Zahl
; PE:	DE	Zeichenkette
; PA:	DE	Ende der Zeichenkette
;	(NUMNX)	Laenge der Zeichenkette
;	(NUMVX)	umgewandelte Zahl
;	HL=NUMNX
;	CY=1	Fehler
; VR:	AF, C, DE, HL
;
RDEZ:	LD	C,10		; Dezimalzahlen
	DEC	DE	; (anstelle von JR RZAHL)

; Erfassung einer eingegebenen Dez- oder Hex-Zahl
; PE:	DE	Adresse Zeichenkette (im VRAM)
;	C	Zahlenbasis (2..32)
; PA:	(NUMNX)	Anzahl der HEX-Zeichen (0-4)
;	(NUMVX)	Wert der erfassten Zahl
;	HL=NUMNX
;	CY=1	Fehler
; VR:	AF, C, DE, HL
;
RHX0:	INC	DE
RZAHL:	LD	A,(DE)
	CP	' '		; Trennzeichen
	JR	Z,RHX0		; uebergehen
	XOR	A
	LD	HL,NUMVX+1
	LD	(HL),A
	DEC	HL		; HL=NUMVX
	LD	(HL),A
	DEC	HL		; HL=NUMNX
	LD	(HL),A		; alles auf 0 setzen
; Erfassen der ASCII-Zeichen
RHX1:	LD	A,(DE)
	AND	0DFH		; CY=0
	RET	Z		; Ende-Dummy oder Space
	LD	A,(DE)		;	       aus: 3A..40 41..60 61..80 81..FF
	SUB	'0'		;	      wird: 0A..10 11..30 31..50 51..CF
	RET	C		; kleiner als '0'
	CP	10
	JR	C,RHX2		; 0..9
	SUB	17		; HEX-Korrektur	    F9..FF 00..1F 20..3F 40..BE
	AND	11011111b	; klein -> gross    D9..DF 00..1F 00..1F 40..9E
	ADD	10		; kleiner als 'A'   E3..E9 0A..29 0A..29 4A..A8
RHX2:	CP	C		; > Zahlenbasis?
	CCF
	RET	C
	INC	(HL)		; NUMNX erhoehen
RHX3:	INC	HL
	PUSH	DE
	LD	D,(HL)	
	CALL	MULA		;(NUMVX)*Basis + Ziffer
	POP	DE
	LD	(HL),A
	LD	A,B		; naechste Stelle
	BIT	0,L		; HL=NUMVX ?
	JR	NZ,RHX3		; wiederholen
	DEC	HL
	DEC	HL		; HL=NUMNX
	NEG			; CY=1 wenn A<>0
	RET	C		; Fehler: Zahl zu gross
	INC	DE		; Zeiger auf naechstes Zeichen
	JR	RHX1
;_____________________________________________________________
;							**22**
; Bis zu 10 hexadezimale Argumente erfassen
; PE:	DE	Adresse im VRAM
; PA:	(ARGN) und (ARG1) bis (ARG10)
;	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
GARG:	LD	C,16		; Zahlenbasis = hexadezimal
;_____________________________________________________________
;							**4C**
; Bis zu 10 Argumente erfassen, beliebige Zahlenbasis
; PE:	C	Zahlenbasis 10 oder 16
;	DE	Adresse im VRAM
; PA:	(ARGN) und (ARG1) bis (ARG10)
;	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
GARGC:	XOR	A
	LD	(ARGN),A	; (ARGN)=0
GARG1:	CALL	RZAHL		; Argument erfassen
	RET	C		; Fehler
	LD	A,(HL)		; NUMNX lesen
	OR	A		; Ende erreicht?
	RET	Z		; fertig
	LD	A,(ARGN)
	INC	A		; Argumente mitzaehlen
	CP	11		; 10 Argumente?
	CCF
	RET	C		; zu viele Argumente
	LD	(ARGN),A
	ADD	A,A		; ARGN*2
	OR	LOW(ARGC)
	PUSH	DE
	LD	E,A
	LD	D,HIGH(ARGN)
	INC	HL
	LDI			; Low(ARGx)
	LDI			; High(Argx)
	INC	BC
	INC	BC		; BC korrigieren
	POP	DE
	JR	GARG1

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
; PA:	D=0
; VR:	D, F, HL
;
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
;
; PE:   HL	Dateiname
; PA:   CY=1    Fehler oder BRK
; VR:	AF, BC, DE, HL
;
SAVE:	CALL	CLC		; Kassettenpuffer loeschen
;	LD	DE,CASS		; DE wird von CLC uebergeben
	PUSH	HL		; Dateiname
	LD	BC,12		; 15.06.2019: von 11 auf 12 geaendert
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
	RET	C		; Fehler, bereits beim initialisieren
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
	DB	01H	; LD BC,nn	ueberspringt LDIR = ED B0
;	JR	SAV4
	;
SAV3:	LDIR			; kopieren
SAV4:	LD	(ARG1),HL	; Adresse fuer naechsten Block eintragen
	pop	hl		; Anzahl
	CALL	SUP5		; Blocknummer anzeigen
	LD	BC,160		; kurzer Vorton
	JR	C,SAV5		; BRK -> Datei schliessen
	ld	de,128		; Blocklaenge
	sbc	hl,de		; Ende erreicht?
	jr	z,SAV5		; ja, genau das Blockende
	CCF
	jr	NC,SAV5		; ja, letzter Block mit weniger Bytes
	push	hl		; restliche Anzahl
	CALL	PV7		; Datenblock ausgeben
	DB	0		; MBO
	JR	NC,SAV1		; kein Fehler
	pop	hl		; restliche Byte-Anzahl vom Stack nehmen
	;
SAV5:   PUSH    AF              ; Fehlerstatus merken
	CALL	PV7		; Ausgabe Endeblock 0FFH
	DB	3		; CSRO
	CALL	CLC		; CAOS 4.6: Puffer loeschen nicht mehr in CSRO
	CALL	CRLF		; CAOS 4.6: Zeilenvorschub nicht mehr in CSRO
	POP     AF              ; CY=1 bei Fehler
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
; PA:	CY=1	BRK gedrueckt nach Lesefehler oder Fehler bei Device nicht TAPE
; VR:	AF,BC	(Block<>1) ( 00>  )
;	AF,BC,HL (Block=1) ( Name )

RDBLK:	INC	(IX+3)		; erwarteter Block
LUP1:	CALL	PV7
	DB	1		; MBI
LUP2:	JR	NC,LUP3		; kein Fehler aufgetreten

	CALL	DEV		; Kassette?
	SCF			; Fehlerflag wieder setzen
	RET	NZ		; Abbruch bei Fehler wenn kein TAPE

; bei Kassette nach Wiederholung fragen:
	CALL	BLERR		; fehlerhaften Block mit ? anzeigen
	CALL	CRLF		; neue Zeile
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
	CP	B		; richtige Blocknummer?
	JR	Z,LUP4		; ja
	INC	A		; Block FF?
	JR	Z,LUP5		; Endeblock immer als richtig werten
	CALL	BLNOT		; falsche Blocknummer mit * anzeigen
	JR	LUP1		; weiter versuchen
	;
SUP5:	XOR	A		; nichts anzeigen nach der Blocknummer
	JR	AHSTR
	;
BLNOT:	LD	A,'*'		; falsche Blocknummer mit * anzeigen
	JR	AHSTR
	;
BLERR:	CALL	OSTR		; fehlerhaften Block mit ? anzeigen		
	DB	9,9,9,' ',0	; hinter der letzten korrekten Blocknummer
	LD	A,'?'		; Pruefsumme fehlerhaft
	JR	AHSTR

LUP4:	DEC	A		; Block 01?
	JR	Z,LUP6		; Dateiname anzeigen
LUP5:	CALL	CRT02		; falls eine fehlerhafte Zeile dabei war
;SUP5:				; Zeile komplett loeschen mit CLLN
	LD	A,'>'		; korrekte Blocknummer
AHSTR:	PUSH	AF
	LD	A,(IX+2)	; Blocknummer
	CALL	AHEX		; hexadezimal anzeigen
	POP	AF
	CALL	OCHR		; Kennung (>, *, ?) anhaengen
	LD	A,19H
	CALL	CRT		; zurueck zum Zeilenanfang
;	JP	BRKT
;_____________________________________________________________
;							**2A**
; Test auf BRK-Anforderung

BRKT:	CALL	KBDS		; Tastaturabfrage
	RET	NC		; keine Taste gedrueckt
	CP	3		; war das die BRK-Taste?
	SCF			; CY=1
	RET	Z		; ja, BRK erkannt
	AND	A		; CY=0
	RET

; Dateiname aus Vorblock anzeigen (fuer TAPELIST und LOAD)
; 15.06.2019: Anzahl von 11 auf 12 geaendert wegen 8.3 im Vorblock
; 02.04.2021: wieder 11 (wegen TAPELIST/BASIC)
; 15.05.2021: 12 bei LOAD und 11 bei TAPELIST als Kompromiss
; 11.09.2021: Register HL wird nicht mehr veraendert
LUP6:	LD	B,12		; Kontrolle/Anzeige 11 gueltige Zeichen
TLNAME:	LD	C,B
	PUSH	HL
	LD	HL,CASS		; Dateiname in Kassettenpuffer
LUP7:	LD	A,(HL)		; auf gueltige Zeichen testen
	OR	A
	JR	Z,LUP8		; 00h ist OK
	CP	20h
	JR	C,LUP10		; <20h also raus
	CP	7Fh
	JR	C,LUP8		; 20h..7Eh ist OK
	CP	0D3h
	JR	C,LUP10		; 7Fh..D2h raus
	CP	0DAh		; D3..D5 sind BASIC-Dateitypen,
				; D7..D9 sind geschuetzte BASIC-Dateitypen
	JR	NC,LUP10	; >D9h also raus
LUP8:	INC	L
	DJNZ	LUP7
	LD	L,B		; L=0 wieder zurueck auf Anfang Kassettenpuffer
	LD	B,C		; anzuzeigende Anzahl
LUP9:	LD	A,(HL)		; aus Kassettenpuffer anzeigen
	AND	7Fh		; hohe Zeichen fuer BASIC-Dateiname
	INC	HL
;	OR	A		; CY=0 (kein Fehler)
	JR	NZ,LUP11
	LD	A,' '		; statt 00 Leerzeichen anzeigen
LUP11:	CALL	OCHR		; Dateiname anzeigen
	DJNZ	LUP9
	CALL	SPACE		; Leerzeichen zwischen Dateiname + Adressen
LUP10:	POP	HL
	AND	A		; kein Fehler
	RET			; (CRT und damit OCHR/SPACE veraendern AF nicht)

NOMC:	CALL	OSTR
	DB	'Keine MC-Datei!',CR,LF,0
CLJP:	CALL	CSRI		; Datei schliessen
	SCF			; Fehlerflag setzen
	RET

VERI7F:	DW	7F7FH
	DB	'VERIFY',1
;_____________________________________________________________
;							**11**
; Kassettendatei kontrollieren

VERIF:	CALL	DEV		; Kassette?
	RET	NZ		; nur bei TAPE sinnvoll
	LD	(IX+7),A	; A=0
	JR	LOAD1

LOAD7F:	DW	7F7FH
	DB	'LOAD',1
	CALL	DEV		; Kassette?
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
	CALL	DEV		; Kassette?
	SCF			; Fehlerflag setzen
	RET	NZ		; Abbruch wenn nicht Kassette
LOAD0:	CALL	LUP2		; Blockinhalt auswerten (Name anzeigen usw.)
LOAD2:	JR	C,CLJP		; BRK -> CSRI -> Fehler
	BIT	0,(IX+7)
	JR	Z,LOAD5		; Verify
	LD	A,(CASS+10H)	; Anzahl der Argumente beim Abspeichern
	SUB	2		; 0 oder 1 Argument ist falsch
	CP	9		; MC?
	JR	NC,NOMC		; Fehler: kein gueltiger CAOS-Vorblock
;
; 11.09.2021: ARGN aus Vorblock wird nun um 2 verringert in den Bits 2-4 von
; (IX+7) abgelegt. Das spart das doppelte Laden von (CASS+10H) und vereinfacht
; den Test auf Autostart nach dem Einlesen der Datei
;
	RLCA			; Anzahl ARG-2
	RLCA			; nach Bits 2..4 rotieren
;
; 12.09.2021: Da (IX+7) bei LOAD mit 1 und bei VERIFY mit 0 initialisiert wird,
; muessen die Bits nicht maskiert werden und die Bits 2..4 koennen direkt
; gesetzt werden mittels OR
;
	OR	(IX+7)
;	AND	00011100b	; Bits 0..1 und 5..7 zuruecksetzen
;	XOR	(IX+7)		; a = (a XOR b) XOR b fuer Bits 2..4
	LD	(IX+7),A	; in IX+7 merken fuer spaetere Auswertung
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
	OR	A		; CY=0, damit auch EADR<AADR funktioniert!
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
	AND	3		; Bits 0+1 testen
	DEC	A
	RET	NZ		; kein Start oder Verify
	LD	A,(IX+7)
	AND	00011100b	; Bits 2..4 testen (ARGN-2 in Vorblock)
	LD	(IX+7),0	; Befehl fehlte in CAOS 4.3!
	RET	Z		; kein Fehler, kein Autostart bei ARGN=2
	LD	HL,(ARG3)	; Startadresse
	JP	(HL)		; anspringen

	DW	7F7FH
	DB	'COLOR'
	DB	11H		; Argumente dezimal ab CAOS 4.8
	CALL	CON		; CAOS ROM-C ein
	JP	COLRC		; dort abarbeiten
;_____________________________________________________________
;							**0F**
COLR:	CALL	CON
	JP	SETCO

	DW	7F7FH
	DB	'DISPLAY'
	DB	3
;_____________________________________________________________
;							**3B**
DISP:	CALL	CON		; CLROMC hier nicht nutzbar (wegen BC)
	CALL	DISPC
	JR	JCOFF

	DW	7F7FH
	DB	'MODIFY'
	DB	3
;_____________________________________________________________
;							**2E**
MODI:	LD	BC,MODIC
	JR	JROMC		; im ROM-C ausfuehren
;
;***************************************************************
; Fensterverwaltung
; %WINDOW		aktiviert Fenster 0 in voller Groesse
; %WINDOW nr		aktiviert Fenster nr
; %WINDOW aa bb cc dd e	Fenster definieren
;         |  |  |  |  |
;         |  |  |  |  |_ Fenster-Nr. in (ARG5)
;         |  |  |  |____ Spaltenzahl in (ARG4)
;         |  |  |_______ Spaltenanfang in Register C
;         |  |__________ Zeilenanzahl in Register E
;         |_____________ Zeilenanfang in Register L
;
WIND7F:	DW	7F7FH
	DB	'WINDOW'
	DB	11H		; Argumente dezimal!
	CALL	CON		; CLROMC hier nicht nutzbar (wegen BC)
	CALL	WINDC
JCOFF:	JP	COFF
;_____________________________________________________________
;							**3C**
; PE:	A	Fensternummer 0..9
;	HL	Fensteranfang
;	DE	Fenstrergroesse
; PA:	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
WININ:	LD	BC,WININC
	JR	JROMC		; im ROM-C ausfuehren
;_____________________________________________________________
;							**3D**
; PE:	A	Fensternummer 0..9
; PA:	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
WINAK:	LD	BC,WINAKC
JROMC:	JP	CLROMC		; im ROM-C ausfuehren
;_____________________________________________________________
;
; F-Taste belegen bzw. Belegung auflisten
;
	DW	7F7FH
	DB	'KEY'
	DB	1		; Parameter hexadezimal
	CP	1		; Parameter angegeben?
	JR	NC,KEYL		; ja, F-Taste bearbeiten
;_____________________________________________________________
;							**3A**
KEYLI:	LD	BC,KEYLIC	; Keylist
	JR	JROMC		; im ROM-C ausfuehren

KEYL:	LD	A,L		; Nummer
;_____________________________________________________________
;							**39**
KEY:	LD	BC,KEYC		; Taste belegen
	JR	JROMC		; im ROM-C ausfuehren
;_____________________________________________________________
;							**25**
; Cursor komplementieren
;
CUCP:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
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
	XOR	0FEH		; Strichcursor 7 Pixel breit
	LD	(HL),A
	JR	CUCP3
	;
CUCP1:	LD	DE,(CUMUST)	; Muster
	LD	B,8		; Zeichenhoehe
CUCP2:	LD	A,(DE)
	XOR	(HL)		; verknuepfen
	LD	(HL),A
	INC	DE
	INC	L
	DJNZ	CUCP2		; 8-mal
CUCP3:	JP	POP4
;_____________________________________________________________
;							**30**
; Punkt setzen
; PE:	(HOR)	Horizontalkoordinate
;	(VERT)	Vertikalkoordinate
;	(FARB)	Bildpunktfarbe
; PA:	CY=1	Punkt auáerhalb (Fehler)
; VR:	AF
;
PUSE:	CALL	CON		; CLROMC hier nicht nutzbar (wegen BC)
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
PUDE:	CALL	CON		; CLROMC hier nicht nutzbar (wegen BC)
	CALL	PUDEC		; Punkt loeschen
COF:	JP	COFF
;_____________________________________________________________
;							**3F**
; Kreis zeichnen
; PE:	(ARG1)	X-Koordinate Mittelpunkt
;	(ARG2)	Y-Koordinate Mittelpunkt
;	(ARG3)	Radius
;	(FARB)	Farbe des Kreises
; PA:	-
; VR:	AF, BC, DE, HL, BC', DE', HL'

CIRCL:	LD	BC,CIRCLC	; Kreis
	JR	JROMC		; im ROM-C ausfuehren
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

LINE:	LD	BC,LINEC	; Linie
	JR	JROMC
;_____________________________________________________________
;							**37**
; Byteweise Eingabe:
; PE:	D	Steuerbyte
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
;	Bit 5,(IX+7) Blocknummernanzeige unterdruecken?
; PA:	A	Datenbyte (nicht bei Close)
;	CY=1	Fehler
; VR:	AF, DE, HL
;
MBIN:	CALL	CON		; CAOS-ROM C ein
	CALL	MBIC		; Byte einlesen
	JR	COF		; CAOS-ROM C aus
;_____________________________________________________________
;							**38**
; Byteweise Ausgabe:
; PE:	A	Datenbyte
;	D	Steuerbyte
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
;	Bit 5,(IX+7) Blocknummernanzeige unterdruecken?
; PA:	CY=1	Fehler
; VR:	AF, DE, HL
;
MBOUT:	CALL	CON		; CAOS-ROM C ein
	CALL	MBOC		; Byte ausgeben
	JR	COF		; CAOS-ROM C aus

; aktuelles Menuewort wieder loeschen (fuer HELP-Funktion)

CLR12:	LD	D,12		; maximal 12 Zeichen
CLR12a:	LD	A,1FH		; DEL
	CALL	OCHR		; loeschen
	DEC	D
	JR	NZ,CLR12a
	RET

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
	JP	NZ,BASBYE	; BYE
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
	JR	NC,BSA0		; kein Fehler
	LD	A,E		; Steuerbyte
	AND	6		; Kanalgruppe ausfiltern
	CP	2		; MBIN/MBOUT?
	JR	NZ,BSA0		; bei allen anderen Kanaelen CY uninteressant

; Sprung zu BASIC: ?IO ERROR
BD2:	CALL	IRMOF		; IRM aus
	JP	IOERR		; Sprung zu Fehler-Routine in den BASIC-ROM

BSA0:	LD	A,E		; Steuerbyte
	AND	4FH		; Kanalnummer und Close-Bit ausfiltern
	XOR	43H		; Close bei Kassettenausgabe?
	JR	NZ,BSA1
	CALL	DEV		; DEVICE=Kassette?
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
; (ARG3) => BC	C = Lautstaerke in 2er-Schritten (0 ... 1FH)
;		B = Tondauer in 20ms-Schritten (0 ... FFH)
;		    bzw. 0 = Dauerton)

TON:	CALL	LARG		; Argumente laden
TON1:	BIT	1,(IX+8)	; noch alter Ton?
	JR	NZ,TON1		; warten
; hier Einsprung von Beep und Click:
TON2:	LD	A,C		; Lautstaerke
	AND	00011110b	; maskieren (Bit 1-4)
	XOR	10011111b	; Komplement da nullaktiv
	LD	C,A
	LD	A,B		; Tondauer
	AND	A		; 0=Dauerton?
	DI
	JR	Z,TON3		; ja, keine CTC2 starten
	RES	7,C		; nein, Blinken aus
	SET	1,(IX+8)	; neuer Ton
	LD	A,11000111b	; EI, Zaehler 50Hz
	OUT	(CTC2),A	; CTC2
	LD	A,B		; Dauer
	OUT	(CTC2),A
TON3:	IN	A,(PIOBD)
	AND	01100000b	; RAM8 maskieren
	OR	C		; Lautstaerke einbauen
	OUT	(PIOBD),A	; Lautstaerke ausgeben
	EI

; 01.05.2021 (nach Idee von Frank Ludwig)
; Kanal 1 wird mit 90 Grad Phasenverschiebung programmiert, damit die
; Tonausgabe bei gleichen Parametern fuer Kanal 0 und 1 besser klingt

	LD	BC,0300h+CTC0	; B=Steuerbyte "Kanal Reset"
	LD	A,L		; Tonhoehe 1
  	CALL	TON4		; Tonkanal 1 starten (CY ist 0)
	INC	C		; CTC1
	EX	DE,HL		; Tonkanal 2 starten
	LD	A,L		; Tonhoehe 2
	RRA			; halbe ZK fuer 90Grad Phasenverschiebung
TON4:	OUT	(C),B		; CTC-Kanal anhalten
	ADC	0		; +CY um ZK=1 zu beruecksichtigen
	RET	Z		; kein Ton (ZK=0)
	SRL	H		; Bit 0 (Vorteiler) nach CY
	LD	H,A		; ZK fuer Phasenverschiebung nach H
	LD	A,00101000b	; Steuerbyte "Zeitgeber" vor 3x RRA
	RRA			;   (ohne Kanal Reset)
	RRA			; Vorteiler nach Bit 5 rotieren
	RRA
	DI
	OUT	(C),A		; CTC als Zeitgeber starten
	OUT	(C),H		; bei CTC1 ZK fuer Phasenverschiebung ausgeben		
	OUT	(C),A		; CTC nochmals mit echter ZK programmieren
	EI			; Interrupts wieder ein nach letztem OUT
	OUT	(C),L		; jetzt volle ZK nach ersten Nulldurchgang
	RET

; ESC-Funktionen:

ESCPRG:	LD	HL,STBT		; ShSTOP
	SET	4,(HL)		; Kennung, dass ESC-Funktion folgt
	RET

ESC1:	LD	BC,VRAM0+00H	; 0B200H
	JR	ESCVR

ESC2:	LD	BC,VRAM1+05H	; 0AD05H
	JR	ESCVR

ESC3:	LD	BC,VRAM1+04H	; 0AD04H
	JR	ESCVR

ESC4:	LD	BC,VRAM0+01H	; 0B201H
ESCVR:	LD	H,B
	LD	L,0
	LD	(VRAM),HL

; 08.08.2020: Beim Bildwechsel die Fenster- und Cursorposition merken
;	( Idee von Tim Hennig am 25.07.2020 )

	LD	A,(STBT)	; Bit 7 gesetzt, wenn 2 Monitore
	RLCA			; Bit 7 -> CY
	JR	NC,NOBW		; nein, 2 Bilder (KC85/4-Modus)

	LD	A,(IX+1)	; Istzustand
	XOR	C		; Sollzustand
	BIT	2,A		; Wechsel Bild-Zugriff?
	JR	Z,NOBW		; nein
	LD	HL,WINNR	; B79B
	LD	DE,WIN2		; Tauschbereich	
	LD	B,CCTL0-WINNR	; 11 Byte (Fensternummer + Fenstervektor)
SWAP:	LD	A,(DE)
	LDI
	INC	BC		; BC korrigieren
	DEC	HL
	LD	(HL),A
	INC	HL
	DJNZ	SWAP
	LD	DE,(CURSO)	; gespeicherte Cursor-Position

NOBW:	LD	B,0FAH		; Maske
AXO84:	LD	A,B		; AND/XOR/OUT84
;	DI			; Befehl von CAOS 4.3 bis 4.7 zusaetzlich
	AND	(IX+1)
	XOR	C
OUT84:	LD	(IX+1),A
	OUT	(PORT1),A
;	EI			; Befehl von CAOS 4.3 bis 4.7 zusaetzlich
	RET
;
; Umschaltung Grafikmode mit ESC-G
;
ESCG:	LD	C,10000000b	; 2-Monitor-Bit ein/aus
	DB	21H	; LD HL,nn

ESCC:	LD	C,00100000b	; IBM <-> CAOS
	DB	21H	; LD HL,nn

ESC7:	LD	C,00000100b	; Invers-Bit
XORSTB:	LD	HL,STBT
	LD	A,(HL)
	XOR	C
	LD	(HL),A
	RET

ESC9:	;DI			; Befehl von CAOS 4.3 bis 4.7 zusaetzlich
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

; Druckerinitialisierung V.24 oder Centronics:
LST7F:	DW	7F7FH
	DB	'LSTDEV'
	DB	1
;_____________________________________________________________
;							**47**
; PE:	(ARGN) - Anzahl der Argumente (0 bis 5)
;	(ARG1)	Modulschacht des zu nutzenden Moduls
;	(ARG2)	Kanal des V.24-Moduls (1/2)
;	(ARG3)	USER-Ausgabekanal (2/3)
;	(ARG4)	Reaktion auf SHIFT CLEAR
;	(ARG5)	Druckertyp
; PA:	-
; VR:	AF, BC, DE, HL
;
V24OUT:	LD	BC,LSTC		; Druckerinitialisierung
	JP	CLROMC		; im ROM-C ausfuehren

; Duplexroutine

V24D7F:	DW	7F7FH
	DB	'V24DUP'
	DB	1
;_____________________________________________________________
;							**48**
; PE:	(ARGN)	Anzahl der Argumente (0 oder 3)
;	(ARG1)	Modulschacht V.24-Moduls (8, C ...)
;	(ARG2)	Kanal des V.24-Moduls (1/2)
;	(ARG3)	USER-Aus/Eingabekanal (2/3)
; PA:	-
; VR:	AF, BC, DE, HL
;
V24DUP:	LD	BC,V24DC	; Duplex-Initialisierung
	CALL	CLROMC		; im ROM-C ausfuehren
	CALL	C,ERRM		; Fehler
	RET

; Druckroutine mit Protokoll

ECHO:	CALL	CRT		; Zeichen auf Bildschirm anzeigen
PRINT:	BIT	7,(IX+4)	; CAOS-C bereits ein?
	JP	NZ,PRINTC	; direkt zur Druckausgabe
	PUSH	BC
	LD	BC,PRINTC	; Druckroutine
	CALL	CLROMC		; im ROM-C ausfuehren
	POP	BC
	RET

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
	CALL	LCSO		; SIO-Offset fuer V24OUT ermitteln
V24PR0:	ADD	A,0Ah		; SIO-Steuerport
	LD	C,A
V24PR1:	IN	A,(C)
	AND	00000100b	; bereit?
	JR	Z,V24PR1	; nein, warten
	;
V24PR2:	DEC	C
	DEC	C		; SIO-Datenport
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
	CALL	LCSOD		; SIO-Offset fuer V24DUP ermitteln
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
	 CALL	OUT86		; CAOS-C wie vor Interrupt
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
	CALL	Z,SCROFF	; IRM aus falls erforderlich
	POP	AF
	LD	(HL),A		; Byte eintragen
	JR	JCON

; Datenbyte holen (fuer DISPC, MODIC, FSAVE, view)
LDAME:	CALL	COFF		; CAOS-C aus
	LD	A,(MODST+1)
	AND	1		; IRM aus?
	CALL	Z,SCROFF	; IRM aus falls erforderlich
	LD	A,(HL)		; Byte holen
JCON:	CALL	SCRON		; IRM wieder ein
	JR	CON
;_____________________________________________________________
;							**31**
; CAOS-Arbeitsbereich verlagern
; PE:	A	HW-Teil der IX-Arbeitszellen

SIXD:	LD	BC,SIXC
	JR	CLROMC		; UP im CAOS-ROM C aufrufen
;-------------------------------------------------------------
; Aufruf eines UP im CAOS-ROMC
;  mit Einschalten des ROMC beim Aufruf
;  und ausschalten falls erforderlich beim Ruecksprung
; PE: BC=Adresse des UP im CAOS-ROMC

;CALLC:	EX	(SP),HL		; HL = Adresse der Routine
;	LD	C,(HL)
;	INC	HL		; PTR+1
;	LD	B,(HL)		; BC = CALL-Adresse
;	INC	HL		; PTR+2
;	EX	(SP),HL		; CALL-Adresse auf Stack	 HL  >ADR<
CLROMC:	CALL	CONBC
;
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
;
; CAOS-ROM-C einschalten:
;
CONBC:	PUSH	BC              ; Sprung in das durch BC adressierte UP
CON:	PUSH	AF
	SETIXA	7,4		; SET 7,(IX+4),A
	JR	CSW
;_____________________________________________________________
;							**41**
; Berechnung des Produktes zweier 8-Bit-Zahlen
; PE:	D, C	Faktoren (8 Bit)
; PA:	BA	Produkt (16 Bit)
; VR:	AF,DE,B

; Multiplikation BA=C*D	
MULT:	XOR	A		; Startwert 0

; MULCA: BA=D*C+A 
MULA:	LD	B,8		; 8 Bit Multiplikation/Rotation
	RR	D
MUL1:	JR	NC,MUL2		; Bit nicht gesetzt
	ADD	A,C		; aufaddieren
MUL2:	RRA
	RR	D		; jetzt 16 Bit rotieren
	DJNZ	MUL1		; und weiter addieren
	LD	B,A		; Ergebnis in
	LD	A,D		; BA
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

SETDEV:	LD	BC,SET_DD	; Routine im ROM C/D aufrufen
	JR	CLROMC		; im ROM-C ausfuehren

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

; ********	DEVICE-Routinen mit Modulschaltung aufrufen	********
;_____________________________________________________________
;							**01**
; Einen Block ausgeben:
; PE:	BC	Laenge Vorton
;	(IX+2)	Block-Nr. -1
; PA:	DE=HL	Pufferende + 1
;	(IX+2)	Block-Nr.
; VR:	AF,BC,DE,HL

MBO:	CALL	PV7
	DB	0
	RET
;_____________________________________________________________
;							**05**
; Einen Block einlesen:
; PE:	IX+5/6	Pufferadresse
; PA:	CY=1	Fehler
; VR:	AF,BC

MBI:	CALL	PV7
	DB	1
	RET
;_____________________________________________________________
;							**08**
; Initialisierung Bandausgabe / Datei zum Schreiben oeffnen:
; PE:	Kassettenpuffer mit Daten des Vorblocks
;	HL	Dateiname
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL

ISRO:	CALL	PV7
	DB	2
	RET
;_____________________________________________________________
;							**09**
; Abschluss Bandausgabe / Dateiausgabe schliessen:
; PE:	BC	Laenge Vorton
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL

CSRO:	CALL	PV7
	DB	3
	RET
;_____________________________________________________________
;							**0A**
; Initialisierung Bandeingabe:
; (Datei zum Lesen oeffnen)
; PE:	HL	Dateiname
; PA:	CY=1	Fehler
; VR:	AF,BC

ISRI:	CALL	PV7
	DB	4
	RET
;_____________________________________________________________
;							**0B**
; Abschluss Bandeingabe / Dateieingabe schliessen:
; PA:	CY=1	Fehler

CSRI:	CALL	PV7
	DB	5
	RET
;_____________________________________________________________
;							**14**
; Warteschleife ohne Interrupt
; PE:	A	Zeitwert t = (A) * 6 ms
; PA:	-
; VR:	AF, B

WAIT:	LD	B,A
	XOR	A
WAIT1:	CALL	WTUP		; 6ms-Zeitschleife
	DJNZ	WAIT1
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
TOKJP:	DW	INKEY		; D5
	DW	JOYST		; D6
	DW	STRING		; D7
	DW	INSTR		; D8
TADR5:	DW	RENUM		; D9
	DW	DELETE		; DA
	DW	PAUSE		; DB
	DW	BBEEP		; DC
	DW	WINDOW		; DD
	DW	ARGVL1		; DE (BORDER)
	DW	INK		; DF
	DW	PAPER		; E0
	DW	SNER		; E1 (Funktion AT)
	DW	BCOLOR		; E2
	DW	SOUND		; E3
	DW	PSET		; E4
	DW	PRESET		; E5
	DW	BLOAD		; E6
	DW	SNER		; E7 (Funktion VPEEK)
	DW	VPOKE		; E8
	DW	LOCATE		; E9
	DW	KEYLIST		; EA
	DW	BKEY		; EB
	DW	SWITCH		; EC
	DW	SNER		; ED (Funktion PTEST)
	DW	CLOSE		; EE
	DW	OPEN		; EF
	DW	RANDOM		; F0
	DW	SNER		; F1 (Funktion VGET)
	DW	BLINE		; F2
	DW	CIRCLE		; F3
	DW	SNER		; F4 (Funktion CSRLIN)
	DW	BDEV		; F5 Speichergeraet einstellen
	DW	FILES		; F6 Verzeichnis anzeigen
	DW	CHDIR		; F7 Verzeichnis wechseln
TOKJPE:				; F8-FF noch frei

	ABSFILL	0FDF8h,<ROM-F-Ende>

; Adresse FDF8h fest - wird von anderen Programmen (z.B. TEMO) mit genutzt!

BITTAB:	DB	80H,40H,20H,10H,8,4,2,1
