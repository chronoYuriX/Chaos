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
        collison, L1, L2 = P | Q
        if collison.getLen() > 1e-6:
            collison *= scale
            collison += offset
            L1 *= scale
            L1 += offset
            L2 *= scale
            L2 += offset
            canvas.create_line(collison.A.x, collison.A.y, collison.B.x, collison.B.y, \
                               fill = color, arrow = tkinter.LAST, arrowshape = (2, 6, 3))
            canvas.create_line(L1.A.x, L1.A.y, L1.B.x, L1.B.y, fill = "#DDDD00")
            canvas.create_line(L2.A.x, L2.A.y, L2.B.x, L2.B.y, fill = "#DDDD00")


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

def test_collision_parallel():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg = "#000000", bd = 0, height = 400, width = 600)
    canvas.pack()
    def paint(P, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        for line in P.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill = color)
    def paintCollison(P, Q, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        collison, L1, L2 = P | Q
        if collison.getLen() > 1e-6:
            collison *= scale
            collison += offset
            L1 *= scale
            L1 += offset
            L2 *= scale
            L2 += offset
            canvas.create_line(collison.A.x, collison.A.y, collison.B.x, collison.B.y, \
                               fill = color, arrow = tkinter.LAST, arrowshape = (2, 6, 3))
            canvas.create_line(L1.A.x, L1.A.y, L1.B.x, L1.B.y, fill = "#DDDD00")
            canvas.create_line(L2.A.x, L2.A.y, L2.B.x, L2.B.y, fill = "#DDDD00")

    P = POLYGON(POINT(0, 0), POINT(-4, 3), POINT(4, 3), POINT(4, -3), POINT(-4, -3))
    P += POINT(-10, 0)
    Q = POLYGON(POINT(0, 0), POINT(-3, 2), POINT(3, 2), POINT(3, -2), POINT(-3, -2))
    Q += POINT(0, -2)
    while P.center.x < 10:
        P += POINT(0.1, 0)
        # P <<= 0.1
        Q += POINT(0, 0.02)
        color = "#FF0000" if P & Q else "#00FF00"
        canvas.delete("all")
        paint(P, color)
        paint(Q, color)
        paintCollison(P, Q)
        window.update()
        sleep(0.05)

def test_rebound():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg = "#000000", bd = 0, height = 400, width = 600)
    canvas.pack()
    def paint(P, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        for line in P.polygon.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill = color)
    def paintCollison(collision, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        if collision.getLen() > 1e-6:
            line = collision * scale + offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, \
                               fill = color, arrow = tkinter.LAST, arrowshape = (2, 6, 3))
    P = POLYGON(POINT(0, 0), POINT(-3, -2), POINT(3, -2), POINT(3, 2), POINT(-3, 2))
    P += POINT(-8, 0)
    P >>= pi / 8
    A = RIGID_BODY(P, mass=2.0)
    A.velocity = POINT(3, 0)
    Q = POLYGON(POINT(0, 0), POINT(-2, -1.5), POINT(2, -1.5), POINT(2, 1.5), POINT(-2, 1.5))
    Q += POINT(5, 0)
    B = RIGID_BODY(Q, mass=1.0)
    B.angular_velocity = -2 * pi
    
    dt = 0.01
    for _ in range(500):
        collision, _, _ = A.polygon | B.polygon
        if collision.getLen() > 1e-6:
            RIGID_BODY.rebound(A, B, collision, e = 0.8)
        A.update(dt)
        B.update(dt)
        
        color = "#FF0000" if P & Q else "#00FF00"
        canvas.delete("all")
        paint(A, color)
        paint(B, color)
        paintCollison(collision)
        info_text = f"A: v=({A.velocity.x:.1f}, {A.velocity.y:.1f}), ω={A.angular_velocity:.2f}"
        canvas.create_text(350, 380, text=info_text, fill="#FFFFFF", font=("Consolas", 12))
        info_text = f"B: v=({B.velocity.x:.1f}, {B.velocity.y:.1f}), ω={B.angular_velocity:.2f}"
        canvas.create_text(350, 395, text=info_text, fill="#FFFFFF", font=("Consolas", 12))
        
        window.update()
        sleep(dt)

def test_rebound_parallel():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg = "#000000", bd = 0, height = 400, width = 600)
    canvas.pack()
    def paint(P, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        for line in P.polygon.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill = color)
    def paintCollison(collision, color = "#FFFFFF", scale = 10, offset = POINT(300, 200)):
        if collision.getLen() > 1e-6:
            line = collision * scale + offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, \
                               fill = color, arrow = tkinter.LAST, arrowshape = (2, 6, 3))
    P = POLYGON(POINT(0, 0), POINT(-3, -2), POINT(3, -2), POINT(3, 2), POINT(-3, 2))
    P += POINT(-8, 0)
    # P >>= 3e-2
    A = RIGID_BODY(P, mass=2.0)
    A.velocity = POINT(3, 0)
    Q = POLYGON(POINT(0, 0), POINT(-2, -1.5), POINT(2, -1.5), POINT(2, 1.5), POINT(-2, 1.5))
    Q += POINT(5, 0)
    B = RIGID_BODY(Q, mass=1.0)
    
    dt = 0.02
    for _ in range(500):
        collision, _, _ = A.polygon | B.polygon
        if collision.getLen() > 1e-6:
            RIGID_BODY.rebound(A, B, collision, e = 1)
        A.update(dt)
        B.update(dt)

        color = "#FF0000" if P & Q else "#00FF00"
        canvas.delete("all")
        paint(A, color)
        paint(B, color)
        paintCollison(collision)
        info_text = f"A: v=({A.velocity.x:.1f}, {A.velocity.y:.1f}), ω={A.angular_velocity:.2f}"
        canvas.create_text(350, 380, text=info_text, fill="#FFFFFF", font=("Consolas", 12))
        info_text = f"B: v=({B.velocity.x:.1f}, {B.velocity.y:.1f}), ω={B.angular_velocity:.2f}"
        canvas.create_text(350, 395, text=info_text, fill="#FFFFFF", font=("Consolas", 12))
        
        window.update()
        sleep(dt)

## test_def()
## test_move()
## test_collision()
## test_collision_parallel()
test_rebound()
test_rebound_parallel()
