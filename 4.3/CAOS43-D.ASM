CD43    ASM                                                                                                                      
;**	CAOS 4.3 ROM D		**
;	(17.02.95)
 
	ORG	0D000H
 
JDISC	;MC-Sprungverteiler
	;(f}r die 9 Grundfunktionen)
	;
	CP	9	;Funktion 0-8
	RET	NC	;zugelassen
	PUSH	AF
	LD	HL,0
	CALL	NAM2	;Name ausgeben
	LD	HL,DTAB
	POP	AF
	ADD	A
	LD	D,0
	LD	E,A
	ADD	HL,DE
	LD	E,M	;Tab. lesen
	INC	HL
	LD	D,M
	PUSH	DE	;Sprungadresse
 
DISK	;testet ob Floppy arbeitet
	LD	BC,0FC80H
	IN	A,(C)
	CP	0A7H	;Floppy?
	JR	Z,DISK1
	CALL	OSTR
	DB	'kein D004!'
	DB	7,CR,LF,0
	POP	HL
	RET
DISK1	LD	BC,0B3F3H
	IN	A,(C)
	CP	5	;KC-BA?
	RET	Z
	CALL	OSTR
	DB	'CAOS-Disk starten!'
	DB	7,CR,LF,0
	POP	HL
	RET
;
DTAB	;Sprungtabelle Diskroutinen
	DW	FLOAD
	DW	FSAVE
	DW	DIR
	DW	STAT
	DW	REN
	DW	SETRO
	DW	SETWR
	DW	ERA
	DW	DRIVE
;
NAME	;Name ein-, ausgeben
	EX	DE,HL
	LD	A,M
	AND	0DFH
	JR	NZ,NAM2	;mit arg!
NAM0	LD	HL,6
NAM1	CALL	OSTR
	DB	'Name :',0
	CALL	INLIN
	RET	C	;BRK
	ADD	HL,DE
NAM2	LD	E,12
	LD	BC,82F3H
NAM3	LD	A,M
	OUT	(C),A
	INC	HL
	INC	B
	DEC	E
	JR	NZ,NAM3
	LD	B,80H
	RET
;
STEUER2	;Steuerbyte 2 ausgeben
	LD	BC,80F1H
	JR	STEU0
	;
STEUER	;Steuerbyte 1 ausgeben
	LD	BC,80F3H
STEU0	OUT	(C),A
STEU1	PUSH	BC
	LD	A,1
	CALL	WAIT
	POP	BC
	IN	A,(C)
	BIT	0,A
	JR	NZ,STEU1;fertig?
	BIT	7,A
	RET	Z	;kein Fehler
	POP	BC	;RET-Adresse!
FLERR	LD	BC,83F1H
	IN	A,(C)
	CP	20H	;DEP ab 2.0?
	JR	NC,FLER1;ja
	LD	BC,81F3H
	IN	A,(C)	;Fehlernummer
	CALL	AHEX
	JP	ERRM
	;
FLER1	LD	BC,0F1H	;Fehlertext
FLER2	IN	A,(C)
	INC	B
	AND	A
	RET	Z	;00=Ende
	CALL	OCHR
	JR	FLER2
;
;BASEX-Programmteil mit Fehlertexten
;
BASEX	LD	HL,BAS
	LD	DE,0D8H
	LD	BC,BASE-BAS
	LDIR		;Verteiler
	LD	HL,UMS
	LD	DE,150H
	LD	BC,UMSE-UMS
	LDIR		;Umschaltung
	LD	HL,BASEC
	LD	DE,200H
	LD	BC,UMS-BASEC
	LDIR		;BASEX selbst
	LD	HL,(SUTAB)
	LD	(SUSP),HL
	LD	DE,SUBAS
	LD	(SUTAB),DE
	LD	BC,92H
	LDIR		;SUTAB
	LD	HL,MBIND
	LD	(SUBAS+6EH),HL
	LD	HL,MBOUTD
	LD	(SUBAS+70H),HL
	RET
 
BASEC	EQU	$	;Adresse im ROMC
RUN	EQU	$-BASEC+200H
	DB	0	;1 ->RUN-Switch
SUSP	EQU	$-BASEC+200H
	DW	0	;Speicher SUTAB
 
MBOUTD	EQU	$-BASEC+200H
	LD	BC,8EF3H
	OUT	(C),A	;Datenbyte
	LD	A,D
	AND	48H
	OR	37H
	CALL	NAMBAS
	BIT	6,A	;close?
	JR	Z,O1
	LD	D,A
	LD	A,(RUN)
	AND	A
	LD	A,D
	JR	Z,O1
	SET	7,A	;run-switch
O1	OUT	(C),A	;Steuerbyte
O2	IN	A,(C)
	BIT	0,A	;fertig?
	JR	NZ,O2
	RLCA
	SET	0,(IX+8);Taste gedr}ckt
	RET	NC
BASERR	LD	BC,83F1H
	IN	A,(C)
	CP	20H	;DEP ab 2.0?
	JR	NC,BERR2;ja
	LD	BC,81F3H
	IN	A,(C)
	CALL	0F003H
	DB	1CH	;Fehler-Nr.
BERR1	LD	HL,(IOERR)
	PUSH	HL
	JP	0F01BH
BERR2	LD	B,0	;Fehlertext
BERR3	IN	A,(C)
	JR	Z,BERR1	;00-Ende
	INC	B
	CALL	0F003H
	DB	24H	;OCHR
	JR	BERR3
 
MBIND	EQU	$-BASEC+200H
	LD	A,D
	AND	48H
	OR	35H
	CALL	NAMBAS
	OUT	(C),A	;Steuerbyte
I1	IN	A,(C)
	BIT	0,A	;fertig?
	JR	NZ,I1
	RLCA
	JR	C,BASERR;Fehler
	LD	B,8EH
	IN	A,(C)	;Datenbyte
	RET
 
SUBAS	EQU	$-BASEC+200H
	;hier wird die SUTAB hin kopiert
 
UMS	;Umschaltung Tape/Disk
	CALL	0F018H
	LD	HL,(SUSP)
	LD	DE,(SUTAB)
	LD	(SUTAB),HL
	LD	(SUSP),DE
	CALL	0F01BH
	RET
 
;Dateiname (in BASEX) ausgeben:
 
NAMBAS	EQU	$-UMS+150H
	LD	BC,83F3H
	BIT	3,A	;open?
	JR	Z,NAMB2
	LD	E,11	;Anzahl
NAMB1	OUTI
	INC	B
	INC	B
	DEC	E
	JR	NZ,NAMB1
NAMB2	LD	B,80H	;BC f}r Steuer-
	RET		;byte
UMSE	;Ende der Umschaltroutine
 
BAS	;BASIC-Schnittstelle
	;(vorbereitet f}r 12 Funktionen)
	LD	A,0	;CALL*D8
	DB	1
	LD	A,1	;CALL*DB
	DB	1
	LD	A,2	;CALL*DE
	DB	1
	LD	A,3	;CALL*E1
	DB	1
	LD	A,4	;CALL*E4
	DB	1
	LD	A,5	;CALL*E7
	DB	1
	LD	A,6	;CALL*EA
	DB	1
	LD	A,7	;CALL*ED
	DB	1
	LD	A,8	;CALL*F0
	DB	1
	LD	A,9	;CALL*F3
	DB	1
	LD	A,10	;CALL*F6
	DB	1
	LD	A,11	;CALL*F9
	DB	1
	LD	A,12	;CALL*FC
	CALL	0F018H
	CALL	JDISK	;Disk-Verteiler
	CALL	0F01BH
	RET
BASE	;Ende BASIC-Verteiler
 
FLOA7F	DW	7F7FH
	DB	'FLOAD'
	DB	1
	CALL	DISK
	CALL	NAM0
	RET	C	;BRK
FLOAD	LD	A,(ARGN)
	CP	2
	LD	A,0
	JR	C,FLO0
	SET	1,A	;kein Autostart
FLO0	LD	(IX+7),A
	LD	A,9	;Lesen open
	CALL	STEUER
	LD	BC,90F2H
	IN	A,(C)	;Anzahl
	SLA	A
	SLA	A
	AND	1CH
	OR	(IX+7)
	LD	(IX+7),A
	IN	A,(C)	;Anzahl
	CP	2
	JR	C,FLO1	;wenn<2
	CP	10
	JR	C,FLO2	;wenn>10
FLO1	CALL	OSTR
	DB	'keine MC-Datei!'
	DB	7,CR,LF,0
	RET
FLO2	INC	B
	IN	L,(C)
	INC	B
	IN	H,(C)	;HL=Ladeadr
	INC	B
	IN	E,(C)
	INC	B
	IN	D,(C)	;DE=Endadr
	CP	3
	JR	C,FLO3	;keine Startadr
	PUSH	HL
	INC	B
	IN	L,(C)
	INC	B
	IN	H,(C)	;Startadr
	EX	(SP),HL
FLO3	EX	AF,AF'
	LD	A,(ARGN)
	AND	A
	JR	Z,FLO5	;ohne Offset
	LD	BC,(ARG1);Offset
	ADD	HL,BC
	EX	DE,HL
	ADD	HL,BC
	EX	DE,HL
	EX	AF,AF'
	JR	C,FLO4	;keine Startadr
	EX	(SP),HL
	ADD	HL,BC
	EX	(SP),HL
FLO4	EX	AF,AF'
FLO5	CALL	HLDE	;Aadr Eadr
	EX	AF,AF'
	JR	C,FLO6	;keine Startadr
	EX	(SP),HL
	LD	(ARG3),HL
	CALL	HLHX	;Sadr
	POP	HL
FLO6	CALL	CRLF
FLO7	LD	A,1	;Lesen normal
	CALL	STEUER
	DEC	C
FLO8	IN	A,(C)
	CALL	LDMAE	;in Speicher
	INC	HL
	INC	B
	PUSH	HL
	SBC	HL,DE	;Ende?
	POP	HL
	JR	NC,FLO9	;ja
	LD	A,B
	AND	A
	JR	NZ,FLO8
	JR	FLO7
FLO9	LD	A,(IX+7)
	BIT	1,A
	RET	NZ	;kein Start
	AND	1CH
	CP	0CH
	RET	C	;keine Sadr
	LD	HL,(ARG3);Sadr
	PUSH	HL
	JP	COFF	;erst CAOSC off!
 
FSAV7F	DW	7F7FH
	DB	'FSAVE'
	DB	5
	CALL	DISK
	CALL	NAM0
	RET	C	;BRK
FSAVE	CALL	CLC	;Puffer l|schen
	LD	L,ARGN
	LD	DE,CASS+10H
	LD	BC,15H
	LDIR		;Arg's
	LD	L,0
	LD	BC,81F2H
	LD	E,80H
FSA2	OUTI		;Vorblock
	INC	B
	INC	B
	DEC	E
	JR	NZ,FSA2
	LD	A,0BH	;Schreib open
	CALL	STEUER
	LD	HL,(ARG1);Aadr
FSA3	LD	BC,80F2H
	LD	E,80H
FSA4	CALL	LDAME	;Daten
	INC	HL
	OUT	(C),A
	INC	B
	DEC	E
	JR	NZ,FSA4
	LD	A,3	;Schreib normal
	CALL	STEUER
	PUSH	HL
	LD	DE,(ARG2);Eadr
	AND	A
	SBC	HL,DE	;fertig?
	POP	HL
	JR	C,FSA3	;nein
	LD	A,43H	;Schreib close
	CALL	STEUER
	CALL	CLC	;Puffer l|schen
	RET
 
;Directory mit Maske
;
DIR7F	DW	7F7FH
	DB	'DIR'
	DB	1FH
	CALL	DISK
	EX	DE,HL	;Maske
	LD	DE,CASS
	LD	BC,11	;L{nge
	LDIR
	JR	DIR0
DIR	LD	DE,CASS
DIR0	XOR	A	;Abschlu~ Maske
	LD	(DE),A
	LD	A,0DH	;Directory open
DIR1	CALL	STEUER
	BIT	2,A
	JR	NZ,DIR6	;Ende
	LD	B,82H
	LD	HL,CASS+0CH
	AND	A	;CY=0
	LD	D,3	;3 Eintr{ge
DIR2	LD	E,11	;11 Zeichen lang
DIR3	IN	A,(C)	;einlesen
	PUSH	AF
	AND	7FH
	INC	B
	LD	M,A
	INC	HL
	LD	A,E
	CP	3	;RO-Byte?
	JR	Z,DIR4
	POP	AF
DIR4	DEC	E
	JR	NZ,DIR3
	LD	M,0	;Ende-Kennung
	INC	HL
	INC	B
	POP	AF
	LD	M,A	;RO-Byte
	INC	HL
	DEC	D
	JR	NZ,DIR2
	LD	HL,CASS+0CH
	LD	B,3
DIR5	PUSH	HL
	LD	DE,CASS
	LD	A,(DE)
	AND	A
	CALL	NZ,GLOB	;Vergleich
	POP	HL
	CALL	NC,DIR7	;Anzeige
	LD	DE,13
	ADD	HL,DE
	DJNZ	DIR5	;3 Eintr{ge
	LD	A,5	;DIR weiter
	JR	DIR1
	;
DIR6	LD	A,(CURSO)
	AND	A
	CALL	NZ,CRLF
	JR	STAT
	;
DIR7	PUSH	HL
	LD	A,(CURSO)
	CP	3*13
	CALL	Z,CRLF
	CALL	ZKOUT
	LD	A,' '	;WR
	BIT	7,M
	JR	Z,DIR8
	LD	A,'*'	;RO
DIR8	CALL	OCHR
	CALL	SPACE
	POP	HL
	RET
;
STAT	LD	A,15H	;Status
	CALL	STEUER
	LD	E,20
	LD	B,82H
STAT1	IN	A,(C)
	AND	7FH
	INC	B
	PUSH	AF
	CALL	OCHR
	POP	AF
	DEC	E
	JR	NZ,STAT1
	RET
 
;Vergleich mit Maske
;
MAT0	INC	HL
MAT0E	INC	DE
GLOB	;Eintritt, HL=Name, DE=Maske
	;PA: CY=1: nicht gefunden
	LD	A,(DE)
	OR	M	;Beide Ende?
	RET	Z	;->OK
	LD	A,(DE)
	CP	'*'
	JR	Z,MAT0S
	CP	1
	RET	C	;M Ende
	LD	A,M
	CP	1
	RET	C	;N Ende
	LD	A,(DE)
	CP	'?'
	JR	Z,MAT0	;Joker
	BIT	5,M
	CALL	Z,UPCASE
	CP	M
	JR	Z,MAT0
	SCF
	RET
	;
MAT0S	PUSH	HL
	PUSH	DE
	CALL	MAT0E	;Rekursion!
	POP	DE
	POP	HL
	RET	NC	;-> OK
	LD	A,M
	INC	HL
	OR	A
	JR	NZ,MAT0S
	SCF
	RET
;
REN7F	DW	7F7FH
	DB	'REN'
	DB	1FH
	CALL	DISK
REN	CALL	OSTR
	DB	'Alter ',0
	LD	HL,12
	CALL	NAM1
	RET	C	;BRK
	LD	A,29H	;Ren old
	CALL	STEUER
	CALL	OSTR
	DB	'Neuer ',0
	LD	HL,12
	CALL	NAM1
	LD	A,21H	;Ren new
OUT	RET	C	;BRK
	CALL	STEUER
	RET
;
ERA7F	DW	7F7FH
	DB	'ERA'
	DB	1FH
	CALL	DISK
	CALL	NAME
	JR	ERA1
ERA	CALL	NAM0
ERA1	LD	A,11H	;Era
	JR	OUT
;
SRO7F	DW	7F7FH
	DB	'SETRO'
	DB	1FH
	CALL	DISK
	CALL	NAME
	JR	SETR1
SETRO	CALL	NAM0
SETR1	LD	A,25H	;Setro
	JR	OUT
;
SWR7F	DW	7F7FH
	DB	'SETWR'
	DB	1FH
	CALL	DISK
	CALL	NAME
	JR	SETW1
SETWR	CALL	NAM0
SETW1	LD	A,31H	;Setwr
	JR	OUT
;
DRIV7F	DW	7F7FH
	DB	'DRIVE'
	DB	1FH
	CALL	DISK
	JR	DRIV0
DRIVE	LD	DE,DRIV1
DRIV0	LD	BC,83F1H
	IN	A,(C)
	CP	20H	;DEP ab 2.0?
	JR	NC,DRIV2;nein
	CALL	OSTR
	DB	'kein DEP2!'
	DB	7,CR,LF
DRIV1	NOP
	RET
DRIV2	EX	DE,HL
	LD	A,M
	AND	0DFH
	JR	NZ,DRIV3;mit arg
	CALL	OSTR
	DB	'Laufwerk:',0
	LD	A,1	;akt. lesen
	CALL	STEUER2
	LD	BC,81F1H
	IN	A,(C)
	CALL	OCHR	;Laufwerk
	INC	B
	IN	A,(C)	;akt. User
	AND	0FH
	CALL	NZ,AHEX0;Userbereich
	LD	A,9
	LD	(CURSO),A
	CALL	INLIN	;Eingabe
	RET	C	;BRK
	LD	HL,9
	ADD	HL,DE
DRIV3	LD	BC,81F1H
	LD	A,M
	AND	A	;leer?
	RET	Z
	CALL	UPCASE
	OUT	(C),A	;LW-Code
	INC	HL
	LD	A,M
	AND	A	;USER angeg.?
	LD	A,3	;nur LW anf.
	JR	Z,DRIV5
	EX	DE,HL
	CALL	RHEX
	LD	A,0	;USER 0, weil
	JR	C,DRIV4	;keine HEX-Zahl
	LD	A,(NUMVX)
DRIV4	INC	B
	OUT	(C),A	;USER-Nr.
	LD	A,7	;LW+User anf.
DRIV5	CALL	STEUER2
	RET
;
TYPE7F	DW	7F7FH
	DB	'TYPE'
	DB	1FH
	CALL	DISK
	CALL	NAME
	RET	C	;BRK
	LD	A,9	;read-open
	CALL	STEUER
	LD	BC,90F2H
	INF
	LD	B,80H
	JR	NZ,TYPE2;kein Vorblock
TYPE1	LD	A,1	;read
	CALL	STEUER
	DEC	C
TYPE2	IN	A,(C)	;Zeichen
	CP	3
	JP	Z,CRLF	;CAOS-Ende
	CP	1AH
	JP	Z,CRLF	;CP/M-Ende
	CP	20H
	JR	NC,TYPE5
	CP	LF
	JR	Z,TYPE5
	CP	CR
	JR	Z,TYPE5
	CP	9
	JR	NZ,TYPE4
TYPE3	LD	A,' '
	CALL	OCHR
	LD	A,(CURSO)
	AND	7
	JR	NZ,TYPE3
	JR	TYPE6
TYPE4	LD	A,'.'	;statt Steuerz.
TYPE5	CALL	OCHR
TYPE6	LD	A,(CURSO+1)
	INC	A
	LD	HL,WINLG+1
	CP	(HL)	;Seite voll?
	JR	NZ,TYPE9
TYPE7	CALL	INTB
	CP	3
	RET	Z	;BRK
	CP	0FH
	JR	NZ,TYPE8
	CALL	OCHR	;Hardcopy
	JR	TYPE7
TYPE8	LD	A,0CH
	CALL	CRT
	LD	A,LF
	CALL	CRT
TYPE9	INC	B
	JR	NZ,TYPE2;weiter
	JR	TYPE1	;neuen Block
;
DUMP7F	DW	7F7FH
	DB	'DUMP'
	DB	1FH
	CALL	DISK
	CALL	NAME
	RET	C	;BRK
	LD	A,9	;read-open
	CALL	STEUER
	JR	DUMP2
DUMP1	LD	A,1	;read
	CALL	STEUER
DUMP2	DEC	C
DUMP3	PUSH	BC
DUMP4	LD	A,' '
	CALL	OCHR
	IN	A,(C)
	CALL	AHEX
	INC	B
	LD	A,7
	AND	B	;8*Hexwert
	JR	NZ,DUMP4
	CALL	OSTR
	DB	'   ',0
	POP	BC
DUMP5	IN	A,(C)
	CALL	CSTBT
	INC	B
	LD	A,7
	AND	B	;8*Ascii
	JR	NZ,DUMP5
	CALL	CRLF
	LD	A,(CURSO+1)
	INC	A
	LD	HL,WINLG+1
	CP	(HL)	;Seite voll?
	JR	NZ,DUMP8
DUMP6	CALL	INTB
	CP	3	;BRK?
	RET	Z	;ja
	CP	0FH
	JR	NZ,DUMP7
	CALL	OCHR	;Hardcopy
	JR	DUMP6
DUMP7	LD	A,0CH
	CALL	CRT
	LD	A,LF
	CALL	CRT
DUMP8	LD	A,B
	AND	A
	JR	NZ,DUMP3
	JR	DUMP1
;
UUU	DB	'INITIAL.UUU',0
	DW	7F7FH
	DB	'INIT'
	DB	1FH
	CALL	DISK
	EX	DE,HL
	LD	A,M
	AND	0DFH
	JR	Z,INIT3	;Standard
	PUSH	HL
INIT1	LD	A,M
	AND	A
	JR	Z,INIT4	;Ende Dateiname
	INC	HL
	CP	':'	;LW?
	JR	NZ,INIT1
	LD	A,M
	POP	HL
	AND	A
	JR	NZ,INIT4	;LW & Na
	LD	BC,81F1H
	LD	A,M
	OUT	(C),A	;LW-Code
	INC	HL
	LD	A,M
	CP	':'
	LD	A,3	;ohne USER
	JR	Z,INIT2	;nur LW
	EX	DE,HL
	CALL	RHEX
	LD	A,(NUMVX)
	INC	B
	OUT	(C),A	;User
	LD	A,7
INIT2	CALL	STEUER2
INIT3	LD	HL,UUU
	PUSH	HL
INIT4	POP	HL
INIT5	CALL	NAM2
	LD	DE,CASS
	LD	A,9	;read-open
INIT6	CALL	STEUER
	DEC	C
INIT7	IN	A,(C)	;Zeichen
	AND	A
	JR	Z,INIT8	;00 ignorieren
	CP	3
	JR	Z,INIT9	;CAOS-Ende
	CP	0AH
	JR	Z,INIT8	;LF ignorieren
	CP	1AH
	JR	Z,INIT9	;CP/M-Ende
	LD	(DE),A
	INC	E
	LD	A,E
	CP	7FH
	JR	Z,INIT9	;Puffer voll!
INIT8	INC	B
	JR	NZ,INIT7;weiter
	LD	A,1
	JR	INIT6	;neuen Block
INIT9	LD	A,0
	LD	(DE),A	;Ende
	LD	HL,CASS
	LD	(FTAST),HL
	SET	6,(IX+8)
	RET
 
	;Rest frei
 
	END
�@�t(�p6�n Block
INIT9	LD	A,0
	LD	(DE),A	;Ende
	LD	HL,CASS
	LD	(FTAST),HL
	SET	6,(IX+8)
	RET
 
	;Rest frei
 
	END
�@�t(�p6�n