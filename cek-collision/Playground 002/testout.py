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
        
def test_move_with_info():
    window = tkinter.Tk()
    canvas = tkinter.Canvas(window, bg="#000000", bd=0, height=500, width=800)
    canvas.pack()
    
    def paint(P, color="#FFFFFF", scale=10, offset=POINT(400, 250)):
        for line in P.getEdge():
            line *= scale
            line += offset
            canvas.create_line(line.A.x, line.A.y, line.B.x, line.B.y, fill=color, width=2)

    def paint_info(info, scale=10, offset=POINT(400, 250)):
        """绘制碰撞信息"""
        if not info.collided:
            return
        
        # 绘制碰撞法线（红色箭头）
        start = info.contact_point * scale + offset
        end = POINT(
            start.x + info.normal.x * 30,
            start.y + info.normal.y * 30
        )
        canvas.create_line(start.x, start.y, end.x, end.y, 
                          fill="#FF0000", width=3, arrow=tkinter.LAST)
        
        # 绘制碰撞点（黄色圆点）
        cp = info.contact_point * scale + offset
        canvas.create_oval(cp.x-5, cp.y-5, cp.x+5, cp.y+5, 
                          fill="#FFFF00", outline="")
        
        # 绘制碰撞边（高亮）
        for edge in info.contact_edge_P1:
            edge *= scale
            edge += offset
            canvas.create_line(edge.A.x, edge.A.y, edge.B.x, edge.B.y, 
                             fill="#FF8800", width=4)
        for edge in info.contact_edge_P2:
            edge *= scale
            edge += offset
            canvas.create_line(edge.A.x, edge.A.y, edge.B.x, edge.B.y, 
                             fill="#FF8800", width=4)

    # 创建两个多边形
    P = POLYGON(POINT(0, 0), POINT(-4, 3), POINT(4, 3), POINT(4, -3), POINT(-4, -3))
    P += POINT(-10, 0)
    Q = POLYGON(POINT(0, 0), POINT(-3, 2), POINT(3, 2), POINT(3, -2), POINT(-3, -2))
    Q += POINT(0, -2)
    
    step = 0
    while P.center.x < 14:
        P += POINT(0.05, 0)
        P <<= 0.03
        Q += POINT(0, 0.015)
        
        # 使用新的碰撞检测
        info = detect_collision_with_info(P, Q)
        
        if info.collided:
            color = "#FF0000"
        else:
            color = "#00FF00"
        
        canvas.delete("all")
        paint(P, color)
        paint(Q, color)
        if info.collided:
            paint_info(info)
            # 显示碰撞信息
            canvas.create_text(400, 480, 
                             text=f"碰撞! 法线: ({info.normal.x:.2f}, {info.normal.y:.2f}), "
                                  f"深度: {info.depth:.2f}", 
                             fill="#FFFFFF", font=("Arial", 16))
        
        window.update()
        sleep(0.05)
        step += 1



## test_def()
## test_move()
# 运行测试
test_move_with_info()
