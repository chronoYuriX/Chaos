##CHCMR5.py
'''
CHCMR v7.0.1.
    --Bugs in your whiskers!
'''

#Dependencies
#-math
from math      import sin      as math_sin
from math      import cos      as math_cos
from math      import sqrt     as math_sqrt
#-typing
from typing    import Any      as type_any
from typing    import List     as type_list
from typing    import Dict     as type_dict
from typing    import Tuple    as type_tuple
from typing    import Union    as type_multi
from typing    import Final    as type_const
from typing    import final    as retoucher_final
#-tkinter
from tkinter   import Tk       as window
from tkinter   import Canvas   as canvas
from tkinter   import ALL
#-threading
from threading import Thread   as thread
#-functions
from functions import void_set

#Values
ERROR:type_const[float] = 1e-6
INF  :type_const[float] = 1e6

#Structures
class determinant:
    def __init__(self,\
                 info:type_list[\
                     type_list[\
                         type_multi[int,float]]] \
                 = [[]]) \
                 -> None:
        self.carrier:type_list[\
            type_list[\
                type_multi[int,float]]] \
                = info
        length:int = len(info)
        if length == 0:
            self.size:type_tuple[int,int] = (0,0)
        else:
            self.size:type_tuple[int,int] = (len(info[0]),length)
        self.value:str = void_set.VOID

    @staticmethod
    def evaluate(target:type_list[\
        type_list[\
            type_multi[\
                int,\
                float]]],\
                 cache:type_multi[\
                     type_dict[\
                         tuple,\
                         type_multi[int,float]],\
                     str] \
                 = void_set.VOID) \
                 -> type_multi[int,float]:
        if cache == void_set.VOID:
            cache:type_dict[\
                tuple,\
                type_multi[int,float]] \
                = {}
        tupledtarget:tuple = tuple(map(tuple,target))
        if tupledtarget in cache:
            return cache[tupledtarget]
        if len(target) == 1:
            return target[0][0]
        result:int = 0
        for index in range(len(target)):
            subtarget:type_list[\
                type_list[\
                    type_multi[int,float]]] \
                    = [row[:index] + row[index+1:] \
                       for row in target[1:]]
            subresult:int = determinant.evaluate(subtarget,cache)
            result += ((-1)**index) * target[0][index] * subresult
        cache[tupledtarget] = result
        return result
    def selfevaluate(self) -> None:
        self.value:int = determinant.evaluate(self.carrier,void_set.VOID)
        
class vector:
    def __init__(self,\
                 *coordinate:type_multi[int,float]) \
                 -> None:
        self.carrier:type_tuple[\
            type_multi[int,float]] \
            = coordinate
        self.dimension:int = len(coordinate)
        self.length:str = void_set.VOID

    def poly_with_tuple(self,\
                        info:type_tuple[\
                            type_multi[int,float]]):
        self.carrier:type_tuple[\
            type_multi[int,float]] \
            = info
        self.dimension:int = len(info)
        self.length:str = void_set.VOID

    def compute_length(self) -> float:
        summation:float = 0.0
        for num in self.carrier:
            summation += num ** 2
        self.length:float = math_sqrt(summation)
        return self.length

    @property
    def coorepre(self) -> type_list[type_multi[int,float]]:
        result:type_list[type_multi[int,float]] = []
        for item in self.carrier:
            result.append(item)
        return result
    @coorepre.setter
    def coorepre(self,*args:type_multi[int,float]) -> None:
        self.__init__(args)
    @coorepre.deleter
    def coorepre(self) -> None:
        pass

    @staticmethod
    def cross(*vectors:type_any) -> type_multi[int,float]:
        temporary:type_list[\
            type_list[\
                type_multi[int,float]]] \
                = []
        for vector_item in vectors:
            temporary.append(vector_item.carrier)
        return determinant.evaluate(temporary)

    def __mul__(self,other) -> type_multi[int,float]:
        summation:type_multi[int,float] = 0
        for index in range(0,min(self.dimension,other.dimension),1):
            summation += self.carrier[index] * other.carrier[index]
        return summation

    def __pow__(self,other) -> float:
        if self.dimension == other.dimension == 3:
            return vector(self.carrier[1] * other.carrier[2] - \
                          self.carrier[2] * other.carrier[1],  \
                         -self.carrier[0] * other.carrier[2] - \
                          self.carrier[2] * other.carrier[0],  \
                          self.carrier[0] * other.carrier[1] - \
                          self.carrier[1] * other.carrier[0])
        elif self.dimension == other.dimension == 7:
            raise ValueError('Why do you need this???')
        else:
            raise ValueError('Cannot find a non-trivial normal vector.')

    def compute(self,other,op:str = '+') -> type_any:
        if op in '+-%//':
            result:type_any = vector()
            arg:type_tuple[\
                type_multi[int,float]] \
                = tuple()
            if op == '+':
                for index in range(0,min(self.dimension,other.dimension),1):
                    arg += (self.carrier[index] + other.carrier[index],)
            elif op == '-':
                for index in range(0,min(self.dimension,other.dimension),1):
                    arg += (self.carrier[index] - other.carrier[index],)
            elif op == '%':
                for index in range(0,min(self.dimension,other.dimension),1):
                    arg += (self.carrier[index] % other.carrier[index],)
            elif op == '/':
                for index in range(0,min(self.dimension,other.dimension),1):
                    arg += (self.carrier[index] / other.carrier[index],)
            elif op == '//':
                for index in range(0,min(self.dimension,other.dimension),1):
                    arg += (self.carrier[index] // other.carrier[index],)
            result.poly_with_tuple(arg)
            return result

    def __add__(self,other):
        return self.compute(other,'+')
    def __sub__(self,other):
        return self.compute(other,'-')
    def __truediv__(self,other):
        return self.compute(other,'/')
    def __floordiv__(self,other):
        return self.compute(other,'//')

    def copy(self,op:str = '+') -> type_any:
        result:type_any = vector()
        arg:type_tuple[\
                type_multi[int,float]] \
                = tuple()
        if op == '+':
            for index in range(0,self.dimension,1):
                arg += (self.carrier[index],)
        elif op == '-':
            for index in range(0,self.dimension,1):
                arg += (-self.carrier[index],)
        result.poly_with_tuple(arg)
        return result
    def __pos__(self):
        return self.copy('+')
    def __neg__(self):
        return self.copy('-')
    def __invent__(self):
        return self.copy('-')
    def __abs__(self):
        if self.length == void_set.VOID:
            return self.compute_length
        return self.length

#2d renderer
class img:
    def __init__(self                                  ,\
                 resolution:type_tuple[int,int] = (0,0),\
                 background:type_tuple[int,int,int] = (0,0,0)) \
                 -> None:
        self.resolution:type_tuple[int,int] = resolution
        self.bgcolor:type_tuple[int,int,int] = background
        self.carrier:type_list[\
            type_list[\
                type_tuple[int,int,int]]] \
                = [[background for _ in range(resolution[0])] \
                               for _ in range(resolution[1])]
    def clear(self) -> None:
        self.carrier:type_list[\
            type_list[\
                type_tuple[int,int,int]]] \
                = [[self.bgcolor for _ in range(self.resolution[0])] \
                                 for _ in range(self.resolution[1])]

    @property
    def vectorc(self) \
        -> type_list[\
            type_list[\
                type_tuple[int,int,int]]]:
        return self.carrier
    @vectorc.setter
    def vectorc(self,\
                info:type_list[\
                    type_list[\
                        type_tuple[int,int,int]]]) \
                        -> None:
        self.carrier:type_list[\
            type_list[\
                type_tuple[int,int,int]]] \
                = info
        self.resolution:type_tuple[int,int] = (0,len(info))
        if len(info) > 0:
            self.resolution[0] = len(info[0])
    @vectorc.deleter
    def vectorc(self) -> None:
        pass

    @property
    def pix(self) -> None:
        pass
    @pix.setter
    def pix(self,info) -> None:
        self.carrier[info[1]][info[0]] = info[2]
    @pix.deleter
    def pix(self) -> None:
        pass

    def cast(self ,\
             other,\
             startpos_root:type_tuple[int,int] = (0,0),\
             endpos_root  :type_tuple[int,int] = (0,0),\
             startpos_mask:type_tuple[int,int] = (0,0),\
             endpos_mask  :type_tuple[int,int] = (0,0)) \
             -> None:
        xmax:int = min(other.resolution[0] + startpos_root[0],\
                       self.resolution[0]                    ,\
                       endpos_root[0]                        ,\
                       startpos_root[0] + endpos_mask[0])
        ymax:int = min(other.resolution[1] + startpos_root[1],\
                       self.resolution[1]                    ,\
                       endpos_root[1]                        ,\
                       startpos_root[1] + endpos_mask[1])
        y_mask:int = startpos_mask[1]
        for y_root in range(startpos_root[1],ymax+1,1):
            x_mask:int = startpos_mask[0]
            for x_root in range(startpos_root[0],xmax+1,1):
                if other.carrier[y_mask][x_mask] != void_set.VOID:
                    self.carrier[y_root][x_root] = other.carrier[y_mask][x_mask]
                x_mask += 1
            y_mask += 1
    def cast_auto(self ,\
                  other,\
                  startpos:type_tuple[int,int] = (0,0)) \
                  -> None:
        xmax:int = other.resolution[0] + startpos[0]
        if xmax > self.resolution[0]:
            xmax:int = self.resolution[0]
        ymax:int = other.resolution[1] + startpos[1]
        if ymax > self.resolution[1]:
            ymax:int = self.resolution[1]
        y_copy:int = 0
        for y in range(startpos[1],ymax+1,1):
            x_copy:int = 0
            for x in range(startpos[0],xmax+1,1):
                if other.carrier[y_copy][x_copy] != void_set.VOID:
                    self.carrier[y][x] = other.carrier[y_copy][x_copy]
                x_copy += 1
            y_copy += 1

class GUI:
    def __init__(self                                ,\
                 size:type_tuple[int,int] = (100,100),\
                 bgcolor:type_tuple[int,int,int] = (0,0,0)) \
                 -> None:
        self.size:type_tuple[int,int] = size
        self.window:type(window) = window()
        self.window.geometry('%dx%d' % (size[0]+2,size[1]+2))
        self.window.title('Video Output')
        self.bgcolor:type_tuple[int,int,int] = bgcolor
        self.canvas:type(canvas) = canvas(self.window     ,\
                                          width = size[0] ,\
                                          height = size[1],\
                                          bg = '#{:02X}{:02X}{:02X}'.format\
                                          (bgcolor[0],bgcolor[1],bgcolor[2]))
        self.canvas.place(x=0,y=0)

    def line(self,\
             start:type_tuple[int,int],\
             end  :type_tuple[int,int],\
             color:type_tuple[int,int,int]) \
             -> None:
        self.canvas.create_line(start[0],start[1],end[0],end[1],\
                                fill = '#{:02X}{:02X}{:02X}'.format\
                                (color[0],color[1],color[2]))
    def polygon(self,points,color):
        self.canvas.create_polygon(points,\
                                   fill = '#{:02X}{:02X}{:02X}'.format\
                                   (color[0],color[1],color[2]))
    def clear(self):
        self.canvas.delete(ALL)
    def refresh(self):
        self.canvas.update()

    def setimg(self,img):
        if self.size == img.resolution:
            self.clear()
            y:int = 0
            for line in img.carrier:
                startx:int = 0
                x:int = 0
                color:type_tuple[int,int,int] = self.bgcolor
                for item in line:
                    if color != item:
                        if color == void_set.VOID:
                            self.line((startx,y),(x+1,y),self.bgcolor)
                            color:type_tuple[int,int,int] = self.bgcolor
                        else:
                            self.line((startx,y),(x+1,y),color)
                            color:type_tuple[int,int,int] = item
                        startx:int = x
                    x += 1
                self.line((startx,y),(x+1,y),color)
                y += 1

#3d renderer
class vector3d(vector):
    def __init__(self,\
                 *coordinate:type_multi[int,float]) \
                 -> None:
        self.carrier:type_tuple[\
            type_multi[int,float]] \
            = coordinate
        self.dimension:int = len(coordinate)
        self.compute_length()

    def poly_with_tuple(self,\
                        info:type_tuple[\
                            type_multi[int,float],\
                            type_multi[int,float],\
                            type_multi[int,float]]):
        self.carrier:type_tuple[\
            type_multi[int,float],\
            type_multi[int,float],\
            type_multi[int,float]] \
            = info
        self.dimension:int = 3
        self.compute_length()

    @staticmethod
    def cross(o,a,b) -> type_multi[int,float]:
        return (a.carrier[0] - o.carrier[0]) * (b.carrier[1] - o.carrier[1]) - \
               (b.carrier[0] - o.carrier[0]) * (a.carrier[1] - o.carrier[1])
    def __sub__(self,other):
        return vector3d(self.carrier[0] - other.carrier[0],\
                        self.carrier[1] - other.carrier[1],\
                        self.carrier[2] - other.carrier[2])
    def __mul__(self,other) -> type_multi[int,float]:
        return self.carrier[0] * other.carrier[0] + \
               self.carrier[1] * other.carrier[1] + \
               self.carrier[2] * other.carrier[2]
    def __pow__(self,other) -> float:
        return vector3d(self.carrier[1] * other.carrier[2] - \
                        self.carrier[2] * other.carrier[1],  \
                       -self.carrier[0] * other.carrier[2] - \
                        self.carrier[2] * other.carrier[0],  \
                        self.carrier[0] * other.carrier[1] - \
                        self.carrier[1] * other.carrier[0])
    def compute(self) -> None:
        pass

class triangle:
    def __init__(self      ,\
                 p1 = (0,0,0),\
                 p2 = (0,0,0),\
                 p3 = (0,0,0),\
                 color = (255,255,255)):
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3
        self.color = color
        va = vector3d()
        vb = vector3d()
        vc = vector3d()
        va.poly_with_tuple(p1)
        vb.poly_with_tuple(p2)
        vc.poly_with_tuple(p3)
        vab = vb - va
        vac = vc - va
        self.surface = vab ** vac

    @property
    def tpos(self):
        return (self.p1,self.p2,self.p3)
    @tpos.setter
    def tpos(self,info):
        self.__init__(info[0],info[1],info[2],self.color)
    @tpos.deleter
    def tpos(self):
        pass

class triangles:
    def __init__(self):
        self.vector = []
        self.length = 0
    def __len__(self):
        return self.length

    def split(self,obj,size,color):
        if size < 4:
            return [obj]
        result = []
        root = obj[0]
        for index in range(1,size-1,1):
            copy = triangle(root,obj[index],obj[index+1],color)
            result.append(copy)
        return result
    def addsurface(self,item,color=(255,255,255)):
        size = len(item)
        if size < 3:
            return False
        for obj in self.split(item,size,color):
            self.vector.append(obj)
            self.length += 1
    def addtriangle(self,item):
        self.vector.append(item)
        self.length += 1
    def clear(self):
        self.vector = []
        self.length = 0

class renderer_BASE:
    def summon_GUI(self):
        self.GUI = GUI(self.resolution,self.bgcolor)
    def summon_IMG(self):
        self.img = img(self.resolution,self.bgcolor)
    def __init__(self                           ,\
                 resolution = (100,100)         ,\
                 magnify = 2                    ,\
                 bgcolor = (0,0,0)              ,\
                 maxrenderrange = 1e3           ,\
                 lightdirections = ((1,-2,3,1),),\
                 backgroundlight = 0.15):
        self.shift_x = resolution[0] // 2
        self.shift_y = resolution[1] // 2
        self.resolution = resolution
        self.magnify = magnify
        self.bgcolor = bgcolor
        self.summon_IMG()
        self.maxrenderrange = maxrenderrange
        self.seekv  = vector3d(0,0,1)
        self.lightvs = []
        self.lightis = []
        for lightx,lighty,lightz,lightintensity in lightdirections:
            self.lightvs.append(vector3d(lightx,lighty,lightz))
            self.lightis.append(lightintensity)
        self.lightns = len(lightdirections)
        self.backgroundlight = backgroundlight
        self.summon_GUI()

    @property
    def heading(self):
        return (self.camera_angle,self.camera_pitch)
    @heading.setter
    def heading(self,val):
        self.camera_angle = val[0]
        self.camera_pitch = val[1]
        seekx = math_sin(val[0])
        seeky = math_sin(val[1])
        seekz = math_cos(val[0]) * math_cos(val[1])
        self.seekv = vector3d(seekx,seeky,seekz)
    @heading.deleter
    def heading(self):
        pass

    @property
    def ir(self):
        return self.img.vectorc

    @staticmethod
    def posshift(pos = (0,0,0)   ,\
                 camera_angle = 0,\
                 camera_pitch = 0,\
                 magnify = 1):
        x,y,z = pos
        sa = math_sin(camera_angle)
        ca = math_cos(camera_angle)
        sp = math_sin(camera_pitch)
        cp = math_cos(camera_pitch)
        temporary = sa*z + ca*x
        z = ca*z - sa*x
        x = temporary
        temporary = sp*z + cp*y
        z = cp*z - sp*y
        y = temporary
        return (x*magnify,y*magnify,z*magnify)

    def clear(self,clearimg = True):
        if clearimg:
            self.img.clear()
        self.GUI.clear()

    def compute_pos(self,obj):
        return (renderer_BASE.posshift(obj.p1           ,\
                                       self.camera_angle,\
                                       self.camera_pitch,\
                                       self.magnify)    ,\
                renderer_BASE.posshift(obj.p2           ,\
                                       self.camera_angle,\
                                       self.camera_pitch,\
                                       self.magnify)    ,\
                renderer_BASE.posshift(obj.p3           ,\
                                       self.camera_angle,\
                                       self.camera_pitch,\
                                       self.magnify))

    def compute_color(self,obj):
        colorr,colorg,colorb = obj.color
        lightval = 0
        for index in range(0,self.lightns,1):
            lightval += -(self.lightvs[index] * obj.surface) / \
                        self.lightvs[index].length / \
                        obj.surface.length * \
                        self.lightis[index]
        lightval = 0 if lightval < 0 else lightval
        lightval += self.backgroundlight
        colorr = int(colorr * lightval)
        colorg = int(colorg * lightval)
        colorb = int(colorb * lightval)
        colorr = 255 if colorr>255 else colorr
        colorg = 255 if colorg>255 else colorg
        colorb = 255 if colorb>255 else colorb
        colorr = 0 if colorr<0 else colorr
        colorg = 0 if colorg<0 else colorg
        colorb = 0 if colorb<0 else colorb
        return (colorr,colorg,colorb)

    def renderobj(self,obj,rangex,rangey,zs,windowed=False):
        a,b,c = self.compute_pos(obj)
        x1,y1,z1 = a
        x2,y2,z2 = b
        x3,y3,z3 = c
        va = vector3d(x1,y1)
        vb = vector3d(x2,y2)
        vc = vector3d(x3,y3)
        A = ((y2-y1) * (z3-z1)) - ((z2-z1) * (y3-y1))
        B = ((x3-x1) * (z2-z1)) - ((x2-x1) * (z3-z1))
        C = ((x2-x1) * (y3-y1)) - ((x3-x1) * (y2-y1))
        s_abc = abs(vector3d.cross(va,vb,vc))
        color = self.compute_color(obj)
        local_vector = vector3d()
        for y in range(int(max(min(a[1],b[1],c[1])         ,\
                               rangey[0] - self.shift_y))  ,\
                       int(min(max(a[1],b[1],c[1])         ,\
                               rangey[1] - self.shift_y + 1,\
                               self.shift_y)) + 1          ,\
                       1):
            yp = y + self.shift_y
            if windowed:
                startx = 0
                started = False
                not_started = True
            for x in range(int(max(min(a[0],b[0],c[0])         ,\
                                   rangex[0] - self.shift_x))  ,\
                           int(min(max(a[0],b[0],c[0])         ,\
                                   rangex[1] - self.shift_x + 1,\
                                   self.shift_x)) + 1          ,\
                           1):
                z = INF if C == 0 else (A*(x1-x) + B*(y1-y)) / C + z1
                xp = x + self.shift_x
                local_vector.poly_with_tuple((x,y))
                if -ERROR < s_abc - \
                   abs(vector3d.cross(local_vector,va,vb)) - \
                   abs(vector3d.cross(local_vector,vb,vc)) - \
                   abs(vector3d.cross(local_vector,vc,va)) < ERROR \
                   and z < zs[yp][xp]:
                    self.img.pix = (xp,yp,color)
                    zs[yp][xp] = z
                    if windowed and not_started:
                        startx = xp
                        started = True
                        not_started = False
                elif windowed and started:
                    started = False
                    not_started = True
                    self.GUI.line((startx,yp),(xp+1,yp),color)
            if windowed and started:
                self.GUI.line((startx,yp),(xp+1,yp),color)

    def renderobjs(self             ,\
                   objs = triangles(),\
                   rangex = (0,100) ,\
                   rangey = (0,100)):
        zs = [[self.maxrenderrange for _ in range(self.resolution[0])] \
                                   for _ in range(self.resolution[1])]
        for obj in objs.vector:
            if obj.surface * self.seekv < 0:
                self.renderobj(obj,rangex,rangey,zs,True)
        ##self.GUI.refresh()

    def renderMULT(self             ,\
                   objs = triangles(),\
                   rangex = (0,100) ,\
                   rangey = (0,100)):
        zs = [[self.maxrenderrange for _ in range(self.resolution[0])] \
                                   for _ in range(self.resolution[1])]
        threads = []
        for obj in objs.vector:
            if obj.surface * self.seekv < 0:
                threads.append(thread(target = self.renderobj,\
                                      args = (obj,rangex,rangey,zs,False)))
                threads[-1].start()
        for current_thread in threads:
            current_thread.join()
        self.GUI.setimg(self.img)
        ##self.GUI.refresh()

    def renderFAST(self,objs):
        zl = []
        posl = []
        indexl = []
        indexo = length = 0
        for obj in objs.vector:
            if obj.surface * self.seekv < 0:
                a,b,c = self.compute_pos(obj)
                z = a[2] + b[2] + c[2]
                if length == 0:
                    zl.append(z)
                    posl.append((a[0],a[1],b[0],b[1],c[0],c[1]))
                    indexl.append(indexo)
                    firstadd = False
                    length = 1
                    indexo += 1
                    continue
                for indexi in range(0,length,1):
                    if z < zl[indexi]:
                        zl.insert(indexi,z)
                        posl.insert(indexi,(a[0],a[1],b[0],b[1],c[0],c[1]))
                        indexl.insert(indexi,indexo)
                        break
                else:
                    zl.append(z)
                    posl.append((a[0],a[1],b[0],b[1],c[0],c[1]))
                    indexl.append(indexo)
                length += 1
            indexo += 1
        indexo = 0
        for index in indexl:
            self.GUI.polygon((posl[indexo][0] + self.shift_x,\
                              posl[indexo][1] + self.shift_y,\
                              posl[indexo][2] + self.shift_x,\
                              posl[indexo][3] + self.shift_y,\
                              posl[indexo][4] + self.shift_x,\
                              posl[indexo][5] + self.shift_y),\
                              self.compute_color(objs.vector[index]))
            indexo += 1
        ##self.GUI.refresh()

class renderer_nGUI(renderer_BASE):
    def summon_GUI(self):
        pass
    def renderFAST(self):
        pass
    def clear(self):
        self.img.clear()

    def renderobj(self,obj,rangex,rangey,zs):
        a,b,c = self.compute_pos(obj)
        x1,y1,z1 = a
        x2,y2,z2 = b
        x3,y3,z3 = c
        va = vector3d(x1,y1)
        vb = vector3d(x2,y2)
        vc = vector3d(x3,y3)
        A = ((y2-y1) * (z3-z1)) - ((z2-z1) * (y3-y1))
        B = ((x3-x1) * (z2-z1)) - ((x2-x1) * (z3-z1))
        C = ((x2-x1) * (y3-y1)) - ((x3-x1) * (y2-y1))
        s_abc = abs(vector3d.cross(va,vb,vc))
        color = self.compute_color(obj)
        local_vector = vector3d()
        for y in range(int(max(min(a[1],b[1],c[1])         ,\
                               rangey[0] - self.shift_y))  ,\
                       int(min(max(a[1],b[1],c[1])         ,\
                               rangey[1] - self.shift_y + 1,\
                               self.shift_y)) + 1          ,\
                       1):
            yp = y + self.shift_y
            for x in range(int(max(min(a[0],b[0],c[0])         ,\
                                   rangex[0] - self.shift_x))  ,\
                           int(min(max(a[0],b[0],c[0])         ,\
                                   rangex[1] - self.shift_x + 1,\
                                   self.shift_x)) + 1          ,\
                           1):
                z = INF if C == 0 else (A*(x1-x) + B*(y1-y)) / C + z1
                xp = x + self.shift_x
                local_vector.poly_with_tuple((x,y))
                if -ERROR < s_abc - \
                   abs(vector3d.cross(local_vector,va,vb)) - \
                   abs(vector3d.cross(local_vector,vb,vc)) - \
                   abs(vector3d.cross(local_vector,vc,va)) < ERROR \
                   and z < zs[yp][xp]:
                    self.img.pix = (xp,yp,color)
                    zs[yp][xp] = z

    def renderobjs(self             ,\
                   objs = triangles(),\
                   rangex = (0,100) ,\
                   rangey = (0,100)):
        zs = [[self.maxrenderrange for _ in range(self.resolution[0])] \
                                   for _ in range(self.resolution[1])]
        for obj in objs.vector:
            if obj.surface * self.seekv < 0:
                self.renderobj(obj,rangex,rangey,zs)

    def renderMULT(self             ,\
                   objs = triangles(),\
                   rangex = (0,100) ,\
                   rangey = (0,100)):
        zs = [[self.maxrenderrange for _ in range(self.resolution[0])] \
                                   for _ in range(self.resolution[1])]
        threads = []
        for obj in objs.vector:
            if obj.surface * self.seekv < 0:
                threads.append(thread(target = self.renderobj,\
                                      args = (obj,rangex,rangey,zs)))
                threads[-1].start()
        for current_thread in threads:
            current_thread.join()

class renderer_nIMG(renderer_BASE):
    def summon_IMG(self):
        pass
    def clear(self):
        self.GUI.clear()

    def renderobj(self,obj,rangex,rangey,zs,windowed=False):
        a,b,c = self.compute_pos(obj)
        x1,y1,z1 = a
        x2,y2,z2 = b
        x3,y3,z3 = c
        va = vector3d(x1,y1)
        vb = vector3d(x2,y2)
        vc = vector3d(x3,y3)
        A = ((y2-y1) * (z3-z1)) - ((z2-z1) * (y3-y1))
        B = ((x3-x1) * (z2-z1)) - ((x2-x1) * (z3-z1))
        C = ((x2-x1) * (y3-y1)) - ((x3-x1) * (y2-y1))
        s_abc = abs(vector3d.cross(va,vb,vc))
        color = self.compute_color(obj)
        local_vector = vector3d()
        for y in range(int(max(min(a[1],b[1],c[1])         ,\
                               rangey[0] - self.shift_y))  ,\
                       int(min(max(a[1],b[1],c[1])         ,\
                               rangey[1] - self.shift_y + 1,\
                               self.shift_y)) + 1          ,\
                       1):
            yp = y + self.shift_y
            if windowed:
                startx = 0
                started = False
                not_started = True
            for x in range(int(max(min(a[0],b[0],c[0])         ,\
                                   rangex[0] - self.shift_x))  ,\
                           int(min(max(a[0],b[0],c[0])         ,\
                                   rangex[1] - self.shift_x + 1,\
                                   self.shift_x)) + 1          ,\
                           1):
                z = INF if C == 0 else (A*(x1-x) + B*(y1-y)) / C + z1
                xp = x + self.shift_x
                local_vector.poly_with_tuple((x,y))
                if -ERROR < s_abc - \
                   abs(vector3d.cross(local_vector,va,vb)) - \
                   abs(vector3d.cross(local_vector,vb,vc)) - \
                   abs(vector3d.cross(local_vector,vc,va)) < ERROR \
                   and z < zs[yp][xp]:
                    zs[yp][xp] = z
                    if windowed and not_started:
                        startx = xp
                        started = True
                        not_started = False
                elif windowed and started:
                    started = False
                    not_started = True
                    self.GUI.line((startx,yp),(xp+1,yp),color)
            if windowed and started:
                self.GUI.line((startx,yp),(xp+1,yp),color)

#test
if __name__ == '__main__':
    from time import time
    def test(have_window = False   ,\
             target = void_set.VOID,\
             exe = void_set.VOID   ,\
             CYCLE = 32            ,\
             clearimg = False      ,\
             add = (1e-2,-2e-3)):
        anglex = 0
        angley = 0
        objs = triangles()
        r = target((200,200))
        if have_window:
            r.GUI.window.title('FPS:Unknown')
        objs.clear()
        #front
        objs.addsurface([(-10,-10,-10),\
                         (-10, 10,-10),\
                         ( 10, 10,-10),\
                         ( 10,-10,-10)],\
                        (255,255,255))
        #left
        objs.addsurface([(-10,-10,-10),\
                         (-10, 10,-10),\
                         (-10, 10, 10),\
                         (-10,-10, 10)],\
                        (255,255,255))
        #top
        objs.addsurface([(-10,-10, 10),\
                         ( 10,-10, 10),\
                         ( 10,-10,-10),\
                         (-10,-10,-10)],\
                        (255,255,255))
        #back
        objs.addsurface([( 10, 10, 10),\
                         (-10, 10, 10),\
                         (-10,-10, 10),\
                         ( 10,-10, 10)],\
                        (255,255,255))
        #right
        objs.addsurface([( 10,-10, 10),\
                         ( 10, 10, 10),\
                         ( 10, 10,-10),\
                         ( 10,-10,-10)],\
                        (255,255,255))
        #bottom
        objs.addsurface([( 10, 10, 10),\
                         ( 10, 10,-10),\
                         (-10, 10,-10),\
                         (-10, 10, 10)],\
                        (255,255,255))
        t0 = time()
        while True:
            for _ in range(CYCLE):
                r.heading = (anglex,angley)
                if clearimg:
                    r.clear()
                else:
                    r.clear(False)
                exec(f'r.{exe}')
                anglex += add[0]
                angley += add[1]
                anglex %= 6.283185307179586
                angley %= 6.283185307179586
            t1 = time()
            if have_window:
                r.GUI.window.title('FPS: %.2f' % (1/(t1-t0)*CYCLE))
            else:
                print('FPS:',round(1/(t1-t0)*CYCLE,2))
            t0 = t1

    #starttest

    print('>>test for BASE.renderobj<<')
    test(True,\
         renderer_BASE,\
         'renderobjs(objs,(40,160),(40,160))',\
         8,\
         True,\
         (1e-2,-2e-3))

##    print('>>test for nGUI.renderobj<<')
##    test(False,\
##         renderer_nGUI,\
##         'renderobjs(objs,(40,160),(40,160))',\
##         16,\
##         True,\
##         (1e-2,-2e-3))

##    print('>>test for nIMG.renderobj<<')
##    test(True,\
##         renderer_nIMG,\
##         'renderobjs(objs,(40,160),(40,160))',\
##         8,\
##         True,\
##         (1e-2,-2e-3))

##    print('>>test for BASE.renderFAST<<')
##    test(True,\
##         renderer_BASE,\
##         'renderFAST(objs)',\
##         256,\
##         False,\
##         (1e-4,-2e-5))

##    print('>>test for nGUI.renderMULT<<')
##    test(False,\
##         renderer_nGUI,\
##         'renderMULT(objs,(40,160),(40,160))',\
##         32,\
##         True,\
##         (1e-2,-2e-3))

##    print('>>test for BASE.renderMULT<<')
##    test(True,\
##         renderer_BASE,\
##         'renderMULT(objs,(40,160),(40,160))',\
##         8,\
##         True,\
##         (1e-2,-2e-3))

    print('Done!')
