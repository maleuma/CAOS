;*****************************************
;**	CAOS 4.8	ROM C		**
;**					**
;**	Adresse:  C000h bis CFFFh	**
;**					**
;**	letzte Aenderung: 21.05.2022	**
;*****************************************

;	ORG	0C000H

;
; IRM-Defaulttabelle:
;
KCTAB1:	DW	NOOP		; Hardcopy-Adresse		B799
WIN0:	DB	0		; WINNR
	DW	0		; WINON
	DW	2028H		; WINLG
	DW	0		; CURSO
	DB	80H		; STBT		(2-Monitor-Bit 7 gesetzt ab CAOS 4.8)
	DB	39H		; COLOR
	DW	SCRLPG		; WEND
WIN1:	DW	LARGE		; CCTL0
	DW	SMALL		; CCTL1
	DW	LARGE		; CCTL2
	DW	SMALL		; CCTL3
	DW	STACK		; SYSP
	DW	SUT_IRM		; SUTAB
	DW	CRTTAB		; CTAB
	OUT	(PIOAD),A	; NCAOS
	JP	0F012H
	DW	OUTT1		; OUTAB = CRT (0)
	DW	INTT1		; INTAB = KBD (4)
	JP	NOOP		; UOUT1
	JP	NOOP		; UIN1
	JP	NOOP		; UOUT2
	JP	NOOP		; UIN2
	DW	IOERR		; IOERR (BASIC)
	DW	VRAM0		; VRAM=0B200H (Bild 0)		B7C8

KCTAB2:	DB	47H,0CH		; CTC2 (blinken)		B7DB
	DW	ESCTAB		; L3TAB (Tabelle der ESC-Routinen)
	DB	(ESCX-ESCTAB)/2	; L3SIZ (Anzahl der ESC-Funktionen)
	DB	5		; COUNT
	DB	0		; HCPZ
	DW	0A801H		; INTV1
	DB	8		; INTV1L
	DW	0A809H		; INTV2
	DB	9		; INTV2L
	DB	11101110b	; HCPZ2 (8 Bit, Senden ein)
	JP	NOOP		; OFILT				B7E9
	DB	'%'		; PROMPT
	DB	0FFH		; LINTYP
	DW	0EFD8H		; CUMUST
	DW	JOYIRM		; JOYTAB			B7F0

; Modulschaltzustaende (interne Module):

KCTAB3:	DB	3	; RAM0
	DB	1	; IRM
	DB	0	; USER-ROMC
	DB	3	; RAM8
	DB	3	; RAM4
	DB	0	; CAOSC
KCTAB4:
;
; Power-ON Initialisierung:
;
PWRONC:	DI
	IM	2

; 02.03.2019: vor Loeschen des Speichers die Ports initialisieren um sicherzu-
; stellen, dass vor dem Speicherloeschen der RAM0 und der IRM eingeschaltet ist
	LD	SP,STACK	; dazu SP setzen um CALL zu ermoeglichen
	LD	HL,IOTAB1	; Daten der Erstinitialisierung
	CALL	INIMEX		; komplett benutzen

	LD	BC,0080H	; alle Module abschalten
PWRC2:	OUT	(C),D		; D=0 vom vorhergehenden INIMEX
	DJNZ	PWRC2

	LD	SP,0C000H	; Loeschbereich 0..BFFFh
	LD	BC,6000H	; Anzahl = 60h * 256 Wort = C000h
	LD	H,C
	LD	L,C		; HL=0
PWRC1:	PUSH	HL		; schnelles Speicher loeschen (rueckwaerts)
	DEC	C
	JR	NZ,PWRC1
	DJNZ	PWRC1
	LD	SP,STACK	; System-Stack setzen

; neu ab CAOS 4.8: Autostart INIT (z.B. bei USB) vorbereiten
; (Die Aktivierung erfolgt erst bei der Initialisierung des Modul-Treibers)
	LD	HL,INIT		; INIT-Kommando als Autostart
	LD	(FTAST),HL	; vorbereiten fuer Abarbeitung

; 08.10.2020: SYSI nach der Portinitialisierung aufrufen, sodass der KOUT-
; Impuls zum Ruecksetzen der D005-Tastatur nur bei RESET kommt.
	CALL	SYSI2		; Systeminitialisierung (Rueckkehr mit EI)
	LD	HL,JOYTBD	; vorbereitete Joystick-Tabelle
	LD	DE,JOYIRM	; JOYTAB-Standardadresse im IRM
	LD	BC,12
	LDIR			; Tabelle in den IRM kopieren

; seit CAOS 4.1 werden an dieser Stelle die Fenster 0-9 in dieser Reihenfolge
; mit den Standardwerten vorbelegt. Nebeneffekt: Beim Einschalten ist Fenster
; Nr. 9 aktiv entgegen dem eigentlich erwarteten Fenster Nr. 0
; -> Reihenfolge geaendert, nun ist Fenster 0 wieder aktiv wie bis CAOS 3.4

	LD	A,9		; Fenster Nr. 9
PWRC3:	LD	(WINNR),A	; Nr. eintragen
	CALL	WCOPY		; Fenstervektorspeicher initialisieren
	SUB	1		; fuer 10 Fenster (9-0)
	JR	NC,PWRC3
	CALL	ESC3		; Anzeige Bild 0, Zugriff Bild 1
;	LD	A,0CH
	CALL	CLSX		; Bild 1: CLS
	CALL	ESC1		; Anzeige+Zugriff Bild 0

	JP	PWR4		; weiter im ROM-F
;
; System-Initialisierung (bei PWRON und RESET):
;
SYSI:	DI
	IM	2
	LD	HL,IOTAB1	; PIO A,B und Ausgabe-Ports 84H/86H
	CALL	INIMEX		; initialisieren
SYSI2:	LD	IX,1F0H
	LD	(IX+1),28H	; OUT84-Merker (Bild 0, RAM8 Ebene 0)
	LD	(IX+4),0E3H	; OUT86-Merker (RAM4 ein, USER-ROM Ebene 3)
	LD	(IX+8),80H	; Defaultbelegung (Kassette, kein CAPS-Lock)
	LD	DE,HCADR
	LD	HL,KCTAB1	; Defaulttabelle
	LD	BC,KCTAB2-KCTAB1; fuer IRM-Arbeitszellen
	LDIR			; 1. Teil in IRM kopieren
	LD	E,LOW(CTCMD)	; (D war 0B7h)
	LD	C,KCTAB3-KCTAB2	; (B war 0)
	LDIR			; 2. Teil in IRM kopieren
	LD	DE,MODST	; Modulsteuerbytespeicher
	LD	A,(MODST+4)	; RAM4 aktueller Schaltzustand
	AND	4		; RAM4-Ebene herausloesen
	LD	C,KCTAB4-KCTAB3	; (B war 0)
	LDIR			; 3. Teil in IRM kopieren
	LD	HL,MODST+4
	OR	(HL)		; RAM4 bisherige Ebene wieder einbauen
	LD	(HL),A

; 08.08.2020: zusaetzlich fuer Cursor-Rettung bei Bildwechsel:
; 20.05.2021: von PWRON zu RESET verlagert

	LD	DE,WIN2		; Schattenspeicher
	CALL	WINC2		; mit Standardwerten initialisieren

; CAOS 4.6 - SUTAB vom ROM in den IRM kopieren:

	ld	de,SUT_IRM-1	; SUTAB im IRM (beginnend mit SULEN)
	ld	hl,SUTB-1	; SUTAB im ROM
	ld	BC,SUEND-SUTB+1	; Laenge
	LDIR			; SUTAB zur Nutzung in den IRM kopieren

	LD	A,1		; Standardwert fuer IXH- und I-Register
	CALL	SIXC		; Initialisierung, ISR-Tabelle kopieren, EI

; Interrupt hier bereits freigegeben, damit Treiber-Initialisierung
; Interrupts aktivieren kann (z.B. USB-Tastatur)?

; CAOS 4.6 - Device-Treibertabellen erzeugen

	LD	DE,DEVTAB	; Treibertabelle(n) im IRM aufbauen
	LD	A,8		; fuer 8 Treiber
DINIT:	LD	HL,DRV0		; zunaechst den Kassettentreiber
	LD	BC,32		; je 32 Byte (Groesse festgeschrieben)
	LDIR			; kopieren
	DEC	A		; alle 8 Treiber auf Kassette initialisieren
	JR	NZ,DINIT
	LD	(DEVTAB),A	; Tape-Treiber mit '0' aktivieren
	LD	DE,DEVTAB+32	; folgende Treiber als Nr. 1 eintragen

; Suche D004/D008 mit CAOS-Betriebsart (Treiber 1):

	LD	BC,0FC80H	; Modulsteckplatz FC
	IN	A,(C)
	CP	0A7H		; Floppy vorhanden?
	JR	NZ,NODISK
	LD	A,4		; nur Kopplung einschalten
	LD	(MODST+0FCH),A	; in Modulsteuerbytespeicher eintragen
	OUT	(C),A		; und schalten
	LD	BC,UROK		; 0B3F3H
	IN	A,(C)
	CP	5		; CAOS-Betriebsart?
	JR	NZ,NODISK

; 01.01.2022:
; Falls eine alte Original-CAOS-Diskette benutzt wird, wird diese Speicherzelle
; mit der CAOS-Versionsnummer nicht beschrieben und enthaelt den Inhalt vom
; Urlader. Dieser fuehrt zu falschen Versionsmeldungen. Deshalb wird hier eine
; Versionsnummer kleiner als 2.0 eingetragen - somit funktioniert die DEP-
; Versionsabfrage zuverlaessiger.
	LD	BC,DEPVER	; DEP-Version
	OUT	(C),A		; Version 0.5 setzen

;	LD	HL,DRV1		; Floppy-Treiber
	LD	BC,32
	LDIR			; in Treibertabelle kopieren

; Suche USB- bzw. GIDE-Modul mit Treiber:
;	DE	zeigt auf DEVTAB wo naechster Treiber hin kommt

NODISK:	LD	B,8		; ab Steckplatz 8 suchen
	LD	C,80H		; I/O-Adresse Modulsteuerung
USB1:	IN	A,(C)		; Kennbyte lesen
	CP	0FDH		; Kennung M052?
	JR	Z,USB2
	CP	0F9h		; Kennung M064?
	JR	Z,USB2
USB4:	INC	B		; naechster Steckplatz
	JR	NZ,USB1
	JR	NOUSB		; Kein passendes Modul gefunden!
	;
USB2:	PUSH	BC		; Steckplatz B merken
	LD	L,B
	call	SLOT1		; Modul auf 4000h einblenden
;	ld	hl,4000h
	ld	a,(hl)		; ROM-Inhalt 1. Byte
	inc	hl
	cp	(hl)		; ROM-Inhalt 2. Byte
	jr	nz,USB3		; Kennung 1+2 nicht identisch
	cp	46h		; Kennung Treiber korrekt? 
	jr	nz,USB3		; Kennung falsch
	inc	hl		; SWITCH-AUS in Treibertabelle
	ld	a,e
	rlca
	rlca
	rlca			; Nr. des naechsten Treibers
	ld	(de),a		; Treibernummer eintragen in DEVTAB+0
	inc	de
	EX	(SP),HL		; H=Steckplatz
	ld	a,H
	ld	(de),a		; Steckplatz eintragen in DEVTAB+1
	inc	de
	EX	(SP),HL
	LD	BC,32-2
	LDIR			; restliche Treibertabelle aus ROM kopieren
	ld	hl,-1Dh
	ADD	HL,DE		; zurueck zu SWITCH-EIN des Moduls
	ld	A,(HL)		; SWITCH-EIN

USB3:	POP	BC		; B = Steckplatz, C=80h
	PUSH	BC
	PUSH	DE		; Zeiger in DEVTAB retten
	; kein Befehl beeinflusst das Z-Flag !
	CALL	Z,JROM		; Treiber gefunden: INIT im Modul-ROM aufrufen
	CALL	SLOT0		; alle Modulzustaende regenerieren
	POP	DE		; DEVTAB
	POP	BC		; B = Steckplatz, C=80h
	LD	A,D
	CP	1+High(DEVTAB)	; 0AAh
	JR	NZ,USB4		; weitere Module suchen bis DEVTAB voll ist
NOUSB:	LD	A,(DEVTAB+32)	; Treiber 1 vorhanden?
	CP	1
	CALL	Z,SET_DD	; dann aktivieren!
	RET
;
; CAOS-UP zur Verlagerung des IX-Arbeitsbereiches
; PE:	A	HW-Teil der IX-Arbeitszellen
;	A=FFH	zusaetzlich DEVICE-Tabelle und SUTAB neu aufbauen
;
SIXC:	CP	0FFH
	JP	Z,SYSI		; grosse Initialisierung mit Device-Tabelle usw.
	LD	HL,ISRTAB	; ISR-Tabelle im ROM
	DI
	LD	(MIXIT),A	; neuer Wert fuer IXH- und I-Register
	LD	I,A		; I-Register aktualisieren
	IM	2
	LD	E,0D0H
	LD	D,A
	LD	BC,ISRTX-ISRTAB	; Interrupttabelle fuer 16 Interrupts
	LDIR			; kopieren aus dem ROM nach xxD0h bis xxF0h
	LD	H,(IX+1)
	LD	L,(IX+4)	; alte Merker mitnehmen
	LD	A,(IX+8)
	PUSH	DE		; neuer Wert fuer IX = xxF0h
	POP	IX
	LD	(IX+8),A
	LD	(IX+1),H	; mitgenommene Merker neu eintragen
	LD	(IX+4),L
	LD	(IX+14),LOW(KTAB) ; Tastaturtabelle
	LD	(IX+15),HIGH(KTAB)
	LD	(IX+9),7FH	; Standard-Prologbyte
	LD	HL,IOTAB2	; PIO A,B und CTC
	CALL	INIMEX		; Ports neu initialisieren
	LD	HL,CENINI
	CALL	INIMEX		; M021 initialisieren
	CALL	KPUFF		; Kassettenpuffer (IX+5 und IX+6) definieren
	EI
	RET

; Systemcheck oder Modulkontrollanzeige (interne Module):

SYSTC:	LD	L,-1		; mit Steckplatz 0 beginnen, vorher aber ROM E
	CALL	OSTR2
	DB	'CAOSE',0
	IN	A,(PIOAD)
	AND	1		; CY=0, Z=?
	JR	WONOF1

SYST0:	LD	A,1		; lesen
	CALL	MODUC
	CALL	LHD		; Anzeige Steckplatz, Kennbyte, Steuerbyte

	LD	B,L		; Steckplatz
	INC	B		; +1 wegen folgendem DJNZ
	DJNZ	SYST1		; nach Steckplatz verzweigen
; 0
	CALL	OSTR
	DB	'RAM0',0
	IN	A,(PIOAD)
	AND	00001111b
	CP	00001000b	; CY=NOT(Bit 3)
	BIT	1,A		; Schaltzustand
	JR	WONOF1
; 1
SYST1:	DJNZ	SYST2
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
WONOF1:	JR	SYST6
; 2
SYST2:	DJNZ	SYST3
	CALL	OSTR
	DB	'USER ',0
	LD	A,(IX+4)
	RLCA			; Bits 5,6
	RLCA
	RLCA			; nach Bits 0,1
	AND	3
	XOR	'3'
	CALL	OCHR		; Ebene ausgeben
	IN	A,(PIOAD)
	AND	10000000b	; CY=0, Z=NOT(Bit 7)
	JR	SYST6
; 3
SYST3:	DJNZ	SYST4
	CALL	OSTR
	DB	'RAM8 ',0
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
syst7:	JR	SYST6
; 4
SYST4:	DJNZ	SYST5
	CALL	OSTR
	DB	'RAM4',0
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
	JR	SYST6
; 5
SYST5:	CALL	OSTR
	DB	'CAOSC',0
	LD	A,(MODST+5)
	AND	1
SYST6:	CALL	WONOF
	INC	L		; Steckplatz+1
	LD	A,6
	CP	L
	JP	NZ,SYST0	; bei 06 abbrechen
	RET

OSTR2:	CALL	OSTR
	DB	'-- -- -- ',0
	JP	OSTR

; Modulkontrollanzeige (externe Module):

MODULC:	XOR	A
	OUT	(91h),A		; M008/M021 PIO-B Daten
	IN	A,(91h)
	AND	A		; PIO vorhanden?
	JR	NZ,MODC1	; nein, nicht anzeigen
	CALL	OSTR2
	DB	'JOY/CEN',CR,LF,0
MODC1:	LD	L,7		; ab Platz 7
MODC2:	PUSH	HL
	LD	A,1		; lesen
	CALL	MODUC
	LD	A,H
	INC	A		; Kennbyte=FFh?
	JR	Z,MODC8		; kein Modul

	CALL	LHD		; Anzeige Steckplatz, Kennbyte, Steuerbyte

	LD	C,H
	ld	a,h
	cp	0f3h		; M062?
	jr	z,M062		; Test ob RAM- oder ROM-Version
	cp	0fbh		; Software-Modul?
	jr	z,slot		; Inhalt analysieren!
	cp	0fdh		; M052?
	jp	z,M052		; Test, welche Variante vorliegt
	ld	hl,muser
	cp	0c0h
	jr	c,MODC3		; kleiner als C0
	cp	0d8h
	jr	c,MODC6		; USER-Modul von C0-D7
MODC3:	LD	HL,MTAB
MODC4:	LD	A,(HL)
	INC	HL
	CP	0FFH		; Tabelle durchsucht?
	JR	Z,MODC6
	CP	C		; Strukturbyte in Tabelle MTAB?
	JR	Z,MODC6		; Text gefunden
MODC5:	LD	A,(HL)
	INC	HL
	AND	A		; Stringende?
	JR	NZ,MODC5
	JR	MODC4
	;
MODC6:	CALL	ZKOUT		; Modulname
MODC7:	CALL	CRLF
MODC8:	POP	HL
	INC	L		; Steckplatz+1
	JR	NZ,MODC2	; bis FFh
	RET
;
; Test, ob M062 als 8*8K ROM oder 4*8K RAM vorliegt.
; Zur Unterscheidung wird ein RAM-Test durchgefuehrt
;
; PE:	L=Steckplatz	H=Kennbyte = 0FBH
;
M062:	call	SLOT1		; Modul auf 4000h einblenden
;	ld	hl,4000h	; RAM-Inhalt lesen
	ld	a,(hl)
	inc	(hl)		; Inhalt veraendern
	cp	(hl)		; RAM oder ROM
	ld	(hl),a		; regenerieren
	ld	hl,M062B	; ROM
	jr	z,SLOT2
	ld	hl,M062A	; RAM
SLOT2:	call	zkout		; Testergebnis anzeigen
SLOT3:	call	SLOT0		; Modulzustaende nach Tabelle regenerieren
	jr	MODC7		; weiter suchen auf naechstem Steckplatz
;
; Suche nach Inhalt der Softwaremodule M012, M026, M027 usw.
; Es wird das erste Menwort gesucht, und dieses als Name angezeigt!
; PE:	L=Steckplatz	H=Kennbyte = 0FBH
;
slot:	call	SLOT1		; Modul auf 4000h einblenden
;	ld	hl,4000h	; Suchbereich Beginn
	ld	de,MTAB2	; Vergleichskette 0-String
	ld	b,20h		; 8K durchsuchen
;	ld	a,07Fh		; CAOS-Prologbyte fuer Menuewort-Suche
	call	zs0		; erstes Menuewort suchen
	jr	c,SLOT4		; Menuewort gefunden
	ld	hl,mtab1	; ansonsten als "Software" anzeigen
	jr	SLOT6
	;
SLOT4:	dec	hl
	ld	a,(hl)
	cp	7fh		; zurueck bis zu Prologbyte
	jr	nz,SLOT4
SLOT5:	inc	hl
SLOT6:	ld	a,(hl)
	cp	20h		; Epilog?
	jr	c,SLOT3		; ja
	call	ochr		; Menuewort anzeigen
	jr	SLOT5
;
; Testen welche Variante des M052 vorliegt. Dazu werden die Datenports von der
; Netzwerk-PIO gelesen und falls ein Wert ungleich FFh kommt, von einem
; Kombimodul mit USB+Netzwerk ausgegangen.
; M052-Portadressen: 28h..2Bh Netzwerk-PIO / 2Ch..2Fh USB-PIO
;
; PE:	L=Steckplatz	H=Kennbyte = 0FDH

M052:	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	ld	a,(hl)		; aktueller Zustand
	set	2,a		; PIO's ein
	ld	b,l		; Steckplatz
	ld	c,80h
	out	(c),a		; PIO's des M052 einschalten
	ld	hl,M052A
	in	a,(28h)		; M052 PIO NET Daten A
	inc	a
	jr	nz,M052N1	; Netzwerk-PIO gefunden
	in	a,(29h)		; M052 PIO NET Daten B
	inc	a
	jr	nz,M052N1	; Netzwerk-PIO gefunden
	ld	hl,M052B
M052N1:	JR	SLOT2		; Modulname anzeigen, Schaltzustaende regen.

; Module lesen und schalten (Menuewort):

SWIC:	LD	D,E		; Steuerbyte
	CP	1
	PUSH	AF
	JR	Z,SWI3		; nur lesen
	DEC	L
	JR	NZ,SWI2		; nicht IRM
	INC	L
	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	E,(HL)		; altes Steuerbyte lesen
	LD	(HL),D		; neues Steuerbyte eintragen
	LD	H,0FFH		; Kennung internes Modul
	JR	SWI4
	;
SWI2:	INC	L
SWI3:	CALL	MODUC		; lesen/schalten
	LD	E,A
SWI4:	CALL	ALSPC		; Steckplatz L anzeigen
	LD	A,H
	CALL	AHSPC		; Kennbyte Modul
	LD	A,E
	CALL	AHEX		; Steuerbyte alt
	POP	AF
	JR	Z,SWI5		; das war nur lesen
	LD	A,9
	CALL	CSTBT		; Pfeil nach rechts anzeigen
	LD	A,D
	CALL	AHEX		; Steuerbyte neu
SWI5:	JP	CRLF
;
; Modul auf Adresse 4000h einblenden
; (fuer M012, M026, M027, M052 und M062)
; PE:	L =	Steckplatz
; PA:	HL = 4000h fuer folgende Operationen
; VR:	AF, BC, HL
;
SLOT1:	LD	BC,0880h	; alle Module von 8 bis L abschalten
slo1:	XOR	A		; Modul abschalten
	OUT	(C),A		; war vorher ED 71 = OUT (C),0
	inc	b		; naechster Steckplatz
	ld	a,l		; Steckplatz, der zu untersuchen ist
	cp	b		; Steckplatz erreicht?
	jr	nz,slo1
	ld	a,41h		; Modul auf 4000h einschalten
	out	(c),a
	RESIXA	0,4		; RES 0,(IX+4),A
	OUT	(PORT2),A	; RAM4 aus!
	LD	HL,4000h	; Uebergabe der zu untersuchenden Adresse
	RET
;
; Modulzustand regenerieren laut Modulsteuerbytetabelle
; (erforderlich falls Modulsuche Aenderungen gemacht hat)
; PE:	-
; PA:	-
; VR:	AF, HL, D
;
SLOT0:	ld	hl,modst+4	; beginnend ab RAM4
slo0:	ld	d,(hl)		; gespeicherter Schaltzustand
	push	hl
;	ld	a,2		; schalten
	call	MODUC2		; Schaltzustaende nach Tabelle regenerieren
	pop	hl
	inc	l		; naechster Steckplatz
	jr	nz,slo0		; bis FFh
	ret
;
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
	;
V24S2:	LD	L,B		; Steckplatz
MODUC1:	LD	D,1		; EIN
MODUC2:	LD	A,2		; Schalten

; Module schalten:
;
; PE:	A=1	Modultyp lesen (Register L)
;	A=2	Modul schalten (Register D und L)
;	L	Modulsteckplatz
;	D	Modulsteuerbyte neu
; PA:	H	Modultyp (Strukturbyte)
;	D	Modulsteuerbyte neu
;	A	Modulsteuerbyte alt
; VR:	AF, D, H, BC

MODUC:	LD	H,HIGH(MODST)	; Modulsteuerbytespeicher
	LD	C,80H		; I/O-Adresse
	LD	B,L		; Steckplatz
	CP	2		; 2 Parameter angegeben?
	LD	A,(HL)		; urspruenglicher Schaltzustand
	JR	NC,MODU1	; Module schalten
	LD	D,(HL)		; Schaltzustand aus Steuerbytespeicher lesen
RSTRB:	IN	H,(C)		; Strukturbyte aus Modul lesen
	RET

	;Modul schalten
MODU1:	PUSH	AF		; alter Zustand
	LD	(HL),D		; neues Steuerbyte eintragen
	LD	H,A		; fuer 2. RAM4
	LD	A,L
	CP	6		; interne Module?
	JR	C,MODU2
	OUT	(C),D		; Steuerbyte an Modul senden
	POP	AF		; urspruenglicher Schaltzustand
	JR	RSTRB		; und noch das Strukturbyte lesen

	;interne Module
MODU2:	INC	B		; Steckplatz
	DI			; damit PIO nicht von einer ISR geaendert wird
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

MODU4:	DJNZ	MODU5		; 2: USER-ROM?
	LD	A,D
	CPL			; negieren
	RLCA			; Bits 4..5 auf 5..6 rotieren
	AND	01100000b	; ausfiltern (2 Ebenen)
	LD	H,A
;	DI
	LD	A,(IX+4)
	AND	10011111b
	OR	H		; einbauen
	CALL	OUT86		; (IX+4) und PORT2 einstellen
;	EI
	LD	A,D
	RRCA			; CY = Bit 0
	IN	A,(PIOAD)
	RLA			; als Bit 0 einschieben
	RRCA			; und auf Bit 7 rotieren
OUT88:	OUT	(PIOAD),A
	JR	STBFF

MODU5:	DJNZ	MODU6		; 3: RAM8?
	LD	A,00000011b	; on/off und Schreibschutz aus Steuerbyte
	AND	D
	RRCA			; Bits 0..1 auf 5..6 rotieren
	RRCA
	RRCA
	LD	H,A		; in H merken
;	DI
	IN	A,(PIOBD)
	AND	10011111b
	OR	H		; on/off und Schreibschutz neu einstellen
	OUT	(PIOBD),A
;	EI
	LD	A,D
	RLCA			; Bits 2..5 auf 4..7 rotieren
	RLCA
	ADD	A,20h		; Segment-Verschiebung
	AND	0F0h		; nur Bit 4-7 benutzen
	LD	H,A
;	DI
	LD	A,(IX+1)
	AND	0Fh		; Bit 4-7 ruecksetzen
	OR	H		; RAM8-Segment gemaess Steuerbyte einbauen
	CALL	OUT84		; (IX+1) und PORT1 einstellen
STBFF:	EI
	POP	AF		; urspruenglicher Zustand
	LD	H,0FFh		; internes Modul melden
	RET

IF CAOS2
MODU6:	DJNZ	MODU7		; 5=CAOSC
else
MODU6:	DJNZ	STBFF		; 5=CAOSC
endif
	LD	A,D		; 4=RAM4
	XOR	H
	AND	4		; Aenderung?
;	DI
	JR	Z,RAM41
	LD	A,(IX+4)
	OR	3
	OUT	(PORT2),A	; RAM4 on
	PUSH	DE
	PUSH	HL
	IN	A,(PIOAD)
	PUSH	AF
	RES	0,A
; dieser Datenaustausch muss im DI laufen, da der CAOS-ROM E aus ist!
	OUT	(PIOAD),A	; CAOSE off
	LD	A,(IX+1)
	PUSH	AF
	AND	11111001b
	LD	L,3		; 3 IRM-Ebenen
	LD	BC,1000H	; 800H frei!
	LD	DE,4000H
R4IS:	ADD	A,2
	OUT	(PORT1),A	; IRM-Ebene
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
	OUT	(PORT1),A	; IRM wie zuvor
	POP	AF
	OUT	(PIOAD),A	; CAOSE on
; ab hier koennte wieder EI eingestellt werden, CAOS-ROM E ist wieder ein
	POP	HL
	POP	DE
RAM41:	LD	A,(IX+4)
	XOR	D
	AND	11111100b	; Bits 0..1: a = (0 XOR d) = d
RAM42:	XOR	D		; Bits 2..7: a = (a XOR d) XOR d
	CALL	OUT86		; (IX+4) und PORT2 einstellen
	JR	STBFF
if CAOS2
MODU7:	LD	A,(IX+4)
	XOR	D
	AND	11101111b	; Bit 4 schaltet ROM-Ebene (Zeichensatz) um
	JR	RAM42
endif

; F-Taste belegen:

KEYC:	LD	HL,FTASTE	; 0B900H
	AND	A
	JR	NZ,KEY0
	LD	B,9CH
KEYCL:	LD	(HL),A		; loeschen
	INC	L
	DJNZ	KEYCL
	RET
	;
KEY0:	CP	15+1
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
	;
KEY3:	BIT	0,B
	JR	Z,KEY4		; interpretieren
	CP	3
	JR	Z,KEY12		; BRK=Abbruch
	CP	CR
	JR	Z,KEY5		; Enter=belegen
	CALL	OCHR
	JR	KEY2
	;
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
	;
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
	;
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

KEYLIC:	LD	HL,FTASTE+1
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
	LD	H,A		; 0 = aktuelle Farben anzeigen
	LD	A,(COLOR)
	RRCA
	RRCA
	RRCA
	AND	1FH
	LD	L,A
	CALL	HLDEZ		; Anzeige Vordergrundfarbe
	CALL	SPACE
	LD	A,(COLOR)
	AND	7
	LD	L,A
	CALL	HLDEZ		; Anzeige Hintergrundfarbe
	CALL	CRLF
	JR	COLRC2

; COLOR-Unterprogramm = Farbe setzen (aus L und E)

SETCO:	LD	A,1FH		; 1 oder 2 
	AND	L		; Maske fuer Vordergrundfarbe
	RLA
	RLA
	RLA
	LD	L,A
	LD	A,(ARGN)
	CP	2		; auch Hintergrund einstellen?
	LD	A,E		; neue Hintergrundfarbe
	JR	NC,COLRC1
	LD	A,(COLOR)	; alte Hintergrundfarbe
COLRC1:	AND	7
	OR	L
	LD	(COLOR),A	; neuer Farbwert
COLRC2:	JP	COFF

; WINDOW-Menuewort
; 21.05.2021: Bedeutung der Argumente wie bei BASIC angepasst
; PE:	A	Anzahl Argumente
;	A=0	Standardfenster (volle Groesse) einstellen
;	A=1	Fenster mit Nr. aufrufen
;	A=4(5)	Fenster (mit Nr.) definieren
;	L	Erste Zeile / Fensternummer bei A=1
;	E	Letzte Zeile	(bisher Anzahl Zeilen)
;	C	Erste Spalte
;	(ARG4)	Letzte Spalte	(bisher Anzahl Spalten)
;	(ARG5)	Fensternummer
;
WINDC:	AND	A		; Kein Argument?
	JR	Z,WINC0		; Standardfenster initialisieren
	DEC	A		; 1 Arg?
	JR	Z,WINAKL	; Aufruf Nummer L
	SUB	3
	JR	C,WINCJE	; 2..3 Arg => ERROR
	JR	Z,WINC1		; Fenster Nr. 0
	LD	A,(ARG5)	; Fenster nr definieren
WINC1:	LD	B,A		; Fensternummer

	LD	A,E		; letzte Zeile
	SUB	L		; erste Zeile
;	JR	C,WINCE		; Anfang > Ende (wird von WININ ueberwacht)
	INC	A
	LD	D,A		; Anzahl Zeilen (Fensterhoehe)

	LD	A,(ARG4)	; letzte Spalte
	SUB	C		; erste Spalte
;	JR	C,WINCE		; Anfang > Ende (wird von WININ ueberwacht)
	INC	A
	LD	E,A		; Anzahl Spalten (Fensterbreite)

	LD	H,L		; H = erste Zeile
	LD	L,C		; L = erste Spalte
	LD	A,B		; A = Fensternummer

	CALL	WININC		; Fenster initialisieren
	RET	NC		; OK
WINCJE:	JP	ERRM		; ansonsten Fehler anzeigen

WINC0:	LD	DE,WINNR	; von Fensternummer
WINC2:	LD	HL,WIN0		; bis WEND-Adresse
	LD	BC,WIN1-WIN0	; mit Standardwerten aus ROM
	JR	LDIRET		; Fenster 0 neu initialisieren

; definiertes Fenster aufrufen:
; PE:	A	Fensternummer 0..9
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL

WINAKL:	LD	A,L		; Fensternummer
WINAKC:	CALL	WCOPY		; aktuelles Fenster retten
	LD	(WINNR),A	; neue Fensternummer
;Einsprung fuer BASIC (PRINT AT)
WINAKB:	CALL	WINADR		; Adresse Vektorspeicher in DE
	RET	C		; Nr. ausserhalb
	EX	DE,HL		; HL=Adresse der gespeicherten Fensterdaten
	LD	DE,WINON	; gespeichertes Fenster laden
	JR	WCOP1

; Testet Fenster-Nr. und berechnet Position in Fenstervektorspeicher
; PE:	A	Fensternummer 0-9
; PA:	CY=1	Nr. zu gross
;	DE	Adresse im Fenstervektorspeicher

WINADR:	ADD	A,-10
	RET	C		; zu gross
	ADD	A,A	; *2
	LD	E,A
	ADD	A,A	; *4
	ADD	A,A	; *8
	ADD	A,E	; *10
	LD	E,A
	LD	D,High(WNDFN)	; Adresse im Fenstervektorspeicher
	AND	A		; CY=0
	RET

; aktuelles Fenster (WINNR) in Fenstervektorspeicher retten
; PE:	A	Fensternummer 0..9
; PA:	CY=1	Nr. zu gross
;	BC=0
; VR:	F,BC,DE,HL

WCOPY:	PUSH	AF
	LD	A,(WINNR)	; aktuelle Fensternummer
	CALL	WINADR		; Adresse berechnen
	JR	C,WINIE		; Fehler
	POP	AF
	LD	HL,WINON	; ab Fensteranfang 10 Byte retten
WCOP1:	LD	BC,10		; Laenge Fenstervektor
LDIRET:	LDIR
	RET

; Initialisierung eines Fensters
; PE:	A	Fensternummer 0..9
;	HL	Fensteranfang
;	DE	Fenstrergroesse
; PA:	CY=1	Fehler
; VR:	AF,BC,DE,HL

WININC:	PUSH	AF
	CALL	WININ1
WINIE:	POP	AF		; Fehlerende
	SCF
	RET
	;
WININ1:	CP	10		; Fensternummer
	RET	NC		; nur 10 Fenster moeglich
	LD	A,D		; Zeilen
	AND	A
	RET	Z		; Zeilenanzahl=0 -> ERROR
	ADD	A,H
	RET	C		; Anzahl+Anfang>256 -> ERROR
	CP	33
	RET	NC		; Anzahl+Anfang>32 -> ERROR
	LD	A,E		; Spalten
	AND	A
	RET	Z		; Spaltenanzahl=0 -> ERROR
	ADD	A,L
	RET	C		; Anzahl+Anfang>256 -> ERROR
	CP	41
	RET	NC		; Anzahl+Anfang>40 -> ERROR
	POP	AF		; Ruecksprung nach WINIE vom Stack
	POP	AF
;Einsprung fuer BASIC (PRINT AT)
WININB:	PUSH	DE
	PUSH	HL
	CALL	WCOPY		; aktuelles Fenster retten
	POP	HL
	POP	DE
	LD	(WINON),HL	; Fensteranfang
	LD	(WINLG),DE	; Fenstergroesse
	LD	(CURSO),BC	; BC=0
	LD	(WINNR),A	; Fensternummer
	AND	A		; CY=0
	RET

if SYSROM
;
; 13.06.2021:
; JUMP-Erweiterung zum Sprung in 8 verschiedene CAOS-Versionen.
; PE:	A=0-7
;
JUMPC:	DI			; Interrupt global sperren
	LD	DE,NCAOS+2-LENX	; im IRM vor dem JUMP zu NCAOS
	PUSH	DE		; ist gleichzeitig Sprungziel
	LD	HL,CODEX	; Umschalt-Code vom ROM
	LD	BC,LENX		; Laenge
	JR	LDIRET		; in IRM kopieren und dort starten
; ueberschreibt zwar CCTL-Zeichentabellen und auch den System-SP-Merker sowie
; den Zeiger auf die SUTAB - macht aber nichts, da mit dem Sprung zu F012H das
; gesamte System neu initialisiert wird (entspricht RESET)

CODEX:	LD	B,A		; CAOS-Ebene 0..7
	INC	B		; Ebene 1-8
	ld	a,0E7h		; CAOS-C ein, RAM4 ein, Reset-Impuls ein
	OUT	(PORT2),A
R0:	OR	8		; Zaehl-Impuls ein
	OUT	(PORT2),A	; Impuls ausloesen
	AND	0F3h		; RESET-Impuls und Zaehl-Impuls aus
	OUT	(PORT2),A	; Impuls wieder aus
	DJNZ	R0		; Bis n Impulse ausgegeben sind, danach:
;	JP	0F012H		; in neues System springen
LENX	EQU $-CODEX
endif

; Linie zeichnen					**3E**
; von (ARG1)/(ARG2) nach (ARG3)/(ARG4)

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
	CALL	C,POINT		; Punkt setzen
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
	RET	Z	; CY=0
	DEC	BC
	EX	AF,AF'		; Bitreg
	JR	LINCS

; Kreis zeichnen					**3F**

CIRCLC:	LD	A,(ARG3)	; Radius R
	LD	C,A
	LD	D,C
	CALL	MULT		; BA=C*D
	LD	H,B
	LD	L,A		; HL = R*R
	LD	D,H
	LD	E,L		; DE = HL
	LD	A,C
	LD	B,0		; BC=Radius
	SLA	C
	RL	B		; BC=Radius*2
	DEC	BC
	PUSH	BC
	LD	BC,1
	EXX
	 LD	C,A
	 LD	B,0		; BC=Radius
;
CLOOP:	 PUSH	HL
	 LD	HL,LINTYP	; Linientype beruecksichtigen
	 RLC	(HL)		; CY=1 wenn Punkt zu setzen ist
	 POP	HL
	 JR	NC,CIRN		; keinen Punkt setzen
	 CALL	QUA		; und jeweils 8
	 CALL	QUA		; Punkte setzen
CIRN:	EXX
	AND	A
	SBC	HL,BC
	INC	BC
	INC	BC
	SBC	HL,DE
	ADD	HL,DE
	EXX
	 JR	NC,CIR3
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
CIR3:	 INC	B
	 LD	A,C
	 CP	B
	 JR	NC,CLOOP
	POP	AF
	OR	A	; CY=0 (wegen BASIC erforderlich)
	RET

; setzt 4 Punkte um Mittelpunkt

QUA:	LD	A,B
	LD	B,C
	LD	C,A
	XOR	A
	LD	D,A
	LD	HL,(ARG2)	; Y-Mittelpunkt
	PUSH	HL
	LD	E,C
	ADD	HL,DE		; oberhalb vom Mittelpunkt
	CALL	UPP		; 2 Punkte setzen
	XOR	A	; CY=0
	LD	D,A
	POP	HL
	LD	E,C
	SBC	HL,DE		; unterhalb vom Mittelpunkt
	;			; und reinlaufen
UPP:	PUSH	HL
	LD	HL,(ARG1)	; X-Mittelpunkt
	LD	E,B
	AND	A	; CY=0
	SBC	HL,DE		; links vom Mittelpunkt
	POP	DE		; Y-Koordinate
	CALL	POINT		; 1 Punkt setzen
	PUSH	DE
	LD	D,A
	LD	E,B
	ADD	HL,DE
	ADD	HL,DE		; rechts vom Mittelpunkt
	POP	DE		; und reinlaufen

;***************************************************************
; universelles Punkt(re)set-Programm
; PE:	HL	X-Koordinate
;	DE	Y-Koordinate
;	(FARB)	Bit 3...7 : Farbe
;		Bit 2=1   : Farbebene nicht aendern
;		Bit 1=1   : PRESET
;		Bit 0=1   : PXOR
; VR:	-

POINT:	PUSH	HL
	PUSH	DE
	PUSH	BC
	PUSH	AF
	CALL	CALXY		; Grafikposition berechnen
	CALL	NC,PUSET	; Punkt setzen wenn OK
	JP	POP4		; POP	AF,BC,DE,HL   RET
	;
PUSET:	BIT	3,(IX+1)	; HiRes?
	JR	Z,HIRES		; Punkt in Vordergrundfarbe setzen
	BIT	1,D
	JR	NZ,PUERA	; Punkt loeschen
	BIT	0,D
	JR	NZ,PUXOR	; Punkt invertieren

; Punkt setzen:
	OR	(HL)		; Bit setzen
	JR	PUCOL

; Punkt invertieren:
PUXOR:	XOR	(HL)		; alternierend
	RES	0,D
PUCOL:	LD	(HL),A		; in Pixel-RAM eintragen
	BIT	2,D		; Bit 2 in CAOS 3.4 und ab CAOS 4.7 benutzt
	RET	NZ		; wenn Bit 2=1, dann Farbe nicht setzen
	LD	A,(IX+1)
	LD	E,A
	XOR	2		; Farbebene
;	 DI
;	 OUT	(PORT1),A	; Umschaltung Farbebene
	CALL	OUT84		; Umschaltung Farbebene (IX+1) und PORT1
	LD	A,(HL)
	AND	7		; alte vFarbe loeschen
	OR	D		; neue vFarbe einbauen
HIR2:	LD	(HL),A		; und in Color-RAM eintragen
HIR3:	LD	A,E
;	 OUT	(PORT1),A	; zurueck zu Pixelebene
;	 EI
	CALL	OUT84		; zurueck zu Pixelebene (IX+1) und PORT1
	RET

; Punkt loeschen:
PUERA:	CPL
	AND	(HL)		; loeschen
	LD	(HL),A
	RET

; Punkt im HIRES-Modus setzen:
HIRES:	LD	B,A		; Bitmaske
	OR	(HL)		; Bit setzen in
	BIT	3,D		; Pixelebene?
	JR	NZ,HIR1
	XOR	B		; wieder loeschen
HIR1:	LD	(HL),A
	LD	A,(IX+1)
	LD	E,A
	XOR	2		; Farbebene
;	 DI
;	 OUT	(PORT1),A	; Umschaltung Farbebene
	CALL	OUT84		; Umschaltung Farbebene (IX+1) und PORT1
	LD	A,B
	OR	(HL)		; Bit setzen in
	BIT	4,D		; Farbebene?
	JR	NZ,HIR2
	XOR	B		; wieder loeschen
	JR	HIR2

; Punkt setzen						**30**

PUSEC:	PUSH	HL
	PUSH	DE
	push	bc		; B-Register bei HIRES benutzt
	CALL	CALHV		; Grafikposition berechnen
	CALL	NC,PUSET	; Punkt setzen wenn OK
	JR	PUDEE

; Punkt loeschen (HIRES nur testen)			**2F**

PUDEC:	PUSH	HL
	PUSH	DE
	push	bc		; B-Register bei HIRES benutzt
	CALL	CALHV		; Grafikposition berechnen
	JR	C,PUDEE		; ausserhalb
	BIT	3,(IX+1)	; HiRes?
	JR	NZ,PUDEL
; Punktfarbe HIRES ermitteln
	LD	D,0
	LD	B,A		; Bitmaske
	LD	A,(HL)
	AND	B		; Bit in Pixelebene gesetzt?
	JR	Z,HIR4
	SET	0,D
HIR4:	LD	A,(IX+1)
	LD	E,A
	XOR	2		; Farbebene
;	 DI
;	 OUT	(PORT1),A	; Umschaltung Farbebene
	CALL	OUT84		; Umschaltung Farbebene (IX+1) und PORT1
	LD	A,(HL)
	AND	B		; Bit in Farbebene gesetzt?
	CALL	HIR3		; zurueck zu Pixelebene
	JR	Z,HIR5
	SET	1,D
HIR5:	INC	B		; Z=0 (Bitmaske)
	JR	HIR6		; Farbbyte uebergeben
	;
PUDEL:	LD	B,A		; Bitmaske
	LD	C,(HL)		; Pixel-RAM lesen
	CPL			; Bitmaske invertieren
	AND	C		; Bit loeschen
	LD	(HL),A		; und in Pixel-RAM schreiben
	LD	A,(IX+1)
	LD	E,A
	XOR	2
;	 DI
;	 OUT	(PORT1),A	; Umschaltung Farbebene
	CALL	OUT84		; Umschaltung Farbebene (IX+1) und PORT1
	LD	D,(HL)		; Color-RAM lesen
	LD	A,E
;	 OUT	(PORT1),A	; zurueck zu Pixelebene
;	 EI
	CALL	OUT84		; zurueck zu Pixelebene (IX+1) und PORT1
	LD	A,B		; Bitmaske
	AND	C		; Z=1: Punkt war geloescht
HIR6:	LD	A,D		; Farbbyte
PUDEE:	JP	POP3		; POP	BC,DE,HL   RET
;
; Grafikposition berechnen fuer Punktroutinen
; PE:	HL	X-Koordinate (Horizontalposition)
;	DE	Y-Koordinate (Vertikal-  "	)
; PA:	HL	IRM-Adresse Pixel/Farbe
;	D	Farbwert
;	A	Bit-Maske
;	CY	Fehler (ausserhalb)
; VR:	AF,DE,HL
;
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
	CP	40
	JR	NC,CALEN
	OR	80H		; IRM
	LD	H,A
	LD	A,0FFH
	ADD	A,D
	JR	C,CALEN
	XOR	E		; CY=0, kein Fehler
	LD	L,A		; Zeile
	LD	A,(FARB)
	LD	D,A
	LD	A,(BC)		; Bitmaske
	POP	BC
	RET
	;
CALEN:	POP	BC
	SCF			; CY=1, Fehler: ausserhalb
	RET
;
; MENU-HACK damit DISPLAY immer mit D abgekuerzt werden kann
;
	DW	7F7FH
	DB	'D '		; verstecktes Menuewort
	DB	3		; mindestens 1 Argument

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
DISP2:	CALL	DPMEMO		; 1 Zeile anzeigen
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
	;
DISP5:	CP	13H		; STOP?
	JR	NZ,DISP1
	JR	MODI1

; Anzeige/Veraendern Speicherbereich
; HL=Adresse, E=Anzahl Bytes pro Zeile
; A=Anzahl Argumente

MODIC:	LD	C,E		; Breite
	CP	2		; 2 Argumente?
	JR	NC,MODI1
	LD	C,1		; Standardbreite
MODI1:	CALL	DPMEMO		; 1 Zeile anzeigen
	LD	A,5
	LD	(CURSO),A
	CALL	INLIN		; Eingabe
	RET	C		; BRK
	PUSH	HL
	CALL	RHEX		; Adresse erfassen
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
	;
MODI5:	CP	':'		; Rueckschritt?
	JR	NZ,MODI6
	LD	B,0
	SBC	HL,BC		; C Bytes zurueck
	JR	MODI1
	;
MODI6:	CP	'/'		; neue Adresse?
	JR	NZ,MODI0
	INC	DE
	CALL	RHEX		; Adresse erfassen
	JR	C,MODI9		; Fehler
	LD	HL,(NUMVX)
	JR	MODI1
	;
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
	;
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
	JP	C,ERRM		; Modul nicht gefunden oder Parameterfehler
	LD	A,CR		; CR+LF drucken
;	CALL	OFILT
;	LD	A,LF
	JP	OFILT

LSTC1:	LD	A,(HCPZ)	; bisheriges Steuerbyte
	AND	0F0h		; Druckertyp uebernehmen
	RRCA			; und auf Bits 1..4 rotieren
	RRCA
	RRCA
	JR	LSTC6

; M021
LSTC2:	CALL	USRARG		; Steuerwort aus Argumenten bilden (CY war 0)
	XOR	A
	OUT	(91h),A		; M021 PIO-B Daten
	IN	A,(91h)
	ADD	A,-1		; 0 zurueckgelesen?
	RET	C		; sonst kein M021 vorhanden
	PUSH	BC
	LD	HL,M021PR
	JR	LSTC7

; M003:
LSTC3:	LD	L,7		; ab Steckplatz 7
LSTC4:	CALL	SIOARG		; Steuerwort aus Argumenten bilden
	PUSH	BC		; Adresse ShCLR-Reaktionsprogramm
	LD	B,L
	CALL	V24SU		; M003 suchen und einschalten
	LD	D,B		; Steckplatz in D zurueckgeben
	JR	C,IOEPOP	; nicht gefunden, keine weitere Initialisierung
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
LSTCA:	LD	H,A		; Anzahl Argumente
	LD	A,5
	CP	H		; (ARGN) <= 5?
	RET	C		; mehr als 5 Argumente!
	JR	NZ,LSTC1	; Druckertyp erhalten
LSTC5:	LD	A,(ARG5)	; neuer Druckertyp
	CP	13		; (ARG5) < 13?
	JR	NC,IOERET	; Fehler: Druckertyp nicht passend
	ADD	A,A		; 0 fuer Bit 3 einschieben
LSTC6:	LD	D,A
	LD	A,H		; Anzahl Argumente
	AND	A		; Steckplatz angegeben?
	JR	Z,LSTC3		; nein, M003 ab Steckplatz 7 suchen
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
	JR	NZ,IOERET	; Fehler: kein passendes Modul

; M001:
	CALL	USRARG		; Steuerwort aus Argumenten bilden (CY war 0)
	PUSH	BC		; Adresse ShCLR-Reaktionsprogramm
;	LD	A,2
;	LD	D,1
	CALL	MODUC1		; M001 einschalten
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
	LD	(UOUT1+1),HL
	LD	(UIN1+1),BC
	RET
	;
SETU2:	LD	(UOUT2+1),HL
	LD	(UIN2+1),BC
	RET

IOEPOP:	POP	HL
IOERET:	SCF			; Fehler-Flag
	RET

; Auswertung der Argumente 2..4 fuer I/O-Initialisierung:
;
; PE:	H	Argumentzahl
;	E	SIO-Kanal	(0..1 = A, 2 = B)
;	C	User-Kanal	(2 = UOUT1, 3 = UOUT2)
;	(ARG4)	ShCLR-Reaktion
;	D	bisheriges Steuerwort
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
	CP	H		; mehr als 1 Argument?
	JR	NC,NOARG2	; nein, SIO-Kanal nicht angegeben
	DEC	E		; (ARG2): SIO-Kanal
	SUB	E		; 1 <= (ARG2) <= 2?
	JR	C,IOEPOP	; Fehler: ungueltiger SIO-Kanal
	CP	E		; CY = 1 wenn K2
USRARG:	LD	E,D		; bisheriges Steuerbyte
	RL	E		; Bit 2 neu einschieben
	LD	A,2
	LD	D,A
	CP	H		; mehr als 2 Argumente?
	JR	NC,NOARG3	; nein. I/O-Kanal nicht angegeben
	LD	A,C		; (ARG3): User-I/O-Kanal
	SUB	D
	CP	D		; 2 <= (ARG3) <= 3?
	JR	NC,IOEPOP	; Fehler: ungueltiger I/O-Kanal
	RRCA			; CY = 1 wenn User-Kanal #3
	RL	E		; Bit 1 einschieben
;	LD	BC,NOOP
	INC	D		; D = 3
	LD	A,D
	CP	H		; mehr als 3 Argumente?
	JR	NC,NOARG4	; nein, ShCLR-Reaktion nicht angegeben
	LD	A,(ARG4)	; Reaktion auf ShCLR
	AND	A		; (ARG4) = 0?
	JR	Z,NOARG4
	SUB	D		; 0 <= (ARG4) <= 2?
	JR	NC,IOEPOP	; Fehler: ungueltiger ShCLR-Parameter
	LD	BC,HCPGM
	RRCA			; CY = 1 wenn Hardcopy
	RL	E		; Bit 0 einschieben
	RET

NOARG2:	LD	E,D		; bisheriges Steuerbyte
	RLC	E		; Bit 2 wie bisher erhalten
NOARG3:	RLC	E		; Bit 1 wie bisher erhalten
NOARG4:	RLC	E		; Bit 0 wie bisher erhalten
	LD	BC,NOOP		; keine Reaktion auf Sh-CLR
	AND	A		; CY=0
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
	RET	C		; mehr als 3 Argumente -> Fehler
	LD	A,(HCPZ2)	; bisheriges Steuerwort
	RRCA
	RRCA			; zur Vorbereitung 3x rotieren
	RRCA
	LD	D,A
	CALL	SIOARG		; neues Steuerwort aus Argumenten 2-4 bilden
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
	JR	Z,V24DC6	; nein
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
V24DC7:	CALL	V24SU		; M003 suchen und einschalten
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

; V.24-Modul suchen und initialisieren (bei POWER-ON und RESET):
;	-Tabellen in IRM kopieren
;	-M003 suchen und einschalten
;	-M003-Steckplatz merken

V24INI:	LD	HL,V24TAB	; vorbereitete V24-Tabelle
	LD	DE,V24PL	; Steckplatz (A800H)
	LD	BC,V24UMT - V24TAB
	LDIR			; in IRM kopieren
	XOR	A		; ARGN=0: M003 suchen
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
	LD	DE,ZEI3		; UP 03H = UOUT2
	JR	NZ,PROTO3
	LD	DE,ZEI2		; UP 02H = UOUT1
PROTO4:	LD	(UOUT1+1),HL
	JR	PROTO5
	;
PROTO2:	LD	HL,PRINT	; Protokoll deaktivieren
	LD	DE,OUTT1	; UP 00H = CRT
	CP	2
	JR	Z,PROTO4
PROTO3:	LD	(UOUT2+1),HL	; Sprungadresse der Routine
PROTO5:	LD	(OUTAB),DE	; und Zeiger setzen
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
K6313b:	LD	C,8		; 8 Nadeln
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
POP2:	POP	BC
	POP	HL
	RET

; C Bytes zum Puffer

CBYTES:	PUSH	HL
	PUSH	BC
	CALL	PADR
	LD	DE,CASS
	LD	B,0
	LDIR
	JR	POP2	; POP BC, HL	RET

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

; Byteausgabe von Datentraeger (BASIC-Schnittstelle)	**38**

; Zunaechst Puffer fuellen und nur wenn Puffer voll ist in die
; MBOUT-Routine(n) des eingestellten Treibers springen

; PE:	A	Datenbyte
;	D	Steuerbyte
;		Bit 3 = 1 INIT (Block 01)
;		Bit 6 = 1 Close (Block FF)
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
; PA:	CY=1	Fehler
; VR:	AF,DE,HL
;
MBOC:	LD	E,D		; Steuerbyte
	LD	D,A		; Datenbyte
	PUSH	DE
	PUSH	BC
	BIT	3,E		; Init?
	JR	Z,NOINIT
	SET	1,(IX+7)	; INIT merken
	LD	A,'U'+80H
	CP	(HL)		; List-Ausgabe?
	PUSH	DE
	LD	DE,160		; kurze Vortonlaenge normal
	JR	NZ,NOHIU
	LD	DE,1280		; bei 'U' gedehnt
NOHIU:	LD	(VORTN),DE
	CALL	BNAME		; Dateiname konvertieren/merken fuer ISRO
	CALL	CLC		; Kassettenpuffer loeschen
	CALL	DEV		; Kassette?
	JR	NZ,MBONN	; nein
	LD	BC,11		; Laenge Dateiname + Typ
	LDIR			; bei TAPE zunaechst mit Name fuellen
MBONN:	EX	DE,HL		; HL jetzt Zeiger in Puffer
	POP	DE
	LD	(HL),D		; 1. Datenbyte eintragen
	INC	L
MBOC1:	LD	A,L
MBOC0:	LD	(DTPTR),A	; Zeiger neu setzen
MBOCE:	AND	A		; CY=0 / kein Fehler (zusaetzlich ab CAOS 4.7)
MBCERR:	POP	BC
	POP	DE
	RES	3,E		; Init zuruecksetzen
	RET
	;
NOINIT:	LD	HL,(DTPTR)	; Zeiger holen
	LD	H,High(CASS)	; und mit Kassettenpuffer verbinden
	LD	(HL),D		; Datenbyte eintragen
	BIT	6,E		; Close?
	JR	NZ,MCLOS	; letzten Block schreiben
	INC	L		; Zeiger auf naechstes Byte
	JP	P,MBOC1		; Puffer noch nicht voll
	BIT	1,(IX+7)	; steht INIT noch aus?
	JR	NZ,MBOC2	; ja, dann erst einmal den Vorblock ausgeben!
	CALL	MBLO		; Datenblock ausgeben, Blocknummer anzeigen
	JR	C,MBCERR	; BRK oder Fehler -> IO-Error
MBOC6:	CALL	CLC		; Kassettenpuffer jetzt wieder loeschen
	JR	MBOC0		; Zeiger zuruecksetzen
	;
MBOC2:	RES	1,(IX+7)	; INIT ruecksetzen
	LD	HL,FNAME	; Dateiname fuer Ausgabe
	CALL	PV7
	DB	2		; Vorblock ausgeben mit ISRO
;	JR	C,MBCERR	; Fehler
	CALL	NC,MBLO1	; Blocknummer anzeigen, neue Zeile, BRK-Test
	JR	C,MBCERR	; BRK oder Fehler
	JR	MBOC6		; Zeiger auf leeren Puffer stellen
	;
MCLOS:	BIT	1,(IX+7)	; steht noch INIT aus?
	JR	Z,MBOC3		; nein, nur noch der Endeblock
	LD	HL,FNAME	; Dateiname fuer Ausgabe
	CALL	PV7
	DB	2		; sonst erst noch ISRO ausfuehren
	JR	MBOC4		; und danach sofort Close
;
; 17.02.2019
; Bei der byteweisen Ausgabe wurde bisher der Endeblock immer 2x ausgegeben.
; Dateien auf Diskette und USB-Laufwerken sind deshalb immer einen Block zu lang
;
; 24.11.2020
; Offenbar wurde dieses Verhalten mit Absicht eingebaut, denn BASIC benoetigt
; nach Einlesen des letzten Datenbytes eine gewisse "Bedenkzeit" bevor das Close
; zum Lesen des Endebytes 03h folgt. Wird also der erste Ende-Block nicht
; erwischt, dann folgt noch ein weiterer. Ohne diesen zusaetzlichen Block bleibt
; BASIC beim Lesen hengen und man muss zurueckspulen und den letzten Block
; nochmals abspielen.
; -> Also wird bei TAPE der Endeblock weiterhin 2x nacheinander ausgegeben.
;
MBOC3:	CALL	DEV		; Kassette?
	CALL	Z,MBLO		; bei TAPE letzten Block als Datenblock ausgeben
	RET	C		; BRK oder Fehler
MBOC4:	LD	BC,(VORTN)
	CALL	PV7
	DB	3		; Endeblock ausgeben mit CSRO
	PUSH	AF		; CY=1 bei Fehler
	CALL	CRLF
	POP	AF		; Fehlerflag
	JR	MBRES		; fertig
;
MBICL:	CALL	PV7
	DB	5		; Dateieingabe stoppen mit CSRI
MBRES:	RES	5,(IX+7)	; 'U'-Merker ruecksetzen (Blocknummern anzeigen)
	JR	MBCERR		; fertig

; Byteeingabe von Datentraeger (BASIC-Schnittstelle)	**37**

; Zunaechst Daten aus Puffer entnehmen und nur wenn Puffer leer ist in die
; MBIN-Routine(n) des eingestellten Treibers springen

; PE:	D	Steuerbyte
;		Bit 3 = 1 INIT (Block 01)
;		Bit 6 = 1 Close (Block FF)
;	HL	Name nur bei INIT (Adresszeiger 11 Byte)
; PA:	A	Datenbyte
;	CY=1	Fehler
; VR:	AF, DE, HL
;
MBIC:	LD	E,D		; Steuerbyte
;	LD	D,A		; Datenbyte (PA!)
	PUSH	DE
	PUSH	BC
	BIT	6,E		; Close?
	JR	NZ,MBICL	; ja, Bandeingabe stoppen
	BIT	3,E		; Init?
	JR	Z,MBIU		; nein, Datenbyte holen
; Init
	PUSH	HL		; Dateiname merken
;
; 29.03.2021:
; Dateityp hier schon testen zur Unterdrueckung bei BASIC-Listings *.UUU
; der Test nach dem Einlesen des ersten -blocks funktioniert nur bei TAPE.
;
	LD	A,(HL)		; vorangestellter Dateityp: 1. Zeichen testen
	CP	'U'+80H		; hohes 'U'?
	JR	NZ,MBIC0
	SET	5,(IX+7)	; merken zur Blocknummernausgabeunterdrueckung
				; beim Einlesen von ASCII-Listings
MBIC0:	CALL	BNAME		; Dateiname konvertieren/merken fuer ISRI
	LD	HL,FNAME	; konvertierter Dateiname
	CALL	PV7
	DB	4		; Block 01h einlesen mit ISRI
MBIC1:	POP	HL		; gesuchter Dateiname fuer Vergleich
MBIC2:	JP	C,JBLERR	; Lesefehler: '*'
	LD	A,(IX+2)	; gelesener Block
	DEC	A
	JP	NZ,JBLERR	; falsche Blocknummer: '*'
	RES	7,(IX+7)	; Kopfblock ist geladen
	INC	(IX+3)		; naechster erwarteter Block
	LD	DE,CASS		; Daten stehen jetzt im Kassettenpuffer
	CALL	DEV		; Kassette?
	JR	NZ,MBINN	; nein, sofort Datenbyte entnehmen
	
; Dateiname anzeigen/vergleichen
	LD	B,11		; Laenge des Dateinamens bei Tape im Kassettenpuffer
	LD	A,(DE)		; vorangestellter Dateityp: 1. Zeichen testen
;	CP	'U'+80H		; hohes 'U'?
;	JR	NZ,MBIC3
;	SET	5,(IX+7)	; merken zur Blocknummernausgabeunterdrueckung
;				; beim Einlesen von ASCII-Listings
MBIC3:	ADD	A,29H		; geschuetztes Programm?
	JR	NC,MBIC4
	LD	A,(DE)		; geschuetzte BASIC-Programme
	SUB	4		; als ungeschuetzt anzeigen
	LD	(DE),A		; 1. Zeichen vom Dateityp
	INC	DE
	LD	(DE),A		; 2. Zeichen vom Dateityp
	INC	DE
	LD	(DE),A		; 3. Zeichen vom Dateityp
;	LD	(DATFLG),A	; <=hier LIST/EDIT/BYE-Schutz setzen
;	SET	6,(IX+7)	; Das war ein RESET-Schutz bei CAOS 3.1
	DEC	DE
	DEC	DE		; DE wieder auf Anfang Kassettenpuffer
MBIC4:	LD	A,(DE)		; Name aus Kassettenpuffer
	CALL	OCHR		; anzeigen
	CP	(HL)		; stimmt Name?
	JR	Z,MBIC5
	SET	7,(IX+7)	; nein-merken
MBIC5:	INC	HL
	INC	DE
	DJNZ	MBIC4
	CALL	CRLF
	BIT	7,(IX+7)	; war der Name korrekt?
	SCF			; Fehlerflag
	JP	NZ,MBCERR	; Name falsch -> ?IO ERROR
MBINN:	EX	DE,HL
MBIC6:	LD	D,(HL)		; Datenbyte aus Kassettenpuffer entnehmen
	INC	L		; Zeiger auf naechstes Byte
	LD	A,L
	LD	(DTPTR),A	; Zeiger neu setzen
	LD	A,D		; Datenbyte in A bereitstellen
	JP	MBOCE		; fertig

; Datenbyte holen

MBIU:	LD	HL,(DTPTR)	; Zeiger
	LD	H,HIGH(CASS)	; in Kassettenpuffer
	BIT	7,L		; Pufferende erreicht?
	JR	Z,MBIC6		; Daten aus Puffer entnehmen
MBIU1:	CALL	PV7
	DB	1		; einen Block lesen mit MBI
	JR	C,MERR		; Lesefehler
	LD	A,0FFH
	CP	(IX+2)		; Block FF?
	JR	Z,MBIU3		; FF immer als korrekt durchgehen lassen
	LD	A,(IX+3)
	CP	(IX+2)		; erwarteter Block?
	JR	Z,MBIU3		; ja, stimmt!
	CALL	BLNOT		; nicht erwarteter Block mit * anzeigen
MBIU2:	CALL	BRKT		; Abbruch mit BRK?
	JR	NC,MBIU1	; weiter versuchen
JMBERR:	JP	MBCERR		; BRK => IO-ERROR
	;
MBIU3:	INC	(IX+3)		; naechsten Block erwarten
	DEC	A
	BIT	5,(IX+7)	; Blocknummer unterdruecken?
	CALL	Z,LUP5		; Blocknummer ausgeben, ">" anhaengen und zum
				; Zeilenanfan gehen
MBIU4:	LD	HL,CASS		; Pufferbeginn
	JR	MBIC6		; jetzt Datenbyte entnehmen

; Lesefehler
MERR:	CALL	DEV		; Kassette?
	JR	NZ,JBLE1	; nein, Abbruch bei anderen Devices
	CALL	BLERR		; Blocknummer mit ? anzeigen
	CALL	CRLF		; neue Zeile
;	CALL	OSTR
;	DB	9,9,9,9,'?',0	; Fehler im Block
;	CALL	BNROST		; mit Blocknummer anzeigen
;	DB	1EH,0		; neue Zeile
	JR	MBIU2		; BRK testen, sonst weiter versuchen

; falsche Blocknummer oder Lesefehler beim Kopfblock
JBLERR:	CALL	DEV		; Kassette?
JBLE1:	SCF			; Fehlerkennung
	JR	NZ,JMBERR	; Abbruch wenn nicht Kassette
	CALL	BLNOT		; nicht erwartete Blocknummer mit * anzeigen
;	CALL	OSTR
;	DB	'*',8,0		; nicht erwarteter 1. Block
;	CALL	BRKT
	JR	C,JMBERR	; BRK => IO-ERROR
	CALL	PV7
	DB	1		; Block erneut lesen mit MBI
	JP	MBIC2

; Datenblock ausgeben innerhalb der Byte-Routinen
; PA:	CY=1	Fehler

MBLO:	LD	BC,(VORTN)	; Vortonlaenge
	CALL	PV7
	DB	0		; Block ausgeben mit MBO
	RET	C		; Fehler
MBLO1:	BIT	5,(IX+7)	; Blocknummern unterdruecken?
	RET	NZ		; ja
	JP	SUP5		; Blocknummer anzeigen, neue Zeile, BRK-Test

; Tabelle der Modulnamen:

MTAB:	DB	000h,'VARIABEL',0	; M042 (Tim Henning)
	DB	001H,'START-ROM',0	; M033 z.B. Typestar
	DB	070H,'32K ROM',0	; M045
	DB	071H,'64K ROM',0	; M046
	DB	072H,'128K ROM',0	; M047
	DB	073H,'256K ROM',0	; M048 (Mario Leubner)
	DB	074H,'512K ROM',0	; M049 (Andreas Schlechte)
;	DB	075H,'1M ROM',0	;	 M050 reserviert!
	DB	076H,'4M ROM',0		; M044 (Bert Lange, Andreas Schlechte)
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
	DB	0BFH,'MT02',0		; MT02 (Nachbau Pruefmodul MT01)
;	DB	0D8h,'Z8E',0		; M127 (reserviert fuer Thomas Schramm)
	DB	0D9H,'EPROMER 32K',0	; M030 (fuer 2k-32k EPROMs)
;	DB	0DAH,'PIO-3',0		; M002 (geplantes Modul mit 3*PIO)
	DB	0DAH,'WEATHER',0	; M004 (Wetter-Modul, Rene Nitzsche)
	DB	0DBH,'EPROMER 64K',0	; M030 (fuer 1k-64k EPROMs)
	DB	0DCH,'SOUND',0		; M066 (Soundmodul, Rene Nitzsche)
	DB	0DFH,'MP3',0		; M366 (MP3-Modul, Rene Nitsche)
	DB	0E3H,'DAU1',0		; M029
	DB	0E7H,'ADU1',0		; M010
	DB	0ECH,'SCANNER',0	; M051 (KC-Club)
	DB	0EEH,'V.24',0		; M003, M053
	DB	0EFH,'DIGITAL I/O',0	; M001
	DB	0F0H,'8K CMOS-RAM',0	; M120 (Nachbau Thomas Schramm)
	DB	0F1H,'16K CMOS-RAM',0	; M122
	DB	0F2H,'32K CMOS-RAM',0	; M124
;	DB	0F3H,'8*8K EPROM',0	; M062-RAM/ROM       (Sonderbehandlung!)
	DB	0F4H,'16K RAM',0	; M022
	DB	0F5H,'32K RAM',0	; M024
	DB	0F6H,'64K RAM',0	; M011
	DB	0F7H,'8K ROM',0		; M025, M040
	DB	0F8H,'16K ROM',0	; M028, M040, M041
	DB	0F9H,'GIDE',0		; M064 (Mario Leubner, 2016)
;	DB	0FBH,'8K SOFTWARE',0	; M012, M026, M027   (Sonderbehandlung!)
	DB	0FCH,'BASIC',0		; M006
;	DB	0FDH,'USB+NET',0	; M052               (Sonderbehandlung!)
	DB	0FFH,'???',0		; unbekanntes Modul
MTAB1:	DB	'SOFTWARE'		; Standardanzeige falls ohne Menuewort
MTAB2:	DB	0

MUSER:	DB	'USER',0		; M005 (USER-Module C0-D7)
	;
M052A:	DB	'NET+'			; M052 (USB+Netzwerk, 32K EEPROM 4 * 8K)
M052B:	DB	'USB',0			; M052 (nur USB, 8K EEPROM)
	;
M062A:	DB	'4*8K RAM',0		; M062-RAM (E. Mueller)
M062B:	DB	'8*8K ROM',0		; M062-ROM (E. Mueller)

;	ABSFILL	0D000h,<ROM-C-Ende>
