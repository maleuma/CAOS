;*****************************************
;**	CAOS 4.6	ROM D		**
;**					**
;**	Adresse:  D000h bis DAB7h	**
;**					**
;**	letzte Aenderung: 03.03.2016	**
;*****************************************

;	ORG	0D000H

;*******************************************************************************
;	CAOS 4.6 - Floppy-Treiber fuer D004/D008
;*******************************************************************************
;
; Eingabedatei oeffnen und Block 01 einlesen		** 0A **
; PE:	HL	Dateiname
; PA:	CY=1	Fehler
;
FISRID:	LD	(IX+2),0	; 1. Block lesen
	LD	(IX+3),1	; erwarteter Block = 1
	CALL	NAMOUT		; Dateiname ausgeben zum D004
	LD	A,9		; Lesen oeffnen
	DB	21h		; LD HL,nn
;
; Einlesen Datenblock 128 Byte				** 05 **
; PE:	(IX+3)	erwartete Blocknummer
; PA:	(IX+2)	gelesene Blocknummer
;	CY=1	Lesefehler
; VR:	AF,BC
;
FMBID:	LD	A,1		; Lesen normal
	CALL	STEUER
	RET	C		; Fehler
	DEC	C		; STBYT1 -> FDATA (80F2H)
	PUSH	HL
	LD	L,(IX+5)
	LD	H,(IX+6)	; Kassettenpuffer
FMBI1:	IN	A,(C)
	CALL	LDMAE		; Daten in Speicher ablegen
	INC	HL
	INC	B
	JR	NZ,FMBI1	; CY von allen Befehlen nach RET C unbeeinflusst
	POP	HL
;	LD	A,(IX+3)	; erwarteten Block
;	LD	(IX+2),A	; als gelesenen Block melden
	INC	(IX+2)		; Blocknummer erhoehen
	RET
;
; Eingabedatei schliessen				** 0B **
; PE:
; PA:	CY=1	bei Fehler
;
FCSRID:	LD	A,41H		; READ - close
	JR	STEUER
;
; Ausgabedatei oeffnen und ersten Block 01h ausgeben	** 08 **
; PE:	Kassettenpuffer mit Daten des Vorblocks
;	Dateiname am Anfang des Puffers
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL
;
FISROD:	LD	(IX+2),0	; mit Block 1 beginnen
	LD	L,(IX+5)	; HL = Dateiname im Kassettenpuffer
	LD	H,(IX+6)
	CALL	NAMOUT		; Dateiname ausgeben zum D004
	LD	A,0BH		; Schreiben oeffnen
	DB	21h		; LD HL,nn
;
; Ausgabe Datenblock 128 Byte				** 01 **
; PE:	Datenblock im Kassettenpuffer
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL
;
FMBOD:	LD	A,3		; Schreiben normal
	INC	(IX+2)		; Blocknummer erhoehen
	LD	L,(IX+5)	; Kassettenpuffer
	LD	H,(IX+6)
	LD	BC,FDATA	; 80F2H
	LD	E,128
	EX	AF,AF'		; Kommando in A' merken
FMBO1:	 CALL	LDAME		; Daten aus Speicher holen
	 OUT	(C),A		; und in Koppel-RAM schreiben
	 INC	HL
	 INC	B
	 DEC	E
	 JR	NZ,FMBO1
	EX	AF,AF'		; Kommando zurueck nach A
;
; Steuerbyte 1 ausgeben, bei Fehler: Fehlercode/-text anzeigen
; PE:	A	Kommando-Steuerbyte
; PA:	A	Antwort von DEP
;	CY=1	Fehler aufgetreten
; VR:	AF,BC
;
STEUER:	LD	BC,STBYT1	; 80F3H
STEU0:	OUT	(C),A
STEU1:	PUSH	BC
	LD	A,1		; 6ms
	CALL	WAIT		; warten
	POP	BC
	IN	A,(C)
	AND	A		; CY-Flag=0
	BIT	0,A
	JR	NZ,STEU1	; fertig?
	BIT	7,A		; Bit 7 gesetzt?
	RET	Z		; kein Fehler
FLERR:	LD	BC,DEPVER	; 83F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	NC,FLER1	; ja
	LD	BC,FCODE	; 81F3H
	IN	A,(C)		; Fehlernummer
	CALL	AHEX		; anzeigen
	CALL	ERRM
FLER3:	SCF			; CY=1: Fehler
	RET
	;
FLER1:	LD	B,HIGH(ERRTX)	; 00F1H = Fehlertext von DEP 2.0
FLER2:	IN	A,(C)
	INC	B
	AND	A
	JR	Z,FLER3		; 00=Ende
	CALL	OCHR
	JR	FLER2

; Steuerbyte 2 ausgeben

STEUB2:	LD	BC,STBYT2	; 80F1H
	JR	STEU0
;
; Ausgabedatei schliessen und Endeblock FFh ausgeben	** 09 **
; PE:	Datenblock im Kassettenpuffer
; PA:	CY=1	bei Fehler
; VR:	AF,BC,DE,HL
;
FCSROD:	LD	(IX+2),0FEH	; Nr. letzter Block
	CALL	FMBOD		; Datenblock ausgeben
	RET	C		; Fehler
	LD	A,43H		; write close
	JR	STEUER

; Dateiname ausgeben
; PE:	HL	Dateiname (12 Zeichen)
; VR:	F,BC,E,HL

NAMOUT:	LD	E,12		; Dateiname 8.3 = 12 Zeichen
NAMO1:	LD	BC,DIRBUF+100H	; 83F3H
NAMO2:	OUTI			; Dateiname ausgeben zum D004
	INC	B
	INC	B
	DEC	E
	JR	NZ,NAMO2
	RET
;
; Byteeingabe (BASIC-Schnittstelle)			** 37 **
; PE:	D	Steuerbyte
;		Bit 3 = 1 INIT (Block 01)
;		Bit 6 = 1 Close (Block FF)
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
; PA:	A	Datenbyte
; VR:	AF, DE, HL
;
FMBIND:	LD	A,D		; Steuerbyte
	AND	01001000b	; nur Bit 3 und 6 ausfiltern
	OR	35H		; BASIC-Byte lesen
	CALL	NAMBAS		; Dateiname ausgeben
	LD	B,HIGH(BASBYT)	; 08EF3H
	IN	A,(C)		; Datenbyte abholen
	RET
;
; Byteausgabe (BASIC-Schnittstelle)			** 38 **
; PE:	A	Datenbyte
;	D	Steuerbyte
;		Bit 3 = 1 INIT (Block 01)
;		Bit 6 = 1 Close (Block FF)
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
; PA:	-
; VR:	AF,DE, HL
;
FMBOUD:	LD	BC,BASBYT	; 08EF3H
	OUT	(C),A		; Datenbyte ablegen
	LD	A,D		; Steuerbyte
	AND	01001000b	; nur Bit 3 und 6 ausfiltern
	OR	37H		; BASIC-Byte schreiben
;	BIT	6,A		; schliessen?
;	JR	Z,NAMBAS	; nein
;	LD	D,A
;	LD	A,(0200h)	; RUN ???
;	AND	A
;	LD	A,D
;	JR	Z,O1
;	SET	7,A		; run-switch im Steuerbyte setzen

; Dateiname + Steuerbyte ausgeben:
; PE:	A	Steuerbyte
;	HL	Dateiname bei gesetzem Bit 3 im Steuerbyte
; PA:	BC	80F3H (Steuerbyte 1)
;	Absprung zu I/O-Error wenn Fehler aufgetreten ist
; VR:	AF,BC,E,HL

NAMBAS:	LD	E,11		; 11 Zeichen (hier TYP+Name ohne Punkt)
	BIT	3,A		; OPEN-Bit gesetzt?
	CALL	NZ,NAMO1	; Dateiname ausgeben bei Bedarf
	LD	BC,STBYT1	; 80F3H (Steuerbyte 1)
	OUT	(C),A		; Steuerbyte ausgeben
NB1:	IN	A,(C)		; Antwort von DEP abfragen
	BIT	0,A		; abwarten, bis D004 fertig meldet
	JR	NZ,NB1
	RLCA			; Bit 7 nach CY rotieren
	RET	NC		; kein Fehler
	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	NC,BERR2	; ja
	LD	BC,FCODE	; 081F3H
	IN	A,(C)
	CALL	AHEX		; Fehler-Nr.
BERR1:	JP	JIOERR		; CAOS-C aus, IRM aus, Sprung zu I/O-ERROR
	;
BERR2:	LD	B,HIGH(ERRTX)	; Fehlertext - Register C ist bereits korrekt
BERR3:	IN	A,(C)
	JR	Z,BERR1		; 00-Ende
	INC	B
	CALL	OCHR
	JR	BERR3

; Test ob Floppy arbeitet (fuer SETWR, SETRO, time)

DISK:	LD	BC,0FC80H	; Modulsteckplatz FC
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	Z,DISK1
	CALL	OSTR
	DB	'Kein D004!'
	DB	7,CR,LF,0
	POP	HL		; RET-Adresse vom Stack nehmen
	RET			; und Kommando beenden!
	;
DISK1:	LD	BC,UROK		; 0B3F3H
	IN	A,(C)
	CP	5		; CAOS-Betriebsart?
	RET	Z
	CALL	OSTR
	DB	'CAOS-Disk starten!'
	DB	7,CR,LF,0
	POP	HL		; RET-Adresse vom Stack nehmen
	RET			; und Kommando beenden!

SRO7F:	DW	7F7FH
	DB	'SETRO'
	DB	1FH		; Textargument zulaessig (Dateiname)
	CALL	DISK		; D004 bereit?
	CALL	FNAME		; Dateiname abfragen und ausgeben zum D004
	LD	A,25H		; Setro
	JR	DOUT

SWR7F:	DW	7F7FH
	DB	'SETWR'
	DB	1FH		; Textargument zulaessig (Dateiname)
	CALL	DISK		; D004 bereit?
	CALL	FNAME		; Dateiname abfragen und ausgeben zum D004
	LD	A,31H		; Setwr
DOUT:	RET	C		; BRK
	JP	STEUER		; Kommando ausgeben
;
; Name ein-, ausgeben: (fuer REN, SETRO, SETWR)
; PE:	DE	Zeiger auf Dateinamen, wenn 0 dann Eingabe
; PA:	CY=1	BRK
;	DE	Zeiger auf Zeichen nach Dateinamen (2. Name bei REN)
;
FNAME:	EX	DE,HL
	LD	A,(HL)		; Zeichen nach Menuewort holen
	AND	0DFH
	JR	NZ,FNAM2	; Argument angegeben, als Dateiname nutzen!
FNAM0:	LD	HL,6
FNAM1:	CALL	NAMHL		; Dateiname eingeben
	RET	C		; BRK
FNAM2:	LD	E,12		; Dateiname 8.3 = max. 12 Zeichen
	LD	BC,DIRBUF	; 082F3H
FNAM3:	LD	A,(HL)
	OUT	(C),A		; Zeichen in DIRBUF eintragen
	AND	0DFH		; war das ein Leerzeichen oder 0?
	JR	Z,FNAM4
	INC	HL		; ansonsten weiter zum naechsten Zeichen
FNAM4:	INC	B
	DEC	E
	JR	NZ,FNAM3
	EX	DE,HL		; Zeiger jetzt wieder in DE
	INC	DE		; Trennzeichen uebergehen
	RET

; CAOS-D-Teil der Pixelausgaberoutine WPIX

WPIXD:	LD	A,C
	LD	BC,0DC00H
	ADD	A,A
	JR	C,WPX02		; 80..FF
	DEC	B
	CP	40H
	JR	C,WPX02		; 00..1F
	LD	B,0EEH-1	; Grossbuchstaben
	CP	2*5BH
	JR	C,WPX02		; 20..5A
	CP	2*5EH
	JR	C,WPX01		; 5B..5D
	CP	2*60H
	JR	C,WPX02		; 5E..5F
	JR	Z,WPX01		; 60
	LD	B,0FEH-2	; Kleinbuchstaben
	CP	2*7BH
	JR	C,WPX02		; 61..7A
WPX01:	LD	BC,0DAB8H	; Zusatztabelle
	SUB	2*5BH
	CP	6
	JR	C,WPX02		; 5B..5D
	SUB	4
	CP	6
	JR	Z,WPX02		; 60
	SUB	34H
WPX02:	LD	L,A
	LD	H,0
	ADD	HL,HL		; *4
	JP	WPX07		; Zurueck zu CAOS-E

; Rolle Fenster fuer Scrollroutine

MOVELN:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	EX	AF,AF'
	PUSH	AF
	LD	A,C		; BC: Pixelzeilen
	EX	AF,AF'
	LD	A,(WINLG)	; Spalten
MOLN1:	PUSH	HL
	PUSH	DE
MOLN2:	LDI			; zeichenweise
	LDI
	LDI
	LDI
	LDI
	LDI
	LDI
	LDI
	JP	PE,MOLN2
	POP	DE
	POP	HL
	INC	H		; neue Spalte
	INC	D
	EX	AF,AF'
	LD	C,A
	EX	AF,AF'
	DEC	A		; Spaltenzaehler
	JR	NZ,MOLN1
	EX	AF,AF'
	POP	AF
	EX	AF,AF'
	JP	POP4		; POP	AF,BC,DE,HL   RET

; Loeschen einer Zeile
; HL: Adresse, C: Laenge, A: Byte

CLLINE:	LD	B,C
CLL1:	LD	D,L		; L retten
	LD	(HL),A		; zeichenweise
	INC	L		; ohne Schleife
	LD	(HL),A
	INC	L
	LD	(HL),A
	INC	L
	LD	(HL),A
	INC	L
	LD	(HL),A
	INC	L
	LD	(HL),A
	INC	L
	LD	(HL),A
	INC	L
	LD	(HL),A
	LD	L,D		; L holen
	INC	H
	DJNZ	CLL1
	RET

; CAOS-D-Teil der Scrollroutine SCRLPG

SCRLD:	LD	A,(WINLG)	; Spalten
	LD	C,A
	LD	B,0
	LD	A,(WINLG+1)	; Zeilen
	DEC	A		; nur 1 Zeile?
	JR	Z,SCRL3		; nur CLLN
	PUSH	DE
	PUSH	AF
	LD	D,B		; DE=0
	LD	E,B		; Cursor links/oben
	CALL	DABR
	EX	DE,HL
SCRL1:	LD	HL,40		; 1 Zeile tiefer
	ADD	HL,DE
	PUSH	HL
	PUSH	BC
	LDIR			; im VRAM eine Zeile rollen
	POP	BC
	POP	DE
	DEC	A
	JR	NZ,SCRL1	; A-mal rollen
	POP	AF
	ADD	A,A
	ADD	A,A
	ADD	A,A		; *8: Pixelzeile
	LD	C,A
	LD	D,B		; B war 0
	LD	E,B		; DE=0
	CALL	PADR1
	EX	DE,HL		; DE=Pixeladresse
	LD	L,8		; H war 0
	ADD	HL,DE
	LD	A,(STBT)
	RRA
	RRA
	JR	C,SCRL2		; Farbe AUS
	PUSH	AF
	CALL	ESC9
	CALL	MOVELN
	CALL	ESC9
	POP	AF
SCRL2:	RLA			; Pixel AUS?
	CALL	NC,MOVELN
	POP	DE
SCRL3:	DEC	D

; CAOS-D-Teil der Zeilenloeschroutine CLLN (Shift-DEL)

CLLND:	LD	E,0
	CALL	DABR		; ausserhalb?
	RET	C		; (Selbstschutz)
	LD	A,(WINLG)
	LD	B,A		; Spalten
	LD	C,A
	XOR	A
CLLN1:	LD	(HL),A		; VRAM loeschen
	INC	HL
	DJNZ	CLLN1
	CALL	PADR1
	RET	C
	PUSH	DE
	BIT	3,(IX+1)
	LD	A,(STBT)
	JR	NZ,CLLALT	; LORES
	BIT	6,A
	JR	NZ,CLLHR	; HRG-Modus
CLLALT:	RRCA			; Pixel AUS?
	LD	E,A		; nach E merken
	JR	C,CLLN2
	XOR	A
	PUSH	HL
	CALL	CLLINE		; Pixel loeschen
	POP	HL
CLLN2:	RRC	E		; Farbe AUS?
	JR	C,CLLN3
	CALL	ESC9
	LD	A,(COLOR)
CLLCOL:	CALL	CLLINE		; Farbe setzen
	CALL	ESC9
CLLN3:	POP	DE
	RET

CLLHR:	LD	A,(COLOR)
	AND	00000011b	; Paper, CY=0
	RRA
	DEC	A		; NEG ohne Veraenderung von CY
	CPL
	LD	E,A		; E = FFh * Bit 1 (Farbe)
	SBC	A,A		; A = FFh * Bit 0 (Pixel)
	PUSH	HL
	CALL	CLLINE
	POP	HL
	CALL	ESC9
	LD	A,E		; Farbe
	JR	CLLCOL

	DW	7F7FH
	DB	'time',1
	CALL	DISK		; D004 bereit?
	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	30H		; DEP ab 3.0?
	JR	NC,UHR
	CALL	OSTR
	DB	'Kein DEP 3!'
	DB	7,CR,LF,0
	RET
UHR:	LD	A,19H		; Cursor ganz nach links
	CALL	OCHR
	LD	E,2
	LD	BC,DATE+200H
UHR1:	IN	A,(C)		; Datum aus Koppel-RAM
	CALL	AHEX
	LD	A,'.'
	CALL	OCHR
	DEC	B
	DEC	E
	JR	NZ,UHR1
	IN	A,(C)		; Jahr
	CP	78H
	LD	A,19H		; 1978-1999
	ADC	A,0
	DAA			; 2000-2077
	CALL	AHEX		; Jahrhundert
	IN	A,(C)
	CALL	AHEX		; Jahr
	CALL	OSTR
	DB	'  ',0
	LD	E,2
	LD	BC,TIME
UHR2:	IN	A,(C)		; Uhrzeit aus Koppel-RAM
	CALL	AHEX
	LD	A,':'
	CALL	OCHR
	INC	B
	DEC	E
	JR	NZ,UHR2
	IN	A,(C)
	CALL	AHEX
	LD	A,(ARGN)	; Parameter angegeben?
	AND	A		; =Dauerlauf?
	JR	Z,UHR3		; nein!
	CALL	BRKT		; BRK?
	JR	NC,UHR
UHR3:	JP	CRLF

; Quadratwurzel 16Bit A=SQR(HL)

SQRC:	AND	A		; CY=0
	LD	DE,0FFFFH
SQ1:	INC	DE
	INC	DE
	SBC	HL,DE
	JR	NC,SQ1
	LD	A,E
	SRL	D
	RRA			; A=RADIUS
	JP	COFF

; Multiplikation BA=C*D

MULC:	LD	B,0
	LD	E,9
MULC1:	LD	A,D
	RRA
	DEC	E
	JP	Z,COFF
	LD	D,A
	LD	A,B
	JR	NC,MULC2
	ADD	A,C
MULC2:	RRA
	LD	B,A
	JR	MULC1 		; 8*

; Speicherschnellansicht

VIEW7F:	DW	7F7FH
	DB	'view'
	DB	3
	LD	A,L
	CP	10H
	JR	NC,VIEW1	; L>0Fh
	LD	A,H
	AND	A
	JR	NZ,VIEW1	; H>0
	LD	HL,ARG1
	RLD
	LD	H,(HL)
	LD	L,0
VIEW1:	LD	DE,8800H
	LD	BC,2000H
VIEW2:	CALL	LDAME		; Byte holen
	LD	(DE),A		; in IRM poken
	INC	HL
	INC	DE
	DEC	C
	JR	NZ,VIEW2
	DJNZ	VIEW2
VIEW3:	LD	A,0BH		; Zeile hoch
	CALL	CRT
	LD	A,8
	LD	(CURSO),A
	CALL	VIEW4
VIEW4:	LD	A,1
VIEW5:	LD	C,A
	CALL	AHEX		; HEX-Leiste
	LD	A,C
	ADD	A,22H
	JR	NC,VIEW5
	RET

; Druckereinstellungen

PRI7F:	DW	7F7FH
	DB	'PRINT',1FH
PRI:	LD	A,(DE)
	AND	A
	RET	Z		; fertig
	CP	' '
	JR	Z,PRINC
	CP	','
	JR	Z,PRK		; Zeichen
	CP	27H
	JR	Z,PRSTR		; String
	CALL	RHEX
	JP	C,ERRM		; Fehler!
	LD	A,(NUMVX)
PRIC:	CALL	OFILT
PRINC:	INC	DE
	JR	PRI
	;
PRK:	INC	DE
	LD	A,(DE)		; Zeichen
	JR	PRIC
	;
PRSTR:	INC	DE
	LD	A,(DE)
	AND	A
	RET	Z		; Ende-0
	CP	27H
	JR	Z,PRINC		; Stringende
	CALL	OFILT
	JR	PRSTR

; Schreibe ON, ON* oder OFF fuer Systemcheck
; PE:	Z=1	OFF
;	Z=0 und CY=0 ON
;	Z=0 und CY=1 ON*

WONOF:	JR	Z,WOF
	PUSH	AF
	CALL	OSTR
	DB	' ON',0
	POP	AF
	LD	A,' '
	JR	NC,WRR
	LD	A,'*'		; Schreibschutz anzeigen
WRR:	CALL	OCHR
	JR	WRLF
	;
WOF:	CALL	OSTR
	DB	' OFF',0
WRLF:	JP	CRLF

; Anzeige der CAOS-Version mit Datum

HELP7F:	DW	7F7FH
	DB	'help'
	DB	1
	CALL	OSTR
	DB	' KC-Club CAOS 4.6 '
if beta
	DB	'beta '		; Beta-Version?
endif
	DATUM			; Versionsdatum
	DB	CR,LF,0
	RET

; * * * *	ab hier DEVICE-Treiber-Funktionen (neu)	* * * *

;
; Floppy-Treiber Funktion 0: Directory mit Maske
; PE:	DE	Maske
; PA:	-
;
FDIR:	EX	DE,HL		; Maske aus Kommandozeile
	LD	DE,CASS
	LD	BC,11		; max. Laenge
	LDIR			; In Kassettenpuffer ablegen
	XOR	A		; Abschluss Maske
	LD	(DE),A
	LD	A,0DH		; Directory oeffnen
DIR1:	CALL	STEUER
	BIT	2,A
	JR	NZ,DIR6		; Ende
	LD	B,HIGH(DIRBUF)	; 82F3H
	LD	HL,CASS+12
	AND	A		; CY=0
	LD	D,3		; 3 Eintraege
DIR2:	LD	E,11		; 11 Zeichen lang
DIR3:	IN	A,(C)		; einlesen
	PUSH	AF
	AND	7FH
	INC	B
	LD	(HL),A
	INC	HL
	LD	A,E
	CP	3		; RO-Byte?
	JR	Z,DIR4
	POP	AF
DIR4:	DEC	E
	JR	NZ,DIR3
	LD	(HL),0		; Ende-Kennung
	INC	HL
	INC	B
	POP	AF
	LD	(HL),A		; RO-Byte
	INC	HL
	DEC	D
	JR	NZ,DIR2
	LD	HL,CASS+12
	LD	B,3
DIR5:	PUSH	HL
	LD	DE,CASS
	LD	A,(DE)
	AND	A
	CALL	NZ,GLOB		; Vergleich mit Maske
	POP	HL
	CALL	NC,DIR7		; Anzeige eines passenden Eintrags
	LD	DE,13
	ADD	HL,DE
	DJNZ	DIR5		; 3 Eintraege
	LD	A,5		; DIR weiter
	JR	DIR1
	;
DIR6:	LD	A,(CURSO)
	AND	A		; Cursor auf neuer Zeile?
	CALL	NZ,CRLF		; wenn nicht, dann Zeilenwechsel

; Nach Ende aller Dateien noch die Statusmeldung anzeigen:

STAT:	LD	A,15H		; Status abfragen
	CALL	STEUER
	LD	E,20
	LD	B,HIGH(DIRBUF)	; 82F3H
STAT1:	IN	A,(C)
	AND	7FH		; Bit 7 abtrennen
	INC	B
	PUSH	AF
	CALL	OCHR
	POP	AF
	DEC	E
	JR	NZ,STAT1
	RET

; DIR-Eintrag anzeigen:

DIR7:	PUSH	HL
	LD	A,(CURSO)
	CP	3*13
	CALL	Z,CRLF
	CALL	ZKOUT
	LD	A,' '		; WR
	BIT	7,(HL)
	JR	Z,DIR8
	LD	A,'*'		; RO
DIR8:	CALL	OCHR
	CALL	SPACE
	POP	HL
	RET

; Vergleich mit DIR-Maske:

MAT0:	INC	HL
MAT0E:	INC	DE
GLOB:	; Eintritt, HL=Name, DE=Maske
	; PA: CY=1: nicht gefunden
	LD	A,(DE)
	OR	(HL)		; Beide Ende?
	RET	Z		; -> OK
	LD	A,(DE)
	CP	'*'
	JR	Z,MAT0S
	CP	1
	RET	C		; M Ende
	LD	A,(HL)
	CP	1
	RET	C		; N Ende
	LD	A,(DE)
	CP	'?'
	JR	Z,MAT0		; Joker
	BIT	5,(HL)
	CALL	Z,UPCASE
	CP	(HL)
	JR	Z,MAT0
	SCF
	RET
	;
MAT0S:	PUSH	HL
	PUSH	DE
	CALL	MAT0E		; Rekursion!
	POP	DE
	POP	HL
	RET	NC		; -> OK
	LD	A,(HL)
	INC	HL
	OR	A
	JR	NZ,MAT0S
	SCF
	RET
;
; Floppy-Treiber Funktion 2: Datei loeschen
; PE:	DE	Dateiname
;	(DE)=0	Dateinamen werden abgefragt
; PA:	CY=1	Fehler
;
FERA:	EX	DE,HL
	LD	A,(HL)		; Parameter in Kommandozeile
	AND	0DFH		; angegeben?
	CALL	Z,NAME		; Dateiname abfragen falls nicht angegeben
	CCF			; BRK nicht als Fehler anzeigen
	RET	NC		; Eingabe Dateiname mit BRK abgebrochen
	CALL	NAMOUT		; Dateiname ausgeben zum D004
	LD	A,11H		; Era
	CALL	STEUER		; mit Steuerbyte 1 im D004 anfordern
	RET	C		; Fehler aufgetreten
	JR	STAT
;
; Floppy-Treiber Funktion 3: Datei umbenennen
; PE:	DE	Zeiger auf Zeichenkette mit 2 Dateinamen
;	(DE)=0	Dateinamen werden abgefragt
; PA:	CY=1	Fehler
;
FREN:	LD	A,(DE)		; Parameter in Kommandozeile
	AND	0DFH		; angegeben?
	JR	NZ,REN1		; ja, dann diesen verwenden
	CALL	OSTR
	DB	'Alter ',0
	LD	HL,12
	CALL	FNAM1		; Dateiname abfragen
	RET	C		; BRK
	JR	REN2
	;
REN1:	EX	DE,HL		; Dateiname in HL
	CALL	FNAM2
REN2:	LD	A,29H		; REN alter Dateiname
	CALL	STEUER
	RET	C		; Fehler
	LD	A,(DE)		; Parameter in Kommandozeile
	AND	0DFH		; 2. Dateiname angegeben?
	JR	NZ,REN3		; ja, dann diesen verwenden
	CALL	OSTR
	DB	'Neuer ',0
	LD	HL,12
	CALL	FNAM1		; Dateiname abfragen
	RET	C		; BRK
	JR	REN4
	;
REN3:	EX	DE,HL		; Dateiname in HL
	CALL	FNAM2
REN4:	LD	A,21H		; REN neuer Dateiname
	CALL	STEUER		; Steuerbyte ausgeben
	RET	C		; Fehler
	CALL	OSTR
	DB	'OK.',CR,LF,0
	RET
;
; Anzeige einer beliebigen Text-Datei (nicht fuer TAPE-Modus)
;
TYPE7F:	DW	7F7FH
	DB	'TYPE'
	DB	1FH		; Textargument zulaessig (Dateiname)
	CALL	TDUP1		; Dateiname in HL
	RET	C		; TAPE oder BRK
	CALL	PV1		; HL = Dateiname
	DB	0AH		; ISRI (ersten Block einlesen)
	JR	C,FCLOSE	; Fehler aufgetreten
	LD	A,(CASS+10h)	; Anzahl ARG in Vorblock
	CP	2
	JR	C,TYPE4		; kein Vorblock, da < 2
	CP	5
	JR	NC,TYPE4	; kein Vorblock, da > 4
	JR	TYPE3		; Vorblock -> dann ersten Datenblock einlesen
	;
TYPE1:	LD	A,0CH		; CLS
	CALL	CRT		; CRT, damit nicht zu Druckprotokoll!
	LD	A,LF
	CALL	CRT		; oben eine Zeile frei lassen
TYPE2:	LD	A,E
	CP	80h		; Ende Kassettenpuffer erreicht?
	JR	NZ,TYPE5	; nein, weiter anzeigen
TYPE3:	CALL	PV1		; naechsten Block einlesen mit
	DB	05H		; MBI
	JR	C,TYPE10		; Fehler
TYPE4:	LD	DE,CASS		; Kassettenpuffer zur Entnahme der Daten
TYPE5:	LD	A,(DE)		; Zeichen
	INC	E		; Zeiger auf naechstes Zeichen
	CP	3
	JR	Z,TYPE10	; CAOS-Ende
	CP	1AH
	JP	Z,TYPE10	; CP/M-Ende
	CP	20H
	JR	NC,TYPE8	; ASCII-Zeichen
	CP	LF
	JR	Z,TYPE8
	CP	CR
	JR	Z,TYPE8
	CP	9		; TAB in Space umwandeln
	JR	NZ,TYPE7	; andere Steuerzeichen als Punkt anzeigen
TYPE6:	LD	A,' '
	CALL	OCHR
	LD	A,(CURSO)
	AND	7		; TAB-Stopp alle 8 Zeichen
	JR	NZ,TYPE6
	JR	TYPE9
	;
TYPE7:	LD	A,'.'		; statt Steuerzeichen anzeigen
TYPE8:	CALL	OCHR
TYPE9:	LD	A,(CURSO+1)
	INC	A
	LD	HL,WINLG+1
	CP	(HL)		; Seite voll?
	JR	NZ,TYPE2
	CALL	TDUP2		; Eingabe abwarten
	JR	NZ,TYPE1	; auf neuer Seite weiter anzeigen
TYPE10:	CALL	CRLF		; noch eine neue Zeile nach dem Text
	JR	FCLOSE		; Datei schliessen
;
; Hexadezimele Dateianzeige einer beliebigen Datei (nicht fuer TAPE-Modus)
;
DUMP7F:	DW	7F7FH
	DB	'DUMP'
	DB	1FH		; Textargument zulaessig (Dateiname)
	CALL	TDUP1		; Dateiname in HL
	RET	C		; TAPE oder BRK
	CALL	PV1		; HL = Dateiname
	DB	0AH		; ISRI (ersten Block einlesen)
	JR	NC,DUMP4	; kein Fehler aufgetreten
FCLOSE:	CALL	PV1		; Datei schliessen
	DB	0BH		; CSRI
	CALL	CLC		; Kassettenpuffer loeschen
	LD	A,(ARGC)	; urspruenglichen ROM-C Schaltzustand 
	LD	(MODST+5),A	; wieder eintragen
	JP	COFF		; und so schalten -> DUMP-Ende
	;
DUMP2:	LD	A,0CH		; CLS
	CALL	CRT		; CRT damit nicht zu Druckprotokoll!
	LD	A,LF
	CALL	CRT		; oben eine Zeile frei lassen
DUMP3:	LD	A,E
	CP	80h		; Ende Kassettenpuffer erreicht?
	JR	NZ,DUMP5	; nein, weiter anzeigen
	CALL	PV1		; naechsten Block einlesen mit
	DB	05H		; MBI
	JR	C,FCLOSE	; Fehler
DUMP4:	LD	DE,CASS		; Kassettenpuffer zur Entnahme der Daten
DUMP5:	EX	DE,HL
	CALL	SPACE		; Zeile mit Leerzeichen beginnen
	LD	C,8		; 8 Bytes
	CALL	DPME0		; Anzeigen mit der Display-Rutine
	EX	DE,HL
	CALL	CRLF		; neue Zeile
	LD	A,(CURSO+1)
	INC	A
	LD	HL,WINLG+1
	CP	(HL)		; Seite voll?
	JR	NZ,DUMP3	; nein, weiter anzeigen
	CALL	TDUP2		; Eingabe abwarten
	JR	Z,FCLOSE	; BRK -> Datei schliessen und Ende
	JR	DUMP2
;
; Abarbeitung von Kommandos aus einer Datei INITIAL.UUU wie F-Tastencode
; Laufwerksangabe wird hier nicht mehr unterstuetzt, das macht DEP 3.x
; Ablage und Abarbeitung auf Adresse 100h
; Dateigroesse max. 1 Block = 128 Byte !
;
UUU:	DB	'INITIAL.UUU',0	; Standard-Dateiname
	DW	7F7FH
	DB	'INIT'
	DB	1FH		; Textargument zulaessig (Dateiname)
	LD	A,(DE)		; Parameter in Kommandozeile
	AND	0DFH		; angegeben?
	JR	NZ,INIT1	; ja, diesen verwenden
	LD	DE,UUU		; ansonsten Standard-Dateiname INITIAL.UUU
INIT1:	CALL	TDUP1		; Dateiname in HL
	RET	C		; TAPE oder BRK
	CALL	PV1		; HL = Dateiname
	DB	0AH		; ISRI (ersten Block einlesen)
	JR	C,FCLOSE	; Fehler aufgetreten
	LD	HL,CASS		; Daten aus Kassettenpuffer
	LD	DE,100H		; hier aufbereiten zur Abarbeitung
INIT2:	LD	A,(HL)		; Zeichen
	AND	A
	JR	Z,INIT3		; 00 ignorieren
	CP	3
	JR	Z,INIT4		; CAOS-Ende
	CP	0AH
	JR	Z,INIT3		; LF ignorieren
	CP	1AH
	JR	Z,INIT4		; CP/M-Ende
	LD	(DE),A		; Zeichen ablegen
	INC	E		; Ziel +1
	LD	A,E
	CP	7FH
	JR	Z,INIT4		; Pufferende erreicht
INIT3:	INC	L		; Quelle +1
	LD	A,L
	CP	80H		; Pufferende?
	JR	NZ,INIT2	; weitere Zeichen bearbeiten
INIT4:	XOR	A
	LD	(DE),A		; Ende-Kennung anhaengen
	LD	HL,100h		; Ablage hier (wie DEP 3.2)
	LD	(FTAST),HL	; Aktivierung als ob es
	SET	6,(IX+8)	; eine Funktionstaste waere
	JP	FCLOSE		; Datei schliessen, CAOS ROM-C wie vorher
;
; UP1 fuer TYPE, DUMP, INIT, ERA: Vorbereitung fuer Datei oeffnen
; PA:	CY=1	Fehler (DEVICE=0 oder BRK)
;
TDUP1:	LD	A,(IX+8)
	AND	00011100b	; DEVICE-Nummer
	SCF			; Fehlerkennung
	RET	Z		; bei TAPE macht das keinen Sinn...
	EX	DE,HL
	LD	A,(HL)		; erstes Zeichen nach Menuewort holen
	AND	0DFH
	CALL	Z,NAME		; Dateiname abfragen falls nicht angegeben
	RET	C		; mit BRK abgebrochen
	CALL	KPUFF		; Standardpuffer setzen
	LD	A,(MODST+5)	; Schaltzustand Moduladresse 5 (ROM C)
	LD	(ARGC),A	; hier merken (zeigt sonst USER-ROM-Zustand an)
	LD	A,1		; und voruebergehend fest einschalten
	LD	(MODST+5),A
	RET
;
; UP2 fuer TYPE+DUMP: Eingabe bei Seitenende
; PA:	Z=1	BRK
;
TDUP2:	CALL	INTB
	CP	3		; BRK?
	RET	Z		; ja, Datei schliessen und Ende
	CP	0FH		; Hardcopy?
	RET	NZ
	CALL	OCHR		; Hardcopy zulassen
	JR	TDUP2

; Geraetecode in Bits 2-4 (IX+8) eintragen und Treiberadressen in SUTAB kopieren
; PE:	A	Geraetecode 0...7
;		0	Kassette - Standard
;		1	Diskette - D004/D008
;		2	USB (M052)
;		3	Netzwerk (M052)
;		4-7	Anwender-Treiber
;		8-FE	aktuellen Treiber anzeigen
;		FF	Auflisten aller Treiber
; PA:		CY=1	ausgewaehlter Treiber nicht aktiv
; VR:	AF,BC,DE,HL

SET_DD:	CP	8		; Treiber aendern oder nur Name abfragen?
	JR	NC,DLIST
	ld	c,a		; Nr. zum Vergleich merken
	ADD	A,A
	ADD	A,A		; nach Bits 2..4
	XOR	(IX+8)
	AND	00011100b	; Bits 0..1 und 5..7 zuruecksetzen
	XOR	(IX+8)		; a = (a XOR b) XOR b fuer Bits 2..4
	ld	b,a		; vorbereiteter neuer Wert fuer IX+8
	and	00011100b	; nur die Bits 2..4 ausfiltern
	add	a,a
	add	a,a		; *32
	add	a,a
	ld	h,High(devtab)	; = A900H
	ld	l,a
	ld	a,(hl)
	cp	c		; aktiver Treiber?
	SCF			; Fehlercode
	ret	nz		; nein, dann nicht umstellen!
	LD	(IX+8),B	; neuen Treiber jetzt eintragen
	set	3,l		; hier beginnen die Treiberadressen
	ld	de,(sutab)	; SUTAB-Adresse zum Modifizieren
	inc	de
	inc	de
	ldi			; 01h = MBO
	ldi
	ex	de,hl
	ld	bc,6		; 2*(MBI-MBO-1)=6
	add	hl,bc
	ex	de,hl
	ldi			; 05h = MBI
	ldi
	ex	de,hl
;	ld	bc,4		; 2*(ISRO-MBI-1)=4	(BC=4 ist 4 nach 2x LDI)
	add	hl,bc
	ex	de,hl
	ld	c,8
	ldir			; 08h = ISRO bis 0Bh = CSRI
	ex	de,hl
	ld	c,56h		; 2*(MBIN-CSRI-1)=56h
	add	hl,bc		; CY=0
	ex	de,hl
	ld	c,4
	ldir			; 37h = MBIN und 38h = MBOUT
	RET

; aktuellen Treiber anzeigen (4 Zeichen):

DLIST:	INC	A		; A=FFh?
	jr	z,dl0		; ja, dann alle Treiber auflisten
dl2:	LD	A,(IX+8)
	AND	00011100b	; aktuelles Geraet ausfiltern
	add	a,a
	add	a,a
	add	a,a
	LD	HL,DEVTAB+4	; Zeiger auf String
	or	l
	ld	l,a
dl5:	ld	b,4		; 4 Zeichen
dl3:	ld	a,(hl)
	inc	hl
	cp	20h		; keine Steuerzeichen!
	call	nc,ochr		; Name des Treibers anzeigen
	djnz	dl3
	ret

; alle Treiber auflisten:

dl0:	ld	hl,devtab	; Treiber-Tabelle
	ld	c,0		; Beginne mit Treiber Nr. 0
dl1:	ld	a,(hl)		; Treibernummer
	cp	c		; Treiber aktiviert?
	jr	nz,dl6
	inc	hl
	inc	hl
	inc	hl
	inc	hl		; Zeiger auf String
	CALL	AHEX0		; Geraetenummer einstellig anzeigen
	LD	A,'='
	CALL	OCHR
	CALL	DL5		; Treiber-Name anzeigen
	call	CRLF
dl6:	ld	a,l
	or	31
	inc	a
	ld	l,a		; weiter zum naechsten Treiber
	inc	c		; Nr. erhoehen
	ld	a,c
	cp	8
	jr	nz,dl1		; Bis alle 8 Treiber erreicht sind
	ret

; Laufwerkwechsel per ESC-D:

ESCDD:	CALL	CRLF		; neue Zeile
	CALL	DL0		; Treiber auflisten
DL7:	CALL	OSTR
	DB	2,'?',0		; Eingabeaufforderung anzeigen
	CALL	KBD		; Tastatureingabe
	CP	'0'
	JR	C,DL7		; Eingabewert zu klein!
	CP	'8'
	JR	NC,DL7		; zu gross
	push	af
	AND	0FH
	CALL	SET_DD		; Treiber einstellen
	jr	nc,dl8		; kein Fehler
	pop	af
	jr	dl7		; wiederholen
	;
dl8:	pop	af
	call	crt		; anzeigen
	jp	crlf		; neue Zeile
;
; Floppy-Treiber Funktion 1: Laufwerkwechsel (DEP 2.0)
; PE:	DE	Zeichenkette neues Laufwerk
;	(DE)=0	Anzeige aktuelles Laufwerk
; PA:	CY=1	Fehler
;
DRIVE:	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	NC,DRIVE1	; OK, Laufwerkwechsel moeglich
	CALL	OSTR
	DB	'Kein DEP2!',7,CR,LF,0
	SCF
	RET
	;
DRIVE1:	EX	DE,HL		; LW-Zeichenkette jetzt in HL
	LD	A,(HL)
	AND	0DFH
	JR	NZ,DRIVE3	; mit Argument aufgerufen!
	CALL	OSTR
	DB	'Drive:',0
	LD	A,1		; aktuell lesen
	CALL	STEUB2
	LD	BC,LWANF	; 081F1H
	IN	A,(C)
	CALL	OCHR		; Laufwerk anzeigen
	INC	B		; 082F1H
	IN	A,(C)		; aktueller User-Bereich
	AND	0FH
	CALL	NZ,AHEX0	; Userbereich hex anzeigen
	LD	A,6
	LD	(CURSO),A
	CALL	INLIN		; Eingabe
	CCF
	RET	NC		; BRK
	LD	HL,6
	ADD	HL,DE
DRIVE3:	LD	BC,LWANF	; 081F1H
	LD	A,(HL)
	AND	A		; leer?
	RET	Z
	CALL	UPCASE
	OUT	(C),A		; LW-Code
	INC	HL
	LD	A,(HL)
	AND	A		; USER mit angegeben?
	LD	A,3		; nur LW anfordern
	JR	Z,DRIVE5
	EX	DE,HL
	CALL	RHEX
	LD	A,0		; USER 0, weil
	JR	C,DRIVE4	; keine HEX-Zahl
	LD	A,(NUMVX)
DRIVE4:	INC	B		; 082F1H
	OUT	(C),A		; USER-Nr.
	LD	A,7		; LW+User anfordern
DRIVE5:	CALL	STEUB2		; Stack-Unwinding:
	RET			; nicht in JP umwandeln!
;
; CAOS-Kommandos mit DEVICE-Treiberfunktionen
;
	DW	7F7FH
	DB	'REN'
	DB	1FH		; Textargument zulaessig (2 Dateinamen)
	LD	A,3		; Funktion 3
	JR	FKT_DD

	DW	7F7FH
	DB	'ERA'
	DB	1FH		; Textargument zulaessig (Dateiname)
	LD	A,2		; Funktion 2
	JR	FKT_DD

	DW	7F7FH
	DB	'CD'		; ersetzt bisheriges Menuewort DRIVE
	DB	1FH		; Textargument zulaessig (Verzeichnis)
	LD	A,1		; Funktion 1
	JR	FKT_DD

	DW	7F7FH
	DB	'DIR'
	DB	1FH		; Textargument zulaessig (Maske in DE)
	XOR	A		; Funktion 0

; Geraetetreiber-Funktion aufrufen
; PE:	A	Funktionsnummer
;		0 = DIR bzw. TAPELIST
;		1 = DRIVE oder CD
;		2 = ERA (Datei loeschen)
;		3 = REN (Datei umbenennen)
;		?? (SETWR, SETRO)
;	DE	Dateiname bzw. Maske
; PA:		entsprechend Treiber
; VR:	AF,BC,DE,HL

FKT_DD:	and	3		; nur Treiberfunktionen 0..3 definiert
	ld	c,a
	LD	A,(IX+8)
	AND	00011100b	; die Bits 2..4 (DEVICE-Nr.) ausfiltern
	add	a,a
	add	a,a		; *16
	add	a,c		; FKT-Nummer dazu
	add	a,a		; *32
	ld	c,a
	ld	b,0
	ld	HL,DEVTAB+24	; DIR-Kommando des jeweiligen Treibers
	ADD	HL,BC
	LD	A,(HL)
	INC	HL
	LD	H,(HL)		; Aus Treibertabelle ermitteln
	LD	L,A
	JP	(HL)		; und anspringen
;
; TAPELIST = DIR-Kommando fuer Kassette
;
TLIST:	CALL	OSTR
	DB	'Tape-Directory:',CR,LF,0
	IN	A,(PIOAD)
	OR	01100000b	; Motor+LED ein
	OUT	(PIOAD),A
	LD	A,10
	CALL	WAIT		; 60ms warten (Bandanlauf)
	CALL	KPUFF		; Standard-Puffer einstellen
TLW:	LD	(IX+3),1	; Block Nr. 1 erwarten
	CALL	PV1
	DB	05H		; MBI
	JR	C,TLBRK		; Fehler aufgetreten
	LD	A,(IX+2)
	DEC	A
	JR	Z,TL01		; Block Nr. 1 gefunden
	CALL	BNROST
	DB	'*'		; falsche Blocknummer
	DB	19H,0
	JR	TLBRK
	;
TL01:	CALL	LUP3		; Name aus Vorblock anzeigen
	LD	A,(CASS+10H)	; Anzahl der Argumente
	SUB	2
	CP	8		; 2 bis 10 Argumente?
	JR	NC,TLNO		; keine MC-Datei!
	LD	HL,(CASS+11H)	; AADR
	LD	DE,(CASS+13H)	; EADR+1
	CALL	HLDE		; Anfangsadresse und Endadresse anzeigen
	LD	HL,(CASS+15H)	; SADR
	LD	A,(CASS+10H)
	CP	3
	CALL	NC,HLHX		; Startadresse anzeigen
TLNO:	CALL	CRLF		; Zeilenvorschub nach Dateiname + Adressen
TLBRK:	CALL	BRKT
	JR	NC,TLW		; wiederholen bis BRK gedrueckt wird
TLX:	IN	A,(PIOAD)
	AND	10011111b	; Motor+LED aus
	OUT	(PIOAD),A
	RET
;
; Unterprogrammtabelle (wird in IRM kopiert):
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
	DW	SETDEV	; 49 - CAOS 4.6: Speichergeraet einstellen
	DW	DEVFKT	; 4A - CAOS 4.6: Geraetefunktion aufrufen
SUEND:

; CAOS 4.6 - Sprungtabelle Treiber Nr. 0 - Kassette

DRV0:	DB	-1		; Kennung (Vorgabe = nicht aktiv)
	DB	0		; Modulsteckplatz (D001)
	DB	3,3		; SWITCH-Befehl ein/aus
	DB	'TAPE'		; Name des Treibers
	DW	MBO		; 01 Ausgabe Datenblock 128 Byte
	DW	MBI		; 05 Einlesen Datenblock 128 Byte
	DW	ISRO		; 08 Ausgabedatei oeffnen und ersten Block 01h ausgeben
	DW	CSRO		; 09 Ausgabedatei schliessen und Endeblock FFh ausgeben
	DW	ISRI		; 0A Eingabedatei oeffnen und Block 01 einlesen
	DW	CSRI		; 0B Eingabedatei schliessen
	DW	MBIN		; 37 Einlesen 1 Byte (BASIC-Schnittstelle)
	DW	MBOUT		; 38 Ausgabe 1 Byte (BASIC-Schnittstelle)
	DW	TLIST		; DIR = TAPELIST
	DW	MOT		; CD  = Kassette ein/aus (statt Laufwerkwechsel)
	DW	ERR1		; ERA - nicht vorgesehen
	DW	ERR1		; REN - nicht vorgesehen

; CAOS 4.6 - Sprungtabelle Treiber Nr. 1 Diskette D004/D008

DRV1:	DB	1		; Kennung (Treiber Nr. 1)
	DB	0FCH		; Modulsteckplatz (D004)
	DB	0FFH,0FFH	; SWITCH-Befehl ein/aus
	DB	'DISK'		; Name des Treibers
	DW	FMBO		; 01 Ausgabe Datenblock 128 Byte
	DW	FMBI		; 05 Einlesen Datenblock 128 Byte
	DW	FISRO		; 08 Ausgabedatei oeffnen und ersten Block 01h ausgeben
	DW	FCSRO		; 09 Ausgabedatei schliessen und Endeblock FFh ausgeben
	DW	FISRI		; 0A Eingabedatei oeffnen und Block 01 einlesen
	DW	FCSRI		; 0B Eingabedatei schliessen
	DW	FMBIN		; 37 Einlesen 1 Byte (BASIC-Schnittstelle)
	DW	FMBOUT		; 38 Ausgabe 1 Byte (BASIC-Schnittstelle)
	DW	FDIR		; DIR = Verzeichnisanzeige mit Maske
	DW	DRIVE		; CD  = Laufwerkwechsel
	DW	FERA		; ERA = Datei loeschen
	DW	FREN		; REN = Datei umbenennen
;
; Portinitialisierungstabellen:
;
IOTAB1:	DB	7		; 7 Ports fuer Erstinitialisierung

	 DB	PIOAS,1		; PIO-A Steuerwort
	  DB	0E4h		; Interruptvektor

	 DB	PIOAD,2		; PIO-A Daten
	  DB	00011111b	; D005-Reset
	  DB	00001111b	; IRM, RAM0, CAOS-E

	 DB	PIOAS,2		; PIO-A Steuerwort
	  DB	00001111b	; Byte-Ausgabemodus
	  DB	00000011b	; DI

	 DB	PIOBS,3		; PIO-B Steuerwort
	  DB	0E6h		; Interruptvektor
	  DB	00001111b	; Byte-Ausgabemodus
	  DB	10000011b	; EI

	 DB	PIOBD,1		; PIO-B Daten
	  DB	0FFh		; Daten

	 DB	84h,1		; Ausgabeport 84h
	  DB	00101000b	; RAM8 0, LoRes

	 DB	86h,1		; Ausgabeport 86h
	  DB	11100011b	; RAM4 ein, USER-C 3, CAOS-C ein

IOTAB2:	DB	4		; 4 Ports initialisieren

	 DB	PIOAS,1		; PIO-A Steuerwort
	  DB	0E4h		; Interruptvektor

	 DB	PIOBS,1		; PIO-B Steuerwort
	  DB	0E6h		; Interruptvektor

	 DB	CTC0,1		; CTC Kanal 0
	  DB	0E8h		; Interruptvektor

	 DB	CTC2,2		; CTC Kanal 2: blinken
	  DB	01000111b	; DI, Zaehler, RESET
	  DB	12		; Zeitkonstante

; Initialisierungstabelle fuer M001 als Centronics-Druckerschnittstelle:
;
DIOINI:	DB	3
	 DB	06h,2		; PIO-A Steuerwort
	  DB	11001111b	; Bitbetrieb
	  DB	00000000b	; alles Ausgaenge
	 DB	07h,2		; PIO-B Steuerwort
	  DB	11001111b	; Bitbetrieb
	  DB	11111110b	; Bit 0 Ausgang
	 DB	05h,1		; PIO-B Daten
	  DB	00000001b	; Strobe passiv

; Voreingestellte Zuordnung von Joystickfunktionen zu ASCII-Tastencodes
; Tabelle wird bei Power-On in den IRM kopiert, Bereich A894..A89F
; Hinweis: Fire2 ist beim M008 die primaere oder einzige Feuertaste
;
JOYTBD:	DB	2		; Wartezyklen fuer Tastenwiederholung
	DB	0Bh		; Up
	DB	0Ah		; Down
	DB	CR		; Fire+Fire2
	DB	08h		; Left
	DB	0		; Up+Left
	DB	0		; Down+Left
	DB	CR		; Fire
	DB	09h		; Right
	DB	0		; Up+Right
	DB	0		; Down+Right
	DB	' '		; Fire2
;
; Interrupttabelle (wird in den RAM kopiert):
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
; IBM-Kodierungen fuer allgemeine Druckroutine PRINTC
;
ZIBM:	DB	7BH,7CH,7DH,7EH
	DB	5BH,5CH,5DH
	DB	84H,94H,81H,0E1H
	DB	8EH,99H,9AH

	ABSFILL	0DAB8h,<ROM-D-Ende>
