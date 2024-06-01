;-------------------------------------------------------;
;	EDAS Arbeitszellen (fuer EDAS 1.7 / 2.0)	;
;	----------------------------------------	;
;	(und CAOS-Editor, Assembler und Reassembler)	;
;							;
; EDAS.INC		ML-Soft 21.01.2018-04.03.2020	;
;-------------------------------------------------------;

NAME0	EQU	0000H		; Dateiname 8 Zeichen, Typ 3 Zeichen
;MODE	EQU	000Bh		; M052: 1 Byte fuer Interrupt-Mode USB
;DATUM	EQU	000Ch		; M052: 4 Byte fuer Datum

NAMP	EQU	0B7F5h		; Namenspuffer der geladenen (Quell-)Textdatei
				; 11 Zeichen bis 0B7FFh

RANDL	EQU	0010h		; Position linker Rand
RANDR	EQU	RANDL+1		; Position rechter Rand
MEML	EQU	RANDR+1		; Zeilennummer von Textanfang an
FTXT	EQU	MEML+2		; Zeiger auf Suchtext bei REPLACE
RTXT	EQU	FTXT+2		; Zeiger auf Ersetz-Text bei REPLACE

T1A	EQU	RTXT+2		; Beginn Text 1
T2E	EQU	T1A+2		; nach Ende Text 2
T1E	EQU	T2E+2		; nach Ende Text 1
T2A	EQU	T1E+2		; Beginn Text 2

COL	EQU	T2A+2		; EDIT-Cursor (Spalte)
LIN	EQU	COL+1		; EDIT-Cursor (Zeile)
M4A	EQU	LIN+1		; wenn ungleich 0 -> no Memory
STRING	EQU	M4A+1		; String-Puffer
LSTR	EQU	27		; max. Laenge Stringpuffer (26 Zeichen + 00)
STAT	EQU	STRING+LSTR	; Merkzelle fuer verschiedene Einstellungen
		;Bit 0 = 1 - Floppy vorhanden
		;Bit 1 = 1 - DEP mit LW-Steuerung
		;Bit 2 = 1 - Fehler bei Diskette
		;	     oder MTAB-Ueberschreitung aufgetreten
		;Bit 3 = 1 - Option 'S' merken
		;Bit 4 = 1 - Ueberschreiben (0=Einfuegen)
		;Bit 5 = 1 - Zeichensatz (0=IBM, 1=deutsch)
		;Bit 6 = 1 - Zeichensatz (0=IBM, 1=CAOS)
		;Bit 7 = 1 - END-Befehl erkannt (ASM)

; Assembler-Arbeitszellen:

OPT	EQU	STAT+1		; Assembler-Optionen
		;Bit 0	L - Listing ausgeben
		;Bit 1	P - Umschaltung Drucker
		;Bit 2	+ - Markentabelle erweitern
		;Bit 3	1 - nur PASS 1!
		;Bit 4	3 - PASS 3 laeuft (Save MC)
		;Bit 5	B - Bildschirmformat (40 Spalten)
		;Bit 6	O - MC in Speicher laden
		;Bit 7	2 - nur PASS 2
OFFS	EQU	OPT+1		; Assembler-Offset
OBJ	EQU	OFFS+2		; Beginn Maschinencode
MBOT	EQU	OBJ+2		; RAM-Ende+1	8000H oder C000H
MTOP	EQU	MBOT+2		; Marken-Tabelle (Anfang)
PBL	EQU	MTOP+2		; Pointer auf Byte-Liste
M78	EQU	PBL+2
M79	EQU	M78+1
MPC	EQU	M79+1		; Adressenzaehler fuer MC-Erzeugung
ERRNO	EQU	MPC+2		; Zaehler (BCD) fuer Fehler beim Assemblieren
MCL	EQU	ERRNO+2		; MC-Endadresse
NUMB	EQU	MCL+2		; + Berechneter Wert eines Ausdrucks (Operand)
DDFD	EQU	NUMB+2		; | DD/FD-Speicher
KLAM	EQU	DDFD+1		; | Klammer-Speicher
M84	EQU	KLAM+1		; | Merker, ob Operand gefunden wurde
VORZ	EQU	M84+1		; | Rechenzeichen-Speicher
REGNR	EQU	VORZ+1		; + Registerkennbyte
REGC	EQU	REGNR+1		; Registercode
	; Registerkennbyte	Registercode	DDFD
	;				Bit
	; A	00H		07H	8
	; B	01H		00H	8
	; C	02H		01H	8
	; D	03H		02H	8
	; E	04H		03H	8
	; H	07H		04H	8
	; HX	07H		04H	8	DD
	; HY	07H		04H	8	FD
	; L	0BH		05H	8
	; LX	0BH		05H	8	DD
	; LY	0BH		05H	8	FD
	; I	08H		47H	8
	; R	11H		4FH	8
	; M(HL)	3AH		06H	8
	; (IX+)	3AH		06H	8	DD	
	; (IY+)	3AH		06H	8	FD
	; BC	2BH		00H	16
	; (BC)	2BH		00H	8
	; DE	2FH		10H	16
	; (DE)	2FH		10H	8
	; HL	3AH		20H	16
	; IX	3AH		20H	16	DD
	; IY	3AH		20H	16	FD
	; SP	49H		30H	16
	; AF	2DH		31H	16

; Hilfsprogramme fuer Speicherzugriff hinter IRM und ROM-C bis DFFFh

RAMC	EQU	REGC+1		; Hilfsprogramme fuer Speicherzugriff bis DFFFh
	; 005Ch bis 00A0h	  nur bei ext. Editor erforderlich

; Assemblierpuffer:	(hier steht die erzeugte Quelltextzeile)

NERR	EQU	0A880h		; 1 Byte Assembler-Fehlercode
				; 1 Byte "Stern" wenn Fehlercode vorhanden ist
ADR	EQU	NERR+2		; 4 Byte fuer Adresse
				; 10 Byte fuer Hexcodes
TEXT	EQU	NERR+16		; Quelltext (max. 80 Byte bei ext. Editor)
	; A880h bis A8FFh
