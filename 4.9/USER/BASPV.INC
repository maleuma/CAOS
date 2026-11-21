;------------------------------------------------------------
; Include-Datei zu CAOS 4.9 (BASIC-Ein/Ausgabe)
; ausgelagert vom ROM-E bzw. ROM-F in den USER-ROM
;
; Erweitert um einen Fullscreen-Editor fuer BASIC
;
; Wird nicht aktiviert bei veraenderter Ausgabe oder Eingabe,
; zum Beispiel bei BASICODE (leitet zum Konvertieren des ASCII-
; Files zu BASIC die Eingaberoutine um und zur Anzeige in der
; letzten Bildschirmzeile die Ausgaberoutine).
;
; BASPV.INC				26.03.2023-17.11.2026
;------------------------------------------------------------

BASICODE EQU	1		; Modifikation fuer BASICODE

INPBUF	EQU	0361H+1		; BASIC-Eingabepuffer
LIN4	EQU	0C414H		; Ausgabe Prompt im BASIC-ROM
SCRLPG	EQU	0E271H		; Scroll-UP in CAOS 4.9 (manuell eintragen!)
ZEIG4	EQU	0F017H		; Zeiger zu UP 04 (KBD) = Tastatureingabe

IF1
	if BASICODE
	.PRINTX	'FullScreen-Editor mit BASICODE'
	else
	.PRINTX	'FullScreen-Editor ohne BASICODE'
	endif
ENDIF

; UP-Nummer ermitteln fuer BASIC-I/O-Verteiler
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
;	Bit 4	BIT8 - ungenutzt?
;	Bit 5	Ruecksprung zu CAOS (BYE)
;	Bit 6	Close = Datei schliessen
;	Bit 7	Abfrage Tastaturstatus
;	D	Datenbyte bei Ausgabe
; PA:	A	Ausgabebyte fuer OCHR, MBOUT, UOT1, UOT2
;	D	Steuerbyte fuer MBIN, MBOUT
;	E	UP-Nummer-1
;	E=0FFH	ungueltigen UP-Nummer, um Ausgabe zu unterdruecken
; VR:	AF, BC, DE, HL

BASPV:	LD	A,E		; Kanalnummer
	AND	7		; 0-3
	JR	Z,BKEY		; 0=Tastatur (Fullscreen-Editor einbinden)
	DEC	A
	JR	Z,BCRT		; 1=Bildschirm (Prompt abfangen)
	LD	HL,BUPTAB-1	; Tabelle der Unterprogramme
	ADD	A,L		; Kanalnummer addieren
	LD	L,A		; und wieder nach HL
	LD	A,D		; Ausgabebyte
	LD	D,E		; Steuerbyte (fuer MBIN/MBOUT)
	LD	E,(HL)		; UP-Nummer-1
	RET

; High-Byte der Adresse muss konstant sein!
; je 1 Byte fuer Eingabe + Ausgabe pro Kanal

BUPTAB:	;DB	INTB, OCHR	; Kanel #0 - Konsole
	DB	MBIN-1, MBOUT-1	; Kanal #1 - Kassette (Device)
	DB	USIN1-1, UOT1-1	; Kanal #2 - Anwenderkanal 2
	DB	USIN2-1, UOT2-1	; Kanal #3 - Anwenderkanal 3
;
; Prompt abfangen:
; ----------------
;
BCRT:	LD	C,D		; auszugebendes Zeichen
	LD	HL,(OUTAB)	; Zeiger auf
	LD	A,(HL)		; aktuellen Ausgabekanal
	CP	CRT		; modifiziert?
	JR	NZ,BPV1		; ja, dann nicht eingreifen
	LD	DE,(CURSO)
	CALL	PV1
	DB	DABR		; VRAM-Adresse ermitteln
	LD	A,(HL)		; aktuelles Zeichen
	AND	11011111b	; Zeichenposition leer oder Leerzeichen?
	JR	Z,BPV1		; ja, anzeigen
	IN	A,(88H)
	LD	B,A
	RES	2,A		; IRM aus
	OUT	(88H),A
	LD	L,(IY+18)
	LD	H,(IY+19)	; BASIC-SP-Zeiger
	LD	A,B
	OUT	(88H),A		; IRM wieder ein
	LD	DE,LIN4+5	; Ruecksprungadr muss
	SBC	HL,DE		; hinter CALL CHARO zeigen
	LD	E,0FFH		; ungueltige UP-Nummer
	RET	Z		; -> Prompt unterdruecken
BPV1:	LD	A,C		; Ausgabebyte
	LD	E,OCHR-1	; Bildschirmausgabe
	RET
;
; EDITOR-ROUTINE
; --------------
;
BKEY:	DB	0FDH		; Register HY
	LD	A,H		; Stack unterhalb 500h?
	CP	5		;  dann Startumgebung fuer
	JP	C,BPV2		;  Eingabe "MEMORY END"
;
; BASICODE lenkt die Eingaberoutine auf USER-INPUT 2 um, um die
; Konvertierung ASCII nach BASIC einzubinden. Deswegen wird bei einem
; modifizierten Eingabekanal der Fullscreen-Editor nicht aktiviert.
;
	LD	HL,(INTAB)	; Zeiger auf
	LD	A,(HL)		; aktuellen Eingabekanal
	CP	KBD		; modifiziert?

	IF BASICODE
; Der Fullscreen-Editor wird dadurch in BASICODE gar nicht mehr aktiv
; Ausser, man testet, ob gerade eine Konvertierung laeuft.
; Dazu muss man aber eine Speicherzelle testen, die sich auch aendern koennte
; in einer spaeteren Version...
;
	JR	Z,BX		; keine modifizierte Eingaberoutine
	LD	A,(003BH)	; Bit 0 zeigt an, dass eine Konvertierung laeuft
	RRA			; laeuft gerade Konvertierung ASCII -> BASIC?
	JP	C,BPV2		; ja, dann nicht eingreifen
BX:
	ELSE
	JR	NZ,BPV2		; ja, dann nicht eingreifen
	ENDIF

	IN	A,(88H)
	LD	C,A
	RES	2,A		; IRM aus
	OUT	(88H),A
	LD	L,(IY+14)	; BASIC-SP-Zeiger
	LD	H,(IY+15)
	LD	DE,LIN4+8	; Ruecksprungadr muss
	SBC	HL,DE		; hinter CALL EING zeigen
	LD	A,C
	OUT	(88H),A		; IRM wieder ein
	JR	NZ,BPV2		; nein, dann nicht umleiten
	LD	A,(WINLG)	; Wenn Breite=40, dann muss Beginn Spalte 0 sein
	CP	40		; ist Fenster 40 Spalten breit?
	JR	NZ,BPV2		; nein, dann nicht umleiten
	LD	HL,(WEND)
	LD	DE,SCRLPG	; Adresse fuer Scrollmodus
	SBC	HL,DE
	JR	NZ,BPV2
	JR	BKEY2		; ja, dann umleiten zu INLIN
;
BKEY1:	JP	PE,BKEY6	; BASIC-Zeile voll?
	POP	AF
	CALL	PV1
	DB	OSTR
	DB	0AH,7,0		; CUD, BEEP
	;JR	BKEY2		; zurueck nach INLIN, der Cursor steht hinter 
				; dem letzten uebernommenen Zeichen
BKEY2:	IF BASICODE
	LD	HL,(INTAB)
	PUSH	HL		; INTAB retten (wegen BASICODE)
	LD	HL,ZEIG4	; KBD erzwingen
	LD	(INTAB),HL
	ENDIF
	CALL	PV1
	DB	INLIN		; Zeileneingabe, Editieren auf ganzem Bildschirm
	IF BASICODE
	POP	HL
	LD	(INTAB),HL
	ENDIF
;	LD	A,03H		; wird von INLIN geliefert
	JR	C,BPV3		; Uebergabe <BRK> an BASIC
	PUSH	AF		; Enter oder RUN
	ld	hl,(curso)	; Cursorposition unterhalb der Eingabezeile
	ex	de,hl		; HL=VRAM-Adresse / DE=Cursorpos.

;Test, ob wir am Anfang der BASIC-Zeile stehen, dazu das letzte Zeichen auf
;der vorherigen Zeile testen. Das ist dann in der Regel 00H.
;Ansonsten noch eine Zeile zurueck gehen vor der Uebertragung in den Puffer
	dec	d		; zurueck auf Beginn der Eingabezeile
	JR	Z,BKEY4		; Test ueberspringen wenn Cursor in Zeile 0
	dec	HL
	ld	a,(HL)		; letztes Zeichen der vorherigen Zeile
	inc	HL
	or	a		; dort leer?
	jr	z,BKEY4		; ja
	LD	BC,-40		; Laenge der Bildschirmzeile zurueck
	add	hl,bc		; im VRAM eine Zeile zurueck
	dec	D		; Cursor noch eine Zeile zurueck

BKEY4:	LD	E,74-40-1	; Cursorspalte falls BASIC-Zeile zu lang
	LD	A,(HL)		; erstes Zeichen der Eingabezeile
	CP	'>'		; Prompt am Zeilenanfang?
	JR	NZ,BKEY5	; nein
	INC	HL		; Prompt uebergehen im VRAM
	INC	E		; und auch am Bildschirm
BKEY5:	ld	(curso),DE	; neue Position setzen
				; HL=Zeilenbeginn im VRAM
	LD	DE,INPBUF	; DE=BASIC-Eingabepuffer
	LD	BC,74		; BC=Puffergroesse
	XOR	A
BKEY6:	CP	(HL)		; Zeilenende=0?
	LDI			; ein Zeichen in Puffer kopieren
	JR	NZ,BKEY1	; bis Ende-0
	ld	HL,CURSO
	LD	(HL),A		; Cursor auf Zeilenanfang
	POP	AF
;	LD	A,0DH		; Uebergabe <ENTER>
BPV3:	ld	(ix+13),a	; als Tastencode
	set	0,(ix+8)	; Tastencode aktivieren
BPV2:	LD	E,INTB-1	; mit der normalen Eingaberoutine (KBD)
	RET
