CF31    ASM                                                                                                                      
;**	CAOS 3.1 ROM F		**
 
	ORG	0F000H
 
POWER	JP	PWRON
	JP	PV1	;m. DEFB nr
	JP	PV2	;(ARGC)=nr
	JP	PV3	;E=nr
	JP	PV4	;E=nr, IRM e/a
	JP	RCALL	;m. DEFW offset
	JP	PWR3	;JUMP-Einsprung
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
	PUSH	AF
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
	EX	(SP),HL
	RET		;UP anspringen
 
PV2	AND	A	;VR: F
PV21	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	HL
	LD	HL,POP3	;Returnadresse
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
	OUT	88H
	LD	SP,(SYSP)
	EI
	LD	A,(IX+11)
	PUSH	BC
	RET		;wie JP (BC)
 
IRMOF	POP	BC	;analog IRMON
	LD	(IX+11),A
	IN	A,88H
	RES	2,A	;IRM aus
	DI
	OUT	88H
	LD	SP,IY
	EI
	LD	A,(IX+11)
	POP	IY
	PUSH	BC
	RET
 
RCALL	EX	(SP),HL	;VR: DE
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
 
PWRON	;Einschalt-Init
	LD	SP,1C4H
	XOR	A
	LD	L,A
	LD	H,A
	LD	B,A
	LD	C,A
PWR1	LD	M,A	;Speicher
	CPI		;l|schen
	JP	PE,PWR1
	LD	BC,0FF80H
	OTIR		;Module aus
	LD	B,8
	IN	A,(C)
	CP	0F4H	;M022 in 08?
	JR	NZ,PWR2
	LD	A,2
	LD	L,B
	LD	D,43H	;als RAM4
	CALL	MODU	;zuschalten
	CALL	ERAM4	;RAM4 l|schen
PWR2	LD	D,3
	LD	HL,IOTAB
	CALL	INIME	;Ports init
PWR3	DI
	CALL	SYSI	;IRM init
	LD	A,0CH
	CALL	CRT	;CLS
	LD	HL,WINON+1
	INC	M	;ab Zeile 1
	INC	HL
	INC	HL
	DEC	M	;bis Zeile 30
	DEC	M
	LD	BC,880H	;ROM-Modul mit
	IN	A,(C)	;Strukturbyte
	DEC	A	;01?
	JR	NZ,MEN0	;nein
	LD	L,B
	LD	D,43H	;auf 4000H
	LD	A,2	;zuschalten
	CALL	MODU
	JP	4000H	;anspringen
 
	DW	7F7FH
	DB	'MENU',1
MEN0	CALL	OSTR
	DB	0CH
	DB	'* HC-CAOS 3.1 *'
	DB	LF,CR,0
MEN1	LD	HL,0
	LD	C,L
	LD	B,H
MENU	CALL	OSTR		;**46**
	DB	CR,2,'%',0
	CALL	BRKT
	JR	C,LOOP1
	LD	A,(IX+9)
MEN2	CPIR
	JP	PO,LOOP1;ausgesucht
	CPI		;2. Mal?
	JR	NZ,MEN2
MEN3	LD	A,M
	CP	2	;Epilog?
	JR	C,MEN4
	CP	30H	;<30h?
	JR	C,MENU
	CP	5FH	;>5Fh, Kleinb.?
	JR	NC,MENU
	CALL	OCHR
	INC	HL
	DEC	BC
	JR	MEN3
MEN4	CALL	CRLF
	JR	MENU
 
LOOP	CALL	OSTR		;**12**
	DB	'%',0	;Prompt
LOOP1	CALL	INLIN
	INC	DE	;Zeichen danach
	LD	A,(DE)
	CP	' '
	JR	Z,LOOP
	AND	A
	JR	Z,LOOP
	LD	A,(IX+9)
	CALL	MSUCH
	JR	NC,LOOP2;nicht gef.
	PUSH	HL
	CALL	GARG	;Argumente
	JR	NC,LOOP3
	POP	HL
LOOP2	CALL	ERRM
	JR	LOOP
LOOP3	LD	HL,LOOP	;R}ckkehradr.
	EX	(SP),HL	;kellern
	PUSH	HL
	CALL	LARG
	EX	(SP),HL
	LD	A,M	;Epilog
	AND	A
	JR	NZ,LOOP4
	IN	A,88H
	RES	2,A	;IRM off
	OUT	88H
LOOP4	INC	HL	;Epilog
	EX	(SP),HL	;}bergehen
	RET
 
MSUCH	LD	BC,0
	LD	HL,0
ZSUCH	PUSH	AF		;**1D**
ZS1	POP	AF
	PUSH	AF
ZS2	CPIR
	JP	PO,ZS6	;ausgesucht
	CPI
	JR	NZ,ZS2	;falsch
	PUSH	DE
ZS3	LD	A,(DE)
	INC	DE
	CP	'!'	;keine SPC und
	JR	C,ZS4	;Steuerzeichen
	CPI
	JR	Z,ZS3
	POP	DE
	JR	ZS1
ZS4	LD	A,1
	CP	M	;Epilog?
	JR	NC,ZS5
	POP	DE
	JR	ZS1
ZS5	POP	AF
	POP	AF
	SCF		;CY=1
	RET
ZS6	POP	AF
	AND	A	;CY=0
	RET
 
OSTR	EX	(SP),HL		;**23**
	CALL	ZKOUT
	EX	(SP),HL
NOT	RET
 
NORM	CALL	NIN		;**13**
NOUT	LD	HL,ZEI0		;**20**
SOUT	PUSH	HL		;**1E**
	LD	HL,(OUTAB)
	EX	(SP),HL
	LD	(OUTAB),HL
	POP	HL
	RET
 
NIN	LD	HL,BUPTAB	;**21**
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
	PUSH	AF
	LD	A,M
	LD	(ARGC),A
	POP	AF
	CALL	PV2
POP3	POP	BC
	POP	DE
	POP	HL
	RET
 
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
 
KBD	CALL	CUCP		;**04**
	PUSH	HL
	PUSH	BC
	BIT	6,(IX+8);F-Taste aktiv?
	JR	NZ,KBD6;ja
KBD1	CALL	KBDZ
	JR	NC,KBD1
	CP	0F1H	;F-Taste?
	JR	C,KBD4
	SET	6,(IX+8);ja - merken
	SUB	0F0H
	LD	B,A
	LD	HL,0B900H
KBD2	LD	A,M	;Pos. in F-
	AND	A	;Puffer best.
	INC	HL
	JR	NZ,KBD2
	DJNZ	KBD2
	LD	A,M	;erstes CHR
	AND	A
	JR	Z,KBD7	;F-Taste frei
	INC	HL
KBD3	LD	(FTAST),HL;Zeiger
KBD4	BIT	5,(IX+8);Click ein?
	JR	Z,KBD5
	LD	HL,19H
	LD	BC,0FH
	LD	E,H
	PUSH	AF
	CALL	TON1	;Tastenclick
	LD	A,0FH
	CALL	WAIT
	LD	A,3
	OUT	8CH	;Ton aus
	POP	AF
KBD5	POP	BC
	POP	HL
	JP	CUCP
	;
KBD6	CALL	BRKT	;Abbruch?
	JR	NC,KBD8
KBD7	RES	6,(IX+8);F-Taste inaktiv
	JR	KBD1
	;
KBD8	LD	HL,(FTAST);Zeiger auf
	LD	A,M	;n{chstes CHR
	INC	HL
	AND	A	;Ende?
	JR	NZ,KBD3
	JR	KBD7
 
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
	DW	01C4H	;SYSP
	DW	SUTB	;SUTAB
	DW	CRTTAB	;CTAB
	OUT	88H	;BJUMP
	JP	0F012H
	DW	ZEI0	;OUTAB
	DW	ZEI4	;INTAB
	JP	NOT	;UOUT1
	JP	NOT	;UIN1
	JP	NOT	;UOUT2
	JP	NOT	;UIN2
	DW	0DC82H	;IOERR (BASIC)
 
INLIN	PUSH	HL		;**17**
INLI1	CALL	INTB
	CALL	OCHR
	CP	CR	;Enter?
	JR	NZ,INLI1
	CALL	CRLF
	LD	DE,(CURSO)
	CALL	DABR
	LD	DE,-40
	ADD	HL,DE
	EX	DE,HL
	POP	HL
	RET
 
HLHX	LD	A,H		;**1A**
	CALL	AHEX
	LD	A,L
	CALL	AHEX
SPACE	LD	A,20H		;**2B**
SPAC0	JP	OCHR
 
ERRM	CALL	OSTR		;**19**
	DB	'ERROR',7,0
	;
CRLF	CALL	OSTR		;**2C**
	DB	CR,LF,0
	RET
 
HOME	LD	A,10H		;**2D**
	JR	SPAC0
 
AHEX	PUSH	AF		;**1C**
	RRA		;Tetradentausch
	RRA
	RRA
	RRA
	CALL	AHEX0	;erst rufen,
	POP	AF	;dann reinlaufen
AHEX0	AND	0FH	;Maske
	ADD	30H
	CP	3AH
	JR	C,SPAC0	;0..9
	ADD	7
	JR	SPAC0	;A..F
 
HLDE	CALL	HLDE1		;**1B**
HLDE1	CALL	HLHX
	EX	DE,HL
	RET
 
RHEX	LD	A,(DE)		;**18**
	CP	' '	;Trennzeichen
	INC	DE	;}bergehen
	JR	Z,RHEX
	DEC	DE
	LD	HL,0
	LD	(NUMVX),HL
	XOR	A
	LD	HL,NUMNX
	LD	M,A
RH1	LD	A,(DE)
	OR	A	;Ende-Dummy?
	RET	Z
	CP	' '	;Ende-Space?
	RET	Z
	SUB	'0'
	RET	C
	CP	10
	JR	C,RH2
	SUB	7
	CP	10
	RET	C
	CP	10H
	JR	C,RH2
	SCF
	RET
RH2	INC	DE
	INC	M	;NUMNX erh|hen
	INC	HL
	RLD		;Tetrade
	INC	HL	;durchschieben
	RLD
	DEC	HL
	DEC	HL	;RLD setzt auch
	JR	Z,RH1	;Z-Flag!
	DEC	DE
	SCF		;Zahl zu gro~
	RET
 
GARG	;bis 10 Arg. erfassen	;**22**
	LD	BC,ARGN
	XOR	A
	LD	(BC),A	;(ARGN)=0
GARG1	CALL	RHEX
	RET	C	;Fehler
	LD	A,M
	OR	A
	RET	Z	;Ende-Dummy
	INC	HL
	INC	BC
	LD	A,M	;L(NUMVX)
	LD	(BC),A	;L(Argument)
	INC	HL
	INC	BC
	LD	A,M	;H(NUMVX)
	LD	(BC),A	;H(Argument)
	LD	HL,ARGN
	INC	M
	LD	A,M
	CP	11	;10 Argumente?
	JR	C,GARG1
	SCF		;zu viele
	RET
 
SYSI	LD	DE,HCADR
	LD	HL,KCTAB
	LD	BC,32H
	LDIR		;IRM init
	XOR	A
	CALL	WINAK	;Fenster 0 init.
	LD	HL,WNDFN
	LD	DE,WNDFN+10
	LD	BC,90
	LDIR		;Fenster 1..9
	LD	A,1
	;
SIXD	LD	HL,ISRTAB	;**31**
	LD	(MIXIT),A
	LD	E,0E4H
	LD	D,A
	LD	A,E
	LD	BC,12	;Interrupttab.
	LDIR		;kopieren
	ADD	12
	LD	E,A
	PUSH	DE	;IX-Arbeitsber.
	POP	IX
	LD	(IX+8),88H
	LD	(IX+9),7FH
	LD	(IX+14),KTAB	;Low
	LD	(IX+15),KTAB/256;High
INIT	DI
	PUSH	AF
	PUSH	DE
	PUSH	HL
	IM	2
	LD	A,(MIXIT)
	LD	I,A
	LD	D,5
	LD	HL,IOTAB2
	CALL	INIME	;Kan{le init.
	POP	HL
	POP	DE
	POP	AF
	EI
	LD	(IX+5),CASS	;Low
	LD	(IX+6),CASS/256	;High
	RET
 
IOTAB	DB	8AH	;PIO-A-Steuerw.
	 DB	2
	  DB	0E4H	;Int-Vektor
	  DB	4FH	;Byte-IN
	DB	88H	;PIO-A-Daten
	 DB	1
	  DB	9FH	;IRM ein...
	DB	8AH	;PIO-A-Steuerw.
	 DB	2
	  DB	0FH	;Byte-OUT
	  DB	3	;DI
 
IOTAB2	DB	8BH	;PIO-B-Steuerw.
	 DB	3
	  DB	0FH	;Byte-OUT
	  DB	83H	;EI
	  DB	0E6H	;Int-Vektor
	DB	89H	;PIO-B-Daten
	 DB	1
	  DB	9FH	;Daten
	DB	8CH	;CTC0
	 DB	1
	  DB	0E8H	;Int-Vektor
	DB	8EH	;CTC2 (blinken)
	 DB	2
	  DB	47H	;DI,Z{hler,RES
	  DB	14H	;Zeitkonstante
	DB	8AH	;PIO-A-Daten
	 DB	1
	  DB	0E4H	;Int-Vektor
;
	DW	7F7FH
	DB	'SAVE',1
	CALL	OSTR
	DB	'NAME :',0
	CALL	INLIN
	LD	HL,6	;'NAME :'
	ADD	HL,DE	;}bergehen
	;
SAVE	LD	DE,CASS		;**36**
	LD	BC,11
	LDIR		;Name -> Puffer
	EX	DE,HL
	LD	M,0	;Abschlu~-Null
	LD	HL,ARGN
	LD	A,M
	CP	2	;mind 2 Arg.!
	RET	C
	LD	DE,CASS+10H
	LD	C,15H	;alle Argumente
	LDIR		;nach Puffer
	CALL	ISRO
	LD	HL,(ARG1)
SAV1	LD	(IX+5),L
	LD	(IX+6),H
	LD	A,(IX+2)
	CALL	AHEX	;Blocknummer
	CALL	OSTR
	DB	'  ',0
	CALL	BRKT
	LD	BC,0A0H
	JR	C,SAV2
	LD	DE,80H
	ADD	HL,DE
	LD	DE,(ARG2)
	AND	A
	SBC	HL,DE	;Ende erreicht?
	JR	NC,SAV2
	CALL	MBO
	JR	SAV1
	;
SAV2	CALL	CRLF
	JP	CSRO
 
BRKT	CALL	KBDS		;**2A**
	RET	NC
	CP	3	;Break?
	SCF
	RET	Z
	AND	A
	RET
 
	LD	(IX+9),7FH
LARG	LD	BC,(ARG3)	;**15**
	LD	DE,(ARG2)
	LD	HL,(ARG1)
	LD	A,(ARGN)
	RET
 
 
	DW	7F7FH
	DB	'VERIFY',1
VERIF	RES	0,(IX+7)	;**11**
	JR	LOAD0
 
	DW	7F7FH
	DB	'LOAD',1
LOAD	SET	0,(IX+7)	;**10**
LOAD0	CALL	ISRI
LOAD1	LD	A,(IX+2)
	JR	NC,LOAD3
LOAD2	CALL	AHEX
	CALL	OSTR
	DB	'*',CR,LF,0
	CALL	MBI0
	JR	LOAD1
	;
LOAD3	CP	2	;Block 0 oder 1?
	JR	NC,LOAD2
	INC	A
	LD	E,A
	LD	HL,CASS
	LD	B,11
LOAD4	LD	A,M	;Name anzeigen
	INC	HL
	CALL	CRT
	DJNZ	LOAD4
	LD	(IX+3),E;erwarteter Blk
	LD	A,(CASS+10H)
	BIT	0,(IX+7)
	JR	Z,LOAD6	;wenn VERIFY
	LD	(IX+4),A;Anz. Arg.
	CP	2
	JR	C,NOMC	;zu wenig Arg.
	CP	11
	JR	NC,NOMC	;zu viel Arg.
	LD	DE,(CASS+13H)
	LD	HL,(CASS+11H)
	LD	A,(ARGN)
	AND	A	;Ladeoffset?
	JR	Z,LOAD5
	LD	BC,(ARG1)
	ADD	HL,BC	;AAdr, EAdr
	EX	DE,HL	;umrechnen
	ADD	HL,BC
	EX	DE,HL
	BIT	2,(IX+4);4..7 Args?
	JR	NZ,LOAD5
	PUSH	HL
	LD	HL,(CASS+15H)
	ADD	HL,BC	;SAdr umrechnen
	LD	(CASS+15H),HL
	POP	HL
LOAD5	PUSH	HL
	PUSH	DE
	LD	HL,CASS+11H
	LD	DE,ARG1
	LD	BC,20	;alle Argumente
	LDIR		;zur}ck kopieren
	POP	DE
	POP	HL
	CALL	SPACE
	CALL	HLDE	;AAdr, Eadr
LOAD6	CALL	CRLF
	;Load-Hauptschleife
LOAD7	CALL	BRKT
	BIT	7,(IX+7);COM-File?
	JR	Z,LOAD8
	JP	C,POWER	;BRK -> POWER
LOAD8	JR	C,CLJP	;BRK -> Abbruch
	CALL	MBI0
	LD	A,(IX+2);Blocknr.
	PUSH	AF
	CP	(IX+3)
	JR	Z,LOAD9	;erwarteter Blk.
	INC	A
	JR	Z,LOAD9	;letzter Blk.
	POP	AF
	CALL	AHEX	;Blocknummer
	CALL	OSTR
	DB	'* ',0
	JR	LOAD7
	;
NOMC	CALL	OSTR
	DB	'???',19H,LF,0
CLJP	LD	(IX+4),2;kein Start!
	JR	CSRI
	;
LOAD9	POP	AF
	JR	C,LOAD14;Fehler
	PUSH	AF
	CALL	AHEX
	CALL	OSTR
	DB	'> ',0
	POP	AF
LOAD10	BIT	0,(IX+7)
	JR	Z,LOAD15;wenn VERIFY
	PUSH	DE
	EX	DE,HL
	SBC	HL,DE
	LD	BC,81H
	SBC	HL,BC
	ADD	HL,BC
	DEC	BC
	JR	NC,LOAD11
	LD	C,L
LOAD11	LD	HL,CASS	;MC in Speicher
	LDIR		;kopieren
	EX	DE,HL
	POP	DE
	JR	C,CSRI	;fertig!
LOAD12	INC	(IX+3)	;n{chster Block
LOAD13	JR	LOAD7
	;
LOAD14	BIT	7,(IX+7)
	JR	NZ,LOAD7;COM-File!
	CALL	AHEX	;Blocknummer
	CALL	OSTR	;fehlerhaft
	DB	' ?',19H,LF,0
	BIT	0,(IX+7)
	JR	Z,LOAD7	;wenn VERIFY
	CALL	OSTR
	DB	'REPEAT (N)?',LF,CR,0
	RES	6,(IX+8)
	CALL	KBD
	CP	'N'
	JR	NZ,LOAD13
	LD	(IX+4),2;kein Start
	JR	LOAD10
	;
LOAD15	INC	A
	JR	NZ,LOAD12
CSRI	CALL	CSROI	;Abschlu~
	CALL	CRLF
	BIT	0,(IX+7)
	RET	Z	;VERIFY
	LD	A,(IX+4)
	LD	(ARGN),A
	CP	3	;Autostart?
	LD	(IX+4),2
	RET	C	;nein
	LD	HL,(ARG3)
	JP	(HL)	;Start
 
 
	DW	7F7FH
	DB	'COLOR',1
COLR	LD	A,1FH
	AND	L
	RLA
	RLA
	RLA
	LD	L,A
	LD	A,(ARGN)
	CP	2
	JR	NC,COLR2
	LD	A,(COLOR)
COLR1	AND	7
	OR	L
	LD	(COLOR),A
	RET
COLR2	LD	A,E
	JR	COLR1
 
SUTB	;Unterprogrammtabelle
	DW	CRT	;00
	DW	MBO	;01
	DW	UOUT1	;02
	DW	UOUT2	;03
	DW	KBD	;04
	DW	MBI	;05
	DW	UIN1	;06
	DW	UIN2	;07
	DW	ISRO	;08
	DW	CSRO	;09
	DW	ISRI	;0A
	DW	CSRI	;0B
	DW	KBDS	;0C
	DW	BYE	;0D
	DW	KBDZ	;0E
	DW	COLR	;0F
	DW	LOAD	;10
	DW	VERIF	;11
	DW	LOOP	;12
	DW	NORM	;13
	DW	WAIT	;14
	DW	LARG	;15
	DW	INTB	;16
	DW	INLIN	;17
	DW	RHEX	;18
	DW	ERRM	;19
	DW	HLHX	;1A
	DW	HLDE	;1B
	DW	AHEX	;1C
	DW	ZSUCH	;1D
	DW	SOUT	;1E
	DW	SIN	;1F
	DW	NOUT	;20
	DW	NIN	;21
	DW	GARG	;22
	DW	OSTR	;23
	DW	OCHR	;24
	DW	CUCP	;25
	DW	MODU	;26
	DW	JUMP	;27
	DW	LDMA	;28
	DW	LDAM	;29
	DW	BRKT	;2A
	DW	SPACE	;2B
	DW	CRLF	;2C
	DW	HOME	;2D
	DW	MODI	;2E
	DW	PUDE	;2F
	DW	PUSE	;30
	DW	SIXD	;31
	DW	DABR	;32
	DW	TCIF	;33
	DW	PADR	;34
	DW	TON	;35
	DW	SAVE	;36
	DW	MBIN	;37
	DW	MBOUT	;38
	DW	KEY	;39
	DW	KEYLI	;3A
	DW	DISP	;3B
	DW	WININ	;3C
	DW	WINAK	;3D
	DW	LINE	;3E
	DW	CIRCL	;3F
	DW	SQR	;40
	DW	MULT	;41
	DW	CSTBT	;42
	DW	INIEA	;43
	DW	INIME	;44
	DW	ZKOUT	;45
	DW	MENU	;46
 
LDMA	LD	M,A		;**28**
	RET
 
LDAM	LD	A,M		;**29**
	RET
 
	DW	7F7FH
	DB	'MODIFY',1
MODI0	CALL	LARG
MODI	LD	A,0BFH		;**2E**
	CP	H
	RET	C	;ab C000h gesp.
	LD	(ARG1),HL
	CALL	HLHX	;Adresse
	LD	A,M
	CALL	AHEX	;Datenbyte
	CALL	OSTR
	DB	' ',8,8,8,0
MODI1	CALL	INLIN
	CALL	RHEX	;Adresse
	JR	C,MODI9	;Fehler
	LD	A,M
	OR	A
	JR	Z,MODI9	;kein Zeichen
	INC	HL
	PUSH	DE
	LD	E,M
	INC	HL
	LD	D,M
	EX	DE,HL	;HL=Adresse
	POP	DE
	LD	B,0
MODI2	PUSH	HL
	CALL	RHEX	;Datenbyte
	LD	A,M
	OR	A
	JR	Z,MODI4	;kein Zeichen
	INC	HL
	LD	A,M
	POP	HL
MODI3	LD	M,A	;eintragen
	INC	B
	INC	HL
	PUSH	HL
MODI4	POP	HL
	LD	A,(DE)
	CP	'.'	;Abschlu~?
	RET	Z
	CP	','	;1 Zeichen
	JR	NZ,MODI5
	INC	DE
	LD	A,(DE)	;vom Video-RAM
	INC	DE
	JR	MODI3	;eintragen
	;
MODI5	LD	A,B
	OR	A
	JR	NZ,MODI6
	INC	HL
MODI6	LD	A,(DE)
	CP	':'	;R}ckschritt?
	JR	NZ,MODI7
	DEC	HL
	DEC	HL
	JR	MODI
	;
MODI7	CP	'/'	;neue Adresse?
	JR	NZ,MODI8
	INC	DE
	CALL	RHEX	;Adresse
	JR	C,MODI9	;Fehler
	LD	HL,(NUMVX)
	JR	MODI
	;
MODI8	OR	A	;Ende-Dummy?
	JR	Z,MODI
	CP	' '
	JR	Z,MODI2	;noch ein Byte
MODI9	CALL	ERRM
	JR	MODI0
 
JPTAB	;Sprungtabelle f}r CRT-Routinen
	DW	CLR	;01
	DW	CAPS	;16
	DW	CLICK	;14
	DW	BEEP	;07
	DW	HCOPY	;0F
	DW	CBL	;0D
	DW	CLS	;0C
	DW	CUL	;08
	DW	DEL	;1F
	DW	CUD	;0A
	DW	INS	;1A
	DW	CBL	;19
	DW	CLLN	;02
	DW	CUU	;0B
	DW	CUR	;09
	DW	HOMEPG	;10
	DW	SCROL	;12
	DW	PAGE	;11
 
CCTAB:	;CCTL-Tabelle
	;Aufbau: Zeichen, Offset, CCTL
	DB	0E0H,0C0H,CCTL3
	DB	0A0H,0A0H,CCTL2
	DB	080H,080H,CCTL3
	DB	060H,040H,CCTL1
	DB	020H,020H,CCTL0
	DB	000H,000H,CCTL1
 
TCIF	;Test Cursor im Fenster	 **33**
	LD	A,(WINLG)
	DEC	A
	SUB	E	;Cursor-Spalte
	RET	C
	LD	A,(WINLG+1)
	DEC	A
	SUB	D	;Cursor-Zeile
	RET
 
CRT	PUSH	HL		;**00**
	PUSH	DE
	PUSH	BC
	PUSH	AF
	PUSH	AF
	LD	A,(STBT)
	LD	DE,(CURSO)
	LD	L,A
	POP	AF
	CP	20H	;Steuerfunktion?
	JR	NC,CRT1
	BIT	3,L	;darstellen?
	JR	NZ,CRT1
	LD	HL,(CTAB)
	LD	BC,18
	CPIR		;Funktion def.?
	JR	NZ,CRT3;nein
	LD	HL,JPTAB
	ADD	HL,BC
	ADD	HL,BC
	LD	C,M
	INC	HL
	LD	H,M
	LD	L,C	;HL=Sprungadr.
	CALL	JPHL
	JR	CRT2
	;
CRT1	CALL	PCHR
CRT2	LD	(CURSO),DE
CRT3	POP	AF
	JP	POP3
	;
PCHR	PUSH	AF
	CALL	DABR
	POP	AF
	LD	M,A	;ASCII eintragen
	CALL	WPIX	;darstellen
CUR	INC	E
	LD	A,(WINLG)
	CP	E
	RET	NZ
	LD	E,0
CUD	INC	D
	LD	A,(WINLG+1)
	CP	D
	RET	NZ
	LD	HL,(WEND)
JPHL	JP	(HL)	;PAGE o. SCROLL
 
HCOPY	LD	HL,(HCADR)	;ShCLR
	JP	(HL)
 
SCRLPG	LD	DE,0	;Cursor li/oben
	CALL	DABR
	INC	D
	PUSH	HL
	CALL	DABR	;n{chste Zeile
	POP	DE
	JR	C,SCRL9
	LD	A,(WINLG)  ;Spalten
	LD	C,A
	LD	A,(WINLG+1);Zeilen
	DEC	A
	LD	B,0
SCRL1	PUSH	HL
	PUSH	BC
	LDIR		;im VRAM eine
	POP	BC	;Zeile rollen
	POP	DE
	LD	HL,40	;1 Zeile tiefer
	ADD	HL,DE
	DEC	A
	JR	NZ,SCRL1
	LD	A,(WINON+1)
	RLCA
	RLCA
	RLCA
	LD	H,A	;Zeile*8
	LD	A,(WINLG+1)
	DEC	A
	RLCA
	RLCA
	RLCA		;L{nge*8
	PUSH	AF
	LD	A,(WINON);Spalte
	CP	32
	LD	C,A
	LD	A,(WINLG);Breite
	JR	NC,SCRL2 ;rechter Teil
	ADD	C
	CP	32
	JR	C,SCRL3
	LD	B,32
	SUB	B
	LD	D,C
	LD	C,A
	LD	A,B
	JR	SCRL4
SCRL2	LD	B,C
	LD	C,A
	LD	DE,0
	JR	SCRL5
SCRL3	LD	D,C
	LD	BC,0
SCRL4	SUB	D
	LD	E,A
SCRL5	POP	AF
SCRL6	PUSH	AF
	LD	A,E
	AND	A
	JR	Z,SCRL7
	LD	L,D
	CALL	SCRLUP
SCRL7	LD	A,C
	AND	A
	JR	Z,SCRL8
	LD	L,B
	CALL	SCRLUP
SCRL8	INC	H
	POP	AF
	DEC	A
	JR	NZ,SCRL6
SCRL9	LD	A,(WINLG+1)
	DEC	A
	LD	D,A
	JR	CLLN
 
CLR	PUSH	DE
	CALL	DABR
	LD	A,' '	;Leerzeichen
	LD	M,A
	CALL	WPIX
	POP	DE
CUL	LD	A,E	;Cursor links
	SUB	1
	JR	C,CUL1
	LD	E,A
	RET
CUL1	LD	A,D	;Zeile hoch
	SUB	1
	RET	C
	LD	D,A
	LD	A,(WINLG)
	DEC	A
	LD	E,A
	RET
 
CLLN	LD	E,0	;ShDEL
	PUSH	DE
	CALL	DABR
	LD	A,(WINLG)
	LD	B,A	;Spalten
CLLN1	LD	M,0	;VRAM l|schen
	LD	A,' '
	CALL	WPIX	;Space anzeigen
	INC	HL
	DJNZ	CLLN1
	POP	DE
	RET
 
CLS	LD	D,0	;ShHOME
	LD	A,(WINLG+1)
CLS1	PUSH	AF
	PUSH	DE
	CALL	CLLN	;Zeile f}r
	POP	DE	;Zeile l|schen
	POP	AF
	INC	D
	DEC	A
	JR	NZ,CLS1
HOMEPG	LD	D,0
CBL	LD	E,0
	RET
 
CUU	LD	A,D
	SUB	1
	RET	C
	LD	D,A
	RET
 
SCROL	LD	HL,SCRLPG	;ShCUD
WADR	LD	(WEND),HL
	RET
 
PAGE	LD	HL,HOMEPG	;ShCUU
	JR	WADR
 
DEL	CALL	DABR
	LD	A,M
	AND	A	;Ende?
	RET	Z
	PUSH	DE	;Cursor
	PUSH	HL	;VRAM
DEL1	INC	E	;n{chste Spalte
	CALL	DABR	;raus?
	JR	NC,DEL2	;nein
	LD	E,0	;Spalte 0 in
	INC	D	;neuer Zeile
	CALL	DABR	;raus?
	JR	C,DEL3	;ja, Ende
DEL2	LD	A,M
	AND	A	;Ende?
	JR	Z,DEL3
	EX	(SP),HL
	LD	M,A	;eintragen
	CALL	WPIX	;zeichnen
	JR	DEL1	;von vorn
DEL3	POP	HL
	LD	M,0	;letztes CHR
	LD	A,' '	;l|schen
	CALL	WPIX
	POP	DE
	RET
 
INS	PUSH	DE	;Cursor
	LD	A,' '
	CALL	DABR
INS1	LD	B,M	;altes
	LD	M,A	;neues Zeichen
	CALL	WPIX	;zeichnen
	LD	A,B
	AND	A	;Dummy?
	JR	Z,INS2
	INC	E	;CUR
	CALL	DABR	;raus?
	JR	NC,INS1	;nein
	LD	E,0	;Spalte 0 in
	INC	D	;neuer Zeile
	CALL	DABR	;raus?
	JR	NC,INS1	;nein
	POP	DE
	RET
	;
INS2	PUSH	HL
	INC	E	;CUR
	CALL	DABR	;raus?
	JR	NC,INS3	;nein
	LD	E,0
	INC	D
	CALL	DABR
	JR	C,DEL3
INS3	LD	A,M
	AND	A
	JR	NZ,DEL3
	POP	HL
	POP	DE
	RET
 
CLICK	LD	A,(IX+8)	;ShINS
	XOR	20H
XOR8	LD	(IX+8),A
	RET
 
CAPS	LD	A,(IX+8)
	XOR	80H
	JR	XOR8
 
CRTTAB	;Bildschirmsteuercodes
	DB	11H	;PAGE
	DB	12H	;SCROL
	DB	10H	;HOME
	DB	09H	;CUR
	DB	0BH	;CUU
	DB	02H	;CLLN
	DB	19H	;CBL
	DB	1AH	;INS
	DB	0AH	;CUD
	DB	1FH	;DEL
	DB	08H	;CUL
	DB	0CH	;CLS
	DB	0DH	;ENTER
	DB	0FH	;HCOPY
	DB	07H	;BEEP
	DB	14H	;CLICK
	DB	16H	;SHLOK
	DB	01H	;CLR
 
FADR	;Berechnet Farbadresse
	;PE:	HL=VRAM-Adresse
	;PA:	HL=Farbadresse
	LD	BC,-VRAM
	ADD	HL,BC
	LD	B,0
	LD	DE,40	;Zeilenbreite
FADR1	INC	B
	XOR	A
	SBC	HL,DE
	JR	NC,FADR1
	ADD	HL,DE
	ADD	B
	DEC	A
	RLA
	RLA
	RLA
	LD	H,A
	RET
 
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
	DB	16H	;INTB	#0
	DB	24H	;OCHR
	DB	37H	;MBIN	#1
	DB	38H	;MBOUT
	DB	06H	;USIN1	#2
	DB	02H	;UOT1
	DB	07H	;USIN2	#3
	DB	03H	;UOUT2
 
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
BASB1	JP	LOOP
 
MBOUT	;Byteweise MB-Ausgabe	;**38**
	LD	E,D	;Steuerbyte
	LD	D,A	;Datenbyte
	PUSH	DE
	PUSH	BC
	BIT	3,E
	JR	Z,NOINIT
	SET	1,(IX+7);Init merken
	LD	A,'U'+80H
	CP	M
	PUSH	DE
	LD	DE,0A0H	;Vortonl{nge
	JR	NZ,NOHIU;bei 'U'
	LD	DE,400H	;gedehnt
NOHIU	LD	(VORTN),DE
	LD	DE,CASS
	LD	BC,11
	LDIR		;Name
	EX	DE,HL
	POP	DE
	LD	M,D	;1. Byte
	LD	A,74H	;noch Bytes
MBOUT1	CALL	SETDT
MBOUTE	POP	BC
	POP	DE
	RES	3,E
	RET
 
NOINIT	LD	HL,(DTADR)
	LD	M,D	;Byte eintragen
	BIT	6,E
	JR	NZ,MCLOS
	LD	A,(DTNR)
	DEC	A
	JR	NZ,MBOUT1
	BIT	1,(IX+7);schon Init?
	JR	NZ,MBOUT2;nein
	CALL	MBLO	;ja-Blockout
	CALL	SETDTN
	JR	MBOUTE
 
MBOUT2	RES	1,(IX+7);r}cksetzen
	CALL	ISRO	;Init
	CALL	MBLNR	;Blocknr.
	CALL	SETDTN
JMBOUTE	JR	MBOUTE
 
MCLOS	BIT	1,(IX+7);Init?
	JR	Z,MBOUT3
	CALL	ISRO	;aber jetzt!
	JR	MBOUT4
 
MBOUT3	CALL	MBLO	;Blockout
MBOUT4	LD	BC,(VORTN)
	CALL	CSRO
MBOUT5	PUSH	DE
	CALL	CSRI
	RES	5,(IX+7);'U'-Merker
	POP	DE	;r}cksetzen
	JR	JMBOUTE
 
MBIN	;Byteweise MB-Eingabe	;**37**
	LD	E,D	;Steuerbyte
	LD	D,A	;Datenbyte (PA!)
	PUSH	DE
	PUSH	BC
	BIT	6,E	;Close?
	JR	NZ,MBOUT5
	BIT	3,E	;Init?
	JR	Z,MBIU
	LD	A,1	;erwarteter Blk
	LD	(IX+3),A
	PUSH	HL
	CALL	ISRI
MBIN1	POP	HL
	JP	C,JBLERR;Fehler
	LD	A,(IX+2);gelesener Blk.
	CP	1
	JP	NZ,JBLERR;Fehler
	RES	2,(IX+7);Kopfblk geladen
	INC	(IX+3)	;erw. Block
	LD	DE,CASS
	LD	B,11
	LD	A,(DE)
	CP	'U'+80H	;hohes 'U'?
	JR	NZ,MBIN2
	SET	5,(IX+7);Merker zur
 ;Blocknummernausgabeunterdr}ckung
MBIN2	ADD	29H	;gesch}tzte
	JR	NC,MBIN3;BASIC-
	LD	A,(DE)	;Programme
	SUB	4	;als unge-
	LD	(DE),A	;sch}tzt
	INC	DE	;anzeigen
	LD	(DE),A	;und Setzen
	INC	DE	;LIST/EDIT/BYE-
	LD	(DE),A	;Schutz
	LD	(35EH),A;<=hier
	SET	6,(IX+7);RESET-Schutz
	LD	DE,CASS
MBIN3	LD	A,(DE)
	CALL	MBCRT	;anzeigen
	LD	A,(DE)
	CP	M	;stimmt Name?
	JR	Z,MBIN4
	SET	2,(IX+7);nein-merken
MBIN4	INC	HL
	INC	DE
	DJNZ	MBIN3
	LD	A,LF
	CALL	MBCRT
	LD	A,CR
	CALL	MBCRT
	BIT	2,(IX+7)
	JP	NZ,JIOERR ;?IO ERROR
	EX	DE,HL
	LD	A,75H	;noch Bytes
MBIN5	LD	D,M	;auszulesen
	CALL	SETDT
	LD	A,D
	POP	BC
	POP	DE
	RES	3,E
	RET
 
MBIU	LD	HL,(DTADR)
	LD	A,(DTNR)
	DEC	A
	JR	NZ,MBIN5
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
MBIU2	CALL	MBCRT	;nicht
	CALL	BLNR	;erwartet
	CALL	JBRKT
	JR	MBIU1
 
MBIU3	INC	(IX+3)	;n{chsten
	DEC	A	;erwarteten
	BIT	5,(IX+7)
	JR	NZ,MBIU4
	CALL	BLNR	;Blocknr.
	LD	A,'>'
	CALL	MBCRT
	LD	A,' '
	CALL	MBCRT
MBIU4	CALL	SETDTN
	LD	HL,(DTADR)
	JR	MBIN5
 
MERR	LD	A,'?'	;Fehler in Block
	JR	MBIU2
 
JBLERR	LD	A,'*'	;nicht erw. Blk.
	CALL	MBCRT
	CALL	JBRKT
	CALL	MBI
	PUSH	HL
	JP	MBIN1
 
BLNR	LD	A,(IX+2)
	PUSH	AF	;siehe AHEX!
	RRA
	RRA
	RRA
	RRA
	CALL	BLNR1
	POP	AF
BLNR1	AND	0FH
	ADD	'0'
	CP	'9'+1
	JR	C,BLNR2
	ADD	7
BLNR2	CALL	CRT
	RET
 
	PUSH	AF	;Space-Ausgabe
	LD	A,' '	;nicht benutzt!
CRTA	CALL	CRT
	POP	AF
	RET
 
MBCRT	PUSH	AF
	JR	CRTA
 
MBLO	LD	BC,(VORTN)
	CALL	MBO
MBLNR	CALL	BLNR	;AHEX
	LD	A,'>'
	CALL	MBCRT
	LD	A,' '
	CALL	MBCRT
JBRKT	CALL	BRKT	;BRK?
	RET	NC
JIOERR	LD	HL,(IOERR)
	JP	(HL)
 
SETDTN	;Setzen Datenzeiger neu
	LD	A,80H
	LD	HL,CASS
	JR	SETDT1
 
SETDT	;n{chsten Datenzeiger setzen
	INC	HL
SETDT1	LD	(DTADR),HL
	LD	(DTNR),A
	RET
 
 
	DW	7F7FH
	DB	'DISPLAY',1
DISP	LD	A,(ARGN)	;**3B**
	CP	3
	JR	NC,DISP1
	LD	C,8	;default
DISP1	PUSH	BC
DISP2	CALL	HLHX	;Adresse
	LD	A,0BFH
	CP	H
	JR	C,DISP6	;C000H...
	PUSH	HL
	LD	B,8	;8 Byte
DISP3	LD	A,M
	CALL	AHEX	;hex anzeigen
	DEC	B
	JR	Z,DISP4
	CALL	SPACE
	INC	HL
	JR	DISP3
DISP4	LD	A,9	;CUR
	CALL	OCHR
	CALL	CSTBT
	POP	HL
	LD	B,8	;8 Byte
DISP5	LD	A,M
	CALL	OCHR	;ASCII anzeigen
	INC	HL
	DJNZ	DISP5
	CALL	CSTBT
	CALL	CRLF
	CALL	BRKT
	JR	C,DISP6	;BRK
	PUSH	DE
	EX	DE,HL
	SBC	HL,DE	;EAdr erreicht?
	EX	DE,HL
	POP	DE
	JR	C,DISP6
	DEC	C
	JR	NZ,DISP2
	CALL	KBD	;Warten Taste
	CP	3	;BRK?
	POP	BC
	RET	Z
	CP	13H	;STOP?
	JR	NZ,DISP1
	CALL	OSTR
	DB	0BH,9,9,9,9,9,0
	JP	MODI1	;-> MODIFY
 
DISP6	POP	BC
	RET
 
CSTBT	PUSH	AF
	LD	A,(STBT)
	XOR	8
	LD	(STBT),A
	POP	AF
	RET
 
KEYUP	LD	HL,0B900H
	LD	B,A
KUP1	LD	A,M
	AND	A	;Dummys
	INC	HL	;ausz{hlen
	JR	NZ,KUP1
	DJNZ	KUP1
KUP2	LD	A,M
	AND	A	;Ende-Dummy?
	RET	Z
	INC	HL
	CALL	CSTBT
	CALL	OCHR	;anzeigen
	CALL	CSTBT
	JR	KUP2
 
	DW	7F7FH
	DB	'KEYLIST',1
KEYLI	LD	C,1		;**39**
KEYL1	LD	A,'F'
	CALL	OCHR	;F
	LD	A,C
	CALL	AHEX0	;Nr.
	CALL	OSTR
	DB	' : ',0
	LD	A,C
	CALL	KEYUP	;pr{sentieren
	CALL	CRLF
	INC	C
	LD	A,13
	CP	C	;F1-FC
	RET	Z
	JR	KEYL1
 
	DW	7F7FH
	DB	'KEY',1
	LD	A,L
KEY	AND	A		;**3A*
	RET	Z	;0
	CP	13
	RET	NC	;>12
	CALL	KEYUP	;pr{sentieren
	LD	DE,0B99BH
	EX	DE,HL
	AND	A
	SBC	HL,DE
	RET	C
	EX	DE,HL
	LD	C,E	;BC=L{nge bis
	LD	B,D	;Pufferende
KEY1	CALL	KBD
	AND	A
	JR	Z,KEY1	;keine Taste
	CP	13H
	JP	Z,CRLF	;STOP-Ende
	CP	1
	JR	Z,KEY2	;CLR
	PUSH	HL
	LD	HL,0B99AH;vorletztes
	INC	M	;Zeichen 0?
	DEC	M
	JR	NZ,KEY3;nein-voll!
	EX	(SP),HL
	CALL	CSTBT
	POP	HL
	LD	D,H
	LD	E,L
	INC	DE
	PUSH	BC	;Pufferrest
	LDDR		;hinterziehen
	POP	BC
	INC	HL
	LD	M,A	;eintragen
	INC	HL	;und
	CALL	CRT	;anzeigen
	CALL	CSTBT
	DEC	BC	;L{nge-1
	JR	KEY1
	;
KEY2	DEC	HL
	LD	A,M
	AND	A	;Leere Belegung
	INC	HL	;nicht l|schen!
	JR	Z,KEY1
	DEC	HL
	PUSH	HL
	INC	HL
	INC	BC	;L{nge+1
	PUSH	BC
	LD	D,H
	LD	E,L
	DEC	DE	;Pufferrest
	LDIR		;vorziehen
	POP	BC
	CALL	OSTR
	DB	8,' ',8,0
KEY3	POP	HL
	JR	KEY1
 
WININ	CP	10		;**3C**
	RET	NC	;Nr. zu gro~
	LD	B,A
	LD	A,L
	CP	40
	RET	NC	;Spalte zu gro~
	ADD	E
	CP	41
	RET	NC	;Ende zu gro~
	LD	A,H
	CP	32
	RET	NC	;Zeile zu gro~
	ADD	D
	CP	33
	RET	NC	;Ende zu gro~
	PUSH	BC
	PUSH	HL
	PUSH	DE
	LD	DE,WINNR
	CALL	WIN1
	EX	DE,HL
	LDIR		;Daten kopieren
	POP	HL
	LD	(WINLG),HL
	POP	HL
	LD	(WINON),HL
	POP	AF
	LD	(WINNR),A
	XOR	A
	LD	HL,CURSO
	LD	M,A	;Spalte=0
	INC	HL
	LD	M,A	;Zeile=0
	INC	HL
	LD	M,A	;STBT=0
	INC	HL
	LD	M,39H	;COLOR
	INC	HL
	LD	M,SCRLPG
	INC	HL	;WEND
	LD	M,SCRLPG/256
	SCF
	RET
 
WIN1	;Adresse von Fenstervektor ber.
	LD	A,(DE)	;Nummer
WIN2	LD	HL,WNDFN-10
	LD	BC,10	;L{nge Vektor
	INC	A
WIN3	ADD	HL,BC
	DEC	A
	JR	NZ,WIN3
	INC	DE	;DE=WINON
	RET
 
WINAK	CP	10		;**3D**
	RET	NC	;zu gro~
	PUSH	AF
	LD	DE,WINNR
	PUSH	DE
	CALL	WIN1
	EX	DE,HL	;akt. Vektor
	LDIR		;retten
	POP	DE
	POP	AF
	LD	(DE),A
	CALL	WIN2	;neuen Vektor
	LDIR		;holen
	SCF
	RET
 
LINE	;Linie zeichnen		;**3E**
; von (ARG1)/(ARG2) nach (ARG3)/(ARG4)
	LD	HL,(ARG3) ;XEND
	LD	BC,(ARG1) ;XANF
	LD	(HOR),BC	;X-Start
	AND	A
	SBC	HL,BC
	LD	BC,0
	JR	NC,LINE1;wenn XANF>=XEND
	DEC	BC	;r}ckl{ufig
LINE1	LD	(XDIR),BC
	LD	B,7
LINE2	SLA	L	;XSTEPS
	RL	H	;*128
	DJNZ	LINE2
	LD	(XCNT),HL
	LD	(XSTEP),HL
	LD	A,(ARG4) ;YEND
	LD	L,A
	LD	A,(ARG2) ;YANF
	LD	(VERT),A	;Y-Start
	LD	C,A
	LD	H,0
	LD	B,H
	XOR	A
	SBC	HL,BC
	JR	NC,LINE3
	DEC	A	;r}ckl{ufig
LINE3	LD	(YDIR),A
	LD	B,7
LINE4	SLA	L	;YSTEPS
	RL	H	;*128
	DJNZ	LINE4
	LD	(YSTEP),HL
	LD	(YCNT),HL
	LD	BC,200H
	;LINE-Hauptschleife
LINE5	CALL	PUSE	;Punkt setzen
	LD	HL,(XCNT)
	LD	DE,(XSTEP)
	ADD	HL,DE
	LD	(XCNT),HL
	LD	HL,(HOR)
	LD	DE,(XDIR)
	ADC	HL,DE
	LD	(HOR),HL;X neu
	LD	HL,(YCNT)
	LD	DE,(YSTEP)
	ADD	HL,DE
	LD	(YCNT),HL
	LD	A,(VERT);Y neu
	LD	E,A
	LD	A,(YDIR)
	ADC	E
	LD	(VERT),A
	DEC	BC
	LD	A,B
	OR	C
	JR	NZ,LINE5
	RET
 
CIRCL	;Kreis zeichnen		;**3F**
	;um (ARG1)/(ARG2)
	LD	A,(ARG3);Radius
	LD	C,A
	LD	D,182	;1/SQR(2) * 256
	CALL	MULT
	LD	A,B
	LD	(YCNT),A
	LD	D,C
	CALL	MULT
	LD	C,A
	LD	(XCNT),BC
	;CIRCLE-Hauptschleife
CIR1	LD	HL,(XCNT)
	LD	A,(YCNT)
	LD	C,A
	LD	D,A
	PUSH	DE	;Achtelkreis
	CALL	MULT	;berechnen
	LD	C,A
	AND	A
	SBC	HL,BC
	CALL	SQR
	POP	DE
	LD	E,A
	AND	A	;und jeweils 8
CIR2	CCF		;Punkte setzen
	PUSH	AF
	LD	HL,(ARG1)
	LD	B,0
	LD	C,D
	ADD	HL,BC
	LD	(HOR),HL
	LD	A,(ARG2)
	PUSH	AF
	ADD	E
	LD	(VERT),A
	CALL	PUSE	;1. Punkt
	POP	AF
	PUSH	AF
	SUB	E
	LD	(VERT),A
	CALL	PUSE	;2. Punkt
	SBC	HL,BC
	SBC	HL,BC
	LD	(HOR),HL
	CALL	PUSE	;3. Punkt
	POP	AF
	ADD	E
	LD	(VERT),A
	CALL	PUSE	;4. Punkt
	LD	A,E
	LD	E,D	;D/E vertauscht
	LD	D,A
	POP	AF
	JR	C,CIR2	;5. bis 8. Punkt
	LD	A,(YCNT)
	SUB	1
	LD	(YCNT),A
	JR	NC,CIR1	;weiter
	RET
 
SQR	;A=SQR(HL)		;**40**
	XOR	A
	LD	A,0FFH
	LD	E,A
	LD	D,A
SQR1	INC	A
	INC	DE
	INC	DE
	SBC	HL,DE
	JP	P,SQR1
	RET
 
MULT	;BA=C*D			;**41**
	LD	B,0
	LD	E,9
MUL1	LD	A,D
	RRA
	DEC	E
	RET	Z
	LD	D,A
	LD	A,B
	JR	NC,MUL2
	ADD	C
MUL2	RRA
	LD	B,A
	JR	MUL1	;8*
 
SCRLUP	PUSH	HL	;Scroll-UP
	PUSH	DE
	PUSH	BC
	PUSH	HL
	CALL	PADR
	POP	BC
	PUSH	HL	;Pixel-neu
	PUSH	DE	;Farbe-neu
	LD	L,A
	LD	A,8
	ADD	B
	LD	H,A
	LD	A,L
	LD	L,C
	CALL	PADR
	EX	(SP),HL	;Pixel-alt
	EX	DE,HL	;Farbe-alt
	LD	C,A
	LD	B,0
	PUSH	BC	;Breite
	LDIR		;Farbscrolling
	POP	BC
	POP	HL
	POP	DE
	LDIR		;Pixelscrolling
	JP	POP3
 
CUCP	PUSH	HL		;**25**
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	DE,(CURSO)
	CALL	DABR
	JR	C,CUCP2	;au~erhalb!
	CALL	FADR
	CALL	PADR
	LD	A,(DE)
	CPL		;1. Farbbyte
	LD	(DE),A
	SET	5,E
	LD	A,(DE)
	CPL		;2. Farbbyte
	BIT	7,(IX+8);CAPS?
	JR	NZ,CUCP1
	XOR	4
CUCP1	LD	(DE),A
CUCP2	JP	POP4
 
BEEP	LD	HL,30H	;CHR(7)
	LD	BC,0FH
	PUSH	DE
	LD	E,H	;E=0
	CALL	TON1
	LD	A,1EH	;Tondauer
	CALL	WAIT
	LD	A,3
	OUT	8CH	;Ton aus
	LD	A,10H
	POP	DE	;Cursor
	JP	WAIT	;Tonpause
 
ZEI0	DB	0	;CRT
	DB	38H	;MBOUT
	DB	2	;UOUT1
	DB	3	;UOUT2
ZEI4	DB	4	;KBD
	DB	37H	;MBIN
	DB	6	;UIN1
	DB	7	;UIN2
 
	END
�@�t(�p6�nTop of Text:  End of Text:  Start of MC:  ASM-Offset :  �ۈ�$ӈ����B	37H	;MBIN
	DB	6	;UIN1
	DB	7	;UIN2
 
	END
�@�t(�p6�nTop of Text:  End of Text:  Start of MC:  ASM-Offset :  �ۈ�$ӈ����