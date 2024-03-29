CE31    ASM                                                                                                                      
;*********************************
;**				**
;**	CAOS 3.1 reassembliert	**
;**		 von ML-Soft	**
;**				**
;**	14.05.95 bis 21.05.95	**
;**				**
;*********************************
 
;	Speicher}bersicht	**
;	_________________
 
 
;C000	BASIC-ROM
 
;------------------------------------
 
;E000	RESET und BASIC-Verteiler
;E024	DABR
;E055	Zeichenausgabe WPIX
;E0BF	PADR
;E13A	Punktroutinen PUDE, PUSE
;E189	KBD-Treiber
;E213	Tastaturtabelle KTAB
;E293	Interrupttabelle ISRTAB
;E29F	KBDZ, KBDS, INIEA, INIME, ERAM
;E2CF	SWITCH und MODUL
;E347	JUMP
;E36A	Tape-Treiber
;E527	TON-Ausgabe
;E581	BASIC-Token
;E62B	Sprungtabelle f. BASIC-Token
;E669	BASIC-Extension
;EE00	Zeichenbildtab. Gro~buchst. *)
 
;------------------------------------
 
;F000	PWRON und Programm-Verteiler
;F109	Kommandointerpreter (MENU)
;F1C6	Ein-/Ausgabe OSTR, INTB, OCHR
;F210	KBD, IRM-Default, INLIN
;F2C0	Hexzahlenverarbeitung
;F357	Systeminit, SIXD, Portinittab.
;F3D5	SAVE
;F43E	BRKT, LARG
;F45B	VERIFY, LOAD, COLOR
;F5D4	Unterprogrammtabelle SUTAB
;F662	LDMA, LDAM, MODIFY
;F6E3	Sprungtabelle f}r CRT-Routinen
;F707	CCTL-Belegungstabelle CCTAB
;F725	CRT-Treiber
;F8BE	Bildschirm-Steuercodes CRTTAB
;F8D0	FADR,
;F8E7	BASIC-I/O-Verteiler
;F956	MBOUT, MBIN
;FAFB	DISPLAY
;FB69	CSTBT, KEYLIST, KEY
;FC1A	WININ, WINAK
;FC82	LINE, CIRCLE, SQR, MULT
;FE00	Zeichenbildtab. Kleinbuchst. *)
;------------------------------------
 
;*) nicht in diesem Listing aufgef}hrt
 
;**	EQU-Liste		**
 
LF	EQU	0AH
CR	EQU	0DH
 
VRAM	EQU	0B200H	;VIDEO-RAM
CASS	EQU	0B700H	;Kass.-Puffer
ARGC	EQU	0B780H	;UP-Nr. (PV2)
ARGN	EQU	0B781H	;Anz. Argumente
ARG1	EQU	0B782H	;1. Argument
ARG2	EQU	0B784H	;2. Argument
ARG3	EQU	0B786H	;3. Argument
ARG4	EQU	0B788H	;4. Argument
 
NUMNX	EQU	0B796H	;Zeichenanzahl
NUMVX	EQU	0B797H	;Wert der Zahl
HCADR	EQU	0B799H	;Hardcopy-Adr.
WINNR	EQU	0B79BH	;Fensternummer
WINON	EQU	0B79CH	;Fensteranfang
WINLG	EQU	0B79EH	;Fensterl{nge
CURSO	EQU	0B7A0H	;Cursorposition
STBT	EQU	0B7A2H	;BS-Steuerbyte
COLOR	EQU	0B7A3H	;Zeichenfarbe
WEND	EQU	0B7A4H	;PAGE/SCROLL
CCTL0	EQU	0B7A6H	;Zeichentab. 0
CCTL1	EQU	0B7A8H	;Zeichentab. 1
CCTL2	EQU	0B7AAH	;Zeichentab. 2
CCTL3	EQU	0B7ACH	;Zeichentab. 3
SYSP	EQU	0B7AEH	;System-Stack
SUTAB	EQU	0B7B0H	;UP-Tabelle
CTAB	EQU	0B7B2H	;Adresse CRTTAB
BJUMP	EQU	0B7B4H	;JUMP-Adresse
OUTAB	EQU	0B7B9H	;Ausgabe-UP
INTAB	EQU	0B7BBH	;Eingabe-UP
UOUT1	EQU	0B7BDH	;USER-out #1
UIN1	EQU	0B7C0H	;USER-in #1
UOUT2	EQU	0B7C3H	;USER-out #2
UIN2	EQU	0B7C6H	;USER-in #2
IOERR	EQU	0B7C9H	;-> ?IO ERROR
 
ZWEND	EQU	0B7CFH	;WEND-Merker
FTAST	EQU	0B7D1H	;F-Tastenzeiger
HOR	EQU	0B7D3H	;hor. Koordin.
VERT	EQU	0B7D5H	;vert. Koordin.
FARB	EQU	0B7D6H	;Grafikfarbe
MIXIT	EQU	0B7D7H	;IX-Bereich
ZEITB	EQU	0B7D8H	;Zeichentabelle
VORTN	EQU	0B7DAH	;Vortonl{nge
DTADR	EQU	0B7DCH	;Datenzeiger
DTNR	EQU	0B7DEH	;Datennummer
XSTEP	EQU	0B7DFH	;X-Schrittma~
YSTEP	EQU	0B7E1H	;Y-Schrittma~
XCNT	EQU	0B7E3H	;X-Z{hler
YCNT	EQU	0B7E5H	;Y-Z{hler
XDIR	EQU	0B7E7H	;X-Richtung
YDIR	EQU	0B7E9H	;Y-Richtung
 
WNDFN	EQU	0B99CH	;Fenstervektoren
 
 
 
 
 
 
;**	CAOS 3.1 ROM E		**
 
	ORG	0E000H
;
BYE	JP	RESET	;Tasten-RESET
	JP	BEXP1	;BASIC-
	JP	BEXP2	;Expansionen
	JP	BEXP3
	DW	TOKTAB
BASIO	JP	BASPV
 
RESET	;Tasten-RESET
	LD	A,(IX+7)
	AND	0C0H	;RESET-Schutz?
	JR	Z,BYE1
	CALL	ERAM4	;RAM4 l|schen!
	JP	POWER
	;
BYE1	LD	SP,1C4H
	JP	PWR2
 
DABR	PUSH	DE		;**32**
	PUSH	AF
	CALL	TCIF
	JR	C,IAD2
	LD	A,(WINON+1)
	ADD	D	;Cursor-Zeile
	LD	H,0
	LD	L,A
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	PUSH	BC
	PUSH	HL
	POP	BC
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,BC
	LD	A,(WINON)
	ADD	E	;Cursor-Spalte
	ADD	L
	LD	L,A
	LD	A,0
	ADC	H
	LD	H,A
	LD	BC,VRAM
	ADD	HL,BC
	POP	BC
	POP	AF
	AND	A
	POP	DE
	RET
IAD2	POP	AF
	SCF
	POP	DE
	LD	HL,VRAM
	RET
 
WPIX	;Zeichen auf Grafikbildschirm
	;sichtbar machen, PE: DE,A
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	PUSH	AF
	CALL	FADR	;Farbadresse
	POP	AF
	PUSH	HL	;wof}r?
	CALL	PADR
	JR	C,POP4	;au~erhalb!
	PUSH	HL	;Pixeladresse
	LD	HL,CCTAB
	LD	C,0B7H	;High(CCTLx)
WPIX1	CP	M
	INC	HL
	JR	NC,WPIX2
	INC	HL
	INC	HL
	JR	WPIX1
WPIX2	SUB	M	;Offset
	INC	HL
	LD	L,M
	LD	H,C	;HL=CCTLx
	LD	C,M
	INC	HL
	LD	H,M
	LD	L,C	;Zeichentabelle
	LD	(ZEITB),HL
	LD	C,A
	LD	A,(STBT)
	LD	B,A
	BIT	1,B
	JR	NZ,WPIX3;Farbe aus
	LD	A,(COLOR)
	LD	(DE),A	;oberes Byte
	SET	5,E
	LD	(DE),A	;unteres Byte
WPIX3	POP	DE	;Pixeladr.
	POP	HL	;Farbadr.
	BIT	0,B
	JR	NZ,POP4	;Pixel aus
	LD	B,0
	LD	A,3
WPIX4	SLA	C
	RL	B	;*8
	DEC	A
	JR	NZ,WPIX4
	LD	HL,(ZEITB)
	ADD	HL,BC	;Zeichenadr.
	PUSH	DE
	CALL	WPIX5	;4 Byte schreib.
	POP	DE
	SET	5,E
	CALL	WPIX5	;4 Byte schreib.
POP4	POP	AF
	JP	POP3
	;
WPIX5	LD	BC,80H	;Offset f}r
	LD	A,4	;n{chstes Byte
WPIX6	LDI
	EX	DE,HL
	ADD	HL,BC
	EX	DE,HL
	INC	BC	;BC korrigieren
	DEC	A
	JR	NZ,WPIX6
	RET
 
PADR	;Pixel- und Farbadr berechnen
	PUSH	AF		;**34**
	LD	A,L	;Spalte
	CP	40
	JR	NC,PADR5;zu gro~
	PUSH	BC
	CP	32
	JR	NC,PADR6;rechte Seite
	LD	A,H
	RLCA
	RLCA
	RLCA
	AND	60H
	OR	L
	BIT	0,H
	JR	Z,PADR1
	SET	7,A
PADR1	LD	C,A
	LD	A,H
	RRA
	RRA
	RRA
	AND	1EH
	BIT	1,H
	JR	Z,PADR2
	SET	0,A
PADR2	OR	80H	;Pixel-RAM I
	LD	B,A
	RRA
	RRA
	AND	7
	OR	0A8H	;Color-RAM I
	LD	D,A
	LD	A,C
	AND	7FH
	BIT	4,H
	JR	Z,PADR3
	SET	7,A
PADR3	LD	E,A
PADR4	LD	L,C
	LD	H,B
	POP	BC
	POP	AF
	OR	A	;CY=0
	RET
	;
PADR5	POP	AF
	SCF		;au~erhalb
	RET
	;
PADR6	AND	7
	LD	L,A
	LD	A,H
	RLCA
	RLCA
	RLCA
	AND	60H
	OR	L
	LD	L,A
	LD	A,H
	RRA
	AND	18H
	BIT	0,H
	JR	Z,PADR7
	SET	7,A
PADR7	OR	L
	LD	C,A
	LD	A,H
	RLCA
	RLCA
	RLCA
	AND	6
	OR	0A0H	;Pixel-RAM II
	BIT	1,H
	JR	Z,PADR8
	SET	0,A
PADR8	LD	B,A
	LD	D,0B0H	;Color-RAM II
	BIT	2,A
	JR	Z,PADR9
	INC	D
PADR9	LD	A,C
	AND	7FH
	BIT	1,B
	JR	Z,PADR10
	SET	7,A
PADR10	LD	E,A
	JR	PADR4
 
PUDE	;Punkt l|schen		;**2F**
	SCF
	JR	PU1
PUSE	;Punkt setzen		;**30**
	AND	A
PU1	PUSH	HL
	PUSH	DE
	PUSH	BC
	LD	HL,(HOR)
	LD	BC,(VERT)
	PUSH	AF
	XOR	A
	LD	B,3
PU2	SRL	H
	RR	L
	RRA
	DJNZ	PU2
	LD	E,A
	LD	A,C
	CPL
	LD	H,A
	LD	C,E
	CALL	PADR
	JR	C,PU5
	SCF
	LD	A,C
	LD	C,0
PU3	RR	C
	SUB	20H
	JR	NC,PU3
	POP	AF
	LD	A,C
	JR	C,PU6
	OR	M
	LD	M,A
	LD	A,(FARB)
	AND	0F8H
	LD	B,A
	LD	A,(DE)
	AND	7
	OR	B
	LD	(DE),A
PU4	JP	POP3
PU5	POP	AF
	SCF
	JR	PU4
PU6	PUSH	AF
	LD	B,M
	CPL
	AND	M
	LD	M,A
	POP	AF
	AND	B
	LD	A,(DE)
	JR	PU4
 
;KBD-Treiber
 
ISRC3	;ISR CTC Kanal 3 (Tastatur)
	EI
	PUSH	AF
	LD	A,23H	;DI,ZG256,Res
	OUT	8FH	;CTC K3
	SET	3,(IX+8);Timeout
	LD	(IX+13),0 ;Zeichen tot
	JR	TST4
 
ISRPB	;ISR PIO Kanal B (Tastatur)
	EI
	PUSH	AF
	IN	A,8FH	;gemessene
	PUSH	AF	;Zeit retten
	LD	A,0A7H	;EI,ZG256,Res
	OUT	8FH
	LD	A,8FH	;ZK
	OUT	8FH
	POP	AF
	BIT	3,(IX+8);Timeout?
	RES	3,(IX+8);r}cksetzen
	JR	NZ,TST4
	CP	101
	JR	NC,TST5	;0-Bit
	CP	68
	JR	NC,TST6 ;1-Bit
	PUSH	HL
	PUSH	DE
	SRL	(IX+12)
	LD	A,(IX+8)
	AND	80H	;CAPS?
	RLCA
	XOR	(IX+12)	;7bit-Scancode
	LD	H,0
	LD	E,(IX+14)
	LD	D,(IX+15)
	LD	L,A
	ADD	HL,DE	;Pos. in KTAB
	LD	A,M	;ASCII holen
	POP	DE
	POP	HL
	CP	(IX+13)	;=letzter Code?
	JR	Z,TST2
	LD	(IX+13),A ;eintragen
	RES	4,(IX+8);Repeat r}cks.
TST1	SET	0,(IX+8);g}ltig machen
	LD	(IX+10),0 ;neuer Code
	JR	TST4
	;
TST2	INC	(IX+10)	;Zeit abwarten
	BIT	4,(IX+8);Fast Repeat?
	JR	NZ,TST3
	BIT	4,(IX+10) ;16 erreicht?
	JR	Z,TST4
	SET	4,(IX+8);Repeat aktiv
	JR	TST1
	;
TST3	BIT	1,(IX+10) ;nur jeder 2.
	JR	NZ,TST1	  ;Interrupt!
TST4	POP	AF
	RETI
	;
TST5	OR	A	;0-Bit
	JR	TST7
TST6	SCF		;1-Bit
TST7	RR	(IX+12)	;einschieben
	JR	TST4
 
 
KTAB	DB	'Ww'
	DB	'Aa'
	DB	'2"'
	DB	8,19H		;CUL
	DB	10H,0CH		;HOME
	DB	'-='
	DB	0F2H,0F8H	;F2
	DB	'Yy'
	DB	'Ee'
	DB	'Ss'
	DB	'3#'
	DB	'^]'
	DB	1,0FH		;CLR
	DB	':*'
	DB	0F3H,0F9H	;F3
	DB	'Xx'
	DB	'Tt'
	DB	'Ff'
	DB	'5%'
	DB	'Pp'
	DB	1FH,2		;DEL
	DB	'0',40H
	DB	0F5H,0FBH	;F5
	DB	'Vv'
	DB	'Uu'
	DB	'Hh'
	DB	'7',27H
	DB	'Oo'
	DB	1AH,14H		;INS
	DB	'9)'
	DB	3,3		;BRK
	DB	'Nn'
	DB	'Ii'
	DB	'Jj'
	DB	'8('
	DB	' ['		;SPC
	DB	'Kk'
	DB	',<'
	DB	13H,13H		;STOP
	DB	'Mm'
	DB	'Zz'
	DB	'Gg'
	DB	'6&'
	DB	'  '	;freie Taste
	DB	'Ll'
	DB	'.>'
	DB	0F6H,0FCH	;F6
	DB	'Bb'
	DB	'Rr'
	DB	'Dd'
	DB	'4$'
	DB	'_\'
	DB	'+;'
	DB	'/?'
	DB	0F4H,0FAH	;F4
	DB	'Cc'
	DB	'Qq'
	DB	16H,16H		;CAPS
	DB	'1!'
	DB	LF,12H		;CUD
	DB	0BH,11H		;CUU
	DB	09H,18H		;CUR
	DB	0F1H,0F7H	;F1
	DB	CR,CR		;ENTER
 
ISRTAB	DW	ISRPA	;PIO A:Kassette
	DW	ISRPB	;PIO B:Tastatur
	DW	0	;CTC 0:frei
	DW	ISRC1	;CTC 1:Kassette
	DW	ISRC2	;CTC 2:Tondauer
	DW	ISRC3	;CTC 3:Tastatur
 
KBDZ	;Abfrage mit Quittung	;**0E**
	CALL	KBDS
	RET	NC
	RES	0,(IX+8)
	RET
 
KBDS	;Abfrage ohne Quittung	;**0C**
	OR	A
	BIT	0,(IX+8);Code g}ltig?
	RET	Z
	LD	A,(IX+13)
	SCF		;wenn ja
	RET
 
INIEA	PUSH	BC		;**43**
	LD	C,M	;1.Byte=Adr.
	INC	HL
	LD	B,M	;2.Byte=Anz.
	INC	HL
	OTIR
	POP	BC
	RET
 
INIME	CALL	INIEA		;**44**
	DEC	D	;D Kan{le
	JR	NZ,INIME
	RET
 
ERAM4	;RAM l|schen
	LD	HL,4000H;RAM4
	LD	B,H
	LD	C,L
ERAM	LD	A,L
ERA4	LD	M,A	;l|schen
	CPI		;INC HL, DEC BC
	RET	PO	;fertig
	JR	ERA4
 
 
	DW	7F7FH
	DB	'SWITCH',1
	LD	A,(ARGN)
	LD	D,E
	CALL	MODU
	LD	A,L	;Platz
	CALL	AHEX
	CALL	SPACE
	LD	A,H	;Kennbyte
	CALL	AHEX
	CALL	SPACE
	LD	A,E	;Steuerbyte
	CALL	AHEX
	JP	CRLF
 
MODU	;Lesen und Schalten	;**26**
	CP	2
	JR	C,MODU5	;nur lesen
	LD	A,L
	CP	3
	JR	NC,MODU4
	LD	C,88H	;interne Module
	IN	H,(C)
	AND	A
	JR	NZ,MODU2
	BIT	0,D	;f}r RAM0
	RES	1,H
	JR	Z,MODU1
	SET	1,H
MODU1	BIT	1,D
	RES	3,H
	JR	Z,MODU3
	SET	3,H
	JR	MODU3
 
MODU2	DEC	A
	JR	NZ,MODU7
	BIT	0,D	;f}r IRM
	RES	2,H
	JR	Z,MODU3
	SET	2,H
MODU3	OUT	(C),H	;int. schalten
MODU4	LD	A,D
	LD	C,80H
	LD	B,L
	OUT	(C),A	;Senden Steuerb.
MODU5	LD	H,0
	PUSH	HL
	PUSH	AF
	LD	BC,0B800H
	ADD	HL,BC
	POP	AF
	JR	C,MODU6	;nur lesen
	LD	M,A	;eintragen
MODU6	LD	E,M	;R}ckgabe STB
	POP	HL	;(f}r Kommando)
	LD	C,80H
	LD	B,L
	IN	H,(C)	;Modultyp lesen
	RET
 
MODU7	BIT	0,D	;f}r BASIC-ROM
	RES	7,H
	JR	Z,MODU3
	SET	7,H
	JR	MODU3
 
	DW	7F7FH
	DB	'JUMP',1
	LD	A,L
JUMP	LD	B,A		;**27**
	LD	C,80H
	IN	A,(C)
	INC	A	;Kennbyte FF?
	JP	Z,LOOP2	;ja-Fehler
	LD	A,0FFH
	OUT	(C),A	;Ausgabe FF
	LD	H,0
	LD	DE,0B800H
	ADD	HL,DE
	LD	M,A	;Eintrag
	IN	A,88H
	AND	7EH
	JP	BJUMP	;in IRM
 
 
;TAPE-Treiber
 
ISRO	;Init Bandausgabe	;**08**
	LD	L,60H	;Motor, LED ein
	CALL	ISRI1
	LD	(IX+2),0 ;1. Block
	LD	BC,2000H ;Vortonl{nge
	;
MBO	;Ausgabe 1 Block	;**01**
	INC	(IX+2)
	DI
	XOR	A
	LD	(IX+1),A;Pr}fsumme
	LD	A,87H	;EI,ZG16,Res
	OUT	8DH
	LD	A,2FH	;Vorton
	OUT	8DH
	EI
	LD	D,A	;D=2Fh
MBO1	CALL	HBITOT
	CPI
	JP	PE,MBO1	;BC mal
	CALL	ZTON	;Trennzeichen
	LD	A,(IX+2)
	CALL	BYTOT	;Blocknummer
	LD	L,(IX+5)
	LD	H,(IX+6)
	LD	B,80H
BLKOT	LD	A,M
	CALL	BYTOT	;Datenbyte
	LD	A,(IX+1)
	ADD	M	;Pr}fsumme
	LD	(IX+1),A;berechnen
	INC	HL
	DJNZ	BLKOT
	CALL	BYTOT	;und ausgeben
	LD	A,D
	CALL	BITO1	;noch ein Knacks
	LD	A,3
	OUT	8DH	;CTC stoppen
	RET
 
BYTOT	;Ausgabe eines Bytes
	PUSH	BC
	LD	C,A
	LD	B,8
BYTOUT	RRC	C	;mit Bit0
	PUSH	AF	;beginnend
	CALL	C,HBITOT
	POP	AF
	CALL	NC,LBITOT
	DJNZ	BYTOUT	;8*
	POP	BC
ZTON	LD	E,5DH	;ZK Trennz.
	JR	BITOUT
 
LBITOT	LD	E,17H	;ZK High-Bit
	JR	BITOUT
 
HBITOT	LD	E,2FH	;ZK Low-Bit
BITOUT	LD	A,D
	CALL	BITO1	;aufrufen und
	LD	A,D	;reinlaufen
BITO1	LD	(IX),A
BITO2	LD	A,(IX)	;Warten auf
	OR	A	;Interrupt
	JR	NZ,BITO2
	LD	D,E
	RET
 
ISRI1	DI
	IN	A,88H
	OR	L	;schalten
	OUT	88H
	EI
	LD	HL,(WEND)
	LD	(ZWEND),HL
	JP	PAGE
 
CSRO	;Abschlu~ Bandausgabe	;**09**
	LD	(IX+2),0FEH
	CALL	MBO
CSROI	LD	HL,CASS	;Kassettenpuffer
	LD	BC,7EH
	CALL	ERAM	;l|schen
	LD	HL,(ZWEND)
	LD	(WEND),HL
	IN	A,88H
	AND	9FH	;Motor, LED aus
	OUT	88H
	JP	INIT	;Ports init
 
ISRI	;Init. Bandeingabe	;**0A**
	LD	L,40H	;Motor ein
	CALL	ISRI1
	CALL	MBI0	;Block lesen
	RET	C	;Fehler
	LD	DE,COM
	LD	H,(IX+6)
	LD	A,(IX+5)
	ADD	8
	LD	L,A
	LD	B,4
ISRI2	LD	A,(DE)
	SUB	M
	AND	A
	INC	HL
	INC	DE
	RET	NZ
	DJNZ	ISRI2
	SET	7,(IX+7);COM-File
	RET
 
MBI	;Einlesen 1 Block	;**05**
	BIT	7,(IX+7)
	SCF		;kein COM-File!
	RET	NZ
MBI0	LD	A,5	;DI,ZG16
	OUT	8EH	;CTC K2
	LD	A,83H	;EI
	OUT	8AH	;an PIO A
	PUSH	HL
	PUSH	DE
	LD	A,0A3H	;ZK
	OUT	8EH
	EI
MBI1	LD	B,16H
	XOR	A
	LD	(IX+1),A ;Pr}fsumme=0
MBI2	CALL	STOP1
	JR	C,MBI1
	CP	0BAH
	CALL	LEDOO
	JR	C,MBI1	;10h korrekte
	DJNZ	MBI2	;Schwingungen
MBI3	LD	B,2	;erkennen
MBI4	XOR	A
	LD	C,A
	LD	(IX),A
	CALL	STOP21	;2 halbe Trenn-
	CP	5DH	;zeichen er-
	JR	NC,MBI3	;kennen
	DJNZ	MBI4
	CALL	BYTIN	;Blocknummer
	CALL	C,LEDOO
	LD	(IX+2),A
	LD	B,80H
	LD	L,(IX+5)
	LD	H,(IX+6)
BLKIN	CALL	BYTIN	;Datenbyte
	CALL	C,LEDOO
	LD	M,A
	LD	A,(IX+1)
	ADD	M	;aufsummieren
	LD	(IX+1),A
	INC	HL
	DJNZ	BLKIN
	CALL	BYTIN	;Pr}fsumme
	CALL	C,LEDOO
	LD	B,A
	LD	A,(IX+1)
	CP	B	;gleich?
	POP	DE
	POP	HL
	LD	A,3	;DI an PIO-A
	OUT	8AH
	JR	Z,LEDOO
	SCF		;St|rung
LEDOO	PUSH	AF
	IN	A,88H
	SET	5,A	;LED ein
	JR	NC,MBI5
	RES	5,A	;LED aus
MBI5	OUT	88H
	POP	AF
	RET
 
COM	DB	'COM',1
 
ISRC1	;ISR CTC Kanal 1 (Kassette-Out)
	PUSH	AF
	LD	A,3	;DI,ZG16,Res
	OUT	8DH
	LD	A,87H	;EI,ZG16,Res
	OUT	8DH
	LD	A,(IX)	;Uebergabezelle
	OUT	8DH	;ZK
	XOR	A
	LD	(IX),A	;Quittierung
	JR	ISR2E
 
ISRPA	;ISR PIO Kanal A (Kassette-In)
	PUSH	AF
	IN	A,8EH
	LD	(IX),A	;Uebergabezelle
	LD	A,7
	OUT	8EH
	LD	A,0A3H
ISR1E	OUT	8EH
ISR2E	POP	AF
	EI
	RETI
 
ISRC2	;ISR CTC Kanal 2 (Tondauer)
	PUSH	AF
	LD	A,3	;Ton aus
	OUT	8CH	;CTC0
	OUT	8DH	;CTC1
	IN	A,89H
	SET	7,A	;Blinken ein
	OUT	89H
	LD	A,47H	;CTC2 auf
	OUT	8EH	;Blinken stellen
	LD	A,20H	;ZK
	RES	1,(IX+8);Ton ist aus
	JR	ISR1E
 
STOP1	XOR	A	;1 Bit einlesen
	LD	(IX),A
STOP11	LD	A,(IX)	;Warten auf
	OR	A	;Interrupt
	JR	Z,STOP11
	LD	C,A
	XOR	A
	LD	(IX),A
STOP21	LD	A,(IX)
	OR	A
	JR	Z,STOP21
	ADD	C	;beide 'Zeiten'
	RET		;addieren
 
BYTIN	;Einlesen eines Bytes
	LD	D,8	;8 Bit
	XOR	A
	LD	E,A
TRN1	CALL	STOP1	;Bit einlesen
	CCF
	JR	NC,TRN2
	CP	0BAH	;zu intolerant
	RET	C
	SCF
TRN2	RR	E	;Bit einschieben
	DEC	D
	JR	NZ,TRN1	;8*
	CALL	STOP1	;Trennzeichen
	LD	A,E	;mit CY-
	RET		;R}ckmeldung
 
TON	CALL	LARG		;**35**
TON1	BIT	1,(IX+8);alten Ton
	JR	NZ,TON1	;abwarten
	LD	A,L
	AND	A
	LD	A,3
	JR	Z,TON3	;kein Ton1
	LD	A,7
	BIT	0,H
	JR	Z,TON2	;VT16
	OR	20H	;VT256
TON2	OUT	8CH
	LD	A,L	;ZK1
TON3	OUT	8CH
	LD	A,E
	AND	A
	LD	A,3
	JR	Z,TON5	;kein Ton2
	LD	A,7
	BIT	0,D
	JR	Z,TON4	;VT16
	OR	20H	;VT256
TON4	OUT	8DH
	LD	A,E	;ZK2
TON5	OUT	8DH
	LD	A,C	;Lautst{rke
	XOR	1FH	;da nullaktiv
	AND	1FH	;maskieren
	SET	7,A	;Blinken ein
	LD	C,A
	LD	A,B
	AND	A
	JR	Z,TON6	;Dauerton
	RES	7,C	;Blinken aus
	SET	1,(IX+8);neuer Ton
	LD	A,0C7H	;EI, Z{hler 50Hz
	OUT	8EH	;CTC2
	LD	A,B	;Dauer
	OUT	8EH
TON6	LD	B,60H
TONB	IN	A,89H
	AND	B	;maskieren
	OR	C
	OUT	89H	;Ltst. ausgeben
	RET
 
ZKOUT	LD	A,M		;**45**
	INC	HL
	AND	A
	RET	Z
	CALL	OCHR
	JR	ZKOUT
 
TOKTAB	;BASIC-Token
	;(Bit 7 im 1. Byte gesetzt!)
	DB	'�NKEY$'
	DB	'�OYST'
	DB	'�TRING$'
	DB	'�NSTR'
	DB	'�ENUMBER'
	DB	'�ELETE'
	DB	'�AUSE'
	DB	'�EEP'
	DB	'�INDOW'
	DB	'�ORDER'
	DB	'�NK'
	DB	'�APER'
	DB	'�T'
	DB	'�OLOR'
	DB	'�OUND'
	DB	'�SET'
	DB	'�RESET'
	DB	'�LOAD'
	DB	'�PEEK'
	DB	'�POKE'
	DB	'�OCATE'
	DB	'�EYLIST'
	DB	'�EY'
	DB	'�WITCH'
	DB	'�TEST'
	DB	'�LOSE'
	DB	'�PEN'
	DB	'�ANDOMIZE'
	DB	'�GET$'
	DB	'�INE'
	DB	'�IRCLE'
	DB	'�SRLIN'
	DB	80H
 
TOKJP	;Sprungtabelle f}r BASIC-Token
	DW	INKEY
	DW	02FDH	;JOYST
	DW	STRING
	DW	INSTR
	DW	RENUM
	DW	DELETE
	DW	PAUSE
	DW	BBEEP
	DW	WINDOW
	DW	BORDER
	DW	INK
	DW	PAPER
	DW	0C348H	;AT
	DW	BCOLOR
	DW	SOUND
	DW	PSET
	DW	PRESET
	DW	BLOAD
	DW	0C348H	;VPEEK
	DW	VPOKE
	DW	LOCATE
	DW	KEYLIST
	DW	BKEY
	DW	SWITCH
	DW	0C348H	;PTEST
	DW	CLOSE
	DW	OPEN
	DW	RANDOM
	DW	0C348H	;VGET
	DW	BLINE
	DW	CIRCLE
 
BEXP1	;BASIC-Exp 1
	LD	A,B
	SUB	9
	JR	C,SNERR1
	CP	27
	JR	NC,SNERR1	;>26
	RLCA		;*2
	LD	C,A
	LD	B,0
	EX	DE,HL
	LD	HL,TOKJP+8
	JP	0C8B7H	;R}cksprung
 
BEXP2	;BASIC-Exp 2
	LD	A,M
	CP	0DFH
	RET	C		;<DFH
	CP	0E3H
	RET	NC		;>E2H
	CP	0E1H	;AT?
	JP	Z,PRAT
	LD	A,(3FDH)
	AND	A	;PRINT-Erw.?
	JR	NZ,SNERR1
	INC	A
	LD	(3FDH),A
PREX1	PUSH	HL
	LD	HL,COLOR
	CALL	IRMRD
	POP	HL
	LD	(37EH),A;Farbe merken
	LD	A,M
	CP	0DFH	;INK?
	JR	Z,PRINK
	CP	0E2H	;COLOR?
	JP	Z,PRCOL
	CALL	0C8BDH
	CALL	PAPER
PREX2	LD	A,M
	CP	';'
	JR	Z,PREX5
	JR	SNERR1
 
PRCOL	CALL	0C8BDH
	CALL	BCOLOR
	JR	PREX2
 
SNERR1	JP	0C348H	;SN-ERROR
 
PRINK	CALL	0C8BDH
	CALL	INK
	LD	A,M
	CP	';'
	JR	Z,PREX5
	CALL	0C8CCH	;Komma?
	DB	','
	CP	0E0H	;PAPER?
	JR	NZ,SNERR1
	CALL	0C8BDH
	CALL	PAPER
	CALL	0C8CCH
	DB	';'
PREX3	CALL	0CB03H
	LD	A,(37EH);Farbe
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMWR	;eintragen
	POP	HL
	POP	BC
	RET
PREX4	PUSH	BC
	JR	PREX1
PREX5	CALL	0C8BDH
	JR	PREX3
 
BEXP3	;BASIC-Exp 3
	LD	A,C
	CP	62H
	JP	Z,VPEEK
	CP	6EH
	JP	Z,PTEST
	CP	7CH
	JP	Z,CSRLN
	CP	76H
	JP	Z,VGET
	SUB	3EH
	JR	C,SNERR1
	CP	7
	JR	NC,SNERR1
	EX	DE,HL
	LD	BC,TOKJP ;Tokentab.
	POP	HL
	LD	L,A
	ADD	HL,BC
	LD	C,M
	INC	HL
	LD	H,M
	LD	L,C
	PUSH	HL	;Routine
	EX	DE,HL	;anspringen
	RET
 
STRING	;String vervielf{ltigen
	CALL	0C8CCH
	DB	'('	;Klammer auf?
	CALL	0D421H
	PUSH	AF
	CALL	0C8D6H	;Komma?
	CALL	0CD3AH
	CALL	0C8DBH	;Klammer zu?
	POP	AF
	PUSH	HL
	PUSH	AF
	CALL	0D330H
	INC	HL
	INC	HL
	LD	E,M
	INC	HL
	LD	D,M
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
STR1	ADD	C
	JR	C,STR5
	DJNZ	STR1
STR2	LD	B,A
	LD	C,0
	PUSH	BC
	CALL	0D1E1H	;Str.-Arithm.
	POP	BC
	POP	BC
	PUSH	BC
	CALL	0D17EH
	POP	HL
	EX	(SP),HL
	LD	A,H
	POP	HL
	EX	(SP),HL
	LD	L,A
	INC	H
STR3	DEC	H
	PUSH	HL
	PUSH	BC
	JR	Z,STR4
	CALL	0D2F2H
	POP	BC
	POP	HL
	JR	STR3
STR4	POP	BC
	POP	HL
	POP	DE
	CALL	0D302H
	JP	0D1A9H
STR5	LD	E,1CH	;ST-
	JP	0C356H	;ERROR
 
RENUM	;neu nummerieren
	PUSH	HL
	LD	HL,10
	LD	(354H),HL	;DISTAN
	LD	HL,(35FH)	;Start
	PUSH	AF
	PUSH	HL
	INC	HL
	INC	HL
	LD	A,M
	INC	HL
	LD	H,M
	LD	L,A
	LD	(34EH),HL	;ZL-Nr
	LD	(352H),HL	;NANF
	LD	DE,(3D7H)	;Ende
	DEC	DE
	DEC	DE
REN1	POP	HL
	PUSH	HL
	LD	A,M
	INC	HL
	LD	H,M
	LD	L,A
	CALL	0C689H
	EX	(SP),HL
	JR	NZ,REN1
	POP	DE
	INC	HL
	INC	HL
	LD	A,M
	INC	HL
	LD	H,M
	LD	L,A
	LD	(350H),HL	;ZL-Abst
	LD	B,4
	POP	AF
	LD	HL,34EH
	EX	(SP),HL
REN2	JR	Z,REN5
	CALL	0C986H
	PUSH	AF
	LD	A,D
	OR	E
REN3	JP	Z,0C967H ;FC-ERROR
	POP	AF
	EX	(SP),HL
	LD	M,E
	INC	HL
	LD	M,D
	INC	HL
	JR	Z,REN5
	PUSH	AF
	DEC	B
	JR	Z,REN4
	POP	AF
	EX	(SP),HL
	CALL	0C8D6H	;Komma?
	JR	REN2
REN4	POP	AF
	JP	NZ,SNERR2
REN5	LD	HL,(350H)
	LD	DE,(34EH)
	CALL	0C689H
REN6	JP	C,0C967H
	LD	HL,(35FH)
REN7	CALL	0C4BEH
	JR	C,REN8
	JR	Z,REN3
	JR	REN7
REN8	POP	HL
	PUSH	BC
	LD	DE,(350H)
	LD	HL,0
	LD	(350H),HL
REN9	LD	H,B
	LD	L,C
	LD	C,M
	INC	HL
	LD	B,M
	LD	A,B
	OR	C
	JR	Z,REN3
	INC	HL
	LD	A,M
	INC	HL
	LD	H,M
	LD	L,A
	CALL	0C689H
	LD	HL,(350H)
	INC	HL
	LD	(350H),HL
	JR	NZ,REN9
	INC	HL
	INC	HL
	ADD	HL,HL
	INC	HL
	LD	DE,(3D7H)
	ADD	HL,DE
	JR	C,REN6
	CALL	0C327H	;genug RAM?
	LD	(3D7H),HL
	XOR	A
	DEC	HL
	LD	M,A
	DEC	HL
	LD	M,A
	DEC	DE
	DEC	DE
	EX	DE,HL
	LD	M,E
	INC	HL
	LD	M,D
	INC	HL
	DEC	A
	LD	M,A
	INC	HL
	LD	M,A
	INC	HL
	EX	DE,HL
	LD	HL,(352H)
	LD	(34EH),HL
REN10	POP	HL
	LD	C,M
	INC	HL
	LD	B,M
	INC	HL
	PUSH	BC
	LD	BC,34EH
	LD	A,M
	LD	(DE),A
	LD	A,(BC)
	LD	M,A
	INC	HL
	INC	DE
	INC	BC
	LD	A,M
	LD	(DE),A
	LD	A,(BC)
	LD	M,A
	INC	DE
	LD	HL,(34EH)
	LD	BC,(354H)
	ADD	HL,BC
	LD	(34EH),HL
	LD	HL,(350H)
	DEC	HL
	LD	A,H
	OR	L
	LD	(350H),HL
	JR	NZ,REN10
	LD	(DE),A
	POP	HL
	LD	HL,(35FH)
	PUSH	HL
REN11	POP	HL
	LD	C,M
	INC	HL
	LD	B,M
	INC	HL
	PUSH	BC
	LD	A,M
	INC	HL
	AND	M
	INC	A
	JR	Z,REN15
REN12	INC	HL
REN13	LD	A,M
	OR	A
	JR	Z,REN11
	CP	88H	;GOTO
	JR	Z,REN16
	CP	8CH	;GOSUB
	JR	Z,REN16
	CP	8BH	;RESTORE
	JR	Z,REN14
	CP	0D4H	;ELSE
	JR	Z,REN14
	CP	0A9H	;THEN
	JR	NZ,REN12
REN14	CALL	0C987H
	LD	A,E
	OR	D
	CALL	NZ,RENUP1
	CALL	NZ,RENUP2
	JR	REN13
REN15	DEC	HL
	LD	(3D7H),HL
	DEC	HL
	LD	M,A
	DEC	HL
	LD	M,A
	POP	HL
	POP	HL
	JP	0C48AH
REN16	CALL	0C987H
	LD	A,E
	OR	D
	JR	Z,REN13
	CALL	RENUP1
	CALL	NZ,RENUP2
	LD	A,M
	CP	','
	JR	NZ,REN13
	JR	REN16
 
RENUP1	PUSH	HL
	PUSH	DE
	LD	DE,-1
	CALL	0C4BBH	;n{chste Zeile
	POP	DE
	INC	BC
	INC	BC
	INC	BC
	INC	BC
	LD	H,B
	LD	L,C
	LD	BC,(352H)
RUP11	LD	A,M
	INC	HL
	PUSH	HL
	OR	M
	JR	Z,RUP13
	LD	A,M
	DEC	HL
	LD	L,M
	LD	H,A
	CALL	0C689H
	JR	Z,RUP12
	LD	HL,(354H)
	ADD	HL,BC
	LD	B,H
	LD	C,L
	POP	HL
	INC	HL
	JR	RUP11
RUP12	LD	A,0FFH
	OR	A
RUP13	POP	HL
	POP	HL
	RET
 
RENUP2	PUSH	BC
	EX	DE,HL
	LD	HL,(3D7H)
	SBC	HL,DE
	PUSH	HL
RUP21	POP	BC
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
RUP22	EX	DE,HL
	POP	DE
	PUSH	HL
	PUSH	BC
	XOR	A
	LD	B,98H
	CALL	0D6AEH
	CALL	0D834H
	POP	BC
	POP	DE
	INC	HL
	INC	DE
RUP23	LD	A,M
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
RUP24	PUSH	DE
	LD	DE,(35FH)
	CALL	0C493H
RUP25	INC	HL
	LD	A,M
	INC	HL
	OR	M
	JR	NZ,RUP25
	EX	DE,HL
	LD	M,E
	INC	HL
	LD	M,D
	INC	DE
	INC	DE
	LD	(3D7H),DE
	POP	HL
	LD	D,H
	LD	E,L
RUP26	LD	A,M
	OR	A
	INC	HL
	JR	NZ,RUP26
	POP	BC
	EX	(SP),HL
	PUSH	BC
	EX	DE,HL
	RET
 
DELETE	;Zeile(n) l|schen
	RET	Z
	CALL	0C986H
	JP	Z,0C442H
	CALL	0C8D6H
	PUSH	DE
	CALL	0C986H
	POP	HL
	RET	NZ
	EX	DE,HL
	PUSH	HL
	CALL	0C4BBH	;n{chste Zeile
	JP	NC,0C44DH ;UL-ERROR
	POP	DE
	PUSH	AF
	PUSH	BC
	CALL	0C4BEH
	JP	NC,0C44DH ;UL-ERROR
	POP	BC
	JP	0C450H	;einsortieren
 
PAUSE	;Programm unterbrechen
	CALL	0C8BEH
	JR	NZ,PAUS2
PAUS1	CALL	KBDS
	JR	NC,PAUS1
	CP	3	;BRK?
	RET	Z
	CP	0AH	;CUU?
	JR	NZ,PAUS1
	JP	0DDE4H	;Eingabe ASCII
PAUS2	CALL	0D421H
	LD	C,A
PAUS3	LD	A,10H	;96 sek.
	LD	E,14H	;WAIT
	CALL	0F015H	;PV5
	PUSH	BC
	CALL	KBDS
	POP	BC
	JR	NC,PAUS4
	CP	3	;BRK?
	RET	Z
	CP	0AH	;CUU?
	JR	NZ,PAUS4
	JP	0DDE4H	;Eingabe ASCII
PAUS4	DEC	C
	JR	NZ,PAUS3
	RET
 
BBEEP	;Signalton
	LD	B,1	;ohne Angabe 1x
	CALL	0C8BEH
	JR	Z,BBP1
	CALL	0D421H
	LD	B,A	;Anzahl
BBP1	LD	A,7	;Beep
	LD	E,0	;CRT
	CALL	0F015H	;PV5
	DJNZ	BBP1
	RET
 
WINDOW	;Fenster einstellen
	CALL	0C8BEH
	JR	Z,WINDOW1
	PUSH	HL
	CALL	0D421H
	EX	(SP),HL	    ;in
	LD	HL,WNDFN+90 ;Fenster 9
	CALL	IRMWR	    ;erstellen
	INC	HL
	EX	(SP),HL
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	CALL	IRMWR
	INC	HL
	EX	(SP),HL
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	CALL	IRMWR
	EX	(SP),HL
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	LD	D,A
	CALL	0F018H	;IRMON
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
	JR	WINDOW2
WINDOW1	PUSH	HL
	CALL	0F018H	;IRMON
	LD	HL,100H
	LD	(WINON),HL
	LD	HL,1E28H
	LD	(WINLG),HL
WINDOW2	LD	HL,0
	LD	(CURSO),HL
	CALL	0F01BH	;IRMOF
	POP	HL
	RET
WINERR	CALL	0F01BH	;IRMOF
	JR	SNERR2
 
BLOAD	;MC-Programm laden
	PUSH	HL
	LD	HL,ARGN
	XOR	A
	CALL	IRMWR	;ARGN=0
	POP	HL
	LD	E,10H	;LOAD
	JP	0F015H	;PV5
 
VPEEK	;Zeichen aus IRM lesen
	CALL	0CDE1H
	EX	(SP),HL
	LD	DE,0CDF3H ;RET-Adr.
	PUSH	DE
	CALL	0C96FH
	PUSH	HL
	LD	HL,8000H;IRM-Offset
	ADD	HL,DE
	JR	C,VPK	;Error
	LD	A,0BFH
	CP	H
VPK	JR	C,SNERR2
	CALL	IRMRD	;lesen
	POP	HL
	JP	0D3E9H
 
SNERR2	JP	0C348H	;SN-ERROR
 
VPOKE	;Zeichen in IRM schreiben
	CALL	0C96CH
	PUSH	DE
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	LD	DE,8000H;IRM-Offset
	ADD	HL,DE
	JR	C,VPK	;Error
	LD	D,A
	LD	A,0BFH
	CP	H
	JR	C,VPK	;nur bis BFFFH
	LD	A,D
	CALL	IRMWR	;schreiben
	POP	HL
	RET
 
BORDER	;schaltet Bit 5,6 von PIO-B
	;(beim KC85/4 RAM8!)
	CALL	0D421H
	RRCA
	RRCA
	RRCA
	AND	60H	;Auswahl
	LD	C,A
	LD	B,9FH	;Maske
	JP	TONB
 
PRAT	;PRINT AT
	LD	A,(3FDH)
	BIT	1,A
	SET	1,A
	LD	(3FDH),A
	JR	NZ,SNERR2
	PUSH	DE
	PUSH	HL
	CALL	0F018H	;IRMON
	LD	HL,WINON
	LD	(WNDFN+90),HL
	LD	DE,WNDFN+80
	LD	BC,6	;Fenster
	LDIR		;retten
	LD	HL,0	;Fenster gro~
	LD	(WINON),HL
	LD	HL,2028H
	LD	(WINLG),HL
	CALL	0F01BH	;IRMOF
	POP	HL
	CALL	LOCAT
	CALL	0C8DBH
	CALL	0C8CCH
	DB	';'
	LD	A,M
	CP	0DFH
	JR	C,PRAT2	;<DFH
	CP	0E3H
	JR	NC,PRAT2	;>E2H
	CALL	PREX4	;INK,PAPER,COLOR
PRAT1	PUSH	HL
	CALL	0F018H	;IRMON
	LD	HL,WNDFN+80
	LD	DE,WINON
	LD	BC,6	;Fenster
	LDIR		;regenerieren
	CALL	0F01BH	;IRMOF
	POP	HL
	POP	DE
	POP	BC
	RET
PRAT2	CALL	0CB03H
	JR	PRAT1
 
SNERR3	JP	0C348H	;SN-ERROR
 
INK	;Vordergrundfarbe
	CALL	0D421H
	CP	32
	JR	NC,SNERR3
	SLA	A
	SLA	A
	SLA	A
	LD	D,A
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMRD
	POP	HL
	AND	7
	JR	COL2
 
PAPER	;Hintergrundfarbe
	CALL	0D421H
	CP	8
	JR	NC,SNERR3
	LD	D,A
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMRD
	POP	HL
	AND	0F8H
COL2	OR	D
	PUSH	HL
	LD	HL,COLOR
	CALL	IRMWR	;neuer Farbwert
	POP	HL
	RET
 
BCOLOR	;Farbe einstellen
	CALL	0C8BEH
	JR	Z,SNERR3
	CALL	INK	;Vordergrund
	CALL	0C8BEH
	RET	Z
	CALL	0C8D6H
	JR	PAPER	;Hintergrund
	RET		;???
 
LOCAT	;Cursor positionieren
	CALL	0C8BDH
	CALL	0C8CCH
	DB	'('
LOCATE	CALL	0D421H
	LD	D,A
	PUSH	HL
	LD	HL,WINLG+1
	CALL	IRMRD
	POP	HL
	DEC	A
	CP	D
	JR	C,SNERR3
	CALL	0C8D6H
	PUSH	DE
	CALL	0D421H
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
LOC1	CALL	IRMWR
	POP	HL
	RET
 
INKEY	;Eingabe ein Zeichen
	PUSH	HL
	CALL	KBDS
	JR	NC,INKEY3
	LD	A,1
	CALL	0D17BH
	CALL	0DDE4H	;Eingabe ASCII
INKEY1	LD	HL,(3C2H)
	LD	M,A
INKEY2	JP	0D1A9H
INKEY3	XOR	A
	CALL	0D17BH
	JR	INKEY2
 
SOUND	;Tonausgabe
	CALL	0D421H
	PUSH	HL
	LD	HL,ARG1
	LD	B,4	;4 Argumente
SOUND1	CALL	IRMWR
	INC	HL
	EX	(SP),HL
	DEC	B
	JR	Z,SOUND2
	PUSH	BC
	CALL	0C8D6H
	CALL	0D421H
	POP	BC
	EX	(SP),HL
	JR	SOUND1
SOUND2	CALL	0C8BEH
	JR	Z,SOUND3
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	CALL	IRMWR
	INC	HL
	EX	(SP),HL
	CALL	0C8BEH
	JR	Z,SOUND3
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	CALL	IRMWR
	EX	(SP),HL
SOUND3	LD	E,35H	;TON
	POP	BC
	JP	0F015H	;PV5
 
PSET	;Punkt setzen
	LD	B,1	;setzen
POINT	PUSH	BC
	CALL	0C96CH
	PUSH	HL
	LD	A,E
	LD	HL,HOR
	CALL	IRMWR
	LD	A,D
	INC	HL
	CALL	IRMWR
	EX	(SP),HL
	CALL	0C8D6H
	CALL	0D421H
	EX	(SP),HL
	INC	HL
	CALL	IRMWR
	POP	HL
	CALL	GFARB
	LD	E,30H	;PUSE
	POP	BC
	DEC	B
	JR	Z,POINT2
	DEC	E	;PUDE
POINT2	CALL	0F015H	;PV5
	JP	C,SNERR2
	RET
 
PRESET	;Punkt l|schen
	LD	B,0	;l|schen
	JR	POINT
 
GFARB	;Grafikfarbe setzen
	CALL	0C8BEH
	RET	Z
	CALL	0C8D6H
	CALL	0D421H
	PUSH	HL
	LD	HL,FARB	;Grafik-Farbe
	RLA
	RLA
	RLA
	JP	LOC1	;in IRM schreib.
 
INSTR	;String1 in String2 suchen
	CALL	0CD36H	;Klammer auf?
	CALL	0C8D6H
	PUSH	HL
	CALL	0D330H
	JR	Z,INSTR4
	LD	B,A
	INC	HL
	INC	HL
	LD	E,M
	INC	HL
	LD	D,M
	POP	HL
	PUSH	DE
	PUSH	BC
	CALL	0CD3AH
	CALL	0C8DBH	;Klammer zu?
	POP	BC
	POP	DE
	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	0D330H
	JR	Z,INSTR4
	INC	HL
	INC	HL
	LD	C,M
	INC	HL
	LD	H,M
	LD	L,C
	POP	BC
	LD	C,A
	POP	DE
	PUSH	HL
INSTR0	PUSH	BC
	PUSH	DE
	LD	A,(DE)
INSTR1	CP	M
	JR	Z,INSTR5
	INC	HL
	DEC	C
	JR	NZ,INSTR1
INSTR2	XOR	A
	POP	HL
	POP	HL
	POP	HL
INSTR3	LD	DE,0CDF3H ;RET-Adr.
	PUSH	DE
	JP	0D0C0H
INSTR4	JP	0C967H	;FC-ERROR
INSTR5	INC	HL
	PUSH	HL
	DEC	HL
INSTR6	INC	HL
	DEC	C
	JR	Z,INSTR7
	INC	DE
	DEC	B
	JR	Z,INSTR8
	LD	A,(DE)
	CP	M
	JR	Z,INSTR6
	POP	HL
	POP	DE
	LD	A,C
	POP	BC
	LD	C,A
	JR	INSTR0
INSTR7	INC	DE
	DEC	B
	POP	HL
	JR	NZ,INSTR2
	JR	INSTR9
INSTR8	POP	HL
INSTR9	POP	DE
	POP	DE
	POP	DE
	AND	A
	SBC	HL,DE
	LD	A,L
	JR	INSTR3
 
IRMWR	LD	E,28H	;LDMA
	JR	JPV5
 
IRMRD	LD	E,29H	;LDAM
	JR	JPV5
 
BKEY	;F-Taste belegen
	CALL	0D421H
	AND	A
	JR	Z,SNERR4
	CP	0DH	;F1...F12
	JR	NC,SNERR4
	LD	E,39H	;KEY
JPV5	JP	0F015H	;PV5
 
KEYLIST	;F-Tasten auflisten
	LD	E,3AH	;KEYLI
	JR	JPV5
 
SWITCH	;Module schalten
	CALL	0D421H
	PUSH	AF
	CALL	0C8D6H
	CALL	0D421H
	LD	D,A	;Steuerbyte
	POP	AF
	PUSH	HL
	LD	L,A	;Steckplatz
	LD	A,2	;schalten
	LD	E,26H	;MODU
	CALL	0F015H	;PV5
	POP	HL
	RET
 
PTEST	;Punkt testen
	CALL	0CDE1H
	EX	(SP),HL
	LD	DE,0CDF3H ;RET-Adr.
	PUSH	DE
	CALL	0C96FH
	PUSH	HL
	LD	A,E	;nur
	LD	HL,HOR	;X-Koordinate
	CALL	IRMWR	;eintragen
	LD	A,D
	INC	HL
	CALL	IRMWR
	LD	E,2FH	;PUDE
	CALL	0F015H	;PV5
	LD	B,0
	JR	Z,PTST1	;war gel|scht
	INC	HL
	INC	HL
	CALL	IRMWR
	LD	E,30H	;PUSE
	CALL	0F015H	;PV5
	LD	B,1
PTST1	LD	A,B	;R}ckgabewert
	POP	HL
	JP	0D3E9H
 
SNERR4	JP	0C348H	;SN-ERROR
 
CLOSE	;Kanal schlie~en
	LD	C,0
	LD	A,M
	CP	'I'	;Eingabe?
	JR	Z,CLOS1
	INC	C
	CP	'O'	;Ausgabe?
	JR	NZ,SNERR4
CLOS1	PUSH	BC
	INC	HL
	LD	A,M
	CP	'#'
CLOS2	JR	NZ,SNERR4
	INC	HL
	CALL	0D421H
	AND	3	;4 Ger{te
	POP	BC
	RET	Z	;Console
	PUSH	HL
	RLA		;*2
	ADD	C	;Ein/Ausgabe
	PUSH	AF
	DEC	A
	LD	B,0
	SCF
CLOS3	RL	B	;Bit platzieren
	DEC	A
	JR	NZ,CLOS3
	LD	HL,307H	;E/A-Flag
	LD	A,M
	XOR	B	;negieren
	LD	M,A
	POP	AF
	POP	HL
	SET	6,A	;close
	PUSH	DE
	LD	E,A
	LD	D,3
	CALL	BASIO
	POP	DE
	RET
 
RANDOM	;Zufallsgenerator
	LD	A,R
	LD	(31DH),A
	RET
 
OPEN	;Kanal |ffnen
	LD	A,M
	INC	HL
	CP	'I'	;Eingabe?
	JR	Z,OPEN1
	CP	'O'	;Ausgabe?
	JR	NZ,CLOS2
	CALL	0DE25H
	CALL	0DDC8H
	RET	Z
	LD	A,0D5H
	CALL	0DCB2H
	LD	HL,3EAH	;Druckpuffer
	XOR	A
	CALL	0DDD5H
	POP	HL
	RET
OPEN1	CALL	0DE5FH	;Eingabe
	LD	A,(309H);IN-Index
	AND	3
	RET	Z
	LD	A,0D5H
	CALL	0DCB2H
	LD	HL,3EAH	;Druckpuffer
	CALL	0DDE4H	;Eingabe ASCII
	POP	HL
	RET
 
BLINE	;Linie zeichnen
	LD	BC,43EH	;4 Par.,LINE
	JR	GRAPH
 
CIRCLE	;Kreis zeichnen
	LD	BC,33FH	;3 Par.,CIRCLE
GRAPH	PUSH	BC
	CALL	0C96CH
	POP	BC
	PUSH	BC
	PUSH	HL
	LD	HL,ARG1
GRAPH1	LD	A,E
	CALL	IRMWR
	INC	HL
	LD	A,D
	CALL	IRMWR
	INC	HL
	DEC	B
	JR	Z,GRAPH2
	EX	(SP),HL
	PUSH	BC
	CALL	0C8D6H
	CALL	0C96CH
	POP	BC
	EX	(SP),HL
	JR	GRAPH1
GRAPH2	POP	HL
	CALL	GFARB
	POP	DE
	JP	0F015H	;PV5
 
CSRLN	;akt. Zeile holen
	CALL	0CDE1H
	EX	(SP),HL
	LD	DE,0CDF3H ;RET-Adr.
	PUSH	DE
	CALL	0D424H
	PUSH	HL
	AND	A
	LD	A,0
	JR	NZ,CSRLN1
	LD	HL,WINON+1
	CALL	IRMRD
CSRLN1	LD	B,A
	LD	HL,CURSO+1
	CALL	IRMRD
	ADD	B	;akt. Zeile
	POP	HL
	JP	0D3E9H
 
VGET	;Zeichen von Bildschirm lesen
	EX	(SP),HL
	LD	A,1
	CALL	0D17BH
	CALL	0F018H	;IRMON
	LD	DE,(CURSO)
	CALL	DABR
	LD	A,M
	CALL	0F01BH	;IRMOF
	JP	INKEY1	;als Zeichen
			;zur}ckgeben
 
	DS	10,0FFH	;frei
 
	END
��
VGET:��
CSRLN1:��
CSRLN:��
GRAPH2:��
GRAPH1:��
GRAPH:��
CIRCLE:��
BLeichen
			;zur}ckgeben
 
	DS	10,0FFH	;frei
 
	END
��
VGET:��
CSRLN1:��
CSRLN:��
GRAPH2:��
GRAPH1:��
GRAPH:��
CIRCLE:��
BL