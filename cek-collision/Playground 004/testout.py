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

def test_physics():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg="#000000", bd=0, height=400, width=700)
    canvas.pack()
    
    def paint(body, color="#FFFFFF", scale=10, offset=POINT(350, 100)):
        P = body.polygon
        for line in P.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill=color, width=2)
    
    def paint_collision(collision_line, scale=10, offset=POINT(350, 100)):
        if collision_line.getLen() < 1e-6:
            return
        line = collision_line * scale + offset
        canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, 
                          fill="#FF0000", width=3, arrow=tkinter.LAST)
        # 绘制碰撞点
        canvas.create_oval(line.A.x-4, line.A.y-4, line.A.x+4, line.A.y+4, 
                          fill="#FFFF00", outline="")
    
    # 创建两个刚体
    # 方块A：较大，初始静止
    poly_A = POLYGON(POINT(0, 0), POINT(-3, -2), POINT(3, -2), POINT(3, 2), POINT(-3, 2))
    poly_A += POINT(-8, 0)
    body_A = RIGID_BODY(poly_A, mass=2.0, inertia=8.0)
    body_A.velocity = POINT(3, 0)  # 向右运动
    
    # 方块B：较小，初始静止
    poly_B = POLYGON(POINT(0, 0), POINT(-2, -1.5), POINT(2, -1.5), POINT(2, 1.5), POINT(-2, 1.5))
    poly_B += POINT(5, 0)
    body_B = RIGID_BODY(poly_B, mass=1.0, inertia=4.0)
    
    dt = 0.1  # 时间步长
    
    for frame in range(int(10 / dt)):
        # 检测碰撞
        collision = body_A.polygon | body_B.polygon
        
        if collision.getLen() > 1e-6:
            # 处理碰撞
            body_A.resolve_collision(body_B, collision, restitution=0.8)
        
        # 更新物理状态
        body_A.update(dt)
        body_B.update(dt)
        
        # 绘制
        canvas.delete("all")
        paint(body_A, "#4488FF")
        paint(body_B, "#FF8844")
        paint_collision(collision)
        
        # 显示信息
        info_text = f"A: v=({body_A.velocity.x:.1f}, {body_A.velocity.y:.1f}), ω={body_A.angular_velocity:.2f}"
        canvas.create_text(350, 380, text=info_text, fill="#FFFFFF", font=("Arial", 12))
        info_text = f"B: v=({body_B.velocity.x:.1f}, {body_B.velocity.y:.1f}), ω={body_B.angular_velocity:.2f}"
        canvas.create_text(350, 395, text=info_text, fill="#FFFFFF", font=("Arial", 12))
        
        window.update()
        sleep(dt)

## test_def()
## test_move()
## test_collision()
test_physics()
