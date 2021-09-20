;*****************************************
;**	CAOS 4.7	ROM D		**
;**					**
;**	Adresse:  D000h bis DAB7h	**
;**					**
;**	letzte Aenderung: 03.03.2019	**
;*****************************************

;	ORG	0D000H
if turbo
;*******************************************************************************
;	DEVICE-Treiber fuer "MPM-Turbo" Kassette (ab CAOS 4.7 im ROM-D)
;*******************************************************************************

; Zeitkonstanten "Turbo"
			; Vorton	"0"-Bits	
ZK1	EQU	24	; "0"-Bit	Halbschwingung ca. 2200 Hz
ZK3	EQU	40	; Unterscheidung 0/1 beim Lesen
ZK2	EQU	60	; "1"-Bit	Halbschwingung ca.  900 Hz

; Initialisierung Bandausgabe:				**08**
; PA:	CY=0	(kein Fehler bei Bandausgabe)

XISRO:	DI
	IN	A,(PIOAD)
	OR	01100000b	; Motor+LED ein
	CALL	ISRO1		; Initialisierung
	LD	(IX+2),0	; Block 1 (Vorblock)
	LD	BC,4096		; langer Vorton

; Ausgabe 1 Block					**01**
; PE:	BC	Laenge Vorton

XMBO:	INC	(IX+2)
	LD	L,(IX+5)
	LD	H,(IX+6)	; Blockanfang
	LD	A,87H		; EI,ZG16,Res
	OUT	(CTC1),A
	LD	A,ZK1-3		; "0"-Bit als Vorton
	OUT	(CTC1),A	; CTC starten
	EI
SR1:	LD	E,ZK1-3		; "0"-Bit als Vorton
SR2:	BIT	7,E
	JR	Z,SR2		; Abwarten bis Bit 7 in ISR gesetzt wird
	DEC	BC
	LD	A,B
	OR	C
	JR	NZ,SR1		; bis Vorton abgearbeitet ist
	LD	A,0C0H
	CALL	BYTS		; zwei "1"-Bits als Startbit
	LD	A,(IX+2)
	LD	B,128		; Anzahl Datenbytes
	CALL	BYTS		; Blocknummer ausgeben
SR3:	LD	A,(HL)		; Datenbyte
	INC	HL
	CALL	BYTS		; Datenbyte schreiben
	XOR	C		; Pruefbyte berechnen
	LD	C,A		; in Register C
	DJNZ	SR3
	LD	D,10		; 10 Bit fuer Pruefbyte + Stoppbit
	CALL	BYS1
	AND	A		; CY=0 (kein Fehler)
	LD	A,3
	OUT	(CTC1),A	; CTC stoppen
	RET
;
; Ausgabe eines Bytes "Turbo"
; PE:	A	Datenbyte
;
BYTS:	LD	D,8		; 8 Bit abarbeiten
BYS1:	RRCA
	LD	E,ZK1-3		; ZK "0"-Bit
	JR	NC,BYS2
	LD	E,ZK2-3		; ZK "1"-Bit
BYS2:	BIT	7,E
	JR	Z,BYS2		; Quittung von ISR abwarten
	DEC	D
	JR	NZ,BYS1		; noch 1 Bit!
	RET

; Initialisierung Bandeingabe				**0A**
; PE:	HL	Dateiname
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC

XISRI:	CALL	ISRI1		; Motor ein, LED aus

; Einen Block einlesen:					**05**
; PA:	CY=1	Pruefbyte fehlerhaft
; VR:	AF,BC

XMBI:	PUSH	HL
	PUSH	DE
	LD	L,(IX+5)	; Blockpuffer
	LD	H,(IX+6)
	LD	A,83H		; EI
	OUT	(PIOAS),A	; PIO-A Interrupt freigeben
	LD	BC,8000H	; B=128 Byte, C=0 fuer Pruefbyte
	LD	E,-1
SRL1:	IN	A,(PIOAD)
	and	11011111b	; LED aus
	OUT	(PIOAD),A	; PIO-Logik freigeben
	LD	A,E		; aktuell gelesene Bits
	CP	0C0H		; Startbit erfasst?
	JR	NZ,SRL1		; warten
	CALL	BYTR		; einlesen
	LD	(IX+2),A	; Blocknummer
SRL2:	CALL	BYTR		; ein Byte einlesen
	LD	(HL),A		; in Speicher ablegen
	INC	HL
	XOR	C		; Pruefbyte berechnen
	LD	C,A		; in Register C
	DJNZ	SRL2
	CALL	BYTR		; Pruefbyte lesen
	SUB	C		; und mit Berechnung vergleichen
	ADD	A,0FFH		; CY=1, wenn Fehler!

	di
	LD	A,00010111b	; Interruptsteuerwort: DI, RESET
	OUT	(PIOAS),A	; PIO-A Interrupt sperren
	OUT	(PIOAS),A	; Maske (Wert nicht relevant, da Byteausgabe)
	LD	A,3		; DI
	OUT	(CTC1),A	; CTC stoppen
	ei

	POP	DE
	POP	HL
	RET

; Lesen eines Bytes
; PA:	E	Datenbyte
;	D=7	Anzahl fuer naechsten Aufruf
; VR:	DE, AF

BYTR:	LD	D,8-1		; 8 Byte einlesen
BYR1:	IN	A,(PIOAD)
	or	00100000b	; LED ein	
	OUT	(PIOAD),A	; PIO-Logik freigeben
	BIT	7,D		; abwarten bis PIO-ISR D auf -1 zurueck
	JR	Z,BYR1		; gezaehlt hat, dann sind 8 Bit eingelesen
	LD	A,E		; Datenbyte
	RET

; Abschluss Bandausgabe					**09**
; PA:	CY=0

XCSRO:	LD	(IX+2),0FEH	; FFH = Kennung Ende-Block
	CALL	XMBO		; Block Turbo ausgeben

; Abschluss Bandeingabe					**0B**
; PA:	CY=0

XCSRI:	JP	TCSRI

endif

;*******************************************************************************
;	DEVICE-Treiber fuer Kassette (ab CAOS 4.7 im ROM-D)
;*******************************************************************************

; Zeitkonstanten fuer TAPE-Schreiben:

			; Vorton	"1"-Bits
ZKKAT	EQU	93	; Trennzeichen	Vollschwingung ca.  557 Hz
ZKKA0	EQU	23	; "0"		Vollschwingung ca. 1950 Hz
ZKKA1	EQU	47	; "1"		Vollschwingung ca. 1050 Hz
;
; Zeitkonstanten fuer TAPE-Lesen:
;
IKEZK	EQU	163	; Zeitkonstante fuer CTC
IKEGN	EQU	93	; Erkennung Nullbit
IKEG1	EQU	186	; Erkennung Einsbit

; Initialisierung Bandausgabe:				**08**
; PA:	CY=0	(kein Fehler bei Bandausgabe)

TISRO:	DI
	IN	A,(PIOAD)
	OR	01100000b	; Motor+LED ein
	CALL	ISRO1		; Initialisierung
	LD	(IX+2),0	; Block 1 (Vorblock)
	LD	BC,4096		; langer Vorton

; Ausgabe 1 Block					**01**
; PE:	BC	Laenge Vorton

TMBO:	INC	(IX+2)
	LD	A,10000111b	; EI,ZG16,Res
	DI
	OUT	(CTC1),A
	LD	A,ZKKA1		; Vorton "1"
	OUT	(CTC1),A
	EI
	LD	E,A		; E=47 (ZK Vorton)
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

; Bei einer Halbschwingung als Endimpuls aendert sich mit jedem Block die
; Polaritaet der Signale. Das ist bei allen CAOS-Versionen so ausser CAOS 4.5.
; Eventuell ist das ja besser - deshalb ab CAOS 4.6 wieder eine Halbschwingung
; als Endimpuls. (bemerkt von Micha im Forum am 02.02.2015)

	CALL	HBITOT		; Trennzeichen (2. Halbschwingung)
	LD	E,L
	LD	D,H		; DE=HL (Pufferende+1)
	LD	A,3		; CTC stoppen
	OUT	(CTC1),A
	RET

; Ausgabe eines Bytes auf Kassette
; PE:	A	Datenbyte
; PA:	CY=0
;	E	Zeitkonstante fuer Trennzeichen
; VR:	AF, E

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

; Ausgabe einer Vollschwingung
; PE:	E	Zeitkonstante

BITOUT:	CALL	HBITOT		; Aufrufen und reinlaufen
HBITOT:	LD	(IX),E		; Zeitkonstante -> (IX)
HBIT1:	LD	A,(IX)		; Warten bis Zeitkonstante von
	AND	A		; Interruptroutine zurueckgesetzt
	JR	NZ,HBIT1
	RET

; Initialisierung Bandeingabe				**0A**
; PE:	HL	Dateiname
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC

TISRI:	CALL	ISRI1		; Motor ein, LED aus

; Einen Block einlesen:					**05**
; PA:	CY=1	Fehler
;	CY=0	Block korrekt gelesen
; VR:	AF,BC

TMBI:	LD	A,10000011b
	OUT	(PIOAS),A	; EI an PIO A (Kassette)
	PUSH	HL
	PUSH	DE
MBI1:	LD	B,22
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
	DJNZ	MBI4		; Warten auf ein komplettes Nullbit
	LD	L,(IX+5)	; Blockpuffer
	LD	H,(IX+6)
	CALL	BYTIN		; Blocknummer lesen
	JR	C,MBI5		; Stoerung
	LD	(IX+2),A	; Blocknummer eintragen
	LD	B,128		; Blocklaenge
	LD	E,0		; Pruefsumme=0
BLKIN:	CALL	BYTIN		; Datenbyte lesen
	JR	C,MBI5		; Stoerung
	LD	(HL),A
	ADD	A,E		; Pruefsumme berechnen
	LD	E,A
	INC	HL
	DJNZ	BLKIN
	LD	L,A		; berechnete Pruefsumme
	CALL	BYTIN		; Vergleichspruefsumme einlesen
	JR	C,MBI5		; Stoerung
	SUB	L		; Pruefsumme richtig?
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
; 1 Bit (Vollschwingung) einlesen:
; PA:	A	Zeitwert
;	CY=0	Einsbit
;	CY=1	Nullbit
; VR:	AF, C
;
STOP1:	XOR	A		; mit 0 beginnen
	CALL	STOP2		; erst aufrufen, dann reinlaufen
STOP2:	LD	C,A		; 'Zeit' der 1. Halbschwingung in C merken
	LD	(IX),0
STOP21:	IN	A,(PIOAD)
	OUT	(PIOAD),A	; PIO-Logik freigeben
;	LD	A,(IX)		; CTC-Stand von Interrupt
;	OR	A		; eingetragen?
	XOR	A
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
	JR	C,IB2		; Nullbit ?
	CP	IKEG1		; Einsbit
	RET	C		; CY=1: Trennzeichen
IB2:	CCF
	RR 	D		; Bit einschieben
	JR	NC,IB1		; bis 8 Bit abgearbeitet sind
	CALL	STOP1		; Trennzeichen (mit CY-Rueckmeldung)
	LD	A,D		; Byte in A
	RET

;*******************************************************************************
;	DEVICE-Treiber fuer Floppy D004/D008 (ab CAOS 4.6)
;*******************************************************************************

; Eingabedatei oeffnen und Block 01 einlesen		**0A**
; PE:	HL	Dateiname
; PA:	CY=1	Fehler

FISRI:	LD	(IX+2),0	; 1. Block lesen
	LD	(IX+3),1	; erwarteter Block = 1
	CALL	NAMOUT		; Dateiname ausgeben zum D004
	LD	A,9		; Lesen oeffnen
	DB	21h		; LD HL,nn

; Einlesen Datenblock 128 Byte				**05**
; PE:	(IX+3)	erwartete Blocknummer
; PA:	(IX+2)	gelesene Blocknummer
;	CY=1	Lesefehler
; VR:	AF,BC

FMBI:	LD	A,1		; Lesen normal
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

; Eingabedatei schliessen				**0B**
; PE:
; PA:	CY=1	bei Fehler

FCSRI:	LD	A,41H		; READ - close
	JR	STEUER

; Ausgabedatei oeffnen und ersten Block 01h ausgeben	**08**
; PE:	Kassettenpuffer mit Daten des Vorblocks
;	HL	Dateiname
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL

FISRO:	LD	(IX+2),0	; mit Block 1 beginnen
;	LD	L,(IX+5)	; HL = Dateiname im Kassettenpuffer
;	LD	H,(IX+6)
	CALL	NAMOUT		; Dateiname ausgeben zum D004
	LD	A,0BH		; Schreiben oeffnen
	DB	21h		; LD HL,nn

; Ausgabe Datenblock 128 Byte				**01**
; PE:	Datenblock im Kassettenpuffer
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL

FMBO:	LD	A,3		; Schreiben normal
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

; Ausgabedatei schliessen und Endeblock FFh ausgeben	**09**
; PE:	Datenblock im Kassettenpuffer
; PA:	CY=1	bei Fehler
; VR:	AF,BC,DE,HL

FCSRO:	LD	(IX+2),0FEH	; Nr. letzter Block
	CALL	FMBO		; Datenblock ausgeben
	RET	C		; Fehler
	LD	A,43H		; write close
	JR	STEUER

; Dateiname ausgeben
; PE:	HL	Dateiname, Ende mit 00 (max. 12 Zeichen)
;	D	Maske FFh (Trennzeichen=0) oder DFh (Trennzeichen=SPC oder 0)
; VR:	AF,BC,E,HL

NAMOUT:	LD	DE,0FF0Ch	; D=Maske, E=max. Laenge Dateiname 8.3 (12)
NAMO1:	LD	BC,DIRBUF	; 82F3H
NAMO2:	LD	A,(HL)
	OUT	(C),A		; Dateiname zum D004 ausgeben
	AND	D		; war das ein Trennzeichen?
	JR	Z,NAMO3
	INC	HL		; naechstes Zeichen solange kein Trennzeichen
NAMO3:	INC	B
	DEC	E
	JR	NZ,NAMO2
	RET
;
; Name ein-, ausgeben: (fuer RENAME)
; PE:	DE	Zeiger auf Dateinamen, wenn 0 dann Eingabe
; PA:	CY=1	BRK
;	DE	Zeiger auf Zeichen nach Dateinamen (2. Name bei REN)
;
FNAM1:	CALL	NAMHL		; Dateiname eingeben
	RET	C		; BRK
FNAM2:	LD	DE,0DF0Ch	; D=Maske, E=max. Laenge Dateiname 8.3 (12)
	CALL	NAMO1		; ausgeben zum D004	
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

CLLND:	LD	E,0		; Cursor auf Spalte 0
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

; Quadratwurzel 16Bit A=SQR(HL)				**40**

SQRC:	XOR	A		; A=0, CY=0
	DEC	A		; A=FF
	LD	E,A
	LD	D,A		; DE=FFFF
SQR1:	INC	A		; mitzaehlen
	INC	DE
	INC	DE
	SBC	HL,DE
	JR	NC,SQR1
	JP	COFF		; -> COFF

; Multiplikation BA=C*D					**41**

MULC:	XOR	A		; Startwert 0
	LD	B,8		; 8 Bit Multiplikation/Rotation
	RR	D
MUL1:	JR	NC,MUL2		; Bit nicht gesetzt
	ADD	A,C		; aufaddieren
MUL2:	RRA
	RR	D		; jetzt 16 Bit rotieren
	DJNZ	MUL1		; und weiter addieren
	LD	B,A		; Ergebnis in
	LD	A,D		; BA
	RET

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

; Anzeige Steckplatz, Kennbyte, Steuerbyte fuer Modulcheck

LHD:	LD	A,L
	CALL	AHSPC		; Steckplatz
	LD	A,H
	CALL	AHSPC		; Kennbyte
	LD	A,D
	JP	AHSPC		; Steuerbyte

; * * * *	ab hier DEVICE-Treiber-Funktionen (neu)	* * * *

;
; Floppy-Treiber Funktion 0: Directory mit Maske
; PE:	DE	Maske
; PA:	HL	Anzahl passender Eintraege
;
FDIR:	EX	DE,HL		; Maske aus Kommandozeile
	LD	DE,CASS
	LD	BC,11		; max. Laenge
	LDIR			; In Kassettenpuffer ablegen
	XOR	A
	LD	(DE),A		; Abschluss Maske
	LD	(NUMNX),A	; Zeilenzaehler fuer Anzeige
	ld	h,a
	ld	l,a
	ld	(NUMVX),hl	; Zaehler fuer angezeigte Dateien
	LD	A,0DH		; Directory oeffnen
DIR1:	CALL	STEUER
	BIT	2,A
	JR	NZ,DIR6		; Ende
	LD	B,HIGH(DIRBUF)	; 82F3H
	LD	HL,CASS+12
;	AND	A		; CY=0
	LD	D,3		; 3 Eintraege einlesen von DEP
DIR2:	LD	E,11		; 11 Zeichen lang
DIR3:	IN	A,(C)		; Zeichen einlesen
	PUSH	AF
	AND	7FH		; ohne Attribut
	INC	B
	LD	(HL),A		; ablegen
	INC	HL
	LD	A,E
	CP	3		; R/O-Byte?
	JR	Z,DIR4
	POP	AF		; vom Stack nehmen wenn nicht das R/O-Byte
DIR4:	DEC	E
	JR	NZ,DIR3
	LD	(HL),0		; Ende-Kennung als 12. Byte
	INC	HL
	INC	B
	POP	AF
	LD	(HL),A		; R/O-Byte als 13. Byte ablegen
	INC	HL
	DEC	D
	JR	NZ,DIR2

	LD	HL,CASS+12	; 3 abgelegte DIR-Eintraege
	LD	B,3		; 3 Eintraege testen/anzeigen
DIR5:	PUSH	HL
	LD	DE,CASS		; die Vergleichsmaske
	LD	A,(DE)
	AND	A		; CY=0
	CALL	NZ,GLOB		; Vergleich mit Maske, falls angegeben
	POP	HL
	CALL	NC,DIR7		; Anzeige eines passenden Eintrags
	LD	DE,13
	ADD	HL,DE		; naechster Eintrag
	DJNZ	DIR5		; bis alle 3 Eintraege abgearbeitet sind
	LD	A,5		; DIR weiter
	JR	DIR1
	;
DIR6:	LD	A,(CURSO)
	AND	A		; Cursor auf neuer Zeile?
	CALL	NZ,CRLF		; wenn nicht, dann jetzt ein Zeilenwechsel

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
	ld	hl,(NUMVX)	; Anzahl Dateien zurueck geben
	RET

; einen DIR-Eintrag anzeigen:

DIR7:	LD	A,(HL)		; erstes Zeichen
	CP	' '		; leer?
	RET	Z		; dann kein Dateiname!
	PUSH	HL		; DIR-Eintrag
	LD	A,(CURSO)
	CP	3*13		; wenn schon 3 Eintraege auf der Zeile stehen,
	CALL	Z,DIR9		; dann auf neue Zeile wechseln
	POP	HL
	PUSH	HL
	CALL	ZKOUT		; Dateiname anzeigen
	LD	A,' '		; W/R
	BIT	7,(HL)
	JR	Z,DIR8
	LD	A,'*'		; R/O
DIR8:	CALL	OCHR
	CALL	SPACE
	ld	hl,(NUMVX)
	inc	hl		; angezeigte Dateien mitzaehlen
	ld	(NUMVX),HL
	POP	HL
	RET

; Neue Zeile bei DIR-Kommando / warten nach Bildschirmseite:

DIR9:	LD	HL,NUMNX	; Zeilenzaehler
	INC	(HL)		; Zeilen zaehlen
	LD	A,(WINLG+1)	; Anzahl Zeilen im Fenster
	DEC	A		; eine Zeile zum ueberlappen der Anzeige lassen
	SUB	(HL)
	JR	NZ,DIR10
	LD	(HL),A		; wieder von vorn zaehlen
	call	kbd		; Tastatureingabe abwarten
DIR10:	JP	CRLF
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
	CALL	PV7		; HL = Dateiname
	DB	4		; ISRI (ersten Block einlesen)
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
TYPE3:	CALL	PV7		; naechsten Block einlesen mit
	DB	1		; MBI
	JR	C,TYPE10		; Fehler
TYPE4:	LD	DE,CASS		; Kassettenpuffer zur Entnahme der Daten
TYPE5:	LD	A,(DE)		; Zeichen
	INC	E		; Zeiger auf naechstes Zeichen
	CP	3
	JR	Z,TYPE10	; CAOS-Ende
	CP	1AH
	JR	Z,TYPE10	; CP/M-Ende
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
	CALL	PV7		; HL = Dateiname
	DB	4		; ISRI (ersten Block einlesen)
	JR	NC,DUMP4	; kein Fehler aufgetreten
FCLOSE:	CALL	PV7		; Datei schliessen
	DB	5		; CSRI
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
	CALL	PV7		; naechsten Block einlesen mit
	DB	1		; MBI
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
	CALL	PV7		; HL = Dateiname
	DB	4		; ISRI (ersten Block einlesen)
	JR	C,FCLOSE	; Fehler aufgetreten
	LD	HL,CASS		; Daten aus Kassettenpuffer
	LD	DE,INITU	; hier aufbereiten zur Abarbeitung (A880h)
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
	LD	HL,INITU	; Ablage hier (bei DEP 3.2/3.3 noch ab 100h)
	LD	(FTAST),HL	; Aktivierung als ob es
	SET	6,(IX+8)	; eine Funktionstaste waere
	JP	FCLOSE		; Datei schliessen, CAOS ROM-C wie vorher
;
; UP1 fuer TYPE, DUMP, INIT, ERA: Vorbereitung fuer Datei oeffnen
; PA:	CY=1	Fehler (DEVICE=0 oder BRK)
;
TDUP1:	CALL	DEV1		; DEVICE?
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

; DEVICE einstellen, abfragenn oder anzeigen
; PE:	A = Geraetecode 0...7
;		0	Kassette - Standard
;		1	Kassette - Turbo/FAST
;		2	Diskette - D004/D008
;		3	USB (M052)
;		3-7	Anwender-Treiber
;	A = 8		aktuellen Treiber abfragen (ohne Anzeige)
;	A = 9-FE	aktuellen Treiber anzeigen
;	A = FF		Auflisten aller Treiber
; PA:	CY=1	ausgewaehlter Treiber nicht aktiv
;   bei CY=0:	HL	Name des aktuellen Treibers
;	und	Z=1	Treiber ist Kassette
; VR:	AF,BC,DE,HL

SET_DD:	CP	8		; Treiber aendern oder nur Name abfragen?
	JR	Z,GETDRV	; Treiber nur abfragen
	JR	NC,DLIST	; Treiber anzeigen
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
if turbo
	and	a		; Treiber 0?
	jr	nz,dd1
	LD	(IX-12),LOW(ISRPA)	; ISR PIO A Kassette (Lesen)
	LD	(IX-11),HIGH(ISRPA)	; Interrupttabelle 01E4H+01E5H
	LD	(IX-6),LOW(ISRC1)	; ISR CTC1 Kassette (Schreiben)
	LD	(IX-5),HIGH(ISRC1)	; Interrupttabelle 01EAH+01EBH
dd1:	dec	a		; Treiber 1?
	ret	nz
	LD	(IX-12),LOW(INTL)	; ISR PIO A Turbo (Lesen)
	LD	(IX-11),HIGH(INTL)	; Interrupttabelle 01E4H+01E5H
	LD	(IX-6),LOW(INTS)	; ISR CTC1 Turbo (Schreiben)
	LD	(IX-5),HIGH(INTS)	; Interrupttabelle 01EAH+01EBH
else
;	AND	A			; CY = 0
endif
GETDRV:	CALL	DEV		; aktuelle DEVICE-Nr. ermitteln
	push	af		; Z-Flag erhalten
	LD	HL,DEVTAB+4	; Zeiger auf String
	or	l		; Treibernummer einbauen
	ld	l,a		; HL zeigt jetzt auf den Namen
	pop	af
	ret

; aktuellen Treiber anzeigen (4 Zeichen):

DLIST:	INC	A		; A=FFh?
	jr	z,DLI3		; ja, dann alle Treiber auflisten
	CALL	GETDRV		; DEVICE abfragen
DLI1:	ld	b,4		; 4 Zeichen
DLI2:	ld	a,(hl)
	inc	hl
	cp	20h		; keine Steuerzeichen!
	call	nc,ochr		; Name des Treibers anzeigen
	djnz	DLI2
	ret

; alle Treiber auflisten:

DLI3:	ld	hl,devtab	; Treiber-Tabelle = A900h
	ld	c,l		; Beginne mit Treiber Nr. 0
DLI4:	ld	a,(hl)		; Treibernummer
	cp	c		; Treiber aktiviert?
	jr	nz,DLI5
	inc	hl
	inc	hl
	inc	hl
	inc	hl		; Zeiger auf String
	CALL	AHEX0		; Geraetenummer einstellig anzeigen
	LD	A,'='
	CALL	OCHR
	CALL	DLI1		; Treiber-Name anzeigen
	call	CRLF
DLI5:	ld	a,l
	or	31
	inc	a
	ld	l,a		; weiter zum naechsten Treiber
	inc	c		; Nr. erhoehen
	ld	a,c
	cp	8
	jr	nz,DLI4		; Bis alle 8 Treiber erreicht sind
	ret

; Laufwerkwechsel per ESC-D:

ESCDD:	CALL	CRLF		; neue Zeile
	CALL	DLI3		; Treiber auflisten
DLI6:	CALL	OSTR
	DB	2,'?',0		; Eingabeaufforderung anzeigen
	CALL	KBD		; Tastatureingabe
	CP	'0'
	JR	C,DLI6		; Eingabewert zu klein!
	CP	'8'
	JR	NC,DLI6		; zu gross
	push	af
	AND	0FH
	CALL	SET_DD		; Treiber einstellen
	jr	nc,DLI7		; kein Fehler
	pop	af
	jr	DLI6		; wiederholen
	;
DLI7:	pop	af
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
	CALL	DEV		; DEVICE-Nr. in Bit 2-4
	add	a,c		; FKT-Nummer dazu
	add	a,c
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
	ld	hl,0
	ld	(NUMVX),hl	; Zaehler fuer angezeigte Dateien
	IN	A,(PIOAD)
	OR	01100000b	; Motor+LED ein
	OUT	(PIOAD),A
	LD	A,10
	CALL	WAIT		; 60ms warten (Bandanlauf)
	CALL	KPUFF		; Standard-Puffer einstellen
TLW:	LD	(IX+3),1	; Block Nr. 1 erwarten
	CALL	PV7
	DB	1		; MBI
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
	ld	hl,(NUMVX)
	inc	hl		; angezeigte Dateien mitzaehlen
	ld	(NUMVX),HL
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
	ld	hl,(NUMVX)	; Anzahl Dateien zurueck geben
	RET
;
; Kassette ein/aus (CD im Tape-Mode)
;
MOT:	IN	A,(PIOAD)
	XOR	01000000b	; Motor ein/aus
	OUT	(PIOAD),A
	RET
;
; BASIC-Dateiname ab NAME im Format 8.3 kopieren
;
; PE:	HL	Zeiger auf Dateiname im BASIC-Format = SSSName
;
; PA:	FNAME	Dateiname 8.3-Format = Name.SSS
;
; VR:	BC, AF
;
BNAME:	PUSH	HL
	PUSH	DE
	push	hl		; Beginn Dateityp merken
	inc	hl
	inc	hl
	inc	hl		; mit Name beginnen
	ld	de,FNAME	; Ablage Dateiname im IRM
	ld	bc,8
	ldir			; Name direkt kopieren
	pop	hl
	ld	b,3
bnam2:	ld	a,(hl)
	inc	hl
	and	7fh		; bei Dateityp Bit 7 ruecksetzen
	ld	(de),a
	inc	de
	djnz	bnam2
	POP	DE
	POP	HL
	RET

; Ausgabe einer Displayzeile
; HL=Adresse, C=Anzahl Bytes

DPMEMO:	CALL	HLHX		; Adresse
DPME0:	PUSH	HL		; Hier Einsprung von DUMP-Routine
	LD	B,C
DPME1:	CALL	LDAME		; Byte holen
	INC	HL
	CALL	AHSPC		; anzeigen
	DJNZ	DPME1		; C*
	POP	HL
	LD	B,C
	LD	A,9
	CALL	OCHR		; CUR
DPME2:	CALL	LDAME		; Byte holen
	INC	HL
	CALL	CSTBT		; als ASCII
	DJNZ	DPME2		; C*
	RET

; Grafiksteuersequenzen fuer die Drucker

C6313:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',5
	DW	320

C6314:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',5
	DW	640

C6311:	DB	14		; Laenge
	DB	ESC,'[01e'
	DB	ESC,'[12`'
	DB	ESC,'K'
	DW	320

C6312:	DB	14		; Laenge
	DB	ESC,'[01e'
	DB	ESC,'[12`'
	DB	ESC,'K'
	DW	640

C6304:	DB	9		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'K'
	DW	320

CMINI:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',39
	DW	320

C24N1:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',0
	DW	320

C24N2:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',39
	DW	960

; V.24-Tabellen, zum Kopieren in den IRM ab A800H:

V24TAB:	DB	0		; Steckplatz M003 (Aktualisierung durch Suche)

; Kanal 1 - Druckerausgabe (9600 Baud)
	DB	01000111b	; CTC: DI, Zaehler, neg. Flanke, RESET
	DB	91		; Zeitkonstante
	DB	4,00000100b	; WR4: Vorteiler=1, 1 Stop-Bit, ohne Paritaet
	DB	3,00100000b	; WR3: Empfaenger aus, CTS+DCD-Steuerung
	DB	5,01101010b	; WR5: Sender ein, 8Bit, /DTR=1, RTS=1

; Kanal 2 - Duplex (1200 Baud)
	DB	01000111b	; CTC: DI, Zaehler, neg. Flanke, RESET
	DB	46		; Zeitkonstante
	DB	00011000b	; Port-RESET
	DB	4,01000100b	; WR4: Vorteiler=16, 1 Stop-Bit, ohne Paritaet
	DB	3,11100001b	; WR3: Empfaenger ein, 8Bit, CTS+DCD-Steuerung
	DB	5,01101010b	; WR5: Sender ein, 8Bit, /DTR=1, RTS=1

; Initialisierung SIO-B fuer Empfangs-Interrupt (1200 Baud):

V24UMT:	DB	2		; 2 Ports
	 DB	0Dh,2		; CTC1
	  DB	01000111b	; DI, Zaehler, neg. Flanke, ZK folgt, RESET
	  DB	46		; Zeitkonstante
V24INT:	 DB	0Bh,11		; SIO-B Steuerwort
	  DB	00011000b	; Port-RESET
	  DB	2,0E2h		; WR2: INT-Vektor
	  DB	4,01000100b	; WR4: Vorteiler=16, 1 Stop-Bit, ohne Paritaet
	  DB	3,11100001b	; WR3: Empfaenger ein, 8Bit, CTS/DCD-Steuerung
	  DB	5,11101010b	; WR5: Sender ein, 8Bit, /DTR=0 (bereit), RTS=1
	  DB	11h,00011000b	; WR1: Interrupt bei Zeichenempfang

; Initialisierung SIO-B fuer Polling-Betrieb:

V24POL:	DB	0Bh,7		; SIO-B Steuerwort, 7 Byte
	 DB	00011000b	; Port RESET
	 DB	4,01000100b	; Vorteiler 16, 1 Stop-Bit, ohne Paritaet
	 DB	3,11100001b	; Empfaenger ein, 8 Bit, CTS/DCD-Steuerung
	 DB	5,01101010b	; Sender ein, 8 Bit, DTR aus, RTS ein
;
; Unterprogrammtabelle (wird seit CAOS 4.6 in IRM kopiert):
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
	; bis hier in HC900-CAOS, CAOS 2.2 und 2.3 enthalten
	DW	MBIN	; 37 - Byteweise Eingabe fuer BASIC
	DW	MBOUT	; 38 - Byteweise Ausgabe fuer BASIC
	DW	KEY	; 39
	DW	KEYLI	; 3A
	DW	DISP	; 3B
	DW	WININ	; 3C
	DW	WINAK	; 3D
	; bis hier in CAOS 2.4 und 2.5 enthalten
	DW	LINE	; 3E
	DW	CIRCL	; 3F

	DW	SQR	; 40
	DW	MULT	; 41
	DW	CSTBT	; 42
	; bis hier in HC901-CAOS enthalten
	DW	INIEA	; 43
	DW	INIME	; 44
	DW	ZKOUT	; 45
	DW	MENU	; 46
	; bis hier in CAOS 3.1, 3.3, 3.4 und OS/PI'88/'90 enthalten
	DW	V24OUT	; 47 - Initialisierung Druckerausgabe
	DW	V24DUP	; 48 - Initialisierung V.24-Duplexbetrieb
	; bis hier in CAOS 4.1 bis 4.5 enthalten
	DW	SETDEV	; 49 - Speichergeraet einstellen/anzeigen
SUEND:	; bis hier in CAOS 4.6 und 4.7 enthalten

; CAOS 4.6 - Sprungtabelle Treiber Nr. 0 - Kassette

DRV0:	DB	-1		; Kennung (Vorgabe = nicht aktiv)
	DB	5		; Treiber befindet sich in CAOS ROM-C
	DB	0,1		; SWITCH-Befehl aus/ein
	DB	'TAPE'		; Name des Treibers
	DW	TMBO		; 01 Ausgabe Datenblock 128 Byte
	DW	TMBI		; 05 Einlesen Datenblock 128 Byte
	DW	TISRO		; 08 Ausgabedatei oeffnen und ersten Block 01h ausgeben
	DW	TCSRO		; 09 Ausgabedatei schliessen und Endeblock FFh ausgeben
	DW	TISRI		; 0A Eingabedatei oeffnen und Block 01 einlesen
	DW	TCSRI		; 0B Eingabedatei schliessen
	DW	NOOP		;
	DW	NOOP		;
	DW	TLIST		; DIR = TAPELIST
	DW	MOT		; CD  = Kassette ein/aus (statt Laufwerkwechsel)
	DW	ERR1		; ERA - nicht vorgesehen
	DW	ERR1		; REN - nicht vorgesehen

if turbo

; CAOS 4.7 - Treiber Nr. 1 - Turbo-Save/Load

	DB	1		; Treiber aktiv
	DB	5		; Treiber befindet sich in CAOS ROM-C
	DB	0,1		; SWITCH-Befehl aus/ein
	DB	'FAST'		; Name des Treibers
	DW	XMBO		; 01 Ausgabe Datenblock 128 Byte
	DW	XMBI		; 05 Einlesen Datenblock 128 Byte
	DW	XISRO		; 08 Ausgabedatei oeffnen und ersten Block 01h ausgeben
	DW	XCSRO		; 09 Ausgabedatei schliessen und Endeblock FFh ausgeben
	DW	XISRI		; 0A Eingabedatei oeffnen und Block 01 einlesen
	DW	XCSRI		; 0B Eingabedatei schliessen
	DW	NOOP		;
	DW	NOOP		;
	DW	TLIST		; DIR = TAPELIST
	DW	MOT		; CD  = Kassette ein/aus (statt Laufwerkwechsel)
	DW	ERR1		; ERA - nicht vorgesehen
	DW	ERR1		; REN - nicht vorgesehen

; CAOS 4.7 - Sprungtabelle Treiber Nr. 2 Diskette D004/D008

DRV1:	DB	2		; Kennung (Treiber Nr. 2)

else

; CAOS 4.6 - Sprungtabelle Treiber Nr. 1 Diskette D004/D008

DRV1:	DB	1		; Kennung (Treiber Nr. 1)
endif
	DB	5		; Treiber befindet sich in CAOS ROM-C
	DB	0,1		; SWITCH-Befehl aus/ein
	DB	'DISK'		; Name des Treibers
	DW	FMBO		; 01 Ausgabe Datenblock 128 Byte
	DW	FMBI		; 05 Einlesen Datenblock 128 Byte
	DW	FISRO		; 08 Ausgabedatei oeffnen und ersten Block 01h ausgeben
	DW	FCSRO		; 09 Ausgabedatei schliessen und Endeblock FFh ausgeben
	DW	FISRI		; 0A Eingabedatei oeffnen und Block 01 einlesen
	DW	FCSRI		; 0B Eingabedatei schliessen
	DW	NOOP		;
	DW	NOOP		;
	DW	FDIR		; DIR = Verzeichnisanzeige mit Maske
	DW	DRIVE		; CD  = Laufwerkwechsel
	DW	FERA		; ERA = Datei loeschen
	DW	FREN		; REN = Datei umbenennen
;
; Portinitialisierungstabellen:
;
if SYSROM
IOTAB1:	DB	6		; 6 Ports fuer Erstinitialisierung
else
IOTAB1:	DB	7		; 7 Ports fuer Erstinitialisierung
endif
; Bei Erstinitialisierung des PIO-Port-A muss sichergestellt werden, dass der
; CAOS-ROM-E immer im Zugriff bleibt. Im Einschaltzustand sind die PIO-Ports
; hochohmig und werden ueber Pull-Up-Widerstaende auf High gezogen, deshalb:
; 1. Interruptvektor einschreiben und PIO aus dem RESET-Zustand holen
;    (nur so wird das Ausgaberegister beschreibbar)
; 2. Datenwort einschreiben
; 3. Betriebsart Byte-Ausgabe einstellen um das Datenwort zu uebernehmen.
; Der RESET-Impuls zur D005-Tastatur wirkt nur, wenn die PIO bereits vorher
; auf Byte-Ausgabe initialisiert ist (also nur bei RESET, nicht bei POWER-ON)
	 DB	PIOAS,1		; PIO-A Steuerwort
	  DB	0E4h		; Interruptvektor

	 DB	PIOAD,2		; PIO-A Daten
	  DB	00011111b	; D005-Reset
	  DB	00001111b	; IRM ein, RAM0 WR ein, CAOS-E ein, USER-ROM aus

	 DB	PIOAS,2		; PIO-A Steuerwort
	  DB	00001111b	; Byte-Ausgabemodus
	  DB	00000011b	; DI

	 DB	PIOBS,3		; PIO-B Steuerwort
	  DB	0E6h		; Interruptvektor
	  DB	00001111b	; Byte-Ausgabemodus
	  DB	10000011b	; EI

	 DB	PIOBD,1		; PIO-B Daten
	  DB	0FFh		; Daten

	 DB	PORT1,1		; Ausgabeport 84h
	  DB	00101000b	; RAM8 0, LoRes
iff SYSROM
	 DB	PORT2,1		; Ausgabeport 86h
	  DB	11100011b	; RAM4 ein, USER-C 3, CAOS-C ein
endif
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

; Anzeige der CAOS-Version mit Datum

HELP7F:	DW	7F7FH
	DB	'help'
	DB	1
	CALL	OSTR
	DB	' KC-Club CAOS 4.7 '
if beta
	DB	'beta '		; Beta-Version?
endif
	DATUM			; Versionsdatum
	DB	CR,LF,0
	RET

	ABSFILL	0DAB8h,<ROM-D-Ende>
