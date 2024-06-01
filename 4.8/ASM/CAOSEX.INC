;*****************************************
;**	CAOS 4.7	Erweiterungen	**
;**					**
;** Kommandos:				**
;**	SETRO, SETWR, TIME, PRINT	**
;**					**
;**	in USER-ROM (EDAS-Ebene)	**
;**					**
;**	letzte Aenderung: 27.01.2023	**
;*****************************************

	DW	7F7FH
	DB	'SETRO'
	DB	1FH		; Textargument zulaessig (Dateiname)
	LD	HL,EXIT2	; Ruecksprungadresse
	PUSH	HL
	CALL	DISK		; D004 bereit?
	CALL	FNAME		; Dateiname abfragen und ausgeben zum D004
	LD	A,25H		; Setro
	JR	DOUT

SWR7F:	DW	7F7FH
	DB	'SETWR'
	DB	1FH		; Textargument zulaessig (Dateiname)
	LD	HL,EXIT2	; Ruecksprungadresse
	PUSH	HL
	CALL	DISK		; D004 bereit?
	CALL	FNAME		; Dateiname abfragen und ausgeben zum D004
	LD	A,31H		; Setwr
DOUT:	RET	C		; BRK
;	JP	STEUER		; Kommando ausgeben
;
; DEP-Steuerbyte 1 ausgeben, bei Fehler: Fehlercode/-text anzeigen
; PE:	A	Kommando-Steuerbyte
; PA:	A	Antwort von DEP
;	CY=1	Fehler aufgetreten
; VR:	AF,BC
;
STEUER:	LD	BC,STBYT1	; 80F3H
STEU0:	OUT	(C),A
STEU1:	PUSH	BC
	LD	A,1		; 6ms
	CALL	PV1
	DB	WAIT		; warten
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
	CALL	PV1
	DB	AHEX		; anzeigen
	CALL	DERR		; Aufruf CAOS-UP "ERRM"
FLER3:	SCF			; CY=1: Fehler
	RET
	;
FLER1:	LD	B,HIGH(ERRTX)	; 00F1H = Fehlertext von DEP 2.0
FLER2:	IN	A,(C)
	INC	B
	AND	A
	JR	Z,FLER3		; 00=Ende
	CALL	PV1
	DB	OCHR
	JR	FLER2

; Test ob Floppy arbeitet (fuer SETWR, SETRO, time)

DISK:	LD	BC,0FC80H	; Modulsteckplatz FC
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	Z,DISK1
	CALL	PV1
	DB	OSTR
	DB	'Kein D004!'
	DB	7,CR,LF,0
	POP	HL		; RET-Adresse vom Stack nehmen
	RET			; und Kommando beenden!
	;
DISK1:	LD	BC,UROK		; 0B3F3H
	IN	A,(C)
	CP	5		; CAOS-Betriebsart?
	RET	Z
	CALL	PV1
	DB	OSTR
	DB	'CAOS-Disk starten!'
	DB	7,CR,LF,0
	POP	HL		; RET-Adresse vom Stack nehmen
	RET			; und Kommando beenden!
;
; Name ein-, ausgeben: (fuer SETRO, SETWR)
; PE:	DE	Zeiger auf Dateinamen, wenn 0 dann Eingabe
; PA:	CY=1	BRK
;	DE	Zeiger auf Zeichen nach Dateinamen (2. Name bei REN)
;
FNAME:	EX	DE,HL
	LD	A,(HL)		; Zeichen nach Menuewort holen
	AND	0DFH
	JR	NZ,FNAM2	; Argument angegeben, als Dateiname nutzen!
	LD	HL,6
	CALL	PV1
	DB	OSTR
	DB	'Name :',0
	CALL	PV1
	DB	INLIN		; Dateiname abfragen
	RET	C		; BRK
	ADD	HL,DE		; Beginn Name
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

	DW	7F7FH
	DB	'TIME',1
	LD	HL,EXIT2	; Ruecksprungadresse
	PUSH	HL
	CALL	DISK		; D004 bereit?
	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	30H		; DEP ab 3.0?
	JR	NC,UHR
	CALL	PV1
	DB	OSTR
	DB	'Kein DEP 3!'
	DB	7,CR,LF,0
	RET

UHR:	LD	A,19H		; Cursor ganz nach links
	CALL	PV1
	DB	OCHR
	LD	E,2
	LD	BC,DATE+200H
UHR1:	IN	A,(C)		; Datum aus Koppel-RAM
	CALL	PV1
	DB	AHEX
	LD	A,'.'
	CALL	PV1
	DB	OCHR
	DEC	B
	DEC	E
	JR	NZ,UHR1
	IN	A,(C)		; Jahr
	CP	78H
	LD	A,19H		; 1978-1999
	ADC	A,0
	DAA			; 2000-2077
	CALL	PV1
	DB	AHEX		; Jahrhundert
	IN	A,(C)
	CALL	PV1
	DB	AHEX		; Jahr
	LD	BC,TIME
	LD	E,3
	LD	A,' '		; Trenner zwischen Datum und Uhrzeit
UHR2:	CALL	PV1
	DB	OCHR
	IN	A,(C)		; Uhrzeit aus Koppel-RAM
	CALL	PV1
	DB	AHEX
	LD	A,':'
	INC	B
	DEC	E
	JR	NZ,UHR2
	LD	A,(ARGN)	; Parameter angegeben?
	AND	A		; =Dauerlauf?
	JR	Z,UHR3		; nein!
	CALL	PV1
	DB	BRKT		; BRK?
	JR	NC,UHR
UHR3:	CALL	PV1
	DB	CRLF
	RET

; Druckereinstellungen

	DW	7F7FH
	DB	'PRINT',1FH
	LD	HL,EXIT2	; Ruecksprungadresse
	PUSH	HL
PRI:	LD	A,(DE)
	AND	A
	RET	Z		; fertig
	CP	' '
	JR	Z,PRINC
	CP	','
	JR	Z,PRK		; Zeichen
	CP	27H
	JR	Z,PRSTR		; String
	CALL	PV1
	DB	RHEX
	JP	C,DERR		; Bei Fehler -> CAOS-UP "ERRM"
	LD	A,(NUMVX)
PRIC:	CALL	OFILT		; Druckerausgabe ungefiltert
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
	CALL	OFILT		; Druckerausgabe ungefiltert
	JR	PRSTR

; nachfolgender Code wird aus Assembler-Teil mitbenutzt.
;
; Kommando beenden und USER-ROM abschalten
;
;EXIT2:	LD	DE,(ARGC-1)	; D = urspruenglicher Schaltzustand USER-C
;	LD	A,2		; USER-ROM
;	LD	L,A		; schalten
;	LD	E,MODU
;	JP	PV3		; zuerst USER-ROM schalten, dann zu CAOS
