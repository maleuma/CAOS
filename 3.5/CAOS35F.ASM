;	CAOS 3.5 - ROM F
;	Version V.24 / RS 232 und Centronics
;	Date: 11.06.1991  (c) by '#+$'
;	Intern-Version (Direkteinbau in KC85/3)
; 06.07.2026: BASIC in USER-ROM Ebene 0 (kompatibel zu CAOS 3.1)
;	      bei JUMP 0 wird BASIC-ROM eingeblendet
; 10.07.2026: Statt Software-Modul wird ein M052 bei PWRON aktiviert
; 12.07.2026: Tastaturcodetabelle wie aus CAOS 4.9 uebernommen mit
;	      Codes fuer LIST/RUN, Sh-BRK, Sh-Enter und Sh-CAPS
; 20.07.2026: Beschaffung (ARGC) fuer PV2 wie CAOS 4.9 - 1 Byte gespart
; Hinweis: IRMON / IRMOF ohne Nutzung von IY programmiert
; 20.07.2026: PWRON initialisierte Fenster Nr. 10-0 (statt 9-0)
; 23.07.2026: ZSUCH optimiert bezueglich CY, 1 Byte gespart
;	(Kleinbuchstabe in Eingabe finden keine Menueworte in Grossbuchstaben)
; 25.07.2026: LIST/EDIT/BYE-Schutz fuer BASIC-Programme entfernt
; 25.07.2026: ZKOUT optimiert nach CAOS 4.9
; 25.07.2026: SIXD2/SIXD3 umgeschrieben: USER-ROM wird nicht mehr beeinflusst
; 29.07.2026: SIXD4 von ROM-F in ROM-E verschoben, damit USER-ROM bei RESET als
;	      AUS in MODST eingetragen werden kann
; 09.08.2026: blinkender Strichcursor wie bei CAOS 4.3 und hoher
; 12.08.2026: automatische Suche von Menueworten im USER-ROM, dafuer die Menue-
;	      worte USER und SEEK entfernt. GO wie in CAOS 4.9 eingebaut
;	      (Menuwort nicht sichtbar) -> 18 Byte frei im ROM F
; 15.08.2026: Menuewort WINDOW ohne Argumente stellt Standardfenster, volle
;	      Bildschirmgroesse weiss auf blau ein

	ORG	0F000H
 				;			  feste Adresse
	JP	PWRON		; >> POWER ON <<		F000
	JP	PV1		; DEFB nr			F003
	JR	PV2		; (ARGC)=nr			F006
ZEI0:	DB	0
	JR	PV3		; REG. E=nr			F009
ZEI2:	DB	2
	JR	PV4		; E=nr, IRM on/off		F00C
ZEI4:	DB	4
	JP	RCALL		; DEFW offset			F00F
	JP	NEWOS		; JUMP-Einsprung		F012
	JP	PV5		; E=nr, IRM/Stack		F015
	JP	IRMON		;				F018
	JP	IRMOF		;				F01B
 
PV6:	PUSH	BC		;*** PV 6 ***
	CALL	IRMON
	CALL	PV2
	JR	PV51
 
PV5:	PUSH	BC		;*** PV 5 ***
	CALL	IRMON
	CALL	PV3
PV51:	CALL	IRMOF
	POP	BC
	RET
 
PV4:	PUSH	AF		;*** PV 4 ***
	IN	A,(PIOAD)
	OR	4		; IRM ein
	OUT	(PIOAD),A
	POP	AF
	CALL	PV3
SCROFF:	PUSH	AF
	IN	A,(PIOAD)
	AND	0FBH		; IRM aus
	OUT	(PIOAD),A
	POP	AF
	RET
 
PV1:	PUSH	HL		;*** PV 1 ***
	DI
	POP	HL
	POP	HL
	INC	HL		; nr uebergehen
	PUSH	HL
	DEC	HL		; (HL)=nr
	DEC	SP
	DEC	SP
	EI
TABJP:	PUSH	DE
PV23:	LD	E,(HL)		; Nr. der Routine
PV11:	LD	D,0
	LD	HL,(SUTAB)
	PUSH	AF		; Flags unveraendert (fuer UP mit VR: -)
	ADD	HL,DE
	ADD	HL,DE		; HL=Pos. in UP-Tabelle
	POP	AF		; Flags regenerieren
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; Adresse des UP
	EX	DE,HL		; jetzt in HL
	POP	DE
PV1END:	EX	(SP),HL		; auf Stack legen
	RET			; UP anspringen
 
PV3:	SCF			;*** PV 3 ***
	DB	0EDH		; ED A7 = NOP
PV2:	AND	A		;*** PV 2 ***
PV21:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	HL,POPS3	; Return-Adresse
	EX	(SP),HL		; auf Stack legen
	PUSH	HL
	PUSH	DE
	JR	C,PV11		; falls PV3
	LD	HL,ARGC		; sonst E beschaffen aus ARGC
	JR	PV23
;
; IRM ein und Stack auf (SYSP) setzen
; aktuellen SP in VRAM merken
; VR:	BC, F
;
IRMON:	POP	BC		; Rueckkehradresse
	DI
	LD	(IX+11),A	; Inhalt von Register A merken
	IN	A,(PIOAD)
	SET	2,A		; IRM ein
	OUT	(PIOAD),A
	LD	(BSYSP),SP	; BASIC-SP hier merken (analog OS/PI)
	LD	SP,(SYSP)
OFFEND:	LD	A,(IX+11)
	EI
	PUSH	BC
	RET
;
; IRM ausschalten und Stack auf Wert in VRAM setzen
; VR:	BC
;
IRMOF:	POP	BC		; analog IRMON
	DI
	LD	SP,(BSYSP)	; SP regenerieren (analog OS/PI)
	LD	(IX+11),A	; Inhalt von Register A merken
	IN	A,(PIOAD)
	RES	2,A		; IRM aus
	OUT	(PIOAD),A
	JR	OFFEND
;_____________________________________________________________
;							**16**
; Eingabe eines Zeichens ueber def. Kanal
;
INTB:	PUSH	HL
	LD	HL,(INTAB)	; Eingabekanal
	JR	TABJP		; Ansprung ueber SUTAB

; Prompt ausgeben
OCHRP:	LD	A,(PROMPT)	; PROMPT
;_____________________________________________________________
;							**24**
; Ausgabe eines Zeichens ueber def. Kanal
; VR:	AF
OCHR:	PUSH	HL
	LD	HL,(OUTAB)	; Ausgabekanal
	JR	TABJP		; Ansprung ueber SUTAB

; Relativer UP-Aufruf (VR: DE)

RCALL:	EX	(SP),HL		;*** RCALL ***
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; DE=Offset
	INC	HL
	EX	DE,HL		; zur Absolutadresse
	ADD	HL,DE		; addieren
	EX	DE,HL
	EX	(SP),HL		; Zieladresse
	PUSH	DE
	RET

; Module listen, lesen, schalten:
; SWITCH ohne Argument listet alle Module auf
; SWITCH mit 1 Argument zeigt Schaltzustand des angegebenen Moduls
; SWITCH mit 2 Argumenten schaltet das Modul

	DW	7F7FH
	DB	'SWITCH',1
	JR	Z,MODUL		; ARGN=0, Module auflisten
	;
	LD	D,E		; Steuerbyte
	CALL	MODU		; Modul lesen oder schalten
MODOUT:	;Ausgabe 3 Bytes L,H,D
	CALL	LHSPCE		; Steckplatz anzeigen
	LD	A,H
	CALL	AHSPCE		; Kennbyte anzeigen
	LD	A,D
	CALL	AHEX		; Steuerbyte anzeigen
	JP	CRLF

; Modulkontrollanzeige (externe Module):
MODUL:	PUSH	DE
	LD	L,A
MODUL1:	XOR	A		; Schaltzustand lesen mit A=0
	CALL	MODU
	LD	A,H		; Kennbyte
	INC	A		; FF?
	CALL	NZ,MODOUT	; anzeigen falls <> FFh
	INC	L		; naechster Steckplatz
	JR	NZ,MODUL1	; bis Steckplatz FF
	POP	DE
	RET
IFF SEEK
; Module lesen und Schalten
MODUL2:	LD	L,2		; 2=USER-ROM
	LD	A,L		; A=2, Modul schalten
ENDIF
;_____________________________________________________________
;							**26**
; Module lesen und Schalten
; PE:	A=0 oder 1	Lesen
;	A=2		Schalten 8
;	L	Modulsteckplatz
;	D	Modulsteuerbyte fuer Port 80h
; PA:	H	Modultyp (Strukturbyte)
;	D	Modulsteuerbyte neu
; VR:	AF, D, H, BC

MODU:	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	C,80H		; I/O-Adresse fuer ext. Module
	LD	B,L		; Steckplatz
	CP	2		; mind. 2 Parameter angegeben?
	JR	NC,MDL2		; ja, Module schalten
	LD	D,(HL)		; Schaltzustand lesen
MDL1:	IN	H,(C)		; Strukturbyte lesen
	RET
 
MDL2:	LD	(HL),D		; neues Steuerbyte eintragen
	INC	B		; Steckplatz+1
;	DI			; damit PIO nicht von einer ISR geaendert wird
	IN	A,(PIOAD)	; aktueller Schaltzustand

	DJNZ	NRAM0		; 0: RAM0?
	AND	0F5H
	BIT	0,D
	JR	Z,RAM01
	SET	1,A		; RAM0 ON
RAM01:	BIT	1,D
	JR	Z,OUTB
	SET	3,A		; RAM0 Schreibschutz
OUTB:	OUT	(PIOAD),A
	JR	MDL1
 
NRAM0:	DJNZ	NIRM		; 1: IRM?
	BIT	0,D
	RES	2,A		; IRM OFF
	JR	Z,OUTB
	SET	2,A		; IRM ON
	JR	OUTB
 
NIRM:	DJNZ	NROM		; 2: USER-ROM?
; Nur in der internen Version: Segmentschaltung!
; SWITCH 2 01	oder	SWITCH 2 C1	Segment 0 = BASIC
; SWITCH 2 11	oder	SWITCH 2 D1	Segment 1 = EDAS
; SWITCH 2 21	oder	SWITCH 2 E1	Segment 2 = FORTH
; SWITCH 2 31	oder	SWITCH 2 F1	Segment 3 = WordPro
	ld	a,d		; Steuerbyte
	rla			; 1x nach links rotieren
	and	60h		; nur Bit 5+6 auswerten
	ld	h,a
	in	a,(PIOBD)	; PIO B
	and	9fh		; alten Schaltzustand ausfiltern
	or	h		; PUSER-ROM-Ebene einbauen in Bit 5+6
	OUT	(PIOBD),A
	in	a,(PIOAD)	; PIO-A
	RLA			; nach links rotieren
	LD	H,D		; Steuerbyte
	RR	H		; Bit0 nach CY
	RRA			; mit CY nach rechts rein rotieren
	JR	OUTB		; USER-ROM ein/aus

; externe Module schalten:
NROM:	LD	B,L		; Steckplatz
	OUT	(C),D		; Modul schalten
	JR	MDL1		; Strukturbyte lesen

; Betriebssystem wechseln:

	DW	7F7FH
	DB	'JUMP',1
	RET	Z		; ohne ARG nichts tun
	LD	A,L		; Steckplatz
;_____________________________________________________________
;							**27**
; Sprung in ein neues Betriebssystem
; PE:	A	Modulsteckplatz 8-FF
;		oder CAOS-ROM-Ebene 0-7

JUMP:	OR	A
	JR	Z,SYS		; JUMP 0 -> CAOS 3.1
	LD	B,A		; Steckplatz
	LD	C,80H		; I/O-Adresse fuer ext. Module
	IN	A,(C)		; Kennbyte lesen
	INC	A		; Kennbyte FF?
	JP	Z,ERRM		; ERROR (kein Modul)
; ab CAOS 4.3 werden hier alle Speichermodule ausgeschaltet
	LD	D,0FFH		; Ausgabe FF (Adresse C000)
	CALL	MOSW2		; Modul schalten
	in	a,(PIOAD)
	and	7eh		; CAOS-E und USER-C abschalten
	JR	SYS1
	;
SYS:	IN	a,(PIOAD)
	res	4,a		; res "NMI"
	set	7,a		; 06.07.2026: BASIC-ROM ein fuer CAOS 3.1
SYS1:	DI
	JP	NCAOS		; Sprung ueber IRM
 
;====== POWER ON =======================
;(Einspruenge PWRON,NEWOS,BYE)
 
PWRON:	DI
	LD	SP,STACK	; System-Stack KC85/3-5
;	LD	A,3FH
;	OUT	(PIOAD),A	; ROM-E ein, ROM-C aus
	CALL	MODSW1		; RAM4-Modul ein

; Speicher loeschen von 0 bis DFFFh
	XOR	A
	LD	B,0E0H
	LD	H,A
	LD	L,A		; HL=0000h
PWRO1:	LD	(HL),A
	INC	L
	JR	NZ,PWRO1
	INC	H
	DJNZ	PWRO1

; Module abschalten von Steckplatz 0 bis FF
	LD	C,80H
PWRO2:	OUT	(C),A		;Modul off
	DJNZ	PWRO2

; Einsprungadresse von JUMP
NEWOS:	DI
	CALL	SYSI		; Systeminit
	LD	A,9		; Fenster Nr. 9 (CAOS 3.4: 10 ?)
PWRO4:	LD	(WINNR),A	; Nr. eintragen
	CALL	WCOPY		; Fenstervektorspeicher initialisieren
	sub	1		; fuer 10 Fenster (9-0)
	JR	NC,PWRO4
	CALL	MODSW		; RAM4-Modul, Druckermodul, USB ein
;_____________________________________________________________
;							**0D**
; Einsprungadresse bei RESET

BYE:	LD	SP,STACK
;	LD	A,1FH
;	OUT	(PIOAD),A	; init. MEM on !!
	CALL	SYSI		; Systeminit
	JR	MEN1
 
	DW	7F7FH
	DB	'MENU',1
	;
	POP	HL		; Stack clear
MEN1:	CALL	OSTR
	DB	0CH,0AH
	DB	'* KC-CAOS 3.5 *',0
;_____________________________________________________________
;							**46**
; Menue anzeigen und Uebergang zur Kommandoeingabe
; PE:	(IX+9)	Prologbyte

MENU:	LD	HL,0C000H	; Menuewortsuche immer ab C000H
	LD	C,L
	LD	B,L		; BC=0 (Suchlaenge)
MEN2:	CALL	CRLF
MEN3:	CALL	BRKT
	JR	C,LOOP
	LD	A,(IX+9)	; Prologbyte
MEN4:	CPIR
	JP	PO,LOOP		; ausgesucht
	CPI			; 2mal nacheinander?
	JR	NZ,MEN4		; nicht 2mal
	CALL	OCHRP		; Prompt ausgeben
MEN5:	LD	A,(HL)
	CP	2		; Epilog 0 oder 1?
	JR	C,MEN2		; Menuewort fertig angezeigt
	CP	'0'
	JR	C,MEN6		; kleiner als '0'
	CP	60H
	JR	NC,MEN6		; groesser als 5FH
	CALL	OCHR		; Zeichen anzeigen
	INC	HL
	DEC	BC
	JR	MEN5
	;
MEN6:	CALL	CLLN		; Zeile loeschen
	JR	MEN3		; Prompt und weiter suchen

; LOOP
NLP1:	POP	HL
NLP2:	CALL	ERRM		; kommt mit CY=0 zurueck
LOOP0:	LD	A,0BH		; Cursor hoch
	CALL	C,OCHR		; bei BRK
	CALL	CLLN		; Zeile loeschen
;_____________________________________________________________
;							**12**
; Eingabeschleife:

LOOP:	CALL	OCHRP		; Prompt ausgeben
	CALL	INLIN		; Eingabe
	JR	C,LOOP0		; Brk -> Zeile loeschen
	INC	DE
	LD	A,(DE)		; erstes Zeichen nach Prompt
	CP	21H		; leer?
	JR	C,LOOP		; nichts tun
IF SEEK
	CALL	MSUCH		; Menuewort im Speicher suchen
	JR	NC,NLP2		; nicht gefunden -> Error
	PUSH	HL
	CALL	GARG
ELSE
	LD	A,(MODST+2)	; aktueller Schaltzustand USER-ROM
	LD	(ARGC),A	; CAOS 4.6: hier uebergeben an USER-Programme
	ld	b,0		; gesamten Speicher durchsuchen
inpt1:	call	msuch		; ab C000H
	JR	C,INPT4		; gefunden
; USER-ROM noch durchsuchen:
	LD	A,(MODST+2)	; normalerweise 0 (aus)
	OR	0C1H		; Schaltzustand EIN
	ADD	A,10H		; Offset zur naechsten Ebene: C1, D1, E1, F1
	ld	b,20h		; Suchbereich jetzt nur noch 2000h
	JR	NC,INPT1	; noch nicht ausgesucht
	LD	DE,(ARGC-1)	; CAOS 4.6: D = USERC-Schaltzustand vor Suche
	CALL	MODUL2		; USER-ROMC wie zuvor
	JR	NLP2		; nicht gefunden -> Error
	;
INPT4:	PUSH	HL		; Adresse (Epilog)
	CALL	GARG		; Argumente erfassen
ENDIF
	JR	C,NLP1		; Parameterfehler -> Error
	LD	HL,LOOP
	EX	(SP),HL		; Rueckkehradresse auf Stack
	LD	A,(HL)		; Epilogbyte lesen
	INC	HL		; uebergehen
	PUSH	HL		; Startadresse des Programms
	AND	A		; Epilog = 0?
	CALL	LARG		; Argumente laden
	CALL	Z,SCROFF	; IRM aus bei Epilog=0
	AND	A		; Anzahl Argumente = 0?
	RET
IF SEEK
MSUCH:	LD	HL,0C000H	; Standard-Suchbeginn
	LD	B,L
	LD	C,L		; BC=0
ELSE
; A = Steuerbyte USER-ROM
; B = High(Suchlaenge)
; DE = Suchkette
MSUCH:	PUSH	DE		; DE = Suchkette
	 LD	D,A		; Steuerbyte
	 CALL	MODUL2		; USER-ROM-C Ebene einschaltem
	POP	DE		; DE = Suchkette
	LD	HL,0C000H	; Standard-Suchbeginn
	LD	C,L		; C=0
ENDIF
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
; VR:	AF, BC, DE, HL

ZSUCH:	AND	A		; CY=0
	CPIR			; (CY wird nicht veraendert)
	RET	PO		; Prolog nicht gefunden
	CPI
	JR	NZ,ZSUCH	; nur 1x gefunden
	 PUSH	DE		; Beginn der Vergleichskette retten
	 PUSH	AF		; Prologbyte
ZS1:	LD	A,(DE)		; Zeichen der Vergleichskette
	CP	21H
	JR	C,ZS3		; Kettenende bei Vergleichskette erkannt
	INC	DE
	CPI
	JR	Z,ZS1
ZS2:	 POP	AF		; verwendetes Prologbyte restaurieren
	 POP	DE		; und wieder zu Beginn der Vergleichskette
	JR	ZSUCH		; Suche fortse
	;
ZS3:	LD	A,(HL)		; Zeichen aus Speicher weiter testen
	CP	2		; bis Epilogbyte (0/1) kommt
	JR	C,ZS4		; Epilog gefunden
	CP	20H
	JR	C,ZS2		; 02h-1Fh nicht zugelassen
	INC	HL
	DEC	BC		; Befehl wie bei CAOS 4.9 ergaenzt
	JR	ZS3
	;
ZS4:	POP	BC
	POP	BC		; Anfang der Vergleichskette
;	SCF			; CY=1 vom CP 02H
	RET
;_____________________________________________________________
;							**14**
; Warteschleife ohne Interrupt
; PE:	A	Zeitwert t = (A) * 6 ms
; PA:	-
; VR:	AF, B

WAIT:	LD	B,A
	XOR	A
WAIT0:	CALL	WAIT1		; 6ms-Zeitschleife
	DJNZ	WAIT0
	RET
;
; Zeitschleife 6ms fuer UP WAIT
; PE:	A=0	ergibt 6ms
; PA:	A=0
;
WAIT1:	DEC	A		; 4 Takte
	RET	Z		; 5 (11) Takte
	PUSH	AF		; 11 Takte
	POP	AF		; 10 Takte
	JR	WAIT1		; 12 Takte

BNROST:	LD	A,(IX+2)	; Blocknummer
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
;_____________________________________________________________
;							**13**
; Normalein- und Ausgabe: CRT und KBD
; PE:	-
; PA:	HL	alter Ausgabezeiger
; VR:	HL

NORM:	CALL	NIN		; Normaleingabe (KBD)
;_____________________________________________________________
;							**20**
; Setzen Normalausgabe (CRT)

NOUT:	LD	HL,ZEI0		; Normalausgabe (CRT)
;_____________________________________________________________
;							**1E**
; Setzen eines neuen Output Kanals

SOUT:	PUSH	HL
	LD	HL,(OUTAB)	; bisheriger Zeiger
	EX	(SP),HL
	LD	(OUTAB),HL	; neuer Zeiger
	POP	HL
	RET
;_____________________________________________________________
;							**21**
; Setzen Normaleingabe (KBD)

NIN:	LD	HL,ZEI4		; Normaleingabe (KBD)
;_____________________________________________________________
;							**1F**
; Setzen eines neuen Input Kanals

SIN:	PUSH	HL
	LD	HL,(INTAB)	; bisheriger Zeiger
	EX	(SP),HL
	LD	(INTAB),HL	; neuer Zeiger
	POP	HL
	RET
;_____________________________________________________________
;							**2A**
; Test auf BRK-Anforderung
; CAOS 3.4: zusaetzlich wird die STOP-Taste abgefragt und  danach
; gewartet, bis eine weitere Taste zur Fortsetzung gedrueckt wurde
; PA:	CY=1	BRK-Taste wurde gedrueckt
;	A	Tastencode, 03H bei BRK-Taste

BRKT:	CALL	KBDS		; Tastaturstatusabfrage
BRKT1:	CP	3		; BRK-Taste?
	SCF			; CY=1
	RET	Z
	CP	13H		; STOP-Taste?
	SCF		; CY=1
	CCF		; CY=0
	RET	NZ		; nicht STOP
	CALL	RESFL		; Tastencode quittieren
	CALL	KBD1		; neue Taste abwarten
	JR	BRKT1		; und pruefen
;_____________________________________________________________
;							**04**
; Eingabe eines Zeichens mit Cursoreinblendung
;
KBD:	BIT	6,(IX+8)	; F-Taste aktiv?
KBD1:	PUSH	HL
	PUSH	DE
	PUSH	BC
	JR	NZ,KBD8		; F-Taste aktiv
KBD2:	CALL	CUCP		; Cursor einblenden
KBD3:	CALL	KBDZ		; Tastatur abfragen
	JR	NC,KBD3		; keine Taste gedrueckt
	CALL	CUCP		; Cursor wieder wegnehmen
KBD4:	BIT	5,(IX+8)	; Click ein?
	LD	D,A		; Tastencode retten
	CALL	NZ,RCLICK	; Tastenclick erzeugen
	LD	A,D		; Tastencode zurueck in A
	LD	HL,STBT
	BIT	4,(HL)		; ESC aktiv?
	CALL	NZ,CRT		; ESC-Sequenz ausfuehren
	JR	NZ,KBD2		; und in Eingabeschleife bleiben
KBD5:	CP	ESC
	JR	NZ,KBD6
	SET	4,(HL)		; ESC vorerst nur merken
	JR	KBD2		; und in Eingabeschleife bleiben
	;
KBD6:	CP	0F1H		; F-Taste?
	JP	C,POPS3		; nein
	SUB	0F0H		; Nr. der Taste (1-15)
	LD	B,A
	LD	HL,FTASTE	; 0B900H
KBD7:	LD	A,(HL)
	AND	A		; Trenn-Null?
	INC	HL
	JR	NZ,KBD7		; Position im Puffer
	DJNZ	KBD7		; auszaehlen
	SET	6,(IX+8)	; F-Taste aktiv
	JR	KBD9		; gefunden
; F-Taste aktiv, Zeichen aus Speicher entnehmen
KBD8:	CALL	BRKT		; Abbruch?
	JR	C,KBD10		; ja, deaktivieren
	LD	HL,(FTAST)	; Zeiger
KBD9:	LD	A,(HL)		; Zeichen entnehmen
	INC	HL		; naechstes CHR
	LD	(FTAST),HL	; Zeiger aktualisieren
	AND	A		; NUL-Ende?
	JP	NZ,POPS3	; nein, Zeichen uebergeben
KBD10:	RES	6,(IX+8)	; F-Taste deaktivieren
	JR	KBD2		; und in Eingabeschleife bleiben
;_____________________________________________________________
;							**17**
; Eingabe einer Zeile mit Funktion aller Cursortasten,
; Abschluss mit <ENTER> oder Abbruch <BRK>
;
; PA:	CY=0	Eingabe mit Enter beendet (ab CAOS 4.3)
;	CY=0	Eingabe mit LIST/RUN/CONT beendet (ab CAOS 4.9)
;	CY=1	Eingabe mit BRK beendet (ab CAOS 4.3)
;	DE	VRAM-Adresse auf Beginn der Eingabezeile
;	A	Tastencode mit dem INLIN abgeschlossen wurde
; VR:	AF, DE
;
INLIN:	CALL	INTB		; Zeicheneingabe
	CALL	OCHR		; und Anzeige
	CP	CR
	JR	Z,ILN1
	CP	3
	JR	NZ,INLIN
	SCF
ILN1:	PUSH	AF		; CY=1 bei BRK, CY=0 bei Enter
	CALL	CRLF		; Cursor an Anfang der naechsten Zeile
	LD	DE,(CURSO)
	CP	D		; Zeile 1 durch PAGE-Mode?
	JR	NZ,ILN2
	LD	A,(WINLG+1)	; Anzahl Zeilen
	LD	D,A		; D muss dann in letzte Zeile zeigen
ILN2:	DEC	D		; Zeile darueber
	PUSH	HL		; HL sichern
	CALL	DABR		; VRAM-Adresse ermitteln
	EX	DE,HL		; VRAM-Adresse in DE zuruekgeben
	POP	HL		; HL unveraendert
	POP	AF		; PA: CY=BRK
	RET
;_____________________________________________________________
;							**1A**
; 16-Bit-Wert hexadezimal anzeigen und ein Leerzeichen danach
; VR:	AF
HLHX:	LD	A,H
	CALL	AHEX
LHSPCE:	LD	A,L
AHSPCE:	CALL	AHEX
;_____________________________________________________________
;							**2B**
; Leerzeichen anzeigen
; VR:	AF
SPACE:	LD	A,' '		; Leerzeichen
SPAC0:	JP	OCHR
;_____________________________________________________________
;							**2D**
; Cursor in Home-Position
; VR:	AF
HOME:	LD	A,10H		; Home-Code
	JR	SPAC0
 
ERRM:	CALL	OSTR		;UP 19 ERRM
	DEFM	'Error'
	DEFB	7
	DEFB	0
	;
CRLF:	CALL	OSTR		;UP 2C CRLF
	DEFB	0DH
	DEFB	0AH
	DEFB	0
	RET
 
CLLN:	LD	A,2
	JR	SPAC0
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
	JR	SPAC0

; OCHR:

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
RHEX0:	INC	DE
RHEX:	LD	A,(DE)		; Zeichen holen
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
	RET	C		; Zahl zu klein
	CP	10
	JR	C,RH2
	SUB	7
	AND	0DFH		; klein -> gross
	CP	10
	RET	C		; Buchstabe zu klein
	CP	16
	CCF
	RET	C		; Buchstabe zu gross
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
; Bis zu 10 hexadezimale Argumente erfassen
; PE:	DE	Adresse im VRAM
; PA:	(ARGN) und (ARG1) bis (ARG10)
;	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
GARG:	LD	BC,ARGN
	XOR	A
	LD	(BC),A		; (ARGN)=0
	INC	DE	; ???
GARG1:	CALL	RHEX		; HEX-Zahl erfassen
	RET	C		; Fehler
	LD	A,(HL)		; NUMNX lesen
	OR	A		; Zeichenanzahl=0?
	RET	Z		; ja, fertig
	INC	HL		; NUMVX
	INC	BC
	LD	A,(HL)		; L(NUMVX)
	LD	(BC),A		; L(Argument)
	INC	HL
	INC	BC
	LD	A,(HL)		; H(NUMVX)
	LD	(BC),A		; H(Argument)
	LD	L,Low(ARGN)
	INC	(HL)		; erfasste Argumente mitzaehlen
	LD	A,(HL)
	ADD	-11
	JR	NC,GARG1	; noch keine 10 Arg erfasst
	RET
;
; System-Initialisierung (bei PWRON und RESET):
;
SYSI:	IM	2
	LD	HL,KCTAB0	; Init-Daten PIO, CTC und I/O-Modul
IF V24
	LD	D,6+2		; +2 bei V.24-Interface (M003)
ELSE
	LD	D,6+3		; +3 bei CEN-Interface (M001)
ENDIF
	CALL	INIME		; Ports initialisieren
	;	HL=SYSTAB	; IRM-Defaulttabelle folgt KCTAB0
	LD	DE,HCADR
	LD	BC,STBL1
	LDIR			; 1. Teil in IRM kopieren
;	LD	E,0D8H		; H-Teil von Bittab	B7D8H
	LD	E,Low(Prompt)
	LD	C,STBL2
	LDIR			; 2. Teil in IRM kopieren
	LD	A,1		; Standardwert fuer I-Regiester und IXh
;_____________________________________________________________
;							**31**
; CAOS-Arbeitsbereich verlagern
; PE:	A	HW-Teil der IX-Arbeitszellen

SIXD:	DI
	LD	HL,ISRTAB	; ISR-Tabelle fuer System-ISR im ROM
	LD	(MIXIT),A	; neuer Wert fuer IXH- und I-Register
	LD	I,A		; I-Register aktualisieren
	LD	E,0E4H		; 01E4H = ISR fuer PIO-A
	LD	D,A		; 01
	LD	BC,KCTAB3-ISRTAB ; Interrupttabelle fuer 6 Interrupts
	LDIR
	PUSH	DE		; 01F0H
	POP	IX
	LD	(IX+8),88H
	LD	(IX+9),7FH	; Standard-Prologbyte
	LD	(IX+14),Low(KTAB)
	LD	(IX+15),High(KTAB)
	;	HL=KCTAB3	; KCTAB3 folgt ISRTAB
	LD	DE,MODST
	LD	C,3		; B ist noch 0 vom letzten LDIR
	LDIR			; Schaltzustand Steckplatz 0,1,2
	;	HL=KCTAB0
	LD	D,2		; 2 Ports
	CALL	INIME		; PIO-B initialisieren
	LD	HL,KCTAB1
	LD	D,3		; 4 Ports
	CALL	INIME		; PIO-A Steuerwort, CTC0, CTC2 initialisieren
	LD	A,(IX+7)
	AND	1FH		; Bit 5=0: Blocknummern anzeigen
	LD	(IX+7),A	; Bit 6,7=0
;	LD	A,9FH		; USER-ROM Segment 0 (modifiziert 06.07.2026)
;	OUT	(PIOBD),A	; Blinken ein, Lautstaerke = aus
	EI
	JP	SIXD4		; Standard-Kassettenpuffer
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
	OTIR			; ... Bytes ausgeben
	POP	BC
	RET
;_____________________________________________________________
;							**44**
; Initialisierung mehrerer E/A-Kanaele
; PE:	HL	Tabelle mit Initialisierungsdaten
;	D	Anzahl der Kanaele
; VR:	F, D, HL

INIME:	CALL	INIEA		; einen Kanal initialisieren
	DEC	D		; noch ein Kanal
	JR	NZ,INIME	; ja, wiederholen
	RET
 
;*** Tape-Routinen ***
 
	DW	7F7FH		;>SAVE<
	DB	'SAVE',1
	;
	CP	2		; mindestens 2 Argumente erforderlich
	JP	C,ERRM		; sonst Fehler
SAVE1:	CALL	OSTR
	DB	'Name :',0
	CALL	INLIN		; Dateiname abfragen
	RET	C		; BRK
	LD	HL,6
	ADD	HL,DE		; Beginn Name
;_____________________________________________________________
;							**36**
; Datei speichern
;
; PE:   HL	Dateiname
; PA:   CY=1    Fehler oder BRK
; VR:	AF, BC, DE, HL
;
SAVE:	LD	DE,CASS
	LD	BC,11		; 8+3 (NAME/TYP)
	LDIR			; Name in Puffer kopieren
	XOR	A
	LD	(DE),A		; Ende-0 anhaengen
	LD	HL,ARGN		; alle Argumente inklusive ARGN
	LD	E,10H
	LD	C,15H
	LDIR			; in den Puffer kopieren
	CALL	NBWI		; Vorblock ausgeben
	RES	5,(IX+7)	; Blocknummern anzeigen
	LD	HL,(ARG1)	; Adresse fuer aktuellen Block
SAVE0:	LD	(IX+5),L
	LD	(IX+6),H	; Adresse als neuen Puffer setzen
	LD	DE,128		; Blocklaenge
	ADD	HL,DE
	LD	DE,(ARG2)	; Endadresse+1
	SBC	HL,DE		; Ende erreicht?
	CCF
	JP	C,NBW1		; ja, Endeblock ausgeben mit CY=1
	CALL	NBW		; Datenlock auf Kassette ausgeben
	JP	C,CRO0		; bei BRK: Bandein-/ausgabe beenden
	JR	SAVE0		; naechsten Block
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

; Fehlermeldung: keine MC-Datei
NBL1:	CALL	OSTR
	DB	'???',0
NBL2:	JP	CMBI		; Abschluss Bandeingabe (CSRO)
 
	DW	7F7FH		;>VERIFY<
	DB	'VERIFY',1
;_____________________________________________________________
;							**11**
; Kassettendatei kontrollieren

VERIFY:	RES	0,(IX+7)	; VERIFY, nicht laden
	JR	LOAD0

	DW	7F7FH		;>LOAD<
	DB	'LOAD',1
	;
LOAD:	SET	0,(IX+7)	; Daten lesen
	;
LOAD0:	CALL	IMBII		; Blocknummer 1 erwarten, MB-Eingabe init.
	CALL	NBR		; Vorblock einlesen
	RES	5,(IX+7)	; Blocknummern anzeigen
LOAD1:	JR	C,NBL2		; Abbruch/Fehler > CSRO
	LD	HL,CASS		; Dateiname am Blockanfang
	LD	B,11		; 11 Zeichen Dateiname/Typ
LOADN:	LD	A,(HL)		; Zeichen aus Kassettenpuffer
	INC	HL
	CALL	CRT		; anzeigen
	DJNZ	LOADN
	CALL	SPACE		; Leerzeichen zwischen Dateiname + Adressen
	BIT	0,(IX+7)	; LOAD oder VERIFY?
	JR	Z,LOAD4		; VERIFY
	LD	L,10H
	LD	A,(HL)		; Anzahl der Argumente beim Abspeichern
	SUB	2		; 0 oder 1 Argument ist falsch
	CP	8		; MC?
	JR	NC,NBL1		; Fehler: kein gueltiger CAOS-Vorblock
	RLCA			; Anzahl ARG-2
	RLCA			; nach Bits 2..4 rotieren
	LD	H,A
	LD	A,(IX+7)
	AND	3		; Bit 0,1 erhalten
	OR	H		; Bit 2-4 einbauen
	LD	(IX+7),A
	LD	BC,(CASS+15H)	; SADR
	LD	DE,(CASS+13H)	; EADR+1
	LD	HL,(CASS+11H)	; AADR
	LD	A,(ARGN)
	AND	A		; Ladeoffset?
	JR	Z,LOAD3		; nein
	PUSH	BC		; Startadresse auf Stack
	LD	BC,(ARG1)	; Offset
	ADD	HL,BC		; Anfangsadresse umrechnen
	EX	DE,HL
	ADD	HL,BC		; Endadresse umrechnen
	EX	DE,HL
	BIT	3,(IX+7)	; Bit 4 testen ???
	JR	NZ,LOAD2
	EX	(SP),HL
	ADD	HL,BC		; Startadresse auch umrechnen
	EX	(SP),HL
LOAD2:	POP	BC		; Startadresse
LOAD3:	LD	(ARG1),HL
	LD	(ARG2),DE
	LD	(ARG3),BC
	CALL	HLDE		; Anfangsadresse und Endadresse anzeigen
LOAD4:	CALL	CRLF		; neue Zeile
	EX	DE,HL		; Ziel=DE
LOAD5:	CALL	NBR		; neuen Block einlesen
	JR	C,LOAD1		; Abbruch >CSRO
	BIT	0,(IX+7)
	JR	Z,LOAD6		; Verify
	PUSH	HL		; Endadresse
	SBC	HL,DE		; noch zu lesende Bytes
	LD	BC,128		; Blockgroesse 128 Byte
;	OR	A		; CY=0, damit auch EADR<AADR funktioniert!
	SBC	HL,BC		; voller Block?
	ADD	HL,BC
	JR	NC,BLC1		; ja
	LD	C,L		; restliche Anzahl
BLC1:	LD	HL,CASS		; Standard-Kassettenpuffer
	LDIR			; gelesenen Block kopieren
	POP	HL
LOAD6:	INC	(IX+2)		; Block FF?
	JR	NZ,LOAD5	; nein, weiter lesen
;_____________________________________________________________
;							**0B**
; Abschluss Bandeingabe / Dateieingabe schliessen:
; PA:	CY=1	Fehler

CSRI:	CALL	CMBI		; Abschluss Bandeingabe
	LD	A,(IX+7)
	RRA			; Bit 0 nach CY
	RET	NC		; nichts tun bei VERIFY
	AND	0EH		; Bits 2..4 testen (ARGN-2 in Vorblock)
	RES	0,(IX+7)
	RET	Z		; kein Autostart, da nur 2 ARG
	;
	BIT	1,(IX+7)	; AutoStart enable ?
	JR	Z,CSRI3
CSRI1:	CALL	OSTR
	DB	'Run',0
	CALL	YESNO		; Ja/Nein-Abfrage
	JR	Z,CSRI3		; Yes geantwortet => Autostart
	CP	'S'
	RET	NZ		; fertig
; Falls die Frage Run? mit "S" beantwortet wird, erfolgt sofort ein SAVE
; der soeben eingelesenen Datei
	LD	A,3
	LD	(ARGN),A
	CALL	SAVE1		; PROGRAMM COPY
	JR	CSRI1
; Autostart
CSRI3:	LD	HL,(ARG3)	; Startadresse
	JP	(HL)		; anspringen

; Ja/Nein-Abfrage (VERIFY nach SAVE / RUN nach LOAD)
; PA:	Z=1 bei YES
; VR:	AF

YESNO:	CALL	OSTR
	DB	'? (Y):',0
	CALL	INTB		; Zeicheneingabe
	PUSH	AF		; Code merken
	CALL	OCHR		; ..anzeigen
	CALL	CRLF		; neue Zeile
	POP	AF
	RES	5,A		; upcase
	CP	'Y'		; Yes?
	RET

; Neuen Block einlesen fuer MBIN und LOAD
; NewBlockRead
; PA:	CY=1	Abbruch oder Fehler

NBR:	PUSH	HL
	LD	L,4		; 4 Versuche
	LD	H,(IX+3)	; erwartete Blocknummer
NBR0:	CALL	BRKT
	JR	C,NBREND	; Abbruch mit BRK
	CALL	PV1		; Block einlesen mit
	DB	5		; MBI
	LD	A,(IX+2)	; gelesene Blocknummer
	PUSH	AF
	CP	H		; korrekt?
	JR	Z,NBR2		; ja
	INC	A		; Endeblock FF?
	JR	NZ,NBR4		; nein
	LD	A,L
	CP	4		; noch 4 Versuche uebrig?
	JR	NZ,NBR4		; Kein 1.Versuch
	DEC	H
	JR	Z,NBR41		; 01 erwartet
	INC	H		; H korrigieren
NBR2:	POP	AF		; gelesene Blocknummer
	JR	C,NBR5		; Fehler von MBI gemeldet
	BIT	5,(IX+7)	; Blocknummern anzeigen?
	JR	NZ,NBR3		; nein
	CALL	CLLN		; Zeile loeschen
	CALL	BNROST		; Blocknummer und Folgetext anzeigen
	DB	'> ',0		; Block korrekt eingelesen
NBR3:	INC	H		; naechste Blocknummer erwarten
	LD	(IX+3),H	; und hier eintragen
NBREND:	POP	HL
	RET

; Block Nr. 1 erwartet
NBR41:	INC	H		; H korrigieren
NBR4:	POP	AF		; gelesene Blocknummer
	CALL	BNROST		; Blocknummer und Folgetext anzeigen
	DB	'*',8,8,8,0	; nicht erwartete Blocknummer
	JR	NBR0		; weiter versuchen

; Block fehlerhaft gelesen
NBR5:	LD	H,A		; Soll=Ist
	DEC	L		; letzter Versuch?
	JR	Z,NBR6		; Fehlerhaften Block uebernehmen
	CALL	BNROST		; Blocknummer und Folgetext anzeigen
	DB	'? ',7,0	; Lesefehler (Pruefsumme falsch)
	JR	NBR0		; weiter versuchen
 
NBR6:	CALL	BNROST		; Blocknummer und Folgetext anzeigen
	DB	'!',1EH,0	; Fehlerhaften Block uebernommen
	JR	NBR3		; fertig

; NewBlockWrite
; PA:	CY=1	BRK
NBW:	CALL	BRKT
	RET	C
; Endeblock ausgeben mit CY=1
NBW1:	LD	BC,(VORTN)	; kurze Vortonlaenge normal
	BIT	5,(IX+7)	; Blocknummern anzeigen?
	JR	Z,NBW2		; ja
	LD	BC,1280		; bei 'U' gedehnt
NBW2:	JP	C,CSRO		; Endeblock ausgeben
	CALL	PV1		; Datenblock ausgeben mit
	DB	1		; MBO 
NBW3:	CALL	BNROST		; Blocknummer und Folgetext anzeigen
	DB	'<',CR,0
	RET

; Vorblock ausgeben
NBWI:	CALL	ISROI
	CALL	CLLN
	JR	NBW3
 
;*** Monitor ***
 
	DW	7F7FH		;>COLOR<
	DB	'COLOR',1
;_____________________________________________________________
;							**0F**
; Farbe setzen (aus L und E)

COLR:	PUSH	AF		; ARGN
	LD	A,1FH		; Maske fuer Vordergrundfarbe
	AND	L		; ARG1 = Vordergrundfarbe
	RLA
	RLA
	RLA
	LD	L,A
	POP	AF
	CP	2		; auch Hintergrund einstellen?
	LD	A,E		; neue Hintergrundfarbe
	JR	NC,COL1
	LD	A,(COLOR)	; alte Hintergrundfarbe
COL1:	AND	7
	OR	L
COL2:	LD	(COLOR),A	; neuer Farbwert
	RET

; ESC-2: Blinken Vordergrundfarbei ein/aus

ESC2:	LD	A,(COLOR)
	XOR	80H		; Blinken on/off
	JR	COL2

; Ausgabe einer Displayzeile
; HL=Adresse, C=Anzahl Bytes

DPMEMO:	CALL	HLHX		; Adresse
	PUSH	HL
	LD	B,C		; Anzahl Bytes
DPM1:	LD	A,(HL)		; Byte holen
	INC	HL
	CALL	AHSPCE		; anzeigen
	DJNZ	DPM1
	POP	HL		; Adresse auf alten Wert
	LD	B,C		; Anzahl Bytes
	LD	A,9
	CALL	OCHR		; Cursor nach rechts
DPM2:	LD	A,(HL)		; Byte holen
	INC	HL
	CALL	CSTBT		; als ASCII anzeigen
	DJNZ	DPM2
	RET
;_____________________________________________________________
;							**42**
CSTBT:	PUSH	HL
	LD	HL,STBT
	SET	3,(HL)
	CALL	OCHR		; Steuerzeichen darstellen
	RES	3,(HL)
	POP	HL
	RET

	DW	7F7FH		;>DISPLAY<
	DB	'DISPLAY',1
;_____________________________________________________________
;							**3B**
; Anzeige Speicherbereich:
; PE:	HL=Adresse
;	E=Listblocklaenge
;	C=Anzahl Bytes pro Zeile
;	A=Anzahl Argumente

DISP:	SUB	3
	JR	NC,DISP1
	LD	C,8		; default = 8 Bytes/Zeile
	INC	A
	JR	Z,DISP1
	LD	E,4		; default = 4 Zeilen
DISP1:	PUSH	DE
DISP2:	CALL	DPMEMO		; 1 Zeile anzeigen
	CALL	CRLF
	CALL	BRKT
	JR	C,DISP3		; Abbruch
	DEC	E
	JR	NZ,DISP2	; E* Zeile
	CALL	INTB		; Warten auf Tastenbetaetigung
DISP3:	POP	DE
	CP	3		; BRK?
	RET	Z
	CP	13H		; STOP?
	JR	NZ,DISP1
	JR	MO1		; Uebergang zu MODIFY
 
	DW	7F7FH
	DB	'MODIFY',1
	LD	C,E		; Breite
	CP	2		; 2 Argumente?
	JR	NC,MO1
;_____________________________________________________________
;							**2E**
; Anzeige/Veraendern Speicherbereich
; PE:	HL=Adresse
;	E=Anzahl Bytes pro Zeile
;	A=Anzahl Argumente

MODI:	LD	C,1		; Standardbreite = 1 Byte
MO1:	CALL	DPMEMO		; 1 Zeile anzeigen
	LD	A,5
	LD	(CURSO),A
	CALL	INLIN		; Eingabe
	RET	C		; BRK
	PUSH	HL
	CALL	RHEX		; Adresse erfassen
	LD	A,(HL)
	POP	HL
	JR	C,MO10		; Fehler
	AND	A
	JR	Z,MO1
	LD	HL,(NUMVX)	; Wert (Adresse)
MO2:	PUSH	HL
	CALL	RHEX		; Datenbyte erfassen
	LD	A,(HL)
	AND	A		; kein Zeichen einlesbar?
	INC	HL
	LD	A,(HL)
	POP	HL
	JR	Z,MO4
MO3:	LD	(HL),A		; eintragen
	INC	HL
MO4:	LD	A,(DE)
	CP	'.'		; Abschluss?
	RET	Z
	CP	','		; 1 Zeichen?
	JR	NZ,MO5
	INC	DE
	LD	A,(DE)		; vom Video-RAM
	INC	DE
	JR	MO3		; eintragen
	;
MO5:	CP	':'		; Rueckschritt?
	JR	NZ,MO6
	LD	B,0
	SBC	HL,BC		; C Bytes zurueck
	JR	MO1
	;
MO6:	CP	'/'		; neue Adresse?
	JR	NZ,MO7
	INC	DE
	CALL	RHEX		; Adresse erfassen
	JR	C,MO10		; Fehler
	LD	HL,(NUMVX)	; neue Adresse
	JR	MO1
	;
MO7:	CP	27H		; Zeichenkette?
	JR	NZ,MO9
	INC	DE
MO8:	LD	A,(DE)		; Zeichen holen
	INC	DE
	AND	A		; Dummy-Ende?
	JR	Z,MO1
	CP	27H		; Ende der Zeichekette?
	JR	Z,MO4
	LD	(HL),A		; Zeichen eintragen
	INC	HL
	JR	MO8
	;
MO9:	AND	A		; Ende Zeile?
	JR	Z,MO1
	CP	' '		; Leerzeichen?
	JR	Z,MO2
MO10:	CALL	ERRM		; sonst Error
	JR	MO1
;
;***************************************************************
; Fensterverwaltung
; %WINDOW		aktiviert Fenster 0
; %WINDOW nr		aktiviert Fenster nr
; %WINDOW aa bb cc dd e	Fenster definieren
;         |  |  |  |  |
;         |  |  |  |  |_ Fenster-Nr. in (ARG5) optional
;         |  |  |  |____ Spaltenzahl in (ARG4)
;         |  |  |_______ Spaltenanfang in Register C
;         |  |__________ Zeilenanzahl in Register E
;         |_____________ Zeilenanfang in Register L
;
	DW	7F7FH
	DB	'WINDOW',1
	AND	A		; Kein Argument?
IF SEEK
	JR	Z,WINAK		; Fenster 0 aktivieren
ELSE
	JR	Z,WINC0		; Standard-Fenster 0 aktivieren
ENDIF
	DEC	A		; 1 Arg?
	JR	Z,WINAK0	; Aufruf Nummer L
	CP	3
	JR	C,ERRM2		; 2..3 Arg => ERROR
	LD	H,L		; erste Zeile
	LD	L,C		; erste Spalte
	LD	D,E		; Anzahl Zeilen
	LD	A,(ARG4)
	LD	E,A		; Anzahl Spalten
	LD	A,(ARG5)	; neue Fensternummer
	JR	NZ,WIN5
	LD	A,(WINNR)	; aktuelle Fensternummer beibehalten
WIN5:	CALL	WININ
	RET	NC		; OK
ERRM2:	JP	ERRM

IFF SEEK
WINC0:	LD	DE,WINNR
	LD	HL,WIN0
	LD	BC,11
	JR	JPLD		; LDIR, RET
ENDIF
;_____________________________________________________________
;							**3C**
; Initialisierung eines Fensters							**3C**
; PE:	A	Fensternummer 0..9
;	HL	Fensteranfang
;	DE	Fenstrergroesse
; PA:	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
WININ0:	LD	A,(WINNR)	; aktuelles Fenster (von BASIC genutzt)
WININ:	PUSH	AF		; Fensternummer
	CALL	PART		; Parameter-Test
IAD21:	JP	C,IAD2		; Fehler (POP AF, SCF, RET)
	POP	AF		; neue Fensternummer
	PUSH	DE		; neue Fenstergroesse
	PUSH	HL		; neuer Fensteranfang
	CALL	WCOPY		; aktuelles Fenster retten
	POP	HL		; Fensteranfang
	POP	DE		; Fenstergrosse
	LD	(WINON),HL	; Fensteranfang setzen
	LD	(WINLG),DE	; Fenstergroesse setzen
	LD	(CURSO),BC	; BC=0
	RET
;_____________________________________________________________
;							**3D**
; PE:	A	Fensternummer 0..9
; PA:	CY=1	Fehler
; VR:	AF, BC, DE, HL
;
WINAK0:	LD	A,L		; Fensternummer
WINAK:	CALL	WCOPY		; aktuelles Fenster retten
	; hier Einsprung fuer BASIC (PRINT AT)
WINAKB:	CALL	WINADR		; Adresse Vektorspeicher in DE
	RET	C		; Nr. ausserhalb
	INC	HL		; HL=WINON
	EX	DE,HL
	JR	WCPY1		; 10 Byte kopieren

; Testet Fenster-Nr. und berechnet Position in Fenstervektorspeicher
; PE:	WINNR	Fensternummer 0-9
; PA:	CY=1	Nr. zu gross
;	HL	WINNR
;	DE	Adresse im Fenstervektorspeicher
WINADR:	LD	HL,WINNR
	LD	A,(HL)		; aktuelle Fensternummer
	ADD	A,-10
	RET	C		; Nr. zu gross
	ADD	A,A	; *2
	LD	E,A
	ADD	A,A	; *4
	ADD	A,A	; *8
	ADD	A,E	; *10
	LD	E,A
	LD	D,High(WNDFN)	; Adresse im Fenstervektorspeicher
	AND	A		; CY=0
	RET

; Parameter testen fuer Fensterinitialisierung:
PART:	ADD	A,-10
	RET	C		; nur 10 Fenster moeglich -> ERROR
	LD	A,D		; Zeilen
	SUB	1
	RET	C		; Zeilenanzahl=0 -> ERROR
	ADD	A,H
	RET	C		; Anzahl+Anfang>256 -> ERROR
	ADD	A,-32
	RET	C		; Anzahl+Anfang>32 -> ERROR
	LD	A,E		; Spalten
	SUB	1
	RET	C		; Spaltenanzahl=0 -> ERROR
	ADD	A,L
	RET	C
	ADD	A,-40		; Anzahl+Anfang>40 -> ERROR
	RET

; aktuelles Fenster (WINNR) in Fenstervektorspeicher retten
; PE:	A	Fensternummer 0..9
; PA:	CY=1	Nr. zu gross
;	BC=0
; VR:	F, BC, DE, HL

WCOPY:	PUSH	AF
	CALL	WINADR		; Adresse Vektorspeicher in DE
	JR	C,IAD21		; Fehler
	POP	AF
	LD	(HL),A		; neue Fensternummer eintragen
	INC	HL		; WINON
WCPY1:	LD	BC,10		; ab Fensteranfang 10 Byte retten
; JPLD mitgenutzt von Menuewort USER
JPLD:	LDIR
	OR	A
	RET

; Belegung einer F-Taste praesentieren:
; PE:	C	Nummer der Taste
;	HL	Zeiger in F-Tastenspeicher

KEYDI:	CALL	CLLN		; Zeile loeschen
	LD	A,C
	CALL	AHOSTR		; Fn
	DB	' :',0
KEYD1:	LD	A,(HL)		; Code
	OR	A		; Ende?
	RET	Z		; fertig
	CALL	CSTBT		; Zeichen darstellen
	INC	L
;	LD	A,L
;	CP	9CH		; Speicherende?
;	RET	NC		; Abbruch
	JR	KEYD1		; naechstes Zeichen

;_____________________________________________________________
;
; F-Taste belegen bzw. Belegung auflisten
;
	DW	7F7FH
	DB	'KEY',1
	;
	JR	Z,KEYLI		; ohne ARG, KEYLIST
	LD	A,L		; Nummer
;_____________________________________________________________
;							**39**
KEY:	LD	HL,FTASTE	; 0B900H
	AND	A
	JR	Z,KKILL		; KEY 0
	CP	15+1		; Nummer
	RET	NC		; nur F1 bis F15 zugelassen
	LD	B,A
	OR	0F0H		; Nr. fuer Anzeige bei KEYDI
	LD	C,A		; in C bereitstellen
KEY1:	LD	A,(HL)		; B Dummys auszaehlen
	INC	HL
	AND	A		; Dummy?
	JR	NZ,KEY1
	DJNZ	KEY1
	CALL	KEYDI		; Belegung praesentieren
KEY2:	PUSH	AF
	DEC	HL
KEY3:	POP	AF
KEY4:	CALL	INTB		; Eingabe
	CP	13H		; Stop?
	JP	Z,CRLF		; fertig
	PUSH	AF
	LD	B,0
	DEC	A		; CLR?
	JR	NZ,KEY5
	LD	A,(HL)		; leere Belegung
	AND	A
	JR	Z,KEY3		; nicht loeschen
	LD	D,H
	LD	E,L		; DE=Ziel
	PUSH	HL
	INC	HL		; HL=Quelle
	LD	A,9CH
	SUB	L		; Laenge, B=0
	LD	C,A
	LDIR			; Pufferrest vorziehen
	POP	HL
	POP	AF		; Zeichen
	CALL	OCHR		; anzeigen
	JR	KEY2
	;
KEY5:	LD	A,(FTASTE+9AH)	; vorletztes Zeichen
	AND	A		; Trennzeichen?
	JR	NZ,KEY3		; nein, voll
	PUSH	HL
	LD	A,9BH
	SUB	L
	LD	C,A		; Laenge, B=0
	LD	HL,FTASTE+9AH
	LD	DE,FTASTE+9BH
	LDDR			; Pufferrest hinterher ziehen
	POP	HL
	POP	AF
	INC	HL
	LD	(HL),A		; Zeichen eintragen
	CALL	CSTBT		; & anzeigen
	JR	KEY4

; KEY 0 loescht alle F-Tasten
KKILL:	LD	B,09CH		; Groesse F-Tastenspeicher
KKIL1:	LD	(HL),A		; loeschen
	INC	HL
	DJNZ	KKIL1
	RET
;_____________________________________________________________
;							**3A**
; Keylist - Anzeige belegter F-Tasten:

KEYLI:	LD	HL,FTASTE+1	; B900 ist immer 0
	LD	BC,01F1H	; B=1 (Zaehler), C=F1 Tasten-Nr.
KEYL1:	LD	A,(HL)
	AND	A		; Taste belegt?
	JR	Z,KEYL2
	CALL	KEYDI		; Belegung praesentieren
	CALL	CRLF		; neue Zeile
	INC	B		; Zaehler
KEYL2:	INC	L		; Adresse +1
	INC	C		; Taste +1
	JR	NZ,KEYL1	; F1 bis FF
KEYL3:	DEC	B
	RET	NZ		; F-Tasten sind belegt
	CALL	OSTR
	DB	'F-Tasten leer'
	DB	CR,LF,0
	RET
;_____________________________________________________________
;							**25**
; Cursor komplementieren
; VR:	-

CUCP:	PUSH	HL
	PUSH	DE
	PUSH	AF
	LD	DE,(CURSO)
	CALL	DABR		; VRAM-Adresse berechnen
	JR	C,CEND		; ausserhalb
	LD	A,(HL)		; Zeichencode auf Cursorposition
	CALL	PADR1		; Pixel- und Farbadresse berechnen
	OR	A		; Zeichencode 0?
	JR	NZ,CFARB	; Farbcursor
	SET	5,L		; untere Zeichenhaelfte
	SET	0,H		; vorletzte Pixelzeile
	LD	A,(HL)
	XOR	7FH		; Strichcursor in Pixelebene
	LD	(HL),A
	LD	A,80H		; nur Blinken ein/aus
	DB	21H	; LD HL,nn
; Farbcursor, wenn Zeichen auf Cursorposition
CFARB:	LD	A,0FFH		; Farbe komplett negieren (inkl. Blinkbit)
	LD	L,A		; Maske fuer XOR
	LD	A,(DE)
	XOR	L		; 1. Farbbyte
	LD	(DE),A
	SET	5,E
	LD	A,(DE)
	XOR	L		; 2. Farbbyte
	BIT	7,(IX+8)	; CAPS?
	JR	NZ,CFAB1
	XOR	4		; 2farbig bei CAPS
CFAB1:	LD	(DE),A
CEND:	POP	AF
	JR	PUDEND
 
;*** Grafiktreiber ***
 
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
PUDE:	DB	3EH	; LD A,0AFH	A=AFH
	;
;_____________________________________________________________
;							**30**
; Punkt setzen
; PE:	(HOR)	Horizontalkoordinate
;	(VERT)	Vertikalkoordinate
;	(FARB)	Bildpunktfarbe
; PA:	CY=1	Punkt auáerhalb (Fehler)
; VR:	AF
;
PUSE:	XOR	A		; A=0
	PUSH	HL
	PUSH	DE
	LD	HL,(HOR)	; X-Koordinate (Horizontalposition)
	LD	DE,(VERT)	; Y-Koordinate (Vertikalposition)
	OR	A
	JR	NZ,PUDEJP	; Punkt loeschen
; Punkt setzen:
	CALL	WTESTV		; Test der X-Koordinate
	CALL	NC,POINT	; setzen, wenn nicht ausserhalb
PUDEND:	POP	DE
	POP	HL
	RET

; Punkt loeschen:
PUDEJP:	CALL	WTESTV		; Test der X-Koordinate
	JR	C,PUDEND	; ausserhalb
	PUSH	BC
	CALL	BADR		; Grafikposition und Bitmaske berechnen
	LD	B,A		; Bitmaske
	LD	C,(HL)		; Inhalt Pixel-RAM
	CPL			; Bitmaske negiert
	AND	C		; Pixel loeschen
	LD	(HL),A		; und zurueck schreiben
	LD	A,B		; Bitmaske
	AND	C		; war Bit gesetzt?
	LD	A,(DE)		; Farbwert zurueck geben
	POP	BC
	JR	PUDEND
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

LINE:	;EXX
	;PUSH	HL
	;PUSH	DE
	;PUSH	BC
	 LD	DE,(ARG4)	; DE' = Y Ende
	 LD	HL,(ARG2)	; HL' = Y Anfang
	EXX
	CALL	LARG		; HL = X Anfang
	LD	D,B		; DE = X Ende
	LD	E,C
LINE1:	XOR	A		; A=0
	SBC	HL,DE		; HL = XSTEPS
	JR	NC,LINE2	; vorwarts
; rueckwaerts:
	ADD	HL,DE		; Korrektur
	EX	DE,HL		; X-Richtung tauschen
	EXX
	 EX	DE,HL		; Y-Richtung tauschen
	EXX
	JR	LINE1		; Linie jetzt vorwaerts zeichnen

; vorwaerts:
LINE2:	EXX
	 PUSH	DE		; Y Ende
LINE3:	 SBC	HL,DE		; HL' = YSTEPS
	 JR	NC,LINE4	; fallend
	 ADD	HL,DE
	 EX	DE,HL		; Y-Richtung tauschen
	 OR	1		; steigend
	 JR	LINE3
	;
LINE4:	 PUSH	HL		; YSTEPS
	EXX
	POP	BC		; BC = YSTEPS
	PUSH	HL		; XSTEPS
	SBC	HL,BC		; flach oder steil?
	POP	HL		; XSTEPS
	PUSH	BC		; YSTEPS
	JR	NC,LINE5	; flach
	EX	(SP),HL
	OR	2		; flach
LINE5:	PUSH	HL		; HL = MAX(STEPS)
	LD	B,H
	LD	C,L		; Pixelzaehler
	EXX
	 POP	BC		; BC'=MAX(...)
	 POP	DE		; DE'=MIN(...)
	 LD	H,B
	 LD	L,C
	 SRL	H		; Slave mit 1/2
	 RR	L		; HL=1/2*MAX(...)
	EXX
	POP	HL
	EX	DE,HL
	INC	BC

; Hauptschleife
; AF Bitregister
; HL lfd. X-Koordinate	HL' Slave
; DE lfd. Y-Koordinate	DE' -Master
; BC Punktezaehler	BC' +Master

LINE6:	CALL	SHLDE		; Punkt setzen
	EXX
	 AND	A
	 SBC	HL,DE
	 JR	NC,LINE7
	 ADD	HL,BC
LINE7:	EXX
	BIT	1,A		; flach?
	JR	NZ,VDE
	JR	NC,IHL
VDE:	BIT	0,A		; steigend?
	INC	DE
	JR	Z,IDE
	DEC	DE
	DEC	DE
IDE:	JR	C,IHL
	DEC	HL
IHL:	CPI
	JP	PE,LINE6
	;
CIREND:	OR	A	; CY=0 (damit BASIC keine Fehler bringt)
	;POP	BC
	;POP	DE
	;POP	HL
	;EXX
	RET
;_____________________________________________________________
;							**3F**
; Kreis zeichnen
; PE:	(ARG1)	X-Koordinate Mittelpunkt
;	(ARG2)	Y-Koordinate Mittelpunkt
;	(ARG3)	Radius, max. 255
;	(FARB)	Farbe des Kreises
; PA:	-
; VR:	AF, BC, DE, HL, BC', DE', HL'

CIRCLE:	;EXX
	;PUSH	HL
	;PUSH	DE
	;PUSH	BC
	LD	A,(ARG3)	; Radius R
	LD	C,A
	LD	D,C
	CALL	MULT		; Achtelkreis berechnen (Ergebnis in BA)
	LD	H,B
	LD	L,A		; HL = R*R
	LD	D,H
	LD	E,L		; DE = HL
	LD	A,C
	LD	B,0		; BC=Radius
	SLA	C
	RL	B		; BC=Durchmesser (Radius*2)
	DEC	BC
	PUSH	BC
	LD	BC,1
	EXX
	 LD	C,A
	 LD	B,0
CIR1:	 CALL	CIRUP1		; 4 Punkte um Mittelpunkt setzen
	 CALL	CIRUP1		; und noch 4 Punkte
	EXX
	AND	A
	SBC	HL,BC
	INC	BC
	INC	BC
	SBC	HL,DE
	ADD	HL,DE
	EXX
	 JR	NC,CIR2
	EXX
	EX	(SP),HL
	EX	DE,HL
	AND	A
	SBC	HL,DE
	DEC	DE
	DEC	DE
	EX	DE,HL
	EX	(SP),HL
	EXX
	 DEC	C
CIR2:	 INC	B
	 LD	A,C
	 CP	B
	 JR	NC,CIR1
	 POP	BC
	 JR	CIREND

; setzt 4 Punkte um Mittelpunkt

CIRUP1:	LD	A,B
	LD	B,C
	LD	C,A
	XOR	A
	LD	D,A
	LD	HL,(ARG2)	; Y-Mittelpunkt
	PUSH	HL
	LD	E,C
	ADD	HL,DE		; oberhalb vom Mittelpunkt
	CALL	CIRUP2		; 2 Punkte setzen
	XOR	A	; CY=0
	LD	D,A
	POP	HL
	LD	E,C
	SBC	HL,DE		; unterhalb vom Mittelpunkt
				; und reinlaufen
CIRUP2:	PUSH	HL		; Y-Koordinate
	LD	HL,(ARG1)	; X-Mittelpunkt
	LD	E,B
	AND	A	; CY=0
	SBC	HL,DE		; links vom Mittelpunkt
	POP	DE		; Y-Koordinate
	CALL	SHLDE		; Punkt setzen
	PUSH	DE
	LD	D,A
	LD	E,B
	ADD	HL,DE
	ADD	HL,DE		; rechts vom Mittelpunkt
	POP	DE		; und reinlaufen
	;
SHLDE:	PUSH	AF
	CALL	WTEST		; Test der Koordinaten
	PUSH	HL
	PUSH	DE
	CALL	NC,POINT	; wenn nicht ausserhalb: Punkt setzen auf HL/DE
	POP	DE
	POP	HL
	POP	AF
	RET

; universelle Punkt(re)set-Routine
; PE:	HL	X-Koordinate
;	DE	Y-Koordinate
;	(FARB)	Bit 3...7 : Farbe
;		Bit 2=1   : Farbebene nicht aendern
;		Bit 1=1   : PRESET
;		Bit 0=1   : PXOR
; VR:	AF, DE, HL

POINT:	CALL	BADR		; Grafikposition und Bitmaske berechnen
	PUSH	DE		; Adresse Color-RAM merken
	LD	DE,(FARB)
	BIT	1,E
	JR	NZ,NOTPT	; Punkt loeschen
	BIT	0,E
	JR	NZ,XORPT	; Punkt invertieren

; Punkt setzen:
	OR	(HL)		; Bit setzen
XORPT1:	LD	(HL),A		; in Pixel-RAM eintragen
	BIT	2,E		; Farbebene veraendern?
	LD	A,E		; A=Farbcode
	POP	DE		; Color-RAM
	RET	NZ		; Farbe so belassen
	AND	0F8H		; nur Vordergrund
	LD	H,A
	LD	A,(DE)		; Color-RAM lesen
	AND	7		; alte vFarbe loeschen
	OR	H		; neue vFarbe einbauen
	LD	(DE),A		; Color-RAM schreiben
	RET

; Punkt loeschen
NOTPT:	CPL			; Bitmaske negieren
	AND	(HL)		; Punkt loeschen
	LD	(HL),A		; in Pixel-RAM schreiben
	POP	DE
	RET

; Punkt invertieren
XORPT:	XOR	(HL)		; alternierend
	JR	XORPT1

; Grafikposition und Bitmaske berechnen
; PE:	HL	Horizontalposition (X)
;	DE	Vertikal-  "       (Y)
; PA:	HL	Pixeladresse
;	DE	Farbadresse
;	A	Bit-Maske
;	CY	Fehler
; VR:	AF, DE, HL
;
BADR:	LD	A,L		; A=Low(X-Position)
	LD	D,E		; D=Low(Y-Position)
	AND	7		; A=Bitposition
	ADD	HL,HL		; *2
	ADD	HL,HL		; *4
	ADD	HL,HL		; *8
	ADD	HL,HL		; *16
	ADD	HL,HL		; *32
	LD	L,H		; Horizontalposition 0..39
	ADD	A,Low(BITTAB)
	LD	E,A
	LD	A,D		; Low(Y-Position)
	CPL			; negieren
	LD	H,A		; Vertikalposition 0..255
	LD	D,High(BITTAB)	; Bittabelle
	LD	A,(DE)		; Bitmaske
	JP	PADR		; Pixel- und Farbadresse berechnen

; Test der Grafikkoordinaten
; PE:	HL	X-Koordinate (0..255)
;	DE	Y-Koordinate (0..320)
; PA:	CY=1	ausserhalb

WTEST:	LD	A,D		; Y-Koordinate
	ADD	0FFH
	RET	C		; D groesser als 0
WTESTV:	LD	A,H		; X-Koordinate
	OR	A
	RET	Z		; H=0: OK
	ADD	0FEH
	RET	C		; H groesser als 1
	LD	A,L
	ADD	0C0H		; HL groesser als 319
	RET
;_____________________________________________________________
;							**38**
; Byteweise MB-Ausgabe:
; PE:	A	Datenbyte
;	D	Steuerbyte
;		Bit 3 = 1 INIT (Block 01)
;		Bit 6 = 1 Close (Block FF)
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
;	Bit 5,(IX+7) Blocknummernanzeige unterdruecken?
; PA:	CY=1	Fehler
; VR:	AF, DE, HL
;
MBOUT:	LD	E,D		; Steuerbyte
	LD	D,A		; Datenbyte
	PUSH	DE
	PUSH	BC
	BIT	3,E		; Init?
	JR	NZ,INITO
	LD	A,(DTPTR)	; Zeiger im Kassettenpuffer
INITO2:	LD	H,High(CASS)	; Kassettenpuffer
	LD	L,A		; aktuelle Position
	LD	(HL),D		; Datenbyte eintragen
	INC	A		; Zeiger auf naechste Position
	BIT	6,E		; Close?
	JR	NZ,CLOSEO	; letzten Block ausgeben
	AND	7FH
	LD	(DTPTR),A	; Zeiger neu setzen
	JR	NZ,POPS		; POP BC, DE, RES 3,E

; Puffer ausgeben:
	BIT	1,(IX+7)	; steht INIT noch aus?
	JR	Z,KINITO	; nein
	CALL	NBWI		; sonst erst einmal den Vorblock ausgeben
	RES	1,(IX+7)	; INIT ruecksetzen
	JR	POPS		; POP BC, DE, RES 3,E
 
KINITO:	CALL	NBW		; Block auf Kassette ausgeben
	JR	NC,POPS		; POP BC, DE, RES 3,E
; BRK oder Fehler:
IOERRO:	CALL	CRO0		; Bandein-/ausgabe beenden
JIOERR:	LD	HL,(IOERR)	; B7C9H
	CALL	IRMOF
	JP	(HL)		; BASIC: I/O ERROR

; CLOSE Bandausgabe:
CLOSEO:	BIT	1,(IX+7)	; steht noch INIT aus?
	PUSH	AF
	CALL	NZ,NBWI		; zunaechst Vorblock ausgeben
	POP	AF
	CALL	Z,NBW		; sonst Datenblock auf Kassette ausgeben
	SCF
	CALL	NBW1		; Endeblock ausgeben
POPS:	POP	BC
	POP	DE
	RES	3,E		; INIT ruecksetzen
	RET

; INIT Bandausgabe:
INITO:	SET	1,(IX+7)	; INIT merken
	LD	A,(HL)
	CP	'U'+80H		; List-Ausgabe?
	JR	NZ,INITO1
	SET	5,(IX+7)	; Blocknummern unterdruecken
				; und langer Vorton
INITO1:	PUSH	DE
	LD	DE,CASS		; Anfang Kassettenpuffer
	LD	BC,11		; Laenge Dateiname + Typ
	LDIR			; mit Name fuellen
	LD	A,E		; A jetzt Zeiger in Kassettenpuffer
	POP	DE
	JR	INITO2

; BRK/Fehler bei Bandeingabe:
IOERRI:	CALL	CMBI		; Abschluss Bandeingabe
	JR	JIOERR

; Byteweise Magnetband-Eingabe				**37**

; Zunaechst Daten aus Puffer entnehmen und nur wenn Puffer leer ist in die
; MBIN-Routine(n) des eingestellten Treibers springen

; PE:	D	Steuerbyte
;		Bit 3 = 1 INIT (Block 01)
;		Bit 6 = 1 Close (Block FF)
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
; PA:	A	Datenbyte
;	CY=1	Fehler (auch falscher Dateiname bei TAPE)
; VR:	AF, DE, HL
;
MBIN:	LD	E,D		; Steuerbyte
	PUSH	DE
	PUSH	BC
	BIT	6,E		; Close?
	JR	NZ,CLOSI	; ja, Bandeingabe stoppen
	BIT	3,E		; Init?
	JR	NZ,INII		; ja
	LD	A,(DTPTR)	; Zeiger
	OR	A		; Puffer leer?
	LD	L,A
	CALL	Z,NBR		; neuen Block einlesen
	JR	C,IOERRI	; BRK oder Fehler
; Datenbyte aus Puffer holen:
BYTHOL:	LD	H,High(CASS)
	LD	D,(HL)		; Datenbyte aus Puffer
	LD	A,L
	INC	A		; naechste Position
	AND	7FH
	LD	(DTPTR),A	; Zeiger neu setzen
	LD	A,D		; Datenbyte zurueck geben
POPS1:	JR	POPS		; POP BC, DE, RES 3,E

; Init Bandeingabe
INII:	RES	1,(IX+7)	; INIT ruecksetzen
	CALL	IMBII		; Blocknummer 1 erwarten, MB-Eingabe init.
	CALL	NBR		; neuen Block einlesen
	JR	C,IOERRI	; BRK oder Fehler
	LD	DE,CASS		; Daten stehen jetzt im Kassettenpuffer
	LD	B,11		; Laenge des Dateinamen
	LD	A,(DE)		; vorangestellter Dateityp: 1. Zeichen testen
	CP	'U'+80H		; hohes 'U'?
	JR	Z,INII1
	RES	5,(IX+7)	; Blocknummern anzeigen (langer Vorton)
INII1:	ADD	29H		; geschuetztes Programm?
	JR	NC,INII2
	LD	A,(DE)		; geschuetzte BASIC-Programme
	SUB	4		; als ungeschuetzt anzeigen
	LD	(DE),A		; 1. Zeichen vom Dateityp
	INC	DE
	LD	(DE),A		; 2. Zeichen vom Dateityp
	INC	DE
	LD	(DE),A		; 3. Zeichen vom Dateityp
	DEC	DE
	DEC	DE
;	LD	(DATFLG),A	; <=hier LIST/EDIT/BYE-Schutz setzen
INII2:	LD	A,(DE)		; Name aus Kassettenpuffer
	CALL	CRT		; anzeigen
	CP	(HL)		; stimmt Name?
	JR	Z,INII3
	SET	1,(IX+7)	; nein-merken
INII3:	INC	DE
	INC	HL
	DJNZ	INII2		; fuer alle 11 Zeichen wiederholen
	CALL	CRLF		; neue Zeile
	BIT	1,(IX+7)	; war der Name korrekt?
	RES	1,(IX+7)
	JR	NZ,IOERRI	; nein, I/O-Error
	LD	L,E		; Pufferzeiger in L
	JR	BYTHOL		; erstes Datenbyte holen

; Close Bandeingabe
CLOSI:	CALL	CMBI		; Abschluss Bandeingabe
	JR	POPS1		; POP BC, DE, RES 3,E


ZKOUT0:	CALL	OCHR
;_____________________________________________________________
;							**45**
; Anzeige einer Zeichenkette
; PE:	HL	Anfang der Zeichenkette
; PA:	HL	Ende der Zeichenkette+1 (nach 0-Byte)

ZKOUT:	LD	A,(HL)		; Zeichen aus Speicher
	INC	HL		; naechste Adresse
	AND	A		; Ende-0?
	JR	NZ,ZKOUT0	; nein, anzeigen
	RET
;_____________________________________________________________
;							**40**
; Berechnen der Quadratwurzel
; PE:	HL	16 Bit-Zahl (vorzeichenlos)
; PA:	A	Ergebnis 8 Bit
; VR:	AF, HL, DE

SQR:	XOR	A		; A=0, CY=0
	DEC	A		; A=FF
	LD	E,A
	LD	D,A		; DE=FFFF
SQR1:	INC	A		; mitzaehlen
	INC	DE
	INC	DE
	SBC	HL,DE
	RET	M
	JR	SQR1
;_____________________________________________________________
;							**41**
; Berechnung des Produktes zweier 8-Bit-Zahlen: BA=C*D
; PE:	D, C	Faktoren (8 Bit)
; PA:	BA	Produkt (16 Bit)
; VR:	AF, DE, B

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
 
; Module schalten fuer JUMP-Einsprung
MODSW:	LD	DE,UMODL	; IO-Modul (V.24 oder Centronics)
	CALL	MOSW
	LD	(HCPZ),A	; 1, wenn IO-Modul vorhanden, sonst 0
; Module schalten fuer PWRON
MODSW1:	LD	DE,43F6H	; 64K-Modul M011
	CALL	MOSW
	LD	E,0F4H		; 16K-Modul M022
	OR	A
	CALL	Z,MOSW		; M022 nur, wenn kein M011 vorhanden ist
	LD	DE,0C1FDH	; M052 (USB-Ebene)
; Modul suchen und schalten
; PE:	E=Modulkennung
;	D=Sollzustand
; PA:	A=1 wenn gefunden
;	A=0 nicht gefunden
MOSW:	LD	BC,0680H	; ab Steckplatz 7
MOSW1:	XOR	A
	INC	B		; Steckplatz+1
	RET	Z		; alles durchsucht
	IN	A,(C)
	CP	E		; gesuchte Modulkennung?
	JR	NZ,MOSW1
	LD	A,1
MOSW2:	LD	H,High(MODST)	; Steuerbytetabelle im IRM
	LD	L,B		; Steckplatz
	OUT	(C),D		; Modul schalten und
	LD	(HL),D		; in Tabelle eintragen
	RET
 
;*** Druckertreiber (V.24 bzw. CEN) ***

; Senden einer Bytefolge an den Drucker:
; dem CALL folgt ein Byte Laenge und dann die Datenbytes
PRZK:	;folgt TAB, 1.Byte Laenge
	EX	(SP),HL
	LD	B,(HL)		; Laenge
	INC	HL
PRZK1:	LD	A,(HL)		; Datenbyte
	INC	HL
	CALL	PRINT		; zum Drucker schicken
	DJNZ	PRZK1		; bis Anzahl abgearbeitet ist
	EX	(SP),HL
	RET

; Protokollfunktion (Drucker-Echo)
ECHO:	PUSH	AF
	PUSH	HL
	CALL	CRT		; Zeichen auf Bildschirm anzeigen
	LD	HL,STBT
	BIT	4,(HL)		; ESC-Folge aktiv?
	JR	NZ,ECH4		; bei ESC keine Druckerausgabe!
	BIT	3,(HL)		; Steuercodes darstellen?
	JR	Z,ECH2		; nein
	AND	7FH		; Bit 7 abschneiden
	CP	20H
	JR	C,ECH1		; Steuerzeichen als '_' ausgeben
	CP	7FH		; 7FH ebenfalls
	JR	NZ,ECH4		; alles andere zum Drucker schicken
ECH1:	LD	A,'_'		; Ersatz-Zeichen
	JR	ECH3
 
ECH2:	CP	9		; CUR
	JR	NZ,ECH3
	LD	A,' '		; Leerzeichen statt CUR
ECH3:	CP	0FH		; SH+CLR
ECH4:	POP	HL
	JR	Z,PRT		; POP+RET, falls nicht zu drucken
	DB	0EDH		; NOP-BYTE
	;
PRINT:	PUSH	AF
IF V24
	NOP			; Freihalte-Bytes
	NOP			; fuer CENTR.Version
WT:	CALL	BRKT
	JR	C,PRT		; Abbruch mit BRK
	IN	A,(SIO+2)
	BIT	2,A		; bereit?
	JR	Z,WT		; nein, warten
	POP	AF
	OUT	(SIO),A		; Datenbyte senden
	RET
	NEG			; Fuell-Bytes?
	NEG			; Fuell-Bytes?
ELSE
	OUT	(PIO),A		; M001 PIO Port A (Daten)
WT:	CALL	BRKT		; Abbruch mit BRK
	JR	C,PRT
	IN	A,(PIO+1)	; M001 PIO Port B (Steuersignale)
	BIT	2,A		; Centr. BUSY?
	JR	NZ,WT		; warten, bis bereit
	RES	0,A
	OUT	(PIO+1),A	; /STROBE-Impuls PIO-B
	SET	0,A
	OUT	(PIO+1),A
ENDIF
PRT:	POP	AF
	RET

; ShCLR-Reaktionsprogramm

SHCLX:	LD	A,(HCPZ)	; Steuerbyte Druckerausgabe
	DEC	A		; HCPZ = 1?
	JR	NZ,HCPX		; nein

; Protokollfunktion ein/aus bei HCPZ=1 (Voreinstellung)

	LD	HL,(OUTAB)	; aktuelle Ausgaberoutine
	LD	A,(HL)
	OR	A		; CRT?
	JR	Z,SHX1		; OUTAB ist aktuell auf CRT-Ausgabe eingestellt
; Protokoll aus:
	LD	HL,PRINT	; Anwenderkanal 1 = Druckerausgabe
	LD	BC,ZEI0		; OUTAB = Bildschirmausgabe
	JR	SHX2
; Protokoll ein:
SHX1:	LD	HL,ECHO		; Anwenderkanal 1 = Echo-Routine (CRT + Drucker)
	LD	BC,ZEI2		; OUTAB = Anwenderkanal 1
SHX2:	LD	(UOUT1+1),HL
	LD	(OUTAB),BC
	RET

; ESC-6: Hardcopy auf Drucker

ESC6:	LD	A,(HCPZ)	; Steuerbyte Druckerausgabe
	OR	A
	RET	Z		; kein IO-Modul
	JR	HCPX0
	;
HCPX:	CP	2		; HCPZ = 2?
	RET	NC		; nein
HCPX0:	PUSH	DE
	LD	HL,0		; Pixelzeile 0, Spalte 0
HCPX1:	CALL	PRZK		; Daten an Drucker senden
	DB	9	; 9 Byte (Initialisierung K6313 und kompatible)
	DB	ESC,'J',24	; Vorschub 1/9"
	DB	CR
	DB	ESC,'*',5	; Plottergrafik
	DW	320
HCPX2:	PUSH	HL		; Cursorposition
	CALL	PADR		; Pixeladresse berechnen
	CALL	RDX		; liest Zeichen nach ARG7-ARG10
	LD	C,8		; 8 Druckspalten
HCPX3:	LD	HL,ARG7		; Pixelcodes in Folge
	LD	B,8		; 8 Nadeln
HCPX4:	RL	(HL)		; Bit 7 nach CY
	RLA			; CY in A einschieben
	INC	L		; naechste Pixelzeile
	DJNZ	HCPX4		; bis 8 Bit gesammelt sind
	LD	L,Low(STBT)
	BIT	2,(HL)		; invers?
	JR	Z,HCPX5
	CPL			; negieren
HCPX5:	CALL	PRINT		; zum Drucker schicken
	DEC	C
	JR	NZ,HCPX3	; naechstes Druckspalte
	POP	HL		; Position
	INC	L		; Spalte +1
	LD	A,L
	CP	40		; letzte Spalte?
	JR	NZ,HCPX2	; nein
	LD	L,0		; Spalte 0
	LD	A,H
	ADD	A,8		; Pixelzeile +8
	LD	H,A
	JR	NC,HCPX1	; naechste Zeile drucken
	CALL	PRZK		; Daten an Drucker senden
	DB	2		; 2 Byte
	DB	CR,LF		; neue Zeile
	POP	DE
	RET
 
RDX:	;liest Zeichen nach ARG7-ARG10
	LD	DE,ARG7		; Ablage hier
	LD	BC,80H		; Offset zur naechsten Pixelzeile
	PUSH	HL
	CALL	RDX1		; 4 Pixel lesen
	POP	HL
	SET	5,L		; zweiter Teil
RDX1:	LD	A,4		; 4 Byte lesen
RDX2:	LDI
	ADD	HL,BC
	INC	BC		; BC korrigieren nach LDI
	DEC	A
	JR	NZ,RDX2
	RET

; Druckermode auswaehlen (LSDTDEV ?)
IF V24
	DW	7F7FH		; Menuewort V24
	DB	'V24',1		; als Parameter 0,1 oder 2 eingeben
ELSE
	DW	7F7FH		; Menuerwort CEN
	DB	'CEN',1		; als Parameter 0,1 oder 2 eingeben
ENDIF
	LD	A,L		; ARG1
	LD	(HCPZ),A	; Steuerbyte Druckerausgabe einstellen
	RET

; Daten zum Drucker senden (koenne auch PRINT heissen ???)

	DW	7F7FH		; Menuewort SEND
	DB	'SEND',1
	;
	RET	Z		; kein Argument eingegeben
	LD	B,A		; Anzahl Argumente
	LD	HL,ARG1		; Daten aus den ARG
SND1:	LD	A,(HL)		; Low(ARG)
	INC	HL
	INC	HL
	CALL	PRINT		; zum Drucker senden
	DJNZ	SND1
	RET
IF SEEK
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Programm auf Adresse starten
; Aufruf: USER Adresse
;
;	oder
;
; Speicherbereich kopieren
; Aufruf: USER Quelle Ziel Anzahl
;
	DW	7F7FH
	DB	'USER',1
	CP	3		; 3 Argumente?
	JP	Z,JPLD		; ja, COPY aufrufen
	DEC	A		; 1 Argument?
	JP	NZ,ERRM		; nein, alles andere erzeugt Fehlermeldung
	JP	(HL)		; Programm starten
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Bytefolge im Speicherbereich suchen
; Aufruf: SEEK Anfangsaresse Endadresse+1 Byte [Byte, Byte ...]
;
	DW	7F7FH
	DB	'SEEK',1
	CP	3		; mindestens 3 Argumente?
	RET	C		; nein, nichts tun
	EX	DE,HL
	SBC	HL,DE
	LD	B,H
	LD	C,L		; BC = Laenge Suchbereich
	EX	DE,HL		; HL = Anfangsadresse
SCHL:	LD	DE,ARG3		; Suchbyte(s)
	LD	A,(DE)
	CPIR			; erstes Byte suchen
	JP	PO,CRLF		; ausgesucht
	PUSH	BC
	PUSH	HL
	LD	A,(ARGN)	; Anzahl Argumente
	SUB	3
	LD	B,A		; restliche Anzahl
	JR	Z,GEF		; nur ein Byte
NBYT:	INC	DE
	INC	DE		; ARG+1
	LD	A,(DE)
	CP	(HL)		; stimmt das naechste Byte auch noch?
	INC	HL		; Adresse+1
	JR	NZ,NGEF		; nein, passt nicht
	DJNZ	NBYT
GEF:	POP	HL
	POP	BC
	DEC	HL
	CALL	HLHX		; Adresse anzeigen
	INC	HL
	CALL	BRKT		; BRK-Taste?
	JP	C,CRLF		; abgebrochen...
	DB	11H	; LD DE,nn
NGEF:	POP	HL
	POP	BC
	JR	SCHL		; weiter suchen
ELSE
;
; Sprung zu einer Adresse mit Registeruebergabe
; Syntax: %GO ADR [HL [DE [BC [A] ] ] ]
;
	DW	7F7FH
	DB	'GO ',1
	OR	A
	JP	Z,ERRM		; mindestens 1 Argument erforderlich
	XOR	A
	LD	(ARGN),A
	PUSH	BC		; (SP)=(ARG3)
	EX	(SP),HL		; Stack = (ARG1), HL=(ARG3)
	EX	DE,HL		; DE = (ARG3), HL=(ARG2)
	LD	BC,(ARG4)	; BC = (ARG4)
	LD	A,(ARG5)	; A = (ARG5)
	RET			; JMP (ARG1)
ENDIF
;
; Tabelle der ESC-Funktionen:
;
ESCTAB:	DEFW	ASC5	;ESC0	Tabulatorschritt
	DEFW	ESC1	;ESC1	Autostart nach LOAD ein/aus
	DEFW	ESC2	;ESC2	Blinken ein/aus
	DEFW	ESC3	;ESC3	Pixel schreiben EIN/AUS
	DEFW	ESC4	;ESC4	Steuercodes darstellen/ausfuehren
	DEFW	MODUL	;ESC5	Auflisten aller Module
	DEFW	ESC6	;ESC6	Hardcopy auf Drucker
	DEFW	ESC7	;ESC7	Invers EIN/AUS
	DEFW	ASC6	;ESC8	Vordergrund/Hintergrund Farbe tauschen
	DEFW	ESCF	;ESC9	Farbe schreiben EIN/AUS
	DEFW	MOT	;ESCA	Motor-Schaltspannung EIN/AUS
;
; Unterprogrammtabelle:
;
SUTB:	DW	CRT	;00
	DW	MBOT	;01
	DW	UOUT1	;02
	DW	UOUT2	;03
	DW	KBD	;04
	DW	MBI	;05
	DW	UIN1	;06
	DW	UIN2	;07
	DW	ISRO	;08
	DW	CSRO	;09
	DW	ISRI	;0A
	DW	CSRI	;0B
	DW	KBDS	;0C
	DW	BYE	;0D
	DW	KBDZ	;0E
	DW	COLR	;0F
	DW	LOAD	;10

	DW	VERIFY	;11
	DW	LOOP	;12
	DW	NORM	;13
	DW	WAIT	;14
	DW	LARG	;15
	DW	INTB	;16
	DW	INLIN	;17
	DW	RHEX	;18
	DW	ERRM	;19
	DW	HLHX	;1A
	DW	HLDE	;1B
	DW	AHEX	;1C
	DW	ZSUCH	;1D
	DW	SOUT	;1E
	DW	SIN	;1F

	DW	NOUT	;20
	DW	NIN	;21
	DW	GARG	;22
	DW	OSTR	;23
	DW	OCHR	;24
	DW	CUCP	;25
	DW	MODU	;26
	DW	JUMP	;27
	DW	LDMA	;28
	DW	LDAM	;29
	DW	BRKT	;2A
	DW	SPACE	;2B
	DW	CRLF	;2C
	DW	HOME	;2D
	DW	MODI	;2E
	DW	PUDE	;2F
	DW	PUSE	;30

	DW	SIXD	;31
	DW	DABR	;32
	DW	TCIF	;33
	DW	PADR	;34
	DW	TON	;35
	DW	SAVE	;36
	; bis hier in HC900-CAOS, CAOS 2.2 und 2.3 enthalten
	DW	MBIN	;37
	DW	MBOUT	;38
	DW	KEY	;39
	DW	KEYLI	;3A
	DW	DISP	;3B
	DW	WININ	;3C
	DW	WINAK	;3D
	; bis hier in CAOS 2.4 und 2.5 enthalten
	DW	LINE	;3E
	DW	CIRCLE	;3F
	DW	SQR	;40
	DW	MULT	;41
	DW	CSTBT	;42
	; bis hier in HC901-CAOS enthalten
	DW	INIEA	;43
	DW	INIME	;44
	DW	ZKOUT	;45
	DW	MENU	;46
;
; Steuercodetabelle:
;
CRTTAB:	DW	DUMMY	; 00
	DW	CLEAR	; 01
	DW	DELLN	; 02
	DW	DUMMY	; 03=BRK
	DW	DUMMY	; 04=Sh-Brk
	DW	ASC5	; 05=Tab
	DW	ASC6	; 06=Color invertieren
	DW	BEL	; 07
	DW	CUL	; 08
	DW	CUR	; 09
	DW	CUD	; 0A
	DW	CUU	; 0B
	DW	CLS	; 0C
	DW	CRN	; 0D=Enter
	DW	DUMMY	; 0E=Sh-Enter
	DW	HCOPY	; 0F
	DW	HOMEPG	; 10
	DW	PAGE	; 11
	DW	SCROL	; 12
	DW	DUMMY	; 13=STOP
	DW	CLICK	; 14
	DW	DUMMY	; 15
	DW	SHLOCK	; 16=CAPS
	DW	DUMMY	; 17
	DW	CEL	; 18
	DW	CRN	; 9=CCR
	DW	INS	; 1A
	DW	ESCPRG	; 1B
	DW	DUMMY	; 1C=LIST
	DW	DUMMY	; 1D=RUN
	DW	CRNL	; 1E=CONT
	DW	DEL	; 1F
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
	DB	'0@'		;		2A 2B
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
	DB	13H,ESC		; STOP / ESC	4C 4D
	DB	'Mm'		;		4E 4F
	DB	'Zz'		;		50 51
	DB	'Gg'		;		52 53
	DB	'6&'		;		54 55
	DB	1CH,1DH		; LIST / RUN	56 57 Taste nicht vorhanden
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
	DB	16H,16H		; CAPS/SoftRes	72 73
	DB	'1!'		;		74 75
	DB	LF,12H		; CUD		76 77
	DB	0BH,11H		; CUU		78 79
	DB	9,18H		; CUR		7A 7B
	DB	0F1H,0F7H	; F1		7C 7D
	DB	CR,0EH		; ENTER		7E 7F
;
; Interrupttabelle (wird in den RAM kopiert):
;
ISRTAB:	DW	ISRPA   ; PIO A:Kassette	01E4h
	DW	ISRPB   ; PIO B:Tastatur	01E6h
	DW	INTEND  ; CTC 0:frei		01E8h
	DW	ISRC1   ; CTC 1:Kassette	01EAh
	DW	ISRC2   ; CTC 2:Tondauer	01ECh
	DW	ISRC3   ; CTC 3:Tastatur	01EEh

; Modulschaltzustaende (interne Module):			IRM-Adresse

KCTAB3:	DB	3	; RAM0					B800H
	DB	1	; IRM					B801H
	DB	0	; USER-ROMC				B802H
;
; Portinitialisierungstabelle fuer Erstinitialisierung (POWER ON und RESET):
;
; Bei Erstinitialisierung des PIO-Port-A muss sichergestellt werden, dass der
; CAOS-ROM-E immer im Zugriff bleibt. Im Einschaltzustand sind die PIO-Ports
; hochohmig und werden ueber Pull-Up-Widerstaende auf High gezogen, deshalb:
; 1. PIO-Port B initialisieren, und PIO damit aus dem RESET-Zustand holen
;    (so wird auch das Ausgaberegister von Port A beschreibbar)
; 2. Port A Datenwort einschreiben
; 3. Port A Betriebsart Byte-Ausgabe einstellen um das Datenwort zu uebernehmen.
;
KCTAB0:	DB	PIOBS,3		; PIO B Steuerwort
	 DB	0E6H		; Interrupt-Vektor
	 DB	0FH		; Byte-Ausgabe
	 DB	83H		; EI
	;
	DB	PIOBD,1		; PIO-B Daten
	 DB	9FH		; USER-ROM Segment 0, Blinken ein, Lautstaerke 0
	;
	DB	PIOAD,1		; PIO A Daten
	 DB	00011111b	; CAOS-E, RAM0, IRM: on, USER: off
				; /NMI=H! (interner ROM mit CAOS 3.1 aus)
	;
KCTAB1:	; Neu-Initialisierung bei SIXD
	DB	PIOAS,3		; PIO A Steuerwort
	 DB	0E4H		; Interruptvektor
	 DB	0FH		; Byte-Ausgabe
	 DB	3		; DI
	;
	DB	CTC0,1		; CTC 0
	DB	0E8H		; Interrupt-Vektor
	;
	DB	CTC2,2		; CTC 2
	DB	47H		; Zaehler, DI
	DB	0CH		; Blink-ZK
IF V24
	DB	CTC,2		; SIO-CTC: 9600 Bd fuer Drucker
	 DB	47H		; CTC: DI, Zaehler, neg. Flanke, RESET
	 DB	91		; Zeitkonstante
	;
	DB	SIO+2,6		; SIO: 8 Datenbits, 1 Stopbit, keine Paritaet
	 DB	4,4		; WR4: Vorteiler=1, 1 Stop-Bit, ohne Paritaet
	 DB	3,20H		; WR3: Empfaenger aus, CTS+DCD-Steuerung
	 DB	5,6AH		; WR5: Sender ein, 8Bit, /DTR=1, RTS=1
ELSE
	DB	PIO+1,1		; M001 PIO-B Daten
	 DB	1		; /STROBE=1 (passiv)
	;
	DB	PIO+2,2		; M001 PIO-A Steuerwort
	 DB	0FH		; Byte-Ausgabe
	 DB	3		; DI
	;
	DB	PIO+3,3		; M001 PIO-B Steuerwort
	 DB	0CFH		; Bit-Modus
	 DB	0FEH		; E/A-Maske (Bit 0 = Ausgang)
	 DB	3		; DI
ENDIF
;
; IRM-Defaulttabelle:				IRM-Adresse
;
SYSTAB:	DW	SHCLX		; HCADR		B799H
WIN0:	DB	0		; WINNR		B79BH
	DW	0		; WINON		B79CH
	DW	2028H		; WINLG		B79EH
	DW	0		; CURSO		B7A0H
	DB	0		; STBT		B7A2H
	DB	39H		; COLOR		B7A3H
	DW	SCRLPG		; WEND		B7A4H
	DW	LARGE		; CCTL0=EE00H	B7A6H
	DW	SMALL		; CCTL1=FE00H	B7A8H
	DW	LARGE		; CCTL2=EE00H	B7AAH
	DW	SMALL		; CCLT3=FE00H	B7ACH
	DW	STACK		; SYSP=01C4H	B7AEH
	DW	SUTB		; SUTAB		B7B0H
	DW	CRTTAB		; CTAB		B7B2H
	OUT	(PIOAD),A	; JUMP IN NEUES	B7B4H
	JP	0F012H		; CAOS
	DW	ZEI0		; OUTAB=CRT	B7B9H
	DW	ZEI4		; INTAB=KBD	B7BBH
	JP	PRINT		; USER OUT 2	B7BDH
	JP	DUMMY		; USER  IN 2	B7C0H
	JP	DUMMY		; USER OUT 3	B7C3H
	JP	DUMMY		; USER  IN 3	B7C6H
	DW	IOER		; IOERR (BASIC)	B7CBH
STBE1:	;
;	DB	High(BITTAB)	; H-Teil Bittab	B7D8H
	DB	'%'		; Prompt	B7D9H
	DW	160		; Vortonlaenge	B7DAH
	DB	0		; Byte-Zaehler	B7DCH
	DW	ESCTAB		; ESC-Tabelle	B7DDH
	DB	11		; L3SIZ		B7DFH
	DB	5		; COUNT		B7E0H
STBE2:	;
	;
STBL1	EQU	STBE1-SYSTAB
STBL2	EQU	STBE2-STBE1
 
 	ABSFILL	0FDF8h,<ROM-F-Ende>

BITTAB:	DB	80H,40H,20H,10H,8,4,2,1

;	END
