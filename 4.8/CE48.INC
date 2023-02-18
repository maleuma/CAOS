;*****************************************
;**	CAOS 4.8	ROM E		**
;**					**
;**	Adresse:  E000h bis EDFFh	**
;**					**
;**	letzte Aenderung: 14.06.2021	**
;*****************************************

;	ORG	0E000H

	JP	BYE		; Tasten-RESET
	JP	BEXP1		; BASIC-Erweiterung 1 - Token starten
	JP	BEXP2		; BASIC-Erweiterung 2
	JP	BEXP3		; BASIC-Erweiterung 3
	DW	TOKTAB		; Token-Tabelle der Erweiterungen
BASIO:	JP	BASPV		; BASIC Programmverteiler

	DW	7F7FH
	DB	'BASIC',1
	CALL	BASON		; BASIC-ROM ein, CAOS-C aus, IRM aus
	JP	PRIST		; BASIC-Kaltstart

	DW	7F7FH
	DB	'REBASIC',1
	CALL	BASON		; BASIC-ROM ein, CAOS-C aus, IRM aus
	JP	SECST		; BASIC-Warmstart

; BASIC vorbereiten:
;
; Der Einsprung BASON ist bei allen CAOS-Versionen auf E02FH und kann deshalb
; innerhalb der mit BSAVE erzeugten KCB-Dateien zum Einschalten des BASIC-ROM
; genutzt werden.
; Das gilt fuer alle CAOS-Versionen ab 4.1, 4.2, 4.3, 4.4, 4.5, 4.7, 4.8

BASON:	LD	D,0
	LD	L,5		; SWITCH 5 0
	CALL	MODUSW		; CAOS-C aus
	LD	D,0C1H		; SWITCH 2 C1
	CALL	MODUL2		; BASIC-Ebene einschalten
	JR	SCROFF		; IRM aus

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

; CRT-Treiber:

PADR0:	LD	DE,(CURSO)
PADR1:	LD	HL,(WINON)
	ADD	HL,DE
	SLA	H
	SLA	H
	SLA	H
;_____________________________________________________________
;							**34**
; Berechne Pixel- und Farbadresse aus Zeichenposition
; PE:	H	Vertikalposition (0 ... FFH)
;	L	Horizontalposition (0 ... 27H)
; PA:	HL	Zeichen- und Farbadresse*
;	CY=1	Position ausserhalb
; VR:	F, HL, DE STACK: 1

PADR:	PUSH	AF
	LD	A,L		; Spalte
	LD	L,H		; Pixelzeile
	CP	28H
	JR	NC,IAD2		; zu gross
	OR	80H
	LD	H,A		; HL=Pixeladresse
IAD1:	POP	AF
OK:	AND	A		; CY=0
	RET
	;
IAD2:	POP	AF
NOK:	SCF			; CY=1
	RET
;_____________________________________________________________
;							**33**
; Test Cursor im Fenster
; PE:	D	Zeile der Cursorposition
;	E	Spalte der Cursorposition
; PA:	CY=1	Cursor ausserhalb
; VR:	AF

TCIF:	LD	A,(WINLG)	; Fenster-Breite
	SUB	1
	RET	C		; Breite=0
	SUB	E		; Cursor-Spalte
	RET	C		; ausserhalb
	LD	A,(WINLG+1)	; Fenster-Hoehe
	SUB	1
	RET	C		; Hoehe=0
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
	ADD	A,E		; Cursor-Spalte
	PUSH	DE
	LD	E,A		; absolut
	LD	A,(WINON+1)
	ADD	A,D		; Cursor-Zeile
	ADD	A,A
	ADD	A,A
	ADD	A,A		; *8
	LD	L,A		; Pixelzeile
	LD	H,0
	LD	D,H		; D=0
	ADD	HL,HL
	ADD	HL,HL		; *4
	ADD	HL,DE		; *5, zus. *40
	LD	E,A
	LD	A,(VRAM+1)
	LD	D,A
	ADD	HL,DE
	POP	DE
	JR	IAD1
;	POP	AF		; CY=0
;	AND	A
;	RET

CLSX:	LD	A,0Ch			; CLC
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
	CALL	PCHR		; Zeichen anzeigen
	LD	(CURSO),DE	; neue Cursorposition
POP4:	POP	AF
POP3:	POP	BC
	POP	DE
	POP	HL
	RET

; Zeichen auf Grafikbildschirm sichtbar machen
; PE:	DE, A, STBT

WPIX:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	C,A		; Zeichen
	LD	HL,(WINON)
	ADD	HL,DE
	EX	DE,HL		; DE:absolute Cursorposition
	LD	A,E
	CP	28H
	JR	NC,POP4		; ausserhalb!
	LD	HL,STBT
	BIT	5,(HL)		; IBM?
	JR	NZ,WPX14
	LD	A,C
	LD	HL,CCTL0
	ADD	A,A		; *2
	JR	NC,WPX04
	LD	L,LOW(CCTL2)
WPX04:	SUB	40H
	JP	P,WPX06		; CCTL0/2
	INC	L
	INC	L		; CCTL1/3
	XOR	0C0H
WPX06:	LD	C,(HL)
	INC	L		; BC=(CCTL)
	LD	B,(HL)
	ADD	A,A		; *4
	LD	L,A
	LD	H,0
WPX07:	ADD	HL,HL		; *8
	ADD	HL,BC		; HL=Pixelmuster
	LD	A,D
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	D,E
	LD	E,A
	SET	7,D		; DE=IRM-Adresse
	LD	BC,(STBT)
	BIT	1,C
	JR	NZ,WPIXNC	; Farbe AUS
	CALL	ESC9
	BIT	3,(IX+1)
	JR	NZ,WPIXA	; LORES
	BIT	6,C
	JR	NZ,WPIXH1	; HRG-Modus
WPIXA:	LD	A,B		; Farbe
	LD	B,E
	CALL	WCOLOR		; 8 Farbbytes
	LD	E,B
WPIXHE:	CALL	ESC9
WPIXNC:	BIT	0,C
	JR	NZ,WPX11	; Pixel AUS
	BIT	3,(IX+1)
	JR	NZ,WPIXB	; LORES
	BIT	6,C
	JR	NZ,WPIXH2	; HRG-Modus
WPIXB:	BIT	2,C
	CALL	Z,WPIXEL	; normal
	CALL	NZ,WPIXI	; invers
WPX11:	BIT	5,C		; IBM?
	JR	Z,POP4		; nein
	POP	AF
	CALL	OUT86		; CAOS-C wie vorher
	JR	POP4		; POP	AF,BC,DE,HL   RET

WPX14:	LD	A,(IX+4)	; CAOS-C-Schaltzustand
	PUSH	AF		; merken
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-C ein
	JP	WPIXD		; Springe zu CAOS-D-Block

; Farbebene im HRG-Modus
WPIXH1:	LD	A,B
	RRCA
	PUSH	HL
	PUSH	DE
	CALL	WPHRG
	POP	DE
	POP	HL
	JR	WPIXHE

; Pixelebene im HRG-Modus
WPIXH2:	LD	A,B
	PUSH	HL
	PUSH	DE
	CALL	WPHRG
	POP	DE
	POP	HL
	JR	WPX11

; Reaktion im HRG-Modus
WPHRG:	AND	00001001b	; Bits 0,3
	JR	Z,WCOLOR	; beide 0: Ebene ruecksetzen
	JP	PE,COLFF	; beide 1: Ebene setzen
	BIT	2,C
	JR	Z,WPHN
	XOR	00001001b	; invers
WPHN:	DEC	A
	JR	Z,WPIXI		; Pixel invers
WPIXEL:	PUSH	BC
	LDI			; 8 Pixelbytes kopieren
	LDI			; ohne Schleife - schneller!
	LDI
	LDI
	LDI
	LDI
	LDI
	LDI
	POP	BC
	RET

WPIXI:	PUSH	BC
	LD	B,8		; 8 Pixelmuster
WPIXI1:	LD	A,(HL)		; invers
	CPL
	LD	(DE),A
	INC	HL
	INC	E
	DJNZ	WPIXI1
	POP	BC
	RET

COLFF:	LD	A,0FFH
WCOLOR:	LD	(DE),A		; 8 Farbbytes
	INC	E
	LD	(DE),A
	INC	E
	LD	(DE),A
	INC	E
	LD	(DE),A
	INC	E
	LD	(DE),A
	INC	E
	LD	(DE),A
	INC	E
	LD	(DE),A
	INC	E
	LD	(DE),A
	RET

; CRT-Grundprogramm mit Auswertung von ESC-Sequenzen

PCHR:	LD	DE,(CURSO)
	LD	HL,STBT
	BIT	4,(HL)		; ESC aktiv?
	JR	Z,PCHR3
CRT1:	RES	4,(HL)		; ESC quittieren
	CP	'0'
	RET	C		; ESC-Folgecode kleiner als '0'
	CP	'9'+1
	JR	NC,PCHR1	; ESC-Folgecode groesser als '9'
	SUB	'0'
	JR	PCHR2		; ESC-Routine aus der Tabelle ermitteln
	;
PCHR1:	CP	'A'
	RET	C		; ESC-Folgecode zwischen '9' und 'A'
	RES	5,A		; upcase
	CP	'Z'+1
	RET	NC		; ESC-Folgecode groesser als 'Z'
	SUB	'A'-10
PCHR2:	LD	HL,L3SIZ	; Tabelle
	CP	(HL)		; lang genug?
	RET	NC
	LD	HL,(L3TAB)	; Tabelle der ESC-Funktionen
	JR	PCHR4		; klar zum Ansprung
	;
PCHR3:	CP	20H		; Steuerfunktion?
	JR	NC,PCHR5	; nein, darstellbares Zeichen
	BIT	3,(HL)		; Steuerzeichen darstellen?
	JR	NZ,PCHR5	; ja, nicht ausfuehren
	LD	HL,(CTAB)	; Tabelle der CRT-Funktionen
PCHR4:	ADD	A,A		; *2
	LD	C,A
; 08.03.2020:
; Dieser Test sorgt dafuer, dass auch kein Steuerzeichen ausgefuehrt wird,
; wenn der Cursor nicht im Fenster ist oder das Fenster 0x0 gross ist.
; Dann wird aber auch kein HOME oder CLS ausgefuehrt, was problematisch ist.
; 09.03.2020: wieder verworfen
;	CALL	TCIF		; Cursor im Fenster?
;	RET	C		; nein, Abbruch
	LD	B,0
	ADD	HL,BC
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
JPHL:	JP	(HL)		; anspringen

; Tabulatorschritt mit Code 05h bzw. ESC '0'
ESC0:	LD	A,E		; Tabulator
	OR	7		; 8er Schrittweite
	INC	A
	LD	E,A
	JR	CUR1
	;
PCHR5:	CALL	DABR		; VRAM-Adresse berechnen
	RET	C		; Abbruch, da ausserhalb des Fensters
	LD	(HL),A		; ASCII in VRAM eintragen
	CALL	WPIX		; Zeichen darstellen
CUR:	INC	E		; Cursor nach rechts
CUR1:	LD	A,(WINLG)
	DEC	A
	CP	E		; Zeilenende?
	RET	NC

; Neue Zeile mit Code 1Eh
NL:	LD	E,0		; Cursor ganz nach links
CUD:	INC	D		; naechste Zeile
	LD	A,(WINLG+1)
	CP	D
	RET	NZ
	LD	HL,(WEND)
	JP	(HL)		; PAGE oder SCROLL

; Aufruf der Hardcopy-Routine mit Code 0Fh
HCOPY:	LD	HL,(HCADR)	; ShCLR
	JP	(HL)
;
; Cursor nach links:
;
CUL:	LD	A,E		; Spalte
	AND	A		; ganz links?
	JR	Z,CUL1
	DEC	E		; eine Position nach links gehen
	RET
	;
CUL1:	LD	A,D		; Zeile
	AND	A		; ganz oben?
	RET	Z
	DEC	D		; eine Zeile nach oben gehen
CEL:	LD	A,(WINLG)	; Spalten im aktuellen Fenster
	DEC	A		; letzte Spalte
	LD	E,A		; Cursor auf letzte Spalte setzen
	RET
;
; Cursor nach oben:
;
CUU:	LD	A,D		; Zeile
	AND	A		; bereits ganz oben?
	RET	Z
	DEC	D		; eine Zeile nach oben gehen
	RET

PAGEM:	LD	HL,HOMEPG	; ShCUU
WADR:	LD	(WEND),HL
	RET

SCROL:	LD	HL,SCRLPG	; ShCUD
	JR	WADR

; Zeichen nach links loeschen mit Code 01h
CLR:	CALL	CUL		; Cursor nach links

; Zeichen unter Cursor loeschen mit Code 1Fh
DEL:	PUSH	DE		; Cursor retten
	CALL	DABR
	LD	A,(HL)
	AND	A		; Ende?
	JR	Z,POPDE
	PUSH	HL		; VRAM
DEL1:	PUSH	DE		; Cursor
	INC	E		; naechste Spalte
	CALL	DABR		; raus?
	JR	NC,DEL2
	LD	E,0		; ja, wie NL
	INC	D
	CALL	DABR		; raus?
	JR	C,DEL3		; ja-wie Ende
DEL2:	LD	A,(HL)
	AND	A		; Ende?
	JR	Z,DEL3
	LD	B,D
	LD	C,E		; BC:=DE
	POP	DE		; DE=links davon
	EX	(SP),HL		; HL=VRAM links
	LD	(HL),A		; eintragen
	CALL	WPIX		; zeichnen
	LD	D,B
	LD	E,C		; DE:=BC
	JR	DEL1		; von vorn

DEL3:	POP	DE
	POP	HL
	LD	(HL),0
	LD	A,' '
	CALL	WPIX
POPDE:	POP	DE
	RET

INS:	PUSH	DE		; Cursor
	LD	A,' '
	CALL	DABR
INS1:	LD	B,(HL)		; VRAM
	LD	(HL),A		; neues Zeichen
	CALL	WPIX		; zeichnen
	LD	A,B
	AND	A		; Dummy?
	JR	Z,INS2
	INC	E		; CUR
	CALL	DABR		; raus?
	JR	NC,INS1
	LD	E,0		; wie NL
	INC	D
	CALL	DABR		; raus?
	JR	NC,INS1
INS2:	POP	DE		; wenn ja Schluss
	RET

; Bildschirm loeschen mit Code 0Ch (ShHOME)
CLS:	LD	A,(WINLG+1)	; Anzahl Zeilen
	LD	D,0		; Zeile 0
CLS1:	PUSH	AF
	CALL	CLLN		; Zeile loeschen
	POP	AF
	INC	D
	DEC	A
	JR	NZ,CLS1		; A Zeilen

; Cursor in Home-Position mit Code 10h
HOMEPG:	LD	D,0		; Zeile 0
CBL:	LD	E,0		; Spalte 0
	RET

; Scrollroutine
SCRLPG:	LD	A,(IX+4)	; CAOS-C-Schaltzustand
	PUSH	AF		; merken
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-C ein
	CALL	SCRLD		; weiter im CAOS-D-Segment
	JR	CCRES

; Zeile loeschen mit Code 02h (ShDEL)
CLLN:	LD	A,(IX+4)	; CAOS-C-Schaltzustand
	PUSH	AF		; merken
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-C ein
	CALL	CLLND		; weiter im CAOS-D-Segment
CCRES:	POP	AF
OUT86:	LD	(IX+4),A	; CAOS-C
	OUT	(PORT2),A	; wie vorher
	RET

BEEP:	LD	BC,0A0FH	; B=Dauer, C=Lautstaerke
	LD	HL,48		; HL=Tonhoehe 1
	PUSH	DE
	LD	E,H		; E=0 (keine Tonhoehe 2)
	CALL	TON2
	LD	A,30
	CALL	WAIT		; 30*6ms=180ms
	LD	A,3
	OUT	(CTC0),A
	LD	A,16		; 16*6ms=96ms
	POP	DE
;
; Zeitschleife 6ms fuer UP WAIT
; PE:	A=0
; PA:	A=0
;
WTUP:	DEC	A		; 4 Takte
	RET	Z		; 5 (11) Takte
	PUSH	AF		; 11 Takte
	POP	AF		; 10 Takte
	JR	WTUP		; 12 Takte
;
; Ton fuer Tastenclick
;
CLIK:	LD	BC,0307H	; B=Dauer, C=Lautstaerke
	LD	HL,36		; HL=Tonhoehe 1
	PUSH	DE
	LD	E,H		; E=0 (Tonhoehe 2)
	CALL	TON2		; Ton ausgeben
	POP	DE
	RET

ESC8:	LD	HL,COLOR	; Farbtausch
	LD	A,(HL)
	AND	00000111b	; Hintergrund
	RLCA
	RLCA
	RLCA
	LD	C,A
	LD	A,(HL)
	AND	00111000b	; Vordergrund
	RRCA
	RRCA
	RRCA
	OR	C
	LD	C,A
	LD	A,(HL)
	AND	11000000b	; Attribute
	OR	C
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

CLICK:	LD	A,(IX+8)	; ShINS
	XOR	20H
	LD	(IX+8),A
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
	LD	A,23H	  	; DI, ZG256, Reset
	OUT	(CTC3),A
	LD	(IX+13),0	; Zeichen tot
	JR	TST5		; freigeben
;
;*__*__*__*__*__*__*__*__*__*__*__*__*__
;
; Keyboard-Interrupt-Routine
; Zeitkonstanten:
;
TCTZK	EQU	143	; 8FH
TGE0	EQU	101	; 65H
TGE1	EQU	66	; 42H
FEHL1	EQU	20
FEHL2	EQU	120
;
; T A S T A T U R I N T E R R U P T
;
; ISR PIO Kanal B (Tastatur)
; Taste gedrueckt
;
ISRPB:	PUSH	AF
	IN	A,(CTC3)	; gemessene Zeit
	PUSH	AF
	LD	A,0A7H		; EI, ZG256, Reset
	OUT	(CTC3),A
	LD	A,TCTZK		; Zeitkonstante
	OUT	(CTC3),A
	POP	AF
	EI
	CP	FEHL1		; neue Betaetigung? (Zeitinterrupt ausgeloest)
	JR	C,TST5		; oder zu lang
	CP	FEHL2
	JR	NC,TST5		; zu kurz
	CP	TGE0		; Diskriminator
	JR	NC,TST1		; 0-Bit
	ADD	A,-TGE1		; 1-Bit
	JR	NC,TST2		; Stop-Bit
TST1:	RR	(IX+12)		; Bit einschieben
	JR	TST5
	;
TST2:	PUSH	HL
	PUSH	DE
	LD	A,(IX+12)	; Wortregister
	RRA			; 7bit-Scancode
	XOR	1		; Startbit invertieren
;
; in CAOS 5.0/6.0 wird Shift-BRK hier zu Soft-Reset abgefragt
; das koennte hier so aussehen:
;
;	CP	61		; Scancode Shift-BRK
;	JR	NZ,IBRK
;IBRK:	CALL	IRET		; Interrupts freigeben
;	JP	BYE		; Sprung zu RESET
;IBRK:
	LD	L,(IX+14)
	LD	H,(IX+15)	; aktuelle KTAB-Adresse
	LD	D,0
	LD	E,A
	ADD	HL,DE		; Position in KTAB
	LD	A,(HL)		; ASCII holen
	BIT	7,(IX+8)	; CAPS aktiv?
	JR	NZ,TST3		; 1=nein
	CP	40H
	JP	M,TST3		; Befehl aus CAOS 3.4 uebernommen
	XOR	20H		; klein<=>gross
TST3:	LD	D,A		; Code merken
	CP	(IX+13)		; gleicher Code wie zuvor?
	JR	NZ,TST6		; nein
	LD	HL,COUNT
	CALL	IRMGET
	CP	(IX+10)		; FastRepeat?
	JR	C,TST7
	INC	(IX+10)		; Zeit abwarten
TST4:	POP	DE
	POP	HL
TST5:	IN	A,(PIOBD)	; PIO-Logik
	OUT	(PIOBD),A	; freigeben
	POP	AF
	RETI

TST6:	LD	(IX+10),0	; neuer Code
	CP	16H		; CAPS?
	JR	NZ,TST8
	LD	A,(IX+8)
	XOR	80H		; CAPS on/off
	LD	(IX+8),A
TST7:	LD	A,D		; Code
TST8:	LD	(IX+13),A	; eintragen
	SET	0,(IX+8)	; gueltig machen
	JR	TST4
;_____________________________________________________________
;							**0C**
; Tastaturabfrage ohne Quittung

KBDS:	OR	A		; CY=0
	BIT	0,(IX+8)	; Code gueltig?
	RET	Z		; nein
	LD	A,(IX+13)	; Tastencode holen
	SCF			; und mit CY=1 zurueck melden
	RET
;_____________________________________________________________
;							**0E**
; Tastaturabfrage mit Quittung

KBDZ:	CALL	KBDS		; Tastaturabfrage
	RET	NC		; keine Taste gedrueckt
	RES	0,(IX+8)	; Taste quittieren
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
;
; Arbeitszelle aus IRM lesen (innerhalb eines Interrupt)
; PE:	HL	Adresse
; PA:	A	Inhalt
; VR:	A
;
IRMGET:	PUSH	DE
	IN	A,(PIOAD)
	LD	E,A		; IRM-Schaltzustand in Reg. E merken
	SET	2,A		; IRM on!
	DI			; damit Stack nicht genutzt wird!
	OUT	(PIOAD),A
	LD	D,(HL)		; Arbeitszelle lesen
	LD	A,E
	OUT	(PIOAD),A	; IRM wie zuvor
	EI
	LD	A,D		; Wert nach A uebergeben
	POP	DE
	RET
;
; ISR fuer Fremdtastatur an SIO-B
;
ISRT:	PUSH	AF
	PUSH	HL
	PUSH	DE
	PUSH	BC
	LD	HL,V24PL	; Steckplatz M003
	CALL	IRMGET		; aus IRM lesen, Rueckkehr mit EI
	LD	B,A		; Steckplatz M003 holen
	LD	C,80H
	LD	A,1
	OUT	(C),A		; Modul einschalten
	IN	A,(9)		; empfangenen Code abholen
	LD	E,A		; Code in Reg. E merken
	AND	A
	JR	Z,IST2		; Code 0 (Taste losgelassen)
	CP	(IX+13)		; 2x gleicher Tastencode?
	LD	A,0		; kein Repeat
	JR	NZ,IST2
	LD	HL,COUNT	; Repeat-Counter
	CALL	IRMGET		; aus IRM lesen
	INC	A		; sofort aktivieren
IST2:	LD	(IX+10),A	; repeat setzen
	LD	A,E		; gemerkten
	LD	(IX+13),A	; Code eintragen
	AND	A		; Code=0 ?
	JR	Z,IST3		; ja
	SET	0,(IX+8)	; sonst aktivieren
IST3:	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	L,B		; Steckplatz
	CALL	IRMGET		; Schaltzustand vor ISR ermitteln
	OUT	(C),A		; und wiederherstellen
	POP	BC
	JR	IPOP3		; POP DE,HL,AF; RETI

;	=== Joystick-Treiber fuer M008/M021 ===

ISRJ:	PUSH	AF
	IN	A,(90h)		; M008 PIO A Daten
	EI
	CPL
	PUSH	HL
	PUSH	DE
	AND	00111111b	; Achsbewegung und/oder Feuertaste?
	JR	Z,ISRJ4
	CP	00010000b	; Feuertaste?
	JR	C,ISRJ1
	AND	00110000b	; Ignoriere Achse: Feuertasten haben Vorrang
	RRCA			; Nutze Luecken zwischen den Achsrichtungen in
	RRCA			; der Tastaturcode-Tabelle fuer Feuertasten
	XOR	00001111b	; A=0000xx11
ISRJ1:	PUSH	BC
	LD	C,A		; Tabellenindex
	LD	B,0
	IN	A,(PIOAD)
	LD	D,A
	OR	00000100b	; IRM ein
	DI
	OUT	(PIOAD),A
	LD	HL,(JOYTAB)	; Adresse der Codetabelle fuer Joystick
	LD	E,(HL)		; Wartezyklen waehrend Tastenwiederholung
	ADD	HL,BC		; Tastaturcode nachschlagen
	LD	L,(HL)
	LD	A,(COUNT)	; Wartezyklen vor erster Wiederholung
	LD	H,A		; (entsprechend Abfragerate der Tastatur)
	LD	A,D
	OUT	(PIOAD),A	; IRM-Zustand wiederherstellen
	EI
	POP	BC
	LD	A,01001111b	; Byte-Eingabemodus (mit Strobe-Interrupt)
	OUT	(92h),A		; M008 PIO A Steuerwort
	LD	A,L
	AND	A		; Tastencode zugewiesen?
	JR	Z,IPOP3
	CP	(IX+13)		; neuer Tastencode?
	JR	Z,ISRJ5
	LD	(IX+13),A	; Tastencode eintragen
	XOR	A
ISRJ2:	SET	0,(IX+8)	; gueltig markieren
ISRJ3:	LD	(IX+10),A	; Zaehler zuruecksetzen
IPOP3:	POP	DE
	POP	HL
	POP	AF
	RETI

ISRJ4:	CP	(IX+10)		; vorheriger Code war neu? (A war 0)
	INC	A
	JR	NC,ISRJ3	; dann zum Entprellen einen Zyklus warten
	LD	HL,JOYINI
	DI
	CALL	INIEA		; Ruhestellung: Strobe-Interrupt wieder aus
	EI
	XOR	A
	LD	(IX+13),A	; Tastencode loeschen
	RES	0,(IX+8)	; ungueltig markieren
	JR	ISRJ3

ISRJ5:	LD	A,H		; Wartezyklen vor erster Wiederholung
	ADD	A,A		; an Abfragerate des Joysticks anpassen
	ADD	A,A		; (COUNT sollte kleiner als 64 sein)
	CP	E		; weniger als waehrend Wiederholung?
	JR	NC,ISRJ6
	LD	A,E		; Zaehlerende = max(A,E)
ISRJ6:	LD	H,A
	LD	A,(IX+10)
	CP	H		; Zaehlerende erreicht?
	INC	A
	JR	C,ISRJ3
	LD	A,H		; Zaehler = Endwert - Wartezyklen
	SUB	E
	JR	ISRJ2		; erneut gueltig markieren
;
; Initialisierung vor Kassetten-I/O
;
ISRI1:	LD	(IX+3),1	; 1. Block lesen
	DI
	IN	A,(PIOAD)
	OR	01000000b	; Motor ein
	AND	11011111b	; LED aus
ISRO1:	OUT	(PIOAD),A
	EI
	LD	HL,(WEND)	; Page/Scroll-Modus
	LD	(ZWEND),HL	; hier merken
	CALL	NOUT		; CRT-Ausgabe (kein Drucker!)
	LD	(ZOTAB),HL	; alten Zeiger hier merken
	CALL	PAGEM		; Page Mode
TOFF:	LD	A,3		; CTC Stop
	OUT	(CTC0),A	; Tonhoehe Kanal 1
	OUT	(CTC1),A	; Tonhoehe Kanal 2
	RES	1,(IX+8)	; Ton ist jetzt aus
	JR	CSRO2
;_____________________________________________________________
;							**09**
; Abschluss Bandausgabe
; PA:	CY=0

TCSRO:	LD	(IX+2),0FEH	; FFH = Kennung Ende-Block
	CALL	TMBO		; Block auf Kassette ausgeben
;_____________________________________________________________
;							**0B**
; Abschluss Bandeingabe
; PA:	CY=0

TCSRI:	LD	HL,(ZOTAB) 	; intern
	LD	(OUTAB),HL
	LD	HL,(ZWEND)	; grosses
	LD	(WEND),HL	; Ruecksetzen
;	CALL	CLC		; Puffer loeschen (VR: HL)
	IN	A,(PIOAD)
	AND	10011111b	; Motor, LED aus
	OUT	(PIOAD),A
;	CALL	CRLF		; VR: AF
	RES	0,(IX+8)	; kein Tastencode im Puffer
CSRO2:	BIT	3,(IX+1)	; HiRes?
	JR	Z,CSRO3
	IN	A,(PIOBD)	; nein
	SET	7,A		; Blinken ein
	OUT	(PIOBD),A
CSRO3:	BIT	1,(IX+8)	; neuer Ton?
	JR	NZ,CSRO3	; Ende abwarten
	IN	A,(PIOAD)
	LD	L,A		; Schaltzustand IRM merken
	SET	2,A		; IRM on
	DI			; Interrupt verbieten wegen Stacknutzung
	OUT	(PIOAD),A
	LD	A,(CTCMD)	; CTC2-Modus
	AND	01100000b	; nur Zaehler/Zeitgeber und Vorteiler verwenden
	OR	00000111b	; immer RESET, Zeitkonstante laden
	OUT	(CTC2),A	; Blinken
	LD	A,(BLINK)	; ZK laden
	OUT	(CTC2),A
	LD	A,L
	OUT	(PIOAD),A	; IRM wie zuvor einstellen
EIRET:	EI			; Interrupt wieder ein
	RET
;
; Kassettenpuffer loeschen:
; PA:	DE = CASS
;	A = 0
; VR:	DE,A
;
CLC:	LD	DE,CASS+80H
	XOR	A
CLC1:	DEC	E
	LD	(DE),A
	JR	NZ,CLC1
	RET

; Aufruf Treiberroutinen aehnlich PV1 mit CALL PV7, DB nr.
; Nr.	 0 = MBO
;	 1 = MBI
;	 2 = ISRO
;	 3 = CSRO
;	 4 = ISRI
;	 5 = CSRI
;	 6 = MBIN - nicht mehr erforderlich
;	 7 = MBOUT - nicht mehr erforderlich
;	 8 = DIR
;	 9 = CD
;	10 = ERA
;	11 = REN
; PE:	entsprechend der aufgerufenen Routine
; PA:	entsprechend der aufgerufenen Routine
;	CY=1	Fehler (Nr. groesser 11)
; VR:	-
;										auf Stack liegt:
PV7:	PUSH	AF		;						 RET >AF<
	PUSH	AF		;						 RET  AF   >AF<
	DI			; wegen Stack-Operation sperren
	POP	AF		; und wieder bereinigen				 RET >AF<   AF
	POP	AF		;						>RET< AF    AF
	EX	(SP),HL		; HL = RET-Adresse wo CALL herkommt		>HL<  AF    AF
	LD	A,(HL)		; nr. lesen
	INC	HL		; nr uebergehen
	EX	(SP),HL		; neue RET-Adresse nach Datenbyte		>RET< AF    AF

	ADD	-12		; Treiberfunktion > 11?
	JR	C,EIRET		; Abbruch bei Fehler => EI, RET von CRSO3, PA:CY=1
	CALL	PV70		; Modul ein und Funktion ausfuehren		 RET >MOFF< AF

; 3. Module nach Treiberaufruf wieder schalten wie zuvor

MOFF:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF

	; hier alle davor liegenden Speichermodule wieder einschalten?

	LD	HL,MODST+2	; SWITCH 2 n
	LD	D,(HL)		; aktuelles Steuerbyte
	CALL	MODUSW		; USER-ROM wie vorher schalten
	CALL	DEVH		; DEVICE-Nr. in Bit 5-7, (HL)=Sprungadresse
	OR	2		; DEVTAB+2
	LD	L,A
	LD	D,(HL)		; Steuerbyte fuer AUS
	DEC	L
	LD	A,(HL)		; Steckplatz

	LD	L,A		; Steckplatz in L
	CP	5		; CAOS-ROM C?
	jr	z,m4
	CALL	MODUSW		; Modul ausschalten (falls nicht CAOS-ROM C)
	ld	a,(modst+5)	; vorheriger Schaltzustand CAOS-C
	and	1		; ohne Merkbit 7
	ld	d,a
m4:	LD	HL,MODST+5	; SWITCH 5 n
	LD	A,(HL)		; vorheriger Schaltzustand in Bit 7
	LD	(HL),D		; urspruenglicher Schaltzustand vor PV7
	AND	80h		; nur Bit 7 betrachten
	LD	A,(IX+4)
	RES	7,A		; CAOS-C aus
	JR	Z,M5		; es war auch aus!
	SET	7,A		; CAOS-C ein
M5:	CALL	OUT86		; CAOS-C wie vor Aufruf PV7
MSW:	JP	POP4

; 1. Sprungadresse von Treiber berechnen aus Nr. und IX+8

PV70:	DEC	SP
	DEC	SP		; SP ist wieder richtig				RET  MOFF >AF<
	EI
PV71:	EX	(SP),HL		;						RET  MOFF >HL<
	PUSH	HL		;						RET  MOFF  HL  >AF<
	ADD	A,12+4		; 12 UP-Nummern + 2*4 Byte Kopfdaten
	ADD	A,A		; 2 Byte je Sprungadresse
	LD	L,A		; Aufrufadresse der Treiber-Routine
	CALL	DEVH		; DEVICE-Nr. in Bit 5-7, (HL)=Sprungadresse
	OR	L		; Adresse einbauen in Bit 0-4
	LD	L,A
	LD	A,(HL)		; Sprungadresse Low
	INC	HL
	LD	H,(HL)		; Sprungadresse High
	LD	L,A		; HL=Sprungadresse der Routine
	POP	AF		;						RET  MOFF >HL<
	EX	(SP),HL		; Sprungadresse auf Stack			RET  MOFF >JPADR<

; 2. Module passend schalten nach IX+8 (DEVTAB+1 = Steckplatz, DEVTAB+3 = EIN)

	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	CALL	DEVH		; DEVICE-Nr. in Bit 5-7, (HL)=Sprungadresse
	OR	1		; DEVTAB+1
	LD	L,A
	LD	E,(HL)		; Steckplatz

	; hier alle davor liegenden Speichermodule ausschalten?

	LD	D,High(MODST)
	LD	A,(DE)		; aktueller Schaltzustand
	INC	L
	LD	(HL),A		; als Schaltzustand AUS in DEVTAB+2 eintragen
	INC	L		; DEVTAB+3

	LD	A,(HL)		; Steuerbyte EIN
	LD	(DE),A		; in MODST eintragen
	LD	D,A		; Steuerbyte

	LD	A,E		; Steckplatz
	LD	E,0		; AUS fuer CAOS-C
	CP	5		; CAOS-ROM-C?
	JR	NZ,MO1
	LD	E,80h		; EIN fuer CAOS-C
MO1:	CP	7
	JR	C,MO2		; intenes Modul
	LD	C,80h
	LD	B,A
	OUT	(C),D		; Steuerbyte zu Modul senden

MO2:	IN	A,(PIOAD)	; SWITCH 2 0:
	AND	7Fh		; USER-ROM C immer aus!
	OUT	(PIOAD),A

	LD	HL,MODST+5	; SWITCH 5
	LD	A,(IX+4)
	AND	80h		; aktueller Schaltzustand CAOS-ROM
	OR	(HL)		; in Modulsteuerbyte Bit 7 einbauen
	LD	(HL),A		; und dort merken
	LD	A,(IX+4)	; SWITCH 5 0:
	AND	7Fh		; CAOS-ROM C aus
	OR	E		; oder EIN falls Programm dort steht!
	JR	M5		; CAOS-C und Routine aufrufen
;
; ISR CTC Kanal 1 (Kassette-Out)
;
ISRC1:	PUSH	AF
	LD	A,10000111b	; EI,ZG16,Res
	OUT	(CTC1),A
	LD	A,(IX)		; Uebergabezelle Zeitkonstante
	OUT	(CTC1),A	; Start CTC
	LD	(IX),0		; Quittierung
	JR	ISR1E		; EI, RETI
;
; ISR PIO Kanal A (Kassette-In)
;
ISRPA:	PUSH	AF
	IN	A,(CTC2)	; CTC lesen
	LD	(IX),A		; und in Uebergabezelle ablegen
	LD	A,7
	OUT	(CTC2),A	; CTC Neustart
	LD	A,IKEZK
	OUT	(CTC2),A
ISR1E:	POP	AF
IRET:	EI
	RETI

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
	CP	(TOKJPE-TOKJP)/2-4	; 27 waren es Original
	JR	NC,SNERR1	; Token ausserhalb
	RLCA			; *2
	LD	C,A
	LD	B,0		; BC = Offset in Adresstabelle
	EX	DE,HL
	LD	HL,TADR5	; Adresse der Erweiterungstoken
	JP	DLI22		; Ruecksprung zu Tokenverarbeitung (BASIC-ROM)

LOCOL:	;LoadColor
	PUSH	HL
	LD	HL,COLOR	; 0B7A3H
	CALL	IRMRD
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
	JR	NZ,SNERR1
	INC	A
	LD	(PRTFLG),A
PREX1:	CALL	LOCOL		; Farbwert lesen
	LD	(COLRET),A 	; Farbe merken
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

PRCOL:	CALL	TCHAR		; naechstes Zeichen holen
	CALL	BCOLOR		; Vordergrund- und Hintergrundfarbe
	JR	PREX2

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
	CALL	PAPER
	CALL	CPSTX		; Semikolon?
	DB	';'
PREX3:	CALL	PRINT2		; Routine im BASIC-ROM aufrufen
	LD	A,(COLRET)	; Farbe
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMWR		; eintragen
	POP	HL
	POP	BC
	RET

PREX4:	PUSH	BC
	JR	PREX1

PREX5:	CALL	TCHAR		; naechstes Zeichen holen
	JR	PREX3

; BASIC: String vervielfaeltigen

STRING:	CALL	CPSTX		; Klammer auf?
	DB	'('
	CALL	ARGVL1		; Wert abholen
	PUSH	AF
	CALL	CPCOMM		; Komma?
	CALL	SNALY
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
REN01:	POP	HL
	PUSH	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	CALL	CPREG
	EX	(SP),HL
	JR	NZ,REN01
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
REN02:	JR	Z,REN05
	CALL	DCHEX
	PUSH	AF
	LD	A,D
	OR	E
REN03:	JP	Z,FCER	 	; FC-ERROR
	POP	AF
	EX	(SP),HL
	LD	(HL),E
	INC	HL
	LD	(HL),D
	INC	HL
	JR	Z,REN05
	PUSH	AF
	DEC	B
	JR	Z,REN04
	POP	AF
	EX	(SP),HL
	CALL	CPCOMM		; Komma?
	JR	REN02
	;
REN04:	POP	AF
	JP	NZ,SNER		; SN-ERROR
REN05:	LD	HL,(ENDE)
	LD	DE,(ANF)	; ANFANG
	CALL	CPREG
REN06:	JP	C,FCER		; FC-ERROR
	LD	HL,(PSTBEG)
REN07:	CALL	ZPOIT1		; naechste Zeile
	JR	C,REN08
	JR	Z,REN03
	JR	REN07
	;
REN08:	POP	HL
	PUSH	BC
	LD	DE,(ENDE)
	LD	HL,0
	LD	(ENDE),HL
REN09:	LD	H,B
	LD	L,C
	LD	C,(HL)
	INC	HL
	LD	B,(HL)
	LD	A,B
	OR	C
	JR	Z,REN03
	INC	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	CALL	CPREG
	LD	HL,(ENDE)
	INC	HL
	LD	(ENDE),HL
	JR	NZ,REN09
	INC	HL
	INC	HL
	ADD	HL,HL
	INC	HL
	LD	DE,(SVARPT)
	ADD	HL,DE
	JR	C,REN06
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
REN14:	CALL	DCHEX1
	LD	A,E
	OR	D
	CALL	NZ,RNUP1 	; Vergleich mit Tabelle
	CALL	NZ,RNUP2	; Zahl aendern
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
REN16:	CALL	DCHEX1
	LD	A,E
	OR	D
	JR	Z,REN13
	CALL	RNUP1
	CALL	NZ,RNUP2
	LD	A,(HL)
	CP	','
	JR	NZ,REN13
	JR	REN16

RNUP1:	PUSH	HL
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
RUP11:	LD	A,(HL)
	INC	HL
	PUSH	HL
	OR	(HL)
	JR	Z,RUP13
	LD	A,(HL)
	DEC	HL
	LD	L,(HL)
	LD	H,A
	CALL	CPREG
	JR	Z,RUP12
	LD	HL,(DISTAN)
	ADD	HL,BC
	LD	B,H
	LD	C,L
	POP	HL
	INC	HL
	JR	RUP11
	;
RUP12:	XOR	A		; aus CAOS 3.4 uebernommen
	DEC	A		; A=0FFh, Z=0
;	LD	A,0FFH
;	OR	A
RUP13:	POP	HL
	POP	HL
	RET

RNUP2:	PUSH	BC
	EX	DE,HL
	LD	HL,(SVARPT)
	SBC	HL,DE
	PUSH	HL
RUP21:	POP	BC
	LD	H,D
	LD	L,E
	DEC	DE
	LD	A,(DE)
	CP	','
	JR	Z,RUP22
	CP	':'
	JR	NC,RUP22
	PUSH	BC
	PUSH	DE
	LDIR
	POP	DE
	JR	RUP21
	;
RUP22:	EX	DE,HL
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
RUP23:	LD	A,(HL)
	OR	A
	JR	Z,RUP24
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
	JR	RUP23
	;
RUP24:	PUSH	DE
	LD	DE,(PSTBEG)
	CALL	LIN11
RUP25:	INC	HL
	LD	A,(HL)
	INC	HL
	OR	(HL)
	JR	NZ,RUP25
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
RUP26:	LD	A,(HL)
	OR	A
	INC	HL
	JR	NZ,RUP26
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
	CALL	ZPOIT1
	POP	BC
	JP	C,LIN6		; einsortieren
DELET1:	JP	ULER		; UL-ERROR

; BASIC: Programm unterbrechen

PAUSE:	CALL	TCHAR1		; mit oder ohne Paramater?
	JR	NZ,PAUS2
PAUS1:	CALL	KBDS		; Tastaturabfrage
	JR	NC,PAUS1
	CP	3		; BRK?
	RET	Z
	CP	LF		; CUU?
	JR	NZ,PAUS1
	JP	CI		; Eingabe ASCII
	;
PAUS2:	CALL	ARGVL1		; Wert abholen
	LD	C,A
PAUS3:	LD	A,16		; 16*6ms = 96 Millisekunden
	CALL	WAIT		; UP WAIT direkt aufrufen
;	LD	E,14H		; WAIT
;	CALL	PV5
	CALL	KBDS
	JR	NC,PAUS4
	CP	3		; BRK?
	RET	Z
	CP	LF		; CUU?
	JR	NZ,PAUS4
	JP	CI		; Eingabe ASCII
	;
PAUS4:	DEC	C		; Zaehler verringern
	JR	NZ,PAUS3	; weiter warten
	RET

; BASIC: Signalton

BBEEP:	LD	B,1		; ohne Angabe 1x
	CALL	TCHAR1		; Parameter angegeben?
	JR	Z,BBP1
	CALL	ARGVL1		; Wert abholen
	LD	B,A		; Anzahl
BBP1:	LD	A,7		; Beep
	LD	E,0		; CRT
	CALL	PV5
	DJNZ	BBP1
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
	LD	A,(WINNR)	; aktuelle Fensternummer
	CALL	WININ		; Fenster initialisieren
	CALL	IRMOF
	JR	C,SNERR2	; Fehler aufgetreten
	POP	HL
	RET
	;
WDW2:	PUSH	HL		; Standardfenstergoesse bei BASIC:
	LD	HL,100H		; 30 Zeilen, 40 Zeichen breit
	LD	DE,1E28H	; (oben und unten eine Zeile ungenutzt)
	JR	WDW1

; BASIC: in IRM schreiben (8000h bis BFFFh):

VPOKE:	CALL	EPRVL4		; Parameter erfassen (Adresse)
	PUSH	DE
	CALL	MC8D4		; Wert abholen
	EX	(SP),HL
	LD	DE,8000H	; IRM-Offset
	ADD	HL,DE
	JR	C,SNERR2	; < 8000h
	BIT	6,H
	JP	Z,LOC1		; < C000h -> in IRM schreiben
SNERR2:	JP	SNER		; SN-ERROR

; BASIC: PRINT AT:

PRAT:	LD	A,(PRTFLG)
	BIT	1,A
	SET	1,A
	LD	(PRTFLG),A
	JR	NZ,SNERR2
	PUSH	DE
	PUSH	HL
	CALL	IRMON
;
; seit CAOS 3.1 wurden die aktuellen Fensterdaten im Fenstervektorspeicher des
; Fensters Nr. 8 und 9 zwischengespeichert und diese Werte somit ueberschrieben.
; CAOS 3.4 machte es vor, dass es besser geht. Hier werden die aktuellen Werte
; im Fenstervektorspeicher des aktuellen Fensters gesichert
; 29.12.2020: Funktionalitaet fuer CAOS 4.8 uebernommen
;
	; im Fenstervektorspeicher sichern
	LD	A,(WINNR)	; aktuelle Fenster-Nr.
	LD	HL,0		; WINON
	LD	DE,2028H	; WINLG
	LD	BC,WININB	; WINON=HL, WINLG=DE, CURSO=BC einstellen
	CALL	CLROMC		; im ROM-C ausfuehren
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
	CALL	PREX4		; INK, PAPER, COLOR
PRAT1:	PUSH	HL
	CALL	IRMON
	LD	A,(WINNR)	; aktuelle Fenster-Nr.
	LD	BC,WINAKB	; gesicherte Werte zurueck holen
	CALL	CLROMC		; im ROM-C ausfuehren
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
LOCATE:	CALL	ARGVL1		; Wert abholen
	LD	D,A
	PUSH	HL
	LD	HL,WINLG+1
	CALL	IRMRD
	POP	HL
	DEC	A
	CP	D
	JR	C,SNERR3
	PUSH	DE
	CALL	MC8D4		; Wert abholen
	POP	DE
	LD	C,A
	PUSH	HL
	LD	HL,WINLG
	CALL	IRMRD		; Fenstergroesse lesen
	POP	HL
	DEC	A
	CP	C
	JR	C,SNERR3
	LD	A,C
	PUSH	HL
	LD	HL,CURSO
	CALL	IRMWR
	LD	A,D
	INC	HL
LOC1:	CALL	IRMWR		; in IRM schreiben
	POP	HL
	RET

; BASIC: Eingabe eines Zeichens:

INKEY:	PUSH	HL
	CALL	KBDS		; Tastaturabfrage
	JR	NC,INKEY3
	LD	A,1
	CALL	STADTB
	CALL	CI		; Eingabe ASCII
INKEY1:	LD	HL,(STRDAT+2)
	LD	(HL),A
INKEY2:	JP	SLEN3
	;
INKEY3:	XOR	A
	CALL	STADTB
	JR	INKEY2

SNERR3:	JP	SNER		; SN-ERROR

; BASIC: Vordergrundfarbe einstellen:

INK:	CALL	ARGVL1		; Wert abholen
	CP	32
	JR	NC,FCERR	; FC-ERROR	(bisher SN-ERROR)
	ADD	A,A
	ADD	A,A		; *8
	ADD	A,A
	LD	D,A		; neuer Farbwert
	CALL	LOCOL		; Farbwert lesen
	AND	7		; Hintergrund-Bits erhalten
	JR	COL2

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
COL1:	CP	8
	JR	NC,FCERR	; FC-ERROR	(bisher SN-ERROR)
	LD	D,A
	CALL	LOCOL		; Farbwert lesen
	AND	0F8H		; Vordergrund-Bits erhalten
COL2:	OR	D		; neuen Farbwert einbauen
	PUSH	HL
	LD	HL,COLOR
	JR	LOC1		; neuer Farbwert

; BASIC: Punkt loeschen:

PRESET:	LD	E,2FH		; UP PUDE
	DB	1	; LD BC,n

; BASIC: Punkt setzen:

PSET:	LD	E,30H		; UP PUSE
	PUSH	DE
	CALL	EPRVL4		; Parameter erfassen
	PUSH	HL		; Adresszeiger
	LD	A,E
	LD	HL,HOR
	CALL	IRMWR		; X-Koordinate (low)
	LD	A,D
	INC	HL
	CALL	IRMWR		; X-Koordinate (high)
	EX	(SP),HL
	CALL	MC8D4		; Wert abholen
	EX	(SP),HL
	INC	HL		; HL=VERT
	CALL	IRMWR		; Y-Koordinate
GRAPH2:	POP	HL		; HL=Adresszeiger
	CALL	TCHAR1		; optionaler Parameter vorhanden?
	JR	Z,GRAPH4	; nein
	CALL	MC8D4		; Wert abholen nach A
	RLA
	RLA			; Rotieren in Bites fuer Vordergundfarbe
	RLA
	PUSH	HL
	LD	HL,FARB		; Grafik-Farbe
	CALL	IRMWR		; in IRM schreiben
	POP	HL		; HL=Adresszeiger
GRAPH4:	POP	DE		; E=UP-Nr. fuer PUSE, PUDE, LINE, CIRCLE
	CALL	PV5
	RET	NC		; kein FC-ERROR	(bisher SN-ERROR)
FCERR:	JP	FCER		; FC-ERROR

; BASIC: Tonausgabe:

SOUND:	CALL	ARGVL1		; 1. Wert abholen
	PUSH	HL
	LD	HL,ARG1		; Argumente hier ablegen
	LD	B,4		; 4 Argumente holen
SOUND1:	CALL	IRMWR		; Argument einschreiben
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
	JR	Z,SOUND3
	CALL	MC8D4		; optionalen 5. Wert (Lautstaerke) abholen
	EX	(SP),HL
	CALL	IRMWR		; und einschreiben nach ARG3
	INC	HL
	EX	(SP),HL
	CALL	TCHAR1		; weiterer Parameter?
	JR	Z,SOUND3
	CALL	MC8D4		; optionalen 6. Wert (Tondauer) abholen
	EX	(SP),HL
	CALL	IRMWR		; und einschreiben nach ARG3+1
	EX	(SP),HL
SOUND3:	LD	E,35H		; UP-Nr. fuer TON
	POP	BC		; Stack bereinigen
	JP	PV5		; Tonausgabe aufrufen

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
	CALL	SNALY
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
INSTR1:	PUSH	BC
	PUSH	DE
	LD	A,(DE)
INSTR2:	CP	(HL)
	JR	Z,INSTR5
	INC	HL
	DEC	C
	JR	NZ,INSTR2
INSTR3:	XOR	A
	POP	HL
	POP	HL
	POP	HL
INSTR4:	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	JP	POS1
	;
INSTR5:	INC	HL
	PUSH	HL
	DEC	HL
INSTR6:	INC	HL
	DEC	C
	JR	Z,INSTR7
	INC	DE
	DEC	B
	JR	Z,INSTR8
	LD	A,(DE)
	CP	(HL)
	JR	Z,INSTR6
	POP	HL
	POP	DE
	POP	BC
	DEC	C
	JR	INSTR1
	;
INSTR7:	INC	DE
	DEC	B
	POP	HL
	JR	NZ,INSTR3
	JR	INSTR9
	;
INSTR8:	POP	HL
INSTR9:	POP	DE
	POP	DE
	POP	DE
	AND	A
	SBC	HL,DE
	LD	A,L
	JR	INSTR4

; BASIC: F-Taste belegen:

BKEY:	CALL	ARGVL1		; Wert abholen
	CP	16		; F0 ... F15 zugelassen
	JP	NC,SNERR4
	LD	E,39H		; UP-Nr. fuer KEY
	DB	1	; LC BC,nn

; BASIC: F-Tasten auflisten:

KEYLIST:
	LD	E,3AH		; UP-Nr. fuer KEYLI
	DB	1	; LC BC,nn

; BASIC: Byte aus IRM lesen:

IRMRD:	LD	E,29H		; UP-Nr. fuer LDAM
JPV5:	JP	PV5

; BASIC: Byte in IRM schreiben:

IRMWR:	LD	E,28H		; UP-Nr. fuer LDMA
	JR	JPV5

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

; BASIC: Testen, ob Bildpunkt gesetzt ist:

PTEST:	CALL	SNALY6		; Klammer auf?
	CALL	EPRVL3		; Ausdruck berechnen -> DE
	PUSH	HL		; Programmzeiger auf Stack legen
	LD	A,E
	LD	HL,HOR		; X-Koordinate eintragen
	CALL	IRMWR		; Low
	LD	A,D
	INC	HL
	CALL	IRMWR		; High
	INC	HL		; VERT
	EX	(SP),HL		; HL=Programmzeiger, (Stack)=VERT
	ld	c,0		; Y-Koordinate nicht angegeben
	LD	A,(HL)
	CP	','		; weiterer Parameter?
	JR	NZ,PTEST1	; nein
	CALL	MC8D4		; 2. Wert holen
	EX	(SP),HL		; HL=VERT
	CALL	IRMWR		; Y-Koordinate eintragen
	EX	(SP),HL		; HL=Programmzeiger
	ld	c,-1		; Y-Koordinate angegeben
PTEST1:	CALL	CPBRGT		; Klammer zu?
	POP	DE		; VERT - wird nicht mehr benoetigt
	EX	(SP),HL		; Adresszeiger auf Stack
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE		; Sprungadresse auf Stack
	PUSH	HL		; urspruenglicher Stack-Wert
	LD	E,2FH		; UP-Nr. fuer PUDE
	CALL	PV5
	JP	C,FCER		; ausserhalb -> FC-ERROR
	PUSH	AF		; Z-Flag erhalten
	BIT	3,(IX+1)	; HiRes?
	JR	Z,PTEST4	; Farbwert steht bereits in Register A
	POP	AF
	LD	B,0
	JR	Z,PTEST3	; Punkt war geloescht
	LD	HL,FARB
	CALL	IRMWR		; Grafikfarbe eintragen fuer folgendes PUSE
	RRCA
	RRCA			; Vordergrundfarbe nach Bit 0-3 rotieren
	RRCA
	AND	0Fh
	JR	NZ,PTEST2	; Farbe 1-15
	LD	A,8		; Farbe 0 => 8 (beides ist schwarz)
PTEST2:	LD	B,A		; Rueckgabewert = Farbe 1..15
	LD	E,30H		; UP-Nr. fuer PUSE
	CALL	PV5		; Punkt wieder setzen
	inc	c		; war Y-Koordinate angegeben?
	jr	z,PTEST3	; ja, dann Farbcode zurueck geben
	ld	b,c		; ansonsten 1 fuer Punkt gesetzt
PTEST3:	LD	A,B		; Rueckgabewert (0=geloescht oder Farbcode)
	DB	06H		; LD B,0F1H
PTEST4:	POP	AF
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
	CALL	IRMRD
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
	JP	Z,PTEST
	CP	7CH
	JR	Z,CSRLN
	CP	76H
	JR	Z,VGET
	SUB	3EH
	JR	C,SNERR4	; nein
	CP	7
	JR	NC,SNERR4	; nein
;--------------------------------
	EX	DE,HL
	LD	BC,TOKJP	; Token-Adresstabelle
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

CSRLN:	CALL	SNLY14		; Syntax-Check '('
	EX	(SP),HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	ARGVL2
	PUSH	HL
	AND	A		; Fenster beruecksichtigen?
	LD	A,0
	JR	NZ,CSRLN1
	LD	HL,WINON+1
	CALL	IRMRD
CSRLN1:	LD	B,A
	LD	HL,CURSO+1
	CALL	IRMRD
	ADD	A,B		; aktuelle Zeile
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
	JP	INKEY1		; als Zeichen zurueckgeben

; BASIC: Kanal schliessen:

CLOSE:	LD	C,0
	LD	A,(HL)
	CP	'I'		; Eingabe?
	JR	Z,CLOS1
	INC	C
	CP	'O'		; Ausgabe?
	JR	NZ,SNERR4
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
	CALL	BASIO		; Kanal schliessen
	POP	DE
	RET

; BASIC-Erweiterung JOYST(i)
; kompatibel zum KC 87 mit einem Joystick
;
JOYST:	CALL	SNLY14		; Syntax-Check '('
	PUSH	HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	EPRVL3		; Ausdruck berechnen -> DE
	LD	A,E
	DEC	A
	OR	D		; Argument gleich 1?
	LD	A,0
	JR	NZ,CSRLN2	; nein: Rueckgabewert 0
	IN	A,(90h)		; M008 PIO A Daten
	CPL
	LD	D,A
	AND	00001111b	; nur Richtungsbits, CY=0
	RRA
	JR	NC,JOYST1
	OR	00010000b	; <Up> auf Bit 3, CY=0
JOYST1:	RRA
	JR	NC,JOYST2
	OR	00000100b	; <Down> auf Bit 2
JOYST2:	LD	E,A
	LD	A,00110000b	; Feuertasten
	AND	D
	OR	E
	JR	CSRLN2		; Rueckgabewert in A

; BASIC: Zufallsgenerator:

RANDOM:	LD	A,R		; Zufallswert aus Refresh-Register
	LD	(RNDVR3+2),A	; in CONST[0] eintragen
	ld	a,7		; und Index auf 7 setzen,
	ld	(rndvr3),a	; damit dieser Wert sofort benutzt wird.
	RET

; BASIC: Kanal oeffnen:

OPEN:	LD	A,(HL)
	INC	HL
	CP	'I'		; Eingabe?
	JR	Z,OPEN1
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
OPEN1:	CALL	INCHNL		; Kanal-Eingabe
	LD	A,(ININD)	; IN-Index
	AND	3
	RET	Z
	LD	A,0D5H		; *.UUU
	CALL	CASS1		; Dateiname aufbereiten
	LD	HL,INTPRB	; Druckpuffer=Dateiname
	CALL	CI		; Eingabe eines Zeichens
	POP	HL
	RET

; BASIC: Linie zeichnen:

BLINE:	LD	BC,43EH		; 4 Parameter, UP LINE
	JR	GRAPH

; BASIC: Kreis zeichnen:

CIRCLE:	LD	BC,33FH		; 3 Parameter, UP CIRCLE
GRAPH:	PUSH	BC
	CALL	EPRVL4		; Parameter erfassen
	POP	BC
	PUSH	BC
	PUSH	HL
	LD	HL,ARG1		; hier ablegen
GRAPH1:	LD	A,E
	CALL	IRMWR		; Ablage der Parameter
	INC	HL
	LD	A,D
	CALL	IRMWR
	INC	HL
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
;	CALL	GFARB		; optionaler Parameter (Farbwert)
;	POP	DE		; E=UP-Nr. fuer LINE oder CIRCLE
;	JP	PV5

; BASIC: DEVICE einstellen
;	Aufruf: DEVICE nr, wobei nr=0 fuer TAPE usw.

BDEV:	CALL	TCHAR1		; Parameter angegeben?
	LD	A,0FFH		; ohne Parameter auflisten was es gibt
	CALL	NZ,ARGVL1	; ansonsten Wert holen
	PUSH	HL
	CALL	IRMON		; IRM ein
	CALL	SETDEV		; und Geraet entsprechend einstellen
	JP	C,BD2		; Treiber nicht aktiv -> IO-ERROR
	LD	A,0FEh
	CALL	SETDEV		; Name des Treibers anzeigen
	CALL	CRLF
BD1:	CALL	IRMOF		; IRM aus
	POP	HL
	RET

; BASIC: MC-Programm laden
;	Aufruf: BLOAD "name.typ", wobei Name bei DEVICE=TAPE entfallen kann
; PE:	HL	Zeiger auf Kommando
; PA:	HL	Zeiger weiter gerueckt, wieweit abgearbeitet ist

BLOAD:	CALL	TCHAR1		; Parameter angegeben?
	JR	Z,BLOAD2	; ohne Parameter
	CALL	SNALY
	PUSH	HL		; Kommandozeile
;	LD	A,12		; max. Laenge Dateiname+Typ
	CALL	MASK		; Zeichenkette in DE
	EX	DE,HL		; Dateiname in HL
BLOAD1:	CALL	IRMON
	XOR	A
	LD	(ARGN),A	; ARGN=0 (kein Ladeoffset)
	CALL	LOAD		; UP 10H direkt aufrufen
	JR	BD1
	;
BLOAD2:	PUSH	HL
	CALL	DEV		; Kassette?
	JR	Z,BLOAD1	; ja, das ist OK ohne Name
	LD	E,36		; MO-ERROR
	JP	ERROO		; Error anzeigen

; BASIC: DIR anzeigen aktuelles Speichergeraet
;	Aufruf: FILES "maske", wobei Maske optional ist
; PE:	HL	Zeiger auf Kommando
; PA:	HL	Zeiger weiter gerueckt, wieweit abgearbeitet ist

FILES:	CALL	TCHAR1		; Parameter angegeben?
	JR	Z,FILES3	; ohne Parameter
	CALL	SNALY
	PUSH	HL		; Kommandozeile
	CALL	MASK		; Zeichenkette in DE
FILES2:	CALL	IRMON		; IRM ein
	CALL	PV7
	DB	8		; DEVICE-Funktion DIR aufrufen
	JR	BD1
	;
FILES3:	PUSH	HL		; Kommandozeile
	LD	DE,BASON+1	; zeigt auf eine 0
	JR	FILES2		; gesamtes Verzeichnis anzeigen

; BASIC: Verzeichnis wechseln
;	Aufruf: CD "verzeichnisname"
; PE:	HL	Zeiger auf Kommando
; PA:	HL	Zeiger weiter gerueckt, wieweit abgearbeitet ist

CHDIR:	;CALL	TCHAR1		; Parameter angegeben?
	CALL	SNALY
	PUSH	HL		; Kommandozeile
	CALL	MASK		; Zeichenkette "Verzeichnis" in DE bereitstellen
	CALL	IRMON		; IRM ein
	CALL	PV7
	DB	9		; DEVICE-Funktion CD aufrufen
	JR	BD1

; Zeichenkette "Maske" bzw. "Verzeichnis" in DE bereitstellen
; PE:	HL	Zeiger auf Ende einer Zeichenkette, direkt nach dem " am Ende
;
MASK:	DEC	HL		; zurueck auf letztes Zeichen
	LD	A,(HL)
	CP	22H		; Anfuehrungszeichen?
	JR	Z,MASK		; dann auch noch davor gehen
	INC	HL
	PUSH	HL
	CALL	ASC1		; DE zeigt jetzt auf Beginn der Maske nach dem "
	POP	HL
	XOR	A		; A=0 und CY=0
	SBC	HL,DE
	LD	B,H
	LD	C,L		; BC = Laenge der Zeichenkette
	EX	DE,HL		; HL = Beginn der Zeichenkette
	LD	DE,0362H	; INPBUF+1	; Eingabepuffer
	PUSH	DE
	LDIR
	LD	(DE),A		; 0 anhaengen
	POP	DE		; Maske in EDIT-PUFFER
	RET

; Sprung aus CAOS-ROM-C in INIT-Routine des ROM-Moduls (z.B. M052-USB)
; PE:	A	Schaltzustand Modul ein
;	B	Steckplatz

JROM:	LD	D,A		; Schaltzustand
	LD	L,B		; Steckplatz
	PUSH	HL		; Steckplatz merken zum wieder ausschalten
	CALL	MODUSW		; M052 einschalten (und MODST aktualisieren)
;	IN	A,(PIOAD)
;	AND	7Fh		; USER-ROM abschalten (wegen Aufruf aus TEMO)
;	OUT	(PIOAD),A
	RESIXA	7,4		; RES 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-ROM C aus!
	LD	B,L		; Steckplatz
	LD	HL,JREX		; Rueckkehr hierher
	PUSH	HL
	JP	0C020h		; Sprung in Modul-ROM -> Treiber initialisieren
	;
JREX:	POP	HL		; L=Stekplatz des Moduls
	LD	D,0		; Modul AUS
	CALL	MODUSW		; Modul wieder aus schalten
	JP	CON		; Sprung zurueck in CAOS-ROM C
;_____________________________________________________________
;							**40**
; Berechnen der Quadratwurzel
; PE:	HL	16 Bit-Zahl (vorzeichenlos)
; PA:	A	Ergebnis 8 Bit
; VR:	AF,HL,DE

SQR:	CALL	CON
	JP	SQRC
;
; DEVICE ermitteln:
; PA:	A	DEVICE-Nummer*32
;	Z=1	TAPE
;	CY=0
;
DEVH:	LD	H,High(DEVTAB)	; (HL)=Sprungadresse
DEV:	LD	A,(IX+8)	; Device-Nr. in Bit 2-4
DEVROT:	RLCA			;	in Bit 3-5
	RLCA			;	in Bit 4-6
	RLCA			;	in Bit 5-7
	AND	11100000b	; Device-Nr. ausfiltern, Z=1 wenn Device=0
	RET

; Initialisierungstabelle fuer M021:

CENINI:	DB	3

JOYINI:	 DB	92h,5		; PIO-A Steuerwort
	  DB	0E0h		; Interruptvektor
	  DB	11001111b	; Einzelbit-Steuermodus
	  DB	01111111b	; Nur Bit 7 = Ausgabe (Strobe)
	  DB	10010111b	; Interrupt bei jeder Bitaenderung
	  DB	11000000b	; Ueberwache Bits 0..5

	 DB	93h,2		; PIO-B Steuerwort
	  DB	11001111b	; Bitbetrieb
	  DB	00000000b	; alles Ausgaenge

	 DB	90h,1		; PIO-A Daten
	  DB	10000000b	; Strobe passiv
;
; ISR fuer CTC2: Tonlaenge
;
ISRC2:	PUSH	AF
	PUSH	HL
	CALL	TOFF		; kehrt mit EI zurueck
	POP	HL
	POP	AF
	RETI

; Reaktion auf ShCLR:

HCPGM:	PUSH	AF
	LD	A,(IX+4)	; CAOS-C merken
	PUSH	AF
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(PORT2),A	; CAOS-C on
	CALL	HCPGMC
	POP	AF
	CALL	OUT86		; CAOS-C wie vorher
	POP	AF
	RET
;
; Version 0 fuer TAPE- und DISK-Treiber melden
;
VER0:	XOR	A		; Version 0.0
	RET
;_____________________________________________________________
;							**4A**
; Dezimalausgabe des Wertes in HL
; PE:	HL	Zahl
; VR:	AF, HL, BC

HLDEZ:	PUSH	DE		; bei Rekursion: letzten Rest retten
	LD	BC,-10
	LD	DE,-1		; Zaehler fuer Ergebnis (Anzahl Sub's)
HLDEZ1:	ADD	HL,BC
	INC	DE
	JR	C,HLDEZ1	; DE = HL/10
	LD	BC,10
	ADD	HL,BC		; letzten Abzug korrigieren; HL = Rest
	EX	DE,HL		; 10er / Rest tauschen
	LD	A,H
	OR	L		; Zehner = 0 ?
	CALL	NZ,HLDEZ	; Rekursion wenn Zehner vorhanden
	LD	A,E
	ADD	A,'0'		; Rest -> ASCII in A
	CALL	OCHR		; Ziffer ausgeben
	POP	DE		; voriger Rest
	RET
;
; Tabelle der ESC-Funktionen:
;
ESCTAB:	DW	ESC0		; Tabulatorsprung			ESC'0'
	DW	ESC1		; Anzeige Bild 0	Zugriff Bild 0	ESC'1'
	DW	ESC2		;	  Bild 1		Bild 1	ESC'2'
	DW	ESC3		;	  Bild 0		Bild 1	ESC'3'
	DW	ESC4		;	  Bild 1		Bild 0	ESC'4'
	DW	ESC5		; MODUL-Anzeige				ESC'5'
	DW	ESC6		; SYSTEM-Anzeige			ESC'6'
	DW	ESC7		; Pixel invertieren			ESC'7'
	DW	ESC8		; Farbe invertieren			ESC'8'
	DW	ESC9		; Farbebene ein/aus			ESC'9'
	DW	ESCA		; HiRes ein/aus				ESC'A'
	DW	ESCB		; HRG ein/aus				ESC'B'
	DW	ESCC		; IBM/CAOS-Zeichensatz			ESC'C'
	DW	ESCD		; DEVICE-Umschaltung			ESC'D'
	DW	0E000H		; Soft-Reset				ESC'E'
	DW	0F000H		; Soft-Power-On				ESC'F'
	DW	ESCG		; Umschaltung 2-Monitor-Betrieb		ESC'G'
	DW	ESCH		; Hilfefunktion (Idee aus CAOS 6.0)	ESC'H'
ESCX:

INIT:	DB	'INIT',cr,0	; CAOS 4.8: Autostart-Kommando
UUU:	DB	'INITIAL.UUU',0	; Standard-Dateiname fuer %INIT

	ABSFILL	0EDF7h,<ROM-E-Ende>

; High-Byte der Adresse muss konstant sein!
; je 1 Byte fuer Eingabe + Ausgabe pro Kanal

BUPTAB:	DB	16H, 24H	; INTB, OCHR	Kanel #0 - Konsole
	DB	37H, 38H	; MBIN, MBOUT	Kanal #1 - Kassette
	DB	06H, 02H	; USIN1, UOT1	Kanal #2 - Anwenderkanal 2
	DB	07H, 03H	; USIN2, UOT2	Kanal #3 - Anwenderkanal 3

VERSION:
	DB	48H		; CAOS-Version 4.8
