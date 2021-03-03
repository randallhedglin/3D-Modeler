'path$ = "C:\WINDOWS\MSREMOTE.SFS\PROGRA~1\3DMODE~1\"
path$ = "C:\RANDY'~1\3DMODE~1\"
SCREEN 7: CLS
px = 160
py = 100
LC = 0
1 PN = POINT(px, py - 1)
PS = POINT(px, py + 1)
PE = POINT(px + 1, py)
PW = POINT(px - 1, py)
PSET (px, py - 1), (PN + 8) MOD 16
PSET (px, py + 1), (PS + 8) MOD 16
PSET (px + 1, py), (PE + 8) MOD 16
PSET (px - 1, py), (PW + 8) MOD 16
LOCATE 1, 1: PRINT px; py;
2 ZZ$ = INKEY$
: IF ZZ$ = "" THEN GOTO 2
PSET (px, py - 1), PN
PSET (px, py + 1), PS
PSET (px + 1, py), PE
PSET (px - 1, py), PW
LOCATE 1, 1: PRINT px; py;
IF ZZ$ = "0" THEN PSET (px, py), 0: LC = 0
IF ZZ$ = "1" THEN PSET (px, py), 1: LC = 1
IF ZZ$ = "2" THEN PSET (px, py), 2: LC = 2
IF ZZ$ = "3" THEN PSET (px, py), 3: LC = 3
IF ZZ$ = "4" THEN PSET (px, py), 4: LC = 4
IF ZZ$ = "5" THEN PSET (px, py), 5: LC = 5
IF ZZ$ = "6" THEN PSET (px, py), 6: LC = 6
IF ZZ$ = "7" THEN PSET (px, py), 7: LC = 7
IF ZZ$ = "8" THEN PSET (px, py), 8: LC = 8
IF ZZ$ = "9" THEN PSET (px, py), 9: LC = 9
IF ZZ$ = "a" THEN PSET (px, py), 10: LC = 10
IF ZZ$ = "b" THEN PSET (px, py), 11: LC = 11
IF ZZ$ = "c" THEN PSET (px, py), 12: LC = 12
IF ZZ$ = "d" THEN PSET (px, py), 13: LC = 13
IF ZZ$ = "e" THEN PSET (px, py), 14: LC = 14
IF ZZ$ = "f" THEN PSET (px, py), 15: LC = 15
IF ZZ$ = ")" THEN CC = 0: GOSUB NEWCOLOR
IF ZZ$ = "!" THEN CC = 1: GOSUB NEWCOLOR
IF ZZ$ = "@" THEN CC = 2: GOSUB NEWCOLOR
IF ZZ$ = "#" THEN CC = 3: GOSUB NEWCOLOR
IF ZZ$ = "$" THEN CC = 4: GOSUB NEWCOLOR
IF ZZ$ = "%" THEN CC = 5: GOSUB NEWCOLOR
IF ZZ$ = "^" THEN CC = 6: GOSUB NEWCOLOR
IF ZZ$ = "&" THEN CC = 7: GOSUB NEWCOLOR
IF ZZ$ = "*" THEN CC = 8: GOSUB NEWCOLOR
IF ZZ$ = "(" THEN CC = 9: GOSUB NEWCOLOR
IF ZZ$ = "A" THEN CC = 10: GOSUB NEWCOLOR
IF ZZ$ = "B" THEN CC = 11: GOSUB NEWCOLOR
IF ZZ$ = "C" THEN CC = 12: GOSUB NEWCOLOR
IF ZZ$ = "D" THEN CC = 13: GOSUB NEWCOLOR
IF ZZ$ = "E" THEN CC = 14: GOSUB NEWCOLOR
IF ZZ$ = "F" THEN CC = 15: GOSUB NEWCOLOR
IF ZZ$ = "p" THEN PAINT (px, py), LC, LC
IF ZZ$ = "|" THEN GOSUB MAKEBUMP
IF ZZ$ = CHR$(0) + "H" THEN py = py - 1
IF ZZ$ = CHR$(0) + "P" THEN py = py + 1
IF ZZ$ = CHR$(0) + "K" THEN px = px - 1
IF ZZ$ = CHR$(0) + "M" THEN px = px + 1
IF ZZ$ = "S" THEN GOSUB SAVEFILE
IF ZZ$ = "L" THEN GOSUB LOADFILE
GOTO 1

SAVEFILE:
  LOCATE 23, 1
  INPUT file$
  OPEN path$ + file$ + ".pic" FOR OUTPUT AS #1
  FOR Y = 0 TO 199
    FOR X = 0 TO 319
      PRINT #1, POINT(X, Y)
    NEXT
  NEXT
  FOR CC = 0 TO 15
    OUT &H3C6, 255
    OUT &H3C7, CC
    RR = INP(&H3C9)
    GG = INP(&H3C9)
    BB = INP(&H3C9)
    PRINT #1, RR
    PRINT #1, GG
    PRINT #1, BB
  NEXT
  CLOSE #1
RETURN

LOADFILE:
  LOCATE 23, 1
  INPUT file$
  OPEN path$ + file$ + ".pic" FOR INPUT AS #1
  FOR Y = 0 TO 199
    FOR X = 0 TO 319
      INPUT #1, A
      PSET (X, Y), A
    NEXT
  NEXT
  FOR CC = 0 TO 15
    INPUT #1, RR
    INPUT #1, GG
    INPUT #1, BB
    OUT &H3C6, 255
    OUT &H3C8, CC
    OUT &H3C9, RR
    OUT &H3C9, GG
    OUT &H3C9, BB
  NEXT
  CLOSE #1
RETURN

NEWCOLOR:
3 ZZ$ = INKEY$: IF ZZ$ = "" THEN GOTO 3
  IF ZZ$ = "0" THEN RR = 0
  IF ZZ$ = "1" THEN RR = 4
  IF ZZ$ = "2" THEN RR = 8
  IF ZZ$ = "3" THEN RR = 12
  IF ZZ$ = "4" THEN RR = 17
  IF ZZ$ = "5" THEN RR = 21
  IF ZZ$ = "6" THEN RR = 25
  IF ZZ$ = "7" THEN RR = 29
  IF ZZ$ = "8" THEN RR = 33
  IF ZZ$ = "9" THEN RR = 37
  IF ZZ$ = "a" THEN RR = 42
  IF ZZ$ = "b" THEN RR = 46
  IF ZZ$ = "c" THEN RR = 50
  IF ZZ$ = "d" THEN RR = 54
  IF ZZ$ = "e" THEN RR = 58
  IF ZZ$ = "f" THEN RR = 63
4 ZZ$ = INKEY$: IF ZZ$ = "" THEN GOTO 4
  IF ZZ$ = "0" THEN GG = 0
  IF ZZ$ = "1" THEN GG = 4
  IF ZZ$ = "2" THEN GG = 8
  IF ZZ$ = "3" THEN GG = 12
  IF ZZ$ = "4" THEN GG = 17
  IF ZZ$ = "5" THEN GG = 21
  IF ZZ$ = "6" THEN GG = 25
  IF ZZ$ = "7" THEN GG = 29
  IF ZZ$ = "8" THEN GG = 33
  IF ZZ$ = "9" THEN GG = 37
  IF ZZ$ = "a" THEN GG = 42
  IF ZZ$ = "b" THEN GG = 46
  IF ZZ$ = "c" THEN GG = 50
  IF ZZ$ = "d" THEN GG = 54
  IF ZZ$ = "e" THEN GG = 58
  IF ZZ$ = "f" THEN GG = 63
5 ZZ$ = INKEY$: IF ZZ$ = "" THEN GOTO 5
  IF ZZ$ = "0" THEN BB = 0
  IF ZZ$ = "1" THEN BB = 4
  IF ZZ$ = "2" THEN BB = 8
  IF ZZ$ = "3" THEN BB = 12
  IF ZZ$ = "4" THEN BB = 17
  IF ZZ$ = "5" THEN BB = 21
  IF ZZ$ = "6" THEN BB = 25
  IF ZZ$ = "7" THEN BB = 29
  IF ZZ$ = "8" THEN BB = 33
  IF ZZ$ = "9" THEN BB = 37
  IF ZZ$ = "a" THEN BB = 42
  IF ZZ$ = "b" THEN BB = 46
  IF ZZ$ = "c" THEN BB = 50
  IF ZZ$ = "d" THEN BB = 54
  IF ZZ$ = "e" THEN BB = 58
  IF ZZ$ = "f" THEN BB = 63
  OUT &H3C6, 255
  OUT &H3C8, CC
  OUT &H3C9, RR
  OUT &H3C9, GG
  OUT &H3C9, BB
RETURN

MAKEBUMP:
  FOR Y = 0 TO 199
    FOR X = 0 TO 319
      IF POINT(X, Y) THEN PSET (X, Y), 1
    NEXT
  NEXT
  FOR CC = 0 TO 15
    OUT &H3C6, 255
    OUT &H3C8, CC
    OUT &H3C9, CC * 4
    OUT &H3C9, CC * 4
    OUT &H3C9, CC * 4
  NEXT
RETURN

