;	CAOS 3.5 - ROM E (interne Version)
;	Version CENTRONICS & V.24
;
;	Date: 11.06.1991  (c) by '#+$'
;
; BUGFIX: CSRLIN eingebaut!
; 06.07.2026: BASIC in USER-ROM Ebene 0 (kompatibel zu CAOS 3.1)
; 10.07.2026: UP Ton korrigiert, aendert Schaltzustand USER-ROM nicht mehr
; 11.07.2026: Tonhoehe BEEP wie bei CAOS 4.x angepasst
; 12.07.2026: Scrollroutine nach CAOS 4.9 optimiert: 1 Byte gespart
;	      Tonhoehe und Dauer Tastencklick wie bei CAOS 4.x angepasst
; 17.07.2026: BYTIN ohne Verwendung von Register E aus CAOS 4.9 uebernommen
;		(4 Byte gespart im ROM-E, Idee von Fran Ludwig 06.12.2017)
; 18.07.2026: KBDS und KBDZ liefern 0, wenn keine Taste gedrueckt wurde, das
;	      spart zusaetzliche Tests auf CY, z.B. bei BRKT
; 18.07.2026: BITOUT nur noch mit Register E (statt D und E) wie bei CAOS 4.7
;		(4 Byte gespart im ROM E)
; 18.07.2026: DELETE optimiert wie bei CAOS 4.5 (1 Byte gespart im ROM-E)
; 18.07.2026: UP WAIT in BASIC PAUSE direkt aufgerufen statt ueber PV5
;		(2 Byte gespart im ROM E)
; 18.07.2026: Aufruf von SIXD2 und SIXD3 innerhalb der TAPE-Routinen ist
;	      problematisch bei Nutzung eines 32K USER-ROM (schaltet immer auf
;	      BASIC-Ebene zurueck)
; 19.07.2026: %WINDOW ohne ARG aktiviert Fenster 0 (aktuelle Groesse)
;	      2x Sprung zu SNERR2 auskommentiert bei WINDOW, wird von WININ mit
;	      geprueft (4 Byte gespart im ROM E)
; 19.07.2026: BASIC Fehlerauswertung bei PSET, PRESET, PTEST, INK und PAPER
;	      jetzt FC-ERROR statt SN-ERROR wie bei CAOS 4.5
; 19.07.2026: BASIC Reihenfolge BCOLOR und PAPER geaendert - spart einen JR und
;	      damit 2 Byte im ROM E
; 19.07.2026: INSTR9 durch 1-Byte-Sprung ersetzt, 1 Byte gespart
; 19.07.2026: BASIC CLOSE von CAOS 4.4-4.9 uebernommen (4 Byte gespart)
; 19.07.2026: Sprung zu GRAPH2 (bei PTEST) von CAOS 4.9 uebernommen (7 Byte gespart im ROM-E)
; 19.07.2026: Reihenfolge der BASIC-Routinen geaendert, um weitere JP zu JR umzuwandeln
; 19.07.2026: UP SUCOL aufgeloest, da nur noch 1x benutzt
; 25.07.2026: SIXD2/SIXD3 umgeschrieben: USER-ROM wird nicht mehr beeinflusst
; 29.07.2026: SIXD4 von ROM-F in ROM-E verschoben, damit USER-ROM bei RESET als
;	      AUS in MODST eingetragen werden kann
; 29.07.2026: KBDZ verschoben, damit JP RESFL zu JR werden konnte

	ORG	0E000H

RESET:	JP	BYE		; Tasten-RESET
	JP	BEXP1		; BASIC-Erweiterung 1 - Token starten
	JP	BEXP2		; BASIC-Erweiterung 2
	JP	BEXP3		; BASIC-Erweiterung 3
	DW	TOKTAB		; Token-Tabelle der Erweiterungen
	JP	BASPV		; BASIC Programmverteiler
;
; (Fehler-)BEEP:		; Menuewort BASIC nicht auf E011H wegen
;				; Unterscheidung KC85/3 und KC85/4
;
BEL:	LD	BC,0A1FH	; B=Dauer, C=Lautstaerke
	LD	L,48		; L=Tonhoehe 1 (70H bei CAOS 3.4)
	CALL	RTON		; Ton ausgeben nur rechts
	LD	A,46		; 46 x 6ms = 276ms
	JP	WAIT		; Pause

;*** Steuerung ***
 
	DW	7F7FH
	DB	'BASIC',0
	LD	HL,0C000H	; BASIC Kaltstart
	JR	BASON
 
	DW	7F7FH
	DB	'REBASIC',0
	LD	HL,0C002H	; BASIC Warmstart
BASON:	IN	A,(PIOAD)
	SET	7,A		; USER-ROM ein (B802H bleibt unveraendert)
	OUT	(PIOAD),A
	in	a,(PIOBD)
	and	9Fh		; BASIC = Segment 0
	OUT	(PIOBD),A
	JP	(HL)
 
; CRT-Treiber:
 
PADR1:	LD	HL,(WINON)
	ADD	HL,DE
	SLA	H
	SLA	H
	SLA	H
;_____________________________________________________________
;							**34**
; Berechne Pixel- und Farbadresse aus Zeichenposition
; (Programmteil 1:1 aus CAOS 3.3 uebernommen)
; PE:	H	Vertikalposition (0 ... FFH)
;	L	Horizontalposition (0 ... 27H)
; PA:	HL	Zeichen- und Farbadresse*
;	CY=1	Position ausserhalb
; VR:	F, HL, DE STACK: 1

PADR:	PUSH	AF
	LD	A,L		; Spalte
	CP	32
	JR	NC,IAD1		; rechts
; erster Teil (links)
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	L,H
	SRL	H
	SRL	H
	SCF
	RR	H
	RRA
	SRL	H
	RRA
	LD	D,H
	SCF
	RR	D
	SET	3,D
	LD	E,A
	RR	E
	RR	L
	RRA
	RR	L
	LD	L,A
	RL	H
	POP	AF
	AND	A
	RET
	;
IAD1:	CP	40
	JR	NC,IAD2		; zu gross
; zweiter Teil (rechts)
	PUSH	BC
	AND	7
	ADD	A
	LD	L,A
	LD	A,H
	RLCA
	LD	B,A
	AND	1
	OR	0B0H
	LD	D,A		; H-Teil Farbe
	LD	A,B
	RLCA
	LD	C,A
	AND	3
	OR	50H
	LD	B,H
	LD	H,A		; H-Teil Pixel ohne V1
	LD	A,B
	AND	30H
	OR	L
	LD	L,A
	LD	A,C
	RLCA
	RLCA
	AND	0C0H
	OR	L
	LD	L,A		;L-Teil Pixel und Farbe ohne V0 bzw. V6
	RR	B
	RR	L
	RR	B
	RL	H
	RR	C		; fehlende Bits
	RRA			; einschieben
	LD	E,A
	POP	BC
DBRE:	POP	AF
	AND	A		; CY=0
	RET
	;
IAD2:	POP	AF		; ausserhalb!
	SCF			; CY=1
	RET
;_____________________________________________________________
;							**33**
; Test Cursor im Fenster (kein Test auf Breite/Hoeh 0)
; PE:	D	Zeile der Cursorposition
;	E	Spalte der Cursorposition
; PA:	CY=1	Cursor ausserhalb
; VR:	AF

TCIF:	LD	A,(WINLG)	; Fenster-Breite
	DEC	A
	SUB	E		; Cursor-Spalte
	RET	C		; ausserhalb
	LD	A,(WINLG+1)	; Fenster-Hoehe
	DEC	A
	SUB	D		; Cursor-Zeile
	RET
;_____________________________________________________________
;							**32**
; Berechnung der VRAM-Adresse der Cursorposition
; im gerade eingestellten Fenster und Bild
; PE:	D	Zeile auf Bildschirm
;	E	Spalte auf Bildschirm
; PA:	CY=1	ausserhalb (Fehler)
;	HL	Adresse im Speicher
; VR:	F, HL

DABR:	PUSH	AF
	CALL	TCIF		; Cursor im Fenster?
	JR	C,IAD2
	LD	A,(WINON)
	ADD	E		; Cursor-Spalte
	PUSH	DE
	LD	E,A		; absolut
	LD	A,(WINON+1)
	ADD	A,D		; Cursor-Zeile
	ADD	A,A
	ADD	A,A
	ADD	A,A		; *8
	LD	L,A		; Pixelzeile
	LD	H,0
	ADD	HL,HL		; *2
	ADD	HL,HL		; *4
	LD	D,0B2H/2	; D=VRAM-Adresse/2, E=Spalte
	ADD	HL,DE
	LD	E,A		; E=Pixelzeile*^8
	ADD	HL,DE
	POP	DE
	JR	DBRE		; POP AF, AND A, RET

; Zeichen auf Grafikbildschirm sichtbar machen
; PE:	DE	Cursorposition im Fenster
;	A	Zeichencode
;	STBT	Steuerbyte

WPIX:	PUSH	DE
	PUSH	BC
	CALL	PADR1		; Pixel- und Farbadresse berechnen
	JR	C,WPIX4		; ausserhalb
	LD	BC,(STBT)	; B=Farbe, C=STBT
	EX	DE,HL
	BIT	1,C		; Schreiben COLOR?
	JR	NZ,WPIX1	; nein
	LD	(HL),B		; Farbe schreiben
	SET	5,L		; zweites Byte
	LD	(HL),B
WPIX1:	BIT	0,C		; Schreiben Pixel?
	JR	NZ,WPIX4	; nein
	LD	HL,CCTL0
	ADD	A,A
	JR	NC,WPIX2
	LD	L,Low(CCTL2)
WPIX2:	SUB	40H
	JP	P,WPIX3		; CCTL0/2
	INC	L
	INC	L
	XOR	0C0H		; CCTL1/3
WPIX3:	LD	B,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,B		; HL=Zeichentabelle
	ADD	A
	BIT	2,C		; invers?
	LD	C,A
	LD	B,0
	ADD	HL,BC
	ADD	HL,BC		; HL=Pixelmuster
	CALL	CPIX		; 8 Pixel in IRM schreiben
WPIX4:	POP	BC
	POP	DE
	RET
;_______________________________________
;
; Pixelroutine fuer WPIX
; (8 Byte in Pixel-IRM schreiben)
;
; PE:	HL	ZG
;	DE	Pixeladr.
; VR:	A,DE,HL
;
CPIX:	CALL	CPIX1		; 4 Byte schreiben
	SET	5,E
	RES	0,D
CPIX1:	CALL	CPIX2		; 2 Byte schreiben
	SET	0,D
CPIX2:	RES	7,E
	LD	A,(HL)		; Zeichencode
	INC	HL		; naechstes Byte
	JR	Z,CPIX3
	CPL			; Pixel invertieren
CPIX3:	LD	(DE),A		; IRM schreiben
	SET	7,E
	LD	A,(HL)		; Zeichencode
	INC	HL		; naechstes Byte
	JR	Z,CPIX4
	CPL			; Pixel invertieren
CPIX4:	LD	(DE),A		; IRM schreiben
	RET
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Zerlegen des Fensters in einen linken und rechten Teil
; PE:	A	Zeile
; PA:	B	Anfang Teil I
;	C	Laenge Teil I
;	D	Anfang Teil II
;	E	Laenge Teil II oder 00h (nur 1teilig)
; VR:	AF,BC,DE
;
RSPALT:	ADD	A,A
	ADD	A,A
	ADD	A,A		; *8
	LD	H,A		; Pixelzeile in H
	LD	DE,2000H
	LD	A,(WINON)
	LD	B,A
	CP	D
	LD	A,(WINLG)
	LD	C,A
	RET	NC
	ADD	B
	CP	D
	RET	C
	SUB	D
	LD	E,A
	LD	A,D
	SUB	B
	LD	C,A
	RET
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Scrollen einer Zeile (8 Pixelzeilen)
; PE:	H	Zeile (0,4 ... 255)
;	L	Anfang Zeile
;	A	Laenge
; VR:	F
;
MOVELN:	PUSH	HL
	PUSH	DE
	PUSH	BC
	LD	C,A
	PUSH	HL
	CALL	PADR		; Pixel- und Farbadresse berechnen
	JR	C,PIX2
	EX	(SP),HL
	PUSH	DE
	LD	A,H
	ADD	A,8
	LD	H,A
	CALL	PADR		; Pixel- und Farbadresse berechnen
	EX	(SP),HL
	EX	DE,HL
	LD	B,0
	LD	A,(STBT)
	RRA
	BIT	0,A		; Farbe schreiben?
	LD	A,C
	CALL	Z,PIX4		; Farbe scrollen
	POP	HL
	POP	DE
	CALL	NC,PIX31	; 4 Pixel scrollen, falls Pixel schreiben ein
	INC	H
	INC	D
	CALL	NC,PIX3		; 4 Pixel scrollen, falls Pixel schreiben ein
	DEFB	6		;LD B,n
PIX2:	POP	HL
POPS3:	POP	BC
	POP	DE
	POP	HL
	RET
	;
PIX3:	RES	7,L
	RES	7,E
PIX31:	CALL	PIX4		; 2 Pixel scrollen
	SET	7,L
	SET	7,E		; und reinlaufen
PIX4:	RES	5,L
	RES	5,E
	CALL	PIX5		; 1 Pixel scrollen
	SET	5,L
	SET	5,E		; und reinlaufen
PIX5:	PUSH	DE
	PUSH	HL
	LD	C,A
	LDIR
	POP	HL
	POP	DE
	RET
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Anzeigen eines Zeichen auf Bildschirm mit Cursorsteuerung
; und Auswertung von ESC-Sequenzen
; PE:	A	ASCII
;	DE	Cursorposition
; PA:	DE	neuer Cursor
; VR:	AF,BC,DE,HL
;
PCHR:	LD	HL,STBT
	BIT	4,(HL)		; ESC aktiv?
	JR	Z,PCHR0
	RES	4,(HL)		; ESC quittieren
	CP	'0'
	RET	C		; ESC-Folgecode kleiner als '0'
	CP	'9'+1
	JR	C,ESCJ1		; ESC-Folgecode '0' bis '9'
	CP	'A'
	RET	C		; ESC-Folgecode kleiner als 'A'
	RES	5,A		; upcase
	CP	'Z'+1
	RET	NC		; ESC-Folgecode groesser als 'Z'
	SUB	7
ESCJ1:	SUB	'0'
	LD	HL,L3SIZ	; Tabelle
	CP	(HL)		; lang genug?
	RET	NC
	LD	HL,(L3TAB)	; Tabelle der ESC-Funktionen
	JR	PCHV		; klar zum Ansprung
	;
PCHR0:	CP	20H		; Steuerfunktion?
	JR	NC,PCHR1	; nein, darstellbares Zeichen
	BIT	3,(HL)		; Steuerzeichen darstellen?
	JR	NZ,PCHR1	; ja, nicht ausfuehren
	LD	HL,(CTAB)	; Tabelle der Steuerfunktionen
PCHV:	ADD	A,A		; *2
	LD	C,A
	LD	B,0
	ADD	HL,BC
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	LD	A,1		; Vorbereitung fuer ESC-Sequenzen
	JP	(HL)		; Steuerfunktion anspringen

; Tabulatorschritt mit Code 05h
ASC5:	LD	A,E
	AND	0F8H
	ADD	8
	LD	E,A
	JR	PCHR3
	;
PCHR1:	CALL	DABR		; VRAM-Adresse berechnen
	RET	C		; Abbruch, da ausserhalb des Fensters
PCHR2:	LD	(HL),A		; ASCII in VRAM eintragen
	CALL	WPIX		; Zeichen darstellen

; Cursor nach rechts
CUR:	INC	E		; Cursor nach rechts
PCHR3:	LD	A,(WINLG)
	DEC	A
	CP	E		; Zeilenende?
	RET	NC

; Neue Zeile mit Code 1Eh
CRNL:	LD	E,0		; Cursor ganz nach links

; Cursor nach unten
CUD:	INC	D		; naechste Zeile
	LD	A,(WINLG+1)
	CP	D		; letzte Zeile?
	RET	NZ
	LD	HL,(WEND)	; Reaktionsprogramm bei Fensterende
	JP	(HL)		; PAGE oder SCROLL

; Aufruf der Hardcopy-Routine mit Code 0Fh
HCOPY:	LD	HL,(HCADR)	; ShCLR
	JP	(HL)

; Cursor nach links
CUL:	LD	A,E		; Spalte
	AND	A		; ganz lnks?
	JR	Z,LINUP
	DEC	E		; eine Position nach links gehen
	RET
	;
LINUP:	LD	A,D		; Zeile
	AND	A		; ganz oben?
	RET	Z
	DEC	D		; eine Zeile nach oben gehen
CEL:	LD	A,(WINLG)	; Spalten im aktuellen Fenster
	DEC	A		; letzte Spalte
	LD	E,A		; Cursor auf letzte Spalte setzen
	RET

; Cursor nach oben:
CUU:	LD	A,D		; Zeile
	AND	A		; bereits ganz oben?
	RET	Z
	DEC	D		; eine Zeile nach oben gehen
	RET
;
; Page-Mode
;
PAGE:	LD	HL,HOMEPG	; ShCUU
WADR:	LD	(WEND),HL
	RET
;
; Scroll-Mode
;
SCROL:	LD	HL,SCRLPG	; ShCUD
	JR	WADR

; Zeichen nach links loeschen mit Code 01h
CLEAR:	CALL	CUL		; Cursor nach links

; Zeichen unter Cursor loeschen mit Code 1Fh
DEL:	PUSH	DE		; Cursor retten
DEL1:	CALL	DABR
	LD	A,(HL)
	AND	A		; Ende?
	JR	Z,POPDE
	PUSH	DE		; Cursorpos.
	PUSH	HL		; VRAM
	INC	E		; naechste Spalte
	CALL	DABR		; raus?
	JR	NC,DEL2
	LD	E,0		; Spalte 0
	INC	D		; Zeile+1
	CALL	DABR		; raus?
DEL2:	LD	A,(HL)		; Zeichen von neuer Position
	POP	HL		; vorherige Position
	POP	DE		; Cursorpos.
	JR	C,DEL3		; raus bei DABR -> Ende
	OR	A
	JR	Z,DEL3		; leer, Ende
	CALL	PCHR2		; Zeichen darstellen
	JR	DEL1
	;
DEL3:	CALL	DABR		; VRAM-Adresse berechnen
	XOR	A
	LD	(HL),A		; 0 eintragen
	LD	A,' '
	CALL	WPIX		; Leerzeichen anzeigen
POPDE:	POP	DE
	RET

; Leerzeichen an Cursorposition einfuegen
INS:	PUSH	DE		; Cursor
	LD	A,' '		; Leerzeichen
INS1:	CALL	DABR
	LD	B,(HL)		; Zeichen aus VRAM merken
	CALL	PCHR2		; Zeichen darstellen
	LD	A,B
	AND	A		; war Zeichen leer?
	JR	Z,POPDE		; ja, fertig
	LD	A,E		; Spalte
	OR	D		; Zeile
	LD	A,B		; gemerktes Zeichen
	JR	NZ,INS1		; nicht raus (nur im Page-Mode wirksam)
	POP	DE
	RET

; Loeschen einer Zeile bzw. Zeile(nstueck)
; PE:	B	Anfang
;	C	Laenge
;	H	Zeile
; PA:	A=00h
; VR:	AF,BC,DE,HL
;
CLLINE:	LD	L,B		; Spalte
	CALL	PADR		; Pixel- und Farbadresse berechnen
	RET	C		; ausserhalb
	LD	A,(STBT)
	RRA
	BIT	0,A		; Farbe schreiben?
	LD	A,(COLOR)	; Farbwert
	CALL	Z,CM2		; 2x loeschen
	LD	A,0
	RET	C		; Pixel nicht schreiben
	EX	DE,HL		; DE=Pixeladresse
	CALL	CM1		; 4x loeschen
	INC	D		; und reinlaufen
CM1:	RES	7,E
	CALL	CM2		; 2x loeschen
	SET	7,E		; und reinlaufen
CM2:	RES	5,E
	CALL	CM3		; 1x loeschen
	SET	5,E		; und reinlaufen
CM3:	PUSH	DE
	LD	B,C		; Anzahl neu laden
CM4:	LD	(DE),A
	INC	E		; kein Seitenuebertrag
	DJNZ	CM4
	POP	DE
	RET

; Bildschirm loeschen mit Code 0Ch (ShHOME)
CLS:	LD	A,(WINLG+1)	; Anzahl Zeilen
	LD	D,0		; Zeile 0
CLS1:	PUSH	AF
	CALL	DELLN		; Zeile loeschen
	POP	AF
	INC	D
	DEC	A
	JR	NZ,CLS1		; A Zeilen

; Cursor in Home-Position mit Code 10h
HOMEPG:	LD	D,0		; Zeile 0
CRN:	LD	E,0		; Spalte 0
	RET

; Scrollroutine
SCRLPG:	LD	A,(WINLG)	; Anzahl Spalten
	LD	C,A
	LD	B,0
	LD	A,(WINLG+1)	; Anzahl Zeilen
	DEC	A		; nur eine Zeile?
	JR	Z,SCRL4		; ja
	PUSH	DE
	PUSH	AF
	LD	D,B		; DE=0
	LD	E,B		; Cursor links/oben
	CALL	DABR
;	JR	C,SCRL3		; raus
	EX	DE,HL
SCRL1:	LD	HL,40		; eine Zeile tiefer
	ADD	HL,DE
	PUSH	HL
	PUSH	BC
	LDIR			; im VRAM eine Zeile rollen
	POP	BC
	POP	DE
;	INC	H		; diese drei Befehle zusaetzlich in CAOS 3.3
;	BIT	3,H
;	JR	NZ,SCRL11	; illegales Fenster
	DEC	A
	JR	NZ,SCRL1	; A-mal rollen
	LD	A,(WINON+1)
	CALL	RSPALT		; Berechne BC,DE:Fenstergrenzen
SCRL2:	LD	A,C
	LD	L,B
	CALL	MOVELN		; 1. Teil scrollen
	LD	A,E
	AND	A
	LD	L,D
	CALL	NZ,MOVELN	; 2. Teil scrollen, falls erforderlich
	LD	A,H
	ADD	A,8		; naechste Zeile
	LD	H,A
	POP	AF
	DEC	A
	PUSH	AF
	JR	NZ,SCRL2
SCRL3:	POP	AF
	POP	DE
SCRL4:	DEC	D		; letzte Zeile loeschen

; Zeile loeschen mit Code 02h (ShDEL)
DELLN:	LD	E,0		; Cursor auf Spalte 0
	PUSH	DE
	CALL	DABR
	LD	A,(WINLG)
	LD	B,A		; Anzahl Spalten
	XOR	A
DL1:	LD	(HL),A		; im VRAM loeschen
	INC	HL
	DJNZ	DL1
	LD	A,(WINON+1)
	ADD	A,D
	CALL	RSPALT		; Berechne BC,DE:Fenstergrenzen
	PUSH	DE
	PUSH	HL
	CALL	CLLINE		; 1. Teil loeschen
	POP	HL
	POP	BC
	CP	C
	CALL	NZ,CLLINE	; 2. Teil loeschen, falls erforderlich
	POP	DE
	RET
;
; Ton fuer Tastenclick
;
RCLICK:	LD	BC,0307H	; B=Dauer, C=Lautstaerke	CAOS 4.9
	LD	L,36		; L=Tonhoehe 1
;	LD	BC,20FH		; B=Dauer, C=Lautstaerke	CAOS 3.4
;	LD	L,50H		; Tonhoehe 1
RTON:	PUSH	DE		; Ton nur rechts
	LD	H,0
	LD	E,H		; E=0 (Tonhoehe 2)
	CALL	PHALT		; beide Tonkanaele aus
	OUT	(CTC2),A	; CTC2: Tondauer off
	CALL	TON2		; Ton ausgeben
	POP	DE
	RET

; ESC 8: Farbe Vordergrund/Hintergrund tauschen mit Code 06h
ASC6:	LD	HL,COLOR
	LD	A,(HL)		; Farbe compeln
	AND	0C0H
	LD	C,A		; Attribute (Blink/Verschiebung)
	LD	A,(HL)
	AND	7		; Hintergrundfarbe
	RLCA			; zu Vordergrund verschieben
	RLCA
	RLCA
	OR	C		; Attribute wieder einbauen
	LD	C,A
	LD	A,(HL)
	RRCA			; Vordergrundfarbe
	RRCA			; zu Hintergrund verschieben
	RRCA
	AND	7		; nur Hintergrundbits
	OR	C		; Vordergrund + Attribute dazu
;_____________________________________________________________
;							**28**
; LD (HL),A fuer PV4-PV6
; PE:	A	Byte
;	HL	Adresse
; PA:	-
; VR:	-

LDMA:	LD	(HL),A
	RET
;_____________________________________________________________
;							**29**
; LD A,(HL) fuer PV4-PV6
; PE:	HL	Adresse
; PA:	A	Byte auf Adresse (HL)
; VR:	A

LDAM:	LD	A,(HL)
	RET

; CAPS-Funktion (Sh-Lock)
SHLOCK:	LD	A,80H		; CAPS ein/aus
	DB	1		;LD BC,...
CLICK:	LD	A,20H		; Tastenclick ein/aus
	XOR	(IX+8)
	LD	(IX+8),A
DUMMY:	RET
;_____________________________________________________________
;							**00**
; Zeichenausgabe auf Bildschirm
; PE:	A	Zeichencode (ASCII)
; PA:	-
; VR:	-

CRT:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	DE,(CURSO)	; aktuelle Cursorposition
	CALL	PCHR		; Zeichen anzeigen
	LD	(CURSO),DE	; neue Cursorposition
	POP	AF
	JP	POPS3

; Beeinflussung Steuerbits
ESCPRG:	ADD	A,A		; Bit 4: ESC aktiv
ESC4:	ADD	A,A		; Bit 3: Steuercodes darstellen
ESC7:	ADD	A,A		; Bit 2: Inversdarstellung
ESCF:	ADD	A,A		; Bit 1: Schreiben Color-RAM
ESC3:	LD	HL,STBT		; Bit 0: Schreiben Pixel-RAM
	XOR	(HL)
	LD	(HL),A
	RET
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
;	=== KBD-Treiber ===

; Z E I T I N T E R R U P T
;
; ISR CTC Kanal 3 (Tastatur)
; Taste losgelassen
;
ISRC3:	EI
	PUSH	AF
	LD	A,23H		; DI, ZG256, Reset
	OUT	(CTC3),A
	LD	(IX+13),0	; Zeichen tot
	JR	TST4
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Keyboard-Interrupt-Routine
; Zeitkonstanten:
;
TCTZK	EQU	143	; 8FH
TGE0	EQU	101	; 65H
TGE1	EQU	66	; 42H
FEHL1	EQU	20	; 14H
FEHL2	EQU	120	; 78H
;
; T A S T A T U R I N T E R R U P T
;
; ISR PIO Kanal B (Tastatur)
; Taste gedrueckt
;
ISRPB:	EI
	PUSH	AF
	DEC	(IX)
	IN	A,(CTC3)	; gemessene Zeit
	PUSH	AF
	LD	A,0A7H		; EI, ZG256, Reset
	OUT	(CTC3),A
	LD	A,TCTZK		; Zeitkonstante
	OUT	(CTC3),A
	POP	AF
	CP	FEHL1		; neue Betaetigung? (Zeitinterrupt ausgeloest)
	JR	C,TST4		; oder zu lang
	CP	TGE0
	JR	NC,TST3		; 0-Bit
	ADD	A,-TGE1
	JR	C,TST3		; 1-Bit
	LD	A,(IX+12)	; Wortregister
	RRA			; 7bit-Scancode
	XOR	1		; Startbit invertieren
	PUSH	HL
	PUSH	DE
	LD	E,A
	LD	D,0		; DE=Scancode
	LD	L,(IX+14)
	LD	H,(IX+15)	; HL=aktuelle KTAB-Adresse
	ADD	HL,DE		; Position in Tabelle
	LD	A,(HL)		; ASCII holen
	BIT	7,(IX+8)	; CAPS aktiv?
	JR	NZ,TST1		; 1=nein
	CP	40H
	JP	M,TST1
	XOR	20H		; klein <=> gross
TST1:	CP	(IX+13)		; gleicher Code wie zuvor?
	PUSH	AF
	JR	NZ,TST5		; nein
	LD	A,(COUNT)
	CP	(IX+10)		; FastRepeat?
	JR	C,TST7
	POP	AF
	INC	(IX+10)		; Zeit abwarten
TST2:	POP	DE
	POP	HL
TST3:	RR	(IX+12)		; Bit einschieben
TST4:	POP	AF
INTE:	RETI
	;
TST5:	LD	(IX+10),0	; neuer Code
	LD	A,E
	CP	73H		; Tast-Nr. SoftReset (Shift-CAPS)?
	JR	NZ,TST6		; nein
	BIT	3,(IX+8)	; bereits 1x gedrueckt?
	RES	3,(IX+8)	; SoftReset vorbereiten
	JR	NZ,TST7		; erste Betaetigung
	LD	HL,RESET
	PUSH	HL
	RETI			; Sprung zu RESET
	;
TST6:	SET	3,(IX+8)	; SoftReset-Vorbereitung loeschen
TST7:	POP	AF		; Code
	LD	(IX+13),A	; eintragen
	SET	0,(IX+8)	; gueltig machen
	JR	TST2
;_____________________________________________________________
;							**0C**
; Tastaturabfrage ohne Quittung
; PA	CY=0	keine Tastenbetaetigung
;	CY=1	Taste gedrueckt
;	A	Tastencode oder 0, wenn keine Taste betaetigt

KBDS:	;OR	A		; CY=0, A unveraendert
	XOR	A		; CY=0 und A=0 bei CAOS 3.4
	BIT	0,(IX+8)	; Code gueltig?
	RET	Z		; nein
	LD	A,(IX+13)	; Tastencode holen
	SCF			; und mit CY=1 zurueck melden
	RET
 
;*******************************************************************************
;	TAPE-Treiber
;*******************************************************************************

; Zeitkonstanten fuer TAPE-Schreiben:
			; Vorton	"1"-Bits
ZKKAT	EQU	93	; Trennzeichen	Vollschwingung ca.  557 Hz
ZKKA0	EQU	23	; "0"		Vollschwingung ca. 1950 Hz
ZKKA1	EQU	47	; "1"		Vollschwingung ca. 1050 Hz

; Zeitkonstanten fuer TAPE-Lesen:
IKEZK	EQU	163	; Zeitkonstante fuer CTC
IKEGN	EQU	93	; Erkennung Nullbit
IKEG1	EQU	186	; Erkennung Einsbit

ESC1:	LD	A,(IX+7)
	XOR	2		; Autostart AUS/EIN
	LD	(IX+7),A
	RET
 
IMBII:	;intern mit Standardpuffer,
	;Erwarteter Block und Unter-
	;drueckung OK-Block
	CALL	CRO2		; Blinken aus, Standard-Kassettenpuffer
	LD	(IX+3),1	; erwartete Blocknummer
	SET	5,(IX+7)	; Blocknummer im Vorblock unterdruecken
	;
IMBI:	;Init.MB-Eingabe ohne Lesen
	;und ohne Init.(IX+5/6)
	IN	A,(PIOBD)
	RES	7,A		; Blinken off
	OUT	(PIOBD),A
	IN	A,(PIOAD)
	SET	6,A		; MotorSchaltSpannung on
IMBI1:	OUT	(PIOAD),A
	EI
	PUSH	HL		; Dateiname
	LD	HL,(WEND)	; aktuelle WEND-Adresse
	LD	(ZWEND),HL	; merken
	CALL	NOUT		; Normalausgabe einstellen
	LD	(ZOTAB),HL	; und vorherigen OUTAB merken
	CALL	PAGE		; Page-Mode aktivieren
	POP	HL		; Dateiname
PHALT:	LD	A,3		; CTC Stop
	OUT	(CTC0),A	; Tonhoehe Kanal 1
	OUT	(CTC1),A	; Tonhoehe Kanal 2
	RES	1,(IX+8)	; Ton ist jetzt aus
	RET
 
CMBI:	;Abschluss MB-Eingabe
	CALL	CRO0		; Bandein-/ausgabe beenden
;Blinken on
BLION:	DI
	LD	A,47H		; CTC-Mode
	OUT	(CTC2),A
	LD	A,12		; Zeitkonstante
	OUT	(CTC2),A
	EI
BLINK:	IN	A,(PIOBD)
	SET	7,A		; Blinken ein
	OUT	(PIOBD),A
	RET
;_____________________________________________________________
;							**09**
; Abschluss Bandausgabe, Schreiben letzter Block
; PA:	CY=0

CSRO:	LD	(IX+2),0FEH	; FFH = Kennung Ende-Block
	CALL	PV1
	DB	1		; MBO (Block auf Kassette ausgeben)
; Bandein-/ausgabe beenden:
CRO0:	PUSH	HL
	LD	HL,(ZOTAB)	; gespeicherter Wert von OUTAB
	LD	(OUTAB),HL	; regenerieren
	LD	HL,(ZWEND)	; gespeicherter Wert von WEND
	LD	(WEND),HL	; regenerieren
	LD	HL,CASS+128
	XOR	A
CRO1:	DEC	L		; Kassettenpuffer
	LD	(HL),A		; loeschen
	JR	NZ,CRO1
	POP	HL
	IN	A,(PIOAD)
	AND	9FH		; Motor, LED aus
	OUT	(PIOAD),A
	CALL	CRLF
	LD	A,(IX+7)
	AND	1FH		; Bit 5=0: Blocknummern anzeigen
	LD	(IX+7),A	; Bit 6,7=0
; genutzt von ISRO, MBOUT, SAVE
CRO2:	CALL	BLINK		; Blinken ein
; zusaetzlich genutzt von SIXD
SIXD4:	LD	(IX+5),Low(CASS)
	LD	(IX+6),High(CASS) ; Standard-Kassettenpuffer
	JR	RESFL		; kein Tastencode im Puffer
;_____________________________________________________________
;							**0E**
; Tastaturabfrage mit Quittung
; PA	CY=0	keine Tastenbetaetigung
;	CY=1	Taste gedrueckt
;	A	Tastencode oder 0, wenn keine Taste betaetigt

KBDZ:	CALL	KBDS		; Tastaturabfrage
	RET	NC		; keine Taste gedrueckt
RESFL:	RES	0,(IX+8)	; Taste quittieren
	RET

; Initialisierung Bandausgabe:				**08**
; PA:	CY=0	(kein Fehler bei Bandausgabe)

ISROI:	CALL	CRO2		; Blinken aus, Standard-Kassettenpuffer ...
				; schaltet auch USER-ROM-Ebene um ???
ISRO:	IN	A,(PIOAD)
	OR	60H		; Motor+LED ein
	CALL	IMBI1
	LD	(IX+2),0	; Block 1 (Vorblock)
	LD	BC,4096		; langer Vorton
	CALL	PV1
	DB	1		; Block ausgeben (MBO Shell)
	RET

; Ausgabe 1 Block					**01**
; PE:	BC	Laenge Vorton

MBOT:	INC	(IX+2)		; naechste Blocknummer
	LD	A,87H		; EI, ZG16, Reset
	DI
	OUT	(CTC1),A
	LD	A,ZKKA1		; Vorton "1"
	OUT	(CTC1),A
	EI
	LD	E,A		; E=47 (ZK Vorton)
MBOT1:	CALL	BITOUT		; Vorton ausgeben
	CPI
	JP	PE,MBOT1	; BC mal bis BC=0
	CALL	ZTON		; Trennzeichen
	LD	A,(IX+2)
	CALL	BYTOT		; Blocknummer
	LD	L,(IX+5)
	LD	H,(IX+6)	; HL = Adresse Blockanfang
	LD	B,128		; Zahler (Blocklaenge)
MBT1:	LD	A,(HL)
	CALL	BYTOT		; 128 Datenbytes ausgeben
	LD	A,C
	ADD	A,(HL)		; Pruefsumme
	LD	C,A		; nebenher berechnen in Register C
	INC	HL		; naechste Adresse
	DJNZ	MBT1
	CALL	BYTOT		; Pruefsumme ausgeben
	CALL	HBITOT		; Trennzeichen (2. Halbschwingung)
	LD	A,3		; CTC stoppen
	OUT	(CTC1),A
	RET

; Ausgabe eines Bytes auf Kassette
; PE:	A	Datenbyte
; PA:	CY=0
;	E	Zeitkonstante fuer Trennzeichen
; VR:	AF, E

BYTOT:	PUSH	BC
	LD	C,A		; Datenbyte
	LD	B,8		; 8 Bit ausgeben
BYTOUT:	RRC	C		; mit Bit 0 beginnend
	LD	E,ZKKA0
	JR	NC,BITO0	; CY=0 / Nullbit ausgeben
	LD	E,ZKKA1		; CY=1 / Einsbit ausgeben
BITO0:	CALL	BITOUT
	DJNZ	BYTOUT		; 8x wiederholen
	POP	BC
ZTON:	LD	E,ZKKAT		; ZK Trennzeichen

; Ausgabe einer Vollschwingung
; PE:	E	Zeitkonstante

BITOUT:	CALL	HBITOT		; Aufrufen und reinlaufen
HBITOT:	LD	(IX),E		; Zeitkonstante -> (IX)
HBIT1:	LD	A,(IX)		; Warten bis Zeitkonstante von
	AND	A		; Interruptroutine zurueckgesetzt ist
	JR	NZ,HBIT1
	RET
;_____________________________________________________________
; Initialisierung Bandeingabe				**0A**
; PE:	HL	Dateiname
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC
ISRI:	CALL	IMBI		; Blinken aus, Motor ein, WEND und OUTAB retten
	CALL	PV1		; Block einlesen
	DB	5		; (MBI Shell!!)
	RET

; Einen Block von Band einlesen:					**05**
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC
MBI:	LD	A,83H
	OUT	(PIOAS),A	; EI an PIO A (Kassette)
	PUSH	HL
	PUSH	DE
MBI1:	CALL	BRKT
	JR	C,MBI6		; Abbruch mit BRK-Taste
	LD	B,22
MBI2:	CALL	STOP1		; Lesen einer Vollschwingung
	JR	C,MBI1		; kein Einsbit ?
	CP	IKEG1		; Einsbit ?
	CALL	LEDON		; LED on/off
	JR	C,MBI1
	DJNZ	MBI2		; 22 korrekte Schwingungen abwarten
MBI3:	LD	B,2
MBI4:	XOR	A
	LD	C,A
	CALL	STOP2		; Lesen einer Halbschwingung
	CP	IKEGN		; Nullbit ?
	JR	NC,MBI3
	DJNZ	MBI4		; Warten auf ein komplettes Nullbit
	CALL	BYTIN		; Datenbyte lesen
	JR	C,MBI6		; Stoerung
	LD	(IX+2),A	; Blocknummer eintragen
	LD	B,128		; Blocklaenge
	LD	L,(IX+5)	; Blockpuffer
	LD	H,(IX+6)
	LD	E,0		; Pruefsumme=0
MBI5:	CALL	BYTIN		; Datenbyte lesen
	JR	C,MBI6		; Stoerung
	LD	(HL),A		; Datenbyte in Blockpuffer ablegen
	ADD	A,E		; Pruefsumme berechnen
	LD	E,A
	INC	HL		; naechste Adresse
	DJNZ	MBI5		; bis 128 Datenbytes eingelesen sind
	LD	L,A		; berechnete Pruefsumme
	CALL	BYTIN		; Vergleichspruefsumme einlesen
	JR	C,MBI6		; Stoerung
	SUB	L		; Pruefsumme richtig?
	ADD	A,-1		; CY:=/Z
MBI6:	POP	DE
	POP	HL
	LD	A,3		; DI an PIO A
	OUT	(PIOAS),A
LEDON:	IN	A,(PIOAD)	; LED ein/aus
	SET	5,A		; je nach CY
	JR	NC,MBI7
	RES	5,A
MBI7:	OUT	(PIOAD),A
	RET

; Umschaltung Motorschaltspannung mit ESC A
MOT:	IN	A,(PIOAD)
	XOR	60H		; Motor EIN/AUS
	JR	MBI7
;
; ISR CTC Kanal 1 (Kassette-Out)
;
ISRC1:	PUSH	AF
	LD	A,87H		; EI, ZG16, Reset
	OUT	(CTC1),A
	LD	A,(IX)		; Uebergabezelle Zeitkonstante
	OUT	(CTC1),A	; Start CTC
	LD	(IX),0		; Quittierung
	JR	IMBEND		; EI, RETI
;
; ISR PIO Kanal A (Kassette-In)
;
ISRPA:	PUSH	AF
	IN	A,(CTC2)	; CTC Zaehlerstand lesen
	LD	(IX),A		; und in Uebergabezelle ablegen
	LD	A,7
	OUT	(CTC2),A	; CTC Neustart
	LD	A,IKEZK
	OUT	(CTC2),A	; Zeitkonstante
IMBEND:	POP	AF
INTEND:	EI
	RETI
;
; 1 Bit (Vollschwingung) einlesen:	(aus CAOS 4.5 uebernommen)
; PA:	A	Zeitwert
;	CY=0	Einsbit
;	CY=1	Nullbit
; VR:	AF, C
;
STOP1:	XOR	A		; mit 0 beginnen
	CALL	STOP2		; erst aufrufen, dann reinlaufen
STOP2:	LD	C,A		; 'Zeit' der 1. Halbschwingung in C merken
	LD	(IX),0
STOP21:	XOR	A
	OR	(IX)		; CTC-Stand von Interrupt eingetragen
	JR	Z,STOP21	; Warten auf 2. Halbschwingung
	ADD	A,C		; beide 'Zeiten' addieren
	RET

; Einlesen eines Bytes von Kassette
; PA:	A	Byte
;	CY=1	Trennzeichen, Stoerung
; VR:	AF, C, D

BYTIN:	LD	D,80H		; D=Bitschieberegister und Zaehler (Bit 7)
IB1:	CALL	STOP1		; Bit Eingabe
	JR	C,IB2		; Nullbit
	CP	IKEG1		; Einsbit
	RET	C		; CY=1: Trennzeichen
IB2:	CCF
	RR 	D		; Bit einschieben
	JR	NC,IB1		; bis 8 Bit abgearbeitet sind
	CALL	STOP1		; Trennzeichen (mit CY-Rueckmeldung)
	LD	A,D		; Byte in A
	RET
 
;*** Sound Unit ***
 
;
; ISR fuer CTC2: Tonlaenge
;
ISRC2:	PUSH	AF
	CALL	PHALT		; beide Tonkanaele aus
	CALL	BLION		; Blinken wieder aktivieren
	JR	IMBEND		; EI, RETI
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
	AND	00011111b	; maskieren (Bit 0-4)
	XOR	10011111b	; Komplement da nullaktiv
	LD	C,A
	LD	A,B		; Tondauer
	AND	A		; 0=Dauerton?
	DI
	JR	Z,TON3		; ja, keine CTC2 starten
	RES	7,C		; nein, Blinken aus
	SET	1,(IX+8)	; neuer Ton
	LD	A,11000111b	; EI, Zaehler 50Hz, ZK folgt
	OUT	(CTC2),A	; CTC2
	LD	A,B		; Tondauer
	OUT	(CTC2),A	; Zeitkonstante
	EI
TON3:	IN	A,(PIOBD)	; PIO-B
	AND	01100000b	; USER-ROM maskieren
	OR	C		; Lautstaerke einbauen
	OUT	(PIOBD),A	; Lautstaerke ausgeben

	LD	BC,038CH	; B=Steuerbyte "Reset", C=CTC0
	CALL	TON4		; Tonkanal 1 starten
	INC	C		; CTC1
	EX	DE,HL		; Tonkanal 2 starten
TON4:	OUT	(C),B		; CTC-Kanal anhalten
	LD	A,L		; Zeitkonstante
	AND	A
	RET	Z		; kein Ton (ZK=0)
	LD	A,H		; Vorteiler, Zeitgeber/Zaehler
	RRCA
	RRCA
	OR	0AH		; 2*5 (ZK folgt, CY=0)
	RRA			; CTC DI
	XOR	2
	DI
	OUT	(C),A		; CTC starten
	OUT	(C),L		; Zeitkonstante
	EI
	RET
 
 
;====== BASIC-ERWEITERUNG ==============
 
 
;*** BASIC I/O-Verteiler ***
 
BASPV:	PUSH	BC		; BC sichern
	PUSH	HL		; HL sichern (Dateiname)
	CALL	IRMON		; IRM ein und System-SP setzen
	BIT	5,E
	PUSH	DE		; E = Steuerbyte
	JR	NZ,BASBYE	; BYE -> BASIC-ROM aus -> CAOS
	INC	HL
	INC	HL
	BIT	7,E
	JR	NZ,BSA2		; Tastaturstatus
	PUSH	HL
	LD	A,E		; Kanalnummer
	AND	7		; 0-3
	LD	HL,BUPTAB	; Tabelle der Unterprogramme
	ADD	L
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
	CALL	OSTR
	DB	'VERIFY',0
	CALL	YESNO		; Ja/Nein-Abfrage
	CALL	Z,VERIFY	; bei "Y" VERIFY ausfuehren
BSA1:	LD	A,D		; Ausgabeparameter
	RES	3,E		; Init ruecksetzen
	CALL	IRMOF		; IRM aus und BASIC-SP wiederherstellen
BSAE:	POP	HL		; HL regenerieren
	POP	BC		; BC regenerieren
	RET
	;
BSA2:	CALL	KBDS		; Tastaturstatusabfrage
BSA3:	POP	DE
	LD	D,A		; Zeichencode in D zurueck geben
	JR	NC,BSA1		; keine Taste gedrueckt
	RES	7,E		; Rueckmeldung, dass Taste gedrueckt wurde
	JR	BSA1
	;
BASBYE:	IN	A,(PIOAD)
	RES	7,A		; USER-ROM aus (B802H bleibt unveraendert)
	OUT	(PIOAD),A
	JP	LOOP

; ===== BASIC =====

; BASIC-Erweiterung 1: Erweiterungstoken starten
;
;***************************************************************
;*  E: <B>: TOKEN-LDTOK
;*     <HL>: AUF TOKEN
;*  A: <<SP>>: STARTADRESSE ROUTINE
;*     <HL>: AUF TOKEN
;***************************************************************

BEXP1:	LD	A,B		; Nr.
	SUB	9
	JR	C,SNERR1
	CP	27
	JR	NC,SNERR1	; Token ausserhalb
	RLCA			; *2
	LD	C,A
	LD	B,0		; BC = Offset in Adresstabelle
	EX	DE,HL
	LD	HL,TADR5	; Adresse der Erweiterungstoken
	JP	DLI22		; Ruecksprung zu Tokenverarbeitung (BASIC-ROM)
 
LOCOL:	;LoadColor
	PUSH	HL
	LD	HL,COLOR	; Farbwert
	CALL	IRMRD		; aus IRM lesen
	POP	HL
	RET

; BASIC-Erweiterung 2: Print-Erweiterung?
;
;***************************************************************
;*  E:   <HL> AUF SIGNIF. ZEICHEN NACH PRTOK
;*  WENN  PRTFLG<>0 , DANN JP SNER
;*  A:   <HL> AUF ZEILENENDE
;***************************************************************

BEXP2:	LD	A,(HL)
	CP	0DFH		; INK-Token?
	RET	C		; <DFH
	CP	0E3H		; COLOR-Token?
	RET	NC		; >E2H
	CP	0E1H		; AT-Token?
	JP	Z,PRAT
	LD	A,(PRTFLG)
	AND	A		; PRINT-Erweiterung?
	JR	NZ,SNERR1	; nein, Fehler
	INC	A
	LD	(PRTFLG),A
	DB	3EH	;LD A,n
RBE2:	PUSH	BC
	CALL	LOCOL		; Farbwert lesen
	LD	(COLRET),A	; Farbe merken
	LD	A,(HL)
	CP	0DFH		; INK?
	JR	Z,PRINK
	CP	0E2H		; COLOR?
	JR	Z,PRCOL
	CALL	TCHAR		; naechstes Zeichen holen
	CALL	PAPER		; Hintergrundfarbe
PREX2:	LD	A,(HL)
	CP	';'
	JR	Z,PREX5
SNERR1:	JP	SNER		; SN-ERROR
	;
PRCOL:	CALL	TCHAR		; naechstes Zeichen holen
	CALL	BCOLOR		; Vordergrund- und Hintergrundfarbe
	JR	PREX2
	;
PRINK:	CALL	TCHAR		; naechstes Zeichen holen
	CALL	INK		; Vordergrundfarbe
	LD	A,(HL)
	CP	';'
	JR	Z,PREX5
	CALL	CPSTX		; Komma?
	DB	','
	CP	0E0H		; PAPER?
	JR	NZ,SNERR1
	CALL	TCHAR		; naechstes Zeichen holen
	CALL	PAPER		; Hintergrundfarbe
	CALL	CPSTX		; Semikolon?
	DB	';'
PREX3:	CALL	PRINT2		; Routine im BASIC-ROM aufrufen
	LD	A,(COLRET)	; gemerkte Farbe
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMWR		; in IRM eintragen
	POP	HL
	POP	BC
	RET
	;
PREX5:	CALL	TCHAR		; naechstes Zeichen holen
	JR	PREX3

; BASIC: Testen, ob Bildpunkt gesetzt ist:

PTEST:	CALL	SNLY14		; Syntax-Check '('
	EX	(SP),HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	EPRVL3		; Ausdruck berechnen -> DE
	PUSH	HL		; Programmzeiger auf Stack legen
	LD	HL,HOR
	CALL	LDMDE		; X-Koordinate in IRM schreiben
	LD	E,2FH		; UP-Nr. fuer PUDE
	CALL	PV5
	JP	C,FCER		; ausserhalb -> FC-ERROR
	LD	B,0		; Punkt war geloescht
	JR	Z,PTST1		; war geloescht
	INC	B		; B=1: Punkt war gesetzt
	INC	HL		; HL=FARB
	CALL	IRMWR		; Grafikfarbe eintragen fuer folgendes PUSE
	INC	E		; E=30H UP PUSE
	CALL	PV5		; Punkt wieder setzen
PTST1:	LD	A,B		; Rueckgabewert 0 oder 1
	JR	JHPOS		; POP HL, JP POS1

; BASIC: IRM lesen (8000h bis BFFFh):

VPEEK:	CALL	SNLY14		; Syntax-Check '('
	EX	(SP),HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	EPRVL3		; Wert bestimmen
	PUSH	HL
	LD	HL,8000H	; IRM-Offset
	ADD	HL,DE
	JR	C,SNERR4	; < 8000h
	BIT	6,H
	JR	NZ,SNERR4	; > BFFFh
	CALL	IRMRD		; aus IRM lesen
	JR	JHPOS		; POP HL, JP POS1

SNERR4:	JP	SNER		; SN-ERROR

; BASIC-Erweiterung 3: Funktionen
;**************************************************************
;* VERTEILER ZU ZUSAETZLICHEN STANDARDFUNKTIONEN
;* E: <HL> AUF SIGN. ZEICHEN NACH TOKEN
;*    <BC> =<<SP>>  (TOKEN-SGTOK)*2
;* A: <HL> AUF SIGN. ZEICHEN NACH TOKEN
;**************************************************************

BEXP3:	LD	A,C		; zulaessig?
	CP	62H
	JR	Z,VPEEK
	CP	6EH
	JR	Z,PTEST
	CP	7CH
	JR	Z,CSRLIN
	CP	76H
	JR	Z,VGET
	SUB	3EH
	JR	C,SNERR4	; nein
	CP	7
	JR	NC,SNERR4	; nein
;--------------------------------
	EX	DE,HL
	LD	BC,TOKADR	; Token-Adresstabelle
	POP	HL
	LD	L,A
	ADD	HL,BC
	LD	C,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,C
	PUSH	HL		; Routine
	EX	DE,HL		; anspringen
	RET

; BASIC: aktuelle Zeilennummer holen:

CSRLIN:	CALL	SNLY14		; Syntax-Check '('
	EX	(SP),HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	ARGVL2
	PUSH	HL
	AND	A		; Fenster beruecksichtigen?
	LD	A,0		; gesamter Bildschirm
	LD	HL,WINON+1	; Erste Zeile im Fenster
	CALL	Z,IRMRD		; Fenster beruecksichtigen
	LD	B,A		; erste Zeile
	LD	L,Low(CURSO+1)	; Cursor-Zeile
	CALL	IRMRD		; aus IRM lesen
	ADD	A,B		; aktuelle Zeile berechnen
JHPOS:	POP	HL
CSRLN2:	JP	POS1

; BASIC: Zeichen von Bildschirm zurueck lesen

VGET:	EX	(SP),HL
	LD	A,1
	CALL	STADTB
	CALL	IRMON
	LD	DE,(CURSO)
	CALL	DABR
	LD	A,(HL)
	CALL	IRMOF
	JP	AKTCHR		; als Zeichen zurueckgeben

; BASIC: Kanal schliessen:

CLOSE:	LD	C,0
	LD	A,(HL)
	CP	'I'		; Eingabe?
	JR	Z,CLOS1
	INC	C
	CP	'O'		; Ausgabe?
	JR	NZ,SNERR4	; SN-ERROR
CLOS1:	PUSH	BC
	INC	HL
	LD	A,(HL)
	CP	'#'
CLOS2:	JR	NZ,SNERR4	; SN-ERROR
	INC	HL
	CALL	ARGVL1		; Wert abholen
	AND	3		; 4 Geraete
	POP	BC
	RET	Z		; Console -> nichts tun
	PUSH	HL
	RLA			; *2
	ADD	A,C		; Ein-/Ausgabe
	PUSH	AF
	DEC	A
	LD	B,A
	LD	A,-1
CLOS3:	RLA			; Bit platzieren
	DJNZ	CLOS3
	LD	HL,IOCHNL	; E/A-Flag
	AND	(HL)		; Bit loeschen
	LD	(HL),A
	POP	AF
	POP	HL
	SET	6,A		; close
	PUSH	DE
	LD	E,A
	LD	D,3		; BRK-Code
	CALL	BASPV		; Kanal schliessen
	POP	DE
	RET

; BASIC: Kanal oeffnen:

OPEN:	LD	A,(HL)
	INC	HL
	CP	'I'		; Eingabe?
	JR	Z,OPENI
	CP	'O'		; Ausgabe?
	JR	NZ,CLOS2	; SN-ERROR
	CALL	OUTCHL		; Kanal-Ausgabe
	CALL	TESTCO
	RET	Z
	LD	A,0D5H		; *.UUU
	CALL	CASS1		; Dateiname aufbereiten
	LD	HL,INTPRB	; Druckpuffer=Dateiname
	XOR	A
	CALL	CO		; Ausgabe Zeichen
	POP	HL
	RET
	;
OPENI:	CALL	INCHNL		; Kanal-Eingabe
	LD	A,(ININD)	; IN-Index
	AND	3		; 4 Geraete
	RET	Z		; Console -> nichts tun
	LD	A,0D5H		; *.UUU
	CALL	CASS1		; Dateiname aufbereiten
	LD	HL,INTPRB	; Druckpuffer=Dateiname
	CALL	CI		; Eingabe eines Zeichens
	POP	HL
	RET

; BASIC: String vervielfaeltigen

STRING:	CALL	CPSTX		; Klammer?
	DB	'('
	CALL	ARGVL1		; Wert abholen
	PUSH	AF
	CALL	CPCOMM		; Komma?
	CALL	SNAlY
	CALL	CPBRGT		; Klammer zu?
	POP	AF
	PUSH	HL
	PUSH	AF
	CALL	LEN1		; Laenge holen
	INC	HL
	INC	HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	POP	BC
	PUSH	BC
	PUSH	AF
	PUSH	DE
	LD	C,A
	XOR	A
	CP	C
	JR	Z,STR2
	CP	B
	JR	Z,STR2
	LD	A,C
	DEC	B
	JR	Z,STR2
STR1:	ADD	A,C
	JR	C,STR5		; LS-ERROR (String zu lang)
	DJNZ	STR1
STR2:	LD	B,A
	LD	C,0
	PUSH	BC
	CALL	STROP		; String-Arithmetik
	POP	BC
	POP	BC
	PUSH	BC
	CALL	SADTB1
	POP	HL
	EX	(SP),HL
	LD	A,H
	POP	HL
	EX	(SP),HL
	LD	L,A
	INC	H
STR3:	DEC	H
	PUSH	HL
	PUSH	BC
	JR	Z,STR4
	CALL	STRMV1
	POP	BC
	POP	HL
	JR	STR3
	;
STR4:	POP	BC
	POP	HL
	POP	DE
	CALL	STRZS3
	JP	SLEN3
	;
STR5:	LD	E,28		; LS-ERROR
	JP	ERROO		; Ausgabe Fehlercode

; BASIC: Neu nummerieren

RENUM:	PUSH	HL
	LD	HL,10
	LD	(DISTAN),HL	; Zeilennummernabstand
	LD	HL,(PSTBEG)	; Programmbeginn
	PUSH	AF
	PUSH	HL
	INC	HL
	INC	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	LD	(ANF),HL	; erste Zeilennummer
	LD	(NANF),HL
	LD	DE,(SVARPT)	; Ende
	DEC	DE
	DEC	DE
REN1:	POP	HL
	PUSH	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	CALL	CPREG
	EX	(SP),HL
	JR	NZ,REN1
	POP	DE
	INC	HL
	INC	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	LD	(ENDE),HL	; Zeilen-Abstand
	LD	B,4
	POP	AF
	LD	HL,ANF
	EX	(SP),HL
REN2:	JR	Z,REN5
	CALL	DCHEX
	PUSH	AF
	LD	A,D
	OR	E
REN3:	JP	Z,FCER	 	; FC-ERROR
	POP	AF
	EX	(SP),HL
	LD	(HL),E
	INC	HL
	LD	(HL),D
	INC	HL
	JR	Z,REN5
	PUSH	AF
	DEC	B
	JR	Z,REN4
	POP	AF
	EX	(SP),HL
	CALL	CPCOMM		; Komma?
	JR	REN2
	;
REN4:	POP	AF
	JP	NZ,SNER		; SN-ERROR
REN5:	LD	HL,(ENDE)
	LD	DE,(ANF)	; Anfang
	CALL	CPREG
REN6:	JP	C,FCER		; FC-ERROR
	LD	HL,(PSTBEG)
REN7:	CALL	ZPOIT1		; naechste Zeile
	JR	C,REN8
	JR	Z,REN3
	JR	REN7
	;
REN8:	POP	HL
	PUSH	BC
	LD	DE,(ENDE)
	LD	HL,0
	LD	(ENDE),HL
REN9:	LD	H,B
	LD	L,C
	LD	C,(HL)
	INC	HL
	LD	B,(HL)
	LD	A,B
	OR	C
	JR	Z,REN3
	INC	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	CALL	CPREG
	LD	HL,(ENDE)
	INC	HL
	LD	(ENDE),HL
	JR	NZ,REN9
	INC	HL
	INC	HL
	ADD	HL,HL
	INC	HL
	LD	DE,(SVARPT)
	ADD	HL,DE
	JR	C,REN6
	CALL	TMEMO		; genug RAM?
	LD	(SVARPT),HL
	XOR	A
	DEC	HL
	LD	(HL),A
	DEC	HL
	LD	(HL),A
	DEC	DE
	DEC	DE
	EX	DE,HL
	LD	(HL),E
	INC	HL
	LD	(HL),D
	INC	HL
	DEC	A
	LD	(HL),A
	INC	HL
	LD	(HL),A
	INC	HL
	EX	DE,HL
	LD	HL,(NANF)
	LD	(ANF),HL
REN10:	POP	HL
	LD	C,(HL)
	INC	HL
	LD	B,(HL)
	INC	HL
	PUSH	BC
	LD	BC,ANF
	LD	A,(HL)
	LD	(DE),A
	LD	A,(BC)
	LD	(HL),A
	INC	HL
	INC	DE
	INC	BC
	LD	A,(HL)
	LD	(DE),A
	LD	A,(BC)
	LD	(HL),A
	INC	DE
	LD	HL,(ANF)
	LD	BC,(DISTAN)
	ADD	HL,BC
	LD	(ANF),HL
	LD	HL,(ENDE)
	DEC	HL
	LD	A,H
	OR	L
	LD	(ENDE),HL
	JR	NZ,REN10
	LD	(DE),A
	POP	HL
	LD	HL,(PSTBEG)
	PUSH	HL
REN11:	POP	HL
	LD	C,(HL)
	INC	HL
	LD	B,(HL)
	INC	HL
	PUSH	BC
	LD	A,(HL)
	INC	HL
	AND	(HL)
	INC	A
	JR	Z,REN15
REN12:	INC	HL
REN13:	LD	A,(HL)
	OR	A		; Zeilen-Ende
	JR	Z,REN11
	CP	88H		; GOTO
	JR	Z,REN16
	CP	8CH		; GOSUB
	JR	Z,REN16
	CP	8BH		; RESTORE
	JR	Z,REN14
	CP	0D4H		; ELSE
	JR	Z,REN14
	CP	0A9H		; THEN
	JR	NZ,REN12
REN14:	CALL	DCHEX1		; Fuer RESTORE, THEN, ELSE
	LD	A,E
	OR	D
	CALL	NZ,RENLN 	; Vergleich mit Tabelle
	CALL	NZ,RENZN	; Zahl aendern
	JR	REN13
	;
REN15:	DEC	HL
	LD	(SVARPT),HL
	DEC	HL
	LD	(HL),A
	DEC	HL
	LD	(HL),A
	POP	HL
	POP	HL
	JP	LIN10		; Zeile neu einsortieren
	;
REN16:	CALL	DCHEX1		; Fuer GOTO, GOSUB
	LD	A,E
	OR	D
	JR	Z,REN13
	CALL	RENLN
	CALL	NZ,RENZN
	LD	A,(HL)
	CP	','
	JR	NZ,REN13
	JR	REN16
 
RENLN:	PUSH	HL
	PUSH	DE
	LD	DE,-1
	CALL	ZPOIT		; naechste Zeile
	POP	DE
	INC	BC
	INC	BC
	INC	BC
	INC	BC
	LD	H,B
	LD	L,C
	LD	BC,(NANF)
RL1:	LD	A,(HL)
	INC	HL
	PUSH	HL
	OR	(HL)
	JR	Z,REND2
	LD	A,(HL)
	DEC	HL
	LD	L,(HL)
	LD	H,A
	CALL	CPREG
	JR	Z,REND1
	LD	HL,(DISTAN)
	ADD	HL,BC
	LD	B,H
	LD	C,L
	POP	HL
	INC	HL
	JR	RL1
	;
REND1:	XOR	A
	DEC	A		; A=0FFh, Z=0
REND2:	POP	HL
	POP	HL
	RET

RENZN:	PUSH	BC		; NEUE ZEILEN-NR.
	EX	DE,HL		; Fuer SPRUENGE,...
	LD	HL,(SVARPT)
	SBC	HL,DE
	PUSH	HL
RZ1:	POP	BC
	LD	H,D
	LD	L,E
	DEC	DE
	LD	A,(DE)
	CP	','
	JR	Z,RZ2
	CP	':'
	JR	NC,RZ2
	PUSH	BC
	PUSH	DE
	LDIR
	POP	DE
	JR	RZ1
	;
RZ2:	EX	DE,HL
	POP	DE
	PUSH	HL
	PUSH	BC
	XOR	A
	LD	B,98H
	CALL	SGN1
	CALL	NUMKON
	POP	BC
	POP	DE
	INC	HL
	INC	DE
RZ3:	LD	A,(HL)
	OR	A
	JR	Z,RZ4
	PUSH	BC
	PUSH	HL
	EX	DE,HL
	ADD	HL,BC
	LD	D,H
	LD	E,L
	DEC	HL
	LDDR
	POP	HL
	LDI
	POP	BC
	JR	RZ3
	;
RZ4:	PUSH	DE
	LD	DE,(PSTBEG)
	CALL	LIN11
RZ5:	INC	HL
	LD	A,(HL)
	INC	HL
	OR	(HL)
	JR	NZ,RZ5
	EX	DE,HL
	LD	(HL),E
	INC	HL
	LD	(HL),D
	INC	DE
	INC	DE
	LD	(SVARPT),DE
	POP	HL
	LD	D,H
	LD	E,L
RZ6:	LD	A,(HL)
	OR	A
	INC	HL
	JR	NZ,RZ6
	POP	BC
	EX	(SP),HL
	PUSH	BC
	EX	DE,HL
	RET

; BASIC: Zeile(n) loeschen

DELETE:	RET	Z
	CALL	DCHEX
	JP	Z,LIN13		; Zeiger auf folgende Zeilennummer stellen
	CALL	CPCOMM		; Komma?
	PUSH	DE
	CALL	DCHEX
	POP	HL
	RET	NZ
	EX	DE,HL
	PUSH	HL
	CALL	ZPOIT		; naechste Zeile
	JR	NC,DELET1	; UL-ERROR
	POP	DE
	PUSH	AF
	PUSH	BC
	CALL	ZPOIT1		; naechste Zeile
	POP	BC
	JP	C,LIN6		; einsortieren
DELET1:	JP	ULER		; UL-ERROR

; BASIC: Programm unterbrechen

PAUSE:	CALL	TCHAR1		; mit oder ohne Paramater?
	JR	NZ,PSE2
PSE1:	LD	E,2AH		; BRKT
	CALL	PV5		; ueber PV5 aufrufen (IRM ein/aus)
	RET	C		; BRK gedrueckt
	CP	LF		; CUD?
	JR	NZ,PSE1
CONT:	JP	CI		; Eingabe ASCII
	;
PSE2:	CALL	ARGVL1		; Wert abholen
	LD	C,A
PSE3:	LD	A,16		; 16*6ms = 96 Millisekunden
	CALL	WAIT		; UP WAIT direkt aufrufen
;	LD	E,14H		; WAIT benoetigt keinen IRM
;	CALL	PV5
	LD	E,2AH		; BRKT
	CALL	PV5
	RET	C		; BRK gedrueckt
	CP	LF		; CUD?
	JR	Z,CONT
	DEC	C		; Zaehler verringern
	JR	NZ,PSE3		; weiter warten
	RET

; BASIC: Signalton

BEEP:	LD	B,1		; ohne Angabe 1x
	CALL	TCHAR1		; Parameter angegeben?
	JR	Z,BEEP1
	CALL	ARGVL1		; Wert abholen
	LD	B,A		; Anzahl
BEEP1:	LD	A,7		; Beep
	LD	E,0		; CRT
	CALL	PV5
	DJNZ	BEEP1
	RET

; Hilfsroutine fuer mehrfachen Aufruf: naechsten Wert abholen

MC8D4:	CALL	CPCOMM		; Komma?
	JP	ARGVL1		; Wert abholen

; BASIC: Fenster einstellen

WINDOW:	CALL	TCHAR1		; Parameter angegeben?
	JR	Z,WDW2		; nein, dann volles Fenster
	CALL	ARGVL1		; 1. Wert abholen
	LD	B,A		; B = erste Zeile
	PUSH	BC
	CALL	MC8D4		; 2. Wert holen
	LD	D,A		; D = letzte Zeile
	PUSH	DE
	CALL	MC8D4		; 3. Wert holen
	POP	DE
	LD	E,A		; E = erste Spalte
	PUSH	DE
	CALL	MC8D4		; 4. Wert holen
	POP	DE		; A = letzte Spalte
	EX	(SP),HL		; H = erste Zeile
	LD	L,E		; L = erste Spalte
	SUB	E		; Anzahl Spalten
;	JR	C,SNERR2	; Anfang > Ende (wird von WININ ueberwacht)
	INC	A
	LD	E,A		; E = Fensterbreite
	LD	A,D		; letzte Zeile
	SUB	H		; Anzahl Zeilen
;	JR	C,SNERR2	; Anfang > Ende (wird von WININ ueberwacht)
	INC	A
	LD	D,A		; D = Fensterhoehe
WDW1:	CALL	IRMON
	CALL	WININ0		; aktuelles Fenster einstellen
	CALL	IRMOF
	JR	C,SNERR2	; Fehler aufgetreten
	POP	HL
	RET
	;
WDW2:	PUSH	HL		; Standardfenstergoesse bei BASIC:
	LD	HL,100H		; 30 Zeilen, 40 Zeichen breit
	LD	DE,1E28H	; (oben und unten eine Zeile ungenutzt)
	JR	WDW1

; BASIC: MC-Programm von Kassette laden

BLOAD:	PUSH	HL
	LD	HL,ARGN
	XOR	A		; Laden ohne Offset
	CALL	IRMWR		; ARGN=0 in IRM schreiben
	POP	HL
	LD	E,10H		; UP LOAD
	JP	PV5

; BASIC: Eingabe eines Zeichens:

INKEY:	PUSH	HL
	CALL	KBDS		; Tastaturabfrage
	JR	NC,INKEY2	; keine Taste gedrueckt
	LD	A,1
	CALL	STADTB
	CALL	CI		; Eingabe ASCII
AKTCHR:	LD	HL,(STRDAT+2)
	LD	(HL),A
INKEY1:	JP	SLEN3
	;
INKEY2:	XOR	A
	CALL	STADTB
	JR	INKEY1

; BASIC: in IRM schreiben (8000h bis BFFFh):

VPOKE:	CALL	EPRVL4		; Parameter erfassen (Adresse)
	PUSH	DE
	CALL	MC8D4		; Wert abholen
	EX	(SP),HL
	LD	DE,8000H	; IRM-Offset
	ADD	HL,DE
	JR	C,SNERR2	; < 8000h
	BIT	6,H
	JP	Z,JLDMA		; < C000h -> in IRM schreiben
SNERR2:	JP	SNER		; SN-ERROR

; BASIC: PRINT AT:

PRAT:	LD	A,(PRTFLG)
	BIT	1,A
	SET	1,A
	LD	(PRTFLG),A
	JR	NZ,SNERR2	; SN-ERROR
	PUSH	DE
	PUSH	HL
	CALL	IRMON
	LD	HL,0		; WINON
	LD	DE,2028H	; WINLG
	CALL	WININ0		; aktuelles Fenster gross einstellen
	CALL	IRMOF
	POP	HL
	CALL	LOCAT		; Cursor positionieren
	CALL	CPBRGT		; Klammer zu?
	CALL	CPSTX		; Semikolon?
	DB	';'
	LD	A,(HL)
	CP	0DFH
	JR	C,PRAT2		; <DFH
	CP	0E3H
	JR	NC,PRAT2	; >E2H
	CALL	RBE2		; INK, PAPER, COLOR
PRAT1:	PUSH	HL
	CALL	IRMON
	LD	A,(WINNR)	; aktuelle Fenster-Nr.
	CALL	WINAKB		; gesicherte Werte zurueck holen
	CALL	IRMOF
	POP	HL
	POP	DE
	POP	BC
	RET
	;
PRAT2:	CALL	PRINT2		; Routine im BASIC-ROM aufrufen
	JR	PRAT1

; BASIC: Cursor positionieren:

LOCAT:	CALL	TCHAR		; naechstes Zeichen holen
	CALL	CPSTX		; Klammer auf?
	DB	'('
LOCATE:	CALL	ARGVL1		; 1. Wert abholen (Zeile)
	LD	D,A		; D = Zeile
	PUSH	DE
	CALL	MC8D4		; 2. Wert abholen (Spalte)
	POP	DE
	LD	E,A		; E = Spalte
	CALL	IRMON
	CALL	TCIF
	JR	C,LOCAT1	; ausserhalb
	LD	(CURSO),DE	; neue Position eintragen
LOCAT1:	CALL	IRMOF
	RET	NC		; OK
SNERR3:	JP	SNER		; SN-ERROR

; BASIC: Vordergrund- und Hintergrundfarbe einstellen:

BCOLOR:	CALL	TCHAR1
	JR	Z,FCERR		; FC-ERROR	(bisher SN-ERROR)
	CALL	INK		; Vordergrund
	CALL	TCHAR1
	RET	Z
	CALL	CPCOMM		; Komma?
;	JR	PAPER		; Hintergrund

; BASIC: Hintergrundfarbe einstellen:

PAPER:	CALL	ARGVL1		; Wert abholen
	CP	8
	JR	Z,FCERR		; FC-ERROR	(bisher SN-ERROR)
	LD	D,A		; neue Hintergrundfarbe
	CALL	LOCOL		; Farbwert lesen
	AND	0F8H		; Vordergund-Bits erhalten
LDCOL:	OR	D		; neuen Farbwert einbauen
	PUSH	HL
	LD	HL,COLOR	; Farbwert
JLDMA:	CALL	IRMWR		; in IRM schreiben
	POP	HL
	RET

; BASIC: Vordergrundfarbe einstellen:

INK:	CALL	ARGVL1		; Wert abholen
	CP	32
	JR	NC,FCERR	; FC-ERROR	(bisher SN-ERROR)
	ADD	A,A
	ADD	A,A		; *8
	ADD	A,A
	LD	D,A		; neue Vordergrundfarbe
	CALL	LOCOL		; Farbwert lesen
	AND	7		; Hintergrund-Bits erhalten
	JR	LDCOL		; neuen Farbwert eintragen

; BASIC: Punkt loeschen:

PRESET:	LD	E,2FH		; UP PUDE
	DB	1	; LD BC,nn

; BASIC: Punkt setzen:

PSET:	LD	E,30H		; UP PUSE
	PUSH	DE		; UP-Nummer retten
	CALL	EPRVL4		; Parameter erfassen
	PUSH	HL		; Adresszeiger
	LD	HL,HOR
	CALL	LDMDE		; X-Koordinate in IRM schreiben
	EX	(SP),HL
	CALL	MC8D4		; 2. Wert abholen
	EX	(SP),HL
	CALL	IRMWR		; Y-Koordinate in IRM schreiben
GRAPH2:	POP	HL		; HL=Adresszeiger
	CALL	TCHAR1		; optionaler Parameter vorhanden?
	JR	Z,GRAPH4	; nein
	CALL	MC8D4		; Wert abholen nach A
	RLA
	RLA			; Rotieren in Bits fuer Vordergundfarbe
	RLA
	PUSH	HL
	LD	HL,FARB		; Grafik-Farbe
	CALL	IRMWR		; in IRM schreiben
	POP	HL		; HL=Adresszeiger
GRAPH4:	POP	DE		; E=UP-Nr. fuer PUSE, PUDE, LINE, CIRCLE
	CALL	PV5
	RET	NC		; kein Fehler
FCERR:	JP	FCER		; FC-ERROR

; BASIC: Tonausgabe:

SOUND:	CALL	ARGVL1		; 1. Wert abholen
	PUSH	HL
	LD	HL,ARG1		; Argumente hier ablegen
	LD	B,4		; 4 Argumente holen
SOUND1:	CALL	IRMWR		; in IRM schreiben
	INC	HL
	EX	(SP),HL		; Umschaltung auf Programmzeiger
	DEC	B
	JR	Z,SOUND2	; fertig mit 4 Werten
	PUSH	BC
	CALL	MC8D4		; 2. bis 4. Wert abholen
	POP	BC
	EX	(SP),HL		; Umschaltung auf ARG-Zeiger
	JR	SOUND1
	;
SOUND2:	CALL	TCHAR1		; weiterer Parameter?
	JR	Z,SOUND3	; nein
	CALL	MC8D4		; optionalen 5. Wert (Lautstaerke) abholen
	EX	(SP),HL
	CALL	IRMWR		; und einschreiben nach ARG3
	INC	HL
	EX	(SP),HL
	CALL	TCHAR1		; weiterer Parameter?
	JR	Z,SOUND3	; nein
	CALL	MC8D4		; und einschreiben nach ARG3+1
	EX	(SP),HL
	CALL	IRMWR		; in IRM schreiben
	EX	(SP),HL
SOUND3:	POP	BC		; Stack bereinigen
	LD	E,35H		; UP-Nr. fuer TON
	JR	JPV5		; Tonausgabe aufrufen

; BASIC: F-Taste belegen:

BKEY:	CALL	ARGVL1		; Wert abholen
	CP	13		; F0 ... F12 zugelassen
	JP	NC,SNER
	LD	E,39H		; UP-Nr. fuer KEY
	DB	1	; LD BC,nn

; BASIC: F-Tasten auflisten:

KLST:	LD	E,3AH		; UP-Nr. fuer KEYLI
	DB	1	; LD BC,nn

; BASIC: Byte aus IRM lesen:

IRMRD:	LD	E,29H		; UP-Nr. fuer LDAM
JPV5:	JP	PV5

; BASIC: Byte in IRM schreiben:

IRMWR:	LD	E,28H		; UP-Nr. fuer LDMA
	JR	JPV5
 
; BASIC: String1 in String2 suchen

INSTR:	CALL	SNALY6		; Klammer auf?
	CALL	CPCOMM		; Komma?
	PUSH	HL
	CALL	LEN1		; Laenge holen
	JR	Z,FCERR		; FC-ERROR
	LD	B,A
	INC	HL
	INC	HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	POP	HL
	PUSH	DE
	PUSH	BC
	CALL	SNAlY
	CALL	CPBRGT		; Klammer zu?
	POP	BC
	POP	DE
	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	LEN1		; Laenge holen
	JR	Z,FCERR		; FC-ERROR
	INC	HL
	INC	HL
	LD	C,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,C
	POP	BC
	LD	C,A
	POP	DE
	PUSH	HL
ISTR1:	PUSH	BC
	PUSH	DE
	LD	A,(DE)
ISTR2:	CP	(HL)
	JR	Z,ISTR5
	INC	HL
	DEC	C
	JR	NZ,ISTR2
ISTR3:	XOR	A
	POP	HL
	POP	HL
	POP	HL
ISTR4:	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	JP	POS1
	;
ISTR5:	INC	HL
	PUSH	HL
	DEC	HL
ISTR6:	INC	HL
	DEC	C
	JR	Z,ISTR7
	INC	DE
	DEC	B
	JR	Z,ISTR8
	LD	A,(DE)
	CP	(HL)
	JR	Z,ISTR6
	POP	HL
	POP	DE
	LD	A,C
	POP	BC
	LD	C,A
	JR	ISTR1
	;
ISTR7:	INC	DE
	DEC	B
	POP	HL
	JR	NZ,ISTR3
	DB	16H	; LD D,n
ISTR8:	POP	HL
	POP	DE
	POP	DE
	POP	DE
	AND	A
	SBC	HL,DE
	LD	A,L
	JR	ISTR4

; BASIC: Module schalten:

SWITCH:	CALL	ARGVL1		; 1. Wert abholen
	PUSH	AF		; Steckplatz
	CALL	MC8D4		; 2. Wert abholen
	LD	D,A		; Steuerbyte
	POP	AF
	PUSH	HL
	LD	L,A		; Steckplatz
	LD	A,2		; schalten
	LD	E,26H		; UP-Nr. fuer MODU
	CALL	PV5
	POP	HL
	RET

; BASIC: 2 Werte in IRM schreiben (fuer PSET, PTEST, LINE, CIRCLE)
; PE:	DE	Wert
;	HL	IRM-Adresse
; PA:	HL	HL+2

LDMDE:	LD	A,E
	CALL	IRMWR		; Low in IRM schreiben
	INC	HL
	LD	A,D
	CALL	IRMWR		; High in IRM schreiben
	INC	HL
	RET

; BASIC: Zufallsgenerator:

RANDOM:	LD	A,R
	LD	(RNDVR3+2),A
	RET

; BASIC: Linie zeichnen:

BLINE:	LD	BC,43EH		; 4 Parameter, UP LINE
	JR	GRAPH

; BASIC: Kreis zeichnen:

CIRCL:	LD	BC,33FH		; 3 Parameter, UP CIRCLE
GRAPH:	PUSH	BC
	CALL	EPRVL4		; Parameter erfassen
	POP	BC
	PUSH	BC
	PUSH	HL
	LD	HL,ARG1		; hier ablegen
GRAPH1:	CALL	LDMDE		; Wert nach ARG in IRM schreiben
	DEC	B
	JP	Z,GRAPH2	; alle Parameter erfasst
	EX	(SP),HL
	PUSH	BC
	CALL	CPCOMM		; Komma?
	CALL	EPRVL4		; naechsten Parameter
	POP	BC
	EX	(SP),HL
	JR	GRAPH1
	;
;GRAPH2:	POP	HL
;	CALL	SUCOL		; optionale Grafikfarbe ermitteln
;	POP	DE
;	JP	PV5

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
	TOKEN	PRAT		; E1 - bis hier identisch mit BASIC des KC87
				;      bis E1 auch enthalten im RAM-BASIC KC85/1
	TOKEN	COLOR		; E2
	TOKEN	SOUND		; E3
	TOKEN	PSET		; E4
	TOKEN	PRESET		; E5
	TOKEN	BLOAD		; E6
	TOKEN	VPEEK		; E7
	TOKEN	VPOKE		; E8
	TOKEN	LOCATE		; E9 - bis hier enthalten im RAM-BASIC KC85/2
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
	TOKEN	CSRLIN		; F4 - bis hierher in CAOS 3.1 bis 4.4
	DB	80H		; Ende-Kennung
;
; zugehoerige Sprungtabelle fuer BASIC-Token
;
TOKADR:	DW	INKEY		; D5
	DW	02FDH		; D6 (JOYST)
	DW	STRING		; D7
	DW	INSTR		; D8
TADR5:	DW	RENUM		; D9
	DW	DELETE		; DA
	DW	PAUSE		; DB
	DW	BEEP		; DC
	DW	WINDOW		; DD
	DW	ARGVL1		; DE (BORDER)
	DW	INK		; DF
	DW	PAPER		; E0
	DW	SNER		; AT (Funktion AT)
	DW	BCOLOR		; E2
	DW	SOUND		; E3
	DW	PSET		; E4
	DW	PRESET		; E5
	DW	BLOAD		; E6
	DW	SNER		; E7 (Funktion VPEEK
	DW	VPOKE		; E8
	DW	LOCATE		; E9
	DW	KLST		; EA
	DW	BKEY		; EB
	DW	SWITCH		; EC
	DW	SNER		; ED (Funktion PTEST)
	DW	CLOSE		; EE
	DW	OPEN		; EF
	DW	RANDOM		; F0
	DW	SNER		; F1 (Funktion VGET)
	DW	BLINE		; F2
	DW	CIRCL		; F3
;	DW	SNER		; F4 (Funktion CSRLIN)

	DW	7F7FH
	DB	'HELP ',1
	CALL	OSTR
	DATUM			; Versionsdatum anzeigen
	DB	CR,LF,0
	RET 

	ABSFILL	0EDF7h,<ROM-E-Ende>

; High-Byte der Adresse muss konstant sein!
; je 1 Byte fuer Eingabe + Ausgabe pro Kanal

BUPTAB: DB	16H, 24H	; INTB, OCHR	Kanel #0 - Konsole
	DB	37H, 38H	; MBIN, MBOUT	Kanal #1 - Kassette
	DB	06H, 02H	; USIN1, UOT1	Kanal #2 - Anwenderkanal 2
	DB	07H, 03H	; USIN2, UOT2	Kanal #3 - Anwenderkanal 3

VERSION: DB	35H		; CAOS-Version 3.5
