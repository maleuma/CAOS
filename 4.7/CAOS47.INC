;-----------------------------------------------;
;	CAOS-Vereinbarungen (bis CAOS 4.7)	;
;	---------------------------------	;
;						;
; als INCLUDE fuer ASM-Quelltexte		;
;						;
; CAOS47.INC	ML-Soft 08.01.1994-28.07.2018	;
;-----------------------------------------------;

; Portadressen im Grundgeraet:
PORT1	EQU	84H	; Ausgabe-Port D08A	Merkzelle ist (IX+1)
			;	Bit 0 - Anzeige Bild 0 oder 1
			;	Bit 1 - Zugriff auf 0=Pixel- oder 1=Farbebene
			;	Bit 2 - Zugriff auf Bild 0 oder 1
			;	Bit 3 - 0=hohe Farbaufloesung oder 1=niedrige
			;	Bit 4-7 Auswahl der RAM8-Ebene (256K)
PORT2	EQU	86H	; Ausgabe-Port D09A	Merkzelle ist (IX+4)
			;	Bit 0 - RAM4 ein/aus
			;	Bit 1 - RAM4 Schreibschutz (1=aus)
			;	Bit 2-4 Bankauswahl fuer 2MB RAM On Board
			;	Bit 5-6 Auswahl der USER-ROM-Ebene
			;	Bit 7 - CAOS-ROM C ein/aus
PIOAD	EQU	88H	; System-PIO Datenbyte Port A
			;	Bit 0 - CAOS ROM-E ein/aus
			;	Bit 1 - RAM0 ein/aus
			;	Bit 2 - IRM ein/aus
			;	Bit 3 - Schreibschutz RAM0
			;	Bit 4 - K_OUT
			;	Bit 5 - System-LED
			;	Bit 6 - Motorschaltspannung
			;	Bit 7 - USER-ROM ein/aus
PIOBD	EQU	89H	; System-PIO Datenbyte Port B
			;	Bit 0 - Symmetrie-Flip-Flop Tonausgabe
			;	Bit 1 \
			;	Bit 2 - Lautstaerke Tonausgabe
			;	Bit 3 - 4 Bit (Low-aktiv)
			;	Bit 4 /
			;	Bit 5 - RAM8 ein/aus
			;	Bit 6 - Schreibschutz RAM8
			;	Bit 7 - Blinken ein/aus
PIOAS	EQU	8AH	; System-PIO Steuerbyte Port A
PIOBS	EQU	8BH	; System-PIO Steuerbyte Port B
CTC0	EQU	8CH	; CTC Kanal 0 (Tonhoehe 1)
CTC1	EQU	8DH	; CTC Kanal 1 (Kassettenausgabe)
CTC2	EQU	8EH	; CTC Kanal 2 (Tondauer)
CTC3	EQU	8FH	; CTC Kanal 3 (Tastatur)

; RAM0-Arbeitszellen:
SIX	EQU	1F0H	; CAOS-Belegung IX-Register

; IRM-Arbeitszellen:
V24PL	EQU	0A800H	; Steckplatz erstes V.24-Modul
INITU	EQU	0A880h	; Kommandoablage von INITIAL.UUU als Tastencode
	; bis	0A8FFh	; und erzeugte Quelltextzeile des Assemblers
DEVTAB	EQU	0A900H	; Tabelle der 8 Geraetetreiber: 8*32=256 Byte gross
JOYIRM	EQU	0AA94H	; Joystick-Tabelle			       (12 Byte)
	; bis	0AA9Fh
JWORK	EQU	0AAA0H	; Arbeitszellen vom Joystick-Editor	       (17 Byte)
	; bis	0AAB1H
VRAM1	EQU	0AD00H	; ASCII-RAM Bild 1
	; bis	0B1FFH
VRAM0	EQU	0B200H	; ASCII-RAM Bild 0
	; bis	0B6FFh
CASS	EQU	0B700H	; 128 Byte Kassettenpuffer (bis	0B77FH)
ARGC	EQU	0B780H	; UP-Nummer fuer PV2 und PV6
ARGN	EQU	0B781H	; Anzahl Argumente
ARG1	EQU	0B782H	; Argument 1
ARG2	EQU	0B784H	;    "	   2
ARG3	EQU	0B786H	;    "	   3
ARG4	EQU	0B788H	;    "	   4
ARG5	EQU	0B78AH	;    "	   5
ARG6	EQU	0B78CH	;    "     6
ARG7	EQU	0B78EH	;    "     7
ARG8	EQU	0B790H	;    "     8
ARG9	EQU	0B792H	;    "     9
ARG10	EQU	0B794H	;    "	  10
NUMNX	EQU	0B796H	; Anzahl Zeichen einer Hex-Zahl
NUMVX	EQU	0B797H	; Wert HEX-Zahl
HCADR	EQU	0B799H	; Hardcopy-Aufruf
WINNR	EQU	0B79BH	; Nr. des aktuellen Bildschirmfensters
WINON	EQU	0B79CH	; Fenster-Anfang
WINLG	EQU	0B79EH	; Fenster-Groesse
CURSO	EQU	0B7A0H	; Cursor-Position
STBT	EQU	0B7A2H	; Bildschirmsteuerbyte
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
IOERR	EQU	0B7C9H	; Reaktionsprogramm fuer IO-Error bei BASIC
VRAM	EQU	0B7CBH	; VRAM-Adresse des eingestellten Bildes
ZOTAB	EQU	0B7CDH	; Zwischenspeicher fuer OUTAB
ZWEND	EQU	0B7CFH	; Zwischenspeicher fuer WEND
FTAST	EQU	0B7D1H	; F-Tastenpointer
HOR	EQU	0B7D3H	; Grafik Horizontalposition (X-Wert)
VERT	EQU	0B7D5H	; Grafik Vertikalposition (Y-Wert)
FARB	EQU	0B7D6H	; Grafik Farbe
MIXIT	EQU	0B7D7H	; IX-Merker (H-Teil von IX und I-Tabelle)
VORTN	EQU	0B7D8H	; Vortonlaenge bei Tape-Ausgabe
DTPTR	EQU	0B7DAH	; CASS-Pointer (Zeiger in Kassettenpuffer)
CTCMD	EQU	0B7DBH	; CTC2-Modus
BLINK	EQU	0B7DCH	; Blinkfrequenz
L3TAB	EQU	0B7DDH	; Adresse der ESC-Steuerfunktionstabelle
L3SIZ	EQU	0B7DFH	; Anzahl der ESC-Steuerfunktionenn
COUNT	EQU	0B7E0H	; Zeiteinheiten fuer 1. Autorepeat (Tastatur)
HCPZ	EQU	0B7E1H	; Steuerbyte Druckerausgabe
INTV1	EQU	0B7E2H	; Adresse V.24-Init-Tabelle fuer Druckerausgabe
INTV1L	EQU	0B7E4H	; Laenge V.24-Initialisierungtabelle
INTV2	EQU	0B7E5H	; Adresse V.24-Init-Tabelle fuer Duplexroutine
INTV2L	EQU	0B7E7H	; Laenge V.24-Initialisierungstabelle
HCPZ2	EQU	0B7E8H	; Steuerbyte Duplexroutine
OFILT	EQU	0B7E9H	; Sprung zu Druckroutine (ungefiltert)
PROMPT	EQU	0B7ECH	; Systemprompt
LINTYP	EQU	0B7EDH	; Linientyp
CUMUST	EQU	0B7EEH	; Zeiger zu Cursormuster
JOYTAB	EQU	0B7F0H	; Zeiger zu Joystick-Codes
MODST	EQU	0B800H	; Modulsteuerbytespeicher (bis B8FFH)
FTASTE	EQU	0B900H	; Funktionstastenspeicher (bis B99BH)
WNDFN	EQU	0B99CH	; Fenstersvektorpeicher (bis B9FFH)

; Sprungadressen:
RESET	EQU	0E000H	; Einsprungadresse bei "RESET"
CAOSNR	EQU	0EDFFH	; Versionsnummer von CAOS
PWRON	EQU	0F000H	; Einsprungadresse bei "POWER ON"
PV1	EQU	0F003H	; Programmverteiler 1
PV2	EQU	0F006H	; Programmverteiler 2
ZEI2	EQU	0F008H	; Zeiger auf 2 = UOUT1 ab CAOS 4.7
PV3	EQU	0F009H	; Programmverteiler 3
ZEI3	EQU	0F00BH	; Zeiger auf 3 = UOUT2 ab CAOS 4.7
PV4	EQU	0F00CH	; Programmverteiler 4
UREL	EQU	0F00FH	; Relativer UP-Aufruf
JMPADR	EQU	0F012H	; Einsprungadresse "JUMP"
PV5	EQU	0F015H	; Programmverteiler 5
IRMON	EQU	0F018H	; IRM einschalten
IRMOF	EQU	0F01BH	; IRM abschalten
PV6	EQU	0F01EH	; Programmverteiler 6
PV7	EQU	0F021H	; Programmverteiler fuer Treiberaufrufe
			; (bis CAOS 4.5 Floppy-Treiber wie SERVICE.KCC)
BITTAB	EQU	0FDF8H	; Bittabelle

; Bildschirm-Steuerzeichen:
TAB	EQU	09H	; Tabulator
LF	EQU	0AH	; Zeilenvorschub
CLS	EQU	0CH	; Bildschirm loeschen
CR	EQU	0DH	; zum Zeilenanfang
ESC	EQU	1BH	; Steuerzeichen

; CAOS-Unterprogramme:
CRT	EQU	00H	; Zeichenausgabe auf Bildschirm
MBO	EQU	01H	; Ausgabe Datenblock auf Kassette
UOT1	EQU	02H	; Ausgabe auf Anwenderkanal 1
UOT2	EQU	03H	; Ausgabe auf Anwenderkanal 2
KBD	EQU	04H	; Tastatureingebe mit Cursoreinblendung
MBI	EQU	05H	; Datenblock von Kassette einlesen
USIN1	EQU	06H	; Eingabe von Anwenderkanal 1
USIN2	EQU	07H	; Eingabe von Anwenderkanal 2
ISRO	EQU	08H	; Ausgabe 1. Block auf Kassette
CSRO	EQU	09H	; Ausgabe letzten Block auf Kassette
ISRI	EQU	0AH	; Einlesen 1. Block von Kassette
CSRI	EQU	0BH	; Abschluss Magnetbandeingabe
KBDS	EQU	0CH	; Tastenstatusabfrage ohne Quittung
BYE	EQU	0DH	; Sprung zu RESET (Warmstart)
KBDZ	EQU	0EH	; Tastenstatusabfrage mit Quittung
COLORUP	EQU	0FH	; Farbe einstellen
LOAD	EQU	10H	; Einlesen MC-Programm von Kassette
VERIF	EQU	11H	; Ueberpruefung von Kassettenaufzeichnungen
LOOP	EQU	12H	; Ruecksprung zu CAOS-Menueschleife
NORM	EQU	13H	; Rueckschalten Ein-/Ausgabe auf CRT und KBD
WAIT	EQU	14H	; Warteschleife
LARG	EQU	15H	; Register mit Argumenten laden
INTB	EQU	16H	; Eingabe vom aktuellen Eingabekanal
INLIN	EQU	17H	; Eingabe einer Zeile
RHEX	EQU	18H	; Umwandlung Zeichenkette (HEX-Zahl)
ERRM	EQU	19H	; Fehlermeldung "ERROR"
HLHX	EQU	1AH	; Ausgabe des Wertes von HL als HEX-Zahl
HLDE	EQU	1BH	; Ausgabe der Werte von HL und DE als HEX-Zahlen
AHEX	EQU	1CH	; Ausgabe des Wertes von A als HEX-Zahl
ZSUCH	EQU	1DH	; Suche nach Zeichenkette (Menuewort)
SOUT	EQU	1EH	; Zeiger auf Ausgabetabelle setzen
SIN	EQU	1FH	; Zeiger auf Eingabetabelle setzen
NOUT	EQU	20H	; Zeiger auf Normalausgabe (CRT) setzen
NIN	EQU	21H	; Zeiger auf Normaleingabe (KBD) setzen
GARG	EQU	22H	; Erfassen von max. 10 HEX-Zahlen
OSTR	EQU	23H	; Ausgabe folgender Zeichenkette
OCHR	EQU	24H	; Zeichenausgabe ueber Ausgabetabelle
CUCP	EQU	25H	; Cursor komplementieren
MODU	EQU	26H	; Modulsteuerung
JUMP	EQU	27H	; Neues Betriebssystem anspringen
LDMA	EQU	28H	; Speicher (IRM) schreiben
LDAM	EQU	29H	; Speicher (IRM) lesen
BRKT	EQU	2AH	; Test BRK-Taste
SPACE	EQU	2BH	; Ausgabe Leerzeichen
CRLF	EQU	2CH	; Ausgabe CR und LF
HOME	EQU	2DH	; Ausgabe HOME
MODI	EQU	2EH	; Systemkommando "MODIFY"
PUDE	EQU	2FH	; Punkt loeschen
PUSE	EQU	30H	; Punkt setzen
SIXD	EQU	31H	; Verlagerung IX-Arbeitsbereich von CAOS
DABR	EQU	32H	; Berechnung VRAM-Adresse
TCIF	EQU	33H	; Test, ob Cursorposition im Fenster
PADR	EQU	34H	; Berechnen Pixel- und Farbadresse
TON	EQU	35H	; Tonausgabe
SAVE	EQU	36H	; Ausgabe MC-Programm auf Kassette
MBIN	EQU	37H	; Byteweise Eingabe von Kassette
MBOUT	EQU	38H	; Byteweise Ausgabe auf Kassette
KEY	EQU	39H	; Belegen einer F-Taste
KEYLI	EQU	3AH	; Anzeige F-Tastenbelegung
DISP	EQU	3BH	; Systemkommando "DISPLAY"
WININ	EQU	3CH	; Fenster initialisieren
WINAK	EQU	3DH	; Fenster ueber Nummer aufrufen
LINE	EQU	3EH	; Linie zeichnen
CIRCLE	EQU	3FH	; Kreis zeichnen
SQR	EQU	40H	; Quadratwurzel berechnen
MULT	EQU	41H	; Multiplikation
CSTBT	EQU	42H	; Bildschirmanzeige Steuerzeichensymbol
INIEA	EQU	43H	; Initialisierung E/A-Kanal
INIME	EQU	44H	; Initialisierung mehrerer E/A-Kanaele
ZKOUT	EQU	45H	; Ausgabe Zeichenkette
MENU	EQU	46H	; aktuelles Menue ausschreiben
V24OUT	EQU	47H	; Initialisierung Druckerausgabe
V24DUP	EQU	48H	; Initialisierung V.24-Duplexroutine
DEVICE	EQU	49H	; DEVICE-Funktionen ab CAOS 4.6

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
UROK	EQU	0B3F3H	; Betriebsart	0 = Anfangswert
			;		1 = CP/M-System lauft
			;		2 = Not ready
			;		3 = Can't read
			;		4 = No system
			;		5 = CAOS-Betriebsart / DEP laeuft
; allgemeine Macros:

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

; end of include
