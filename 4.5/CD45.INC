;*****************************************
;**	CAOS 4.5	ROM D		**
;**					**
;**	Adresse:  D000h bis DAB7h	**
;**					**
;**	letzte Aenderung: 06.09.2010	**
;*****************************************

;	ORG	0D000H

; MC-Sprungverteiler (fuer die 9 Grundfunktionen)

JDISC:	CP	9		; Funktion 0-8 zugelassen
	RET	NC
	PUSH	AF
	LD	HL,0		; Dateiname
	CALL	FNAM2		; Name ausgeben
	LD	HL,DTAB		; Sprungtabelle der Floppyfunktionen
	POP	AF
	ADD	A,A
	LD	D,0
	LD	E,A
	ADD	HL,DE
	LD	E,(HL)		; Sprungtabelle lesen
	INC	HL
	LD	D,(HL)
	PUSH	DE		; DE=Sprungadresse

; Test ob Floppy arbeitet

DISK:	LD	BC,0FC80H	; Modulsteckplatz FC
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	Z,DISK1
	CALL	OSTR
	DB	'Kein D004!'
	DB	7,CR,LF,0
	POP	HL
	RET
DISK1:	LD	BC,UROK		; 0B3F3H
	IN	A,(C)
	CP	5		; CAOS-Betriebsart?
	RET	Z
	CALL	OSTR
	DB	'CAOS-Disk starten!'
	DB	7,CR,LF,0
	POP	HL
	RET

; Sprungtabelle Diskroutinen
				; Nr.
DTAB:	DW	FLOAD		; 0
	DW	FSAVE		; 1
	DW	DIR		; 2
	DW	STAT		; 3
	DW	REN		; 4
	DW	SETRO		; 5
	DW	SETWR		; 6
	DW	ERA		; 7
	DW	DRIVE		; 8

; Name ein-, ausgeben:
; PA:	DE = Zeiger auf Dateiname

FNAME:	EX	DE,HL
	LD	A,(HL)		; erstes Zeichen nach Menuewort holen
	AND	0DFH
	JR	NZ,FNAM2	; Argument angegeben, als Dateiname nutzen!
FNAM0:	LD	HL,6
FNAM1:	CALL	OSTR
	DB	'Name :',0
	CALL	INLIN
	RET	C		; BRK
	ADD	HL,DE
FNAM2:	PUSH	HL		; Zeiger auf Dateiname
	LD	E,12		; Dateiname 8.3 = 12 Zeichen
	LD	BC,DIRBUF	; 082F3H
FNAM3:	LD	A,(HL)
	OUT	(C),A
	INC	HL
	INC	B
	DEC	E
	JR	NZ,FNAM3
	POP	DE		; Dateiname (fuer FSAVE)
	LD	B,HIGH(STBYT1)	; 80F3H (BC fuer Steuerbyte 1)
	RET

; Steuerbyte 2 ausgeben

STEUB2:	LD	BC,STBYT2	; 080F1H
	JR	STEU0

; Steuerbyte 1 ausgeben

STEUER:	CP	9		; read/open ?
	JR	NZ,STEU3
	LD	A,1
	CALL	SETDEV		; Speichergeraet ist jetzt Diskette
	LD	A,9
STEU3:	LD	BC,STBYT1	; 080F3H
STEU0:	OUT	(C),A
STEU1:	PUSH	BC
	LD	A,1		; 6ms
	CALL	WAIT		; warten
	POP	BC
	IN	A,(C)
	BIT	0,A
	JR	NZ,STEU1	; fertig?
	BIT	7,A
	RET	Z		; kein Fehler
	POP	BC		; RET-Adresse vergessen!
FLERR:	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	NC,FLER1	; ja
	LD	BC,FCODE	; 081F3H
	IN	A,(C)		; Fehlernummer
	CALL	AHEX		; anzeigen
	JP	ERRM
	;
FLER1:	LD	B,HIGH(ERRTX)	; 00F1H = Fehlertext von DEP 2.0
FLER2:	IN	A,(C)
	INC	B
	AND	A
	RET	Z		; 00=Ende
	CALL	OCHR
	JR	FLER2

; BASEX-Programmteil mit Fehlertexten

BASEX:	LD	HL,BAS
	LD	DE,0D8H
	LD	BC,BASE-BAS
	LDIR			; Verteiler
	LD	HL,UMS
	LD	DE,150H
	LD	BC,UMSE-UMS
	LDIR			; Umschaltung
	LD	HL,BASEC
	LD	DE,200H
	LD	BC,UMS-BASEC
	LDIR			; BASEX selbst
	LD	HL,(SUTAB)
	LD	(SUSP),HL
	LD	DE,SUBAS
	LD	(SUTAB),DE
	LD	BC,92H
	LDIR			; SUTAB
	LD	HL,MBIND
	LD	(SUBAS+6EH),HL	; MB-Eingabe modifizieren
	LD	HL,MBOUTD
	LD	(SUBAS+70H),HL	; MB-Ausgabe modifizieren
	RET

BASEC:	EQU	$		; Adresse im ROMC
RUN:	EQU	$-BASEC+200H
	DB	0		; 1 ->RUN-Switch
SUSP:	EQU	$-BASEC+200H
	DW	0		; Speicher SUTAB

MBOUTD:	EQU	$-BASEC+200H
	LD	BC,BASBYT	; 08EF3H
	OUT	(C),A		; Datenbyte
	LD	A,D
	AND	48H
	OR	37H
	CALL	NAMBAS
	BIT	6,A		; schliessen?
	JR	Z,O1
	LD	D,A
	LD	A,(RUN)
	AND	A
	LD	A,D
	JR	Z,O1
	SET	7,A		; run-switch
O1:	OUT	(C),A		; Steuerbyte
O2:	IN	A,(C)
	BIT	0,A		; fertig?
	JR	NZ,O2
	RLCA
	SET	0,(IX+8)	; Taste gedrueckt
	RET	NC
BASERR:	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	NC,BERR2	; ja
	LD	BC,FCODE	; 081F3H
	IN	A,(C)
	CALL	AHEX		; Fehler-Nr.
BERR1:	LD	HL,(IOERR)
	PUSH	HL
	JP	IRMOF		; (vorher JP 0F01BH)
	;
BERR2:	LD	B,HIGH(ERRTX)	; Fehlertext - Register C ist bereits korrekt
BERR3:	IN	A,(C)
	JR	Z,BERR1		; 00-Ende
	INC	B
	CALL	OCHR
	JR	BERR3

MBIND:	EQU	$-BASEC+200H
	LD	A,D
	AND	48H
	OR	35H
	CALL	NAMBAS
	OUT	(C),A		; 80F3H = Steuerbyte
I1:	IN	A,(C)
	BIT	0,A		; fertig?
	JR	NZ,I1
	RLCA
	JR	C,BASERR	; Fehler
	LD	B,HIGH(BASBYT)	; 08EF3H
	IN	A,(C)		; Datenbyte
	RET

SUBAS:	EQU	$-BASEC+200H	; hier wird die SUTAB hin kopiert

; Umschaltung Tape/Disk

UMS:	CALL	IRMON		; bisher CALL 0F018H
	LD	HL,(SUSP)
	LD	DE,(SUTAB)
	LD	(SUTAB),HL
	LD	(SUSP),DE
	CALL	IRMOF		; bisher CALL 0F01BH
	RET

; Dateiname (in BASEX) ausgeben:

NAMBAS:	EQU	$-UMS+150H
	LD	BC,83F3H
	BIT	3,A		; open?
	JR	Z,NAMB2
	LD	E,11		; Anzahl Zeichen
NAMB1:	OUTI
	INC	B
	INC	B
	DEC	E
	JR	NZ,NAMB1
NAMB2:	LD	B,HIGH(STBYT1)	; 80F3H (BC fuer Steuerbyte 1)
	RET
UMSE:	; Ende der Umschaltroutine

; BASIC-Schnittstelle (vorbereitet fuer 12 Funktionen)

BAS:	LD	A,0		; CALL*D8	FLOAD
	DB	1
	LD	A,1		; CALL*DB	FSAVE
	DB	1
	LD	A,2		; CALL*DE	DIR
	DB	1
	LD	A,3		; CALL*E1	STAT
	DB	1
	LD	A,4		; CALL*E4	REN
	DB	1
	LD	A,5		; CALL*E7	SETRO
	DB	1
	LD	A,6		; CALL*EA	SETWR
	DB	1
	LD	A,7		; CALL*ED	ERA
	DB	1
	LD	A,8		; CALL*F0	DRIVE
	DB	1
	LD	A,9		; CALL*F3	frei
	DB	1
	LD	A,10		; CALL*F6	frei
	DB	1
	LD	A,11		; CALL*F9	frei
	DB	1
	LD	A,12		; CALL*FC	frei
	CALL	IRMON		; bisher CALL 0F018H
	CALL	JDISK		; Disk-Verteiler
	CALL	IRMOF		; bisher CALL 0F01BH
	RET
BASE:	; Ende BASIC-Verteiler

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

; Programm von Floppy laden

FLOA7F:	DW	7F7FH
	DB	'FLOAD'
	DB	1
	CALL	DISK		; D004 bereit?
	CALL	FNAM0
	RET	C		; Eingabe mit BRK abgebrochen
FLOAD:	LD	A,(ARGN)
	CP	2
	LD	A,0
	JR	C,FLO0
	SET	1,A		; merken: kein Autostart
FLO0:	LD	(IX+7),A
	LD	A,9		; Lesen oeffnen
	CALL	STEUER
	LD	BC,FDATA+1000H	; 90F2H
	IN	A,(C)		; Anzahl Parameter in Vorblock
	ADD	A,A
	ADD	A,A
	AND	1CH
	OR	(IX+7)
	LD	(IX+7),A
	IN	A,(C)		; Anzahl
	CP	2
	JR	C,FLO1		; wenn<2
	CP	10
	JR	C,FLO2		; wenn>10
FLO1:	CALL	OSTR
	DB	'Keine MC-Datei!'
	DB	7,CR,LF,0
	RET
FLO2:	INC	B
	IN	L,(C)
	INC	B
	IN	H,(C)		; HL=Ladeadresse
	INC	B
	IN	E,(C)
	INC	B
	IN	D,(C)		; DE=Endadresse
	CP	3
	JR	C,FLO3		; keine Startadresse
	PUSH	HL
	INC	B
	IN	L,(C)
	INC	B
	IN	H,(C)		; Startadresse
	EX	(SP),HL
FLO3:	EX	AF,AF'
	LD	A,(ARGN)
	AND	A
	JR	Z,FLO5		; ohne Offset
	LD	BC,(ARG1)	; Offset von Eingabe
	ADD	HL,BC		; Startadresse umrechnen
	EX	DE,HL
	ADD	HL,BC		; Endadresse umrechnen
	EX	DE,HL
	EX	AF,AF'
	JR	C,FLO4		; keine Startadresse
	EX	(SP),HL
	ADD	HL,BC		; Startadresse umrechnen
	EX	(SP),HL
FLO4:	EX	AF,AF'
FLO5:	CALL	HLDE		; Aadr Eadr anzeigen
	EX	AF,AF'
	JR	C,FLO6		; keine Startadresse
	EX	(SP),HL
	LD	(ARG3),HL
	CALL	HLHX		; Sadr anzeigen
	POP	HL
FLO6:	CALL	CRLF
FLO7:	LD	A,1		; Lesen normal
	CALL	STEUER
	DEC	C		; STBYT1 -> FDATA
FLO8:	IN	A,(C)
	CALL	LDMAE		; Daten in Speicher ablegen
	INC	HL
	INC	B
	PUSH	HL
	SBC	HL,DE		; Endadresse erreicht?
	POP	HL
	JR	NC,FLO9		; ja
	LD	A,B
	AND	A		; Sektor eingelesen?
	JR	NZ,FLO8		; nein, weitere Datenbytes abholen
	JR	FLO7		; neuen Sektor lesen
	;
FLO9:	LD	A,(IX+7)	; Autostart ausfuehren?
	BIT	1,A
	RET	NZ		; kein Start
	AND	1CH
	CP	0CH
	RET	C		; keine Sadr vorhanden
	LD	HL,(ARG3)	; Sadr
	PUSH	HL		; auf Stack ablegen
	JP	COFF		; erst noch CAOSC off!

; Datei auf Floppy abspeichern

FSAV7F:	DW	7F7FH
	DB	'FSAVE'
	DB	5
	CALL	DISK		; D004 bereit?
	CALL	FNAM0
	RET	C		; Eingabe mit BRK abgebrochen
FSAVE:	CALL	CLC		; Kassetten-Puffer loeschen
	EX	DE,HL
	LD	BC,12		; Dateiname+Typ 8.3
	LDIR			; an Anfang Kassettenpuffer kopieren
	LD	E,10H		; Argumente im Kassettenpuffer
	LD	HL,ARGN
	LD	A,(HL)		; Anzahl Argumente
	ADD	A,A		; *2
	INC	A
	LD	C,A		; Anzahl Bytes
	LDIR			; Argumente in Kassettenpuffer kopieren
	LD	HL,CASS
	LD	BC,FDATA+100H	; 81F2H
	LD	E,128
FSA2:	OUTI			; Vorblock ausgeben
	INC	B
	INC	B
	DEC	E
	JR	NZ,FSA2
	LD	A,0BH		; Schreiben oeffnen
	CALL	STEUER
	LD	HL,(ARG2)	; Endadresse+1
	LD	DE,(ARG1)	; Anfangsadresse
	AND	A
	SBC	HL,DE		; Anzahl Datenbytes
	EX	DE,HL		; HL=Adresse, DE=Anzahl
FSA3:	DEC	C		; 80F2H=FDATA
	LD	A,128
FSA4:	EX	AF,AF'		; A' = Sektorzaehler
	 CALL	LDAME		; Daten aus Speicher holen
	 INC	HL
	 OUT	(C),A		; und in Koppel-RAM schreiben
	 DEC	DE		; Anzahl-1
	 LD	A,D
	 OR	E
	 JR	Z,FSA5		; Anzahl=0 -> alle Daten geschrieben
	EX	AF,AF'
	INC	B
	DEC	A
	JR	NZ,FSA4		; bis Koppel-RAM voll ist
FSA5:	LD	A,3		; Schreiben normal
	CALL	STEUER
	LD	A,D		; Anzahl=0?
	OR	E
	JR	NZ,FSA3		; weitere Daten vorhanden!
	LD	A,43H		; Schreiben schliessen
	CALL	STEUER
	JP	CLC		; Puffer loeschen

; Directory mit Maske

DIR7F:	DW	7F7FH
	DB	'DIR'
	DB	1FH
	CALL	DISK		; D004 bereit?
	EX	DE,HL		; Maske aus Kommandozeile
	LD	DE,CASS
	LD	BC,11		; max. Laenge
	LDIR			; In Kassettenpuffer ablegen
	JR	DIR0
DIR:	LD	DE,CASS
DIR0:	XOR	A		; Abschluss Maske
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
	CALL	NC,DIR7		; Anzeige
	LD	DE,13
	ADD	HL,DE
	DJNZ	DIR5		; 3 Eintraege
	LD	A,5		; DIR weiter
	JR	DIR1
	;
DIR6:	LD	A,(CURSO)
	AND	A
	CALL	NZ,CRLF
	JR	STAT
	;
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

STAT:	LD	A,15H		; Status
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

REN7F:	DW	7F7FH
	DB	'REN'
	DB	1FH
	CALL	DISK		; D004 bereit?
REN:	CALL	OSTR
	DB	'Alter ',0
	LD	HL,12
	CALL	FNAM1
	RET	C		; BRK
	LD	A,29H		; REN alter Dateiname
	CALL	STEUER
	CALL	OSTR
	DB	'Neuer ',0
	LD	HL,12
	CALL	FNAM1
	LD	A,21H		; Ren neuer Dateiname
DOUT:	RET	C		; BRK
	CALL	STEUER		; Stack-Unwinding:
	RET			; nicht in JP umwandeln!

ERA7F:	DW	7F7FH
	DB	'ERA'
	DB	1FH
	CALL	DISK		; D004 bereit?
	CALL	FNAME
	JR	ERA1
ERA:	CALL	FNAM0
ERA1:	LD	A,11H		; Era
	JR	DOUT

SRO7F:	DW	7F7FH
	DB	'SETRO'
	DB	1FH
	CALL	DISK		; D004 bereit?
	CALL	FNAME
	JR	SETR1
SETRO:	CALL	FNAM0
SETR1:	LD	A,25H		; Setro
	JR	DOUT

SWR7F:	DW	7F7FH
	DB	'SETWR'
	DB	1FH
	CALL	DISK		; D004 bereit?
	CALL	FNAME
	JR	SETW1
SETWR:	CALL	FNAM0
SETW1:	LD	A,31H		; Setwr
	JR	DOUT

DRIV7F:	DW	7F7FH
	DB	'DRIVE'
	DB	1FH
	CALL	DISK		; D004 bereit?
	JR	DRIV0
DRIVE:	LD	DE,DRIV1
DRIV0:	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	20H		; DEP ab 2.0?
	JR	NC,DRIV2	; nein
	CALL	OSTR
	DB	'Kein DEP2!'
	DB	7,CR,LF
DRIV1:	DB	0
	RET
DRIV2:	EX	DE,HL
	LD	A,(HL)
	AND	0DFH
	JR	NZ,DRIV3	; mit Argument aufgerufen!
	CALL	OSTR
	DB	'Laufwerk:',0
	LD	A,1		; aktuell lesen
	CALL	STEUB2
	LD	BC,LWANF	; 081F1H
	IN	A,(C)
	CALL	OCHR		; Laufwerk anzeigen
	INC	B		; 082F1H
	IN	A,(C)		; aktueller User-Bereich
	AND	0FH
	CALL	NZ,AHEX0	; Userbereich hex anzeigen
	LD	A,9
	LD	(CURSO),A
	CALL	INLIN		; Eingabe
	RET	C		; BRK
	LD	HL,9
	ADD	HL,DE
DRIV3:	LD	BC,LWANF	; 081F1H
	LD	A,(HL)
	AND	A		; leer?
	RET	Z
	CALL	UPCASE
	OUT	(C),A		; LW-Code
	INC	HL
	LD	A,(HL)
	AND	A		; USER mit angegeben?
	LD	A,3		; nur LW anfordern
	JR	Z,DRIV5
	EX	DE,HL
	CALL	RHEX
	LD	A,0		; USER 0, weil
	JR	C,DRIV4		; keine HEX-Zahl
	LD	A,(NUMVX)
DRIV4:	INC	B		; 082F1H
	OUT	(C),A		; USER-Nr.
	LD	A,7		; LW+User anfordern
DRIV5:	CALL	STEUB2		; Stack-Unwinding:
	RET			; nicht in JP umwandeln!

TYPE7F:	DW	7F7FH
	DB	'TYPE'
	DB	1FH
	CALL	DISK		; D004 bereit?
	CALL	FNAME
	RET	C		; BRK
	LD	A,9		; read-open
	CALL	STEUER
	LD	BC,FDATA+1000H	; 90F2H
	INFC			; IN F,(C)
	LD	B,HIGH(FDATA)	; 80F2H
	JR	NZ,TYPE2	; kein Vorblock (bei Edas-Quellen hier 00h)
TYPE1:	LD	A,1		; read
	CALL	STEUER
	DEC	C		; 80F2H=FDATA
TYPE2:	IN	A,(C)		; Zeichen
	CP	3
	JP	Z,CRLF		; CAOS-Ende
	CP	1AH
	JP	Z,CRLF		; CP/M-Ende
	CP	20H
	JR	NC,TYPE5
	CP	LF
	JR	Z,TYPE5
	CP	CR
	JR	Z,TYPE5
	CP	9
	JR	NZ,TYPE4
TYPE3:	LD	A,' '
	CALL	OCHR
	LD	A,(CURSO)
	AND	7
	JR	NZ,TYPE3
	JR	TYPE6
TYPE4:	LD	A,'.'		; statt Steuerzeichen anzeigen
TYPE5:	CALL	OCHR
TYPE6:	LD	A,(CURSO+1)
	INC	A
	LD	HL,WINLG+1
	CP	(HL)		; Seite voll?
	JR	NZ,TYPE9
TYPE7:	CALL	INTB
	CP	3
	RET	Z		; BRK
	CP	0FH
	JR	NZ,TYPE8
	CALL	OCHR		; Hardcopy zulassen
	JR	TYPE7
TYPE8:	LD	A,0CH		; CLS
	CALL	CRT
	LD	A,LF
	CALL	CRT
TYPE9:	INC	B
	JR	NZ,TYPE2	; weiter anzeigen
	JR	TYPE1		; neuen Block lesen

DUMP7F:	DW	7F7FH
	DB	'DUMP'
	DB	1FH
	CALL	DISK		; D004 bereit?
	CALL	FNAME
	RET	C		; BRK
	LD	A,9		; Lesen oeffnen
	JR	DUMP2
DUMP7:	LD	A,0CH		; CLS
	CALL	CRT		; CRT damit nicht zu Druckprotokoll!
	LD	A,LF
	CALL	CRT
DUMP8:	LD	A,B
	AND	A
	JR	NZ,DUMP3
DUMP1:	LD	A,1		; Lesen normal
DUMP2:	CALL	STEUER
	DEC	C		; 80F2H=FDATA
DUMP3:	PUSH	BC
DUMP4:	LD	A,' '
	CALL	OCHR
	IN	A,(C)
	CALL	AHEX
	INC	B
	LD	A,7
	AND	B		; 8*Hexwert
	JR	NZ,DUMP4
	CALL	OSTR
	DB	'   ',0
	POP	BC
DUMP5:	IN	A,(C)
	CALL	CSTBT
	INC	B
	LD	A,7
	AND	B		; 8*Ascii
	JR	NZ,DUMP5
	CALL	CRLF
	LD	A,(CURSO+1)
	INC	A
	LD	HL,WINLG+1
	CP	(HL)		; Seite voll?
	JR	NZ,DUMP8
DUMP6:	CALL	INTB
	CP	3		; BRK?
	RET	Z		; ja
	CP	0FH
	JR	NZ,DUMP7
	CALL	OCHR		; Hardcopy zulassen
	JR	DUMP6

UUU:	DB	'INITIAL.UUU',0
	DW	7F7FH
	DB	'INIT'
	DB	1FH
	CALL	DISK		; D004 bereit?
	EX	DE,HL
	LD	A,(HL)		; Parameter in Kommandozeile
	AND	0DFH		; angegeben?
	JR	Z,INIT3		; nein, Standard-Dateiname verwenden
	PUSH	HL
INIT1:	LD	A,(HL)		; "Mini-Fileparser"
	AND	A
	JR	Z,INIT4		; Ende Dateiname
	INC	HL
	CP	':'		; LW?
	JR	NZ,INIT1
	LD	A,(HL)
	POP	HL
	AND	A
	JR	NZ,INIT4	; LW & Name
	LD	BC,LWANF	; 81F1H
	LD	A,(HL)
	OUT	(C),A		; LW-Code
	INC	HL
	LD	A,(HL)
	CP	':'
	LD	A,3		; ohne USER
	JR	Z,INIT2		; nur LW
	EX	DE,HL
	CALL	RHEX
	LD	A,(NUMVX)
	INC	B		; 82F1H
	OUT	(C),A		; User-Bereich
	LD	A,7
INIT2:	CALL	STEUB2
INIT3:	LD	HL,UUU		; Dateiname INITIAL.UUU
	PUSH	HL
INIT4:	POP	HL		; Dateiname
INIT5:	CALL	FNAM2
	LD	DE,CASS		; Kommando in Kassettenpuffer ablegen
	LD	A,9		; Lesen oeffnen
INIT6:	CALL	STEUER
	DEC	C		; 80F2H=FDATA
INIT7:	IN	A,(C)		; Zeichen
	AND	A
	JR	Z,INIT8		; 00 ignorieren
	CP	3
	JR	Z,INIT9		; CAOS-Ende
	CP	0AH
	JR	Z,INIT8		; LF ignorieren
	CP	1AH
	JR	Z,INIT9		; CP/M-Ende
	LD	(DE),A
	INC	E
	LD	A,E
	CP	7FH
	JR	Z,INIT9		; Puffer voll!
INIT8:	INC	B
	JR	NZ,INIT7	; weiter
	LD	A,1
	JR	INIT6		; neuen Block
INIT9:	LD	A,0
	LD	(DE),A		; Ende-Kennung anhaengen
	LD	HL,CASS
	LD	(FTAST),HL	; Aktivierung als ob es
	SET	6,(IX+8)	; eine Funktionstaste waere
	RET

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

; Anzeige der CAOS-Version mit Datum

HELP7F:	DW	7F7FH
	DB	'help'
	DB	1
	CALL	OSTR
	DB	' KC-Club CAOS 4.5 '
if beta
	DB	'beta '		; Beta-Version?
endif
	DATUM			; Versionsdatum
	DB	CR,LF,0
	RET

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

; IBM-Kodierungen fuer allgemeine Druckroutine PRINTC

ZIBM:	DB	7BH,7CH,7DH,7EH
	DB	5BH,5CH,5DH
	DB	84H,94H,81H,0E1H
	DB	8EH,99H,9AH

	ABSFILL	0DAB8h,<ROM-D-Ende>
