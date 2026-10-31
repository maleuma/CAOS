; Erzeugen des CAOS-Vorblock fuer CAOS 4.9 und USER-ROM:

DATUM	MACRO
	DB	'04.10.2026'	; Versionsdatum nur hier aendern!
	ENDM

	VB_NAME			; Dateiname
	DS	5,0
	DB	2		; 2 Adressen
	DW	AADR		; Anfangsadresse
	DW	EADR		; Endadresse+1
	DS	75,0
	DB	'KC-Club, M. Leubner:  '
	DATUM			; Versionsdatum
