;*****************************************
;**	CAOS 4.6			**
;**					**
;**	Macros und Vereinbarungen	**
;**					**
;**	letzte Aenderung: 05.03.2016	**
;*****************************************

MESSAGE	MACRO	MSG1,N,MSG2
	 .PRINTX MSG1 N MSG2
	ENDM

; Mit FF auffuellen, bis die absolute Adresse LOC erreicht ist:
ABSFILL	MACRO	LOC,WHAT
	 IF $ GT LOC
	  MESSAGE <Fehler! LOC:>,% $ - LOC,<Byte zu viel vor WHAT>
	 ELSE
	  IF2
	   MESSAGE <LOC:>,% LOC - $,<Byte frei vor WHAT>
	  ENDIF
	  DS	LOC - $,0FFh
	 ENDIF
	ENDM

; Undokumentierter Opcode:
;	IN	F,(C)
INFC	MACRO
	 DB	0EDh,70h
	ENDM

; Undokumentierter Opcode:	Hinweis von KaiOr:
;	OUT	(C),0		ED 71 ------- OUT (C),f
;				bewirkt:
;				NMOS-Z80 ---> OUT (C),0
;				CMOS-Z80 ---> OUT (C),255
;				Z380 -------> OUT (C),n -> Ausg. des nachfolgenden Bytes
;
;OUTC0	MACRO
;	 DB	0EDh,71h
;	ENDM
; nicht mehr verwendet ab 05.03.2016 !

; Undokumentierter Opcode:
;	RES	BI,(IX+DISP),A
RESIXA	MACRO	BI,DISP
	 DB	0DDh,0CBh,DISP
	 DB	BI SHL 3 OR 10000111b
	ENDM

; Undokumentierter Opcode:
;	SET	BI,(IX+DISP),A
SETIXA	MACRO	BI,DISP
	 DB	0DDh,0CBh,DISP
	 DB	BI SHL 3 OR 11000111b
	ENDM

; Setze Bit 7 des ersten Zeichens fuer BASIC-Token:
TOKEN	MACRO	STRING
Y	 DEFL	80h
	 IRPC	X,STRING
	  DB	'&X' + Y
Y	  DEFL	0
	 ENDM
	ENDM

;---------------------------------------

; Portadressen im Grundgeraet:

PIOAD	EQU	88H	; PIO-A, Datenbyte
PIOBD	EQU	89H	; PIO-B, Datenbyte
PIOAS	EQU	8AH	; PIO-A, Steuerbyte
PIOBS	EQU	8BH	; PIO-B, Steuerbyte
CTC0	EQU	8CH	; CTC Kanal 0
CTC1	EQU	8DH	; CTC Kanal 1
CTC2	EQU	8EH	; CTC Kanal 2
CTC3	EQU	8FH	; CTC Kanal 3

; von den BASIC-Erweiterungen genutzte Arbeitszellen im RAM:

IOCHNL	EQU	0307H	; Ein-/Ausgabekanal
ININD	EQU	0309H	; IN-Index
RNDVR3	EQU	031BH	; Zufallszahl
ANF	EQU	034EH	; erste Zeilennummer
ENDE	EQU	0350H	; letzte Zeilennummer
NANF	EQU	0352H	; neue erste Zeilennummer
DISTAN	EQU	0354H	; Zeilennummernabstand
DATFLG	EQU	035EH	; List-, Editierschutz wenn <> 0
PSTBEG	EQU	035FH	; Programmstartadresse (Standardwert = 0401H)
COLRET	EQU	037EH	; Ablage Farbbyte
STRDAT	EQU	03C0H	; 4 Byte: Laenge und Adresse des aktuellen Strings
SVARPT	EQU	03D7H	; Adresse der Liste der einfachen und Stringvariablen
INTPRB	EQU	03EAH	; Druckpuffer=Dateiname
PRTFLG	EQU	03FDH	; PRINT-Erweiterung

; von den BASIC-Erweiterungen genutzte Routinen aus dem BASIC-ROM:

PRIST	EQU	0C00DH	; BASIC-Interpreter Kaltstart
SECST	EQU	0C08CH	; BASIC-Interpreter Warmstart
TMEMO	EQU	0C327H	; Test, ob RAM fuer naechste Operationen reicht
SNER	EQU	0C348H	; Sprung zu SN-ERROR
ERROO	EQU	0C356H	; Ausgebe Fehlercode
LIN13	EQU	0C442H	; Zeiger auf Zeile mit naechstgroesserer Nummer stellen
ULER	EQU	0C44DH	; Sprung zu UL-ERROR
LIN6	EQU	0C450H	; Zeile in Programm einsortieren	
LIN10	EQU	0C48AH
LIN11	EQU	0C493H
ZPOIT	EQU	0C4BBH	; Zeiger auf Anfang naechste Zeile stellen
ZPOIT1	EQU	0C4BEH
CPREG	EQU	0C689H
DLI22	EQU	0C8B7H	; Ansprung Tokenverarbeitung
TCHAR	EQU	0C8BDH	; naechstes Zeichen holen
TCHAR1	EQU	0C8BEH
CPSTX	EQU	0C8CCH	; erwartetes Zeichen folgt als DB ..
CPCOMM	EQU	0C8D6H	; Komma?
CPBRGT	EQU	0C8DBH	; Klammer zu?
FCER	EQU	0C967H	; Sprung zu FC-ERROR
EPRVL3	EQU	0C96FH	; Wert bestimmen
EPRVL4	EQU	0C96CH
DCHEX	EQU	0C986H	; Zeilennummer pruefen und in Hexzahl wandeln
DCHEX1	EQU	0C987H
PRINT2	EQU	0CB03H
SNALY6	EQU	0CD36H
SNALY	EQU	0CD3AH
SNLY14	EQU	0CDE1H	; Syntax-Check '('
SNLY16	EQU	0CDF3H	; Syntax-Check ')'
POS1	EQU	0D0C0H
STADTB	EQU	0D17BH
SADTB1	EQU	0D17EH
SLEN3	EQU	0D1A9H
STROP	EQU	0D1E1H	; String-Arithmetik
STRMV1	EQU	0D2F2H
STRZS3	EQU	0D302H
LEN1	EQU	0D330H	; Laenge holen
ARGVL1	EQU	0D421H	; Wert abholen
ARGVL2	EQU	0D424H
SGN1	EQU	0D6AEH
NUMKON	EQU	0D834H
CASS1	EQU	0DCB2H	; Dateiname aufbereiten
TESTCO	EQU	0DDC8H
CO	EQU	0DDD5H	; Ausgabe eines Zeichens (ASCII)
CI	EQU	0DDE4H	; Eingabe eines Zeichens (ASCII)
OUTCHL	EQU	0DE25H	; Ausgabe zu Kanal
INCHNL	EQU	0DE5FH	; Eingabe von Kanal

; Steuerzeichen:

LF	EQU	0AH
CR	EQU	0DH
ESC	EQU	1BH

; IRM-Belegung:

V24PL	EQU	0A800H	; Steckplatz V.24-Modul
;INTV1		0A801H	; Initialisierungsdaten V.24-Druckausgabe
;INTV2		0A809H	; Initialisierungsdaten V.24-duplex

JOYIRM	EQU	0A894H	; Joystick-Tabelle (noch nicht endgueltig)
	; bis	0A89Fh
;		0A8A0H	; Ablage IRM-Inhalt unter Mauspfeil (96 Byte)
	; bis	0A8FFH

DEVTAB	EQU	0A900H	; Tabelle der 8 Geraetetreiber: 8*32=256 Byte gross
SUT_IRM	EQU	0AA00H	; SUTAB per Default im IRM
	; bis	0AA91h
VRAM1	EQU	0AD00H	; ASCII-RAM Bild 1
VRAM0	EQU	0B200H	; ASCII-RAM Bild 0
CASS	EQU	0B700H	; 128 Byte Kassettenpuffer
	; bis	0B77FH
ARGC	EQU	0B780H	; Argument-Nr.
ARGN	EQU	0B781H	; Anzahl Argumente
ARG1	EQU	0B782H	; Argument 1
ARG2	EQU	0B784H	;    "	   2
ARG3	EQU	0B786H	;    "	   3
ARG4	EQU	0B788H	;    "	   4
ARG5	EQU	0B78AH	;    "	   5
ARG6	EQU	0B78CH	;    "	   6
ARG7	EQU	0B78EH	;    "	   7
ARG8	EQU	0B790H	;    "	   8
ARG9	EQU	0B792H	;    "	   9
ARG10	EQU	0B794H	;    "	  10
NUMNX	EQU	0B796H	; Anzahl Zeichen
NUMVX	EQU	0B797H	; Wert HEX-Zahl
HCADR	EQU	0B799H	; Hardcopy-Aufruf
WINNR	EQU	0B79BH	; Fenster-Nr.
WINON	EQU	0B79CH	; Fenster-Anfang
WINLG	EQU	0B79EH	; Fenster-Groesse
CURSO	EQU	0B7A0H	; Cursor-Position
STBT	EQU	0B7A2H	; BS-Steuerbyte
COLOR	EQU	0B7A3H	; Farbe
WEND	EQU	0B7A4H	; Adresse fuer Scrolling-Routine
CCTL0	EQU	0B7A6H	; Zeichentabelle 0
CCTL1	EQU	0B7A8H	;	"	 1
CCTL2	EQU	0B7AAH	;	"	 2
CCTL3	EQU	0B7ACH	;	"	 3
SYSP	EQU	0B7AEH	; System Stackpointer
SUTAB	EQU	0B7B0H	; UP-Tabelle
CTAB	EQU	0B7B2H	; CRT-Steuerprogramm
BJUMP	EQU	0B7B4H	; BASIC-Sprung
OUTAB	EQU	0B7B9H	; Zeiger Ausgabe
INTAB	EQU	0B7BBH	; Zeiger Eingabe
UOUT1	EQU	0B7BDH	; Sprung OUT#2
UIN1	EQU	0B7C0H	; Sprung IN#2
UOUT2	EQU	0B7C3H	; Sprung OUT#3
UIN2	EQU	0B7C6H	; Sprung IN#3
IOERR	EQU	0B7C9H	; BASIC-Error
VRAM	EQU	0B7CBH	; Adresse Video-RAM
ZOTAB	EQU	0B7CDH
ZWEND	EQU	0B7CFH	; WEND-Merker
FTAST	EQU	0B7D1H	; F-Tastenpointer
HOR	EQU	0B7D3H	; Grafik Horizontalposition
VERT	EQU	0B7D5H	; Grafik Vertikalposition
FARB	EQU	0B7D6H	; Grafik Farbe
MIXIT	EQU	0B7D7H	; IX-Merker
VORTN	EQU	0B7D8H	; Vortonlaenge fuer byteweise Kassettenausgabe
DTPTR	EQU	0B7DAH	; CASS-Pointer
CTCMD	EQU	0B7DBH	; CTC2-Modus
BLINK	EQU	0B7DCH	; Blinkfrequenz
L3TAB	EQU	0B7DDH	; ESC-Tabelle
L3SIZ	EQU	0B7DFH	; Anzahl ESC-Routinen
COUNT	EQU	0B7E0H	; Tastatur-Repeat
HCPZ	EQU	0B7E1H	; Druckersteuerbyte
		; Bit 0=0	Protokollfunktion
		; Bit 0=1	Hardcopyfunktion
		; Bit 1=0/1	USER I/O-Kanal 1/2
		; Bit 2=0/1	SIO-Kanal A oder B
		; Bit 3		-
		; Bit 4..7	Druckertyp (0 bis F)
		; Bit 7=0	Matrixdrucker
		; Bit 7=1	Schreibmaschine
INTV1	EQU	0B7E2H	; Druckerinitialisierung
INTV1L	EQU	0B7E4H	;	"  -Laenge
INTV2	EQU	0B7E5H	; Duplexinitialisierung
INTV2L	EQU	0B7E7H	;	"  -Laenge
HCPZ2	EQU	0B7E8H	; Duplexsteuerbyte
		; Bit 0		-
		; Bit 1=0/1	USER I/O-Kanal 1/2
		; Bit 2=0/1	SIO-Kanal A oder B
		; Bit 3		WR5 Bit 3: Senden ein/aus
		; Bit 4		-
		; Bit 5,6	WR5 Bit 5,6: 5-8 Bit pro Zeichen (Senden)
		; Bit 7		WR3 Bit 7: 7/8 Bit pro Zeichen (Empfang)
OFILT	EQU	0B7E9H	; Druckroutine (ungefiltert)
PROMPT	EQU	0B7ECH	; Systemprompt
LINTYP	EQU	0B7EDH	; Linientyp
CUMUST	EQU	0B7EEH	; ^Cursormuster
JOYTAB	EQU	0B7F0H	; Joystick-Tastencodes
;		0B7F2H
;		bis	; reservicert fuer Arbeitszellen von REAS und TEMO
;		0B7FFH
MODST	EQU	0B800H	; Modulsteuerbytespeicher
	; bis	0B8FFH

WNDFN	EQU	0B99CH	; Fenstersvektorpeicher
	; bis	0B9FFH

; Portadressen des Koppel-RAM (D004):

ERRTX	EQU	000F1H	; Fehlertext			(ab DEP 2.0)
STBYT2	EQU	080F1H	; 2. Steuerbyte:		(ab DEP 2.0)
			;	Bit 0 - Anforderung
			;	Bit 1 - Laufwerkswechsel
			;	Bit 2 - Userwechsel
			;	Bit 3..6 reserviert
			;	Bit 7 - Fehlerrueckmeldung
LWANF	EQU	081F1H	; Laufwerk ('A', 'B', ... 'P')
UANF	EQU	082F1H	; User-Bereich (00H ... 0FH)
DEPVER	EQU	083F1H	; DEP-Version BCD
DATE	EQU	084F1H	; Datum BCD			(ab DEP 3.0)
			;	Jahr (00..99)
			;	Monat (1..12)
			;	Tag (1..31)
TIME	EQU	087F1H	; Uhrzeit BCD			(ab DEP 3.0)
			;	Stunde (0..23)
			;	Minute (0..59)
			;	Sekunde (0..59)

FDATA	EQU	080F2H	; Daten- bzw. Sektorpuffer 128 Byte lang
STBYT1	EQU	080F3H	; 1. Steuerbyte:
			;	Bit 0 - Anforderung
			;	Bit 1 - 1=schreiben/0=lesen
			;	Bit 3 - oeffnen
			;	Bit 6 - schliessen
			;	Bit 7 - Fehlerrueckmeldung
			;	Bit	5 4 2	Funktionsauswahl:
			;		0 0 0	Sektor lesen/schreiben
			;		0 0 1	DIR-Anforderung
			;		0 1 0	ERA
			;		0 1 1	STAT
			;		1 0 0	REN
			;		1 0 1	SETRO
			;		1 1 0	SETWR
			;		1 1 1	BASIC-Byte lesen/schreiben
FCODE	EQU	081F3H	; Fehlercode
DIRBUF	EQU	082F3H	; Puffer fuer Dateiname oder Directory
BASBYT	EQU	08EF3H	; Datenbyte fuer BASIC-Byte-Schnittstelle
UROK	EQU	0B3F3H	; Betriebsart

; Belegung der IX - Zellen:

; IX+0	Zeitkonstante fuer Kassettenroutinen
; IX+1	Merkzelle fuer Ausgabeport 84H
; IX+2	gelesene Blocknummer (Kassette)
; IX+3	erwartete Blocknummer (Kassette)
; IX+4	Merkzelle fuer Ausgabeport 86H
; IX+5	Ein- Ausgabepufferadresse (low)
; IX+6	Ein- Ausgabepufferadresse (high)
; IX+7	Kennbyte fuer LOAD/VERIFY
;	 BIT 0: =1 ->LOAD  =0 ->VERIFY
;	 BIT 1: =1 kein Selbstart bei LOAD (in BASIC Kennung fuer 1. Block)
;	 BIT 2-4 : Anzahl Argumente (bei SAVE)/ARGN>3 (Startoffset)
;	 BIT 5-7 : BASIC: fuer CLOAD/CSAVE 
; IX+8	Kennbyte Tastatur
;	 BIT 0: gueltiges Byte (ASCII) in IX+13
;	 BIT 1: Tonausgabe laeuft
;	 BIT 2-4: aktuelles Speichergeraet
;		  0=Kassette, 1=Diskette, 2=USB (M052), 3=Netzwerk (M052)
;		  4=Headersave, 5..7 frei
;	 BIT 5: Tastenclick ein/aus
;	 BIT 6: Code kommt von F-Taste
;	 BIT 7: 0=Shiftlock
; IX+9	Prologbyte (normal 7FH)
; IX+10	Autorepeatzaehler (Tastatur)
; IX+11	Merkzelle fuer Akku (bei BWS on/off mit Stackverlagerung)
;	und Pruefsummenberechnung fuer MB-Eingabe
; IX+12	7-Bit-Scancode von Tastatur
; IX+13	ASCII-Code von Tastatur
; IX+14	Tastaturcodetabelle low
; IX+15	Tastaturcodetabelle high
