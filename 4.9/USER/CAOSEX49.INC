;*****************************************
;**	CAOS 4.9	Erweiterungen	**
;**	    USER-ROM (ASM-Ebene)	**
;**					**
;** Kommandos: TIME, PRINT		**
;**	TIME auch mit RTC von M064	**
;**	(SETRO, SETWR nicht mehr)	**
;**					**
;**	letzte Aenderung: 01.02.2026	**
;*****************************************


; Zeitanzeige vom RTC (M064) oder vom D004/D008 mit DEP 3.0

	DW	7F7FH
	DB	'TIME',1
	LD	HL,EXIT2	; Ruecksprungadresse
	PUSH	HL
UHR:	LD	A,19H		; Cursor ganz nach links
	CALL	PV1
	DB	OCHR
; 13.02.2024
; erst RTC auf Adresse 55h testen (Modul M064), danach D004/D008
	LD	BC,0E55H	; RTC-Register E
	IN	A,(C)		; ist normalerweise = 0
	ADD	A,1		; zumindest niemals FF!
	JR	Z,D004CL	; keine RTC vom M064 gefunden

; R/W-Zugriff vorbereiten
RTCWT:	LD	B,0DH		; Steuerregister D
	LD	A,1
	OUT	(C),A		; HOLD-Bit setzen
	IN	A,(C)
	AND	2		; BUSY?
	JR	Z,RTCOK		; Zugriff erlaubt
	XOR	A
	OUT	(C),A		; HOLD-Bit ruecksetzen
	JR	RTCWT		; nochmal testen
	;
RTCOK:	LD	E,'.'		; Trennzeichen
	LD	B,7		; Tag Zehner/Einer
	CALL	BCD		; Tag + Trennzeichen
	LD	B,9		; Monat Zehner/Einer
	CALL	BCD		; Monat + Trennzeichen
	CALL	PV1
	DB	OSTR		; Jahrhundert fest annehmen
	DB	'20',0		; da 19xx vorbei ist...
	LD	E,' '
	LD	B,11		; Jahr Zehner/Einer
	CALL	BCD		; Jahr + Trennzeichen
	LD	E,':'
	LD	B,5		; Stunde Zehner/Einer
	CALL	BCD		; Stunde + Trennzeichen
	CALL	BCD		; Minute + Trennzeichen
	LD	E,0		; kein Trennzeichen
	CALL	BCD		; Sekunde
	LD	B,0DH
	XOR	A
	OUT	(C),A		; HOLD-Bit ruecksetzen
	JP	UHR3
	;
D004CL:	LD	BC,0FC80H	; Modulsteckplatz FC
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	Z,DISK1		; ja
	CALL	PV1
	DB	OSTR
	DB	'Kein D004!'
	DB	7,CR,LF,0
	RET
	;
DISK1:	LD	BC,UROK		; 0B3F3H
	IN	A,(C)
	CP	5		; CAOS-Betriebsart?
	JR	Z,DISK2		; ja
	CALL	PV1
	DB	OSTR
	DB	'CAOS-Disk starten!'
	DB	7,CR,LF,0
	RET
	;
DISK2:	LD	BC,DEPVER	; 083F1H
	IN	A,(C)
	CP	30H		; DEP ab 3.0?
	JR	NC,UHR0		; ja
	CALL	PV1
	DB	OSTR		; keine Zeitquelle vorhanden
	DB	'Kein DEP 3!'
	DB	7,CR,LF,0
	RET
	;
UHR0:	LD	E,2
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
UHR3:	LD	A,(ARGN)	; Parameter angegeben?
	AND	A		; =Dauerlauf?
	JR	Z,UHR4		; nein!
	CALL	PV1
	DB	BRKT		; BRK?
	JP	NC,UHR
UHR4:	CALL	PV1
	DB	CRLF
	RET
;
BCD:	LD	HL,NUMNX
	IN	A,(C)
	RLD
	DEC	B		; Tag Einer
	IN	A,(C)
	RLD
	DEC	B
	LD	A,(HL)
	CALL	PV1
	DB	AHEX		; Tag anzeigen
	LD	A,E
	CALL	PV1		; Trennzeichen
	DB	OCHR
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
