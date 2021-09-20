;
;**	CAOS 4.4 ROM C		**
;	(08.02.2003)
 
;	ORG	0C000H

;
; IRM-Defaulttabelle:
;
KCTAB1:	DW	NOT	;Hardcopy-Adresse
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
	OUT	(88H),A	;BJUMP
	JP	0F012H
	DW	ZEI0	;OUTAB
	DW	ZEI4	;INTAB
	JP	NOOP	;UOUT1
	JP	NOOP	;UIN1
	JP	NOOP	;UOUT2
	JP	NOOP	;UIN2
	DW	0DC82H	;IOERR (BASIC)
	DW	0B200H	;VRAM

KCTAB2:	DB	47H,0CH	;CTC2 (blinken)
	DW	L3TB	;L3TAB
	DB	13	;L3SIZ (0-C)
	DB	5	;COUNT
	DB	0	;HCPZ
	DW	0A801H	;INTV1
	DB	8	;INTV1L
	DW	0A809H	;INTV2
	DB	9	;INTV2L
	DB	6	;HCPZ2
	JP	NOOP	;OFILT
	DB	'%'	;PROMPT
	DB	0FFH	;LINTYP
	DW	0EFD8H	;CUMUST

;Modulschaltzustaende (interne Module):
KCTAB3:	DB	3	;RAM0
	DB	1	;IRM
	DB	0	;USER-ROMC
	DB	3	;RAM8
	DB	3	;RAM4
	DB	0	;CAOSC
KCTAB4:
;
; Power-ON Initialisierung:
;
PWRONC:	DI
	LD	SP,0C000H
	LD	HL,0
	LD	BC,6000H
PWRC1:	PUSH	HL		; schnelles Speicher loeschen
	DEC	C		; im Bereich 0-BFFFH
	JR	NZ,PWRC1
	DJNZ	PWRC1
	LD	SP,01C4H	; System-Stackpointer
	XOR	A
	LD	C,80H		; alle Module
PWRC2:	OUT	(C),A		; abschalten
	DJNZ	PWRC2
	CALL	SYSI		; Systeminitialisierung
	XOR	A		; Fenster Nr. 0
PWRC3:	LD	(WINNR),A	; Nr. aintragen
	CALL	WCOPY		; Fenstervektorspeicher initialisieren
	INC	A
	CP	10		; fuer 10 Fenster (0-9)
	JR	C,PWRC3
	CALL	ESC3		; Anzeige Bild 0, Zugriff Bild 1
	LD	A,0CH
	CALL	CRT		; Bild 1: CLS
	CALL	ESC1		; Anzeige+Zugriff Bild 0
	JP	PWR4		; weiter im ROM-F ...
;
; System-Initialisierung:
;
SYSI:	DI
	IM	2
	LD	HL,IOTAB1	; PIO A,B und 84H/86H
	CALL	INIMEX		; Ports initialisieren
	LD	IX,1F0H
	LD	(IX+1),28H	; OUT84-Merker
	LD	(IX+4),0E3H	; OUT86-Merker
	LD	DE,HCADR
	LD	HL,KCTAB1	; Defaulttabelle
	LD	BC,KCTAB2-KCTAB1; fuer IRM-Arbeitszellen
	LDIR			; 1. Teil in IRM kopieren
	LD	E,LOW(CTCMD)
	LD	C,KCTAB3-KCTAB2
	LDIR			; 2. Teil in IRM kopieren
	LD	DE,MODST	; Modulsteuerwortspeicher
	LD	A,(MODST+4)	; RAM4 aktueller Schaltzustand
	AND	4		; RAM4-Ebene herausloesen
	LD	C,KCTAB4-KCTAB3
	LDIR			; 3. Teil in IRM kopieren
	LD	HL,MODST+4
	OR	(HL)		; bisherige Ebene wieder einbauen
	LD	A,1
SIXC:	LD	HL,ISRTAB
	LD	(MIXIT),A
	LD	E,0E2H
	LD	D,A
	LD	BC,0EH		; Interrupttabelle
	LDIR			; kopieren xxE2h bis xxF0h
	LD	H,(IX+1)	; alte Merker
	LD	L,(IX+4)	; mitnehmen
	PUSH	DE
	POP	IX
	LD	(IX+8),88H
	LD	(IX+1),H	; Merker neu eintragen
	LD	(IX+4),L
	LD	(IX+14),LOW(KTAB)
	LD	(IX+15),HIGH(KTAB)
	LD	(IX+9),7FH	; Standard-Prologbyte
	DI
	IM	2
	LD	A,(MIXIT)
	LD	I,A
	LD	HL,IOTAB2	; PIO A,B und CTC
	CALL	INIMEX		; Ports neu initialisieren
	JP	KPUFF		; Kassettenpuffer definieren
;
; Portinitialisierungstabelle(n):
;
IOTAB1:	DB	7	; 7 Ports fuer Erstinitialisierung

	DB	8AH	;PIO-A-Steuerw.
	 DB	1
	  DB	0E4H	;Int-Vektor

	DB	88H	;PIO-A-Daten
	 DB	2
	  DB	1FH	;D005-Reset
	  DB	0FH	;IRM,RAM0,CAOSE

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

	DB	84H	;Port 84H
	 DB	1
	  DB	28H	;Ausgabebyte

	DB	86H	;Port 86H
	 DB	1
	  DB	0E3H	;Ausgabebyte
 
IOTAB2:	DB	4	; 4 Ports

	DB	8AH	;PIO-A-Steuerw.
	 DB	1
	  DB	0E4H	;Int-Vektor

	DB	8BH	;PIO-B-Steuerw.
	 DB	1
	  DB	0E6H	;Int-Vektor

	DB	8CH	;CTC0
	 DB	1
	  DB	0E8H	;Int-Vektor CTC

	DB	8EH	;CTC2 (blinken)
	 DB	2
	  DB	47H	;DI,Z{hler,RES
	  DB	0CH	;Zeitkonstante
 
SYSTC:	;Systemcheck
	PUSH	DE
	CALL	OSTR
	DB	CR,'CAOSE',0
	IN	A,(88H)
	AND	1
	CALL	WONOF	;Bit 0
	CALL	OSTR
	DB	'CAOSC',0
	LD	A,(MODST+5)
	AND	1
	CALL	WONOF
	CALL	OSTR
	DB	'ROMC ',0
	LD	A,(IX+4)
	SRL	A	;Bits 5,6
	SRL	A
	SRL	A
	SRL	A
	SRL	A	;nach Bits 0,1
	CPL
	AND	'3'
	CALL	OCHR	;Ebene ausgeben
	IN	A,(88H)
	LD	E,A
	AND	80H
	CALL	WONOF	;Bit 7
	LD	D,'0'
	CALL	WRAM	;RAM0
	BIT	3,E
	JR	NZ,SYSC1
	SCF		;Schreibschutz
SYSC1:	BIT	1,E
	CALL	WONOF
	LD	D,'4'
	CALL	WRAM	;RAM4
	CALL	SPACE
	LD	A,(MODST+4)
	RRCA
	RRCA		;Bit 2
	AND	1
	ADD	'0'
	CALL	OCHR	;Ebene 0/1
	BIT	1,(IX+4)
	JR	NZ,SYSC2
	SCF		;Schreibschutz
SYSC2:	BIT	0,(IX+4)
	CALL	WONOF
	LD	D,'8'
	CALL	WRAM	;RAM8
	LD	A,' '
	CALL	OCHR
	LD	A,(IX+1)
	SRL	A	;Bits 4,5,6,7
	SRL	A
	SRL	A
	SRL	A	;nach 0,1,2,3
	SUB	2
	CALL	AHEX0	;Ebene ausg.
	IN	A,(89H)
	AND	A
	BIT	6,A
	JR	NZ,SYSC3
	SCF		;Schreibschutz
SYSC3:	BIT	5,A
	CALL	WONOF
	LD	A,(IX+1)
	LD	E,A
	CALL	OSTR
	DB	'BILD ',0
	BIT	2,E	;Zugriffs-Bit
	LD	A,'0'
	JR	Z,SYSC4
	INC	A
SYSC4:	CALL	OCHR	;Bildnummer
	LD	A,(MODST+1)
	AND	1	;IRM on?
	CALL	WONOF
	CALL	CRLF
	POP	DE
	RET
 
WRAM:	;Schreibe 'RAMn'
	CALL	OSTR
	DB	'RAM',0
	LD	A,D
	CALL	OCHR
	AND	A	;CY=0
	RET
 
WONOF:	;Schreibe ON, ON* oder OFF
	JR	Z,WOF
	PUSH	AF
	CALL	OSTR
	DB	' ON',0
	POP	AF
	LD	A,' '
	JR	NC,WRR
	LD	A,'*'	;Schreibschutz
WRR:	CALL	OCHR
	CALL	CRLF
	RET
	;
WOF:	CALL	OSTR
	DB	' OFF',CR,LF,0
	RET
 
MODULC:	;Modulkontrollanzeige
	PUSH	DE
	LD	L,7	;ab Platz 7
MODC1:	PUSH	HL
	LD	A,1	;lesen
	CALL	MODUC
	LD	A,H
	INC	A	;Kennbyte=FFh?
	JR	Z,MODC2	;kein Modul
	LD	A,L
	CALL	AHSPC	;Platz
	LD	A,H
	CALL	AHSPC	;Kennbyte
	LD	A,D
	CALL	AHSPC	;Steuerbyte
	LD	C,H
	LD	B,28	;Anzahl in MTAB
	LD	HL,MTAB
MODCC:	LD	A,(HL)
	INC	HL
	CP	C
	JR	Z,MODCA	;Text gefunden
MODCB:	LD	A,(HL)
	INC	HL
	AND	A	;Stringende?
	JR	NZ,MODCB
	DJNZ	MODCC
MODCA:	CALL	ZKOUT	;Modulname
	CALL	CRLF
MODC2:	POP	HL
	INC	L	;Platz+1
	JR	NZ,MODC1;bis FFh
	CALL	CRLF
	POP	DE
	RET
 
MTAB:	;Tabelle der Modulnamen
	DB	001H,'START-ROM',0
	DB	070H,'32K EPROM',0
	DB	071H,'64K EPROM',0
	DB	072H,'128K EPROM',0
	DB	073H,'256K EPROM',0
	DB	078H,'128K RAM',0
	DB	079H,'256K RAM',0
	DB	07AH,'512K RAM',0
	DB	07BH,'1M RAM',0
	DB	0A7H,'FLOPPY',0
	DB	0DAH,'PIO-3',0
	DB	0DBH,'EPROMMER',0
	DB	0E3H,'DAU1',0
	DB	0E7H,'ADU1',0
	DB	0ECH,'SCANNER',0
	DB	0EEH,'V.24',0
	DB	0EFH,'DIG. IN/OUT',0
	DB	0F0H,'8K CMOS-RAM',0
	DB	0F1H,'16K CMOS-RAM',0
	DB	0F2H,'32K CMOS-RAM',0
	DB	0F4H,'16K RAM',0
	DB	0F5H,'32K RAM',0
	DB	0F6H,'64K RAM',0
	DB	0F7H,'8K EPROM',0
	DB	0F8H,'16K EPROM',0
	DB	0FBH,'8K SOFTWARE',0
	DB	0FCH,'BASIC',0
	DB	0FDH,'USB+NET',0
	DB	'???',0
 
;Module lesen und schalten (Men}wort):
 
SWIC:	LD	D,E	;Steuerbyte
	DEC	L
	JR	NZ,SWI2	;nicht IRM
	CP	1
	JR	Z,SWI2	;nur lesen
	INC	L
	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
	LD	(HL),D		; IRM eintragen!
	LD	H,0FFH
	JR	SWI3
SWI2:	INC	L
	CALL	MODUC	;lesen/schalten
SWI3:	LD	A,L
	CALL	AHSPC	;Platz anzeigen
	LD	A,H
	CALL	AHSPC	;Kennbyte
	LD	A,D
	CALL	AHEX	;Steuerbyte
	JP	CRLF
 
MODUC:	;Module schalten
	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
	LD	C,80H	;I/O-Adr.
	LD	B,L
	CP	2	;Parameter?
	JR	NC,MODU1
	LD	D,(HL)	;nur lesen
RSTRB:	IN	H,(C)
	RET
 
MODU1:	LD	A,(HL)	;alter Zustand
	LD	(HL),D	;eintragen
	LD	H,A	;f}r 2. RAM4
	LD	A,L
	CP	6	;interne Module?
	JR	C,MODU2
	OUT	(C),D	;Senden Steuerb.
	JR	RSTRB
 
MODU2:	LD	B,L
	INC	B
	IN	A,(88H)
	DJNZ	MODU3
	AND	0F5H	;0=RAM0
	BIT	0,D
	JR	Z,OUT88
	SET	1,A
	BIT	1,D
	JR	Z,OUT88
	SET	3,A
	JR	OUT88
 
MODU3:	DJNZ	MODU4
	BIT	0,D	;1=IRM
	RES	2,A
	JR	Z,OUT88
	SET	2,A
	JR	OUT88
 
MODU4:	DJNZ	MODU5
	LD	A,D	;2=ROMC
	CPL		;negieren
	ADD	A
	AND	60H	;ausfiltern
	LD	H,A
	DI
	LD	A,(IX+4)
	AND	9FH
	OR	H	;einbauen
	LD	(IX+4),A
	OUT	(86H),A
	EI
	IN	A,(88H)
	BIT	0,D
	RES	7,A	;aus
	JR	Z,OUT88
	SET	7,A	;ein
OUT88:	OUT	(88H),A
	JR	STBFF
 
MODU5:	DJNZ	MODU6
	IN	A,(89H)	;3=RAM8
	AND	9FH
	BIT	0,D
	JR	Z,SRAM8	;wenn aus dann
	SET	5,A
	BIT	1,D	;Schreibschutz
	JR	Z,SRAM8	;unerheblich
	SET	6,A
SRAM8:	OUT	(89H),A
	LD	A,D
	RLCA
	RLCA
	ADD	20H	;Segm.-Verschieb
	AND	0F0H
	LD	H,A
	LD	A,(IX+1)
	AND	0FH
	OR	H	;RAM8-Segment
	OUT	(84H),A
	LD	(IX+1),A
STBFF:	LD	H,0FFH
	RET
 
MODU6:	DJNZ	STBFF	;5=CAOSC
	LD	A,D	;4=RAM4
	XOR	H
	AND	4	;Aenderung?
	JR	Z,RAM41
	LD	A,(IX+4)
	OR	3
	DI
	OUT	(86H),A	;RAM4 on
	PUSH	DE
	PUSH	HL
	IN	A,(88H)
	PUSH	AF
	RES	0,A
	OUT	(88H),A	;CAOSE off
	LD	A,(IX+1)
	PUSH	AF
	AND	0F9H
	LD	L,3	;3 IRM-Ebenen
	LD	BC,1000H;800H frei!
	LD	DE,4000H
R4IS:	ADD	2
	OUT	(84H),A	;IRM-Ebene
	LD	H,A
	PUSH	HL
	LD	HL,0A800H
R4MV:	LD	A,(DE)
	LDI
	DEC	HL
	LD	(HL),A
	INC	HL
	JP	PE,R4MV
	POP	HL
	LD	A,H
	LD	B,18H	;jetzt 1800H!
	DEC	L
	JR	NZ,R4IS
	POP	AF
	OUT	(84H),A	;IRM wie zuvor
	POP	AF
	OUT	(88H),A	;CAOSE on
	POP	HL
	POP	DE
RAM41:	LD	A,(IX+4)
	AND	0FCH
	BIT	0,D
	JR	Z,SRAM4
	SET	0,A
	BIT	1,D
	JR	Z,SRAM4
	SET	1,A
SRAM4:	LD	(IX+4),A
	OUT	(86H),A
	EI
	JR	STBFF
 
KEYC:	;F-Taste belegen
	LD	HL,0B900H
	AND	A
	JR	NZ,KEY0
	LD	B,9CH
KEYCL:	LD	(HL),A	;l|schen
	INC	L
	DJNZ	KEYCL
	RET
KEY0:	CP	16
	RET	NC	;nur F1 bis FF
	LD	B,A
	OR	0F0H
	LD	C,A
KEY1:	LD	A,(HL)	;B Dummys
	INC	L	;ausz{hlen
	RET	Z
	AND	A
	JR	NZ,KEY1
	DJNZ	KEY1
	LD	A,L
	CP	9CH
	RET	NC
	LD	DE,(CURSO)
	CALL	KEYDI	;pr{sentieren
KEY2:	CALL	INTB
	CP	13H
	JR	NZ,KEY3
	INC	B	;Umschaltung
	JR	KEY2
KEY3:	BIT	0,B
	JR	Z,KEY4	;interpret.
	CP	3
	JR	Z,KEY12	;BRK=Abbruch
	CP	CR
	JR	Z,KEY5	;Enter=belegen
	CALL	OCHR
	JR	KEY2
KEY4:	CALL	CSTBT
	JR	KEY2
	;
KEY5:	PUSH	HL	;Pos. in Puffer
	CALL	DABR
	EX	DE,HL
	CALL	RHEX	;Fn lesen
	POP	HL
	JR	C,KERR	;Formatfehler
	LD	A,(NUMVX)
	CP	C	;ge{ndert!
	JR	Z,KEY6
KERR:	CALL	CRLF
	JP	ERRM
KEY6:	INC	DE
	INC	DE
	LD	B,9BH
	CPL
	AND	A
	JR	Z,KEY8	;KEY F
	INC	A
	LD	B,A
	LD	C,L
KEY7:	LD	A,(HL)
	INC	L
	JR	Z,KERR	;Puffer voll!
	AND	A
	JR	NZ,KEY7
	DJNZ	KEY7	;Ende suchen
	DEC	L
	LD	A,L
	CP	9CH
	JR	NC,KERR	;Puffer voll!
	LD	B,H
	PUSH	BC
	SUB	C
	LD	C,A
	LD	B,0
	PUSH	DE
	LD	DE,0B99BH
	LDDR		;Rest hinter
	LD	B,E
	POP	DE
	POP	HL
KEY8:	LD	C,A
KEY9:	DEC	L
	LD	A,(HL)
	AND	A
	JR	NZ,KEY9	;zur}ck bis 0
KEY10:	INC	L
	LD	A,(DE)
	INC	DE
	LD	(HL),A	;belegen
	OR	A
	JR	Z,KEY11
	LD	A,B
	SUB	L
	JR	NZ,KEY10
	LD	(HL),0
	JR	KERR
KEY11:	LD	A,C
	AND	A
	JR	Z,KEY12	;KEY F
	INC	B
	INC	L
	LD	E,L
	LD	D,H
	LD	L,B
	LD	B,0
	LDIR		;Rest nach vorn
KEY12:	JP	CRLF
 
KEYLIC:	;Anzeige belegter F-Tasten
	LD	HL,0B901H
	LD	BC,0F1H
KEYLC1:	LD	A,(HL)
	AND	A
	JR	Z,KEYLC2
	CALL	KEYDI	;Belegung
	CALL	CRLF
	INC	B	;Z{hler
KEYLC2:	INC	L
	LD	A,L
	CP	9CH
	JR	NC,KEYLC3
	INC	C
	JR	NZ,KEYLC1 ;F1 bis FF
KEYLC3:	INC	B
	DEC	B
	RET	NZ
	CALL	OSTR
	DB	'F-Tasten leer'
	DB	CR,LF,0
	RET
 
KEYDI:	LD	A,2
	CALL	OCHR	;Zeile l|schen
	LD	A,C
	CALL	AHOSTR	;Fn
	DB	' :',0
KEYD1:	LD	A,(HL)
	OR	A
	RET	Z
	CALL	CSTBT
	INC	HL
	LD	A,L
	CP	9CH
	RET	NC
	JR	KEYD1
 
SQRC:	;Quadratwurzel 16Bit A=SQR(HL)
	AND	A	;CY=0
	LD	DE,0FFFFH
SQ1:	INC	DE
	INC	DE
	SBC	HL,DE
	JR	NC,SQ1
	LD	A,E
	SRL	D
	RRA		;A=RADIUS
	RET
 
MULC:	;Multiplikation BA=C*D
	LD	B,0
	LD	E,9
MULC1:	LD	A,D
	RRA
	DEC	E
	RET	Z
	LD	D,A
	LD	A,B
	JR	NC,MULC2
	ADD	C
MULC2:	RRA
	LD	B,A
	JR	MULC1 	;8*
 
COLRC:	AND	A
	JR	NZ,SETCO
	;aktuelle Farben anzeigen
	LD	A,(COLOR)
	LD	L,A
	RRCA
	RRCA
	RRCA
	AND	1FH
	CALL	AHSPC
	LD	A,L
	AND	7
	CALL	AHEX
	JP	CRLF
SETCO:	;Farbe setzen (aus L und E)
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
	LD	A,(COLOR);Hintergrund
COLRC1:	AND	7
	OR	L
	LD	(COLOR),A
	RET
 
WINDC:	;WINDOW-Men}wort
	AND	A	;Kein Arg?
	JR	Z,WINAKC;Aufruf No.0
	DEC	A	;1 Arg?
	LD	A,L	;nr
	JR	Z,WINAKC;Aufruf No.L
	LD	H,L	;za
	LD	L,C	;sa
	LD	D,E	;zn
	LD	A,(ARG4)
	LD	E,A	;sn
	LD	A,(ARGN)
	CP	4
	JR	C,WINC2	;2..3 Arg
	LD	A,0
	JR	Z,WINC1	;4 Arg
	LD	A,(ARG5);nr
WINC1:	CALL	WININC
WINC2:	JP	C,ERRM
	RET
 
WINAKC:	;def. Fenster aufrufen
	CALL	WCOPY
	LD	(WINNR),A
	CALL	PART	;Fenster A
	RET	C	;holen
	EX	DE,HL
	LD	DE,WINON
	JR	WCOP1
 
PART:	;Testet A und berechnet Pos.
	;in Fenstervektorspeicher =DE
	ADD	0F6H
	RET	C	;zu gro~
	ADD	A
	LD	E,A
	ADD	A
	ADD	A
	ADD	E
	LD	E,A
	LD	D,0B9H
	AND	A	;CY=0
	RET
 
WCOPY:	;Fenster (WINNR) retten
	PUSH	AF
	LD	A,(WINNR)
	CALL	PART
	JR	C,WINIE	;Fehler
	POP	AF
	LD	HL,WINON
WCOP1:	LD	BC,10
	LDIR
	RET
 
WININC:	;Initialisierung eines Fensters
	PUSH	AF
	CP	10	;No.
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
	ADD	L
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
	LD	(CURSO),BC	;BC=0
	LD	(WINNR),A
	AND	A
	RET
	;
WINIE:	POP	AF	;Fehlerende
	SCF
	RET
 
LINEC:	;Linie zeichnen von
;	(ARG1)/(ARG2) nach (ARG3)/(ARG4)
	 LD	DE,(ARG2) ;YANF
	 LD	HL,(ARG4) ;YEND
	 EXX
	LD	DE,(ARG1) ;XANF
	PUSH	DE
	LD	HL,(ARG3) ;XEND
	LD	A,8	;X=Master
	OR	A
	SBC	HL,DE	;HL=XSTEPS
	JR	NC,LINC1;vorw{rts
	ADD	HL,DE
	EX	DE,HL
	OR	1	;r}ckw{rts
	SBC	HL,DE
LINC1:	 EXX
	 PUSH	DE	;YANF
	 SBC	HL,DE	;HL'=YSTEPS
	 JR	NC,LINC2;steigend
	 ADD	HL,DE
	 EX	DE,HL
	 OR	2	;fallend
	 SBC	HL,DE
LINC2:	 PUSH	HL	;YSTEPS
	 EXX		;HL=XSTEPS
	POP	BC	;BC=YSTEPS
	PUSH	HL
	SBC	HL,BC	;flach/steil?
	POP	HL
	PUSH	BC
	JR	NC,LINC3;flach
	EX	(SP),HL
	RES	3,A	;X=Slave
	SET	2,A	;Y=Master
LINC3:	PUSH	HL	;HL=MAX(STEPS)
	LD	B,H
	LD	C,L	;Pixelz{hler
	 EXX
	 POP	BC	;BC'=MAX(...)
	 POP	DE	;DE'=MIN(...)
	 LD	H,B
	 LD	L,C
	 SRL	H	;Slave mit 1/2
	 RR 	L	;HL=1/2*MAX(...)
	 EXX
	POP	DE	;DE=YANF
	POP	HL	;HL=XANF
 
	;Hauptschleife
;AF Bitregister
;HL lfd. X-Koordinate	HL' Slave
;DE lfd. Y-Koordinate	DE' -Master
;BC Punktez{hler	BC' +Master
 
LINCS:	PUSH	AF
	EX	AF,AF'	;Bitreg merken
	POP	AF
	PUSH	HL
	LD	HL,LINTYP
	RLC	(HL)
	POP	HL
	CALL	C,SHLDE;Punkt setzen
	 EXX
	 AND	A
	 SBC	HL,DE	;-Master subtr.
	 JR	NC,LINC4;kein Uebertrag-
			;kein Sprung!
	 ADD	HL,BC	;+Master add.
			;CY bleibt =1!
LINC4:	 EXX
	JR	NC,LINC5
	OR	0CH	;X und Y!
LINC5:	BIT	3,A	;X?
	JR	Z,LINC6
	INC	HL
	BIT	0,A
	JR	Z,LINC6
	DEC	HL
	DEC	HL
LINC6:	BIT	2,A	;Y?
	JR	Z,LINC7
	INC	DE
	BIT	1,A
	JR	Z,LINC7
	DEC	DE
	DEC	DE
LINC7:	LD	A,B
	OR	C	;letzter Punkt?
	RET	Z
	DEC	BC
	EX	AF,AF'	;Bitreg
	JR	LINCS
 
PUSEC:	;Punkt setzen
	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	CALHV	;berechnen
	CALL	NC,PUSET;& setzen
	POP	BC
	POP	DE
	POP	HL
	RET
 
CIRCLC:	;Kreis zeichnen
	LD	A,(ARG3)
	LD	C,A
	LD	L,C
	XOR	A
	LD	B,8
	RR 	L
CIR1:	JR	NC,CIR2
	ADD	C
CIR2:	RRA
	RR 	L
	DJNZ	CIR1
	LD	H,A	;Achtelkreis
	LD	D,H	;berechnen
	LD	E,L
	LD	A,C
	SLA	C
	RL 	B
	DEC	BC
	PUSH	BC
	LD	BC,1
	EXX
	LD	C,A
	LD	B,0H
CIR3:	PUSH	HL
	LD	HL,LINTYP
	RLC	(HL)
	POP	HL
	JR	NC,CIRN
	CALL	CIRUP	;und jeweils 8
	CALL	CIRUP	;Punkte setzen
CIRN:	EXX
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
CIR4:	INC	B
	LD	A,C
	CP	B
	JR	NC,CIR3
	POP	AF
	RET
 
CIRUP:	;setzt 4 Punkte um Mittelpunkt
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
CIRUP1:	PUSH	HL
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
SHLDE:	;setzt Punkt auf Pos. HL/DE
	;(wenn m|glich)
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	CALL	CALXY	;berechnen
	CALL	NC,PUSET;& setzen
	POP	AF
	POP	BC
	POP	DE
	POP	HL
	RET
	;
PUSET:	BIT	3,(IX+1)
	JR	Z,HIRES	;HiRes
	BIT	1,D
	JR	NZ,PUERA
	BIT	0,D
	JR	NZ,PUXOR
	OR	(HL)	;setzen
	JR	PUCOL
PUXOR:	XOR	(HL)	;alternierend
	RES	0,D
PUCOL:	LD	(HL),A
	LD	A,(IX+1)
	LD	E,A
	XOR	2	;Farbebene
	DI
	OUT	(84H),A
	LD	A,(HL)
	AND	7
	OR	D
	LD	(HL),A	;neue vFarbe
	LD	A,E
	OUT	(84H),A
	EI
	RET
PUERA:	CPL
	AND	(HL)	;l|schen
	LD	(HL),A
	RET
	;
HIRES:	LD	B,A
	OR	(HL)	;Bit setzen in
	BIT	3,D	;Pixelebene?
	JR	NZ,HIR1
	XOR	B	;wieder l|schen
HIR1:	LD	(HL),A
	LD	A,(IX+1)
	LD	C,A
	XOR	2
	DI
	OUT	(84H),A
	LD	A,B
	OR	(HL)	;Bit setzen in
	BIT	4,D	;Farbebene?
	JR	NZ,HIR2
	XOR	B	;wieder l|schen
HIR2:	LD	(HL),A
	LD	A,C
	OUT	(84H),A
	EI
	RET
 
PUDEC:	;Punkt l|schen, PA: A=Farbbyte
	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	CALHV
	JR	C,PUDEE	;au~erhalb
	LD	B,A	;Bit
	LD	C,(HL)
	CPL
	AND	C
	LD	(HL),A	;l|schen
	LD	A,(IX+1)
	LD	E,A
	XOR	2
	DI
	OUT	(84H),A
	LD	D,(HL)	;Farbbyte
	LD	A,E
	OUT	(84H),A
	EI
	LD	A,B	;Z=1: Punkt
	AND	C	;war gel|scht
	LD	A,D	;Farbbyte
PUDEE:	POP	BC
	POP	DE
	POP	HL
	RET
 
;Grafikposition berechnen
;HL=X-Koor., DE=Y-Koor.
;PA:	CY=1	au~erhalb
;HL=IRM-Adr, A=Bitmaske, D=Farb
 
CALHV:	LD	HL,(HOR)
	LD	DE,(VERT)
	LD	D,0
CALXY:	PUSH	BC
	LD	BC,BITTAB
	LD	A,L
	AND	7
	ADD	C
	LD	C,A	;Bit
	LD	A,L
	SRL	H
	RRA
	SRL	H
	RRA
	SRL	H
	RRA		;Spalte
	CP	28H
	JR	NC,CALEN
	OR	80H	;IRM
	LD	H,A
	LD	A,0FFH
	ADD	D
	JR	C,CALEN
	XOR	E	;CY=0!
	LD	L,A	;Zeile
	LD	A,(FARB)
	LD	D,A
	LD	A,(BC)	;MASKE fuer X
	POP	BC
	RET
CALEN:	POP	BC	;au~erhalb
	SCF
	RET
 
DPMEMO:	;Ausgabe einer Displayzeile
	;HL=Adr, C=Anzahl Bytes
	CALL	HLHX	;Adresse
	PUSH	HL
	LD	B,C
DPME1:	CALL	LDAME	;Byte holen
	INC	HL
	CALL	AHSPC	;anzeigen
	DJNZ	DPME1	;C*
	POP	HL
	LD	B,C
	LD	A,9
	CALL	OCHR	;CUR
DPME2:	CALL	LDAME	;Byte holen
	INC	HL
	CALL	CSTBT	;als ASCII
	DJNZ	DPME2	;C*
	RET
 
DISPC:	;Anzeige Speicherbereich
	;HL=Adr, E=Listblockl{nge
	;C=Anzahl Bytes pro Zeile
	;A=Anzahl Argumente
	CP	3
	JR	NC,DISP1
	LD	C,8	;default
	CP	2
	JR	NC,DISP1
	LD	E,4	;default
DISP1:	PUSH	DE
DISP2:	CALL	DPMEMO	;1 Zeile
	CALL	CRLF
	CALL	BRKT
	JR	C,DISP4
	DEC	E
	JR	NZ,DISP2;E*
DISP3:	CALL	INTB	;Warten Taste
DISP4:	POP	DE
	CP	3	;BRK?
	RET	Z
	CP	0FH
	JR	NZ,DISP5
	CALL	OCHR	;Hardcopy
	JR	DISP3
DISP5:	CP	13H	;STOP?
	JR	NZ,DISP1
	JR	MODI1
 
MODIC:	;Anzeige/Ver{ndern Speicher-
	;bereich, Par. wie DISPC
	LD	C,E	;Breite
	CP	2
	JR	NC,MODI1
	LD	C,1	;default
MODI1:	CALL	DPMEMO
	LD	A,5
	LD	(CURSO),A
	CALL	INLIN
	RET	C	;BRK
	PUSH	HL
	CALL	RHEX	;Adresse
	LD	A,(HL)
	POP	HL
	JR	C,MODI9	;Fehler
	AND	A
	JR	Z,MODI1
	LD	HL,(NUMVX)
MODI2:	PUSH	HL
	CALL	RHEX	;Datenbyte
	LD	A,(HL)
	AND	A	;kein Zeichen
	INC	HL	;einlesbar?
	LD	A,(HL)
	POP	HL
	JR	Z,MODI4
MODI3:	CALL	LDMAE	;eintragen
	INC	HL
MODI4:	LD	A,(DE)
	CP	'.'	;Abschlu~?
	RET	Z
	CP	','	;1 Zeichen?
	JR	NZ,MODI5
	INC	DE
	LD	A,(DE)	;vom Video-RAM
	INC	DE
	JR	MODI3	;eintragen
 
MODI5:	CP	':'	;R}ckschritt?
	JR	NZ,MODI6
	LD	B,0
	SBC	HL,BC	;C Bytes zur}ck
	JR	MODI1
 
MODI6:	CP	'/'	;neue Adresse?
	JR	NZ,MODI0
	INC	DE
	CALL	RHEX
	JR	C,MODI9
	LD	HL,(NUMVX)
	JR	MODI1
 
MODI0:	CP	27H	;Zeichenkette?
	JR	NZ,MODI8
	INC	DE
MODI7:	LD	A,(DE)	;Zeichen holen
	INC	DE
	AND	A	;Dummy-Ende?
	JR	Z,MODI1
	CP	27H	;Ende Kette?
	JR	Z,MODI4
	CALL	LDMAE	;eintragen
	INC	HL
	JR	MODI7
 
MODI8:	AND	A	;Ende Zeile?
	JR	Z,MODI1
	CP	' '	;Leerzeichen?
	JR	Z,MODI2
MODI9:	CALL	ERRM	;sonst ERROR
	JR	MODI1
 
 
CENC:	;Initialisierung Centronics-
	;Drucker, PE:Args
	LD	HL,ARGN
	LD	A,(HL)
	INC	(HL)	;Args wie V24!
	AND	A
	JR	Z,CEN1	;kein Arg
	INC	HL
	LD	B,(HL)	;Platz
	LD	C,80H
	IN	A,(C)
	CP	0EFH	;M001?
	JR	NZ,CEN1
	LD	L,B
	LD	A,2
	LD	D,1
	CALL	MODUC	;M001 ein
	LD	HL,M001PR
	JR	CEN2
CEN1:	LD	HL,M021PR
CEN2:	LD	(OFILT+1),HL
	LD	HL,ARG1
	JR	CEN3
;
; Initialisierung V.24-Druckerausgabe:
; PE:	Argumente von Eingabe
;
V24OC:	LD	HL,ARGN
	LD	A,(HL)		; Anzahl Argumente
	CP	2
	JR	NC,V24O3	; mehr als 1 Arg
	CALL	V24SU		; M003 ab Steckplatz 7 suchen
	JR	C,V24O6		; nicht gefunden -> ERROR
	LD	HL,ARG2
CEN3:	LD	A,(ARGN)
	CP	3		; mind. 3 Argumente?
	JR	NC,V24O5	; USER-Ausgabekanal angegeben
	LD	HL,HCPGM
	LD	(HCADR),HL	; ShCLR belegen
	LD	DE,PRINT
	LD	HL,HCPZ
	BIT	1,(HL)		; UOUT1 oder 2?
	JR	NZ,V24O1
	LD	(UOUT1+1),DE
	JR	V24O2
V24O1:	LD	(UOUT2+1),DE
V24O2:	JP	V24O12
 
V24O3:	INC	HL		; HL=ARG1
	LD	B,(HL)		; ab Steckplatz
	CALL	V24S0		; M003 suchen
	JR	C,V24O6		; nicht gefunden -> ERROR
	INC	HL
	INC	HL	;ARG2
	LD	A,(HL)	;SIO A oder B?
	DEC	A
	JR	NZ,V24O4
	LD	A,(HCPZ)
	RES	2,A	;merken SIOA
	LD	(HCPZ),A
	JR	V24O5
 
V24O4:	DEC	A
	JR	NZ,V24O6;Fehler
	LD	A,(HCPZ)
	SET	2,A	;merken SIOB
	LD	(HCPZ),A
V24O5:	INC	HL
	INC	HL
	LD	DE,PRINT
	LD	A,(HL)	;USER1 oder 2?
	DEC	A
	DEC	A
	JR	NZ,V24O7
	LD	(UOUT1+1),DE
	LD	DE,HCPZ
	LD	A,(DE)
	RES	1,A	;merken USER1
	JR	V24O8
 
V24O6:	JP	ERRM	;Fehlersprung
 
V24O7:	DEC	A
	JR	NZ,V24O6;Fehler
	LD	(UOUT2+1),DE
	LD	DE,HCPZ
	LD	A,(DE)
	SET	1,A	;merken USER2
V24O8:	LD	(DE),A
	LD	A,(ARGN)
	CP	4	;4. Argument?
	JR	C,V24O12;nein
	INC	HL
	INC	HL
	LD	A,(HL)	;ShCLR?
	AND	A
	LD	DE,NOT
	JR	Z,V24O11;keine Fkt.
	DEC	A
	JR	Z,V24O9	;Protokoll
	DEC	A	;Hardcopy?
	JR	NZ,V24O6
	LD	A,(HCPZ)
	SET	0,A	;Hcopy merken
	JR	V24O10
 
V24O9:	LD	A,(HCPZ)
	RES	0,A	;Hcopy aus
V24O10:	LD	(HCPZ),A
	LD	DE,HCPGM
V24O11:	LD	(HCADR),DE
	LD	A,(ARGN)
	CP	5	;5. Argument?
	JR	C,V24O12;nein
	INC	HL
	INC	HL
	LD	A,(HL)	;Druckertyp
	SLA	A
	SLA	A
	SLA	A	;nach obere
	SLA	A	;Tetrade
	LD	C,A
	LD	A,(HCPZ)
	AND	0FH	;in (HCPZ)
	OR	C	;einbauen
	LD	(HCPZ),A
V24O12:	LD	A,(OFILT+1)
	CP	LOW(M001PR)
	LD	HL,IN01
	JR	Z,INICEN
	CP	LOW(M021PR)
	LD	HL,IN21
INICEN:	JP	Z,INIMEX;Init Centron.
	;Init V.24-Druck
	CALL	LCSO
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
 
IN01:	;Inittabelle f}r M001
	DB	3
	DB	6	;PIO-A Stw
	 DB	2
	  DB	0CFH	;Bitbetrieb
	  DB	0	;alles Ausg.
	DB	7	;PIO-B Stw
	 DB	2
	  DB	0CFH	;Bitbetrieb
	  DB	0FEH	;Bit 0 Ausg.
	DB	5	;POI-B Daten
	 DB	1
	  DB	1	;Strobe passiv
 
IN21:	;Inittabelle f}r M021
	DB	3
	DB	93H	;PIO-B Stw
	 DB	2
	  DB	0CFH	;Bitbetrieb
	  DB	0	;alles Ausg.
	DB	92H	;PIO-A Stw
	 DB	2
	  DB	0CFH	;Bitbetrieb
	  DB	7FH	;Bit 7 Ausg.
	DB	90H	;PIO-A Daten
	 DB	1
	  DB	80H	;Strobe passiv
;
; V.24-Modul suchen:
; PE:	B	Steckplatz ab dem gesucht wird
;
V24SU:	LD	B,7		; ab Steckplatz 7
V24S0:	LD	C,80H
V24S1:	IN	A,(C)
	CP	0EEH	;M003?
	JR	Z,V24S2	;ja
	INC	B
	JR	NZ,V24S1
	SCF		;nicht gef.
	RET
	;
V24S2:	PUSH	HL
	LD	L,B
	LD	A,2
	LD	D,1
	CALL	MODUC	;einschalten
	LD	HL,V24PR
	LD	(OFILT+1),HL
	POP	HL
	RET
 
PRINTC:	;allgemeine Druckroutine
	PUSH	AF
	LD	A,(HCPZ)
	BIT	7,A
	JR	Z,PRINT3;Drucker
	;Schreibmaschine
	AND	0F0H
	CP	90H
	JR	Z,S6005
	CP	0A0H
	JR	NZ,PRINT3
S6010:	POP	AF
	PUSH	HL
	PUSH	BC
	LD	HL,ZIBM
	LD	BC,7
	CPIR
	JR	NZ,PRINT2
	LD	C,6
	ADD	HL,BC
	LD	A,(HL)
	JR	PRINT2
 
S6005:	POP	AF
	CP	'~'
	JR	NZ,PRINT4
	LD	A,83H
	JR	PRINT4
PRINT2:	POP	BC
	POP	HL
	PUSH	AF
PRINT3:	POP	AF
PRINT4:	CP	9	;CUR
	JR	Z,PRINT5
	PUSH	AF
	LD	A,(STBT)
	BIT	3,A	;Steuerzeichen
	JR	Z,PRINT7;ausf}hren
	POP	AF
	CP	7FH	;und 7Fh
	JR	NZ,PRINT6
PRINT5:	LD	A,20H	;nach SPC
PRINT6:	CP	20H	;und Steuerzei-
	JR	NC,PRINT8;chen nach '_'
	LD	A,'_'	;wandeln
	PUSH	AF
PRINT7:	POP	AF
PRINT8:	JP	OFILT	;in IRM
 
ZIBM:	;IBM-Kodierungen
	DB	'{|}~'
	DB	'[\]'
	DB	84H,94H,81H,0E1H
	DB	8EH,99H,9AH
 
HCPGMC:	;ShCLR-Reaktionsprogramm
	PUSH	HL
	PUSH	DE
	LD	A,(HCPZ)
	BIT	0,A	;Hardcopy?
	JP	NZ,COPIES
 
	;Protokollfunktion
	LD	HL,(OUTAB)
	XOR	A
	OR	(HL)	;CRT?
	JR	NZ,PROTO2
	LD	DE,ECHO	;CRT+PRINT
	LD	HL,ZEI3	;UOUT2
	LD	A,(HCPZ)
	BIT	1,A	;USER1/2?
	JR	Z,PROTO1
	JR	PROTO3
 
PROTO1:	LD	HL,ZEI2	;UOUT1
	JR	PROTO4
 
PROTO2:	;Protokoll deaktivieren
	LD	DE,PRINT
	LD	HL,ZEI0	;CRT
	DEC	A
	DEC	A
	JR	Z,PROTO4
PROTO3:	LD	(UOUT2+1),DE
	JR	PROTO5
	;
PROTO4:	LD	(UOUT1+1),DE
PROTO5:	LD	(OUTAB),HL ;Zeiger
	POP	DE
	POP	HL
	RET
 
COPIES:	;Hard- oder Screencopy
	PUSH	BC
	PUSH	AF
	LD	A,(HCPZ)
	AND	0F0H
	BIT	7,A
	JR	Z,HCOPYC;Hardcopy
;Screencopy
	LD	HL,(VRAM)
	LD	B,32
SCOPY1:	LD	C,40
SCOPY2:	LD	A,(HL)
	INC	HL
	CP	20H
	JR	NC,SCOPY4
	LD	A,' '	;statt Steuerz.
SCOPY4:	CALL	PRINT
	DEC	C	;40 Spalten
	JR	NZ,SCOPY2
	LD	A,CR
	CALL	PRINT4
	LD	A,LF
	CALL	PRINT4
	DJNZ	SCOPY1	;32 Zeilen
HCEND:	POP	AF
	POP	BC
	POP	DE
	POP	HL
	RET
 
HCOPYC:	;..nach Typ verzweigen
	AND	0F0H
	CP	10H
	JP	Z,K6314	;ESC/P breit
	CP	20H
	JP	Z,K6311	;mit ANSI
	CP	30H
	JP	Z,K6312	;ANSI breit
	CP	50H
	JR	Z,HCMIN	;ESC/P2 (Mini)
	CP	70H
	JR	Z,ESCP2	;ESC/P2 24 Nadel
 
;K6313, K6304, ESC/P2 mit 8 Nadeln
K6313:	LD	H,0	;Zeile 0
K6313a:	CALL	PRZK	;Grafik init
	LD	L,0	;Spalte 0
K6313b:	LD	C,8
	CALL	CBYTES
	PUSH	HL
K6313c:	LD	B,8
	LD	HL,CASS
K6313d:	RL 	(HL)
	RLA
	INC	HL
	DJNZ	K6313d
	CALL	OFILT
	DEC	C
	JR	NZ,K6313c ;n{chster
	POP	HL	  ;Cursor
	INC	L
	LD	A,40
	CP	L
	JR	NZ,K6313b
	LD	A,H
	ADD	8	;n{chste Zeile
	LD	H,A
	JR	NC,K6313a
HCCRLF:	LD	A,CR
	CALL	OFILT
	LD	A,LF
	CALL	OFILT
	JR	HCEND
 
HCMIN:	;ESC/P2 Miniformat
	LD	HL,0	;Z=0, Sp=0
MINIa:	CALL	PRZK
MINIb:	LD	C,24	;24 Pixel
	CALL	CBYTES
	CALL	S3PRS
	JR	NZ,MINIb
	LD	A,H
	ADD	24
	LD	H,A
	LD	L,0	;Sp=0
	CP	240
	JR	NZ,MINIa
	CALL	PRZK
MINIc:	LD	C,16
	CALL	CBYTES
	LD	B,8
	XOR	A
MINId:	LD	(DE),A	;8 Nadeln frei
	INC	DE
	DJNZ	MINId
	CALL	S3PRS
	JR	NZ,MINIc
	JR	HCCRLF
 
ESCP2:	;ESC/P2 mit 24 Nadeln
	LD	H,0	;Zeile 0
ESCP2a:	CALL	PRZK
	LD	L,0	;Spalte 0
ESCP2b:	LD	B,8	;8 Pixel
	PUSH	HL
	CALL	PADR
	LD	DE,CASS
ESCP2c:	LD	A,(HL)
	LD	(DE),A
	INC	DE
	LD	(DE),A	;3 mal ablegen
	INC	DE
	LD	(DE),A
	INC	DE
	INC	L
	DJNZ	ESCP2c
	LD	C,8
ESCP2d:	LD	B,24	;3*8 Bytes
	LD	HL,CASS
ESCP2e:	RL	(HL)
	RL	E
	RL	D
	RLA
	INC	HL
	DJNZ	ESCP2e	;Wandlung
	LD	(HL),A
	LD	B,3	;3 mal drucken
ESCP2f:	LD	A,(HL)
	CALL	OFILT	;1.
	LD	A,D
	CALL	OFILT	;2.
	LD	A,E
	CALL	OFILT	;3. Byte
	DJNZ	ESCP2f
	DEC	C
	JR	NZ,ESCP2d
	POP	HL
	INC	L
	LD	A,40	;Zeilenende?
	CP	L
	JR	NZ,ESCP2b
	LD	A,H
	ADD	A,8
	LD	H,A
	JR	NC,ESCP2a
	JP	HCCRLF
 
K6314:	;ESC/P 9-Nadeln (breit)
	LD	H,0
K6314a:	CALL	PRZK
	LD	L,0
K6314b:	LD	B,4
	PUSH	HL
	CALL	PADR
	LD	DE,CASS
K6314c:	LD	A,(HL)
	LD	(DE),A
	INC	DE
	LD	(DE),A
	INC	DE
	INC	L
	DJNZ	K6314c	;4*
	LD	C,8
K6314d:	LD	B,8
	LD	HL,CASS
K6314e:	RL 	(HL)
	RLA
	INC	HL
	DJNZ	K6314e
	CALL	OFILT
	CALL	OFILT
	DEC	C
	JR	NZ,K6314d
	POP	HL
	INC	L
	LD	A,40
	CP	L
	JR	NZ,K6314b
	LD	A,H
	ADD	4
	LD	H,A
	JR	NC,K6314a
	JP	HCCRLF
 
K6311:	;ANSI-Drucker
	CALL	PRZK
	LD	B,256/6
	LD	HL,0
K6311a:	LD	C,6
	CALL	CBYTES
	CALL	SPRS	;6*8-Feld ausg.
	JR	NZ,K6311a
	LD	A,6	;6 Pixel tiefer
	ADD	H
	LD	H,A
	LD	L,0
	CALL	PRZK
	DEC	B
	JR	NZ,K6311a
K6311b:	LD	C,256 MOD 6	;den Rest
	CALL	CBYTES
	XOR	A
	LD	(DE),A	;letzte beiden
	INC	DE	;Zeilen leeren
	LD	(DE),A
	CALL	SPRS
	JR	NZ,K6311b
	JP	HCCRLF
 
K6312:	;ANSI breit
	CALL	PRZK
	LD	B,256/3
	LD	HL,0
K6312a:	LD	C,3
	CALL	CBYTS2
	CALL	S2PRS
	JR	NZ,K6312a
	INC	H
	INC	H
	INC	H
	LD	L,0
	CALL	PRZK
	DJNZ	K6312a
K6312b:	LD	C,256 MOD 6	;den Rest
	CALL	CBYTS2
	XOR	A
	LD	(DE),A
	INC	DE
	LD	(DE),A
	CALL	S2PRS
	JR	NZ,K6312b
	JP	HCCRLF
 
CBYTES:	;C Bytes zum Puffer
	PUSH	HL
	PUSH	BC
	CALL	PADR
	LD	DE,CASS
	LD	B,0
	LDIR
	POP	BC
	POP	HL
	RET
 
CBYTS2:	;C Bytes zum Puffer & doppeln
	PUSH	HL
	CALL	PADR
	LD	DE,CASS
C2BYTL:	LD	A,(HL)
	LD	(DE),A
	INC	DE
	LD	(DE),A
	INC	DE
	INC	L
	DEC	C
	JR	NZ,C2BYTL
	POP	HL
	RET
 
SPRS:	;8* 6-Nadel-Sprosse ausgeben
	PUSH	HL
	PUSH	BC
	LD	L,80H
SPRS1:	LD	B,0
	LD	DE,CASS
SPRS2:	LD	A,(DE)
	AND	L	;CY=0
	JR	Z,SPRS3
	SCF		;CY=1
SPRS3:	RL 	B
	INC	DE
	LD	A,6	;f}r 6 Bit
	CP	E
	JR	NZ,SPRS2;ob. Nadel war 0
	AND	A	;unt. Nadel = 0
	RL 	B
	LD	A,B
	CALL	OFILT
	RRC	L
	BIT	7,L
	JR	Z,SPRS1
	POP	BC
	POP	HL
	INC	L
	LD	A,40
	CP	L
	RET
 
S2PRS:	;8* 6-Nadel-Sprosse doppelt out
	PUSH	HL
	PUSH	BC
	LD	L,80H
S2PR1:	LD	B,0
	LD	DE,CASS ;KassPuff
S2PR2:	LD	A,(DE)
	AND	L
	JR	Z,S2PR3
	SCF
S2PR3:	RL 	B
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
	INC	L
	LD	A,40
	CP	L
	RET
 
S3PRS:	;8* 24-Nadel-Sprosse ausgeben
	PUSH	HL
	LD	E,8
S3PR1:	LD	C,3	;3 Byte
	LD	HL,CASS
S3PR2:	LD	B,8
S3PR3:	RL	(HL)
	RLA
	INC	HL
	DJNZ	S3PR3	;Wandlung
	CALL	OFILT
	DEC	C
	JR	NZ,S3PR2
	DEC	E
	JR	NZ,S3PR1
 	POP	HL
	INC	L
	LD	A,40
	CP	L
	RET
 
PRZK:	;Druckerinit entsprechend HCPZ
	PUSH	HL
	PUSH	BC
	XOR	A
	LD	HL,HCPZ
	RLD
	LD	C,A	;Druckertyp
	RRD
	INC	C
	LD	HL,C6313
PRZK1:	LD	B,(HL)	;L{nge
	DEC	C
	JR	Z,PRZK2
	INC	B
	LD	A,L
	ADD	B
	LD	L,A
	JR	NC,PRZK1
	INC	H
	JR	PRZK1
PRZK2:	INC	HL
	LD	A,(HL)
	CALL	OFILT
	DJNZ	PRZK2
	POP	BC
	POP	HL
	RET
 
;Grafiksteuersequenzen f}r die Drucker
;
C6313:	DB	10	;L{nge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',5
	DW	320
 
C6314:	DB	10
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',5
	DW	640
 
C6311:	DB	14
	DB	ESC,'[01e'
	DB	ESC,'[12`'
	DB	ESC,'K'
	DW	320
 
C6312:	DB	14
	DB	ESC,'[01e'
	DB	ESC,'[12`'
	DB	ESC,'K'
	DW	640
 
C6304:	DB	9
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'K'
	DW	320
 
CMINI:	DB	10
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',39
	DW	320
 
C24N1:	DB	10
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',0
	DW	320
 
C24N2:	DB	10
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',39
	DW	960
 
V24DC:	;Duplex-Init
	;PE: Argumente
	LD	A,(ARGN)
	CP	3
	JR	C,V24DC2;<3 Argumente
	LD	HL,ARG1
	LD	B,(HL)	;ab Platz B
	CALL	V24S0	;M003 suchen
	JP	C,ERRM
	INC	HL
	INC	HL
	LD	A,(HCPZ2)
	BIT	0,(HL)	;SIO A oder B?
	RES	2,A
	JR	NZ,V24DC1
	SET	2,A
V24DC1:	LD	B,A
	INC	HL
	INC	HL
	LD	A,(HL)
	SUB	2	;USER1 oder 2?
	RES	1,B
	JR	Z,V24DC6
	SET	1,B
V24DC6:	LD	A,B
V24DCF:	LD	(HCPZ2),A
	JR	V24DC3
 
V24DC2:	CALL	V24SU	;M003 ab Steckplatz 7 suchen
	JP	C,ERRM
V24DC3:	LD	DE,V24OT
	LD	BC,V24IN
	LD	A,(HCPZ2)
	BIT	1,A	;USER-Kanal ?
	JR	NZ,V24DC4
	LD	(UOUT1+1),DE
	LD	(UIN1+1),BC
	JR	V24DC5
	;
V24DC4:	LD	(UOUT2+1),DE
	LD	(UIN2+1),BC
V24DC5:	CALL	LCSOD	;Kanal-Offset
	LD	A,0CH	;CTC
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
	LD	A,0AH	;SIO
	ADD	C
	LD	C,A
	OTIR		;SIO init.
	EI
	RET
;
; SIO Kanal B auf Empfangsinterrupt initialisieren:
;
UMINIT:	LD	HL,V24OT		; Ausgaberoutine
	LD	(UOUT2+1),HL		; bei Kanal 2 eintragen
	LD	HL,V24UMT		; SIO initialisieren
	JP	INIMEX
;
; V.24-Modul initialisieren:
;	-Tabellen in IRM kopieren
;	-M003 suchen und einschalten
;	-M003-Steckplatz merken
; PA:	CY=1	kein M003 vorhanden
;
V24INI:	LD	HL,V24TAB		; vorbereitete V24-Tabelle
	LD	DE,V24PL		; Steckplatz (A800H)
	LD	BC,V24UMT-V24TAB	; Laenge
	LDIR				; in IRM kopieren
	CALL	V24SU			; V.24-Modul ab Steckplatz 7 suchen
	RET	C			; keins gefunden
	LD	A,B			; Steckplatz..
	LD	(V24PL),A		; ..eintragen
	LD	A,1
	LD	(ARGN),A
	CALL	V24OC			; SIO-A: Druckerinitialisierung
	CALL	UMINIT			; SIO-B: Empfangs-Interrupt
	RET
;
; V.24-Tabellen, zum Kopieren in den IRM ab A800H:
;
V24TAB:	DB	0	; Steckplatz M003 (Aktualisierung durch Suche)

; Kanal 1 - Druckerausgabe (9600 Baud)
	DB	47H	; CTC: DI, Zaehler, neg. Flanke, ZK folgt, RESET
	DB	5BH	; Zeitkonstante = 91
	DB	4,4	; WR4: Vorteiler=1, 1 Stop-Bit, ohne Paritaet
	DB	3,20H	; WR3: Empfaenger aus, CTS+DCD-Steuerung
	DB	5,6AH	; WR5: Sender ein, 8Bit, /DTR=1, RTS=1
 
; Kanal 2 - Duplex (1200 Baud)
	DB	47H	; CTC: DI, Zaehler, neg. Flanke, ZK folgt, RESET
	DB	2EH	; Zeitkonstante = 46
	DB	18H	; Port-RESET
	DB	4,44H	; WR4: Vorteiler=16, 1 Stop-Bit, ohne Paritaet
	DB	3,0E1H	; WR3: Empfaenger ein, 8Bit, CTS+DCD-Steuerung
	DB	5,6AH	; WR5: Sender ein, 8Bit, /DTR=1, RTS=1
 
; Initialisierung SIO-B fuer Empfangs-Interrupt (1200 Baud):

V24UMT:	DB	2	; 2 Ports

	DB	0DH	; CTC1
	 DB	2	; Anzahl folgender Bytes
	  DB	47H	; CTC: DI, Zaehler, neg. Flanke, ZK folgt, RESET
	  DB	2EH	; Zeitkonstante = 46
 
DTRON:	DB	0BH	; SIO-B Steuerwort
	 DB	11	; Anzahl folgender Bytes
	  DB	18H	; Port-RESET
	  DB	2,0E2H	; WR2: INT-Vektor
	  DB	14H,44H	; WR4: Vorteiler=16, 1 Stop-Bit, ohne Paritaet
	  DB	3,0E1H	; WR3: Empfaenger ein, 8Bit, CTS/DCD-Steuerung
	  DB	5,0EAH	; WR5: Sender ein, 8Bit, /DTR=0 (bereit), RTS=1
	  DB	11H,18H	; WR1: Interrupt bei Zeichenempfang
 
MBOC:	;Byteweise MB-Ausgabe
	LD	E,D	;Steuerbyte
	LD	D,A	;Datenbyte
	PUSH	DE
	PUSH	BC
	BIT	3,E
	JR	Z,NOINIT
	SET	1,(IX+7);merken INIT
	LD	A,'U'+80H
	CP	(HL)
	PUSH	DE
	LD	DE,0A0H	;Vortonl{nge
	JR	NZ,NOHIU;bei 'U'
	LD	DE,500H	;gedehnt
NOHIU:	LD	(VORTN),DE
	LD	DE,CASS
	LD	BC,11
	LDIR		;Name
	EX	DE,HL
	POP	DE
	LD	(HL),D	;1.Byte
	INC	L
MBOC1:	LD	A,L
	LD	(DTPTR),A
MBOCE:	POP	BC
	POP	DE
	RES	3,E
	RET
 
NOINIT:	LD	HL,(DTPTR)
	LD	H,0B7H
	LD	(HL),D	;Byte eintragen
	BIT	6,E
	JR	NZ,MCLOS;Close
	INC	L
	BIT	7,L
	JR	Z,MBOC1
	BIT	1,(IX+7)
	JR	NZ,MBOC2;erst mal Init!
	CALL	MBLO	;Blockout
MBOC6:	LD	HL,CASS
	JR	MBOC1
 
MBOC2:	RES	1,(IX+7);r}cksetzen
	CALL	ISRO	;Init
	CALL	MBLNR	;Blocknr.
	JR	MBOC6
 
MCLOS:	BIT	1,(IX+7);Init?
	JR	Z,MBOC3
	CALL	ISRO	;erst noch Init!
	JR	MBOC4
MBOC3:	CALL	MBLO	;Blockout
MBOC4:	LD	BC,(VORTN)
	CALL	CSRO
MBOC5:	PUSH	DE
	CALL	CSRI
	RES	5,(IX+7);'U'-Merker
	POP	DE	;r}cksetzen
	JR	MBOCE
 
MBIC:	;Byteweise MB-Eingabe
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
MBIC1:	POP	HL
	JP	C,JBLERR;Fehler: '*'
	LD	A,(IX+2);gelesener Blk.
	CP	1
	JP	NZ,JBLERR ;Fehler: '*'
	RES	7,(IX+7);Kopfblk geladen
	INC	(IX+3)	;erw. Block
	LD	DE,CASS
	LD	B,11
	LD	A,(DE)
	CP	'U'+80H	;hohes 'U'?
	JR	NZ,MBIC2
	SET	5,(IX+7);merken zur
 ;Blocknummernausgabeunterdr}ckung
MBIC2:	ADD	29H	;Gesch}tzte
	JR	NC,MBIC3;BASIC-
	LD	A,(DE)	;Programme
	SUB	4	;als un-
	LD	(DE),A	;gesch}tzt
	INC	DE	;anzeigen
	LD	(DE),A	;und Setzen
	INC	DE	;LIST/EDIT/BYE-
	LD	(DE),A	;Schutz
	LD	(35EH),A;<=hier
	SET	6,(IX+7);merken wof}r?
	LD	DE,CASS
MBIC3:	LD	A,(DE)	;Name
	CALL	CRT	;anzeigen
	CP	(HL)	;stimmt Name?
	JR	Z,MBIC4
	SET	7,(IX+7);nein-merken
MBIC4:	INC	HL
	INC	DE
	DJNZ	MBIC3
	CALL	CRLF
	BIT	7,(IX+7)
	JP	NZ,JIOERR ;zu ?IO ERROR
	EX	DE,HL
MBIC5:	LD	D,(HL)	;Daten lesen
	INC	L
	LD	A,L
	LD	(DTPTR),A
	LD	A,D
	POP	BC
	POP	DE
	RES	3,E
	RET
 
MBIU:	LD	HL,(DTPTR)
	LD	H,0B7H
	BIT	7,L
	JR	Z,MBIC5	;aus Puffer
MBIU1:	CALL	MBI
	JR	C,MERR
	LD	A,0FFH
	CP	(IX+2)	;Block FF?
	JR	Z,MBIU3
	LD	A,(IX+3)
	CP	(IX+2)	;erwarteter?
	JR	Z,MBIU3
	LD	A,'*'
	CALL	CRT	;nicht
	CALL	BNROST	;erwartet
	DB	19H,0
MBIU2:	CALL	JBRKT
	JR	MBIU1
 
MBIU3:	INC	(IX+3)	;n{chsten
	DEC	A	;erwarten
	BIT	5,(IX+7)
	JR	NZ,MBIU4
	CALL	BNROST	;Blocknummer
	DB	'>'	;unter Vorbe-
	DB	19H,0	;halt ausgeben
MBIU4:	LD	HL,CASS
	JR	MBIC5
 
MERR:	CALL	OSTR	;Fehler im
	DB	9,9,9,9	;Block
	DB	'?',0
	CALL	BNROST
	DB	CR,LF,0
	JR	MBIU2
 
JBLERR:	CALL	OSTR	;nicht erwarte-
	DB	'*',8,0	;ter 1. Block
	CALL	JBRKT
	CALL	MBI
	PUSH	HL
	JP	MBIC1
 
MBLO:	LD	BC,(VORTN)
	CALL	MBO
MBLNR:	CALL	BNROST
	DB	'>',19H,0
JBRKT:	CALL	BRKT	;bei BRK
	RET	NC	;auch ?IO ERROR
	JP	JIOERR
 
VIEW7F:	DW	7F7FH
	DB	'view'
	DB	3
	LD	A,L
	CP	10H
	JR	NC,VIEW1;L>0Fh
	LD	A,H
	AND	A
	JR	NZ,VIEW1;H>0
	LD	HL,ARG1
	RLD
	LD	H,(HL)
	LD	L,0
VIEW1:	LD	DE,8800H
	LD	BC,2000H
VIEW2:	CALL	LDAME	;Byte holen
	LD	(DE),A	;in IRM poken
	INC	HL
	INC	DE
	DEC	C
	JR	NZ,VIEW2
	DJNZ	VIEW2
VIEW3:	LD	A,0BH	;Zeile hoch
	CALL	CRT
	LD	A,8
	LD	(CURSO),A
	CALL	VIEW4
VIEW4:	LD	A,1
VIEW5:	LD	C,A
	CALL	AHEX	;HEX-Leiste
	LD	A,C
	ADD	22H
	JR	NC,VIEW5
	RET
 
;Druckereinstellungen
 
PRI7F:	DW	7F7FH
	DB	'PRINT',1FH
PRI:	LD	A,(DE)
	AND	A
	RET	Z	;fertig
	CP	' '
	JR	Z,PRINC
	CP	','
	JR	Z,PRK	;Zeichen
	CP	27H
	JR	Z,PRSTR	;String
	CALL	RHEX
	JP	C,ERRM	;Fehler!
	LD	A,(NUMVX)
PRIC:	CALL	OFILT
PRINC:	INC	DE
	JR	PRI
PRK:	INC	DE
	LD	A,(DE)	;Zeichen
	JR	PRIC
PRSTR:	INC	DE
	LD	A,(DE)
	AND	A
	RET	Z	;Ende-0
	CP	27H
	JR	Z,PRINC	;Stringende
	CALL	OFILT
	JR	PRSTR

FREIC	EQU	0D000H-$	; freie Bytes
IF	$ GT 0D000H
	MESSAGE	<ROM-C:>,%-FREIC,< Byte(s) zu lang!>
ELSE
	DS	FREIC,0FFH	; mit FF belegen
ENDIF
