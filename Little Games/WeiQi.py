import turtle
from random import randint

def find(obj,tar):
    index = 0
    for item in obj:
        if item == tar:
            return index
        index += 1
    return -1

class Dict():
    def __init__(self):
        self.vector = []
        self.mirror = []
        self.maxn = 0
        self.maxindex = 0
    def append(self,val):
        self.findv = find(self.vector,val)
        if self.findv == -1:
            self.vector.append(val)
            self.mirror.append(1)
        else:
            self.mirror[self.findv] += 1
        del self.findv
    def readv(self):
        return self.vector
    def readm(self):
        return self.mirror
    def findmax(self):
        self.maxn = 0
        self.index = 0
        self.maxindex = 0
        for item in self.mirror:
            if item > self.maxn:
                self.maxn = item
                self.maxindex = self.index
            self.index += 1
        del self.index
        return self.vector[self.maxindex]

class Advancedl():
    def __init__(self):
        self.vector = []
        self.lenth = 0
        self.connected = True
    def append(self,val):
        self.copyv = val
        self.lenthv = len(val)
        for endindex in range(0,self.lenthv,1):
            for index in range(0,self.lenthv-endindex-1,1):
                if self.copyv[index] > self.copyv[index+1]:
                    self.copyv[index],self.copyv[index+1] = \
                    self.copyv[index+1],self.copyv[index]
        if not self.copyv in self.vector:
            self.vector.append(self.copyv)
            self.lenth += 1
            self.connected = False
        del self.copyv,self.lenthv
    def connect(self):
        self.indexl = 0
        for line in self.vector:
            self.indexi = 0
            for tar in self.vector:
                for item in line:
                    if (item in tar) and (self.indexl != self.indexi):
                        self.vector[self.indexl] = \
                        self.vector[self.indexl]+self.vector[self.indexi]
                        self.vector.pop(self.indexi)
                        self.lenth -= 1
                        self.connected = False
                        del self.indexl,self.indexi
                        return False
                self.indexi += 1
            self.indexl += 1
        else:
            del self.indexl,self.indexi
            self.copies = []
            for line in self.vector:
                self.copyv = []
                for item in line:
                    if not item in self.copyv:
                        self.copyv.append(item)
                self.lenthv = len(self.copyv)
                for endindex in range(0,self.lenthv,1):
                    for index in range(0,self.lenthv-endindex-1,1):
                        if self.copyv[index] > self.copyv[index+1]:
                            self.copyv[index],self.copyv[index+1] = \
                            self.copyv[index+1],self.copyv[index]
                self.copies.append(self.copyv)
            self.connected = True
            self.vector = []
            for line in self.copies:
                self.addline = []
                for item in line:
                    self.addline.append(item)
                self.vector.append(self.addline)
            del self.addline,self.copyv,self.copies
            return True
    def read(self):
        return self.vector

def scan(qmap,xsize,ysize):
    qmapcopy = []
    qmapmirror = []
    connections = Advancedl()
    for line in qmap:
        lineinfoc = []
        lineinfom = []
        for item in line:
            lineinfoc.append(item)
            lineinfom.append(True)
        qmapcopy.append(lineinfoc)
        qmapmirror.append(lineinfom)
    ##print(qmapcopy,qmapmirror)
    del lineinfoc,lineinfom
    for yindex in range(1,ysize+1,1):
        for xindex in range(1,xsize+1,1):
            qs = qmapcopy[yindex][xindex]
            qu = qmapcopy[yindex+1][xindex]
            qd = qmapcopy[yindex-1][xindex]
            qr = qmapcopy[yindex][xindex+1]
            ql = qmapcopy[yindex][xindex-1]
            if qu!=0 and qd!=0 and ql!=0 and qr!=0:
                qmapmirror[yindex][xindex] = False
            if qu==qs!=0:
                connections.append([[yindex,xindex,qs],\
                                    [yindex+1,xindex,qs]])
            if qd==qs!=0:
                connections.append([[yindex,xindex,qs],\
                                    [yindex-1,xindex,qs]])
            if qr==qs!=0:
                connections.append([[yindex,xindex,qs],\
                                    [yindex,xindex+1,qs]])
            if ql==qs!=0:
                connections.append([[yindex,xindex,qs],\
                                    [yindex,xindex-1,qs]])
            if qs!=0:
                connections.append([[yindex,xindex,qs]])
    loop = True
    while loop:
        loop = not connections.connect()
    qmapmirroral = connections.read()
    for line in qmapmirroral:
        for ypos,xpos,_ in line:
            if qmapmirror[ypos][xpos] == True:
                break
        else:
            for ypos,xpos,_ in line:
                qmapcopy[ypos][xpos] = 0
    ##print(connections.read())
    return qmapcopy

def unpackline(string):
    unpacked = []
    appendline = []
    item = ''
    itemnum = 0
    for char in string:
        if char==',' and item!='':
            appendline.append(int(item))
            item = ''
            itemnum += 1
        elif char != '\n':
            item = item+char
        if itemnum == 3:
            unpacked.append(appendline)
            appendline = []
            itemnum = 0
    return unpacked

class Qmap():
    def __init__(self,xsize=5,ysize=5):
        self.vector = [[0 for _ in range(xsize+2)]for _ in range(ysize)]
        for ypos in range(ysize):
            self.vector[ypos][0] = 1
            self.vector[ypos][-1] = 1
        self.vector.insert(0,list(1 for _ in range(xsize+2)))
        self.vector.append(list(1 for _ in range(xsize+2)))
        self.vector[0][0] = 0
        ##print(self.vector)
        self.xsize = xsize
        self.ysize = ysize
    def step(self,xpos,ypos,val):
        if self.vector[ypos+1][xpos+1] == 0:
            self.vector[ypos+1][xpos+1] = val
            self.vector = scan(self.vector,self.xsize,self.ysize)
            return True
        ##print(self.vector)
        return False
    def read(self):
        return self.vector

def kickconv(conv):
    kicks = []
    index = 0
    for line in conv:
        for item in line:
            if item == 2:
                copy = []
                for linec in conv:
                    appendline = []
                    for itemc in line:
                        appendline.append(item)
                    copy.append(appendline)
                copy[index//3][index%3] = 3
                kicks.append(copy)
    return kicks

def compconv(conva,convb):
    for yindex in (0,1,2):
        for xindex in (0,1,2):
            if conva[yindex][xindex] != \
               convb[yindex][xindex]:
                return yindex,xindex
                        
        
def drewstep(pen,x,y,maincolor):
    pen.goto(x*10,y*10-5)
    pen.color(maincolor,maincolor)
    pen.pendown()
    pen.begin_fill()
    pen.circle(5)
    pen.end_fill()
    pen.penup()

def dline(pen,startx,starty,endx,endy):
    pen.penup()
    pen.goto(startx,starty)
    pen.pendown()
    pen.goto(endx,endy)
    pen.penup()

def canvsetup(pen,xlenth,ylenth):
    pen.color((0,0,0),(0,0,0))
    for ypos in range(0,ylenth*10,10):
        dline(pen,-10,ypos,xlenth*10,ypos)
    for xpos in range(0,xlenth*10,10):
        dline(pen,xpos,-10,xpos,ylenth*10)

def getcolor(colorv,rate):
    maincolor = int(rate/colorv*255)
    maincolor = (maincolor,maincolor,maincolor)
    return maincolor
 
class Drawer():
    def __init__(self,pen,root,xlenth,ylenth,pmax):
        self.vector = []
        self.xlenth = xlenth
        self.ylenth = ylenth
        self.pmax = pmax
        self.pen = pen
        self.root = root
    def append(self,x,y,rate):
        self.maincolor = getcolor(self.pmax,rate)
        self.vector.append([x,y,self.maincolor])
    def refreash(self):
        self.pen.clear()
        canvsetup(self.pen,self.xlenth,self.ylenth)
        for x,y,maincolor in self.vector:
            drewstep(self.pen,x,y,maincolor)
        self.root.update()
    def mapupdate(self,val):
        self.valcopy = val
        self.vector = []
        self.yindex = 0
        for line in self.valcopy[1:-1:1]:
            self.xindex = 0
            for item in line[1:-1:1]:
                if item >= 2:
                    self.vector.append([self.xindex,\
                                        self.yindex,\
                                        getcolor(self.pmax,item-2)])
                self.xindex += 1
            self.yindex += 1
        del self.xindex,self.yindex,self.valcopy

class Autostudy():
    def __init__(self,selfval):
        self.selfval = selfval
        self.convs = []
        self.basemap = (((-1,-1),(0,-1),(1,-1)),\
                        ((-1,0) ,(0,0) ,(1,0) ),\
                        ((-1,1) ,(0,1) ,(1,1) ))
        self.doinfo = []
        with open('info.txt','r') as f:
            self.line = f.readline()
            while self.line:
                self.doinfo.append(unpackline(self.line))
                self.line = f.readline()
        del self.line
    def study(self,qmap):
        self.xlenth = len(qmap[0])-2
        self.ylenth = len(qmap)-2
        self.studymap = []
        for line in qmap[1:-1:1]:
            self.appendline = []
            for item in line[1:-1:1]:
                if item >= 2:
                    if item == self.selfval:
                        self.appendline.append(2)
                    else:
                        self.appendline.append(1)
                else:
                    self.appendline.append(0)
            self.studymap.append(self.appendline)
        self.yindex = 1
        while self.yindex <= self.ylenth-2:
            self.xindex = 1
            while self.xindex <= self.xlenth-2:
                self.conv = []
                for line in self.basemap:
                    self.convl = []
                    for xplus,yplus in line:
                        self.convl.append(qmap[self.xindex+xplus]\
                                          [self.yindex+yplus])
                    self.conv.append(self.convl)
                if not self.conv in self.convs:
                    self.convs.append(self.conv)
                self.xindex += 1
            self.yindex += 1
        del self.xlenth,self.ylenth
        del self.xindex,self.yindex
        del self.appendline,self.conv,self.convl
    def copytofile(self):
        with open('info.txt','a') as f:
            for conv in self.convs:
                for line in conv:
                    for item in line:
                        f.write(str(item)+',')
                f.write('\n')
    def do(self,qmap):
        self.xlenth = len(qmap[0])-2
        self.ylenth = len(qmap)-2
        self.qmapcopy = []
        for line in qmap[1:-1:1]:
            self.appendline = []
            for item in qmap[1:-1:1]:
                self.appendline.append(item)
            self.qmapcopy.append(self.appendline)
        self.yindex = 1
        self.stepxs = Dict()
        self.stepys = Dict()
        self.steps  = Dict()
        while self.yindex <= self.ylenth-2:
            self.xindex = 1
            while self.xindex <= self.xlenth-2:
                self.conv = []
                for line in self.basemap:
                    self.convl = []
                    for xplus,yplus in line:
                        self.convl.append(qmap[self.xindex+xplus]\
                                          [self.yindex+yplus])
                    self.conv.append(self.convl)
                for line in self.doinfo:
                    if line in kickconv(self.conv):
                        self.xpos,self.ypos = compconv(line,\
                                                       self.conv)
                        ##self.stepxs.append(self.xpos+self.xindex)
                        ##self.stepys.append(self.ypos+self.yindex)
                        self.steps.append((self.xpos+self.xindex,\
                                           self.ypos+self.yindex))
                        break
                else:
                    ##self.stepxs.append(randint(-1,1)+self.xindex)
                    ##self.stepys.append(randint(-1,1)+self.yindex)
                    self.steps.append((randint(-1,1)+self.xindex,\
                                       randint(-1,1)+self.yindex))
                self.xindex += 1
            self.yindex += 1
        ##self.stepx = self.stepxs.findmax()
        ##self.stepy = self.stepys.findmax()
        self.stepx,self.stepy = self.steps.findmax()
        ##del self.xpos,self.ypos    
        del self.xlenth,self.ylenth
        del self.xindex,self.yindex
        del self.stepxs,self.stepys
        del self.appendline,self.conv,self.convl
        return self.stepx,self.stepy
        
def check(qmap):
    players = Dict()
    for line in qmap[1:-1:1]:
        for item in line[1:-1:1]:
            if item >= 2:
                players.append(item-2)
    maxv = 0
    vals = players.readm()
    for val in vals:
        if val > maxv:
            maxv = val
    index = 0
    winners = []
    for val in vals:
        if val == maxv:
            winindex = str(players.readv()[index])
            print(winindex,end=',')
            winners.append(winindex)
        index += 1
    print(' WINS !')
    return winners
    
def main():
    turtle.colormode(255)
    turtle.bgcolor((0,255,0))
    turtle.tracer(False)
    t = turtle.Pen()
    t.hideturtle()
    t.penup()
    lenth = 9
    width = 9
    canvsetup(t,lenth,width)
    qmap = Qmap(lenth,width)
    study = False
    steps = 1
    playersnum = 2
    players = list(range(0,playersnum,1))
    dy = Drawer(t,turtle,\
                lenth,width,\
                playersnum-1)
    dy.mapupdate(qmap.read())
    side = 0
    while True:
        print('回合%d,%d方'%(steps,side))
        y = int(input('y='))
        x = int(input('x='))
        if y<0 or x<0:
            check(qmap.read())
            if study:
                a.copytofile()
            return None
        if qmap.step(x,y,side+2):
            add = 1
        else:
            add = 0
        if study:
            a.study(qmap.read())
        dy.mapupdate(qmap.read())
        dy.refreash()
        side += add
        if side == playersnum:
            side = 0
            steps += 1

if __name__ == '__main__':
    main()
