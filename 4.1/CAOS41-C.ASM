CC41    ASM                                                                                                                      
;*********************************
;**				**
;**	CAOS 4.1 reassembliert	**
;**	von MLsoft		**
;**	13.06.92 bis 05.07.92	**
;**				**
;*********************************
 
;Achtung: mind. FREE 68FF erforderlich!
 
;**	Speicher}bersicht	**
 
;C000	MODUL, SYSTEM
;C0F7	KEYL, SQR, MULT, WININ, COLOR
;C17C	LINE, PUDE
;C206	DISPLAY, MODIFY
;C2B8	V24OUT
;C3B2	ShCLR-Reaktionsprogramm
;C408	Hard- und Screencopyroutinen
;C632	V24DUP und UMINIT
;C6F1	GARG, MBIN, MBOUT
;C887	frei *)
;--------------------------------
;E000	RESET und BASIC-Verteiler
;E040	CRT-Treiber mit PADR
;E339	KBD-Treiber mit BRKT
;E3EF	SWITCH, JUMP
;E4BB	TAPE-Treiber
;E65C	BASIC-Extension *)
;EDE8	frei *)
;EE00	Zeichenbildtab. Gro~buchst. *)
;--------------------------------
;F000	PWRON und Programmverteiler
;F12A	Kommandointerpreter (MENU)
;F238	KBD, IRM-Default, INLIN
;F354	Hexzahlen-Verarbeitung
;F3D0	Systeminit, SIXD, Portinittab.
;F46D	SAVE, VERIFY, LOAD
;F5F2	COLOR, CSTBT, DISPLAY, MODIFY,
;	WINDOW-Routinen, KEY, KEYLIST
;F723	CUCP, Grafikroutinen, CIRCLE
;F864	MBOUT,MBIN, MODUL, SYSTEM
;F8A0	BASIC-I/O-Verteiler
;F915	ZKOUT, TON, ESC-Progr{mmchen
;F9BF	V24OUT, V24DUP, ISR SIO
;FB5A	ROMC-Schalter, MULT, SQR
;FB84	ESC/P->IBM-Codewandeltabelle
;FB92	I/O-UP-Nummer-Tabelle
;FB9A	ESC-Tabelle
;FBB0	Unterprogrammtab. f}r SUTAB
;FC42	Tastaturtabelle KTAB
;FCC2	Interruptvektortabelle
;FCD0	Steuerfunktionstabelle f}r CTAB
;FD10	BASIC-Token-u.Zeiger-Tabelle *)
;FDF6	frei *)
;FDF8	Bittabelle f}r Punktroutinen
;FE00	Zeichenbildtab. Kleinbuchst. *)
;--------------------------------
;*)	ist nicht in diesem Listing
;	aufgef}hrt
 
 
;**	EQU-Liste		**
 
ARGC	EQU	0B780H
ARGN	EQU	0B781H
ARG1	EQU	0B782H
ARG2	EQU	0B784H
ARG3	EQU	0B786H
ARG49	EQU	0B788H
ARG10	EQU	0B794H
NUMNX	EQU	0B796H
NUMVX	EQU	0B797H
HCADR	EQU	0B799H
WINNR	EQU	0B79BH
WINON	EQU	0B79CH
WINLG	EQU	0B79EH
CURSO	EQU	0B7A0H
STBT	EQU	0B7A2H
COLOR	EQU	0B7A3H
WEND	EQU	0B7A4H
CCTL0	EQU	0B7A6H
SYSP	EQU	0B7AEH
SUTAB	EQU	0B7B0H
CTAB	EQU	0B7B2H
BJUMP	EQU	0B7B4H
OUTAB	EQU	0B7B9H
INTAB	EQU	0B7BBH
UOUT1	EQU	0B7BDH
UIN1	EQU	0B7C0H
UOUT2	EQU	0B7C3H
UIN2	EQU	0B7C6H
IOERR	EQU	0B7C9H
VRAM	EQU	0B7CBH
ZOTAB	EQU	0B7CDH
ZWEND	EQU	0B7CFH
FTAST	EQU	0B7D1H
HOR	EQU	0B7D3H
VERT	EQU	0B7D5H
FARB	EQU	0B7D6H
MIXIT	EQU	0B7D7H
VORTN	EQU	0B7D8H
DTADR	EQU	0B7DAH
DTNR	EQU	0B7DCH
L3TAB	EQU	0B7DDH
L3SIZ	EQU	0B7DFH
COUNT	EQU	0B7E0H
HCPZ	EQU	0B7E1H
INTV1	EQU	0B7E2H
INTV1L	EQU	0B7E4H
INTV2	EQU	0B7E5H
INTV2L	EQU	0B7E7H
HCPZ2	EQU	0B7E8H

WNDFN	EQU	0B99CH	; Fenstervektoren
;--------------------------------
 
 
;**	CAOS 4.1 ROM C		**
 
	ORG	0C000H
 
 
MODULC	;Modulkontrollanzeige
	PUSH	DE
	LD	L,8	;ab Platz 8
MODC1	PUSH	HL
	LD	A,1	;lesen
	CALL	MODU
	LD	A,H
	INC	A	;Kennbyte=FFh?
	JR	Z,MODC2	;kein Modul
	LD	A,L
	CALL	AHSPC	;Platz
	LD	A,H
	CALL	AHSPC	;Kennbyte
	LD	A,D
	CALL	AHEX	;Steuerbyte
	CALL	CRLF
MODC2	POP	HL
	LD	A,L
	ADD	4	;Viererschritte
	LD	L,A
	JR	NC,MODC1;bis FCh
	CALL	CRLF
	POP	DE
	RET
 
SYSTC	;Systemcheck, umst{ndlichst!!
	PUSH	DE
	CALL	OSTR
	DEFM	'CAOS'
	NOP
	LD	A,'E'
	CALL	CRT
	IN	A,88H
	RRCA		;Bit 0
	JR	C,SYSC1
	CALL	WON	;on/off ausg.
	JR	SYSC2
 
SYSC1	CALL	WOFF
SYSC2	CALL	OSTR
	DEFM	'ROMC '
	NOP
	LD	A,(IX+4)
	SRL	A	;Bits 5,6
	SRL	A
	SRL	A
	SRL	A
	SRL	A	;nach Bits 0,1
	CPL
	AND	'3'
	CALL	CRT	;Ebene ausgeben
	IN	A,88H
	LD	E,A
	RLCA		;Bit 7
	JR	C,SYSC3
	CALL	WON	;on/off ausg.
	JR	SYSC4
 
SYSC3	CALL	WOFF
SYSC4	CALL	WRAM	;'RAM' ausg.
	LD	A,'0'
	CALL	CRT	;RAM0
	BIT	1,E	;von 88h
	JR	NZ,SYSC5
	CALL	WON
	JR	SYSC6
 
SYSC5	CALL	WOFF
SYSC6	CALL	WRAM
	LD	A,'4'	;RAM4
	CALL	CRT
	BIT	0,(IX+4)
	JR	NZ,SYSC7
	CALL	WON	;on/off ausg.
	JR	SYSC8
 
SYSC7	CALL	WOFF
SYSC8	CALL	WRAM
	CALL	OSTR
	DEFM	'8 '	;RAM8
	NOP
	LD	A,(IX+1)
	SRL	A	;Bits 4,5,6,7
	SRL	A
	SRL	A
	SRL	A	;nach 0,1,2,3
	ADD	'0'-2
	CALL	CRT	;Ebene ausg.
	IN	A,89H
	BIT	5,A
	JR	NZ,SYSC9
	CALL	WON	;on/off ausg.
	JR	SYSC10
 
SYSC9	CALL	WOFF
SYSC10	LD	A,(IX+1)
	LD	E,A
	CALL	OSTR
	DEFM	'BILD '
	NOP
	BIT	2,E	;Zugriffs-Bit
	LD	A,'0'
	JR	Z,SYSC11
	INC	A
SYSC11	CALL	CRT	;Nummer ausg.
	CALL	CRLF
	POP	DE
	RET
 
WRAM	CALL	OSTR
	DEFM	'RAM'
	NOP
	RET
 
WOFF	CALL	OSTR
	DEFM	' ON'
	DEFW	0A0DH
	NOP
	RET
 
WON	CALL	OSTR
	DEFM	' OFF'
	DEFW	0A0DH
	NOP
	RET
 
KEYLIC	;Anzeige Belegung F-Tasten
	LD	HL,0B900H
	LD	BC,0CF0H
KEYLC1	INC	C
	LD	A,C
	CALL	AHOSTR	;Fn ausgeben
	DEFM	' :'
	NOP
	CALL	NCHAR	;Belegung
	CALL	CRLF
	DJNZ	KEYLC1	;12*
	RET
 
SQRC	;Quadratwurzel A=SQR(HL)
	XOR	A
	LD	A,0FFH
	LD	E,A
	LD	D,A
SQRC1	INC	A
	INC	DE
	INC	DE
	SBC	HL,DE
	JP	P,SQRC1
	RET
 
MULC	;Multiplikation BA=C*D
	LD	B,0
	LD	E,9
MULC1	LD	A,D
	RRA
	DEC	E
	RET	Z
	LD	D,A
	LD	A,B
	JR	NC,MULC2
	ADD	C
MULC2	RRA
	LD	B,A
	JR	MULC1	;8*
 
WININC	;Initialisierung eines Fensters
	PUSH	AF
	CP	0AH	;No.
	JR	NC,WINIE
	LD	A,D	;Zeilen
	AND	A
	JR	Z,WINIE
	ADD	H
	JR	C,WINIE
	CP	21H
	JR	NC,WINIE
	LD	A,E	;Spalten
	AND	A
	JR	Z,WINIE
	ADD	C	;richtig: ADD L!
	JR	C,WINIE
	CP	29H
	JR	NC,WINIE
	POP	AF
	PUSH	DE
	PUSH	HL
	CALL	WCOPY
	POP	HL
	POP	DE
	LD	(WINON),HL
	LD	(WINLG),DE
	LD	(CURSO),BC
	LD	(WINNR),A
	AND	A
	RET
 
WINIE	POP	AF
	SCF
	RET
 
COLRC	;Farbe setzen (aus L und E)
	LD	A,1FH
	AND	L
	RLA
	RLA
	RLA
	LD	L,A
	LD	A,(ARGN)
	CP	2
	LD	A,E
	JR	NC,COLRC1
	LD	A,(COLOR)
COLRC1	AND	7
	OR	L
	LD	(COLOR),A
	RET
 
LINEC	;Zeichnen Linie von
;	(ARG1)/(ARG2) nach (ARG3)/(ARG4)
	LD	HL,(ARG2) ;YANF
	LD	DE,(ARG49);YEND
	EXX
	LD	HL,(ARG1) ;XANF
	LD	DE,(ARG3) ;XEND
LINC1	XOR	A	;Bit-Merker=0
	SBC	HL,DE
	JR	NC,LINC2;wenn XANF>=XEND
	ADD	HL,DE	;r}ckg{ngig
	EX	DE,HL
	EXX
	EX	DE,HL
	EXX
	JR	LINC1	;HL=XSTEPS
 
LINC2	EXX
	PUSH	DE	;YEND retten
LINC3	SBC	HL,DE
	JR	NC,LINC4;steigende Linie
	ADD	HL,DE	;r}ckg{ngig
	EX	DE,HL
	OR	1	;merken
	JR	LINC3	;fallende Linie
LINC4	PUSH	HL	;YSTEPS retten
	EXX		;HL=XSTEPS
	POP	BC	;BC=YSTEPS
	PUSH	HL
	SBC	HL,BC	;Vergleich
	POP	HL
	PUSH	BC
	JR	NC,LINC5;HL>=BC, flache
	EX	(SP),HL	;HL<BC, steile
	OR	2	;Linie merken
LINC5	PUSH	HL	;HL=MAX(XSTEPS,
	LD	B,H	; YSTEPS)
	LD	C,L	;=Punktezahl-1
	EXX
	POP	BC	;BC=MAX(...)
	POP	DE	;DE=MIN(XSTEPS,
	LD	H,B	; YSTEPS)
	LD	L,C
	SRL	H	;Slave mit 1/2
	RR	L	;+Master laden
	EXX
	POP	HL
	EX	DE,HL
 
	;Hauptschleife
;AF Bitregister
;HL lfd. X-Koordinate	HL' Slave
;DE lfd. Y-Koordinate	DE' -Master
;BC Punktez{hler	BC' +Master
 
LINCS	CALL	SHLDE	;Punkt setzen
	EXX
	AND	A
	SBC	HL,DE	;-Master subtr
	JR	NC,LINC6;kein ]bertrag-
			;kein Sprung!
	ADD	HL,BC	;+Master add.
			;CY bleibt =1!
LINC6	EXX
	BIT	1,A	;steil?
	JR	NZ,LINC7
	INC	HL
	JR	NC,LINC9
	AND	A
LINC7	BIT	0,A	;fallend?
	INC	DE
	JR	Z,LINC8
	DEC	DE
	DEC	DE
LINC8	JR	NC,LINC9
	INC	HL
LINC9	EX	AF
	LD	A,B
	OR	C	;letzter Punkt?
	RET	Z
	DEC	BC
	EX	AF
	JR	LINCS
 
PUDEC	;Punkt l|schen, PA: A=Farbbyte
	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	BITT	;aus BITTAB ber.
	JR	C,PUDEC1	;au~erhb
	LD	B,A	;Bit
	LD	C,M
	CPL
	AND	C
	LD	M,A	;l|schen
	LD	A,(IX+1)
	LD	E,A
	XOR	2
	DI
	OUT	84H
	LD	D,M	;Farbbyte
	LD	A,E
	OUT	84H
	EI
	LD	A,B	;Z=1: Punkt
	AND	C	;war gel|scht
	LD	A,D	;Frabbyte
PUDEC1	POP	BC
	POP	DE
	POP	HL
	RET
 
DPMEMO	;Ausgabe einer Displayzeile
	;HL=Adr, C=Anzahl Bytes
	CALL	HLHX	;Adresse
	PUSH	HL
	LD	B,C
DPME1	CALL	LDAME	;Byte holen
	INC	HL
	CALL	AHSPC	;anzeigen
	DJNZ	DPME1	;C*
	POP	HL
	LD	B,C
	LD	A,9	;CUR
	CALL	OCHR
DPME2	CALL	LDAME	;Byte holen
	INC	HL
	CALL	CSTBT	;als ASCII
	DJNZ	DPME2
	RET
 
DISPC	;Anzeige Speicherbereich
	;HL=Adr, E=Listblockl{nge
	;C=Anzahl Bytes pro Zeile
	;A=Anzahl Arergumente
	CP	3
	JR	NC,DISP1
	LD	C,8	;default
	CP	2
	JR	NC,DISP1
	LD	E,4	;default
DISP1	PUSH	DE
DISP2	CALL	DPMEMO	;1 Zeile
	CALL	CRLF
	CALL	BRKT
	JR	C,DISP3
	DEC	E
	JR	NZ,DISP2;E*
	CALL	INTB	;Warten Taste
DISP3	POP	DE
	CP	3	;BRK?
	RET	Z
	CP	13H	;STOP?
	JR	NZ,DISP1
	JR	MODI1
 
MODIC	;Anzeige/Ver{ndern Speicher-
	;bereich, Par. wie DISPC
	LD	C,1
MODI1	CALL	DPMEMO
	LD	A,5
	LD	(CURSO),A
	CALL	INLIN
	PUSH	HL
	CALL	RHEX	;Adresse
	LD	A,M
	POP	HL
	JR	C,MODI9	;Fehler
	AND	A
	JR	Z,MODI1
	LD	HL,(NUMVX)
MODI2	PUSH	HL
	CALL	RHEX	;Datenbyte
	LD	A,M
	AND	A	;kein Zeichen
	INC	HL	;einlesbar?
	LD	A,M
	POP	HL
	JR	Z,MODI4
MODI3	CALL	LDMAE	;eintragen
	INC	HL
MODI4	LD	A,(DE)
	CP	'.'	;Abschlu~?
	RET	Z
	CP	','	;1 Zeichen?
	JR	NZ,MODI5
	INC	DE
	LD	A,(DE)	;vom VRAM
	INC	DE
	JR	MODI3	;eintragen
 
MODI5	CP	':'	;R}ckschritt?
	JR	NZ,MODI6
	DEC	HL
	JR	MODI1
 
MODI6	CP	'/'	;neue Adresse?
	JR	NZ,MODI0
	INC	DE
	CALL	RHEX	;Fehler
	JR	C,MODI9
	LD	HL,(NUMVX)
	JR	MODI1
 
MODI0	CP	27H	;Zeichenkette?
	JR	NZ,MODI8
	INC	DE
MODI7	LD	A,(DE)	;Zeichen holen
	INC	DE
	AND	A	;Dummy-Ende?
	JR	Z,MODI1
	CP	27H	;Ende Kette?
	JR	Z,MODI4
	LD	M,A	;PROGRAMMFEHLER
	INC	HL	;(richtig LDMAE)
	JR	MODI7
 
MODI8	AND	A	;Ende Zeile?
	JR	Z,MODI1
	CP	' '	;Leerzeichen?
	JR	Z,MODI2
MODI9	CALL	ERRM	;sonst ERROR
	JR	MODI1
 
V24OC	;Initialisieren Druckerausgabe
	;PE: Argumente
	LD	HL,ARGN
	LD	A,M
	CP	3
	JR	NC,V24O3;mehr als 2 Arg
	LD	B,8	;ab Platz 8
	CALL	V24SU	;M003 suchen
	JR	C,V24O6	;nicht gef.
	LD	HL,HCPGM
	LD	(HCADR),HL;ShCLR belegen
	LD	DE,PRINT
	LD	HL,HCPZ
	BIT	1,M	;UOUT1 oder 2?
	JR	NZ,V24O1
	LD	(UOUT1+1),DE
	JR	V24O2
 
V24O1	LD	(UOUT2+1),DE
V24O2	JP	V24O12
 
V24O3	INC	HL
	LD	B,M	;ab Platz (ARG1)
	CALL	V24SU	;M003 suchen
	JR	C,V24O6	;nicht gef.
	INC	HL
	INC	HL
	LD	A,M	;SIO A oder B?
	DEC	A
	JR	NZ,V24O4
	LD	A,(HCPZ)
	RES	2,A	;merken SIO-A
	LD	(HCPZ),A
	JR	V24O5
 
V24O4	DEC	A
	JR	NZ,V24O6;Fehler
	LD	A,(HCPZ)
	SET	2,A	;merken SIO-B
	LD	(HCPZ),A
V24O5	INC	HL
	INC	HL
	LD	DE,PRINT
	LD	A,M	;USER1 oder 2?
	DEC	A
	DEC	A
	JR	NZ,V24O7
	LD	(UOUT1+1),DE
	LD	DE,HCPZ
	LD	A,(DE)
	RES	1,A	;merken USER1
	LD	(DE),A
	JR	V24O8
 
V24O6	JR	V24O13	;Fehlersprung
 
V24O7	DEC	A
	JR	NZ,V24O6;Fehler
	LD	(UOUT2+1),DE
	LD	DE,HCPZ
	LD	A,(DE)
	SET	1,A	;merken USER2
	LD	(DE),A
V24O8	LD	A,(ARGN)
	CP	4
	INC	HL
	INC	HL
	JR	C,V24O12;weniger als 4
	LD	A,M
	AND	A	;keine Fkt von
	JR	Z,V24O11;ShCLR?-ja
	DEC	A	;Protokoll?
	JR	Z,V24O9
	DEC	A	;Hardcopy?
	JR	NZ,V24O13
	LD	A,(HCPZ)
	SET	0,A	;Hcopy merken
	JR	V24O10
 
V24O9	LD	A,(HCPZ)
	RES	0,A	;Hcopy aus
V24O10	LD	(HCPZ),A
	LD	DE,HCPGM
	LD	(HCADR),DE
V24O11	LD	A,(ARGN)
	CP	5	;5 Agrumente?
	JR	NZ,V24O12
	INC	HL
	INC	HL
	LD	A,M	;Druckertyp
	SLA	A
	SLA	A
	SLA	A	;nach obere
	SLA	A	;Tetrade
	LD	C,A
	LD	A,(HCPZ)
	AND	0FH	;in (HCPZ)
	OR	C	;einbauen
	LD	(HCPZ),A
V24O12	CALL	LCSO
	LD	A,0CH	;CTC-Adresse
	PUSH	BC
	ADD	C
	LD	B,2	;2 Steuerbytes
	LD	C,A
	LD	HL,(INTV1)
	DI
	OTIR		;CTC init.
	POP	BC
	LD	A,(INTV1L)
	DEC	A
	DEC	A	;-2
	LD	B,A
	LD	A,0AH	;SIO-Adresse
	ADD	C
	LD	C,A
	OTIR		;SIO init.
	EI
	RET
 
V24O13	JP	ERRM
 
V24SU	;Suche 1. V24-Modul
	LD	D,3CH
	LD	C,80H
V24S1	IN	A,(C)
	CP	0EEH
	JR	Z,V24S2
	INC	B	;bl|dsinniger-
	INC	B	;weise in
	INC	B	;Viererschritten
	INC	B
	DEC	D
	JR	NZ,V24S1
	SCF		;nicht gef.
	RET
 
V24S2	PUSH	HL
	LD	L,B
	LD	A,2
	LD	D,1
	CALL	MODU	;einschalten
	POP	HL
	RET
 
HCPGMC	;ShCLR-Reaktionsprogramm
	PUSH	HL
	PUSH	DE
	LD	A,(HCPZ)
	BIT	0,A	;Hardcopy?
	JP	NZ,COPIES
 
	;Protokollfunktion
	LD	HL,(OUTAB)
	XOR	A
	OR	M	;Null?
	JR	NZ,PROTO2
	LD	DE,ECHO
	DEC	A	;PROGRAMMFEHLER
	DEC	A	;(A ist Null!)
	JR	Z,PROTO1
	LD	HL,ZEI3	;nein
	JR	PROTO3
 
PROTO1	LD	HL,ZEI2
	JR	PROTO4
 
	LD	(UOUT1+1),DE;RUDIMENT
	LD	HL,ZEI2	    ;RUDIMENT
	JR	PROTO5	    ;RUDIMENT
 
PROTO2	;Protokoll deaktivieren
	DEC	A
	LD	DE,PRINT
	LD	HL,ZEI0
	DEC	A
	JR	Z,PROTO4
PROTO3	LD	(UOUT2+1),DE;r}cksetzen
	JR	PROTO5
 
PROTO4	LD	(UOUT1+1),DE;r}cksetzen
PROTO5	LD	(OUTAB),HL
	POP	DE
	POP	HL
	RET
 
	;undefinierbarer Rest
	DEFW	5B1BH
	DEFW	3131H
	DEFW	5B61H
	DEFW	3130H
	DEFW	1B65H
	DEFW	375BH
	DEFW	7138H
	DEFW	404BH
	DEFB	2
 
COPIES	;Hard- oder Screencopy
	PUSH	BC
	PUSH	AF
	LD	A,(HCPZ)
	BIT	7,A
	JR	Z,HCOPYC
 
	;Screencopy
	LD	HL,(VRAM)
	LD	B,20H
SCOPY1	LD	C,28H
SCOPY2	LD	A,M
	INC	HL
	AND	A
	JR	NZ,SCOPY3
	LD	A,20H	;doppelt
SCOPY3	CP	20H
	JR	NC,SCOPY4
	LD	A,20H	;gemoppelt
SCOPY4	CALL	PRINT
	DEC	C	;40 Spalten
	JR	NZ,SCOPY2
	LD	A,0DH	;CR
	CALL	PRINT4
	LD	A,0AH	;LF
	CALL	PRINT4
	DJNZ	SCOPY1	;32 Zeilen
HCEND	POP	AF
	JP	POP3
 
HCOPYC	;..nach Typ verzweigen
	AND	0F0H
	JR	Z,K6313	;EPSON-komp.
	CP	10H
	JR	Z,K6314	;dto. breit
	CP	20H
	JP	Z,K6311	;mit ANSI
	CP	30H
	JP	Z,K6312	;dto breit
	CP	50H
	JP	Z,K6304	;Thermodrucker
	JR	HCEND
	;K6303 u.a.: keine Hardcopy!
 
K6313	LD	H,0
K6313a	PUSH	HL
	LD	HL,C6313;Tabelle
	LD	B,10	;L{nge
	CALL	PRZK
	POP	HL
	CALL	HCLINE	;1 Zeile
	JR	NC,K6313a ;n{chste
HCCRLF	LD	A,0DH
	CALL	OFILT
	LD	A,0AH
	CALL	OFILT	;CRLF
	JP	HCEND
 
HCLINE	LD	L,0
HCLIN1	LD	BC,8
	PUSH	HL
	LD	DE,0B700H ;Kassetten-
	CALL	PADR	  ;puffer
	LDIR
	LD	C,8
HCLIN2	LD	B,8
	LD	HL,0B700H
HCLIN3	RL	M
	RLA
	INC	HL
	DJNZ	HCLIN3
	CALL	OFILT
	DEC	C
	JR	NZ,HCLIN2 ;n{chster
	POP	HL	  ;Cursor
	INC	L
	LD	A,28H
	CP	L
	JR	NZ,HCLIN1
	LD	A,H
	ADD	8	;n{chste Zeile
	LD	H,A
	RET
 
K6314	LD	H,0
K6314a	PUSH	HL
	LD	HL,C6314;Tabelle
	LD	B,10	;L{nge
	CALL	PRZK
	POP	HL
	LD	L,0
K6314b	LD	B,4
	PUSH	HL
	LD	DE,0B700H
	CALL	PADR
K6314c	LD	A,M
	LD	(DE),A
	INC	DE
	LD	(DE),A
	INC	DE
	INC	L
	DJNZ	K6314c	;4*
	LD	C,8
K6314d	LD	B,8
	LD	HL,0B700H
K6314e	RL	M
	RLA
	INC	HL
	DJNZ	K6314e
	CALL	OFILT
	CALL	OFILT
	DEC	C
	JR	NZ,K6314d
	POP	HL
	INC	L
	LD	A,28H
	CP	L
	JR	NZ,K6314b
	LD	A,H
	ADD	4
	LD	H,A
	JR	NC,K6314a
	JR	HCCRLF
 
CBYTES	;C Bytes nach Kass-Puffer
	PUSH	HL
	PUSH	BC
	CALL	PADR
	LD	B,0
	LDIR
	POP	BC
	POP	HL
	RET
 
SPRS	;8* 6-Nadel-Sprosse ausgeben
	PUSH	HL
	PUSH	BC
	LD	L,80H
SPRS1	LD	B,0
	LD	DE,0B700H
SPRS2	LD	A,(DE)
	AND	L	;CY=0
	JR	Z,SPRS3
	SCF		;CY=1
SPRS3	RL	B
	INC	DE
	LD	A,6	;f}r 6 Bit
	CP	E
	JR	NZ,SPRS2;oberste Nadel
			;war 0
	AND	A	;unterste Nadel
	RL	B	;=0
	LD	A,B
	CALL	OFILT
	RRC	L
	BIT	7,L
	JR	Z,SPRS1
	POP	BC
	POP	HL
	RET
 
K6311	LD	HL,C6311;Tabelle
	LD	B,14	;L{nge
	CALL	PRZK
	LD	B,42	;256/6=42 Rest4
	LD	HL,0
K6311a	LD	DE,0B700H
	LD	C,6
	CALL	CBYTES
	CALL	SPRS	;1 6*8-Feld
	INC	L	;ausgeben
	LD	A,28H
	CP	L
	JR	NZ,K6311a
	LD	A,6	;6 Pixel tiefer
	ADD	H
	LD	H,A
	LD	L,0
	PUSH	HL
	PUSH	BC
	LD	HL,C6311;wieder Tabelle
	LD	B,14	;L{nge
	CALL	PRZK
	POP	BC
	POP	HL
	DEC	B
	JR	NZ,K6311a
K6311b	LD	DE,0B700H
	LD	C,4	;den Rest4
	CALL	CBYTES
	XOR	A
	LD	(DE),A	;letzten beiden
	INC	DE	;Zeilen leeren
	LD	(DE),A
	CALL	SPRS
	INC	L
	LD	A,28H
	CP	L
	JR	NZ,K6311b
	JP	HCCRLF
 
CBYTS2	;C Bytes nach Puffer & doppeln
	PUSH	HL
	LD	DE,0B700H
	CALL	PADR
C2BYTL	LD	A,M
	LD	(DE),A
	INC	DE
	LD	(DE),A	;doppelt
	INC	DE
	INC	L
	DEC	C
	JR	NZ,C2BYTL
	POP	HL
	RET
 
S2PRS	;8* 6-Nadel-Sprosse doppelt out
	PUSH	HL
	PUSH	BC
	LD	L,80H
S2PR1	LD	B,0
	LD	DE,0B700H
S2PR2	LD	A,(DE)
	AND	L
	JR	Z,S2PR3
	SCF
S2PR3	RL	B
	INC	DE
	LD	A,6
	CP	E
	JR	NZ,S2PR2
	LD	A,B
	RLA		;untere beiden
	RLA		;Nadeln tot
	CALL	OFILT
	CALL	OFILT
	RRC	L
	BIT	7,L
	JR	Z,S2PR1
	POP	BC
	POP	HL
	RET
 
K6312	LD	HL,C6312;Tabelle
	LD	B,14	;L{nge
	CALL	PRZK
	LD	B,85	;256/3=85 Rest1
	LD	HL,0
K6312a	LD	C,3
	CALL	CBYTS2
	CALL	S2PRS
	INC	L
	LD	A,28H
	CP	L
	JR	NZ,K6312a
	INC	H
	INC	H
	INC	H
	LD	L,0
	PUSH	HL
	PUSH	BC
	LD	HL,C6312
	LD	B,14
	CALL	PRZK
	POP	BC
	POP	HL
	DJNZ	K6312a
K6312b	LD	C,1	;Rest1
	CALL	CBYTS2
	XOR	A
	LD	(DE),A
	INC	DE
	LD	(DE),A
	CALL	S2PRS
	INC	L
	LD	A,28H
	CP	L
	JR	NZ,K6312b
	JP	HCCRLF
 
K6304	LD	H,0
K6304a	PUSH	HL
	LD	HL,C6304;Tabelle
	LD	B,9	;L{nge
	CALL	PRZK
	POP	HL
	CALL	HCLINE	;1 Zeile
	JR	NC,K6304a
	JP	HCCRLF
 
PRZK	;Durch (HL) adressierte
	;Zeichenkette ausgeben, B lang
	LD	A,M
	INC	HL
	CALL	OFILT
	DJNZ	PRZK
	RET
 
	;Grafiksequenzen f}r
	;die Druckertypen
C6313	DEFW	90DH
	DEFB	1BH
	DEFB	'J'
	DEFB	24
	DEFB	1BH
	DEFB	'*'
	DEFB	5
	DEFW	140H
 
C6314	DEFW	90DH
	DEFB	1BH
	DEFB	'J'
	DEFB	24
	DEFB	1BH
	DEFB	'*'
	DEFB	5
	DEFW	280H
 
C6311	DEFB	1BH
	DEFM	'[01e'
	DEFB	1BH
	DEFM	'[12`'
	DEFB	1BH
	DEFB	'K'
	DEFW	140H
 
C6312	DEFB	1BH
	DEFM	'[01e'
	DEFB	1BH
	DEFM	'[12`'
	DEFB	1BH
	DEFB	'K'
	DEFW	280H
 
C6304	DEFW	90DH
	DEFB	1BH
	DEFB	'J'
	DEFB	24
	DEFB	1BH
	DEFB	'K'
	DEFW	140H
 
V24DC	;Duplex-Init
	;PE: Argumente
	LD	A,(ARGN)
	CP	3
	JR	C,V24DC2;<3 Argumente
	LD	HL,ARG1
	LD	B,M	;ab Platz B
	CALL	V24SU	;M003 suchen
	JP	C,ERRM
	INC	HL
	INC	HL
	LD	A,(HCPZ2)
	BIT	0,M	;SIO A oder B?
	RES	2,A
	JR	NZ,V24DC1
	SET	2,A
V24DC1	LD	B,A
	INC	HL
	INC	HL
	LD	A,M
	SUB	2	;USER1 oder 2?
	RES	1,B
	JR	Z,V24DC6
	SET	1,B
V24DC6	LD	A,B
	LD	(HCPZ2),A
	JR	V24DC3
 
V24DC2	CALL	V24SU	;PROGRAMMFEHLER
	JP	C,ERRM	;(B nicht def.)
V24DC3	LD	DE,V24OT
	LD	BC,V24IN
	LD	A,(HCPZ2)
	BIT	1,A
	JR	NZ,V24DC4
	LD	(UOUT1+1),DE
	LD	(UIN1+1),BC
	JR	V24DC5
 
V24DC4	LD	(UOUT2+1),DE
	LD	(UIN2+1),BC
V24DC5	CALL	LCSOD
	LD	A,0CH
	PUSH	BC
	ADD	C
	LD	B,2	;2 Initbytes
	LD	C,A
	LD	HL,(INTV2)
	DI
	OTIR		;CTC init.
	POP	BC
	LD	A,(INTV2L)
	DEC	A
	DEC	A
	LD	B,A
	LD	A,0AH
	ADD	C
	LD	C,A
UMINI1	OTIR		;SIO init.
	EI
	RET
 
UMINIT	;SIO B auf Interruptbetrieb
	LD	HL,V24OT
	LD	(UOUT2+1),HL
	LD	C,0DH
	LD	HL,V24UMT
	LD	B,2	;2 Initbytes
	DI
	OTIR		;CTC init.
	LD	B,11	;11 Initbytes
	LD	C,0BH
	JR	UMINI1
 
V24INI	;-Tabellen in RAM kopieren
	;-M003 suchen und einschalten
	;-M003-Status in A800h merken
	LD	HL,V24TAB
	LD	DE,0A801H
	LD	BC,17
	LDIR
	LD	B,8
	CALL	V24SU
	RET	C
	LD	A,1
	LD	(0A800H),A
	RET
 
V24TAB	DEFB	47H	;f}r IRM
	DEFB	5BH
	DEFW	0404H
	DEFW	2003H
	DEFW	6A05H
 
	DEFB	47H
	DEFB	2EH
	DEFB	18H
	DEFW	4404H
	DEFW	0E103H
	DEFW	6A05H
 
V24UMT	DEFB	47H	;f}r Interrupt-
	DEFB	2EH	;betrieb
	DEFB	18H
	DEFW	0E202H
	DEFW	4414H
	DEFW	0E103H
	DEFW	0EA05H
	DEFW	1811H
 
GARGC	;bis 10 Argumente erfassen
	LD	BC,ARGN
	XOR	A
	LD	(BC),A	;(ARGN)=0
GARG1	CALL	RHEX
	RET	C	;Fehler
	LD	A,M
	OR	A	;Ende-Dummy?
	RET	Z
	INC	HL
	INC	BC
	LD	A,M	;L(NUMVX)
	LD	(BC),A	;L(Argument)
	INC	HL
	INC	BC
	LD	A,M	;H(NUMVX)
	LD	(BC),A	;H(Argument)
	LD	L,81H	;HL=ARGN
	INC	M
	LD	A,M
	ADD	0F5H	;10 Argumente?
	JR	NC,GARG1
	RET		;Fehler: zu viel
 
MBOC	;Byteweise MB-Ausgabe
	LD	E,D	;Steuerbyte
	LD	D,A	;Datenbyte
	PUSH	DE
	PUSH	BC
	BIT	3,E
	JR	Z,NOINIT
	SET	1,(IX+7);merken INIT
	LD	A,'U'+80H
	CP	M
	PUSH	DE
	LD	DE,0A0H	;Vortonl{nge
	JR	NZ,NOHIU;bei 'U'
	LD	DE,500H	;gedehnt
NOHIU	LD	(VORTN),DE
	LD	DE,0B700H
	LD	BC,11
	LDIR		;Name
	EX	DE,HL
	POP	DE
	LD	M,D	;1. Byte
	LD	A,74H	;noch-Bytes
MBOC1	CALL	SETDT
MBOCE	POP	BC
	POP	DE
	RES	3,E
	RET
 
NOINIT	LD	HL,(DTADR)
	LD	M,D	;Byte eintragen
	BIT	6,E
	JR	NZ,MCLOS
	LD	A,(DTNR);kein Close
	DEC	A
	JR	NZ,MBOC1
	BIT	1,(IX+7);schon Init?
	JR	NZ,MBOC2;nein
	CALL	MBLO	;ja-Blockout
	CALL	SETDTN
	JR	MBOCE
 
MBOC2	RES	1,(IX+7);r}cksetzen
	CALL	ISRO	;Init.
	CALL	MBLNR	;Blocknr.
	CALL	SETDTN
JMBOCE	JR	MBOCE
 
MCLOS	BIT	1,(IX+7);Init?
	JR	Z,MBOC3
	CALL	ISRO	;nein?-aber
			;jetzt!
	JR	MBOC4
 
MBOC3	CALL	MBLO	;Blockout m.Nr.
MBOC4	LD	BC,(VORTN)
	CALL	CSRO
MBOC5	PUSH	DE
	CALL	CSRI
	RES	5,(IX+7);'U'-Merker
	POP	DE	;r}cksetzen
	JR	JMBOCE
 
MBIC	;Byteweise MB-Eingabe
	LD	E,D	;Steuerbyte
	LD	D,A	;Datenbyte (PA!)
	PUSH	DE
	PUSH	BC
	BIT	6,E	;Close?
	JR	NZ,MBOC5
	BIT	3,E	;Init?
	JR	Z,MBIU
	LD	A,1	;erwarteter Blk
	LD	(IX+3),A
	PUSH	HL
	CALL	ISRI
MBIC1	POP	HL
	JP	C,JBLERR;Fehler: '*'
	LD	A,(IX+2);gelesener Blk.
	CP	1
	JP	NZ,JBLERR;Fehler: '*'
	RES	7,(IX+7);Kopfblk geladen
	INC	(IX+3)	;erw. Blk
	LD	DE,0B700H
	LD	B,11
	LD	A,(DE)
	CP	'U'+80H	;hohes 'U'?
	JR	NZ,MBIC2
	SET	5,(IX+7);merken zur
 ;Blocknummernausgabeunterdr}ckung
MBIC2	ADD	29H	;gesch}tzte
	JR	NC,MBIC3;BASIC-
	LD	A,(DE)	;Programme
	SUB	4	;als un-
	LD	(DE),A	;gesch}tzt
	INC	DE	;anzeigen
	LD	(DE),A	;und Setzen
	INC	DE	;LIST/EDIT/BYE-
	LD	(DE),A	;Schutz
	LD	(35EH),A;<=hier
	SET	6,(IX+7);merken
	LD	DE,0B700H ;(f}r wen?)
MBIC3	LD	A,(DE)	;Name
	CALL	CRT	;anzeigen
	CP	M	;stimmt Name?
	JR	Z,MBIC4
	SET	7,(IX+7);nein-merken
MBIC4	INC	HL
	INC	DE
	DJNZ	MBIC3
	CALL	CRLF
	BIT	7,(IX+7)
	JP	NZ,JIOERR ;zu ?IO ERROR
	EX	DE,HL
	LD	A,75H	;noch-Bytes
MBIC5	LD	D,M	;auslesen
	CALL	SETDT
	LD	A,D
	POP	BC
	POP	DE
	RES	3,E
	RET
 
MBIU	LD	HL,(DTADR)
	LD	A,(DTNR)
	DEC	A
	JR	NZ,MBIC5
MBIU1	CALL	MBI
	JR	C,MERR
	CALL	SETDTN
	LD	A,0FFH
	CP	(IX+2)	;Block FF?
	JR	Z,MBIU3
	LD	A,(IX+3)
	CP	(IX+2)	;erwarteter?
	JR	Z,MBIU3
	LD	A,'*'
	CALL	CRT	;nicht
	CALL	BNROST	;erwartet
	DEFB	19H
	NOP
MBIU2	CALL	JBRKT
	JR	MBIU1
 
MBIU3	INC	(IX+3)	;n{chsten
	DEC	A	;erwarten
	BIT	5,(IX+7)
	JR	NZ,MBIU4
	CALL	BNROST	;Blocknummer
	DEFB	'>'	;unter Vorbe-
	DEFB	19H	;halt ausgeben
	NOP
MBIU4	CALL	SETDTN
	LD	HL,(DTADR)
	JR	MBIC5
 
MERR	CALL	OSTR	;Fehler im
	DEFW	909H	;Block
	DEFW	909H
	DEFB	'?'
	NOP
	CALL	BNROST
	DEFW	0A0DH
	NOP
	JR	MBIU2
 
JBLERR	CALL	OSTR	;nicht erwarte-
	DEFM	'*'	;ter 1. Block
	DEFB	8
	NOP
	CALL	JBRKT
	CALL	MBI
	PUSH	HL
	JP	MBIC1
 
MBLO	LD	BC,(VORTN)
	CALL	MBO
MBLNR	CALL	BNROST
	DEFB	'>'
	DEFB	19H
	NOP
JBRKT	CALL	BRKT	;bei BRK
	RET	NC	;auch ?IO ERROR
	JP	JIOERR
 
SETDTN	;Setzen Datenzeiger neu
	LD	A,80H
	LD	HL,0B700H
	JR	SETD1
 
SETDT	;N{chsten Datenzeiger setzen
	INC	HL
SETD1	LD	(DTADR),HL
	LD	(DTNR),A
	RET
 
	END

