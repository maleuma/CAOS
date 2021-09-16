; Erzeugen des CAOS-Vorblock fuer CAOS 4.8 und USER-ROM:

DATUM	MACRO
	DB	'12.09.2021'	; Versionsdatum nur hier aendern!
	ENDM

	VB_NAME			; Dateiname
	DS	5,0
	DB	2		; 2 Adressen
	DW	AADR		; Anfangsadresse
	DW	EADR		; Endadresse+1
	DS	75,0
	DB	'KC-Club, M. Leubner:  '
	DATUM			; Versionsdatum
