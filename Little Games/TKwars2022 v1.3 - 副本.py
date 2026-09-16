#Tankwars 1.3 test
#input an angle and win the race!
import turtle
from time import time as tm
from time import sleep as wait
from math import radians as rad
from math import sin,cos
from random import randint
ground = []
lz = []
rz = []
def spgo(pen,x,y):
    pen.penup()
    pen.goto(x,y)
    pen.pendown()
def boom_on_ground(pen):
    turtle.tracer(False)
    px = pen.xcor()
    py = pen.ycor()
    for i in range(randint(3,7)):
        spgo(pen,px,py+3)
        angle = randint(0,90)
        side = 1 if randint(0,1)else -1
        xa = side*cos(rad(angle))
        ya = sin(rad(angle))
        pen.color('orange')
        pen.pendown()
        gforce = randint(15,30)/1000
        flag = True
        while flag:
            xu = pen.xcor()
            yu = pen.ycor()
            pen.goto(xu+xa,yu+ya)
            ya -= gforce
            if (int(xu),int(yu))in rz:
                turtle.tracer(True)
                return True
            elif (int(xu),int(yu))in lz:
                turtle.tracer(True)
                return False
            elif (int(xu),int(yu))in ground:
                flag = False
            elif xu > 400 or xu < -400:
                flag = False
        pen.penup()
    turtle.tracer(True)
    return None
def fire(l,leftx,lefty,rightx,righty,pen):
    angle = int(input('angle:'))
    cnt = 0
    turtle.tracer(False)
    if l:
        spgo(pen,leftx,lefty+10)
        xa = cos(rad(angle))
        ya = sin(rad(angle))
        pen.color('red')
        pen.pendown()
        while True:
            xu = pen.xcor()
            yu = pen.ycor()
            pen.goto(xu+xa,yu+ya)
            ya -= 0.001
            cnt += 1
            if cnt == 2:
                cnt == 0
                turtle.tracer(True)
                turtle.tracer(False)
            if (int(xu),int(yu))in rz:
                return False
            elif (int(xu),int(yu))in ground:
                condition = boom_on_ground(pen)
                if condition:
                    print('Outch!So close.')
                    return 'rb'
                break
            elif xu > 400:
                break
    else:
        spgo(pen,rightx,righty+10)
        xa = - cos(rad(angle))
        ya = sin(rad(angle))
        pen.color('blue')
        pen.pendown()
        while True:
            xu = pen.xcor()
            yu = pen.ycor()
            pen.goto(xu+xa,yu+ya)
            ya -= 0.001
            cnt += 1
            if cnt == 2:
                cnt == 0
                turtle.tracer(True)
                turtle.tracer(False)
            if (int(xu),int(yu))in lz:
                return True
            elif (int(xu),int(yu))in ground:
                condition = boom_on_ground(pen)
                if condition == False:
                    print('Outch!So close.')
                    return 'lb'
                break
            elif xu < -400:
                break
    return None
def main():
    global ground
    global lz,rz
    turtle.bgcolor('black')
    l = turtle.Pen()
    r = turtle.Pen()
    l.shape('turtle')
    r.shape('turtle')
    CannonBall = turtle.Pen()
    drawer = turtle.Pen()
    turtle.tracer(False)
    CannonBall.hideturtle()
    spgo(drawer,-400,0)
    y = 0
    plus = 0
    for x in range(-400,401):
        if plus < 2 and plus > -2:
            plus += 1 if randint(0,1)else -1
        elif plus == 2:
            if randint(0,1):
                plus -= 1
        else:
            if randint(0,1):
                plus += 1
        y += plus
        ground.append((x,y))
        ground.append((x+1,y))
        ground.append((x,y+1))
        ground.append((x,y-1))
    for x,y in ground[::16]:
        drawer.goto(x,y)
    drawer.hideturtle()
    ll = randint(100,600)
    rl = randint(-600,-100)
    spgo(l,ground[ll][0],ground[ll][1])
    spgo(r,ground[rl][0],ground[rl][1])
    for x in range(-10,11,1):
        for y in range(-10,11,1):
            lz.append((ground[ll][0]+x,
                       ground[ll][1]+y))
            rz.append((ground[rl][0]+x,
                       ground[rl][1]+y))
    LFT = True
    turtle.tracer(True)
    turtle.bgcolor('white')
    for t in range(3,0,-1):
        print(t,end='  ')
        wait(1)
    print('Go!',end='\n')
    ts = tm()
    lb = 2
    rb = 2
    while True:
        flag = fire(LFT,
                    ground[ll][0],ground[ll][1],
                    ground[rl][0],ground[rl][1],
                    CannonBall)
        turtle.tracer(True)
        if isinstance(flag,str):
            rb -= 1 if flag == 'rb'else 0
            lb -= 1 if flag == 'lb'else 0
            if rb == 0:
                print('Left wins!')
                break
            if lb == 0:
                print('Right wins!')
                break
        else:
            if flag:
                print('Right wins!')
                break
            else:
                if flag == False:
                    print('Left wins!')
                    break
        LFT = not LFT
    print('Game over,time: %d s.'%int(tm()-ts))
    turtle.done()
if __name__ == '__main__':
    main()
                
