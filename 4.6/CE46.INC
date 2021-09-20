;*****************************************
;**	CAOS 4.6	ROM E		**
;**					**
;**	Adresse:  E000h bis EDFFh	**
;**					**
;**	letzte Aenderung: 12.03.2017	**
;*****************************************

;	ORG	0E000H

	JP	BYE		; Tasten-RESET
	JP	BEXP1		; BASIC-Erweiterung 1
	JP	BEXP2		; BASIC-Erweiterung 2
	JP	BEXP3		; BASIC-Erweiterung 3
	DW	TOKTAB		; Token-Tabelle der Erweiterungen
BASIO:	JP	BASPV		; BASIC Programmverteiler

	DW	7F7FH
	DB	'BASIC'
	DB	11H
	CALL	BASON
	JP	PRIST		; BASIC-Kaltstart

	DW	7F7FH
	DB	'REBASIC'
	DB	11H
	CALL	BASON
	JP	SECST		; BASIC-Warmstart

; BASIC vorbereiten:

BASON:	LD	D,0
	LD	A,2
	LD	L,5		; SWITCH 5 0
	CALL	MODU		; CAOS-C aus
	LD	D,0C1H
	LD	A,2
	LD	L,A		; SWITCH 2 C1
	CALL	MODU		; BASIC-Ebene einschalten
	JP	SCROFF		; IRM aus

; CRT-Treiber:

PADR0:	LD	DE,(CURSO)
PADR1:	LD	HL,(WINON)
	ADD	HL,DE
	SLA	H
	SLA	H
	SLA	H
	;
PADR:	PUSH	AF		; **34**
	LD	A,L		; Spalte
	LD	L,H		; Pixelzeile
	CP	28H
	JR	NC,IAD2		; zu gross
	OR	80H
	LD	H,A		; HL=Pixeladresse
	POP	AF
	AND	A		; CY=0
	RET
IAD2:	POP	AF
	SCF			; CY=1
	RET

; Test Cursor im Fenster

TCIF:	LD	A,(WINLG)	; **33**
	DEC	A
	SUB	E		; Cursor-Spalte
	RET	C
	LD	A,(WINLG+1)
	DEC	A
	SUB	D		; Cursor-Zeile
	RET

DABR:	PUSH	AF		; **32**
	CALL	TCIF
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
	POP	AF
	AND	A
	RET

CRT:	PUSH	HL		; **00**
	PUSH	DE
	PUSH	BC
	PUSH	AF
	CALL	PCHR
	LD	(CURSO),DE
POP4:	POP	AF
	POP	BC
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
	JR	NZ,WPX11	; Farbe AUS
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
	LD	(IX+4),A	; CAOS-C
	OUT	(86H),A		; wie vorher
	JR	POP4		; POP	AF,BC,DE,HL   RET

WPX14:	LD	A,(IX+4)	; CAOS-C-Schaltzustand
	PUSH	AF		; merken
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(86h),A		; CAOS-C ein
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

; CRT-Grundprogramm

PCHR:	LD	DE,(CURSO)
	LD	HL,STBT
	BIT	4,(HL)		; ESC aktiv?
	JR	Z,PCHR3
CRT1:	RES	4,(HL)
	CP	'0'
	RET	C
	CP	'9'+1
	JR	NC,PCHR1
	SUB	'0'
	JR	PCHR2
	;
PCHR1:	CP	'A'
	RET	C
	RES	5,A
	CP	'Z'+1
	RET	NC
	SUB	'A'-10
PCHR2:	LD	HL,L3SIZ	; Tabelle
	CP	(HL)		; lang genug?
	RET	NC
	ADD	A,A		; *2
	LD	HL,(L3TAB)	; klar zum Ansprung
	JR	PCHR4
	;
PCHR3:	CP	20H		; Steuerfunktion?
	JR	NC,PCHR5
	BIT	3,(HL)		; darstellen?
	JR	NZ,PCHR5
	ADD	A,A		; *2
	LD	HL,(CTAB)
PCHR4:	LD	C,A
	LD	B,0
	ADD	HL,BC
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
JPHL:	JP	(HL)		; anspringen

ESC0:	LD	A,E		; Tabulator
	OR	7		; 8er Schrittweite
	INC	A
	LD	E,A
	JR	CUR1
	;
PCHR5:	CALL	DABR
	RET	C		; ausserhalb
	LD	(HL),A		; ASCII eintragen
	CALL	WPIX		; darstellen
CUR:	INC	E
CUR1:	LD	A,(WINLG)
	DEC	A
	CP	E
	RET	NC
NL:	LD	E,0		; NL fuer 1E
CUD:	INC	D
	LD	A,(WINLG+1)
	CP	D
	RET	NZ
	LD	HL,(WEND)
	JP	(HL)		; PAGE oder SCROLL

HCOPY:	LD	HL,(HCADR)	; ShCLR
	JP	(HL)

CUL:	LD	A,E
	AND	A
	JR	Z,CUL1
	DEC	E
	RET

CUL1:	LD	A,D
	AND	A
	RET	Z
	DEC	D
CEL:	LD	A,(WINLG)
	DEC	A
	LD	E,A
	RET

CUU:	LD	A,D
	AND	A
	RET	Z
	DEC	D
	RET

PAGEM:	LD	HL,HOMEPG	; ShCUU
WADR:	LD	(WEND),HL
	RET

SCROL:	LD	HL,SCRLPG	; ShCUD
	JR	WADR

CLR:	CALL	CUL
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

CLS:	LD	A,(WINLG+1) 	; ShHOME
	LD	D,0
CLS1:	PUSH	AF
	CALL	CLLN
	POP	AF
	INC	D
	DEC	A
	JR	NZ,CLS1		; A Zeilen
HOMEPG:	LD	D,0
CBL:	LD	E,0
	RET

SCRLPG:	LD	A,(IX+4)	; CAOS-C-Schaltzustand
	PUSH	AF		; merken
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(86h),A		; CAOS-C ein
	CALL	SCRLD		; weiter im CAOS-D-Segment
	JR	CCRES

CLLN:	LD	A,(IX+4)	; CAOS-C-Schaltzustand
	PUSH	AF		; merken
	SETIXA	7,4		; SET 7,(IX+4),A
	OUT	(86h),A		; CAOS-C ein
	CALL	CLLND		; weiter im CAOS-D-Segment
CCRES:	POP	AF
	LD	(IX+4),A	; CAOS-C
	OUT	(86h),A		; wie vorher
	RET

BEEP:	LD	BC,0A0FH	; B=Dauer, C=Lautstaerke
	LD	HL,48		; HL=Tonhoehe 1
	PUSH	DE
	LD	E,H		; E=0
	CALL	TON2
	LD	A,1EH
	CALL	WAIT
	LD	A,3
	OUT	(CTC0),A
	LD	A,10H
	POP	DE
	JP	WAIT

CLIK:	LD	BC,0307H	; B=Dauer, C=Lautstaerke
	LD	HL,36		; HL=Tonhoehe 1
	PUSH	DE
	LD	E,H		; E=0
	CALL	TON2
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
LDMA:	LD	(HL),A		; **28**
	RET

LDAM:	LD	A,(HL)		; **29**
	RET

CLICK:	LD	A,(IX+8)	; ShINS
	XOR	20H
	LD	(IX+8),A
	RET

;	=== KBD-Treiber ===

;
; ISR CTC Kanal 3 (Tastatur)
;
ISRC3:	EI
	PUSH	AF
	LD	A,23H	  	; DI,ZG256,Res
	OUT	(CTC3),A
	LD	(IX+13),0	; Zeichen tot
	JR	TST5		; freigeben
;
; ISR PIO Kanal B (Tastatur)
;
ISRPB:	PUSH	AF
	IN	A,(CTC3)	; gemessene Zeit
	PUSH	AF
	LD	A,0A7H		; EI,ZG256,Res
	OUT	(CTC3),A
	LD	A,8FH		; Zeitkonstante
	OUT	(CTC3),A
	POP	AF
	EI
	CP	20
	JR	C,TST5		; zu lang
	CP	120
	JR	NC,TST5		; zu kurz
	CP	101		; Diskriminator
	JR	NC,TST1
	ADD	A,190
	JR	NC,TST2		; Stop-Bit
TST1:	RR	(IX+12)		; Bit einschieben
	JR	TST5
	;
TST2:	PUSH	HL
	PUSH	DE
	LD	A,(IX+12)
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
;
; in CAOS 3.4 gefunden:
;	CP 40H
;	JP M,TST3
;	XOR 20H
;TST3:
;
	JR	C,TST3
	CP	80H
	JR	NC,TST3
	XOR	20H		; klein<=>gross
TST3:	LD	D,A		; Code merken
	CP	(IX+13)		; = letzter Code?
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

; Abfrage ohne Quittung
KBDS:	OR	A		; **0C**
	BIT	0,(IX+8)	; Code gueltig?
	RET	Z
	LD	A,(IX+13)	; Tastencode
	SCF			; wenn ja
	RET

; Abfrage mit Quittung
KBDZ:	CALL	KBDS		; **0E**
	RET	NC
	RES	0,(IX+8)
	RET

; Test auf BRK-Anforderung
BRKT:	CALL	KBDS		; **2A**
	RET	NC
	CP	3
	SCF
	RET	Z
	AND	A
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
;
; Abschluss Bandausgabe		**09**
; PA:	CY=0
;
CSRO:	LD	(IX+2),0FEH	; FFH = Kennung Ende-Block
	CALL	PV1		; Block auf Kassette ausgeben
	DB	01H		; MBO
;
; Abschluss Bandeingabe		**0B**
; PA:	CY=0
;
CSRI:	LD	HL,(ZOTAB) 	; intern
	LD	(OUTAB),HL
	LD	HL,(ZWEND)	; grosses
	LD	(WEND),HL	; Ruecksetzen
;	CALL	CLC		; Puffer loeschen (VR: HL)
	IN	A,(PIOAD)
	AND	10011111b	; Motor, LED aus
	OUT	(PIOAD),A
;	CALL	CRLF		; VR: AF
	RES	0,(IX+8)
CSRO2:	BIT	3,(IX+1)	; HiRes?
	JR	Z,CSRO3
	IN	A,(PIOBD)	; nein-
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
	AND	01100000b
	OR	00000111b
	OUT	(CTC2),A	; Blinken
	LD	A,(BLINK)	; ZK laden
	OUT	(CTC2),A
	LD	A,L
	OUT	(PIOAD),A	; IRM wie zuvor einstellen
	EI			; Interrupt wieder ein
	RET
;
; Kassettenpuffer loeschen:
; PA:	HL = CASS
;
CLC:	LD	HL,CASS+80H
	XOR	A
CLC1:	DEC	L
	LD	(HL),A
	JR	NZ,CLC1
	RET

;***************************************************************
; 
;            K A S S E T T E N E I N / A U S G A B E
;
;***************************************************************
; 
; Zeitkonstanten fuer Schreiben:
;
ZKKAT	EQU	93	;Trennzeichen
ZKKA0	EQU	23	; "0"
ZKKA1	EQU	47	; "1"
;
; Zeitkonstanten fuer Lesen:
;
IKEZK	EQU	163	; Zeitkonstante fuer CTC
IKEGN	EQU	93	; Erkennung Nullbit
IKEG1	EQU	186	; Erkennung Einsbit

; Initialisierung Bandausgabe:
; PA:	CY=0	(kein Fehler bei Bandausgabe)

ISRO:	DI			; **08**
	IN	A,(PIOAD)
	OR	01100000b	; Motor+LED ein
	CALL	ISRO1		; Initialisierung
	LD	(IX+2),0	; Block 1
	LD	BC,4096		; langer Vorton
	; Ausgabe 1 Block
MBO:	INC	(IX+2)		; **01**
	LD	A,10000111b	; EI,ZG16,Res
	DI
	OUT	(CTC1),A
	LD	A,ZKKA1		; Vorton 1
	OUT	(CTC1),A
	EI
	LD	D,A		; D=47
	LD	E,A		; E=47
MBO1:	CALL	BITOUT		; Vorton ausgeben
	CPI
	JP	PE,MBO1		; BC mal
	CALL	ZTON		; Trennzeichen
	LD	A,(IX+2)
	CALL	BYTOT		; Blocknummer
	LD	L,(IX+5)
	LD	H,(IX+6)	; Blockanfang
	LD	B,128		; Zaehler
BLKOT:	LD	A,(HL)
	CALL	BYTOT		; 128 Datenbyte ausgeben
	LD	A,C
	ADD	A,(HL)		; Pruefsumme
	LD	C,A		; nebenher berechnen in Register C
	INC	HL
	DJNZ	BLKOT
	CALL	BYTOT		; Pruefsumme ausgeben
;	LD	A,D		; ???
;	CALL	BITOUT		; Endimpuls (Vollschwingung)

; Bei einer Halbschwingung als Endimpuls aendert sich mit jedem Block die
; Polaritaet der Signale bei jedem Block. Das ist bei allen CAOS-Versionen
; so ausser CAOS 4.5. Eventuell ist das ja besser - deshalb in CAOS 4.6 wieder
; eine Halbschwingung als Endimpuls. (bemerkt von Micha im Forum am 02.02.2015)

	CALL	HBITOT		; Endimpuls (Halbschwingung)
	LD	E,L
	LD	D,H
	LD	A,3		; CTC stoppen
	OUT	(CTC1),A
	RET

; Ausgabe eines Bytes auf Kassette
; PE:	A	Datenbyte
; PA:	CY=0

BYTOT:	PUSH	BC
	LD	C,A
	LD	B,8		; 8 Bit ausgeben
BYTOUT:	RRC	C		; mit Bit 0 beginnend
	LD	E,ZKKA0
	CALL	NC,BITOUT	; CY=0 / Nullbit ausgeben
	LD	E,ZKKA1	
	CALL	C,BITOUT	; CY=1 / Einsbit ausgeben
	DJNZ	BYTOUT		; 8x wiederholen
	POP	BC
ZTON:	LD	E,ZKKAT		; ZK Trennzeichen
BITOUT:	CALL	HBITOT		; Aufrufen und reinlaufen
HBITOT:	LD	(IX),D
HBIT1:	LD	A,(IX)		; Warten bis Zeitkonstante von
	AND	A		; Interruptroutine zurueckgesetzt
	JR	NZ,HBIT1
	LD	D,E
	RET
			
; Initialisierung Bandeingabe
; PE:	HL	Dateiname
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC

ISRI:	CALL	ISRI1		; **0A**

; Einen Block einlesen:
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC

MBI:	LD	A,10000011b	; **05**
	OUT	(PIOAS),A	; EI an PIO A (Kassette)
	PUSH	HL
	PUSH	DE
MBI1:	LD	B,22
	LD	(IX+11),0	; Pruefsumme=0
MBI2:	CALL	STOP1		; Lesen einer Vollschwingung
	JR	C,MBI1		; kein Einsbit ?
	CP	IKEG1		; Einsbit ?
	CALL	LEDOO		; LED on/off
	JR	C,MBI1
	DJNZ	MBI2		; 22 korrekte Schwingungen abwarten
MBI3:	LD	B,2
MBI4:	XOR	A
	CALL	STOP2		; Lesen einer Halbschwingung
	CP	IKEGN		; Nullbit ?
	JR	NC,MBI3
	DJNZ	MBI4
	CALL	BYTIN		; Blocknummer lesen
	JR	C,MBI5		; Stoerung
	LD	(IX+2),A	; Blocknummer eintragen
	LD	B,128
	LD	L,(IX+5)
	LD	H,(IX+6)
BLKIN:	CALL	BYTIN		; Datenbyte lesen
	JR	C,MBI5		; Stoerung
	LD	(HL),A
	LD	A,(IX+11)
	ADD	A,(HL)		; Pruefsumme berechnen
	LD	(IX+11),A
	INC	HL
	DJNZ	BLKIN
	CALL	BYTIN		; Pruefsumme einlesen
	JR	C,MBI5		; Stoerung
	SUB	(IX+11)		; Pruefsumme richtig?
	ADD	A,-1		; CY:=/Z
MBI5:	POP	DE
	POP	HL
	DI
	LD	A,3		; DI an PIO A
	OUT	(PIOAS),A
	EI
LEDOO:	IN	A,(PIOAD)	; LED ein/aus
	SET	5,A		; je nach CY
	JR	NC,MBI6
	RES	5,A
MBI6:	OUT	(PIOAD),A
	RET
;
; ISR CTC Kanal 1 (Kassette-Out)
;
ISRC1:	PUSH	AF
	LD	A,87H
	OUT	(CTC1),A
	LD	A,(IX)		; Uebergabezelle
	OUT	(CTC1),A
	LD	(IX),0		; Quittierung
	JR	ISR1E
;
; ISR PIO Kanal A (Kassette-In)
;
ISRPA:	PUSH	AF
	IN	A,(CTC2)
	LD	(IX),A		; Uebergabezelle
	LD	A,7
	OUT	(CTC2),A
	LD	A,IKEZK
	OUT	(CTC2),A
ISR1E:	POP	AF
IRET:	EI
	RETI

; 1 Bit einlesen:
; PA:	A	Zeitwert
;	CY=0	Vorton
;	CY=1	Nullbit oder Einsbit
;
STOP1:	XOR	A
	CALL	STOP2		; erst aufrufen, dann reinlaufen
STOP2:	LD	C,A		; erste 'Zeit' in C merken
	LD	(IX),0
STOP21:	IN	A,(PIOAD)
	OUT	(PIOAD),A	; PIO-Logik freigeben
	LD	A,(IX)		; Warten auf Interrupt
	OR	A
	JR	Z,STOP21
	ADD	A,C		; beide 'Zeiten' addieren
	RET

; Einlesen eines Bytes von Kassette
; PA:	A=Byte, CY=1: Stoerung
; VR:	AF, BC, DE

BYTIN:	LD	DE,800H
IB1:	CALL	STOP1		; Bit Eingabe
	JR	C,IB2		; Nullbit ?
	CP	IKEG1		; Einsbit
	RET	C		; Trennzeichen
IB2:	CCF
	RR 	E		; Bit einschieben
	DEC	D
	JR	NZ,IB1		; 8*
	CALL	STOP1		; Trennzeichen (mit CY-Rueckmeldung)
	LD	A,E		; Byte in A
	RET

; ===== BASIC =====

; BASIC-Erweiterung 1: Erweiterungstoken starten

BEXP1:	LD	A,B		; Nr.
	SUB	9
	JR	C,SNERR1
	CP	29		; 27 ohne "DRIVE"
	JR	NC,SNERR1	; >28
	RLCA			; *2
	LD	C,A
	LD	B,0
	EX	DE,HL
	LD	HL,TOKJP+8
	JP	DLI22		; Ruecksprung zu Tokenverarbeitung

; BASIC-Erweiterung 2: Print-Erweiterung?

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
PREX1:	PUSH	HL
	LD	HL,COLOR
	CALL	IRMRD
	POP	HL
	LD	(COLRET),A 	; Farbe merken
	LD	A,(HL)
	CP	0DFH		; INK?
	JR	Z,PRINK
	CP	0E2H		; COLOR?
	JR	Z,PRCOL
	CALL	TCHAR		; naechstes Zeichen holen
	CALL	PAPER
PREX2:	LD	A,(HL)
	CP	';'
	JR	Z,PREX5
SNERR1:	JP	SNER		; SN-ERROR

PRCOL:	CALL	TCHAR		; naechstes Zeichen holen
	CALL	BCOLOR
	JR	PREX2

PRINK:	CALL	TCHAR		; naechstes Zeichen holen
	CALL	INK
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
PREX3:	CALL	PRINT2
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

; String vervielfaeltigen

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
	JR	C,STR5
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
STR5:	LD	E,1CH		; ST-ERROR
	JP	ERROO		; Ausgabe Fehlercode

; Neu nummerieren

RENUM:	PUSH	HL
	LD	HL,10
	LD	(DISTAN),HL
	LD	HL,(PSTBEG)	; Start
	PUSH	AF
	PUSH	HL
	INC	HL
	INC	HL
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	LD	(ANF),HL	; ZL-Nr
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
	OR	A
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
	CP	2CH
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
RUP12:	LD	A,0FFH
	OR	A
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

; Zeile(n) loeschen

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

; Programm unterbrechen

PAUSE:	CALL	TCHAR1		; mit oder ohne Paramater?
	JR	NZ,PAUS2
PAUS1:	CALL	KBDS
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
	LD	E,14H		; WAIT
	CALL	PV5
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

; Signalton

BBEEP:	LD	B,1		; ohne Angabe 1x
	CALL	TCHAR1
	JR	Z,BBP1
	CALL	ARGVL1		; Wert abholen
	LD	B,A		; Anzahl
BBP1:	LD	A,7		; Beep
	LD	E,0		; CRT
	CALL	PV5
	DJNZ	BBP1
	RET

; Fenster einstellen

WINDOW:	CALL	TCHAR1		; Parameter angegeben?
	JR	Z,WINDO1	; nein, dann volles Fenster einstellen
	PUSH	HL
	CALL	ARGVL1		; Wert abholen
	EX	(SP),HL
	LD	HL,WNDFN+90	; in Fenster 9
	CALL	IRMWR		; erstellen
	INC	HL
	EX	(SP),HL
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	EX	(SP),HL
	CALL	IRMWR
	INC	HL
	EX	(SP),HL
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	EX	(SP),HL
	CALL	IRMWR
	EX	(SP),HL
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	EX	(SP),HL
	LD	D,A
	CALL	IRMON
	CP	28H
	JR	NC,WINERR
	LD	A,(WNDFN+92)
	LD	E,A
	CP	28H
	JR	NC,WINERR
	LD	A,D
	SUB	E
	JR	C,WINERR
	INC	A
	LD	(WINLG),A
	LD	A,E
	LD	(WINON),A
	LD	A,(WNDFN+91)
	CP	20H
	JR	NC,WINERR
	LD	D,A
	LD	A,(WNDFN+90)
	CP	20H
	JR	NC,WINERR
	LD	E,A
	LD	A,D
	SUB	E
	JR	C,WINERR
	INC	A
	LD	(WINLG+1),A
	LD	A,E
	LD	(WINON+1),A
	JR	WINDO2
	;
WINDO1:	PUSH	HL
	CALL	IRMON
	LD	HL,100H		; Standardfenstergoesse bei BASIC:
	LD	(WINON),HL	; 30 Zeilen, 40 Zeichen breit
	LD	HL,1E28H	; (oben und unten eine Zeile ungenutzt)
	LD	(WINLG),HL
WINDO2:	LD	HL,0
	LD	(CURSO),HL
	CALL	IRMOF
	POP	HL
	RET
	;
WINERR:	CALL	IRMOF
SNERR2:	JP	SNER		; SN-ERROR

; MC-Programm laden:

BLOAD:	PUSH	HL
	LD	HL,ARGN
	XOR	A
	CALL	IRMWR		; ARGN=0
	POP	HL
	LD	E,10H		; LOAD
	JP	PV5

; in IRM schreiben (8000h bis BFFFh):

VPOKE:	CALL	EPRVL4
	PUSH	DE
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	EX	(SP),HL
	LD	DE,8000H	; IRM-Offset
	ADD	HL,DE
	JR	C,SNERR2	; < 8000h
	LD	D,A
	LD	A,0BFH
	CP	H
	JR	C,SNERR2	; > BFFFh
	LD	A,D
	CALL	IRMWR		; schreiben
	POP	HL
	RET

; alte Variante von BORDER bis CAOS 4.4
; schaltet RAM-8 (vermutlich nicht gewollt?)
;
;BORDER:
;	CALL	ARGVL1	; Wert abholen
;	RRCA
;	RRCA
;	RRCA
;	AND	60H	; Auswahl
;	LD	C,A
;	LD	B,9FH	; Maske
;	JP	TONB

; PRINT AT:

PRAT:	LD	A,(PRTFLG)
	BIT	1,A
	SET	1,A
	LD	(PRTFLG),A
	JR	NZ,SNERR2
	PUSH	DE
	PUSH	HL
	CALL	IRMON
	LD	HL,WINON
	LD	(WNDFN+90),HL
	LD	DE,WNDFN+80
	LD	BC,6
	LDIR			; Fenster retten
	LD	HL,0		; Fenster gross
	LD	(WINON),HL
	LD	HL,2028H
	LD	(WINLG),HL
	CALL	IRMOF
	POP	HL
	CALL	LOCAT
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
	LD	HL,WNDFN+80
	LD	DE,WINON
	LD	BC,6
	LDIR			; Fenster regenerieren
	CALL	IRMOF
	POP	HL
	POP	DE
	POP	BC
	RET
	;
PRAT2:	CALL	PRINT2
	JR	PRAT1

; Cursor positionieren:

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
	CALL	CPCOMM		; Komma?
	PUSH	DE
	CALL	ARGVL1		; Wert abholen
	POP	DE
	LD	C,A
	PUSH	HL
	LD	HL,WINLG
	CALL	IRMRD
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
LOC1:	CALL	IRMWR
	POP	HL
	RET

; Eingabe eines Zeichens:

INKEY:	PUSH	HL
	CALL	KBDS
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

; Tonausgabe:

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
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; 2. bis 4. Wert abholen
	POP	BC
	EX	(SP),HL		; Umschaltung auf ARG-Zeiger
	JR	SOUND1
	;
SOUND2:	CALL	TCHAR1		; weiterer Parameter?
	JR	Z,SOUND3
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; optionalen 5. Wert abholen
	EX	(SP),HL
	CALL	IRMWR
	INC	HL
	EX	(SP),HL
	CALL	TCHAR1		; weiterer Parameter?
	JR	Z,SOUND3
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; optionalen 6. Wert abholen
	EX	(SP),HL
	CALL	IRMWR
	EX	(SP),HL
SOUND3:	LD	E,35H		; UP-Nr. fuer TON
	POP	BC		; Stack bereinigen
	JP	PV5		; Tonausgabe aufrufen

; Punkt setzen:

PSET:	LD	B,1		; setzen
POINT:	PUSH	BC
	CALL	EPRVL4
	PUSH	HL
	LD	A,E
	LD	HL,HOR
	CALL	IRMWR		; X-Koordinate (low)
	LD	A,D
	INC	HL
	CALL	IRMWR		; X-Koordinate (high)
	EX	(SP),HL
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	EX	(SP),HL
	INC	HL
	CALL	IRMWR		; Y-Koordinate
	POP	HL
	CALL	GFARB
	LD	E,30H		; UP-Nr. 30H fuer PUSE
	POP	BC
	DEC	B
	JR	Z,POINT2
	DEC	E		; UP-Nr. 2FH fuer PUDE
POINT2:	CALL	PV5
	JR	C,FCERR		; FC-ERROR	(bisher SN-ERROR)
	RET

; Punkt loeschen:

PRESET:	LD	B,0		; loeschen
	JR	POINT

; Grafikfarbe einstellen:

GFARB:	CALL	TCHAR1
	RET	Z
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	PUSH	HL
	LD	HL,FARB		; Grafik-Farbe
	RLA
	RLA
	RLA
	JP	LOC1		; in IRM schreiben

; Vordergrundfarbe einstellen:

INK:	CALL	ARGVL1		; Wert abholen
	CP	32
	JR	NC,FCERR	; FC-ERROR	(bisher SN-ERROR)
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	D,A
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMRD
	POP	HL
	AND	7
	JR	COL2

; Vordergrund- und Hintergrundfarbe einstellen:

BCOLOR:	CALL	TCHAR1
	JR	Z,FCERR		; FC-ERROR	(bisher SN-ERROR)
	CALL	INK		; Vordergrund
	CALL	TCHAR1
	RET	Z
	CALL	CPCOMM		; Komma?
;	JR	PAPER		; Hintergrund

; Hintergrundfarbe einstellen:

PAPER:	CALL	ARGVL1		; Wert abholen
COL1:	CP	8
	JR	NC,FCERR	; FC-ERROR	(bisher SN-ERROR)
	LD	D,A
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMRD
	POP	HL
	AND	0F8H
COL2:	OR	D
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMWR		; neuer Farbwert
	POP	HL
	RET
	;
FCERR:	JP	FCER		; FC-ERROR

; String1 in String2 suchen

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
INSTR0:	PUSH	BC
	PUSH	DE
	LD	A,(DE)
INSTR1:	CP	(HL)
	JR	Z,INSTR5
	INC	HL
	DEC	C
	JR	NZ,INSTR1
INSTR2:	XOR	A
	POP	HL
	POP	HL
	POP	HL
INSTR3:	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
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
	JR	INSTR0
	;
INSTR7:	INC	DE
	DEC	B
	POP	HL
	JR	NZ,INSTR2
	JR	INSTR9
	;
INSTR8:	POP	HL
INSTR9:	POP	DE
	POP	DE
	POP	DE
	AND	A
	SBC	HL,DE
	LD	A,L
	JR	INSTR3

; Byte in IRM schreiben:

IRMWR:	LD	E,28H		; UP-Nr. fuer LDMA
	JR	JPV5

; Byte aus IRM lesen:

IRMRD:	LD	E,29H		; UP-Nr. fuer LDAM
	JR	JPV5

; F-Taste belegen:

BKEY:	CALL	ARGVL1		; Wert abholen
	CP	16		; F0 ... F15 zugelassen
	JR	NC,SNERR4
	LD	E,39H		; UP-Nr. fuer KEY
JPV5:	JP	PV5

; F-Tasten auflisten:

KEYLIST:
	LD	E,3AH		; UP-Nr. fuer KEYLI
	JR	JPV5

; Module schalten:

SWITCH:	CALL	ARGVL1		; Wert abholen
	PUSH	AF
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; Wert abholen
	LD	D,A		; Steuerbyte
	POP	AF
	PUSH	HL
	LD	L,A		; Steckplatz
	LD	A,2		; schalten
	LD	E,26H		; UP-Nr. fuer MODU
	CALL	PV5
	POP	HL
	RET

; Testen, ob Bildpunkt gesetzt ist:

PTEST:	CALL	SNLY14		; Syntax-Check '('
	EX	(SP),HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	EPRVL3
	PUSH	HL
	LD	A,E
	LD	HL,HOR		; nur X-Koordinate eintragen
	CALL	IRMWR
	LD	A,D
	INC	HL		; Y-Koordinate
	CALL	IRMWR
	LD	E,2FH		; UP-Nr. fuer PUDE
	CALL	PV5
	JP	C,FCER		; ausserhalb -> FC-ERROR
	LD	B,0
	JR	Z,PTST1		; war geloescht
	INC	HL
	INC	HL
	CALL	IRMWR		; Grafikfarbe
	LD	E,30H		; UP-Nr. fuer PUSE
	CALL	PV5		; Punkt wieder setzen
	LD	B,1
PTST1:	LD	A,B		; Rueckgabewert
	JR	JHPOS		; POP HL, JP POS1

; IRM lesen (8000h bis BFFFh):

VPEEK:	CALL	SNLY14		; Syntax-Check '('
	EX	(SP),HL
	LD	DE,SNLY16	; Rueckkehr zu Syntax-Check ')'
	PUSH	DE
	CALL	EPRVL3
	PUSH	HL
	LD	HL,8000H	; IRM-Offset
	ADD	HL,DE
	JR	C,SNERR4	; < 8000h
	LD	A,0BFH
	CP	H
	JR	C,SNERR4	; > BFFFh
	CALL	IRMRD
	JR	JHPOS		; POP HL, JP POS1

SNERR4:	JP	SNER		; SN-ERROR

; BASIC-Erweiterung 3: Funktionen

BEXP3:	LD	A,C
	CP	62H
	JR	Z,VPEEK
	CP	6EH
	JR	Z,PTEST
	CP	7CH
	JR	Z,CSRLN
	CP	76H
	JR	Z,VGET
	SUB	3EH
	JR	C,SNERR4
	CP	7
	JR	NC,SNERR4
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

; aktuelle Zeilennummer holen:

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

; Zeichen von Bildschirm zurueck lesen

VGET:	EX	(SP),HL
	LD	A,1
	CALL	STADTB
	CALL	IRMON
	LD	DE,(CURSO)
	CALL	DABR
	LD	A,(HL)
	CALL	IRMOF
	JP	INKEY1		; als Zeichen zurueckgeben

; Kanal schliessen:

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

; Zufallsgenerator:

RANDOM:	LD	A,R
	LD	(RNDVR3+2),A
	RET

; Kanal oeffnen:

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
	CALL	CO		; Ausgabe eines Zeichens
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

; Linie zeichnen:

BLINE:	LD	BC,43EH		; 4 Parameter, LINE
	JR	GRAPH

; Kreis zeichnen:

CIRCLE:	LD	BC,33FH		; 3 Parameter, CIRCLE
GRAPH:	PUSH	BC
	CALL	EPRVL4
	POP	BC
	PUSH	BC
	PUSH	HL
	LD	HL,ARG1
GRAPH1:	LD	A,E
	CALL	IRMWR		; Ablage der Parameter
	INC	HL
	LD	A,D
	CALL	IRMWR
	INC	HL
	DEC	B
	JR	Z,GRAPH2	; fertig
	EX	(SP),HL
	PUSH	BC
	CALL	CPCOMM		; Komma?
	CALL	EPRVL4		; naechsten Wert
	POP	BC
	EX	(SP),HL
	JR	GRAPH1
	;
GRAPH2:	POP	HL
	CALL	GFARB
	POP	DE
	JP	PV5

; Laufwerk wechseln
;	Aufruf: DRIVE Lw[,User]
;	Lw = Laufwerk 0..15 fuer A: bis P:
;	User = 0..31 (2. Parameter optional)

BDRIVE:	LD	BC,UROK		; 0B3F3H
	IN	A,(C)
	CP	5		; D004 KC-Betriebsart?
	JR	NZ,BDR2		; nein -> SN-ERROR
	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	C,BDR2		; nein -> SN-ERROR
	CALL	ARGVL1		; Wert holen
	CP	16
	JR	NC,BDR2		; Laufwerk > 15
	ADD	A,'A'		; in A..P konvertieren
	LD	BC,LWANF
	OUT	(C),A		; LW-Anforderung
	CALL	TCHAR1		; weiterer Parameter?
	LD	A,3		; nur LW-Wechsel
	JR	Z,BDR1
	CALL	CPCOMM		; Komma?
	CALL	ARGVL1		; optionalen 2. Wert holen
	CP	32
	JR	NC,BDR2		; USER > 31
	LD	BC,UANF
	OUT	(C),A		; USER-Anforderung
	LD	A,7		; Laufwerk+USER wechseln
BDR1:	CALL	IRMON		; IRM ein
	CALL	CON		; CAOS-C ein
	CALL	DRIVE5		; ueber Steuerbyte 2 anfordern
	CALL	COFF		; CAOS-C aus
	CALL	IRMOF		; IRM aus
	RET
BDR2:	JP	SNER		; SN-ERROR

; Sprung zu ?IO ERROR
JIOERR:	CALL	COFF
	LD	HL,(IOERR)
	CALL	IRMOF
	JP	(HL)

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

	ABSFILL	0EDFFh,<ROM-E-Ende>

VERSION:
	DB	46H		; CAOS-Version 4.6
