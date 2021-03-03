DECLARE SUB SetPaletteEntry (c, r, g, b)

'path$ = "c:\windows\msremote.sfs\progra~1\3dmode~1\"
path$ = "c:\randy'~1\3dmode~1\"

name$ = "jasmine.3dm"
numobjects = 2
GOSUB Init

YTOP = 30
YBOTTOM = 10
XLEFT = 20
XRIGHT = 20
objnum = 0
face = 1
readname$ = "bfrontbd.pic"
GOSUB NextObject

YTOP = 30
YBOTTOM = 10
XLEFT = 20
XRIGHT = 20
objnum = 1
face = -1
readname$ = "bbackbd.pic"
GOSUB NextObject

GOSUB PreRender

objnum = 0
readname$ = "jfrontbd.pic"
GOSUB NextTexture
 
objnum = 1
readname$ = "jbackbd.pic"
GOSUB NextTexture

GOSUB AllDone
END

Init:
SCREEN 7, , 0, 0: CLS
DIM r(256), g(256), b(256)
xo = POINT(0)
yo = POINT(1)
OPEN path$ + name$ FOR OUTPUT AS #1
PRINT #1, "3-D Model Script: "; name$
a$ = "Number of objects: <" + STR$(numobjects) + ">"
GOSUB Parse
PRINT #1, a$
PRINT #1, ""
RETURN

NextObject:
PRINT #1, "Object" + STR$(objnum)
fx = 0: fy = 0: lx = 0: ly = 0
  OPEN path$ + readname$ FOR INPUT AS #2
  FOR y = 0 TO 199
    FOR x = 0 TO 319
      INPUT #2, a
      PSET (x, y), a
      IF a THEN IF y > 16 AND y < 168 THEN lx = x: ly = y: IF fx = 0 AND fy = 0 THEN fx = x: fy = y
    NEXT
  NEXT
  FOR c = 0 TO 15
    INPUT #2, r
    INPUT #2, g
    INPUT #2, b
    GOSUB SetPalette
  NEXT
  CLOSE #2
nv = 0
FOR y = yo - YTOP TO yo + YBOTTOM
FOR x = xo - XLEFT TO xo + XRIGHT
IF POINT(x, y) THEN nv = nv + 1
NEXT
NEXT
a$ = STR$(nv)
MID$(a$, 1, 1) = "<"
a$ = "  Number of vertices: " + a$ + ">"
PRINT #1, a$
PRINT #1, "  Vertices:"
vn = 0
FOR y = yo - YTOP TO yo + YBOTTOM
FOR x = xo - XLEFT TO xo + XRIGHT
IF POINT(x, y) THEN
a$ = "<" + STR$(((x - xo) * 4) - 2) + "," + STR$(((y - yo) * 4) - 2) + "," + STR$((POINT(x, y) - 1) * 4 * face) + ">"
GOSUB Parse
a$ = "   " + STR$(vn) + " " + a$
PRINT #1, a$
vn = vn + 1
END IF
NEXT
NEXT
PRINT #1, ""
RETURN

PreRender:
PRINT #1, "Pre-render Instructions:"
PRINT #1, "<clr><1:255,255,255>"
PRINT #1, ""
PRINT #1, "Render Instructions:"
PRINT #1, "<render>"
PRINT #1, ""
RETURN

NextTexture:
a$ = "<yprw><" + STR$(objnum) + ">"
GOSUB Parse
PRINT #1, a$
a$ = "<movw><" + STR$(objnum) + ">"
GOSUB Parse
PRINT #1, a$
PRINT #1, ""
n$ = STR$(objnum) + ":"
OPEN path$ + readname$ FOR INPUT AS #2
  FOR y = 0 TO 199
    FOR x = 0 TO 319
      INPUT #2, a
      PSET (x, y), a
    NEXT
  NEXT
  FOR c = 0 TO 15
    INPUT #2, r
    INPUT #2, g
    INPUT #2, b
    GOSUB SetPalette
  NEXT
  CLOSE #2
np = 0
vn = 0
FOR y = yo - YTOP TO yo + YBOTTOM
FOR x = xo - XLEFT TO xo + XRIGHT
IF POINT(x, y) THEN
  vhome = vn
  IF POINT(x + 1, y) THEN veast = vhome + 1
  vsouth = -1
  IF POINT(x, y + 1) THEN
    vsouth = vhome
    FOR x1 = x + 1 TO 319
      IF POINT(x1, y) THEN vsouth = vsouth + 1
    NEXT
    FOR x1 = 0 TO x
      IF POINT(x1, y + 1) THEN vsouth = vsouth + 1
    NEXT
  END IF
  vsoutheast = -1
  IF POINT(x + 1, y + 1) THEN
    vsoutheast = vhome
    FOR x1 = x + 1 TO 319
      IF POINT(x1, y) THEN vsoutheast = vsoutheast + 1
    NEXT
    FOR x1 = 0 TO x + 1
      IF POINT(x1, y + 1) THEN vsoutheast = vsoutheast + 1
    NEXT
  END IF
  IF vsouth > 0 AND veast > 0 AND vsoutheast > 0 THEN
    c = POINT(x, y): GOSUB GetPalette
    r1 = r: g1 = g: b1 = b
    c = POINT(x, y + 1): GOSUB GetPalette
    r2 = r: g2 = g: b2 = b
    c = POINT(x + 1, y): GOSUB GetPalette
    r3 = r: g3 = g: b3 = b
    rr = INT(((r1 + r2 + r3) * 4) / 3)
    gg = INT(((g1 + g2 + g3) * 4) / 3)
    bb = INT(((b1 + b2 + b3) * 4) / 3)
    a$ = "<prgb><" + n$ + STR$(vhome) + "><" + n$ + STR$(vsouth) + "><" + n$ + STR$(veast) + "><" + STR$(rr) + "," + STR$(gg) + "," + STR$(bb) + ">"
    GOSUB Parse
    PRINT #1, a$
    c = POINT(x + 1, y + 1): GOSUB GetPalette
    r1 = r: g1 = g: b1 = b
    c = POINT(x, y + 1): GOSUB GetPalette
    r2 = r: g2 = g: b2 = b
    c = POINT(x + 1, y): GOSUB GetPalette
    r3 = r: g3 = g: b3 = b
    rr = INT(((r1 + r2 + r3) * 4) / 3)
    gg = INT(((g1 + g2 + g3) * 4) / 3)
    bb = INT(((b1 + b2 + b3) * 4) / 3)
    a$ = "<prgb><" + n$ + STR$(vsoutheast) + "><" + n$ + STR$(vsouth) + "><" + n$ + STR$(veast) + "><" + STR$(rr) + "," + STR$(gg) + "," + STR$(bb) + ">"
    GOSUB Parse
    PRINT #1, a$
    np = np + 2
  END IF
  vn = vn + 1
END IF
NEXT
NEXT
PRINT #1, ""
RETURN

AllDone:
PRINT #1, "<end>"
CLOSE #1
RETURN

SetPalette:
r = INT(r)
g = INT(g)
b = INT(b)
CALL SetPaletteEntry(c, r, g, b)
r(c) = r
g(c) = g
b(c) = b
RETURN

GetPalette:
r = r(c)
g = g(c)
b = b(c)
RETURN

Parse:
c2 = LEN(a$)
FOR c1 = 1 TO c2
IF MID$(a$, c1, 1) = " " THEN a$ = MID$(a$, 1, c1 - 1) + MID$(a$, c1 + 1, c2 - c1): c2 = c2 - 1
NEXT
RETURN

SUB SetPaletteEntry (c, r, g, b)

OUT &H3C6, 255
OUT &H3C8, c
OUT &H3C9, r
OUT &H3C9, g
OUT &H3C9, b

END SUB

