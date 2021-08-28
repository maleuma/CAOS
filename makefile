# Makefile zum Erzeugen von CAOS 4.8 unter Windows (CP/M-Emulation)
# (c) Mario Leubner 22.06.2021

.PHONY: all
all: CAOS48-C.ROM CAOS48-E.ROM CAOS48dC.ROM CAOS48dE.ROM

#_______________________________________________________________________________
# Erzeugen der CAOS-Dateien CAOS 4.8 - klassischer Zeichensatz:

CAOS48-C.ROM : CAOS48.KCC
	dd if=CAOS48.KCC of=CAOS48-C.ROM ibs=128 skip=1 count=64

CAOS48-E.ROM : CAOS48.KCC
	dd if=CAOS48.KCC of=CAOS48-E.ROM ibs=128 skip=65 count=64

CAOS48.KCC : CAOS48.Z80 EQU48.INC CC48.INC CD48.INC CE48.INC CF48.INC ZSIBM.INC ZSGROSS.INC ZSKLEIN.INC VORBLOCK.INC
	cpm SLRZ80 NORMAL/F
	mv -f NORMAL.COM CAOS48.KCC
	mv -f NORMAL.prn CAOS48.PRN

#_______________________________________________________________________________
# Erzeugen der CAOS-Dateien CAOS 4.8 - duenner Zeichensatz:

CAOS48dC.ROM : CAOS48d.KCC
	dd if=CAOS48d.KCC of=CAOS48dC.ROM ibs=128 skip=1 count=64

CAOS48dE.ROM : CAOS48d.KCC
	dd if=CAOS48d.KCC of=CAOS48dE.ROM ibs=128 skip=65 count=64

CAOS48d.KCC : CAOS48.Z80 EQU48.INC CC48.INC CD48.INC CE48.INC CF48.INC DAB8DFFF.INC EE00EFFF.INC FE00FFFF.INC VORBLOCK.INC
	cpm SLRZ80 DUENN/F
	mv -f DUENN.COM CAOS48d.KCC
	mv -f DUENN.prn CAOS48d.PRN
