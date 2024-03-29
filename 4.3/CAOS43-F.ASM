CF43    ASM                                                                                                                      
;**	CAOS 4.3 ROM F		**
;	(17.02.95)
 
	ORG	0F000H
 
	JP	PWRON
	JP	PV1	;DEFB nr
	JP	PV2	;(ARGC)=nr
	JP	PV3	;E=nr
	JP	PV4	;E=nr, IRM e/a
	JP	RCALL	;DEFW offset
	JP	BYE	;JUMP-Einsprung
	JP	PV5	;E=nr, IRM/Stack
	JP	IRMON
	JP	IRMOF
	JP	PV6	;(ARGC)=nr
JDISK	CALL	CON
	CALL	JDISC	;DISK-Verteiler
	JP	COFF
 
PV6	PUSH	BC
	CALL	IRMON
	CALL	PV2
	JR	PV51
 
PV5	PUSH	BC
	CALL	IRMON
	CALL	PV3
PV51	CALL	IRMOF
	POP	BC
	RET
 
PV4	CALL	SCRON
	CALL	PV3
SCROFF	PUSH	AF
	IN	A,88H
	RES	2,A
	OUT	88H	;IRM aus
	POP	AF
	RET
 
SCRON	PUSH	AF
	IN	A,88H
	SET	2,A	;IRM ein
	OUT	88H
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
	ADD	IY,SP	;(Kein UP ver-
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
	;ohne Registerzerst|rung
	EX	(SP),HL
	PUSH	DE
	LD	E,M
	INC	HL
	LD	D,M	;DE=Offset
	INC	HL
	EX	DE,HL
	ADD	HL,DE	;addieren
	EX	(SP),HL	;Zieladresse
	EX	DE,HL	;DE von Stack
	DI
	INC	SP
	INC	SP
	EX	(SP),HL	;HL von Stack
	DEC	SP
	DEC	SP
	EI
	RET		;Ziel anspringen
 
;SWITCH und JUMP
 
SWI7F	DW	7F7FH
	DB	'SWITCH'
	DB	1
	AND	A
	JR	NZ,SWI
ESC5	CALL	CON
	CALL	MODULC	;Module listen
	JR	MCOFF
SWI	LD	D,E	;Steuerbyte
	DEC	L
	JR	NZ,SWI2	;nicht IRM
	CP	1
	JR	Z,SWI2	;nur lesen
	INC	L
	LD	H,0B8H	;IRM nur
	LD	M,D	;eintragen!
	LD	H,0FFH
	JR	SWI3
SWI2	INC	L
	CALL	MODU	;lesen/schalten
SWI3	LD	A,L
	CALL	AHSPC	;Platz
	LD	A,H
	CALL	AHSPC	;Kennbyte
	LD	A,D
	CALL	AHEX	;Steuerbyte
	JP	CRLF
 
MODU	;Lesen und Schalten	 **26**
	CALL	CON
	CALL	MODUC
MCOFF	JP	COFF
 
;Betriebssystem wechseln
JUMP7F	DW	7F7FH
	DB	'JUMP'
	DB	3
	LD	A,L
JUMP	LD	B,A		;**27**
	LD	C,80H
	IN	A,(C)
	INC	A	;Kennbyte FF?
	JR	NZ,JUM0
	CALL	OSTR
	DB	'Kein Modul!'
	DB	7,CR,LF,0
	RET
JUM0	PUSH	BC
	LD	H,0B8H
	LD	B,7	;ab Platz 8!
JUM1	INC	B
	JR	Z,JUM2	;bis FF
	IN	A,(C)
	CPL
	AND	70H	;Speichermodul?
	JR	NZ,JUM1
	OUT	(C),A	;aus
	LD	L,B
	LD	M,A
	JR	JUM1
JUM2	POP	BC
	LD	A,0FFH	;Ausgabe FF
	LD	L,B
	LD	M,A	;Eintrag
	DI
	OUT	(C),A	;einschalten
	IN	A,88H
	AND	7EH
	JP	BJUMP	;in IRM
 
PWRON	;Einschalt-Init
	LD	SP,1C4H
	LD	A,63H
	OUT	86H	;RAM4 on
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
	CALL	CON
	CALL	SYSI	;IRM-Init usw.
	XOR	A
PWR3	LD	(WINNR),A
	CALL	WCOPY	;Fenstervektor-
	INC	A	;speicher init.
	CP	10
	JR	C,PWR3
	CALL	ESC3
	LD	A,0CH	;Zweitbild
	CALL	CRT	;l|schen
	CALL	ESC1
	JR	PWR4
 
BYE	;Tasten-RESET
	LD	SP,1C4H
	CALL	CON
	CALL	SYSI	;IRM-Init usw.
	;
PWR4	CALL	V24INI	;M003 suchen
	JR	C,PWR5	;kein M003!
	LD	(ARGN),A
	CALL	V24OC	;Druckerinit
	CALL	UMINIT	;SIO B f}r
PWR5	CALL	COFF	;Interrupt
	LD	BC,880H	;ROM-Modul mit
	IN	A,(C)	;Strukturbyte
	DEC	A	;01?
	JR	NZ,PWR6	;nein
	LD	A,43H
	OUT	(C),A	;ein ab 4000H
	LD	(0B808H),A
	LD	A,(IX+4)
	AND	0FCH	;RAM4 aus
	LD	(IX+4),A
	OUT	86H
	JP	4000H	;anspringen
PWR6	LD	B,0FCH
	IN	A,(C)
	CP	0A7H	;Floppy?
	JR	NZ,MEN0	;nein
	OUT	(C),A	;einschalten
	LD	BC,0B3F3H
	IN	A,(C)	;Betriebsart
	DEC	A
	JR	Z,MEN0	;PC-BA
	CP	4
	JR	Z,MEN0	;KC-BA
	CALL	OSTR
	DB	0CH,LF
	DB	'Autostart Floppy'
	DB	CR,LF,0
	LD	A,1
	LD	(ARGN),A
	LD	A,0FCH
	CALL	JUMP	;Startversuch
	JR	MEN0
;
MENU7F	DW	7F7FH
	DB	'MENU'
	DB	1FH
	POP	HL	;Stack clear
MEN0	CALL	OSTR
	DB	0CH,LF
	DB	'* KC-CAOS 4.3 *',0
MENU	LD	HL,0C000H	;**46**
	LD	C,L
	LD	B,L	;BC=0
MEN1	CALL	CRLF
MEN2	LD	A,2	;CLLN
	CALL	CRT
	LD	A,(PROMPT)
	CP	20H
	JR	NC,MEN3
	LD	A,'%'	;Standard
MEN3	CALL	OCHR
	CALL	BRKT
	JR	C,LOOP2	;Break
	LD	A,(IX+9)
MEN4	CPIR
	JP	PO,LOOP2;ausgesucht
	CPI
	JR	NZ,MEN4	;2mal!
MEN5	LD	A,M
	CP	20H	;Epilog?
	JR	C,MEN1
	CP	30H
	JR	C,MEN2
	CP	60H
	JR	NC,MEN2
	CALL	OCHR
	INC	HL
	DEC	BC
	JR	MEN5
 
LOOP	;Eingabeschleife
	CALL	SCRON		;**12**
	CALL	COFF
	LD	A,(PROMPT)
	CP	20H
	JR	NC,LOOP1
	LD	A,'%'	;Standard
LOOP1	CALL	OCHR	;Prompt
LOOP2	CALL	INPUT
	JR	LOOP
 
INPUT	;Men}kommandoroutine
	CALL	INLIN
	JR	NC,INP1
	LD	A,0BH	;bei BRK
	CALL	CRT	;Cursor hoch
	LD	A,2	;und
	CALL	CRT	;Zeile l|schen
	RET
INP1	INC	DE	;Zeichen danach
	LD	A,(DE)
	AND	0DFH	;leer?
	RET	Z
	CALL	MSUCH	;Suche ab C000H
	JR	C,INP5	;gefunden
	LD	A,(0B805H)
	AND	1
	JR	NZ,INP2	;CAOS-C war an
	CALL	CON
	LD	B,20H
	CALL	MS1
	JR	C,INP5	;gefunden
INP2	PUSH	DE
	LD	A,(0B802H)
	LD	E,A	;ROM-C merken
	LD	D,0D1H	;BASIC nicht mit
INP3	LD	A,2
	LD	L,A
	CALL	MODU	;ROMC-Ebene ein
	RES	7,(IX+4),A
	OUT	86H	;CAOSC aus!
	EX	DE,HL
	EX	(SP),HL	;DE=Suchkette
	EX	DE,HL
	LD	B,20H
	CALL	MS1	;Men}wort?
	JR	C,INP4	;gefunden
	EX	DE,HL
	EX	(SP),HL	;DE=Schaltzust.
	EX	DE,HL
	LD	A,10H
	ADD	D
	LD	D,A
	JR	NC,INP3	;n{chste Ebene
	LD	D,E
	LD	A,2
	LD	L,A
	CALL	MODU	;ROMC wie zuvor
	POP	DE
	JR	ERR1	;invalid command
	;
INP4	POP	AF	;Schaltzustand
	XOR	A	;ROMC vergessen
	LD	(0B805H),A
INP5	LD	A,M	;Epilog
	INC	HL
	PUSH	HL
	CP	1FH
	RET	Z	;ohne arg!
	PUSH	AF
	CALL	GARG	;Arg's erfassen
	POP	BC
	JR	C,ERR4	;bad args
	LD	A,(ARGN)
	LD	C,B
	RES	4,B
	SRA	B
	CP	B
	JR	C,ERR3	;zu wenig arg
	JR	Z,INP6
	BIT	4,C
	JR	NZ,ERR2	;zu viele arg
INP6	BIT	0,C
	CALL	LARG	;Arg's laden
	RET	NZ	;mit IRM on
	JP	SCROFF	;mit IRM off
 
ERR1	CALL	OSTR
	DB	'falsches Kommando',7,0
	JR	ERRCR
ERR2	CALL	OSTR
	DB	'zu viele',0
	JR	ERRA
ERR3	CALL	OSTR
	DB	'zu wenig',0
	JR	ERRA
ERR4	CALL	OSTR
	DB	'fehlerhafte',0
ERRA	POP	HL	;Stack clear
	CALL	OSTR
	DB	' Argumente',7,0
ERRCR	JP	CRLF
 
MSUCH	LD	B,0
MS1	LD	HL,0C000H
	LD	C,L
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
	INC	DE
	CP	21H
	JR	C,ZS3	;Kettenende
	BIT	5,M	;gro~?
	CALL	Z,UPCASE
	CPI
	JR	Z,ZS1
ZS2	POP	DE
	POP	AF
	JR	ZSUCH
	;
ZS3	LD	A,M
	CP	20H	;Epilog?
	JR	C,ZS4	;ja
	JR	Z,ZS2	;SPC!
	CP	80H
	JR	NC,ZS2	;>80H
	INC	HL
	JR	ZS3
	;
ZS4	POP	AF
	POP	AF
	SCF		;gefunden
	RET
 
WAIT	LD	B,A		;**14**
	XOR	A
WAIT1	CALL	WTUP
	DJNZ	WAIT1
	RET
	;
WTUP	DEC	A	;UP f}r WAIT
	RET	Z
	PUSH	AF
	POP	AF
	JR	WTUP
 
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
POP3	POP	BC	;auch f}r PV2
	POP	DE	;bis PV6!
	POP	HL
	RET
 
KBD	PUSH	HL		;**04**
	PUSH	DE
	PUSH	BC
KBD1	BIT	6,(IX+8);F-Taste aktiv?
	JR	NZ,KBD9;ja
KBD2	LD	E,0
KBD3	CALL	CUCP	;blinken
	INC	E	;e/a-Merker
	LD	B,0FH	;F00h mal
KBD4	CALL	KBDZ	;abfragen
	JR	C,KBD5
	DEC	BC
	LD	A,B
	OR	C
	JR	NZ,KBD4	;dann Cursor
	JR	KBD3	;wieder weg usw.
	;
KBD5	BIT	0,E	;gerade ein?
	CALL	NZ,CUCP	;ja - wegnehmen
	LD	D,A
	BIT	5,(IX+8);Click ein?
	CALL	NZ,CLIK
	LD	A,D
	LD	HL,STBT
	BIT	4,M	;ESC aktiv?
	JR	Z,KBD6
	LD	DE,(CURSO)
	CALL	CRT1	;ESC ausf}hren
	LD	(CURSO),DE
	JR	KBD2
	;
KBD6	CP	ESC
	JR	NZ,KBD7
	SET	4,M	;ESC merken
	JR	KBD2
	;
KBD7	CP	0F1H	;F-Taste?
	JR	C,POP3
	AND	0FH
	LD	B,A
	LD	HL,0B900H
KBD8	LD	A,M
	INC	L
	JR	Z,KBD2	;Speicherende
	AND	A
	JR	NZ,KBD8;Pos. in Puffer
	DJNZ	KBD8	;ausz{hlen
	LD	A,L
	CP	9CH
	JR	NC,KBD2	;Speicherende
	SET	6,(IX+8);F-Taste aktiv
	JR	KBD10
	;
KBD9	CALL	BRKT	;Abbruch?
	JR	C,KBD12
	LD	HL,(FTAST);Zeiger auf
KBD10	LD	A,M	;n{chstes CHR
	CP	ESC
	JR	NZ,KBD11
	INC	HL
	LD	(FTAST),HL
	LD	HL,STBT
	SET	4,M	;ESC merken
	JR	KBD1
	;
KBD11	INC	HL
	LD	(FTAST),HL;Zeiger rett.
	AND	A	;Null-Ende?
	JR	NZ,KBD13
KBD12	RES	6,(IX+8);ja - inaktiv
	JP	KBD2
	;
KBD13	LD	HL,STBT
	BIT	4,M
	JP	Z,POP3
	LD	DE,(CURSO)
	CALL	CRT1	;ESC ausf}hren
	LD	(CURSO),DE
	JP	KBD1
 
INLIN	CALL	INTB		;**17**
	CALL	OCHR
	CP	3	;BRK?
	SCF
	JR	Z,INLI1
	CP	CR	;ENTER?
	JR	NZ,INLIN
INLI1	PUSH	AF
	CALL	CRLF	;PA: A=0
	LD	DE,(CURSO)
	CP	D	;Zeile 1 durch
			;PAGE-Mode?
	JR	NZ,INLI2
	LD	A,(WINLG+1)
	LD	D,A	;D korrigieren
INLI2	DEC	D	;Zeile dr}ber
	PUSH	HL
	CALL	DABR
	EX	DE,HL
	POP	HL	;PA: DE
	POP	AF	;PA: CY=BRK
NOOP	RET
 
GO7F	DW	7F7FH
	DB	'go'
	DB	3
	JP	(HL)
 
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
	DB	CR,LF,0
	RET
 
HOME	LD	A,10H		;**2D**
	JR	SPAC0
 
AHEX	PUSH	AF		;**1C**
	RRA		;Tetradentausch
	RRA
	RRA
	RRA
	CALL	AHEX0	;erst aufrufen,
	POP	AF	;dann reinlaufen
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
	LD	M,A	;alles nullsetz
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
	AND	0DFH	;Klein>Gro~
	CP	10
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
 
GARG	;bis 10 Argumente erfassen
	LD	BC,ARGN		;**22**
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
 
UPCASE	;A upcasen, wenn Kleinbuchstabe
	CP	'a'
	RET	C
	CP	'z'+1
	RET	NC
	AND	0DFH
	RET
 
SIXD	CALL	CON		;**31**
	CALL	SIXC
	CALL	COFF
	;
KPUFF	LD	(IX+5H),0
	LD	(IX+6H),CASS/256
	RET
 
INIEA	PUSH	BC		;**43**
	LD	C,M	;1.Byte=Adr.
	INC	HL
	LD	B,M	;2.Byte=Anz.
	INC	HL
	OTIR
	POP	BC
	RET
 
INIMEX	LD	D,M	;Anzahl
	INC	HL
INIME	DI			;**44**
INIME1	CALL	INIEA
	DEC	D	;D Kan{le
	JR	NZ,INIME1
	EI
	RET
;
SAVE7F	DW	7F7FH
	DB	'SAVE'
	DB	5
	CALL	OSTR
	DB	'Name :',0
	CALL	INLIN
	RET	C	;BRK
	LD	HL,6
	ADD	HL,DE	;Beginn Name
	;
SAVE	LD	DE,CASS		;**36**
	LD	BC,11	;Name nach
	LDIR		;Puffer
	XOR	A
	LD	(DE),A
	LD	HL,ARGN	;alle Arg.
	LD	E,10H	;mit ARGN
	LD	C,15H
	LDIR		;nach Puffer
	CALL	KPUFF	;Standard setzen
	CALL	ISRO
SAV1	LD	HL,(ARG1);Ladeadr.
	LD	DE,CASS
	LD	BC,80H
	LD	A,(0B801H)
	AND	1
	JR	NZ,SAV3	;IRM ein
	LD	B,C
SAV2	CALL	SCROFF
	LD	A,M	;aus RAM
	CALL	SCRON
	LD	(DE),A	;in Puffer
	INC	HL
	INC	DE
	DJNZ	SAV2
	JR	SAV4
SAV3	LDIR		;kopieren
SAV4	LD	(ARG1),HL
	CALL	OSTR
	DB	2,0	;CLLN
	CALL	BNROST	;Blocknummer
	NOP
	LD	BC,0A0H
	CALL	BRKT
	JP	C,CSROI	;BRK
	LD	DE,(ARG2)
	SBC	HL,DE	;Ende erreicht?
	JP	NC,CSRO
	CALL	MBO	;Block ausgeben
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
	LD	A,(IX+3)
	DEC	A	;Block 01 erw.?
	JR	Z,LUP1
	CALL	KBD	;nein
	CP	3	;BRK?
	SCF
	RET	Z
	CP	LF	;CUD?
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
	LD	HL,CASS
	LD	B,11	;sondern Name
LUP7	LD	A,M	;ausgeben
	INC	HL
	CALL	CRT
	DJNZ	LUP7
	JP	SPACE
 
NOMC	CALL	OSTR
	DB	'keine MC-Datei!',0
CLJP	JP	CSROI
;
VERI7F	DW	7F7FH
	DB	'VERIFY'
	DB	1
VERIF	LD	(IX+7),0	;**11**
	JR	LOAD1
;
LOAD7F	DW	7F7FH
	DB	'LOAD'
	DB	1
LOAD	LD	(IX+7),1;LOAD	;**10**
	LD	A,(ARGN)
	CP	2
	JR	C,LOAD1	;Autostart?
	SET	1,(IX+7);unterdr}cken
LOAD1	CALL	ISRI1
	CALL	KPUFF	;Standard setzen
	LD	(IX+3),0
	CALL	LUP
LOAD2	JR	C,CLJP	;BRK
	BIT	0,(IX+7)
	JR	Z,LOAD5	;Verify
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
	LD	BC,(CASS+15H)
	PUSH	BC
	LD	DE,(CASS+13H)
	LD	HL,(CASS+11H)
	LD	A,(ARGN)
	AND	A	;Ladeoffset?
	JR	Z,LOAD3	;nein
	LD	BC,(ARG1)
	ADD	HL,BC	;AAdr,Eadr
	EX	DE,HL	;umrechnen
	ADD	HL,BC
	EX	DE,HL
	BIT	4,(IX+7);4..7 Args?
	JR	NZ,LOAD3
	EX	(SP),HL
	ADD	HL,BC	;SAdr umrechnen
	EX	(SP),HL
LOAD3	CALL	HLDE	;AAdr,EAdr ausg.
	LD	A,(CASS+10H)
	CP	3
	JR	C,LOAD4	;keine SAdr
	EX	(SP),HL
	CALL	HLHX	;SAdr ausg.
	EX	(SP),HL
LOAD4	POP	BC	;SAdr merken
	LD	(ARG3),BC
LOAD5	CALL	CRLF
	EX	DE,HL	;DE=Ladeadr.
	;Load-Hauptschleife
LOAD6	CALL	LUP
	JR	C,LOAD2	;BRK
	BIT	0,(IX+7)
	JR	Z,LOAD11;Verify
	PUSH	HL
	SBC	HL,DE
	LD	BC,80H
	SBC	HL,BC
	ADD	HL,BC
	JR	NC,LOAD7
	LD	C,L	;restl. Anzahl
LOAD7	LD	HL,CASS
	LD	A,(0B801H)
	AND	1
	JR	NZ,LOAD9;IRM ein
	LD	B,C
LOAD8	LD	A,M	;aus Puffer
	CALL	SCROFF
	LD	(DE),A	;in RAM
	INC	HL
	INC	DE
	CALL	SCRON
	DJNZ	LOAD8
	JR	LOAD10
LOAD9	LDIR		;kopieren
LOAD10	POP	HL
LOAD11	INC	(IX+2)	;Blocknr. FF?
	JR	NZ,LOAD6
	;
CSRI	CALL	CSROI		;**0B**
	LD	A,(IX+7)
	AND	3
	DEC	A
	RET	NZ	;kein Start
	LD	A,(IX+7)
	AND	1CH
	CP	0CH
	RET	C	;keine Sadr
	LD	HL,(ARG3);Sadr
	JP	(HL)	;Ansprung
;
COL7F	DW	7F7FH
	DB	'COLOR'
	DB	1
COLR	CALL	CON		;**0F**
	CALL	COLRC
	JR	JCOFF
 
CSTBT	PUSH	HL		;**42**
	LD	HL,STBT
	SET	3,M
	CALL	OCHR	;Steuerzeichen
	RES	3,M	;darstellen
	POP	HL
	RET
;
DISP7F	DW	7F7FH
	DB	'DISPLAY'
	DB	3
DISP	CALL	CON		;**3B**
	CALL	DISPC
JCOFF	JP	COFF
 
LDAME	;f}r DISPC, MODIC, FSAVE, view
	CALL	COFF
	LD	A,(0B801H)
	AND	1	;IRM off?
	CALL	Z,SCROFF
	LD	A,M	;Byte holen
JCON	CALL	SCRON
	JP	CON
;
MODI7F	DW	7F7FH
	DB	'MODIFY'
	DB	3
MODI	CALL	CON		;**2E**
	CALL	MODIC
	JR	JCOFF
 
LDMAE	;f}r MODIC, FLOAD
	CALL	COFF
	PUSH	AF
	LD	A,(0B801H)
	AND	1	;IRM off?
	CALL	Z,SCROFF
	POP	AF
	LD	M,A	;Byte eintragen
	JR	JCON
;
WIND7F	DW	7F7FH
	DB	'WINDOW'
	DB	1
	CALL	CON
	CALL	WINDC
	JR	KCOFF
 
WININ	CALL	CON		;**3C**
	CALL	WININC
	JR	KCOFF
 
WINAK	CALL	CON		;**3D**
	CALL	WINAKC
	JR	KCOFF
;
KEY7F	DW	7F7FH
	DB	'KEY'
	DB	1
	CP	1
	JR	NC,KEYL
KEYLI	CALL	CON		;**3A**
	CALL	KEYLIC	;Keylist
	JR	KCOFF
 
KEYL	LD	A,L	;Nummer
KEY	CALL	CON		;**39**
	CALL	KEYC
KCOFF	JP	COFF
 
CUCP	;Cursor compeln
	PUSH	DE		;**25**
	PUSH	AF
	PUSH	HL
	CALL	PADR0
	JR	C,CUCP3	;au~erhalb
	PUSH	HL
	CALL	DABR
	LD	A,M
	POP	HL
	AND	A	;Dummy?
	JR	NZ,CUCP1
	SET	1,L	;vorletzte
	SET	2,L	;Cursorzeile
	LD	A,M
	XOR	7FH	;Strichcursor
	LD	M,A
	JR	CUCP3
CUCP1	PUSH	BC
	LD	DE,(CUMUST);Muster
	LD	B,8	;Zeichenh|he
CUCP2	LD	A,(DE)
	XOR	M	;verkn}pfen
	LD	M,A
	INC	DE
	INC	L
	DJNZ	CUCP2	;8*
	POP	BC
CUCP3	POP	HL
	POP	AF
	POP	DE
	RET
 
PUSE	CALL	CON		;**30**
	CALL	PUSEC	;Punkt setzen
	JR	COF
 
PUDE	CALL	CON		;**2F**
	CALL	PUDEC	;Punkt l|schen
	JR	COF
 
CIRCL	CALL	CON		;**3F**
	CALL	CIRCLC	;Kreis
	JR	COF
 
LINE	CALL	CON		;**3E**
	CALL	LINEC	;Linie
	JR	COF
 
MBOUT	CALL	CON		;**38**
	CALL	MBOC	;Byte ausgeben
	JR	COF
 
MBIN	CALL	CON		;**37**
	CALL	MBIC	;Byte einlesen
	JR	COF
 
JIOERR	CALL	COFF	;Sprung zu
	LD	HL,(IOERR);?IO ERROR
	CALL	IRMOF
	JP	(HL)
;
SYS7F	DW	7F7FH
	DB	'SYSTEM'
	DB	11H
ESC6	CALL	CON
	CALL	SYSTC
COF	JP	COFF
 
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
	LD	A,D	;Parameter
	LD	D,E	;Steuerbyte
	LD	E,M	;UP-Nummer
	POP	HL
	CALL	PV3	;Aufruf
	POP	DE
	LD	D,A	;Ausg.-Param.
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
	DB	03H	;UOT2
 
BSA2	CALL	KBDS
	POP	DE
	LD	D,A
	JR	NC,BSA1
	RES	7,E
	JR	BSA1
 
BASBYE	LD	A,(35EH);gesch}tzt?
	AND	A
	JR	Z,BASB1
	CALL	IRMOF
	CALL	0C641H	;NEW
	CALL	IRMON
BASB1	LD	D,0
	LD	A,2	;BASIC-ROM
	LD	L,A	;abschalten
	CALL	MODU
	LD	HL,(SUTAB)
	LD	DE,SUBAS
	AND	A
	SBC	HL,DE	;BASEX aktiv?
	JR	NZ,BASB2
	LD	HL,(SUSP)
	LD	(SUTAB),HL
BASB2	JP	LOOP
 
ZKOUT	LD	A,M		;**45**
	INC	HL
	AND	A
	RET	Z
	CALL	OCHR
	JR	ZKOUT
 
TON	CALL	LARG		;**35**
TON1	BIT	1,(IX+8);noch alter Ton
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
	RES	7,C	;nein, Blinkaus
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
	JR	Z,TON5
	OR	20H	;ja
TON5	OUT	(C),A
TON6	OUT	(C),L
	RET
 
;ESC-Funktionen:
ESC1	LD	BC,0B200H
	JR	ESCVR
 
ESC2	LD	BC,0AD05H
	JR	ESCVR
 
ESC3	LD	BC,0AD04H
	JR	ESCVR
 
ESC4	LD	BC,0B201H
ESCVR	LD	H,B
	LD	L,0
	LD	(VRAM),HL
	LD	B,0FAH
AXO84	;AND/XOR/OUT84
	LD	A,B
	 DI
	 AND	(IX+1)
	 XOR	C
OUT84	 LD	(IX+1),A
	 OUT	(84H),A
	EI
	RET
 
ESC7	LD	C,4	;Invers-Bit
XORSTB	LD	HL,STBT
	LD	A,M
	XOR	C
	LD	M,A
	RET
 
ESC9	LD	A,(IX+1)
	XOR	2
	DI
	JR	OUT84
 
ESCA	LD	HL,STBT
	RES	6,M	;HRG aus
ESCAB	LD	BC,0FF08H
	CALL	AXO84
	LD	BC,7F00H;Blinken aus
	BIT	3,A
	JR	Z,AXO89
	LD	C,80H	;ein
AXO89	IN	A,89H
	AND	B
	XOR	C
	OUT	89H
	RET
 
ESCB	LD	HL,STBT
	SET	6,M	;HRG ein
	JR	ESCAB
 
ESCC	LD	C,20H	;IBM <-> CAOS
	JR	XORSTB
;
V24O7F	DW	7F7FH
	DB	'V24OUT'
	DB	1
V24OUT	;Druckerroutine		 **47**
	CALL	CON
	CALL	V24OC
V24CRLF	LD	A,CR
	CALL	PRINT
	LD	A,LF	;CR+LF drucken
	CALL	PRINT
COF1	JP	COFF
 
HCPGM	;Reaktion auf ShCLR
	PUSH	AF
	LD	A,(IX+4);CAOSC merken
	PUSH	AF
	SET	7,A	;CAOSC on
	LD	(IX+4),A
	OUT	86H
	CALL	HCPGMC
	POP	AF
	LD	(IX+4),A;wie vorher
	OUT	86H
	POP	AF
	RET
 
;
V24D7F	DW	7F7FH
	DB	'V24DUP'
	DB	1
V24DUP	;Duplexroutine		 **48**
	CALL	CON
	CALL	V24DC
	JR	COF1
 
INIDUP	CALL	CON
	CALL	UMINIT
	JR	COF1
 
;Centronics initialisieren
CEN7F	DW	7F7FH
	DB	'CEN'
	DB	1
	CALL	CON
	CALL	CENC
	JR	V24CRLF
 
ECHO	;Druckroutine mit Protokoll
	CALL	CRT
PRINT	BIT	7,(IX+4)
	JR	NZ,PR1	;CAOSC on!
	PUSH	HL
	LD	HL,COFF	;nach PRINTC
	EX	(SP),HL	;wieder aus
	CALL	CON
PR1	CALL	PRINTC	;drucken
	RET
 
M001PR	;Druckroutine f}r M001
	PUSH	AF
BUSY1	IN	A,(5)
	BIT	2,A	;BUSY?
	JR	NZ,BUSY1
	POP	AF
	PUSH	AF
	OUT	(4),A	;Daten
	XOR	A
	OUT	(5),A
	INC	A	;Strobe-Imp.
	OUT	(5),A
	POP	AF
	RET
 
M021PR	;Druckroutine f}r M021
	PUSH	AF
BUSY2	IN	A,(90H)
	BIT	6,A	;BUSY?
	JR	NZ,BUSY2
	POP	AF
	PUSH	AF
	OUT	(91H),A	;Daten
	XOR	A
	OUT	(90H),A
	LD	A,80H	;Strobe-Imp.
	OUT	(90H),A
	POP	AF
	RET
 
V24PR	;Druckroutine f}r V24
	PUSH	AF
V24PR1	PUSH	BC
	CALL	LCSO
	LD	A,0AH
	ADD	C
	LD	C,A
	IN	A,(C)
	BIT	2,A	;bereit?
	JR	NZ,V24PR2
	LD	A,1	;nein-warten
	CALL	WAIT
	POP	BC
	JR	V24PR1
V24PR2	POP	BC
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
 
V24OT	;Ausgabe V.24-duplex
	PUSH	AF
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
 
V24IN	;Eingabe V.24-duplex
	PUSH	BC
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
	LD	A,0EAH	;DTR on, 8bit,
	OUT	(C),A	;Senderfreigabe
V24IN1	IN	A,(C)
	BIT	0,A	;jetzt bereit?
	JR	NZ,V24IN2
	CALL	BRKT
	JR	C,V24IN4
	JR	V24IN1
	;
V24IN2	LD	A,5	;WR5
	OUT	(C),A
	LD	A,6AH	;DTR off usw.
	OUT	(C),A
	POP	BC
	LD	A,8
	ADD	C
	LD	C,A
	IN	A,(C)	;Einlesen
V24IN3	POP	DE
	POP	HL
	POP	BC
	RET
	;
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
	;Fremdtastatur
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	A,(IX+4);merken f}r
	PUSH	AF	;CAOS-ROM-C
	IN	A,9	;1.Zeichen
	LD	HL,IOTAB3
	CALL	INIEA
	CALL	IRET
	CP	CR	;Fremdtastatur?
	JR	Z,ISB2
	CP	ESC	;MC-Programm?
	JR	NZ,ISB1
	LD	A,6
	LD	(HCPZ2),A
	CALL	V24DUP
	CALL	V24IN
	SUB	'T'
	JR	Z,ISB4	;MC laden
	DEC	A
	JR	Z,ISB6	;MC starten
ISB1	CALL	INIDUP	;nichts
	JR	ISB3
	;
ISB2	CALL	INIDUP
	LD	HL,ISRT	;Neue ISR
	DI
	LD	(1E2H),HL
	EI
ISB3	POP	AF
	OUT	86H	;wie vor INT!
	POP	AF
	POP	BC
	POP	DE
	POP	HL
	RET
	;
ISB4	CALL	V24HL	;Adresse
	EX	DE,HL
	CALL	V24HL	;Anzahl
	EX	DE,HL
ISB5	CALL	V24IN
	LD	M,A	;Daten
	INC	HL
	DEC	DE
	LD	A,E
	OR	D
	JR	NZ,ISB5
	JR	ISB1
	;
ISB6	CALL	V24HL
	CALL	JPHL	;Prog. starten
	JR	ISB1
 
ISRT	;ISR f}r Fremdtastatur
	PUSH	AF
	IN	A,9
	AND	A
	JR	Z,IST2	  ;Code 0
	CP	(IX+13)
	JR	Z,IST1	  ;Repeat!
	LD	(IX+10),0
	JR	IST2
IST1	PUSH	DE
	LD	D,A	  ;Code merken
	IN	A,88H
	LD	E,A
	SET	2,A	  ;IRM on!
	OUT	88H
	LD	A,(COUNT)
	INC	A
	LD	(IX+10),A ;repeat jetzt
	LD	A,E
	OUT	88H
	LD	A,D
	POP	DE
IST2	LD	(IX+13),A ;Code
	AND	A
	JR	Z,IST3
	SET	0,(IX+8)  ;aktivieren
IST3	POP	AF
IRET	EI
	RETI
 
V24HL	CALL	V24IN
	LD	L,A
	CALL	V24IN
	LD	H,A
	RET
 
IOTAB3	DB	0BH	;SIO-B
	 DB	3	;3 Byte
	  DB	18H	;r}cksetzen
	  DB	5	;WR5
	  DB	6AH	;DTR off
 
CON	;CAOS-ROM C on
	PUSH	AF
	SET	7,(IX+4),A
	JR	CSW
 
COFF	;CAOS-ROM C off
	PUSH	AF
	LD	A,(0B805H)
	AND	1
	JR	NZ,CNSW	;bleibt on!
	RES	7,(IX+4),A
CSW	OUT	86H
CNSW	POP	AF
	RET
 
MULT	;BA=D*C			 **41**
	CALL	CON
	CALL	MULC
	JR	COFF
 
SQR	CALL	CON		;**40**
	CALL	SQRC
	JR	COFF
 
ISRC2	;CTC2: Tonlg
	CALL	INTEND
	PUSH	AF
	PUSH	DE
	IN	A,88H
	LD	E,A
	SET	2,A	;IRM on
	OUT	88H
	CALL	TOFF
	LD	A,E
	OUT	88H
	POP	DE
	POP	AF
	RET
 
ZEI0	DB	0	;CRT
	DB	38H	;MBOUT
ZEI2	DB	2	;UOUT1
ZEI3	DB	3	;UOUT2
ZEI4	DB	4	;KBD
	DB	37H	;MBIN
	DB	6	;UIN1
	DB	7	;UIN2
 
SUTB	DW	CRT	;00
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
	DW	V24OUT	;47
	DW	V24DUP	;48
 
 
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
	DB	3,4		;BRK
	DB	'Nn'
	DB	'Ii'
	DB	'Jj'
	DB	'8('
	DB	' ['		;SPC
	DB	'Kk'
	DB	',<'
	DB	13H,ESC		;STOP
	DB	'Mm'
	DB	'Zz'
	DB	'Gg'
	DB	'6&'
	DB	1CH,1DH	    ;LIST/RUN
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
	DB	16H,5		;CAPS
	DB	'1!'
	DB	LF,12H		;CUD
	DB	0BH,11H		;CUU
	DB	09H,18H		;CUR
	DB	0F1H,0F7H	;F1
	DB	CR,0EH		;ENTER
 
 
ISRTAB	DW	ISRSB	;SIO B:Empfang
	DW	ISRPA	;PIO A:Kassette
	DW	ISRPB	;PIO B:Tastatur
	DW	INTEND	;CTC 0:frei
	DW	ISRC1	;CTC 1:Kassette
	DW	ISRC2	;CTC 2:Tondauer
	DW	ISRC3	;CTC 3:Tastatur
 
 
CRTTAB	DW	NOT	;00
	DW	CLR	;01
	DW	CLLN	;02
	DW	NOT	;03=BRK
	DW	NOT	;04=Sh-BRK
	DW	ESC0	;05=TAB
	DW	NOT	;06
	DW	BEEP	;07
	DW	CUL	;08
	DW	CUR	;09
	DW	CUD	;0A
	DW	CUU	;0B
	DW	CLS	;0C
	DW	CBL	;0D=Enter
	DW	NOT	;0E=Sh-Enter
	DW	HCOPY	;0F
	DW	HOMEPG	;10
	DW	PAGE	;11
	DW	SCROL	;12
	DW	NOT	;13=STOP
	DW	CLICK	;14
	DW	NOT	;15
	DW	NOT	;16=CAPS
	DW	NOT	;17
	DW	CEL	;18
	DW	CBL	;19
	DW	INS	;1A
	DW	ESCPRG	;1B
	DW	NOT	;1C=LIST
	DW	NOT	;1D=RUN
	DW	NL	;1E=CONT
	DW	DEL	;1F
 
 
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
 
FREIF	EQU	$
	DS	0FDF8H-$,0FFH	;frei
 
BITTAB	DB	80H,40H,20H,10H
	DB	8,4,2,1
 
	END
��
PRSTR:��
PRK:��
PRINC:}�
PRIC:e�
PRI:]�
PRI7F:S�
VIEW5:Q�
VIEW4:D�
VIEW3:9�
VIEW2:3�
VIEW1:�
VIEW7F:�
JBRKT:�
MBLN
	END
��
PRSTR:��
PRK:��
PRINC:}�
PRIC:e�
PRI:]�
PRI7F:S�
VIEW5:Q�
VIEW4:D�
VIEW3:9�
VIEW2:3�
VIEW1:�
VIEW7F:�
JBRKT:�
MBLN