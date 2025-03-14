# CAOS
 Betriebssysteme fuer die Kleincomputer KC85/2-5 von Muehlhausen (MPM)

### CAOS 2.2
Standardbetriebssystem fuer den KC85/2 von MPM

### CAOS 3.1
Standardbetriebssystem fuer den KC85/3 von MPM

### CAOS 3.4
schnelles Betriebssystem fuer den KC85/3 mit Bildschirmroutinen wie KC85/4

### CAOS 4.1
Originalbetriebssystem von MPM, erste bekannte Version fuer den KC85/4

### CAOS 4.2
Standardbetriebssystem von MPM, korrigierte Version fuer den KC85/4

### CAOS 4.3
erste Version vom KC-Club fuer den KC85/5 mit 256K RAM und 32K USER ROM

### CAOS 4.4
Fehlerkorrekturen zu CAOS 4.3

### CAOS 4.5
Weiterentwicklung, fest integrierter Joystick-Treiber+Editor, Taschenrechner CALC

### CAOS 4.6
DEVICE-Umschaltung TAPE/DISK (TAPE unter BASIC nicht nutzbar)

### CAOS 4.7
DEVICE-Umschaltung USB hinzugefuegt, wahlweise 80-Zeichen-Editor oder FORTH

### CAOS 4.8
- neues DEVICE-Treiber-System
- 8 System-Baenke ueber JUMP 0-7 (KC85/5+ mit 512K FLASH ROM)
- 2-Monitor-Modus
- neue CAOS-UP HLDEZ und RDEZ
- dezimale Argumente bei WINDOW, COLOR und dem User-Bereich im Kommando CD

### CAOS 4.9
${\color{red}ACHTUNG! \space CAOS \space 4.9 \space benoetigt \space 16K \space ROM:C \space und \space 16K \space ROM:E \space (Flash \space ROM \space Erweiterung)}$
- Fullscreen-Editor fuer BASIC (ausgelagert in USER-ROM)
- 16Bit-SWITCH fuer Module, Erkennung Flash-ROM-Groesse bei M044
- Time-Kommando erkennt RTC auf Portadresse 55h
- TAPE-Directory listet nun auch CP/M-Dateien auf mit ARGN=1
