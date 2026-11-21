;******************   PROGRAMM BSAVE	**************************************
;									     *
; Abspeichern von BASIC-Programmen auf MC-Ebene mit AUTO-Start		     *
; Es wird automatisch der Dateityp .KCB vergeben, um die Files von reinen    *
; MC-Programmen unterscheiden zu koennen.				     *
;									     *
; Test, ob BASIC initialisiert wurde.					     *
;									     *
; BASIC-Programmspeicher kann auf 7FFFH begrenzt werden, um Programme kompa- *
; tibel zum KC85/3 zu machen.						     *
;									     *
; NUR Menuewort %BSAVE							     *
;									     *
; 19.03.2017: Anpassungen fuer CAOS 4.7	als Include zu USER-ROM		     *
;									     *
;	(c) ML-Soft 26.01.1993-01.08.1997 (V 1.3) - 28.10.2022 (V 1.4)	     *
;*****************************************************************************

; BASIC-Vereinbarungen

;WSP	EQU	0300h		; BASIC-Arbeitsspeicher
;STDPTR	EQU	0356h		; Ende Stringspeicher (256 Byte vor Speicherende)
PSTBEG	EQU	035Fh		; Programmstart-Adresse
;INPBUF	EQU	0362h		; Eingabepuffer+1
;MEMSIZ	EQU	03B0h		; Speiucherende (MEMORY SIZE)
SWAPTR	EQU	03C4h		; Zeiger fuer naechsten String
SVARPT	EQU	03D7h		; Ende des BASIC-Programms
DVARPT	EQU	03D9h		; Adresse der Liste der Feldvariablen
FSLPTR	EQU	03DBh		; Erste freie Adresse hinter den Listen
NEW2	EQU	0C64Fh		; BASIC-Programm initialisieren
RUNMOD	EQU	0C854h		; RUN ausfuehren

; CAOS-Vereinbarungen

BASMEN	EQU	0E011H		; hier steht beim KC85/4+ das BASIC-Menuewort
BASON	EQU	0E02FH		; BASIC aktivieren (ab CAOS 4.1 gleiche Adresse)
;
; BASIC-Autostart:
;
BSTART:	LD	A,(BASMEN)
	CP	7FH		; KC85/4+ ?
	CALL	Z,BASON		; ja, dann Basic einschalten
	IN	A,(PIOAD)
	AND	0FBH		; fuer CAOS 3.1..4.22 und 5.0 IRM aus
	RRCA			; Bit 0 (ROM E) nach Bit 7 (BASIC) kopieren:
	RLA			;  fuer CAOS 3.4 BASIC-ROM an
	RLA			;  und fuer M006 CAOS-ROMF(=BASIC-ROM) aus
	RRCA			;   je nach Schaltzustand vom ROM E
	OUT	(PIOAD),A
	CALL	NEW2		; Basic-Programm initialisieren
	JP	RUNMOD		; RUN ausfuehren

;	IN	A,(88H)
;	OR	80H		; BASIC-ROM on
;	AND	0FBH		; IRM off
;	OUT	(88H),A
;	LD	A,(IX+4)
;	OR	60H		; BASIC-Ebene
;	OUT	(86H),A
;	LD	(IX+4),A
;	LD	A,89H		; RUN
;	LD	(INPBUF),A
;	JP	0C41FH		; Eingabe ausfuehren
;
;-----------------------------------------------------------------------------
;
STARTE:	DW	7F7FH
	DB	'BSAVE',1
	CALL	UROM		; Schaltzustand User-ROM merken
	PUSH	HL		; Ruecksprungadresse auf Stack legen
;
; Test, ob BASIC initialisiert wurde:
;
	LD	A,(WSP)
	CP	0C3H		; JP RSTLOC vorhanden?
	JR	NZ,NBAS		; nein, BASIC nicht initialisiert
	LD	A,(WSP+3)
	CP	0C3H		; JP USRLOC vorhanden?
	JR	NZ,NBAS		; nein, BASIC nicht initialisiert
	LD	HL,PSTBEG
	LD	E,(HL)
	INC	HL
	LD	D,(HL)		; DE = Beginn BASIC-Programm
	LD	HL,0400H
	SBC	HL,DE		; muss 0401h oder hoeher sein!
	JR	C,AANZ		; OK.
NBAS:	CALL	PV1
	DB	OSTR		; Programm zu gross oder nicht initialisiert
	DB	'BASIC-Programm?'
	DB	7,CR,LF,0
	RET
	;
AANZ:	LD	HL,(SVARPT)	; Programmende
	LD	A,H
	AND	80H		; darf maximal bis 7FFFh gehen...
	JR	NZ,NBAS
;
; Adressen anzeigen:
;
EOK:	LD	(ARG2),HL	; Endadresse
	CALL	PV1
	DB	OSTR
	DB	0Bh		; Cursor nach oben
	DB	'%BSAVE 0300 ',0
	LD	DE,0370h	; Startadresse
	LD	(ARG3),DE
	CALL	PV1
	DB	HLDE		; End- und Startadresse anzeigen
	CALL	NL		; CR+LF
;
; Startprogramm in Eingabepuffer kopieren:
;
	LD	HL,BSTART
;	LD	DE,0370H	; Mitten im BASIC-Eingabepuffer
	LD	BC,STARTE-BSTART
	LDIR			; kopieren
	LD	HL,KCB		; Dateityp *.KCB
	CALL	EIZE2		; Dateiname eingeben
	RET	C		; BRK
	CALL	NL		; CR+LF

	LD	HL,WSP		; ANF-Adresse
	LD	(ARG1),HL
	LD	A,H		; 3 Argumente
	LD	(ARGN),A

; Datei abspeichern:

	EX	DE,HL		; Dateiname
	CALL	PV1
	DB	SAVE
	RET
;
KCB:	DB	'KCB',0		; Dateiname *.KCB

;-----	end of include
