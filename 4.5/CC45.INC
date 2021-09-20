;*****************************************
;**	CAOS 4.5	ROM C		**
;**					**
;**	Adresse:  C000h bis CFFFh	**
;**					**
;**	letzte Aenderung: 06.09.2010	**
;*****************************************

;	ORG	0C000H

;
; IRM-Defaulttabelle:
;
KCTAB1:	DW	NOOP		; Hardcopy-Adresse
WIN0:	DB	0		; WINNR
	DW	0		; WINON
	DW	2028H		; WINLG
	DW	0		; CURSO
	DB	0		; STBT
	DB	39H		; COLOR
	DW	SCRLPG		; WEND
WIN1:	DW	0EE00H		; CCTL0
	DW	0FE00H		; CCTL1
	DW	0EE00H		; CCTL2
	DW	0FE00H		; CCTL3
	DW	1C4H		; SYSP
	DW	SUTB		; SUTAB
	DW	CRTTAB		; CTAB
	OUT	(PIOAD),A	; BJUMP
	JP	0F012H
	DW	ZEI0		; OUTAB
	DW	ZEI4		; INTAB
	JP	NOOP		; UOUT1
	JP	NOOP		; UIN1
	JP	NOOP		; UOUT2
	JP	NOOP		; UIN2
	DW	0DC82H		; IOERR (BASIC)
	DW	0B200H		; VRAM

KCTAB2:	DB	47H,0CH		; CTC2 (blinken)
	DW	L3TB		; L3TAB
	DB	13		; L3SIZ (0-C)
	DB	5		; COUNT
	DB	0		; HCPZ
	DW	0A801H		; INTV1
	DB	8		; INTV1L
	DW	0A809H		; INTV2
	DB	9		; INTV2L
	DB	11101110b	; HCPZ2 (8 Bit, Senden ein)
	JP	NOOP		; OFILT
	DB	'%'		; PROMPT
	DB	0FFH		; LINTYP
	DW	0EFD8H		; CUMUST
	DW	JOYIRM		; JOYTAB

; Modulschaltzustaende (interne Module):

KCTAB3:	DB	3	; RAM0
	DB	1	; IRM
	DB	0	; USER-ROMC
	DB	3	; RAM8
	DB	3	; RAM4
	DB	0	; CAOSC
KCTAB4:
;
; Grundinitialisierung ohne Speicher loeschen, ausser:
; VRAM0/1, Kassettenpuffer, Monitor-RAM, Modulsteuerwortspeicher
; Joystick wird hier neu initialisiert (bei RESET nicht)
;
	DW	7F7Fh
	DB	'NEW ',1	; Menuewort versteckt, IRM ein
	DI
	LD	SP,MODST+100H	; Loeschbereich AD00h..B8FFh
	LD	BC,600h		; Anzahl = 6 * 256 Wort = 0C00h
	JR	PWRC0
;
; Power-ON Initialisierung:
;
PWRONC:	DI
	LD	SP,0C000H	; Loeschbereich 0..BFFFh
	LD	BC,6000H	; Anzahl = 60h * 256 Wort = C000h
PWRC0:	LD	H,C
	LD	L,C		; HL=0
PWRC1:	PUSH	HL		; schnelles Speicher loeschen
	DEC	C
	JR	NZ,PWRC1
	DJNZ	PWRC1
	LD	SP,01C4H	; System-Stackpointer
	LD	C,80h		; alle Module abschalten (B war 0)
PWRC2:	OUTC0			; OUT (C),0
	DJNZ	PWRC2
	CALL	SYSI		; Systeminitialisierung (Rueckkehr mit EI)
	LD	HL,JOYTBD	; vorbereitete Joystick-Tabelle
	LD	DE,JOYIRM	; JOYTAB-Standardadresse im IRM
	LD	BC,12
	LDIR			; Tabelle in den IRM kopieren
	XOR	A		; Fenster Nr. 0
PWRC3:	LD	(WINNR),A	; Nr. eintragen
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
	LD	E,LOW(CTCMD)	; (D war 0B7h)
	LD	C,KCTAB3-KCTAB2	; (B war 0)
	LDIR			; 2. Teil in IRM kopieren
	LD	DE,MODST	; Modulsteuerwortspeicher
	LD	A,(MODST+4)	; RAM4 aktueller Schaltzustand
	AND	4		; RAM4-Ebene herausloesen
	LD	C,KCTAB4-KCTAB3	; (B war 0)
	LDIR			; 3. Teil in IRM kopieren
	LD	HL,MODST+4
	OR	(HL)		; bisherige Ebene wieder einbauen
	LD	(HL),A
	LD	A,1		; Standardwert fuer IXH- und I-Register
SIXC:	LD	HL,ISRTAB
	DI
	LD	(MIXIT),A
	LD	E,0E0H
	LD	D,A
	LD	BC,10H		; Interrupttabelle
	LDIR			; kopieren nach xxE0h bis xxF0h
	LD	H,(IX+1)	; alte Merker
	LD	L,(IX+4)	; mitnehmen
	PUSH	DE
	POP	IX
	LD	(IX+8),80H	; Defaultbelegung (Kassetten kein CAPS-Lock)
	LD	(IX+1),H	; Merker neu eintragen
	LD	(IX+4),L
	LD	(IX+14),LOW(KTAB)
	LD	(IX+15),HIGH(KTAB)
	LD	(IX+9),7FH	; Standard-Prologbyte
	IM	2
	LD	A,(MIXIT)
	LD	I,A
	LD	HL,IOTAB2	; PIO A,B und CTC
	CALL	INIMEX		; Ports neu initialisieren
	LD	HL,CENINI
	CALL	INIMEX		; M021 initialisieren
	CALL	KPUFF		; Kassettenpuffer definieren
	EI
	RET
;
; Portinitialisierungstabellen:
;
IOTAB1:	DB	7		; 7 Ports fuer Erstinitialisierung

	 DB	PIOAS,1		; PIO-A Steuerwort
	  DB	0E4h		; Interruptvektor

	 DB	PIOAD,2		; PIO-A Daten
	  DB	00011111b	; D005-Reset
	  DB	00001111b	; IRM, RAM0, CAOS-E

	 DB	PIOAS,2		; PIO-A Steuerwort
	  DB	00001111b	; Byte-Ausgabemodus
	  DB	00000011b	; DI

	 DB	PIOBS,3		; PIO-B Steuerwort
	  DB	0E6h		; Interruptvektor
	  DB	00001111b	; Byte-Ausgabemodus
	  DB	10000011b	; EI

	 DB	PIOBD,1		; PIO-B Daten
	  DB	0FFh		; Daten

	 DB	84h,1		; Ausgabeport 84h
	  DB	00101000b	; RAM8 0, LoRes

	 DB	86h,1		; Ausgabeport 86h
	  DB	11100011b	; RAM4 ein, USER-C 3, CAOS-C ein

IOTAB2:	DB	4		; 4 Ports initialisieren

	 DB	PIOAS,1		; PIO-A Steuerwort
	  DB	0E4h		; Interruptvektor

	 DB	PIOBS,1		; PIO-B Steuerwort
	  DB	0E6h		; Interruptvektor

	 DB	CTC0,1		; CTC Kanal 0
	  DB	0E8h		; Interruptvektor

	 DB	CTC2,2		; CTC Kanal 2: blinken
	  DB	01000111b	; DI, Zaehler, RESET
	  DB	12		; Zeitkonstante

; Schreibe 'RAMn'

WRAM:	CALL	OSTR
	DB	'RAM',0
	LD	A,D
	CALL	OCHR
	AND	A		; CY=0
	RET

; Systemcheck:

SYSTC:	PUSH	DE
	CALL	OSTR
	DB	CR,'CAOSE',0
	IN	A,(PIOAD)
	AND	1
	CALL	WONOF		; Bit 0
	CALL	OSTR
	DB	'CAOSC',0
	LD	A,(MODST+5)
	AND	1
	CALL	WONOF
	CALL	OSTR
	DB	'ROMC ',0
	LD	A,(IX+4)
	RLCA			; Bits 5,6
	RLCA
	RLCA			; nach Bits 0,1
	AND	3
	XOR	'3'
	CALL	OCHR		; Ebene ausgeben
	IN	A,(PIOAD)
	LD	E,A
	AND	10000000b	; CY=0, Z=NOT(Bit 7)
	CALL	WONOF
	LD	D,'0'
	CALL	WRAM		; RAM0
	LD	A,E
	AND	00001111b
	CP	00001000b	; CY=NOT(Bit 3)
	BIT	1,A		; Schaltzustand
	CALL	WONOF
	LD	D,'4'
	CALL	WRAM		; RAM4
	CALL	SPACE
	LD	A,(MODST+4)
	RRCA
	RRCA			; Bit 2
	AND	1
	ADD	A,'0'
	CALL	OCHR		; Ebene 0/1
	LD	A,(IX+4)
	BIT	0,A		; Schaltzustand
	RRCA			; Schreibschutz (Bit 1)
	RRCA			; ins Carry-Flag rotieren
	CCF			; und invertieren
	CALL	WONOF
	LD	D,'8'
	CALL	WRAM		; RAM8
	LD	A,' '
	CALL	OCHR
	LD	A,(IX+1)
	RRCA			; Bits 4,5,6,7
	RRCA
	RRCA
	RRCA			; nach 0,1,2,3
;	AND	0FH		; redundant: bereits in AHEX0
	SUB	2
	CALL	AHEX0		; Ebene ausgeben
	IN	A,(PIOBD)
	BIT	5,A		; Schaltzustand
	RLCA			; Schreibschutz (Bit 6)
	RLCA			; ins Carry-Flag rotieren
	CCF			; und invertieren
	CALL	WONOF
	CALL	OSTR
	DB	'BILD ',0
	LD	A,(IX+1)
	AND	00000100b	; Zugriffs-Bit
	RRCA		
	RRCA
	ADD	A,'0'
	CALL	OCHR		; Bildnummer
	LD	A,(MODST+1)
	AND	1		; IRM on?
	CALL	WONOF
	JR	SPCRLF		; POP DE; JP CRLF

; Modulkontrollanzeige:

MODULC:	PUSH	DE
	XOR	A
	OUT	(91h),A		; M008 PIO-B Daten
	IN	A,(91h)
	AND	A
	JR	NZ,MODC0
	CALL	OSTR
	DB	'-- -- -- JOY/CEN'
	DB	0Dh,0Ah,0
MODC0:	LD	L,7		; ab Platz 7
MODC1:	PUSH	HL
	LD	A,1		; lesen
	CALL	MODUC
	LD	A,H
	INC	A		; Kennbyte=FFh?
	JR	Z,MODC2		; kein Modul
	LD	A,L
	CALL	AHSPC		; Steckplatz
	LD	A,H
	CALL	AHSPC		; Kennbyte
	LD	A,D
	CALL	AHSPC		; Steuerbyte
	LD	C,H
	ld	a,h
	cp	0fbh		; Software-Modul?
	jr	z,slot		; Inhalt analysieren!
	ld	hl,muser
	cp	0c0h
	jr	c,modcd		; kleiner als C0
	cp	0d8h
	jr	c,modca		; USER-Modul von C0-D7
modcd:	LD	B,29		; Anzahl definierter Module in MTAB
	LD	HL,MTAB
MODCC:	LD	A,(HL)
	INC	HL
	CP	C
	JR	Z,MODCA		; Text gefunden
MODCB:	LD	A,(HL)
	INC	HL
	AND	A		; Stringende?
	JR	NZ,MODCB
	DJNZ	MODCC
MODCA:	CALL	ZKOUT		; Modulname
slot7:	CALL	CRLF
MODC2:	POP	HL
	INC	L		; Steckplatz+1
	JR	NZ,MODC1	; bis FFh
SPCRLF:	POP	DE
	JR	SJCRLF

; Schreibe ON, ON* oder OFF

WONOF:	JR	Z,WOF
	PUSH	AF
	CALL	OSTR
	DB	' ON',0
	POP	AF
	LD	A,' '
	JR	NC,WRR
	LD	A,'*'		; Schreibschutz
WRR:	CALL	OCHR
SJCRLF:	JP	CRLF
	;
WOF:	CALL	OSTR
	DB	' OFF',CR,LF,0
	RET

; Suche nach Inhalt der Softwaremodule M012, M026, M027 usw.
; Es wird das erste Menwort gesucht, und dieses als Name angezeigt!
; PE:	L=Steckplatz	H=Kennbyte = 0FBH

slot:	ld	a,l		; Steckplatz, der zu untersuchen ist
	ld	bc,0880h	; alle Module von 8 bis L abschalten
slot1:	OUTC0			; OUT (C),0
	inc	b		; naechster Steckplatz
	cp	b
	jr	nz,slot1
	ld	a,41h		; Modul auf 4000h einschalten
	out	(c),a
	RESIXA	0,4		; RES 0,(IX+4),A
	OUT	(86H),A		; RAM4 aus!
	ld	de,MTAB2	; Vergleichskette 0-String
	ld	hl,4000h	; Suchbereich Beginn
	ld	bc,2000h	; 8K durchsuchen
	call	zs0		; erstes Menuewort suchen
	jr	c,slot2		; Menuewort gefunden
	ld	hl,mtab1	; ansonsten als "Software" anzeigen
	jr	slot4
	;
slot2:	dec	hl
	ld	a,(hl)
	cp	7fh		; zurueck bis zu Prologbyte
	jr	nz,slot2
slot3:	inc	hl
slot4:	ld	a,(hl)
	cp	20h		; Epilog?
	jr	c,slot5		; ja
	call	ochr		; Menuewort anzeigen
	jr	slot3
	;
slot5:	ld	hl,modst+4	; beginnend ab RAM4
slot6:	ld	d,(hl)		; gespeicherter Schaltzustand
	push	hl
	ld	a,2		; schalten
	call	moduc		; Schaltzustaende nach Tabelle regenerieren
	pop	hl
	inc	l		; naechster Steckplatz
	jr	nz,slot6
	jr	slot7

; Module lesen und schalten (Menuewort):

SWIC:	LD	D,E		; Steuerbyte
	DEC	L
	JR	NZ,SWI2		; nicht IRM
	CP	1
	JR	Z,SWI2		; nur lesen
	INC	L
	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
	LD	(HL),D		; IRM eintragen!
	LD	H,0FFH
	JR	SWI3
SWI2:	INC	L
	CALL	MODUC		; lesen/schalten
SWI3:	LD	A,L
	CALL	AHSPC		; Platz anzeigen
	LD	A,H
	CALL	AHSPC		; Kennbyte
	LD	A,D
	CALL	AHEX		; Steuerbyte
	JR	SJCRLF

; V.24-Modul suchen und einschalten:
;
; PE:	B	Steckplatz ab dem gesucht wird
; PA:	B	Steckplatz des Moduls
;	CY = 1	kein M003 vorhanden
; VR:	AF, BC, D, HL
;
V24SU:	LD	C,80h
V24S1:	IN	A,(C)
	CP	0EEH		; M003?
	JR	Z,V24S2		; ja
	INC	B
	JR	NZ,V24S1
	SCF			; nicht gefunden
	RET
V24S2:	LD	L,B
	LD	A,2
	LD	D,1

; Module schalten:

MODUC:	LD	H,HIGH(MODST)	; Modulsteuerwortspeicher
	LD	C,80H		; I/O-Adresse
	LD	B,L
	CP	2		; Parameter?
	JR	NC,MODU1
	LD	D,(HL)		; nur lesen
RSTRB:	IN	H,(C)
	RET

MODU1:	LD	A,(HL)		; alter Zustand
	LD	(HL),D		; eintragen
	LD	H,A		; fuer 2. RAM4
	LD	A,L
	CP	6		; interne Module?
	JR	C,MODU2
	OUT	(C),D		; Senden Steuerbyte
	JR	RSTRB

MODU2:	LD	B,L
	INC	B
	IN	A,(PIOAD)
	DJNZ	MODU3		; 0: RAM0?
	AND	11110101b
	LD	H,A
	LD	A,D
	RRCA			; Bits 0,1 auf 7,0 rotieren
	AND	10000001b	; CY = 0
	RLA			; 0 einfuegen, CY = Bit 7
	RLA			; Bit 0 = CY
	RLCA			; auf Bits 1,3 rotieren
	OR	H
	JR	OUT88

MODU3:	DJNZ	MODU4		; 1: IRM?
	AND	11111011b
	BIT	0,D
	JR	Z,OUT88
	OR	00000100b
	JR	OUT88

MODU4:	DJNZ	MODU5		; 2: ROMC?
	LD	A,D
	CPL			; negieren
	RLCA			; Bits 4..5 auf 5..6 rotieren
	AND	01100000b	; ausfiltern
	LD	H,A
	DI
	LD	A,(IX+4)
	AND	10011111b
	OR	H		; einbauen
	LD	(IX+4),A
	OUT	(86H),A
	EI
	LD	A,D
	RRCA			; CY = Bit 0
	IN	A,(PIOAD)
	RLA			; als Bit 0 einschieben
	RRCA			; und auf Bit 7 rotieren
OUT88:	OUT	(PIOAD),A
	JR	STBFF

MODU5:	DJNZ	MODU6		; 3: RAM8?
	LD	A,00000011b
	AND	D
	RRCA			; Bits 0..1 auf 5..6 rotieren
	RRCA
	RRCA
	LD	H,A
	DI
	IN	A,(PIOBD)
	AND	10011111b
	OR	H
	OUT	(PIOBD),A
	EI
	LD	A,D
	RLCA			; Bits 2..5 auf 4..7 rotieren
	RLCA
	ADD	A,20h		; Segment-Verschiebung
	AND	0F0h
	LD	H,A
	DI
	LD	A,(IX+1)
	AND	0Fh
	OR	H		; RAM8-Segment
	OUT	(84h),A
	LD	(IX+1),A
STBFFI:	EI
STBFF:	LD	H,0FFh
	RET

MODU6:	DJNZ	STBFF		; 5=CAOSC
	LD	A,D		; 4=RAM4
	XOR	H
	AND	4		; Aenderung?
	DI
	JR	Z,RAM41
	LD	A,(IX+4)
	OR	3
	OUT	(86H),A		; RAM4 on
	PUSH	DE
	PUSH	HL
	IN	A,(PIOAD)
	PUSH	AF
	RES	0,A
	OUT	(PIOAD),A	; CAOSE off
	LD	A,(IX+1)
	PUSH	AF
	AND	11111001b
	LD	L,3		; 3 IRM-Ebenen
	LD	BC,1000H	; 800H frei!
	LD	DE,4000H
R4IS:	ADD	A,2
	OUT	(84H),A		; IRM-Ebene
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
	LD	B,18H		; jetzt 1800H!
	DEC	L
	JR	NZ,R4IS
	POP	AF
	OUT	(84H),A		; IRM wie zuvor
	POP	AF
	OUT	(PIOAD),A	; CAOSE on
	POP	HL
	POP	DE
RAM41:	LD	A,(IX+4)
	XOR	D
	AND	11111100b	; Bits 0..1: a = (0 XOR d) = d
	XOR	D		; Bits 2..7: a = (a XOR d) XOR d
	LD	(IX+4),A
	OUT	(86h),A
	JR	STBFFI

; F-Taste belegen:

KEYC:	LD	HL,0B900H
	AND	A
	JR	NZ,KEY0
	LD	B,9CH
KEYCL:	LD	(HL),A		; loeschen
	INC	L
	DJNZ	KEYCL
	RET
KEY0:	CP	16
	RET	NC		; nur F1 bis FF
	LD	B,A
	OR	0F0H
	LD	C,A
KEY1:	LD	A,(HL)		; B Dummys auszaehlen
	INC	L
	RET	Z
	AND	A
	JR	NZ,KEY1
	DJNZ	KEY1
	LD	A,L
	CP	9CH
	RET	NC
	LD	DE,(CURSO)
	CALL	KEYDI		; praesentieren
KEY2:	CALL	INTB
	CP	13H
	JR	NZ,KEY3
	INC	B		; Umschaltung
	JR	KEY2
KEY3:	BIT	0,B
	JR	Z,KEY4		; interpretieren
	CP	3
	JR	Z,KEY12		; BRK=Abbruch
	CP	CR
	JR	Z,KEY5		; Enter=belegen
	CALL	OCHR
	JR	KEY2
KEY4:	CALL	CSTBT
	JR	KEY2
	;
KEY5:	PUSH	HL		; Pos. in Puffer
	CALL	DABR
	EX	DE,HL
	CALL	RHEX		; Fn lesen
	POP	HL
	JR	C,KERR		; Formatfehler
	LD	A,(NUMVX)
	CP	C		; geaendert!
	JR	Z,KEY6
KERR:	CALL	CRLF
	JP	ERRM
KEY6:	INC	DE
	INC	DE
	LD	B,9BH
	CPL
	AND	A
	JR	Z,KEY8		; KEY F
	INC	A
	LD	B,A
	LD	C,L
KEY7:	LD	A,(HL)
	INC	L
	JR	Z,KERR		; Puffer voll!
	AND	A
	JR	NZ,KEY7
	DJNZ	KEY7		; Ende suchen
	DEC	L
	LD	A,L
	CP	9CH
	JR	NC,KERR		; Puffer voll!
	LD	B,H
	PUSH	BC
	SUB	C
	LD	C,A
	LD	B,0
	PUSH	DE
	LD	DE,0B99BH
	LDDR			; Rest hinter
	LD	B,E
	POP	DE
	POP	HL
KEY8:	LD	C,A
KEY9:	DEC	L
	LD	A,(HL)
	AND	A
	JR	NZ,KEY9		; zurueck bis 0
KEY10:	INC	L
	LD	A,(DE)
	INC	DE
	LD	(HL),A		; belegen
	OR	A
	JR	Z,KEY11
	LD	A,B
	SUB	L
	JR	NZ,KEY10
	LD	(HL),0
	JR	KERR
KEY11:	LD	A,C
	AND	A
	JR	Z,KEY12		; KEY F
	INC	B
	INC	L
	LD	E,L
	LD	D,H
	LD	L,B
	LD	B,0
	LDIR			; Rest nach vorn
KEY12:	JP	CRLF

; Anzeige belegter F-Tasten:

KEYLIC:	LD	HL,0B901H
	LD	BC,0F1H
KEYLC1:	LD	A,(HL)
	AND	A
	JR	Z,KEYLC2
	CALL	KEYDI		; Belegung
	CALL	CRLF
	INC	B		; Zaehler
KEYLC2:	INC	L
	LD	A,L
	CP	9CH
	JR	NC,KEYLC3
	INC	C
	JR	NZ,KEYLC1	; F1 bis FF
KEYLC3:	INC	B
	DEC	B
	RET	NZ
	CALL	OSTR
	DB	'F-Tasten leer'
	DB	CR,LF,0
	RET

KEYDI:	LD	A,2
	CALL	OCHR		; Zeile loeschen
	LD	A,C
	CALL	AHOSTR		; Fn
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

; COLOR-Menuewort:

COLRC:	AND	A		; Anzahl Parameter
	JR	NZ,SETCO	; 1 oder 2 -> Farben einstellen
	LD	A,(COLOR)	; 0 = aktuelle Farben anzeigen
	LD	L,A
	RRCA
	RRCA
	RRCA
	AND	1FH
	CALL	AHSPC		; Anzeige Vordergrundfarbe
	LD	A,L
	AND	7
	CALL	AHEX		; Anzeige Hintergrundfarbe
	CALL	CRLF
	JR	COLRC2

; COLOR-Unterprogramm = Farbe setzen (aus L und E)

SETCO:	LD	A,1FH		; 1 oder 2 
	AND	L
	RLA
	RLA
	RLA
	LD	L,A
	LD	A,(ARGN)
	CP	2
	LD	A,E
	JR	NC,COLRC1
	LD	A,(COLOR)	; Hintergrund
COLRC1:	AND	7
	OR	L
	LD	(COLOR),A
COLRC2:	JP	COFF

; WINDOW-Menuewort

WINDC:	AND	A		; Kein Argument?
	JR	Z,WINC0		; Standardfenster initialisieren
	DEC	A		; 1 Arg?
	LD	A,L		; nr
	JR	Z,WINAKC	; Aufruf No.L
	LD	H,L		; za
	LD	L,C		; sa
	LD	D,E		; zn
	LD	A,(ARG4)
	LD	E,A		; sn
	LD	A,(ARGN)
	CP	4
	JR	C,WINCJE	; 2..3 Arg
	LD	A,0
	JR	Z,WINC1		; 4 Arg
	LD	A,(ARG5)	; nr
WINC1:	CALL	WININC
	RET	NC
WINCJE:	JP	ERRM

WINC0:	LD	DE,WINNR	; von Fensternummer
	LD	HL,WIN0		; bis WEND-Adresse
	LD	BC,WIN1-WIN0	; mit Standardwerten aus ROM
	LDIR			; Fenster 0 neu initialisieren
	RET

; definiertes Fenster aufrufen:

WINAKC:	CALL	WCOPY
	LD	(WINNR),A
	CALL	PART		; Fenster A
	RET	C		; holen
	EX	DE,HL
	LD	DE,WINON
	JR	WCOP1

; Testet A und berechnet Position in Fenstervektorspeicher =DE

PART:	ADD	A,0F6H
	RET	C		; zu gross
	ADD	A,A
	LD	E,A
	ADD	A,A
	ADD	A,A
	ADD	A,E
	LD	E,A
	LD	D,0B9H
	AND	A		; CY=0
	RET

; Fenster (WINNR) retten

WCOPY:	PUSH	AF
	LD	A,(WINNR)
	CALL	PART
	JR	C,WINIE		; Fehler
	POP	AF
	LD	HL,WINON
WCOP1:	LD	BC,10
	LDIR
	RET

; Initialisierung eines Fensters

WININC:	PUSH	AF
	CP	10		; No.
	JR	NC,WINIE
	LD	A,D		; Zeilen
	AND	A
	JR	Z,WINIE
	ADD	A,H
	JR	C,WINIE
	CP	21H
	JR	NC,WINIE
	LD	A,E		; Spalten
	AND	A
	JR	Z,WINIE
	ADD	A,L
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
	LD	(CURSO),BC	; BC=0
	LD	(WINNR),A
	AND	A
	RET
	;
WINIE:	POP	AF		; Fehlerende
	SCF
	RET

; Linie zeichnen von	(ARG1)/(ARG2) nach (ARG3)/(ARG4)

LINEC:	 LD	DE,(ARG2)	; YANF
	 LD	HL,(ARG4)	; YEND
	 EXX
	LD	DE,(ARG1)	; XANF
	PUSH	DE
	LD	HL,(ARG3)	; XEND
	LD	A,8		; X=Master
	OR	A
	SBC	HL,DE		; HL=XSTEPS
	JR	NC,LINC1	; vorwaerts
	ADD	HL,DE
	EX	DE,HL
	OR	1		; rueckwaerts
	SBC	HL,DE
LINC1:	 EXX
	 PUSH	DE		; YANF
	 SBC	HL,DE		; HL'=YSTEPS
	 JR	NC,LINC2	; steigend
	 ADD	HL,DE
	 EX	DE,HL
	 OR	2		; fallend
	 SBC	HL,DE
LINC2:	 PUSH	HL		; YSTEPS
	 EXX			; HL=XSTEPS
	POP	BC		; BC=YSTEPS
	PUSH	HL
	SBC	HL,BC		; flach/steil?
	POP	HL
	PUSH	BC
	JR	NC,LINC3	; flach
	EX	(SP),HL
	RES	3,A		; X=Slave
	SET	2,A		; Y=Master
LINC3:	PUSH	HL		; HL=MAX(STEPS)
	LD	B,H
	LD	C,L		; Pixelzaehler
	 EXX
	 POP	BC		; BC'=MAX(...)
	 POP	DE		; DE'=MIN(...)
	 LD	H,B
	 LD	L,C
	 SRL	H		; Slave mit 1/2
	 RR 	L		; HL=1/2*MAX(...)
	 EXX
	POP	DE		; DE=YANF
	POP	HL		; HL=XANF

; Hauptschleife
; AF Bitregister
; HL lfd. X-Koordinate	HL' Slave
; DE lfd. Y-Koordinate	DE' -Master
; BC Punktezaehler	BC' +Master

LINCS:	PUSH	AF
	EX	AF,AF'		; Bitreg merken
	POP	AF
	PUSH	HL
	LD	HL,LINTYP
	RLC	(HL)
	POP	HL
	CALL	C,SHLDE		; Punkt setzen
	 EXX
	 AND	A
	 SBC	HL,DE		; -Master subtrahieren
	 JR	NC,LINC4	; kein Uebertrag - kein Sprung!
	 ADD	HL,BC		; +Master addieren (CY bleibt =1)
LINC4:	 EXX
	JR	NC,LINC5
	OR	0CH		; X und Y!
LINC5:	BIT	3,A		; X?
	JR	Z,LINC6
	INC	HL
	BIT	0,A
	JR	Z,LINC6
	DEC	HL
	DEC	HL
LINC6:	BIT	2,A		; Y?
	JR	Z,LINC7
	INC	DE
	BIT	1,A
	JR	Z,LINC7
	DEC	DE
	DEC	DE
LINC7:	LD	A,B
	OR	C		; letzter Punkt?
	RET	Z
	DEC	BC
	EX	AF,AF'		; Bitreg
	JR	LINCS

; Punkt setzen:

PUSEC:	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	CALHV		; berechnen
	CALL	NC,PUSET	; & setzen
	JP	POP3		; POP	BC,DE,HL   RET

; Kreis zeichnen:

CIRCLC:	LD	A,(ARG3)
	LD	C,A
	LD	L,C
	XOR	A
	LD	B,8
	RR 	L
CIR1:	JR	NC,CIR2
	ADD	A,C
CIR2:	RRA
	RR 	L
	DJNZ	CIR1
	LD	H,A		; Achtelkreis berechnen
	LD	D,H
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
	CALL	CIRUP		; und jeweils 8
	CALL	CIRUP		; Punkte setzen
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

; setzt 4 Punkte um Mittelpunkt

CIRUP:	LD	A,B
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
SHLDE:	;setzt Punkt auf Pos. HL/DE (wenn moeglich)
	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	CALL	CALXY		; berechnen
	CALL	NC,PUSET	; & setzen
	JP	POP4		; POP	AF,BC,DE,HL   RET
	;
PUSET:	BIT	3,(IX+1)
	JR	Z,HIRES		; HiRes
	BIT	1,D
	JR	NZ,PUERA
	BIT	0,D
	JR	NZ,PUXOR
	OR	(HL)		; setzen
	JR	PUCOL
PUXOR:	XOR	(HL)		; alternierend
	RES	0,D
PUCOL:	LD	(HL),A
	LD	A,(IX+1)
	LD	E,A
	XOR	2		; Farbebene
	DI
	OUT	(84H),A
	LD	A,(HL)
	AND	7
	OR	D
	LD	(HL),A		; neue vFarbe
	LD	A,E
	OUT	(84H),A
	EI
	RET
PUERA:	CPL
	AND	(HL)		; loeschen
	LD	(HL),A
	RET
	;
HIRES:	LD	B,A
	OR	(HL)		; Bit setzen in
	BIT	3,D		; Pixelebene?
	JR	NZ,HIR1
	XOR	B		; wieder loeschen
HIR1:	LD	(HL),A
	LD	A,(IX+1)
	LD	C,A
	XOR	2
	DI
	OUT	(84H),A
	LD	A,B
	OR	(HL)		; Bit setzen in
	BIT	4,D		; Farbebene?
	JR	NZ,HIR2
	XOR	B		; wieder loeschen
HIR2:	LD	(HL),A
	LD	A,C
	OUT	(84H),A
	EI
	RET

; Punkt loeschen, PA: A=Farbbyte

PUDEC:	PUSH	HL
	PUSH	DE
	PUSH	BC
	CALL	CALHV
	JR	C,PUDEE		; ausserhalb
	LD	B,A		; Bit
	LD	C,(HL)
	CPL
	AND	C
	LD	(HL),A		; loeschen
	LD	A,(IX+1)
	LD	E,A
	XOR	2
	DI
	OUT	(84H),A
	LD	D,(HL)		; Farbbyte
	LD	A,E
	OUT	(84H),A
	EI
	LD	A,B		; Z=1: Punkt
	AND	C		; war geloescht
	LD	A,D		; Farbbyte
PUDEE:	JP	POP3		; POP	BC,DE,HL   RET

; Grafikposition berechnen
; HL=X-Koor., DE=Y-Koor.
; PA:	CY=1	ausserhalb
; HL=IRM-Adr, A=Bitmaske, D=Farb

CALHV:	LD	HL,(HOR)
	LD	DE,(VERT)
	LD	D,0
CALXY:	PUSH	BC
	LD	BC,BITTAB
	LD	A,L
	AND	7
	ADD	A,C
	LD	C,A		; Bit
	LD	A,L
	SRL	H
	RRA
	SRL	H
	RRA
	SRL	H
	RRA			; Spalte
	CP	28H
	JR	NC,CALEN
	OR	80H		; IRM
	LD	H,A
	LD	A,0FFH
	ADD	A,D
	JR	C,CALEN
	XOR	E		; CY=0!
	LD	L,A		; Zeile
	LD	A,(FARB)
	LD	D,A
	LD	A,(BC)		; MASKE fuer X
	POP	BC
	RET
CALEN:	POP	BC		; ausserhalb
	SCF
	RET

; Ausgabe einer Displayzeile
; HL=Adresse, C=Anzahl Bytes

DPMEMO:	CALL	HLHX		; Adresse
	PUSH	HL
	LD	B,C
DPME1:	CALL	LDAME		; Byte holen
	INC	HL
	CALL	AHSPC		; anzeigen
	DJNZ	DPME1		; C*
	POP	HL
	LD	B,C
	LD	A,9
	CALL	OCHR		; CUR
DPME2:	CALL	LDAME		; Byte holen
	INC	HL
	CALL	CSTBT		; als ASCII
	DJNZ	DPME2		; C*
	RET

; Anzeige Speicherbereich:
; HL=Adresse, E=Listblocklaenge
; C=Anzahl Bytes pro Zeile
; A=Anzahl Argumente

DISPC:	CP	3
	JR	NC,DISP1
	LD	C,8		; default = 8 Bytes/Zeile
	CP	2
	JR	NC,DISP1
	LD	E,4		; default = 4 Zeilen
DISP1:	PUSH	DE
DISP2:	CALL	DPMEMO		; 1 Zeile
	CALL	CRLF
	CALL	BRKT
	JR	C,DISP4
	DEC	E
	JR	NZ,DISP2	; E*
DISP3:	CALL	INTB		; Warten auf Tastenbetaetigung
DISP4:	POP	DE
	CP	3		; BRK?
	RET	Z
	CP	0FH
	JR	NZ,DISP5
	CALL	OCHR		; Hardcopy
	JR	DISP3
DISP5:	CP	13H		; STOP?
	JR	NZ,DISP1
	JR	MODI1

; Anzeige/Veraendern Speicherbereich
; HL=Adresse, E=Anzahl Bytes pro Zeile
; A=Anzahl Argumente

MODIC:	LD	C,E		; Breite
	CP	2
	JR	NC,MODI1
	LD	C,1		; default
MODI1:	CALL	DPMEMO
	LD	A,5
	LD	(CURSO),A
	CALL	INLIN
	RET	C		; BRK
	PUSH	HL
	CALL	RHEX		; Adresse
	LD	A,(HL)
	POP	HL
	JR	C,MODI9		; Fehler
	AND	A
	JR	Z,MODI1
	LD	HL,(NUMVX)
MODI2:	PUSH	HL
	CALL	RHEX		; Datenbyte
	LD	A,(HL)
	AND	A		; kein Zeichen
	INC	HL		; einlesbar?
	LD	A,(HL)
	POP	HL
	JR	Z,MODI4
MODI3:	CALL	LDMAE		; eintragen
	INC	HL
MODI4:	LD	A,(DE)
	CP	'.'		; Abschluss?
	RET	Z
	CP	','		; 1 Zeichen?
	JR	NZ,MODI5
	INC	DE
	LD	A,(DE)		; vom Video-RAM
	INC	DE
	JR	MODI3		; eintragen

MODI5:	CP	':'		; Rueckschritt?
	JR	NZ,MODI6
	LD	B,0
	SBC	HL,BC		; C Bytes zurueck
	JR	MODI1

MODI6:	CP	'/'		; neue Adresse?
	JR	NZ,MODI0
	INC	DE
	CALL	RHEX
	JR	C,MODI9
	LD	HL,(NUMVX)
	JR	MODI1

MODI0:	CP	27H		; Zeichenkette?
	JR	NZ,MODI8
	INC	DE
MODI7:	LD	A,(DE)		; Zeichen holen
	INC	DE
	AND	A		; Dummy-Ende?
	JR	Z,MODI1
	CP	27H		; Ende Kette?
	JR	Z,MODI4
	CALL	LDMAE		; eintragen
	INC	HL
	JR	MODI7

MODI8:	AND	A		; Ende Zeile?
	JR	Z,MODI1
	CP	' '		; Leerzeichen?
	JR	Z,MODI2
MODI9:	CALL	ERRM		; sonst ERROR
	JR	MODI1

; Initialisierung der Druckerausgabe:
;
; PE:	(ARGN)	0..5 Argumente	(0 = M003 ab Steckplatz 7 suchen)
;	(ARG1)	Steckplatz	(0 = M021)
;	(ARG2)	SIO-Kanal	(0..1 = A, 2 = B)
;	(ARG3)	User-Kanal	(2 = UOUT1, 3 = UOUT2)
;	(ARG4)	ShCLR-Reaktion	(0 = keine, 1 = Protokoll, 2 = Hardcopy)
;	(ARG5)	Druckertyp	(0..12)
; PA:	-
; VR:	AF, BC, DE, HL
;
LSTC:	CALL	LARG		; Arg's laden
	CALL	LSTCA		; Druckerinitialisierung (intern)
	JP	C,ERRM
	LD	A,CR		; CR+LF drucken
	CALL	OFILT
	LD	A,LF
	JP	OFILT

LSTC1:	LD	A,(HCPZ)	; bisheriges Steuerbyte
	AND	0F0h		; Druckertyp uebernehmen
	RRCA			; und auf Bits 1..4 rotieren
	RRCA
	RRCA
	JR	LSTC6

LSTC2:	CALL	USRARG		; Steuerwort aus Argumenten bilden (CY war 0)
	XOR	A
	OUT	(91h),A		; M021 PIO-B Daten
	IN	A,(91h)
	ADD	A,-1		; 0 zurueckgelesen?
	RET	C		; sonst kein M021 vorhanden
	PUSH	BC
	LD	HL,M021PR
	JR	LSTC7

LSTC3:	LD	L,7		; ab Steckplatz 7
LSTC4:	CALL	SIOARG		; Steuerwort aus Argumenten bilden
	PUSH	BC		; Adresse ShCLR-Reaktionsprogramm
	LD	B,L
	CALL	V24SU		; M003 suchen
	LD	D,B		; Steckplatz in D zurueckgeben
	JR	C,IOEPOP	; nicht gefunden
	LD	A,(INTV1L)
	LD	B,A
	LD	HL,(INTV1)
	LD	A,E		; Steuerbyte
	CALL	INISIO
	LD	HL,V24PR
	JR	LSTC7

; Initialisierung der Druckerausgabe (intern):
;
; PE:	A	0..5 Argumente	(0 = M003 ab Steckplatz 7 suchen)
;	L	Steckplatz	(0 = M021)
;	E	SIO-Kanal	(0..1 = A, 2 = B)
;	C	User-Kanal	(2 = UOUT1, 3 = UOUT2)
;	(ARG4)	ShCLR-Reaktion	(0 = keine, 1 = Protokoll, 2 = Hardcopy)
;	(ARG5)	Druckertyp	(0..12)
; PA:	D	Steckplatz M003	(nur gueltig fuer A = 0)
;	CY	Fehlerstatus	(0 = OK, 1 = Fehler)
; VR:	AF, BC, DE, HL
;
LSTCA:	LD	H,A
	LD	A,5
	CP	H		; (ARGN) <= 5?
	RET	C
	JR	NZ,LSTC1
LSTC5:	LD	A,(ARG5)	; neuer Druckertyp
	CP	13		; (ARG5) < 13?
	JR	NC,IOERET
	ADD	A,A		; 0 fuer Bit 3 einschieben
LSTC6:	LD	D,A
	LD	A,H
	AND	A		; Steckplatz angegeben?
	JR	Z,LSTC3
	LD	A,L
	AND	A		; Steckplatz = 0?
	JR	Z,LSTC2		; M021
	PUSH	BC
	LD	B,L
	LD	C,80h
	IN	A,(C)
	POP	BC
	SUB	0EEh		; M003?
	JR	Z,LSTC4
	DEC	A		; M001?
	JR	NZ,IOERET

	CALL	USRARG		; Steuerwort aus Argumenten bilden (CY war 0)
	PUSH	BC		; Adresse ShCLR-Reaktionsprogramm
	LD	A,2
	LD	D,1
	CALL	MODUC		; M001 einschalten
	LD	HL,DIOINI
	CALL	IMEXDI		; PIO konfigurieren
	LD	HL,M001PR
LSTC7:	LD	(OFILT + 1),HL
	POP	HL
	LD	(HCADR),HL	; ShCLR-Reaktionsprogramm setzen
	LD	A,E
	LD	(HCPZ),A	; Steuerbyte eintragen
	LD	HL,PRINT
	LD	BC,NOOP
	AND	A		; CY = 0
;
; Eintragen der Sprungadressen eines I/O-Kanals:
;
; PE:	E = Steuerwort
;	BC = Sprungadresse fuer Eingabe
;	HL = Sprungadresse fuer Ausgabe
; PA:	-
; VR:	F
;
SETUIO:	BIT	1,E		; Ausgabekanal #2 oder #3?
	JR	NZ,SETU2
	LD	(UOUT1 + 1),HL
	LD	(UIN1  + 1),BC
	RET
SETU2:	LD	(UOUT2 + 1),HL
	LD	(UIN2  + 1),BC
	RET

IOEPOP:	POP	HL
IOERET:	SCF
	RET

; Auswertung der Argumente 2..4 fuer I/O-Initialisierung:
;
; PE:	H	Argumentzahl
;	E	SIO-Kanal	(0..1 = A, 2 = B)
;	C	User-Kanal	(2 = UOUT1, 3 = UOUT2)
;	(ARG4)	ShCLR-Reaktion
;	D	Steuerwort
; PA:	E	Steuerwort	(3 neue Bits von rechts einrotiert)
;	BC	Adresse ShCLR-Reaktionsprogramm
;	CY	Fehlerstatus	(0 = OK, 1 = Fehler)
; VR:	AF, BC, DE, SP
;
; Stack-Unwinding: Im Fehlerfall wird die eigentliche Ruecksprungadresse vom
; Stack genommen, und dann ein RET ausgefuehrt.  Die aufrufende Routine darf
; keine Daten auf dem Stack zu liegen haben!
;
SIOARG:	LD	A,1
	CP	H
	JR	NC,USRARG
	DEC	E		; (ARG2): SIO-Kanal
	SUB	E		; 1 <= (ARG2) <= 2?
	JR	C,IOEPOP
	CP	E		; CY = 1 wenn K2
USRARG:	LD	E,D
	RL	E		; Bit 2 einschieben
	LD	A,2
	LD	D,A
	CP	H
	JR	NC,NOARG3
	LD	A,C		; (ARG3): User-I/O-Kanal
	SUB	D
	CP	D		; 2 <= (ARG3) <= 3?
	JR	NC,IOEPOP
	RRCA			; CY = 1 wenn User-Kanal #3
NOARG3:	RL	E		; Bit 1 einschieben
	LD	BC,NOOP
	INC	D		; D = 3
	LD	A,D
	CP	H
	JR	NC,NOARG4
	LD	A,(ARG4)	; Reaktion auf ShCLR
	AND	A		; (ARG4) = 0?
	JR	Z,NOARG4
	SUB	D		; 0 <= (ARG4) <= 2?
	JR	NC,IOEPOP
	LD	BC,HCPGM
	RRCA			; CY = 1 wenn Hardcopy
NOARG4:	RL	E		; Bit 0 einschieben
	RET

; Initialisierung V.24-Duplexbetrieb:
;
; PE:	(ARGN)	0..3 Argumente	(0 = M003 ab Steckplatz 7 suchen)
;	(ARG1)	Steckplatz
;	(ARG2)	SIO-Kanal	(0..1 = A, 2 = B)
;	(ARG3)	User-Kanal	(2 = UOUT1, 3 = UOUT2)
; PA:	CY	Fehlerstatus	(0 = OK, 1 = Fehler)
; VR:	AF, BC, DE, HL
;
V24DC:	CALL	LARG		; Arg's laden
	LD	H,A
	LD	A,3
	CP	H		; (ARGN) <= 3?
	RET	C
	LD	D,0		; Steuerwort zunaechst leer
	CALL	SIOARG		; Steuerwort aus Argumenten 2-4 bilden
	PUSH	HL
	LD	A,(INTV2L)	; Laenge Duplexinitialisierung
	LD	B,A
	LD	HL,(INTV2)	; Duplexinitialisierung
	INC	HL
	JR	V24DC5

V24DC1:	LD	A,10000000b	; Empfangseinstellungen (7/8 Bit)
	JR	V24DC3

V24DC2:	LD	A,(HL)		; Initialisierungsdaten lesen
	AND	00000111b	; Registerauswahl?
	JR	Z,V24DC6
	INC	HL
	CP	3		; WR3?
	JR	Z,V24DC1
	CP	5		; WR5?
	JR	NZ,V24DC5
	LD	A,01101000b	; Sendeeinstellungen (5-8 Bit, ein/aus)
V24DC3:	AND	(HL)		; benoetigte Bits abtrennen
	OR	E		; und in Steuerbyte einbauen
V24DC4:	LD	E,A
V24DC5:	DEC	B
	JR	Z,IOEPOP	; fehlendes Initialisierungsbyte
V24DC6:	INC	HL
	DJNZ	V24DC2

	POP	HL
	LD	A,H
	LD	B,7		; Suche ab Steckplatz 7
	AND	A		; Steckplatz angegeben?
	JR	Z,V24DC7
	LD	B,L		; angegebener Steckplatz
V24DC7:	CALL	V24SU
	RET	C		; Modul nicht gefunden
	LD	HL,V24OT
	LD	BC,V24IN
	CALL	SETUIO		; I/O-Sprungadressen eintragen
V24DC8:	LD	A,(INTV2L)
	LD	B,A
	LD	HL,(INTV2)
	LD	A,E		; Duplex-Steuerbyte
	LD	(HCPZ2),A	; eintragen

; SIO-Initialisierung:
;
; PE:	HL	Initialisierungstabelle
;	B	Laenge der Tabelle (>= 3)
;	A	Steuerbyte
; PA:	CY = 0
; VR:	AF, BC, HL
;
INISIO:	AND	00000100b
	RRCA
	RRCA
	ADD	A,0Ch		; CTC-Port
	LD	C,A
	LD	A,B
	LD	B,2		; 2 Byte fuer CTC-Initialisierung
	SUB	B
	DI
	OTIR			; CTC initialisieren
	LD	B,A
	DEC	C		; SIO-Steuerport
	DEC	C
	OTIR			; SIO initialisieren
	EI
	AND	A		; CY = 0
	RET

; V.24-Modul initialisieren:
;	-Tabellen in IRM kopieren
;	-M003 suchen und einschalten
;	-M003-Steckplatz merken

V24INI:	LD	HL,V24TAB	; vorbereitete V24-Tabelle
	LD	DE,V24PL	; Steckplatz (A800H)
	LD	BC,V24UMT - V24TAB
	LDIR			; in IRM kopieren
	XOR	A
	CALL	LSTCA		; SIO-A: Druckerinitialisierung
	RET	C		; kein M003 gefunden
	LD	A,D
	LD	(V24PL),A	; Steckplatz eintragen
	LD	HL,V24OT	; Duplex-Ausgaberoutine
	LD	(UOUT2 + 1),HL	; fuer User-Kanal #3 eintragen
	LD	HL,V24UMT	; SIO-B: Empfangs-Interrupt

; Initialisierung mehrerer Ports mit Sperren von Interrupts:
;
; PE:	HL	Initialisierungstabelle
;	(HL)	Anzahl der Kanaele
; PA:	-
; VR:	F, D, HL
;
IMEXDI:	LD	D,(HL)		; Anzahl Kanaele
	INC	HL
	JP	INIME		; Ports initialisieren

; allgemeine Druckroutine

PRINTC:	PUSH	AF
	LD	A,(HCPZ)
	BIT	7,A
	JR	Z,PRINT3	; Drucker
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
	JR	NZ,PRINT1
	LD	C,6
	ADD	HL,BC
	LD	A,(HL)
	JR	PRINT1

S6005:	POP	AF
	CP	7EH
	JR	NZ,PRINT4
	LD	A,83H
	JR	PRINT4
PRINT1:	POP	BC
	POP	HL
	PUSH	AF
PRINT3:	POP	AF
PRINT4:	CP	9		; CUR
	JR	Z,PRINT5
	PUSH	AF
	LD	A,(STBT)
	BIT	3,A		; Steuerzeichen
	JR	Z,PRINT7	; ausfuehren
	POP	AF
	CP	7FH		; und 7Fh
	JR	NZ,PRINT6
PRINT5:	LD	A,20H		; nach SPC
PRINT6:	CP	20H		; und Steuerzeichen
	JR	NC,PRINT8
	LD	A,'_'		; nach '_' wandeln
	PUSH	AF
PRINT7:	POP	AF
PRINT8:	JP	OFILT		; in IRM

; ShCLR-Reaktionsprogramm

HCPGMC:	PUSH	HL
	PUSH	DE
	LD	A,(HCPZ)
	BIT	0,A		; Hardcopy?
	JR	NZ,COPIES

; Protokollfunktion

	LD	HL,(OUTAB)
	LD	E,A		; E = (HCPZ)
	LD	A,(HL)
	AND	A		; CRT?
	JR	NZ,PROTO2
	BIT	1,E		; USER1/2?
	LD	HL,ECHO		; CRT+PRINT
	LD	DE,ZEI3		; UOUT2
	JR	NZ,PROTO3
	DEC	DE		; UOUT1
PROTO4:	LD	(UOUT1 + 1),HL
	JR	PROTO5
	;
PROTO2:	LD	HL,PRINT	; Protokoll deaktivieren
	LD	DE,ZEI0		; CRT
	CP	2
	JR	Z,PROTO4
PROTO3:	LD	(UOUT2 + 1),HL
PROTO5:	LD	(OUTAB),DE	; Zeiger
	POP	DE
	POP	HL
	RET

; Hard- oder Screencopy

COPIES:	PUSH	BC
	PUSH	AF
	AND	0F0H
	JP	P,HCOPYC	; Hardcopy

; Screencopy

	LD	HL,(VRAM)
	LD	B,32
SCOPY1:	LD	C,40
SCOPY2:	LD	A,(HL)
	INC	HL
	CP	20H
	JR	NC,SCOPY4
	LD	A,' '		; statt Steuerzeichen
SCOPY4:	CALL	PRINT
	DEC	C		; 40 Spalten
	JR	NZ,SCOPY2
	LD	A,CR
	CALL	PRINT4
	LD	A,LF
	CALL	PRINT4
	DJNZ	SCOPY1		; 32 Zeilen
HCEND:	JP	POP4		; POP	AF,BC,DE,HL   RET

HCOPYC:	;..nach Typ verzweigen
	AND	0F0H
	CP	10H
	JP	Z,K6314		; ESC/P breit
	CP	20H
	JP	Z,K6311		; mit ANSI
	CP	30H
	JP	Z,K6312		; ANSI breit
	CP	50H
	JR	Z,HCMIN		; ESC/P2 (Mini)
	CP	70H
	JR	Z,ESCP2		; ESC/P2 24 Nadel

; K6313, K6304, ESC/P2 mit 8 Nadeln

K6313:	LD	H,0		; Zeile 0
K6313a:	CALL	PRZK		; Grafik init
	LD	L,0		; Spalte 0
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
	JR	NZ,K6313c	; naechster Cursor
	POP	HL
	INC	L
	LD	A,40
	CP	L
	JR	NZ,K6313b
	LD	A,H
	ADD	A,8		; naechste Zeile
	LD	H,A
	JR	NC,K6313a
HCCRLF:	LD	A,CR
	CALL	OFILT
	LD	A,LF
	CALL	OFILT
	JR	HCEND

; ESC/P2 Miniformat

HCMIN:	LD	HL,0		; Zeile=0, Spalte=0
MINIa:	CALL	PRZK
MINIb:	LD	C,24		; 24 Pixel
	CALL	CBYTES
	CALL	S3PRS
	JR	NZ,MINIb
	LD	A,H
	ADD	A,24
	LD	H,A
	LD	L,0		; Spalte=0
	CP	240
	JR	NZ,MINIa
	CALL	PRZK
MINIc:	LD	C,16
	CALL	CBYTES
	LD	B,8
	XOR	A
MINId:	LD	(DE),A		; 8 Nadeln frei
	INC	DE
	DJNZ	MINId
	CALL	S3PRS
	JR	NZ,MINIc
	JR	HCCRLF

; ESC/P2 mit 24 Nadeln

ESCP2:	LD	H,0		; Zeile 0
ESCP2a:	CALL	PRZK
	LD	L,0		; Spalte 0
ESCP2b:	LD	B,8		; 8 Pixel
	PUSH	HL
	CALL	PADR
	LD	DE,CASS
ESCP2c:	LD	A,(HL)
	LD	(DE),A
	INC	DE
	LD	(DE),A		; 3 mal ablegen
	INC	DE
	LD	(DE),A
	INC	DE
	INC	L
	DJNZ	ESCP2c
	LD	C,8
ESCP2d:	LD	B,24		; 3*8 Bytes
	LD	HL,CASS
ESCP2e:	RL	(HL)
	RL	E
	RL	D
	RLA
	INC	HL
	DJNZ	ESCP2e		; Wandlung
	LD	(HL),A
	LD	B,3		; 3 mal drucken
ESCP2f:	LD	A,(HL)
	CALL	OFILT		; 1.
	LD	A,D
	CALL	OFILT		; 2.
	LD	A,E
	CALL	OFILT		; 3. Byte
	DJNZ	ESCP2f
	DEC	C
	JR	NZ,ESCP2d
	POP	HL
	INC	L
	LD	A,40		; Zeilenende?
	CP	L
	JR	NZ,ESCP2b
	LD	A,H
	ADD	A,8
	LD	H,A
	JR	NC,ESCP2a
	JR	JHCRLF

; ESC/P 9-Nadeln (breit)

K6314:	LD	H,0
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
	DJNZ	K6314c		; 4*
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
	ADD	A,4
	LD	H,A
	JR	NC,K6314a
JHCRLF:	JP	HCCRLF

; ANSI-Drucker

K6311:	CALL	PRZK
	LD	B,256/6
	LD	HL,0
K6311a:	LD	C,6
	CALL	CBYTES
	CALL	SPRS		; 6*8-Feld ausg.
	JR	NZ,K6311a
	LD	A,6		; 6 Pixel tiefer
	ADD	A,H
	LD	H,A
	LD	L,0
	CALL	PRZK
	DEC	B
	JR	NZ,K6311a
K6311b:	LD	C,256 MOD 6	; den Rest
	CALL	CBYTES
	XOR	A
	LD	(DE),A		; letzte beiden
	INC	DE		; Zeilen leeren
	LD	(DE),A
	CALL	SPRS
	JR	NZ,K6311b
	JR	JHCRLF

; ANSI breit

K6312:	CALL	PRZK
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
K6312b:	LD	C,256 MOD 6	; den Rest
	CALL	CBYTS2
	XOR	A
	LD	(DE),A
	INC	DE
	LD	(DE),A
	CALL	S2PRS
	JR	NZ,K6312b
	JR	JHCRLF

; C Bytes zum Puffer

CBYTES:	PUSH	HL
	PUSH	BC
	CALL	PADR
	LD	DE,CASS
	LD	B,0
	LDIR
	POP	BC
	POP	HL
	RET

; C Bytes zum Puffer & doppeln

CBYTS2:	PUSH	HL
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

; 8* 6-Nadel-Sprosse ausgeben

SPRS:	PUSH	HL
	PUSH	BC
	LD	L,80H
SPRS1:	LD	B,0
	LD	DE,CASS
SPRS2:	LD	A,(DE)
	AND	L		; CY=0
	JR	Z,SPRS3
	SCF			; CY=1
SPRS3:	RL 	B
	INC	DE
	LD	A,6		; fuer 6 Bit
	CP	E
	JR	NZ,SPRS2	; ob. Nadel war 0
	AND	A		; unt. Nadel = 0
	RL 	B
	LD	A,B
	CALL	OFILT
	RRC	L
	JP	P,SPRS1
	POP	BC
	POP	HL
	INC	L
	LD	A,40
	CP	L
	RET

; 8* 6-Nadel-Sprosse doppelt out

S2PRS:	PUSH	HL
	PUSH	BC
	LD	L,80H
S2PR1:	LD	B,0
	LD	DE,CASS		; Kassetten-Puffer
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
	RLA			; untere beiden
	RLA			; Nadeln tot
	CALL	OFILT
	CALL	OFILT
	RRC	L
	JP	P,S2PR1
	POP	BC
	POP	HL
	INC	L
	LD	A,40
	CP	L
	RET

; 8* 24-Nadel-Sprosse ausgeben

S3PRS:	PUSH	HL
	LD	E,8
S3PR1:	LD	C,3		; 24 Nadeln = 3 Byte
	LD	HL,CASS
S3PR2:	LD	B,8
S3PR3:	RL	(HL)
	RLA
	INC	HL
	DJNZ	S3PR3		; Wandlung
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

; Druckerinitialisierung entsprechend HCPZ

PRZK:	PUSH	HL
	PUSH	BC
	XOR	A
	LD	HL,HCPZ
	RLD
	LD	C,A		; Druckertyp
	RRD
	INC	C
	LD	HL,C6313
PRZK1:	LD	B,(HL)		; Laenge
	DEC	C
	JR	Z,PRZK2
	INC	B
	LD	A,L
	ADD	A,B
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

; Grafiksteuersequenzen fuer die Drucker

C6313:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',5
	DW	320

C6314:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',5
	DW	640

C6311:	DB	14		; Laenge
	DB	ESC,'[01e'
	DB	ESC,'[12`'
	DB	ESC,'K'
	DW	320

C6312:	DB	14		; Laenge
	DB	ESC,'[01e'
	DB	ESC,'[12`'
	DB	ESC,'K'
	DW	640

C6304:	DB	9		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'K'
	DW	320

CMINI:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',39
	DW	320

C24N1:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',0
	DW	320

C24N2:	DB	10		; Laenge
	DB	CR,9
	DB	ESC,'J',24
	DB	ESC,'*',39
	DW	960

; V.24-Tabellen, zum Kopieren in den IRM ab A800H:

V24TAB:	DB	0		; Steckplatz M003 (Aktualisierung durch Suche)

; Kanal 1 - Druckerausgabe (9600 Baud)
	DB	01000111b	; CTC: DI, Zaehler, neg. Flanke, RESET
	DB	91		; Zeitkonstante
	DB	4,00000100b	; WR4: Vorteiler=1, 1 Stop-Bit, ohne Paritaet
	DB	3,00100000b	; WR3: Empfaenger aus, CTS+DCD-Steuerung
	DB	5,01101010b	; WR5: Sender ein, 8Bit, /DTR=1, RTS=1

; Kanal 2 - Duplex (1200 Baud)
	DB	01000111b	; CTC: DI, Zaehler, neg. Flanke, RESET
	DB	46		; Zeitkonstante
	DB	00011000b	; Port-RESET
	DB	4,01000100b	; WR4: Vorteiler=16, 1 Stop-Bit, ohne Paritaet
	DB	3,11100001b	; WR3: Empfaenger ein, 8Bit, CTS+DCD-Steuerung
	DB	5,01101010b	; WR5: Sender ein, 8Bit, /DTR=1, RTS=1

; Initialisierung SIO-B fuer Empfangs-Interrupt (1200 Baud):

V24UMT:	DB	2		; 2 Ports
	 DB	0Dh,2		; CTC1
	  DB	01000111b	; DI, Zaehler, neg. Flanke, ZK folgt, RESET
	  DB	46		; Zeitkonstante
V24INT:	 DB	0Bh,11		; SIO-B Steuerwort
	  DB	00011000b	; Port-RESET
	  DB	2,0E2h		; WR2: INT-Vektor
	  DB	4,01000100b	; WR4: Vorteiler=16, 1 Stop-Bit, ohne Paritaet
	  DB	3,11100001b	; WR3: Empfaenger ein, 8Bit, CTS/DCD-Steuerung
	  DB	5,11101010b	; WR5: Sender ein, 8Bit, /DTR=0 (bereit), RTS=1
	  DB	11h,00011000b	; WR1: Interrupt bei Zeichenempfang

; Initialisierung SIO-B fuer Polling-Betrieb:

V24POL:	DB	0Bh,7		; SIO-B Steuerwort, 7 Byte
	 DB	00011000b	; Port RESET
	 DB	4,01000100b	; Vorteiler 16, 1 Stop-Bit, ohne Paritaet
	 DB	3,11100001b	; Empfaenger ein, 8 Bit, CTS/DCD-Steuerung
	 DB	5,01101010b	; Sender ein, 8 Bit, DTR aus, RTS ein

; Byteweise MB-Ausgabe

MBOC:	LD	E,D		; Steuerbyte
	LD	D,A		; Datenbyte
	PUSH	DE
	PUSH	BC
	BIT	3,E
	JR	Z,NOINIT
	SET	1,(IX+7)	; merken INIT
	LD	A,'U'+80H
	CP	(HL)
	PUSH	DE
	LD	DE,0A0H		; Vortonlaenge
	JR	NZ,NOHIU
	LD	DE,500H		; bei 'U' gedehnt
NOHIU:	LD	(VORTN),DE
	LD	DE,CASS
	LD	BC,11
	LDIR			; Name
	EX	DE,HL
	POP	DE
	LD	(HL),D		; 1.Byte
	INC	L
MBOC1:	LD	A,L
	LD	(DTPTR),A
MBOCE:	POP	BC
	POP	DE
	RES	3,E
	RET
	;
NOINIT:	LD	HL,(DTPTR)
	LD	H,0B7H
	LD	(HL),D		; Byte eintragen
	BIT	6,E
	JR	NZ,MCLOS	; Close
	INC	L
	JP	P,MBOC1
	BIT	1,(IX+7)
	JR	NZ,MBOC2	; erst mal Init!
	CALL	MBLO		; Block ausgeben
MBOC6:	LD	HL,CASS
	JR	MBOC1
	;
MBOC2:	RES	1,(IX+7)	; ruecksetzen
	CALL	ISRO		; Init
	CALL	MBLNR		; Blocknr.
	JR	MBOC6
	;
MCLOS:	BIT	1,(IX+7)	; Init?
	JR	Z,MBOC3
	CALL	ISRO		; erst noch Init!
	JR	MBOC4
MBOC3:	CALL	MBLO		; Block ausgeben
MBOC4:	LD	BC,(VORTN)
	CALL	CSRO
MBOC5:	PUSH	DE
	CALL	CSRI
	RES	5,(IX+7)	; 'U'-Merker ruecksetzen
	POP	DE
	JR	MBOCE

; Byteweise MB-Eingabe

MBIC:	LD	E,D		; Steuerbyte
	LD	D,A		; Datenbyte (PA!)
	PUSH	DE
	PUSH	BC
	BIT	6,E		; Close?
	JR	NZ,MBOC5
	BIT	3,E		; Init?
	JR	Z,MBIU
	LD	A,1		; erwarteter Block
	LD	(IX+3),A
	PUSH	HL
	CALL	ISRI
MBIC1:	POP	HL
	JP	C,JBLERR	; Fehler: '*'
	LD	A,(IX+2)	; gelesener Block
	CP	1
	JP	NZ,JBLERR	; Fehler: '*'
	RES	7,(IX+7)	; Kopfblock geladen
	INC	(IX+3)		; erwarteter Block
	LD	DE,CASS
	LD	B,11
	LD	A,(DE)
	CP	'U'+80H		; hohes 'U'?
	JR	NZ,MBIC2
	SET	5,(IX+7)	; merken zur Blocknummernausgabeunterdrueckung
MBIC2:	ADD	A,29H
	JR	NC,MBIC3
	LD	A,(DE)		; geschuetzte BASIC-Programme
	SUB	4		; als ungeschuetzt anzeigen
	LD	(DE),A
	INC	DE
	LD	(DE),A
	INC	DE
	LD	(DE),A
	LD	(DATFLG),A	; <=hier LIST/EDIT/BYE-Schutz setzen
	SET	6,(IX+7)	; merken wofuer?
	LD	DE,CASS
MBIC3:	LD	A,(DE)		; Name aus Kassettenpuffer
	CALL	CRT		; anzeigen
	CP	(HL)		; stimmt Name?
	JR	Z,MBIC4
	SET	7,(IX+7)	; nein-merken
MBIC4:	INC	HL
	INC	DE
	DJNZ	MBIC3
	CALL	CRLF
	BIT	7,(IX+7)
	JP	NZ,JIOERR	 ; Name falsch -> ?IO ERROR
	EX	DE,HL
MBIC5:	LD	D,(HL)		; Daten lesen
	INC	L
	LD	A,L
	LD	(DTPTR),A
	LD	A,D
	POP	BC
	POP	DE
	RES	3,E
	RET

MBIU:	LD	HL,(DTPTR)
	LD	H,HIGH(CASS)
	BIT	7,L
	JR	Z,MBIC5		; aus Puffer
MBIU1:	CALL	MBI
	JR	C,MERR
	LD	A,0FFH
	CP	(IX+2)		; Block FF?
	JR	Z,MBIU3
	LD	A,(IX+3)
	CP	(IX+2)		; erwarteter?
	JR	Z,MBIU3
	LD	A,'*'
	CALL	CRT		; nicht erwartet
	CALL	BNROST
	DB	19H,0
MBIU2:	CALL	JBRKT
	JR	MBIU1

MBIU3:	INC	(IX+3)		; naechsten
	DEC	A		; erwarten
	BIT	5,(IX+7)
	JR	NZ,MBIU4
	CALL	BNROST		; Blocknummer
	DB	'>',19H,0	; unter Vorbehalt ausgeben
MBIU4:	LD	HL,CASS
	JR	MBIC5

MERR:	CALL	OSTR
	DB	9,9,9,9,'?',0	; Fehler im Block
	CALL	BNROST
	DB	CR,LF,0
	JR	MBIU2

JBLERR:	CALL	OSTR
	DB	'*',8,0		; nicht erwarteter 1. Block
	CALL	JBRKT
	CALL	MBI
	PUSH	HL
	JP	MBIC1

MBLO:	LD	BC,(VORTN)
	CALL	MBO
MBLNR:	CALL	BNROST
	DB	'>',19H,0
JBRKT:	CALL	BRKT		; bei BRK
	RET	NC		; auch ?IO ERROR
	JP	JIOERR

; Speicherschnellansicht

VIEW7F:	DW	7F7FH
	DB	'view'
	DB	3
	LD	A,L
	CP	10H
	JR	NC,VIEW1	; L>0Fh
	LD	A,H
	AND	A
	JR	NZ,VIEW1	; H>0
	LD	HL,ARG1
	RLD
	LD	H,(HL)
	LD	L,0
VIEW1:	LD	DE,8800H
	LD	BC,2000H
VIEW2:	CALL	LDAME		; Byte holen
	LD	(DE),A		; in IRM poken
	INC	HL
	INC	DE
	DEC	C
	JR	NZ,VIEW2
	DJNZ	VIEW2
VIEW3:	LD	A,0BH		; Zeile hoch
	CALL	CRT
	LD	A,8
	LD	(CURSO),A
	CALL	VIEW4
VIEW4:	LD	A,1
VIEW5:	LD	C,A
	CALL	AHEX		; HEX-Leiste
	LD	A,C
	ADD	A,22H
	JR	NC,VIEW5
	RET

; Druckereinstellungen

PRI7F:	DW	7F7FH
	DB	'PRINT',1FH
PRI:	LD	A,(DE)
	AND	A
	RET	Z		; fertig
	CP	' '
	JR	Z,PRINC
	CP	','
	JR	Z,PRK		; Zeichen
	CP	27H
	JR	Z,PRSTR		; String
	CALL	RHEX
	JP	C,ERRM		; Fehler!
	LD	A,(NUMVX)
PRIC:	CALL	OFILT
PRINC:	INC	DE
	JR	PRI
PRK:	INC	DE
	LD	A,(DE)		; Zeichen
	JR	PRIC
PRSTR:	INC	DE
	LD	A,(DE)
	AND	A
	RET	Z		; Ende-0
	CP	27H
	JR	Z,PRINC		; Stringende
	CALL	OFILT
	JR	PRSTR

; Tabelle der Modulnamen:

MTAB:	DB	001H,'START-ROM',0	; M033 Typestar
	DB	070H,'32K EPROM',0	; M045
	DB	071H,'64K EPROM',0	; M046
	DB	072H,'128K EPROM',0	; M047
	DB	073H,'256K EPROM',0	; M048
;	DB	074H,'512K EPROM',0	; reserviert!
;	DB	075H,'1M EPROM',0	; reserviert!
	DB	077H,'64K RAM',0	; M032 - 64k-Variante
	DB	078H,'128K RAM',0	; M036
	DB	079H,'256K RAM',0	; M032 - 256k-Variante
	DB	07AH,'512K RAM',0	; M034
	DB	07BH,'1M RAM',0		; M035, M035x4
;	DB	07CH,'32K EEPROM',0	; reserviert!
;	DB	07DH,'64K EEPROM',0	; reserviert!
;	DB	07EH,'128K EEPROM',0	; reserviert!
;	DB	07FH,'256K EEPROM',0	; reserviert!
	DB	0A7H,'FLOPPY',0		; D004, D008
	DB	0D9H,'EPROMER 32K',0	; M030 (fuer 2k-32k EPROMs)
	DB	0DAH,'PIO-3',0		; M002
	DB	0DBH,'EPROMER 64K',0	; M030 (fuer 2k-64k EPROMs)
	DB	0E3H,'DAU1',0		; M029
	DB	0E7H,'ADU1',0		; M010
	DB	0ECH,'SCANNER',0	; M051 (KC-Club)
	DB	0EEH,'V.24',0		; M003, M053
	DB	0EFH,'DIGITAL I/O',0	; M001
	DB	0F0H,'8K CMOS-RAM',0	; M120
	DB	0F1H,'16K CMOS-RAM',0	; M122
	DB	0F2H,'32K CMOS-RAM',0	; M124
;	DB	0F3H,'4*8K RAM',0	; M062-RAM (E. Mueller)
;	DB	0F3H,'8*8K EPROM',0	; M062-ROM (E. Mueller)
	DB	0F4H,'16K RAM',0	; M022
	DB	0F5H,'32K RAM',0	; M024
	DB	0F6H,'64K RAM',0	; M011
	DB	0F7H,'8K EPROM',0	; M025, M040
	DB	0F8H,'16K EPROM',0	; M028, M040
;	DB	0FBH,'8K SOFTWARE',0	; M012, M026, M027 (Sonderbehandlung!)
	DB	0FCH,'BASIC',0		; M006
	DB	0FDH,'USB+NET',0	; M052 (KC-Club, mit 32K EEPROM 4 * 8K)
	DB	'???',0
MTAB1:	DB	'SOFTWARE'		; Standardanzeige falls ohne Menuewort
MTAB2:	DB	0
MUSER:	DB	'USER',0		; M005 (USER-Module C0-D7)

	ABSFILL	0D000h,<ROM-C-Ende>
