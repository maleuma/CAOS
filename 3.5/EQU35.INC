;*****************************************
;**	CAOS 3.5			**
;**					**
;**	Macros und Vereinbarungen	**
;**					**
;**	letzte Aenderung: 17.07.2026	**
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

; Setze Bit 7 des ersten Zeichens fuer BASIC-Token:
TOKEN	MACRO	STRING
Y	 DEFL	80h
	 IRPC	X,STRING
	  DB	'&X' + Y
Y	  DEFL	0
	 ENDM
	ENDM

;---------------------------------------

; Steuerzeichen:

LF	EQU	0AH
CR	EQU	0DH
ESC	EQU	1BH

;---------------------------------------

; Portadressen im Grundgeraet:

PIOAD	EQU	88H	; System-PIO Datenbyte Port A
			;	Bit 0 - CAOS ROM-E ein/aus
			;	Bit 1 - RAM0 ein/aus
			;	Bit 2 - IRM ein/aus
			;	Bit 3 - Schreibschutz RAM0
			;	Bit 4 - CAOS-ROM 3.5 oder 3.1 (/NMI)
			;	Bit 5 - System-LED
			;	Bit 6 - Motorschaltspannung
			;	Bit 7 - USER-ROM ein/aus
PIOBD	EQU	89H	; System-PIO Datenbyte Port B
			;	Bit 0 \
			;	Bit 1  \
			;	Bit 2   - Lautstaerke Tonausgabe
			;	Bit 3  /  5 Bit (Low-aktiv)
			;	Bit 4 /
			;	Bit 5 - USER-ROM-Ebene Bit 0
			;	Bit 6 - USER-ROM-Ebene Bit 1
			;	Bit 7 - Blinken ein/aus
PIOAS	EQU	8AH	; System-PIO Steuerbyte Port A
PIOBS	EQU	8BH	; System-PIO Steuerbyte Port B
CTC0	EQU	8CH	; CTC Kanal 0 (Tonhoehe 1)
CTC1	EQU	8DH	; CTC Kanal 1 (Kassettenausgabe)
CTC2	EQU	8EH	; CTC Kanal 2 (Tondauer)
CTC3	EQU	8FH	; CTC Kanal 3 (Tastatur)

;---------------------------------------

; Belegung Systembereich im RAM0:

; 0000-000Ah	Dateiname
; 000Bh		Interrupt-Mode fuer USB (von M052-Software verwaltet)
; 000C-000Fh	Datums-Stempel fuer Dateifunktion von USB (MS-DOS-Format)
; 0010-00ADh	Arbeitszellen Systemsoftware
; 00AE-00E0h	Interrupt-Routine von USB-Modul
; 00E1-0107h	Arbeitszellen Systemsoftware
; 0108-012Fh	Tasten-UP von CALC (bei Aktivierung mit %?i)
; 0180-019Ch	temporaere Ladeadresse der Datei *.JOY ab CAOS 4.6
; 0180-01C3h	Systemstack

STACK	EQU	01C4H	; Systemstack

; 01C4-01EFh	Interrupttabelle
; 01F0-01FFh	IX-Arbeitszellen (Standard: IX=01F0h)

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
;	 BIT 5: Blocknummern 0=anzeigen, 1=unterdruecken
;	 BIT 6-7 : BASIC: fuer CLOAD/CSAVE 
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

; IRM-Belegung:

;	EQU	8000H	; PIXEL-RAM
;	EQU	0A800H	; COLOR-RAM
CASS	EQU	0B700H	; 128 Byte Kassettenpuffer (bis	0B77FH)
ARGC	EQU	0B780H	; UP-Nummer fuer PV2 und PV6
ARGN	EQU	0B781H	; Anzahl Argumente
ARG1	EQU	0B782H	; Argument 1
ARG2	EQU	0B784H	;    "	   2
ARG3	EQU	0B786H	;    "	   3
ARG4	EQU	0B788H	;    "	   4
ARG5	EQU	0B78AH	;    "	   5
;ARG6	EQU	0B78CH	;    "	   6
ARG7	EQU	0B78EH	;    "	   7
;ARG8	EQU	0B790H	;    "	   8
;ARG9	EQU	0B792H	;    "	   9
;ARG10	EQU	0B794H	;    "	  10
NUMNX	EQU	0B796H	; Anzahl Zeichen einer Hex-Zahl
NUMVX	EQU	0B797H	; Wert HEX-Zahl
HCADR	EQU	0B799H	; Hardcopy-Aufruf
WINNR	EQU	0B79BH	; Nr. des aktuellen Bildschirmfensters
WINON	EQU	0B79CH	; Fenster-Anfang
WINLG	EQU	0B79EH	; Fenster-Groesse
CURSO	EQU	0B7A0H	; Cursor-Position
STBT	EQU	0B7A2H	; Bildschirm-Steuerbyte
		; Bit 0 = 1 Schreiben Pixel-IRM AUS
		; Bit 1 = 1 Schreiben Color-IRM AUS
		; Bit 2 = 1 Inversdarstellung EIN (ab CAOS 3.4)
		; Bit 3 = 1 Steuercode als Zeichen darstellen
		; Bit 4 = 1 ESC aktiv (ab CAOS 3.4)
		; Bit 5 = 1 IBM-Zeichensatz EIN (ab CAOS 4.3)
		; Bit 6 = 1 HRG-Modus EIN (ab CAOS 4.3)
		; Bit 7 = 1 2-Monitor-Modus aktiv (ab CAOS 4.8)
COLOR	EQU	0B7A3H	; Farbe
WEND	EQU	0B7A4H	; Reaktionsprogramm auf Bildschirmende
CCTL0	EQU	0B7A6H	; Zeichentabelle 0 20H...5FH
CCTL1	EQU	0B7A8H	;	"	 1 00H...1FH und 60H...7FH
CCTL2	EQU	0B7AAH	;	"	 2 A0H...DFH
CCTL3	EQU	0B7ACH	;	"	 3 80H...9FH und E0H...FFH
SYSP	EQU	0B7AEH	; System Stackpointer
SUTAB	EQU	0B7B0H	; Adresse der Unterprogrammtabelle
CTAB	EQU	0B7B2H	; CRT-Steuerprogramm-Tabelle
NCAOS	EQU	0B7B4H	; Sprung in neues System ueber IRM
OUTAB	EQU	0B7B9H	; Adresse fuer Zeiger auf UP-Nr. bei Ausgabe
INTAB	EQU	0B7BBH	; Adresse fuer Zeiger auf UP-Nr. bei Eingabe
UOUT1	EQU	0B7BDH	; Sprung zu USER-Ausgabekanal 2 (Drucker)
UIN1	EQU	0B7C0H	; Sprung zu USER-Eingabekanal 2
UOUT2	EQU	0B7C3H	; Sprung zu USER-Ausgabekanal 3 V.24-duplex
UIN2	EQU	0B7C6H	; Sprung zu USER-Eingabekanal 3 V.24-duplex
IOERR	EQU	0B7C9H	; Reaktionsprogramm fuer BASIC I/O-Error bis CAOS 4.7
BSYSP	EQU	0B7CBH	; Ablage SP bei IRMON (KC85/4: aktuelle VRAM-Adresse)
ZOTAB	EQU	0B7CDH	; Zwischenspeicher fuer OUTAB
ZWEND	EQU	0B7CFH	; Zwischenspeicher fuer WEND
FTAST	EQU	0B7D1H	; F-Tastenpointer
HOR	EQU	0B7D3H	; Grafik Horizontalposition (X-Wert)
VERT	EQU	0B7D5H	; Grafik Vertikalposition (Y-Wert)
FARB	EQU	0B7D6H	; Grafik Farbe
		; Bit 0=1	XOR-Funktion
		; Bit 1=1	Punkt loeschen
		; Bit 2=1	Farbebene nicht veraendern
		; Bit 3-7	Vordergrundfarbe + Blinken
MIXIT	EQU	0B7D7H	; IX-Merker (H-Teil von IX und I-Tabelle)
;BIT8	EQU	0B7D8H	; CAOS 3.4: High(BITTAB)
PROMPT	EQU	0B7D9H	; Systemprompt CAOS 3.4/3.5
VORTN	EQU	0B7DAH	; Vortonlaenge (fuer byteweise Kassettenausgabe)
DTPTR	EQU	0B7DCH	; CASS-Pointer (Zeiger in Kassettenpuffer)
L3TAB	EQU	0B7DDH	; Adresse der ESC-Steuerfunktionstabelle
L3SIZ	EQU	0B7DFH	; Anzahl der ESC-Steuerfunktionenn
COUNT	EQU	0B7E0H	; Zeiteinheiten fuer 1. Autorepeat (Tastatur)
HCPZ	EQU	0B7E1H	; Steuerbyte Druckerausgabe
		; 0	keine Reaktion auf Sh-CLR (kein Druckmodul)
		; 1	Protokollfunktion
		; 2	Hardcopy-Funktion

MODST	EQU	0B800H	; Modulsteuerbytespeicher
	; bis	0B8FFH
FTASTE	EQU	0B900H	; Funktionstastenspeicher
	; bis	0B99BH
WNDFN	EQU	0B99CH	; Fenstersvektorpeicher
	; bis	0B9FFH

; von den BASIC-Erweiterungen genutzte Routinen aus dem BASIC-ROM:

PRIST	EQU	0C00DH	; BASIC-Interpreter Kaltstart
SECST	EQU	0C08CH	; BASIC-Interpreter Warmstart
TMEMO	EQU	0C327H	; Test, ob RAM fuer naechste Operationen reicht
SNER	EQU	0C348H	; Sprung zu SN-ERROR
ERROO	EQU	0C356H	; Ausgabe Fehlercode
LIN13	EQU	0C442H	; Zeiger auf Zeile mit naechstgroesserer Nummer stellen
ULER	EQU	0C44DH	; Sprung zu UL-ERROR
LIN6	EQU	0C450H	; Zeile in Programm einsortieren	
LIN10	EQU	0C48AH
LIN11	EQU	0C493H
ZPOIT	EQU	0C4BBH	; Zeiger auf Anfang naechste Zeile stellen
ZPOIT1	EQU	0C4BEH
NEW1	EQU	0C641H	; BASIC-Programm loeschen
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
ASC1	EQU	0D33FH
ARGVL1	EQU	0D421H	; Parameter abholen
ARGVL2	EQU	0D424H
SGN1	EQU	0D6AEH
NUMKON	EQU	0D834H
IOER	EQU	0DC82H	; Sprung zu ?IO ERROR
CASS1	EQU	0DCB2H	; Dateiname aufbereiten
TESTCO	EQU	0DDC8H
CO	EQU	0DDD5H	; Ausgabe eines Zeichens (ASCII)
CI	EQU	0DDE4H	; Eingabe eines Zeichens (ASCII)
OUTCHL	EQU	0DE25H	; Ausgabe zu Kanal
INCHNL	EQU	0DE5FH	; Eingabe von Kanal
