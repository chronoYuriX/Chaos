from math import sin, cos, sqrt, pi


class POINT:
    def __init__(self, x = float("NaN"), y = float("NaN")):
        self.x, self.y = x, y
    def __add__(A, B):
        return POINT(A.x + B.x, A.y + B.y)
    def __sub__(A, B):
        return POINT(A.x - B.x, A.y - B.y)
    def __mul__(self, scale): # A * scale
        return POINT(self.x * scale, self.y * scale)
    def __or__(A, B): # A * B
        return A.x * B.x + A.y * B.y
    def __truediv__(self, scale):
        return POINT(self.x / scale, self.y / scale)
    def __pow__(A, B): # A x B
        return A.x * B.y - A.y * B.x
    def rotate(self, center, angle):
        offset_self = self - center
        offset_center = POINT(offset_self.x * cos(angle) - offset_self.y * sin(angle), \
                              offset_self.x * sin(angle) + offset_self.y * cos(angle))
        return center + offset_center
    def getLen(self):
        return sqrt(self.x ** 2 + self.y ** 2)
    def __str__(self):
        return "(%.2f, %.2f)" % (self.x, self.y)

class LINE:
    def __init__(self, A, B):
        self.A, self.B = A, B
    def cmp_x(self, func):
        return func(self.A.x, self.B.x)
    def cmp_y(self, func):
        return func(self.A.y, self.B.y)
    def __add__(self, offset):
        return LINE(self.A + offset, self.B + offset)
    def __and__(A1B1, A2B2): # A1B1 ∩ A2B2 != {}
        if any((A1B1.cmp_x(max) < A2B2.cmp_x(min), A1B1.cmp_y(max) < A2B2.cmp_y(min), \
                A2B2.cmp_x(max) < A1B1.cmp_x(min), A2B2.cmp_y(max) < A1B1.cmp_y(min))):
            return False
        C1 = (A1B1.A - A2B2.A) ** (A2B2.B - A2B2.A)
        C2 = (A1B1.B - A2B2.A) ** (A2B2.B - A2B2.A)
        C3 = (A2B2.A - A1B1.A) ** (A1B1.B - A1B1.A)
        C4 = (A2B2.B - A1B1.A) ** (A1B1.B - A1B1.A)
        return C1 * C2 <= 0 and C3 * C4 <= 0
    def __mul__(self, scale):
        return LINE(self.A * scale, self.B * scale)
    def rotate(self, center, angle):
        return LINE(self.A.rotate(center, angle) + self.B.rotate(center, angle))
    def getVT(self):
        heading = self.B - self.A
        heading /= heading.getLen()
        return POINT(-heading.y, heading.x)
    def getLen(self):
        return (self.B - self.A).getLen()
    def __str__(self):
        return str(self.A) + "--" + str(self.B)

class POLYGON:
    def __init__(self, center, *points):
        if type(points[0]) == list:
            self.points = points[0]
        else:
            self.points = list(points)
        self.center = center
    def getEdge(self):
        edge = []
        for i in range(len(self.points) - 1):
            edge.append(LINE(self.points[i], self.points[i + 1]))
        edge.append(LINE(self.points[-1], self.points[0]))
        return edge
    def __and__(P1, P2): # check collision
        P1Edge = P1.getEdge()
        P2Edge = P2.getEdge()
        for P1Edge_i in P1Edge:
            for P2Edge_j in P2Edge:
                if P1Edge_i & P2Edge_j:
                    return True
        return False
    def projectPeak(self, axis):
        shadows = [P | axis for P in self.points]
        return min(shadows), max(shadows)
    def getMTV(P1, P2):
        overlap_min = float("Inf")
        for edge in P1.getEdge() + P2.getEdge():
            axis = edge.getVT()
            min1, max1 = P1.projectPeak(axis)
            min2, max2 = P2.projectPeak(axis)
            if max1 < min2 or max2 < min1:
                return POINT(0, 0)
            overlap = min(max1 - min2, max2 - min1)
            if (P2.center - P1.center) | axis < 0:
                axis = axis * -1
            if overlap < overlap_min:
                overlap_min = overlap
                axis_min = axis
        return axis_min * overlap_min
    def __or__(P1, P2): # get collision info
        MTV = P1.getMTV(P2)
        if MTV.getLen() == 0:
            return LINE(POINT(0, 0), POINT(0, 0))
        P1Shadows = [P | MTV for P in P1.points]
        P2Shadows = [P | MTV for P in P2.points]
        P1_far  = P1.points[P1Shadows.index(max(P1Shadows))]
        P2_near = P2.points[P2Shadows.index(min(P2Shadows))]
        base = (P1_far + P2_near) / 2
        return LINE(base, base + MTV)
    def __add__(self, offset):
        new_points = [point + offset for point in self.points]
        return POLYGON(self.center + offset, new_points)
    def __lshift__(self, angle):
        new_points = [point.rotate(self.center, angle) for point in self.points]
        return POLYGON(self.center, new_points)
    def __rshift__(self, angle):
        return self << -angle
    def __str__(self):
        polygonStr = []
        for point in self.points:
            polygonStr.append(str(point))
        return "[-" + "--".join(polygonStr) + "-]"
