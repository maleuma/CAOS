CF41    ASM                                                                                                                     
;**	CAOS 4.1 ROM F		**
 
	ORG	0F000H
 
	JP	PWRON
	JP	PV1	;m. DEFB nr
	JP	PV2	;(ARGC)=nr
	JP	PV3	;E=nr
	JP	PV4	;E=nr, IRM e/a
	JP	RCALL	;m. DEFW offset
	JP	BYE	;JUMP-Einsprung
	JP	PV5	;E=nr
	JP	IRMON
	JP	IRMOF
	PUSH	BC	;PV6, (ARGC)=nr
	CALL	IRMON
	CALL	PV2
	JR	PV51
 
PV5	PUSH	BC
	CALL	IRMON
	CALL	PV3
PV51	CALL	IRMOF
	POP	BC
	RET
 
PV4	PUSH	AF
	IN	A,88H
	SET	2,A
	OUT	88H	;IRM ein
	POP	AF
	CALL	PV3
SCROFF	PUSH	AF
	IN	A,88H
	RES	2,A
	OUT	88H	;IRM aus
	POP	AF
	RET
 
PV1	DI
	PUSH	HL
	POP	HL
	POP	HL
	INC	HL	;nr }bergehen
	PUSH	HL
	DEC	HL	;(HL)=nr
	DEC	SP
	DEC	SP
	EI
	PUSH	AF
	PUSH	DE
	LD	E,M
PV22	LD	D,0
	LD	HL,(SUTAB)
	ADD	HL,DE
	ADD	HL,DE	;HL=Pos. in Tab
	LD	E,M
	INC	HL
	LD	D,M
	EX	DE,HL
	POP	DE
	POP	AF
EXRT	EX	(SP),HL
	RET		;UP anspringen
 
PV2	AND	A	;VR: F
PV21	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	HL,POPS3;Returnadresse
	EX	(SP),HL	;kellern
	PUSH	HL
	PUSH	AF
	PUSH	DE
	JR	C,PV22	;falls PV3
	LD	A,(ARGC);sonst E
	LD	E,A	;beschaffen
	JR	PV22
 
PV3	SCF		;VR: F
	JR	PV21
 
IRMON	POP	BC	;R}ckkehradr.
	PUSH	IY
	LD	IY,0	;SP in IY merken
	ADD	IY,SP	;(kein UP ver-
	DI		;{ndert IY!)
	LD	(IX+11),A
	IN	A,88H
	SET	2,A	;IRM ein
	SET	5,A	;LED ein
	OUT	88H
	LD	SP,(SYSP)
	EI
	LD	A,(IX+11)
	PUSH	BC
	RET		;wie JP (BC)
 
IRMOF	POP	BC	;analog IRMON
	LD	(IX+11),A
	IN	A,88H
	RES	2,A
	RES	5,A
	DI
	OUT	88H
	LD	SP,IY
	EI
	LD	A,(IX+11)
	POP	IY
	PUSH	BC
	RET
 
RCALL	;Relativer UP-Aufruf
	EX	(SP),HL	;VR: DE
	LD	E,M
	INC	HL
	LD	D,M
	INC	HL	;DE=offset
	EX	DE,HL
	ADD	HL,DE	;zur Absolut-
	EX	DE,HL	;adr. addieren
	EX	(SP),HL
	PUSH	DE
	RET		;wie JP (DE)
 
POPS3	POP	BC	;f}r PV2..6
	POP	DE
	POP	HL
	RET
 
PWRON	;Einschalt-Init
	LD	SP,1C4H
	XOR	A
	LD	B,A
	LD	C,A
PWR1	LD	(BC),A	;Speicher
	DEC	C	;l|schen
	JR	NZ,PWR1
	DJNZ	PWR1
	LD	C,80H	;Module
PWR2	OUT	(C),A	;abschalten
	DJNZ	PWR2
	CALL	SYSI	;IRM-Init usw.
	XOR	A
PWR3	LD	(WINNR),A
	CALL	WCOPY	;Fenstervektor-
	INC	A	;speicher init.
	CP	0AH
	JR	C,PWR3
	CALL	ESC3
	LD	A,0CH	;Zweitbild
	CALL	CRT	;l|schen
	CALL	ESC1
	CALL	CON	;M003 suchen,
	CALL	V24INI	;einschalten,
	CALL	COFF	;Tab. kopieren
	JR	PWR4
 
BYE	;Tasten-RESET
	;M003 wird bei RESET nicht
	;erneut gesucht!
 
	LD	SP,1C4H
	CALL	SYSI	;IRM-Init usw.
	;
PWR4	LD	A,(0A800H)
	DEC	A	;M003 vorh.?
	JR	NZ,PWR5
	CALL	CON
	CALL	V24OC	;Druckerinit
	CALL	UMINIT	;SIO B f}r
	CALL	COFF	;Interrupt
PWR5	LD	BC,880H	;ROM-Modul mit
	IN	A,(C)	;Strukturbyte
	DEC	A	;01?
	JR	NZ,MEN0	;nein
	LD	A,43H
	OUT	(C),A	;ein ab 4000H
	LD	(0B808H),A
	LD	A,(IX+4)
	AND	0FCH	;RAM4 aus
	LD	(IX+4),A
	OUT	86H
	JP	4000H	;anspringen
 
;
	DW	7F7FH
	DB	'MENU',1
	POP	HL	;Stack clear
MEN0	CALL	OSTR
	DB	0CH,0AH	;CLS,CUD
	DB	'* KC-CAOS 4.1 *',0
MENU	LD	HL,0C000H	;**46**
	LD	C,L
	LD	B,L	;BC=0
MEN1	CALL	CRLF
MEN2	CALL	OSTR
	DB	2,'%',0	;CLLN,Prompt
	CALL	BRKT
	JR	C,LOOP3
	LD	A,(IX+9)
MEN3	CPIR
	JP	PO,LOOP3;ausgesucht.
	CPI		;2tes Mal?
	JR	NZ,MEN3
MEN4	LD	A,M
	CP	2	;Epilog?
	JR	C,MEN1
	CP	30H	;<30h?
	JR	C,MEN2
	CP	60H	;>60h, Kleinb.?
	JR	NC,MEN2
	CALL	OCHR
	INC	HL
	DEC	BC
	JR	MEN4
 
LOOP1	POP	HL
LOOP2	CALL	ERRM
LOOP	CALL	OSTR		;**12**
	DB	'%',0	;Prompt
LOOP3	CALL	INLIN
	INC	DE	;Zeichen danach
	LD	A,(DE)
	CP	' '
	JR	Z,LOOP
	AND	A
	JR	Z,LOOP
	CALL	MSUCH
	JR	NC,LOOP2;nicht gef.
	PUSH	HL
	CALL	GARG	;Argumente
	JR	C,LOOP1
	LD	HL,LOOP	;R}ckkehradr.
	EX	(SP),HL	;kellern
	LD	A,M	;A=Epilog
	INC	HL	;}bergehen
	PUSH	HL
	AND	A
	CALL	LARG
	RET	NZ	;A=1: nach (HL)
	JP	SCROFF	;A=0: IRM aus,
			;dann nach (HL)
 
MSUCH	LD	HL,0C000H
	LD	B,L
	LD	C,L	;BC=0
	LD	A,(IX+9)
ZSUCH	CPIR			;**1D**
	SCF
	CCF
	RET	PO	;nicht gef.
	CPI
	JR	NZ,ZSUCH
	PUSH	AF
	PUSH	DE
ZS1	LD	A,(DE)
	CP	'!'	;keine SPC und
	JR	C,ZS3	;Steuerzeichen
	INC	DE
	CPI
	JR	Z,ZS1
ZS2	POP	DE
	POP	AF
	JR	ZSUCH
	;
ZS3	LD	A,M
	CP	2	;Epilog?
	JR	C,ZS4	;ja
	CP	' '
	JR	C,ZS2
	INC	HL
	JR	ZS3
	;
ZS4	POP	AF
	POP	AF
	SCF
	RET		;gefunden
 
WTUP	DEC	A	;UP f}r WAIT
	RET	Z
	PUSH	AF
	POP	AF
	JR	WTUP
 
WAIT	LD	B,A		;**14**
	XOR	A
WAIT1	CALL	WTUP
	DJNZ	WAIT1
	RET
 
BNROST	LD	A,(IX+2);Blocknummer u.
AHOSTR	CALL	AHEX	;RG A und folg.
OSTR	EX	(SP),HL	;ZK	 **23**
OSTR1	LD	A,M	;ausgeben
	INC	HL
	AND	A	;bis 00-Byte
	JP	Z,EXRT
	CALL	OCHR
	JR	OSTR1
 
NORM	CALL	NIN		;**13**
NOUT	LD	HL,ZEI0		;**20**
SOUT	PUSH	HL		;**1E**
	LD	HL,(OUTAB)
	EX	(SP),HL
	LD	(OUTAB),HL
	POP	HL
	RET
 
NIN	LD	HL,ZEI4		;**21**
SIN	PUSH	HL		;**1F**
	LD	HL,(INTAB)
	EX	(SP),HL
	LD	(INTAB),HL
	POP	HL
	RET
 
INTB	PUSH	HL		;**16**
	LD	HL,(INTAB)
	JR	INTA1
OCHR	PUSH	HL		;**24**
	LD	HL,(OUTAB)
INTA1	PUSH	DE	;s.a. PV1
	PUSH	BC	;(sehr {hnlich)
	LD	E,M
	LD	D,0
	LD	HL,(SUTAB)
	PUSH	AF
	ADD	HL,DE
	ADD	HL,DE
	POP	AF
	LD	E,M
	INC	HL
	LD	D,M
	EX	DE,HL
	CALL	JPHL
POP3	POP	BC
	POP	DE
	POP	HL
	RET
 
KBD	PUSH	HL		;**04**
	PUSH	DE
	PUSH	BC
KBD1	BIT	6,(IX+8);F-Taste aktiv?
	JR	NZ,KBD12;ja
KBD2	LD	DE,(CURSO)
	CALL	DABR
	XOR	A	;Dummy
	CP	M	;hinter Cursor?
	JR	Z,KBD5	;ja-Strich
	LD	E,A	;nein-Klotz,E=0
KBD3	CALL	CUCP	;blinken
	INC	E	;Klotz e/a-Merk
	LD	B,0FH	;F00h mal
KBD4	CALL	KBDZ	;abfragen
	JR	C,KBD7
	DEC	BC
	LD	A,B
	OR	C
	JR	NZ,KBD4
	JR	KBD3	;dann Cursor
			;wieder weg usw.
KBD5	CALL	PADR0
	SET	1,L	;vorletzte
	SET	2,L	;Cursorzeile
	LD	A,M
	LD	B,A	;merken
	XOR	7FH
	LD	M,A
KBD6	CALL	KBDZ	;abfragen
	JR	NC,KBD6
	LD	M,B	;r}cksetzen
	JR	KBD8
 
KBD7	BIT	0,E	;Klotz gerade
			;ein?
	CALL	NZ,CUCP	;ja - wegnehmen
KBD8	LD	D,A
	BIT	5,(IX+8);Click ein?
	LD	BC,307H
	CALL	NZ,CLIK
	LD	A,D
	LD	HL,STBT
	BIT	4,M	;ESC aktiv?
	JR	Z,KBD9
	LD	DE,(CURSO)
	CALL	CRT1
	LD	(CURSO),DE
	JR	KBD2
 
KBD9	CP	1BH	;ESC?
	JR	NZ,KBD10
	SET	4,M	;ja
	JR	KBD2
 
KBD10	CP	0F1H	;F-Taste?
	JR	C,POP3
	SET	6,(IX+8);ja - merken
	AND	0FH
	LD	B,A
	LD	HL,0B900H
KBD11	LD	A,M	;Pos. in F-
	AND	A	;Puffer best.
	INC	HL
	JR	NZ,KBD11
	DJNZ	KBD11
	JR	KBD13
 
KBD12	CALL	BRKT	;Abbruch?
	JR	C,KBD15
	LD	HL,(FTAST);Zeiger auf
KBD13	LD	A,M	;n{chstes CHR
	CP	1BH	;ESC?
	JR	NZ,KBD14
	INC	HL
	LD	(FTAST),HL
	LD	HL,STBT
	SET	4,M	;ESC merken
	JP	KBD1
 
KBD14	INC	HL
	LD	(FTAST),HL;Zeiger rett.
	AND	A	;Null-Ende?
	JR	NZ,KBD16
KBD15	RES	6,(IX+8);ja - inaktiv
	JP	KBD2
 
KBD16	LD	HL,STBT
	BIT	4,M
	JP	Z,POP3
	LD	DE,(CURSO)
	CALL	CRT1	;ESC-Seq. ausf.
	LD	(CURSO),DE
	JP	KBD1
 
KCTAB	;IRM-Defaulttabelle
	DW	NOT	;Hardcopy
	DB	0	;WINNR
	DW	0	;WINON
	DW	2028H	;WINLG
	DW	0	;CURSO
	DB	0	;STBT
	DB	39H	;COLOR
	DW	SCRLPG	;WEND
	DW	0EE00H	;CCTL0
	DW	0FE00H	;1
	DW	0EE00H	;2
	DW	0FE00H	;3
	DW	1C4H	;SYSP
	DW	SUTB	;SUTAB
	DW	CRTTAB	;CTAB
	OUT	88H	;BJUMP
	JP	0F012H
	DW	ZEI0	;OUTAB
	DW	ZEI4	;INTAB
	JP	NOOP	;UOUT1
	JP	NOOP	;UIN1
	JP	NOOP	;UOUT2
	JP	NOOP	;UIN2
	DW	0DC82H	;IOERR (BASIC)
	DW	0B200H	;VRAM
 
	DW	L3TB	;L3TAB
	DB	11	;L3SIZ
	DB	5	;COUNT
	DB	0	;HCPZ
	DW	0A801H	;INTV1
	DB	8	;INTV1L
	DW	0A809H	;INTV2
	DB	9	;INTV2L
	DB	6	;HCPZ2
 
INLIN	CALL	INTB		;**17**
	CALL	OCHR
	CP	0DH	;ENTER?
	JR	NZ,INLIN
	CALL	CRLF	;PA: A=0
	LD	DE,(CURSO)
	CP	D	;1 Zeile durch
			;PAGE-Mode?
	JR	NZ,INLI1
	LD	A,(WINLG+1)
	LD	D,A	;D korrigieren
INLI1	DEC	D	;Zeile dar}ber
	PUSH	HL
	CALL	DABR
	EX	DE,HL
	POP	HL	;PA: DE
NOOP	RET
 
HLHX	LD	A,H		;**1A**
	CALL	AHEX
	LD	A,L
AHSPC	CALL	AHEX
SPACE	LD	A,20H		;**2B**
SPAC0	JP	OCHR
 
ERRM	CALL	OSTR		;**17**
	DB	'ERROR',7,0
	;
CRLF	CALL	OSTR		;**2C**
	DB	0DH,0AH,0
	RET
 
HOME	LD	A,10H		;**2D**
	JR	SPAC0
 
AHEX	PUSH	AF		;**1C**
	RRA		;Tetradentausch
	RRA
	RRA
	RRA
	CALL	AHEX0	;erst rufen,
	POP	AF	;dann hinein-
			;laufen
AHEX0	AND	0FH	;Maske
	ADD	90H	;}bliche
	DAA		;Routine
	ADC	40H	;f}r die
	DAA		;Hexausgabe
	JR	SPAC0
 
HLDE	CALL	HLDE1		;**1B**
HLDE1	CALL	HLHX
	EX	DE,HL
	RET
 
RHEX0	INC	DE
RHEX	LD	A,(DE)		;**18**
	CP	' '	;Trennzeichen
	JR	Z,RHEX0	;}bergehen
	XOR	A
	LD	HL,NUMVX+1
	LD	M,A
	DEC	HL	;HL=NUMVX
	LD	M,A
	DEC	HL	;HL=NUMNX
	LD	M,A	;alles nullsetz.
RH1	LD	A,(DE)
	OR	A	;Ende-Dummy?
	RET	Z
	CP	' '	;Ende-Space?
	RET	Z
	SUB	'0'
	RET	C
	CP	0AH
	JR	C,RH2
	SUB	7
	AND	0DFH	;klein>gro~
	CP	0AH
	RET	C
	CP	10H
	CCF
	RET	C
RH2	INC	DE
	INC	M	;NUMNX erh|hen
	INC	HL
	RLD		;Tetrade
	INC	HL	;durchschieben
	RLD
	DEC	HL
	DEC	HL	;RLD setzt auch
	JR	Z,RH1	;Z-Flag!
	DEC	DE	;Fehler: Zahl
	SCF		;zu gro~
	RET
 
GARG	CALL	CON		;**22**
	CALL	GARGC
	JP	COFF
 
SYSI	LD	DE,HCADR
	LD	HL,KCTAB
	LD	BC,34H
	LDIR		;1. St}ck
	LD	C,0CH
	LD	E,L3TAB
	LDIR		;2. St}ck
	LD	HL,103H	;RAM0,IRM,
	LD	(0B800H),HL
	LD	HL,301H	;ROMC,RAM8,
	LD	(0B802H),HL
	LD	A,3	;RAM4
	LD	(0B804H),A
	LD	A,1
	;
SIXD	LD	HL,ISRTAB	;**31**
	LD	(MIXIT),A
	LD	E,0E2H
	LD	D,A
	LD	BC,0EH	;Interrupttab.
	LDIR		;kopieren
; bei CAOS 4.2 werden hier die alten Merker
; von (IX+1) und (IX+4) mitgenommen
	PUSH	DE
	POP	IX
	LD	(IX+8),88H
	LD	(IX+1),28H;Grundstellung
	LD	(IX+4),63H;der Merker
	LD	(IX+14),KTAB;=L(KTAB)
	LD	(IX+15),0FCH;=H(KTAB)
	LD	(IX+9),7FH
	PUSH	AF
	PUSH	DE
	PUSH	HL
	DI
	IM	2
	LD	A,(MIXIT)
	LD	I,A
	LD	D,9	;9 Kan{le
	LD	HL,IOTAB
	CALL	INIME	;Kan{le init.
	POP	HL
	POP	DE
	POP	AF
	;
KPUFF	LD	(IX+5),0
	LD	(IX+6),0B7H
	RET
 
INIEA	PUSH	BC		;**43**
	LD	C,M	;1. Byte=Adr.
	INC	HL
	LD	B,M	;2. Byte=Anz.
	INC	HL
	OTIR
	POP	BC
	RET
 
INIME	DI			;**44**
INIME1	CALL	INIEA
	DEC	D	;D Kan{le
	JR	NZ,INIME1
	EI
	RET
 
IOTAB	DB	8AH	;PIO-A-Steuerw.
	 DB	1
	  DB	0E4H	;Int-Vektor
	DB	88H	;PIO-A-Daten
	 DB	1
	  DB	0FH	;IRM ein...
	DB	8AH	;PIO-A-Steuerw.
	 DB	2
	  DB	0FH	;Byte-Out
	  DB	3	;DI
	DB	8BH	;PIO-B-Steuerw.
	 DB	3
	  DB	0E6H	;Int-Vektor
	  DB	0FH	;Byte-Out
	  DB	83H	;EI
	DB	89H	;PIO-B-Daten
	 DB	1
	  DB	0FFH	;Daten
	DB	8CH	;CTC0
	 DB	1
	  DB	0E8H	;Int-Vektor CTC
	DB	8EH	;CTC3
	 DB	2
	  DB	47H	;DI,Z{hler,Reset
	  DB	0CH	;Zeitkonst.
	DB	84H	;Port 84H
	 DB	1
	  DB	28H	;Ausgabebyte
	DB	86H	;Port 86H
	 DB	1
	  DB	63H	;Ausgabebyte
;
	DW	7F7FH
	DB	'SAVE',1
	CP	2	;mind. 2 Arg.
	JP	C,ERRM
	CALL	OSTR
	DB	'NAME :',0
	CALL	INLIN
	LD	HL,6	;'NAME :'
	ADD	HL,DE	;}bergehen
	;
SAVE	LD	DE,0B700H	;**36**
	LD	BC,11	;Name nach
	LDIR		;Puffer
	XOR	A
	LD	(DE),A
	LD	HL,ARGN	;alle Arg.
	LD	E,10H	;mit ARGN
	LD	C,15H
	LDIR		;nach Puffer
	CALL	ISRO
	LD	HL,(ARG1)
SAV1	LD	(IX+5),L
	LD	(IX+6),H
	CALL	OSTR
	DB	2,0	;CLLN
	CALL	BNROST	;Blocknummer
	NOP
	LD	BC,0A0H
	CALL	BRKT
	JP	C,CSROI
	LD	DE,80H
	ADD	HL,DE
	LD	DE,(ARG2)
	SBC	HL,DE	;Ende erreicht?
	JP	NC,CSRO
	CALL	MBO
	JR	SAV1
 
LARG	LD	BC,(ARG3)	;**15**
	LD	DE,(ARG2)
	LD	HL,(ARG1)
	LD	A,(ARGN)
	RET
 
LUP	;LOAD-Unterprogramm
	;PA: CY=1 BRK gedr}ckt
	INC	(IX+3)	;erwarteter Blk
LUP1	CALL	MBI
	JR	NC,LUP3
	CALL	OSTR
	DB	9,9,9,' ',0
	CALL	BNROST
	DB	' ?'	;fehlerhaft
	DB	1EH,0
	CALL	KBD
	CP	3	;BRK?
	SCF
	RET	Z
	CP	0AH	;CUD?
	JR	NZ,LUP1
LUP3	LD	A,(IX+2)
	LD	B,(IX+3)
	DEC	B	;Block 01 erw.?
	JR	Z,LUP6
	INC	B	;richtige
	CP	B	;Blocknummer?
	JR	Z,LUP5
	INC	A	;Block FF?
	JR	Z,LUP5
LUP4	CALL	BNROST
	DB	'*'	;falsche Nr.
	DB	19H,0
	JR	LUP1
 
LUP5	CALL	BNROST
	DB	'> '	;korrekter Blk.
	DB	19H,0
	RET
 
LUP6	DEC	A	;Block 01?
	JR	NZ,LUP4
	CALL	CRLF	;keine Nr,
	LD	HL,0B700H
	LD	B,11	;sondern Name
LUP7	LD	A,M	;ausgeben
	INC	HL
	CALL	CRT
	DJNZ	LUP7
	JP	SPACE
 
NOMC	CALL	OSTR	;wenn kein MC
	DB	'???',0
CLJP	JP	CSROI
;
	DW	7F7FH
	DB	'VERIFY',1
VERIF	RES	0,(IX+7)	;**11**
	JR	LOAD0
;
	DW	7F7FH
	DB	'LOAD',1
LOAD	SET	0,(IX+7)	;**10**
LOAD0	CALL	ISRI1
	LD	(IX+3),0
	CALL	LUP
LOAD1	JR	C,CLJP	;bei BRK
	BIT	0,(IX+7)
	JR	Z,LOAD4
	LD	L,10H
	LD	A,(IX+7)
	AND	0E3H	;Bits 2..4 Null
	LD	(IX+7),A
	LD	A,M	;Anzahl Arg.
	SLA	A
	SLA	A	;2* schieben
	AND	1CH	;Maske
	OR	(IX+7)
	LD	(IX+7),A
	LD	A,M	;Anzahl Arg.
	SUB	2
	CP	9	;MC?
	JR	NC,NOMC
	LD	BC,(0B715H)
	LD	DE,(0B713H)
	LD	HL,(0B711H)
	LD	A,(ARGN)
	AND	A	;Ladeoffset?
	JR	Z,LOAD3
	PUSH	BC
	LD	BC,(ARG1)
	ADD	HL,BC	;AAdr,EAdr
	EX	DE,HL	;umrechnen
	ADD	HL,BC
	EX	DE,HL
	BIT	4,(IX+7);4..7 Args?
	JR	NZ,LOAD2;nein
	EX	(SP),HL
	ADD	HL,BC	;SAdr umrechnen
	EX	(SP),HL
LOAD2	POP	BC	;SAdr merken
LOAD3	LD	(ARG2),BC
	CALL	HLDE	;AAdr,EAdr ausg.
LOAD4	CALL	CRLF
	EX	DE,HL
	;Load-Hauptschleife
LOAD5	CALL	LUP
	JR	C,LOAD1	;BRK
	BIT	0,(IX+7)
	JR	Z,LOAD7	;wenn VERIFY
	PUSH	HL
	SBC	HL,DE
	LD	BC,80H
	SBC	HL,BC
	ADD	HL,BC
	JR	NC,LOAD6
	LD	C,L
LOAD6	LD	HL,0B700H ;MC in Spei-
	LDIR		;cher kopieren
	POP	HL
LOAD7	INC	(IX+2)	;Blocknr. FF?
	JR	NZ,LOAD5
	;
CSRI	CALL	CSROI		;**0B**
	LD	A,(IX+7);kein Autostart
	AND	3	;wenn VERIFY,
	DEC	A	;Bit 1(IX+7)=1
	RET	NZ
	LD	A,(IX+7)
	AND	1CH	;und bei nur 2
	CP	0CH	;Ladeargumenten
	LD	(IX+7),0
	RET	C
	LD	HL,(ARG2)
	JP	(HL)	;Ansprung
;
	DW	7F7FH
	DB	'COLOR',1
COLR	CALL	CON		;**0F**
	CALL	COLRC
	JP	COFF
 
CSTBT	PUSH	HL		;**42**
	LD	HL,STBT
	SET	3,M
	CALL	OCHR	;Steuerzeichen
	RES	3,M	;darstellen
	POP	HL
	RET
;
	DW	7F7FH
	DB	'DISPLAY',1
DISP	CALL	CON		;**3B**
	CALL	DISPC
JCOFF	JP	COFF
;
LDAME	CALL	COFF	;f}r DISPC
	LD	A,M	;und MODIC
JCON	JP	CON
;
	DW	7F7FH
	DB	'MODIFY',1
MODI	CALL	CON		;**2E**
	CALL	MODIC
	JR	JCOFF
 
LDMAE	CALL	COFF	;f}r MODIC
	LD	M,A
	JR	JCON
;
	DW	7F7FH
	DB	'WINDOW',1
	AND	A	;kein Arg?
	JR	Z,WINAK	;Aufruf No. 0
	DEC	A	;1 Arg?
	JR	Z,WINAKI;Aufruf No.L
	LD	H,L
	LD	L,C
	LD	D,E
	LD	A,(ARG49)
	LD	E,A
	LD	A,(ARGN)
	CP	4
	JR	NZ,WIN1	;PROGRAMMFEHLER
	JR	C,WIN3	;(vertauscht!)
	XOR	A
	JR	WIN2
 
WIN1	LD	A,(ARG49+2);Init No. A
WIN2	CALL	WININ
WIN3	JP	C,ERRM
	RET
 
WININ	CALL	CON		;**3C**
	CALL	WININC
	JP	COFF
 
WINAKI	LD	A,L
WINAK	CALL	WCOPY		;**3D**
	LD	(WINNR),A
	CALL	PART	;Fenster A
	RET	C	;holen
	EX	DE,HL
	LD	DE,WINON
	JR	WCOP1
 
PART	;Testet A und berechnet Pos.
	;in Fenstervektorspeicher =DE
	ADD	0F6H
	RET	C
	ADD	A
	LD	E,A
	ADD	A
	ADD	A
	ADD	E
	LD	E,A
	LD	D,0B9H
	AND	A
	RET
 
WCOPY	;Fenster (WINNR) retten
	PUSH	AF
	LD	A,(WINNR)
	CALL	PART
	JP	C,IAD2
	POP	AF
	LD	HL,WINON
WCOP1	LD	BC,10
	LDIR
	RET
 
NCHAR	INC	HL
KEYUP	LD	A,M	;f}r KEY
	AND	A
	RET	Z
	CALL	CSTBT
	JR	NCHAR
;
	DW	7F7FH
	DB	'KEY',1
	CP	1
	RET	NZ
	LD	A,L
KEY	AND	A		;**3A**
	RET	Z	;KEY 0
	CP	0DH	;nur F-Tasten
	RET	NC	;1..C
	LD	B,A
	LD	HL,0B900H
KEY1	LD	A,M	;B Dummys
	INC	HL	;ausz{hlen
	AND	A
	JR	NZ,KEY1
	DJNZ	KEY1
	CALL	KEYUP	;Belegung
			;pr{sentieren
KEY2	PUSH	AF
	DEC	HL
KEY3	POP	AF
KEY4	CALL	INTB
	CP	13H	;STOP-Ende
	JP	Z,CRLF
	PUSH	AF
	LD	B,0
	DEC	A	;CLR?
	JR	NZ,KEY5
	LD	A,M	;Leere Belegung
	AND	A	;nicht l|schen!
	JR	Z,KEY3
	LD	D,H	;DE=Ziel
	LD	E,L
	PUSH	HL
	INC	HL	;HL=Quelle
	LD	A,9CH
	SUB	L	;L{nge, B=0
	LD	C,A	;Pufferrest
	LDIR		;vorziehen
	POP	HL
	POP	AF
	CALL	OCHR
	JR	KEY2
 
KEY5	LD	A,(0B99AH);vorletztes
	AND	A	;Zeichen<>0?
	JR	NZ,KEY3	;ja-voll!
	PUSH	HL
	LD	A,9BH
	SUB	L
	LD	C,A	;L{nge, B=0
	LD	HL,0B99AH
	LD	DE,0B99BH;Pufferrest
	LDDR		;hinterziehen
	POP	HL
	POP	AF
	INC	HL	;Zeichen
	LD	M,A	;eintragen
	CALL	CSTBT	;& anzeigen
	JR	KEY4
;
	DW	7F7FH
	DB	'KEYLIST',1
KEYLI	CALL	CON		;**39**
	CALL	KEYLIC
	JP	COFF
 
CUCP	;nur f}r Klotz-Cursor!
	PUSH	DE		;**25**
	PUSH	AF
	PUSH	HL
	CALL	PADR0
	JR	C,CUCP2	;au~erhalb
	PUSH	BC
	LD	B,8
CUCP1	LD	A,M
	CPL		;compeln
	LD	M,A
	INC	L
	DJNZ	CUCP1	;8*
	POP	BC
CUCP2	POP	HL
	POP	AF
	POP	DE
	RET
 
BITT	LD	HL,(HOR)
	LD	A,L
	AND	7	;Bit-Nr.
	SRL	H
	RR	L
	SRL	H
	RR	L
	SRL	H
	RR	L	;/8
	ADD	0F8H	;Position in
	LD	C,A	;BITTAB
	LD	B,0FDH
	LD	A,(VERT)
	CPL		;unten =0/0
	LD	H,A
	LD	A,(BC)	;Bit
	JP	PADR
 
PUSE	PUSH	HL		;**30**
	PUSH	DE
	PUSH	BC
	AND	A	;Kennung CY=0
	PUSH	AF
	LD	HL,(HOR)
	LD	DE,(VERT)
	LD	D,0
	JP	PUSEJ
 
PUDE	CALL	CON		;**2F**
	CALL	PUDEC
	JP	COFF
 
CIRCL	LD	A,(ARG3)	;**3F**
	LD	C,A
	LD	L,C
	XOR	A
	LD	B,8
	RR	L
CIR1	JR	NC,CIR2
	ADD	C
CIR2	RRA
	RR	L
	DJNZ	CIR1
	LD	H,A	;Achtelkreis
	LD	D,H	;berechnen
	LD	E,L
	LD	A,C
	SLA	C
	RL	B
	DEC	BC
	PUSH	BC
	LD	BC,1
	EXX
	LD	C,A
	LD	B,0
CIR3	CALL	CIRUP	;und jeweils 8
	CALL	CIRUP	;Punkte setzen
	EXX
	AND	A
	SBC	HL,BC
	INC	BC
	INC	BC
	SBC	HL,DE
	ADD	HL,DE
	EXX
	JR	NC,CIR4
	EXX
	EX	(SP),HL
	EX	DE,HL
	AND	A
	SBC	HL,DE
	DEC	DE
	DEC	DE
	EX	DE,HL
	EX	(SP),HL
	EXX
	DEC	C
CIR4	INC	B
	LD	A,C
	CP	B
	JR	NC,CIR3
	POP	AF
	RET
 
CIRUP	;setzt 4 Punkte um Mittelpunkt
	LD	A,B
	LD	B,C
	LD	C,A
	XOR	A
	LD	D,A
	LD	HL,(ARG2)
	PUSH	HL
	LD	E,C
	ADD	HL,DE
	CALL	CIRUP1
	XOR	A
	LD	D,A
	POP	HL
	LD	E,C
	SBC	HL,DE
CIRUP1	PUSH	HL
	LD	HL,(ARG1)
	LD	E,B
	AND	A
	SBC	HL,DE
	POP	DE
	CALL	SHLDE
	PUSH	DE
	LD	D,A
	LD	E,B
	ADD	HL,DE
	ADD	HL,DE
	POP	DE
	;
SHLDE	;setzt Punkt auf Pos. HL/DE
	;(wenn m|glich)
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
PUSEJ	LD	A,L
	AND	7	;Bitnummer
	ADD	BITTAB%256
	LD	C,A
	LD	B,BITTAB/256
	LD	A,L
	SRL	H
	RRA
	SRL	H	;Bytespalte
	RRA
	SRL	H
	RRA
	CP	28H	;HL>13Fh?
	JR	NC,POP4
	OR	80H
	LD	H,A	;im IRM
	LD	A,0FFH
	ADD	D	;D>0? (DE>0FFh?)
	JR	C,POP4
	XOR	E
	LD	L,A
	LD	A,(FARB)
	LD	D,A
	LD	A,(BC)	;BITTAB lesen
	BIT	3,(IX+1);HiRes?
	JR	Z,HIRES
	BIT	1,D	;L|schen?
	JR	NZ,CPLA
	BIT	0,D	;XOR?
	JR	NZ,XORA
	OR	M	;Punkt setzen
XORJ	LD	M,A
	LD	A,(IX+1)
	LD	E,A
	XOR	2	;Farbebene
	DI
	OUT	84H
	LD	A,M
	AND	7
	OR	D
	LD	M,A	;neue vFarbe
	LD	A,E
	OUT	84H
	EI
POP4	POP	AF
	POP	BC
	POP	DE
	POP	HL
	RET
 
XORA	XOR	M	;Punkt setzen/
	RES	0,D	;r}cksetzen
	JP	XORJ
 
CPLA	CPL
	AND	M	;Punkt l|schen
	LD	M,A
	JP	POP4
 
HIRES	LD	B,A
	OR	M	;Bit setzen
	BIT	3,D	;Farbe rot?
	JR	NZ,HIR1
	XOR	B	;wieder l|schen
HIR1	LD	M,A
	LD	A,(IX+1)
	LD	C,A
	XOR	2
	DI
	OUT	84H	;Farbebene
	LD	A,B
	OR	M	;setzen
	BIT	4,D	;Farbe t}rkis?
	JR	NZ,HIR2
	XOR	B	;l|schen
HIR2	LD	M,A
	LD	A,C
	OUT	84H
	EI
	JR	POP4
 
LINE	CALL	CON		;**3E**
	CALL	LINEC
	JR	COF
 
MBOUT	CALL	CON		;**38**
	CALL	MBOC
	JR	COF
 
MBIN	CALL	CON		;**37**
	CALL	MBIC
	JR	COF
 
JIOERR	CALL	COFF	;Sprung zu
	LD	HL,(IOERR);?IO ERROR
	CALL	IRMOF
	JP	(HL)
;
	DW	7F7FH
	DB	'MODUL',1
ESC5	CALL	CON
	CALL	MODULC
COF	JP	COFF
;
	DW	7F7FH
	DB	'SYSTEM',1
ESC6	CALL	CON
	CALL	SYSTC
	JR	COF
 
BASPV	;BASIC-I/O-Verteiler
	PUSH	HL
	PUSH	BC
	CALL	IRMON
	BIT	5,E
	PUSH	DE
	JR	NZ,BASBYE
	INC	HL
	INC	HL
	BIT	7,E
	JR	NZ,BSA2
	PUSH	HL
	LD	A,E
	AND	7
	LD	HL,BUPTAB
	ADD	L
	LD	L,A
	LD	A,D
	LD	D,E
	LD	E,M	;UP-Nummer
	POP	HL
	CALL	PV3	;Aufruf
	POP	DE
	LD	D,A
	LD	A,E
	AND	4FH
	XOR	43H
	JR	NZ,BSA1
	CALL	OSTR
	DB	'VERIFY ?(Y):',0
	CALL	KBD
	CP	'Y'
	CALL	Z,VERIF
BSA1	LD	A,D
	RES	3,E
	CALL	IRMOF
	POP	BC
	POP	HL
	RET
 
BUPTAB	;H-Teil konstant!
	DB	16H	;INTB
	DB	24H	;OCHR
	DB	37H	;MBIN
	DB	38H	;MBOUT
	DB	06H	;USIN1
	DB	02H	;UOT1
	DB	07H	;USIN2
	DB	03H	;UOT2
 
BSA2	CALL	KBDS
	POP	DE
	LD	D,A
	JR	NC,BSA1
	RES	7,E
	JR	BSA1
 
BASBYE	LD	A,(35EH)
	AND	A
	JR	Z,BASB1
	CALL	IRMOF
	CALL	0C641H	;NEW
	CALL	IRMON
BASB1	IN	A,88H
	AND	5FH	;BASIC-ROM aus
	OUT	88H
	JP	LOOP
 
ZKOUT	LD	A,M		;**45**
	INC	HL
	AND	A
	RET	Z
	CALL	OCHR
	JR	ZKOUT
 
TON	CALL	LARG		;**35**
TON1	BIT	1,(IX+8);noch alter Ton?
	JR	NZ,TON1
TON2	PUSH	BC
	LD	C,8CH	;CTC0
	CALL	TON4	;starten
	INC	C	;CTC1
	EX	DE,HL
	CALL	TON4	;starten
	POP	BC
	LD	A,C	;Lautst{rke
	XOR	1FH	;da nullaktiv
	OR	81H	;maskieren
	LD	C,A
	LD	A,B
	AND	A	;Dauerton?
	JR	Z,TON3
	RES	7,C	;nein, Blink aus
	SET	1,(IX+8);neuer Ton
	LD	A,0C7H	;EI,Z{hler 50Hz
	OUT	8EH	;CTC2
	LD	A,B	;Dauer
	OUT	8EH
TON3	LD	B,60H
TONB	IN	A,89H
	AND	B	;RAM8 maskieren
	OR	C
	OUT	89H	;Ltst. ausgeben
	RET
 
TON4	;CTC (C) programmieren mit HL
	LD	A,L
	AND	A	;Ton?
	LD	L,3
	JR	Z,TON6	;kein Ton
	LD	L,A
	LD	A,7	;Zeitgeber
	BIT	0,H	;Vorteiler?
	JR	Z,TON5	;nein
	OR	20H
TON5	OUT	(C),A
TON6	OUT	(C),L
	RET
 
ESC1	LD	HL,0B200H
	LD	(VRAM),HL
	LD	A,(IX+1)
	AND	0F8H
OUT84	OUT	84H,A
	LD	(IX+1),A
	RET
 
ESC2	LD	HL,0AD00H
	LD	(VRAM),HL
	LD	A,(IX+1)
	OR	5
	JR	OUT84
 
ESC3	LD	HL,0AD00H
	LD	(VRAM),HL
	LD	A,(IX+1)
	AND	0FEH
	OR	4
	JR	OUT84
 
ESC4	LD	HL,0B200H
	LD	(VRAM),HL
	LD	A,(IX+1)
	OR	1
	AND	0F9H
	JR	OUT84
 
ESC7	LD	HL,STBT
	LD	A,M
	XOR	4
	LD	M,A
	RET
 
ESC9	LD	A,(IX+1)
	XOR	2
	JR	OUT84
 
ESCA	IN	A,89H
	XOR	80H	;Blinkbit
	OUT	89H
	LD	A,(IX+1)
	XOR	8
	JR	OUT84
;
	DW	7F7FH
	DB	'V24OUT',1
V24OUT	;Druckerroutine		 **47**
	CALL	CON
	CALL	V24OC
	LD	A,0DH
	CALL	PRINT
	LD	A,0AH
	CALL	PRINT
COF1	JP	COFF
 
HCPGM	;Reaktion auf ShCLR
	CALL	CON
	CALL	HCPGMC
	JR	COF1
;
	DW	7F7FH
	DB	'V24DUP',1
V24DUP	;Duplexroutine		 **48**
	CALL	CON
	CALL	V24DC
	JR	COF1
 
INIDUP	CALL	CON
	CALL	UMINIT
	JR	COF1
 
ECHO	CALL	CRT
PRINT	PUSH	AF
	LD	A,(HCPZ)
	BIT	7,A
	JR	Z,PRINT3;Drucker
	AND	0F0H	;Schreibmasch.
	CP	90H
	JR	Z,S3004	;S3004
	CP	0A0H
	JR	NZ,PRINT3
	POP	AF	;S6005
	PUSH	HL
	PUSH	BC
	LD	HL,ZIBM
	LD	BC,7
	CPIR
	JR	NZ,PRINT2
	LD	C,6
	ADD	HL,BC
	LD	A,M
	JR	PRINT2
 
S3004	POP	AF
	CP	7EH
	JR	NZ,PRINT4
	LD	A,83H	;~
	JR	PRINT4
 
PRINT2	POP	BC
	POP	HL
	PUSH	AF
PRINT3	POP	AF
PRINT4	CP	9	;CUR
	JR	Z,PRINT5
	PUSH	AF
	LD	A,(STBT)
	BIT	3,A	;Steuerzeichen
	JR	Z,OFILT1;ausf}hren
	POP	AF
	CP	7FH	;und 7Fh
	JR	NZ,PRINT6
PRINT5	LD	A,20H	;nach SPC
PRINT6	CP	20H	;und Steuerzei-
	JR	NC,OFILT;chen nach '_'
	LD	A,'_'	;wandeln
OFILT	;Ausgabe ohne Filter
	PUSH	AF
OFILT1	PUSH	BC
	CALL	LCSO
	LD	A,0AH
	ADD	C
	LD	C,A
	IN	A,(C)
	BIT	2,A	;bereit?
	JR	NZ,OFILT2
	LD	A,1
	CALL	WAIT	;nein, warten
	POP	BC
	JR	OFILT1
	;
OFILT2	POP	BC
	POP	AF
	PUSH	BC
	PUSH	AF
	CALL	LCSO
V24PUT	LD	A,8
	ADD	C
	LD	C,A
	POP	AF
	OUT	(C),A	;Ausgabe
	POP	BC
	RET
 
V24OT	PUSH	AF	;wie OFILT
V24OT1	PUSH	BC
	CALL	LCSOD
	LD	A,0AH
	ADD	C
	LD	C,A
	IN	A,(C)
	BIT	2,A
	JR	NZ,V24OT2
	LD	A,1
	CALL	WAIT
	POP	BC
	JR	V24OT1
V24OT2	POP	BC
	POP	AF
	PUSH	BC
	PUSH	AF
	CALL	LCSOD
	JR	V24PUT
 
V24IN	PUSH	BC
	PUSH	HL
	PUSH	DE
	CALL	LCSOD
	PUSH	BC
	LD	A,0AH
	ADD	C
	LD	C,A
	IN	A,(C)
	BIT	0,A	;bereit?
	JR	NZ,V24IN2
	LD	A,5	;WR5
	OUT	(C),A
	LD	A,0EAH	;DTR on, 8 bit,
	OUT	(C),A	;Senderfreigabe
V24IN1	IN	A,(C)
	BIT	0,A	;jetzt bereit?
	JR	NZ,V24IN2
	CALL	BRKT
	JR	C,V24IN4
	JR	V24IN1
 
V24IN2	LD	A,5	;WR5
	OUT	(C),A
	LD	A,6AH	;DTR off usw.
	OUT	(C),A
	POP	BC
	LD	A,8
	ADD	C
	LD	C,A
	IN	A,(C)	;einlesen
V24IN3	POP	DE
	POP	HL
	POP	BC
	RET
 
V24IN4	POP	BC
	JR	V24IN3
 
LCSO	;Lade C mit SIO-Offset f.V24OUT
	PUSH	AF
	LD	A,(HCPZ)
LCSO1	BIT	2,A
	LD	C,1
	JR	NZ,LCSOE
	DEC	C
LCSOE	POP	AF
	RET
 
LCSOD	;dto. f}r V24DUP
	PUSH	AF
	LD	A,(HCPZ2)
	JR	LCSO1
 
ISRSB	;ISR SIO B f}r MC-Load oder
	;Fremd-Tastatur
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	IN	A,9	;1. Zeichen
	PUSH	AF
	LD	A,18H	;Kanal
	OUT	0BH	;r}cksetzen
	LD	A,5	;WR5
	OUT	0BH
	LD	A,6AH	;DTR off
	OUT	0BH
	POP	AF
	CALL	ISB8
	CP	0DH	;Fremdtastatur?
	JR	Z,ISB3
	CP	1BH	;MC-Programm?
	JR	Z,ISB4
ISB1	CALL	INIDUP	;nichts
ISB2	JP	POPS4
 
ISB3	CALL	INIDUP
	LD	HL,ISB9	;neue ISR
	DI
	LD	(1E2H),HL
	EI
	JR	ISB2
 
ISB4	LD	A,6
	LD	(HCPZ2),A
	CALL	V24DUP
	CALL	V24IN
	CP	'T'
	JR	Z,ISB5	;MC laden
	CP	'U'
	JR	Z,ISB7	;MC starten
	JR	ISB1
 
ISB5	CALL	V24IN
	LD	L,A
	CALL	V24IN
	LD	H,A
	CALL	V24IN
	LD	C,A
	CALL	V24IN
	LD	B,A
ISB6	CALL	V24IN
	LD	M,A
	INC	HL
	DEC	BC
	LD	A,C
	OR	B
	JR	NZ,ISB6
	JR	ISB1
 
ISB7	CALL	V24IN
	LD	L,A
	CALL	V24IN
	LD	H,A
	CALL	JPHL
	JR	ISB1
 
ISB8	EI
	RETI
 
ISB9	;ISR SIO-B f}r Fremdtastatur
	EI
	PUSH	AF
	IN	A,9
	JP	TST7
 
CON	;CAOS-ROM C on
	PUSH	AF
	SET	7,(IX+4)
	LD	A,(IX+4)
	OUT	86H
	POP	AF
	RET
 
COFF	;CAOS-ROM C off
	PUSH	AF
	RES	7,(IX+4)
	LD	A,(IX+4)
	OUT	86H
	POP	AF
	RET
 
MULT	;BA=D*C			 **41**
	CALL	CON
	CALL	MULC
	JP	COFF
 
SQR	CALL	CON		;**40**
	CALL	SQRC
	JP	COFF
 
ZIBM	DEFM	'{|}~'
	DEFM	'[\]'
	DEFB	84H
	DEFB	94H
	DEFB	81H
	DEFB	0E1H
	DEFB	8EH
	DEFB	99H
	DEFB	9AH
 
ZEI0	DEFB	0	;CRT
	DEFB	38H	;MBOUT
ZEI2	DEFB	2	;UOUT1
ZEI3	DEFB	3	;UOUT2
ZEI4	DEFB	4	;KBD
	DEFB	37H	;MBIN
	DEFB	6	;UIN1
	DEFB	7	;UIN2
 
L3TB	;ESC-Tabelle
	DEFW	ESC0	;Tabulator
	DEFW	ESC1	;Anz0	Zugr0
	DEFW	ESC2	;1	1
	DEFW	ESC3	;0	1
	DEFW	ESC4	;1	0
	DEFW	ESC5	;MODUL
	DEFW	ESC6	;SYSTEM
	DEFW	ESC7	;PixInvers o/o
	DEFW	ESC8	;ColSwap o/o
	DEFW	ESC9	;Farbebene o/o
	DEFW	ESCA	;HiRes o/o
 
SUTB	DEFW	CRT	;00
	DEFW	MBO	;01
	DEFW	UOUT1	;02
	DEFW	UOUT2	;03
	DEFW	KBD	;04
	DEFW	MBI	;05
	DEFW	UIN1	;06
	DEFW	UIN2	;07
	DEFW	ISRO	;08
	DEFW	CSRO	;09
	DEFW	ISRI	;0A
	DEFW	CSRI	;0B
	DEFW	KBDS	;0C
	DEFW	BYE	;0D
	DEFW	KBDZ	;0E
	DEFW	COLR	;0F
	DEFW	LOAD	;10
	DEFW	VERIF	;11
	DEFW	LOOP	;12
	DEFW	NORM	;13
	DEFW	WAIT	;14
	DEFW	LARG	;15
	DEFW	INTB	;16
	DEFW	INLIN	;17
	DEFW	RHEX	;18
	DEFW	ERRM	;19
	DEFW	HLHX	;1A
	DEFW	HLDE	;1B
	DEFW	AHEX	;1C
	DEFW	ZSUCH	;1D
	DEFW	SOUT	;1E
	DEFW	SIN	;1F
	DEFW	NOUT	;20
	DEFW	NIN	;21
	DEFW	GARG	;22
	DEFW	OSTR	;23
	DEFW	OCHR	;24
	DEFW	CUCP	;25
	DEFW	MODU	;26
	DEFW	JUMP	;27
	DEFW	LDMA	;28
	DEFW	LDAM	;29
	DEFW	BRKT	;2A
	DEFW	SPACE	;2B
	DEFW	CRLF	;2C
	DEFW	HOME	;2D
	DEFW	MODI	;2E
	DEFW	PUDE	;2F
	DEFW	PUSE	;30
	DEFW	SIXD	;31
	DEFW	DABR	;32
	DEFW	TCIF	;33
	DEFW	PADR	;34
	DEFW	TON	;35
	DEFW	SAVE	;36
	DEFW	MBIN	;37
	DEFW	MBOUT	;38
	DEFW	KEY	;39
	DEFW	KEYLI	;3A
	DEFW	DISP	;3B
	DEFW	WININ	;3C
	DEFW	WINAK	;3D
	DEFW	LINE	;3E
	DEFW	CIRCL	;3F
	DEFW	SQR	;40
	DEFW	MULT	;41
	DEFW	CSTBT	;42
	DEFW	INIEA	;43
	DEFW	INIME	;44
	DEFW	ZKOUT	;45
	DEFW	MENU	;46
	DEFW	V24OUT	;47
	DEFW	V24DUP	;48
 
KTAB	DEFM	'Ww'
	DEFM	'Aa'
	DEFM	'2"'
	DEFW	1908H
	DEFW	0C10H
	DEFM	'-='
	DEFW	0F8F2H
	DEFM	'Yy'
	DEFM	'Ee'
	DEFM	'Ss'
	DEFM	'3#'
	DEFM	'^]'
	DEFW	0F01H
	DEFM	':*'
	DEFW	0F9F3H
	DEFM	'Xx'
	DEFM	'Tt'
	DEFM	'Ff'
	DEFM	'5%'
	DEFM	'Pp'
	DEFW	21FH
	DEFM	'0'
	DEFB	40H
	DEFW	0FBF5H
	DEFM	'Vv'
	DEFM	'Uu'
	DEFM	'Hh'
	DEFM	'7'
	DEFB	27H
	DEFM	'Oo'
	DEFW	141AH
	DEFM	'9)'
	DEFW	0303H
	DEFM	'Nn'
	DEFM	'Ii'
	DEFM	'Jj'
	DEFM	'8('
	DEFM	' ['
	DEFM	'Kk'
	DEFM	',<'
	DEFW	1B13H
	DEFM	'Mm'
	DEFM	'Zz'
	DEFM	'Gg'
	DEFM	'6&'
	DEFW	0
	DEFM	'Ll'
	DEFM	'.>'
	DEFW	0FCF6H
	DEFM	'Bb'
	DEFM	'Rr'
	DEFM	'Dd'
	DEFM	'4$'
	DEFM	'_\'
	DEFM	'+;'
	DEFM	'/?'
	DEFW	0FAF4H
	DEFM	'Cc'
	DEFM	'Qq'
	DEFW	1616H
	DEFM	'1!'
	DEFW	120AH
	DEFW	110BH
	DEFW	1809H
	DEFW	0F7F1H
	DEFW	0D0DH
 
ISRTAB	DEFW	ISRSB	;SIO B:Empfang
	DEFW	ISRPA	;PIO A:Kassette
	DEFW	ISRPB	;PIO B:Tastatur
	DEFW	INTEND	;CTC 0:frei
	DEFW	ISRC1	;CTC 1:Kassette
	DEFW	ISRC2	;CTC 2:Tondauer
	DEFW	ISRC3	;CTC 3:Tastatur
 
CRTTAB	DEFW	NOT	;00
	DEFW	CLR	;01
	DEFW	CLLN	;02
	DEFW	NOT	;03=BRK
	DEFW	NOT	;04
	DEFW	NOT	;05
	DEFW	NOT	;06
	DEFW	BEEP	;07
	DEFW	CUL	;08
	DEFW	CUR	;09
	DEFW	CUD	;0A
	DEFW	CUU	;0B
	DEFW	CLS	;0C
	DEFW	CBL	;0D
	DEFW	NOT	;0E
	DEFW	HCOPY	;0F
	DEFW	HOMEPG	;10
	DEFW	PAGE	;11
	DEFW	SCROL	;12
	DEFW	NOT	;13=STOP
	DEFW	CLICK	;14
	DEFW	NOT	;15
	DEFW	NOT	;16=CAPS
	DEFW	NOT	;17
	DEFW	CEL	;18
	DEFW	CBL	;19
	DEFW	INS	;1A
	DEFW	ESC	;1B
	DEFW	NOT	;1C=LIST
	DEFW	NOT	;1D=RUN
	DEFW	NL	;1E=CONT
	DEFW	DEL	;1F
 
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
 
BITTAB	DEFB	80H
	DEFB	40H
	DEFB	20H
	DEFB	10H
	DEFB	8
	DEFB	4
	DEFB	2
	DEFB	1
 
	END

