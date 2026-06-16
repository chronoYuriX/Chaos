from main import *
from time import sleep
import tkinter

def test_def():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg = "#000000", bd = 0, height = 400, width = 600)
    canvas.pack()
    def paint(P, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        for line in P.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill = color)
            
    P = POLYGON(POINT(0, 0), POINT(-4, 3), POINT(4, 3), POINT(4, -3), POINT(-4, -3))
    print(P)
    paint(P, "#FF0000")
    
    P += POINT(4, 3)
    print(P)
    paint(P, "#AAAA00")
    
    P <<= pi / 6 - 1e-2
    print(P)
    paint(P, "#00FF00")
    P <<= 1e-2
    print(P)
    paint(P, "#00FF00")
    P <<= 1e-2
    print(P)
    paint(P, "#00FF00")
    
    P += POINT(-4, -3)
    print(P)
    paint(P, "#00AAAA")
    
    P >>= pi / 3
    print(P)
    paint(P, "#0000FF")
    
    window.mainloop()

def test_move():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg = "#000000", bd = 0, height = 400, width = 600)
    canvas.pack()
    def paint(P, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        for line in P.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill = color)

    P = POLYGON(POINT(0, 0), POINT(-4, 3), POINT(4, 3), POINT(4, -3), POINT(-4, -3))
    P += POINT(-10, 0)
    Q = POLYGON(POINT(0, 0), POINT(-3, 2), POINT(3, 2), POINT(3, -2), POINT(-3, -2))
    Q += POINT(0, -2)
    while P.center.x < 10:
        P += POINT(0.1, 0)
        P <<= 0.1
        Q += POINT(0, 0.02)
        color = "#FF0000" if P & Q else "#00FF00"
        canvas.delete("all")
        paint(P, color)
        paint(Q, color)
        window.update()
        sleep(0.05)

def test_collision():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg = "#000000", bd = 0, height = 400, width = 600)
    canvas.pack()
    def paint(P, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        for line in P.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill = color)
    def paintCollison(P, Q, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        collison = P | Q
        if collison.getLen() > 1e-6:
            collison *= scale
            collison += offset
            canvas.create_line(collison.A.x, collison.A.y, collison.B.x, collison.B.y, \
                               fill = color, arrow = tkinter.LAST, arrowshape = (2, 6, 3))

    P = POLYGON(POINT(0, 0), POINT(-4, 3), POINT(4, 3), POINT(4, -3), POINT(-4, -3))
    P += POINT(-10, 0)
    Q = POLYGON(POINT(0, 0), POINT(-3, 2), POINT(3, 2), POINT(3, -2), POINT(-3, -2))
    Q += POINT(0, -2)
    while P.center.x < 10:
        P += POINT(0.1, 0)
        P <<= 0.1
        Q += POINT(0, 0.02)
        color = "#FF0000" if P & Q else "#00FF00"
        canvas.delete("all")
        paint(P, color)
        paint(Q, color)
        paintCollison(P, Q)
        window.update()
        sleep(0.05)
    

## test_def()
## test_move()
test_collision()

