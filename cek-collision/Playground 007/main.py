from math import sin, cos, sqrt, pi

collision_min_depth = 1e-10
collision_correction_factor = 0.2
collision_parallel_error = 1e-2
default_inertia_mass2_rate = 2.0


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
    def getUnit(self):
        return self / self.getLen()
    def __str__(self):
        return "(%.2f, %.2f)" % (self.x, self.y)

class LINE:
    def __init__(self, A = POINT(0, 0), B = POINT(0, 0)):
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
    def getUnit(self):
        heading = self.B - self.A
        return heading / heading.getLen()
    def getVT(self):
        unit = self.getUnit()
        return POINT(-unit.y, unit.x)
    def getLen(self):
        return (self.B - self.A).getLen()
    def getDistance(self, P):
        return abs((self.B - self.A) ** (P - self.A)) / self.getLen()
    def isParallel(A1B1, A2B2):
        return abs(abs(A1B1.getUnit() | A2B2.getUnit()) - 1.0) < collision_parallel_error
    def isPerpendicul(A1B1, A2B2):
        return abs(A1B1.getUnit() | A2B2.getUnit()) < collision_parallel_error
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
    def __or__(P1, P2): # get collision info
        overlap_min = float("Inf")
        for edge in P1.getEdge() + P2.getEdge():
            axis = edge.getVT()
            min1, max1 = P1.projectPeak(axis)
            min2, max2 = P2.projectPeak(axis)
            if max1 < min2 or max2 < min1:
                return LINE(), LINE(), LINE()
            overlap = min(max1 - min2, max2 - min1)
            if (P2.center - P1.center) | axis < 0:
                axis = axis * -1
            if overlap < overlap_min:
                overlap_min = overlap
                axis_min = axis
        MTV = axis_min * overlap_min
        P1Shadows = [P | MTV for P in P1.points]
        P2Shadows = [P | MTV for P in P2.points]
        P1_far  = P1.points[P1Shadows.index(max(P1Shadows))]
        P2_near = P2.points[P2Shadows.index(min(P2Shadows))]
        contact = (P1_far + P2_near) / 2
        P1Edge_contact = []
        P2Edge_contact = []
        for edge in P1.getEdge():
            if edge.isPerpendicul(axis_min):
                P1Edge_contact.append(edge)
        for edge in P2.getEdge():
            if edge.isPerpendicul(axis_min):
                P2Edge_contact.append(edge)
        if len(P1Edge_contact) > 0 and len(P2Edge_contact) > 0:
            P1Edge_contact_near = min(P1Edge_contact, key = lambda K: K.getDistance(contact))
            P2Edge_contact_near = min(P2Edge_contact, key = lambda K: K.getDistance(contact))
            if P1Edge_contact_near.isParallel(P2Edge_contact_near): # Parallel detected
                edge_axis = P1Edge_contact_near.getUnit()
                P1_project_min = P1Edge_contact_near.A | edge_axis
                P1_project_max = P1Edge_contact_near.B | edge_axis
                if P1_project_min > P1_project_max:
                    P1_project_min, P1_project_max = P1_project_max, P1_project_min
                P2_project_min = P2Edge_contact_near.A | edge_axis
                P2_project_max = P2Edge_contact_near.B | edge_axis
                if P2_project_min > P2_project_max:
                    P2_project_min, P2_project_max = P2_project_max, P2_project_min
                overlap_min = max(P1_project_min, P2_project_min)
                overlap_max = min(P1_project_max, P2_project_max)
                if overlap_max > overlap_min:
                    M = (overlap_min + overlap_max) / 2
                    contact = P1Edge_contact_near.A + edge_axis * (M - (P1Edge_contact_near.A | edge_axis))
            return LINE(contact, contact + MTV), P1Edge_contact_near, P2Edge_contact_near
        return LINE(contact, contact + MTV), LINE(), LINE()
    def __add__(self, offset):
        new_points = [point + offset for point in self.points]
        return POLYGON(self.center + offset, new_points)
    def __sub__(self, offset):
        return self + offset * -1
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

class RIGID_BODY:
    def __init__(self, polygon, mass, inertia = 0):
        self.polygon = polygon
        self.mass = mass
        self.inertia = mass ** 2 * default_inertia_mass2_rate if inertia == 0 else inertia
        self.velocity = POINT(0, 0)
        self.angular_velocity = 0.0
    def force(self, impulse):
        radius = impulse.A - self.polygon.center
        self.velocity += (impulse.B - impulse.A) / self.mass
        self.angular_velocity += (radius ** (impulse.B - impulse.A)) / self.inertia
    def update(self, dt):
        self.polygon += self.velocity * dt
        self.polygon <<= self.angular_velocity * dt
    @staticmethod
    def rebound(B1, B2, collision, e):
        contact_point = collision.A
        normal = collision.B - collision.A
        depth = normal.getLen()
        if depth < collision_min_depth:
            return
        normal /= depth
        R1 = contact_point - B1.polygon.center
        R2 = contact_point - B2.polygon.center
        V1 = POINT(-R1.y, R1.x) * B1.angular_velocity + B1.velocity
        V2 = POINT(-R2.y, R2.x) * B2.angular_velocity + B2.velocity
        relative_velocity = V2 - V1
        vel_along_normal = relative_velocity | normal
        if vel_along_normal > 0:
            return
        inv_mass_sum = 1.0 / B1.mass + 1.0 / B2.mass
        R1_cross_n = R1 ** normal
        R2_cross_n = R2 ** normal
        inv_inertia_sum = R1_cross_n ** 2 / B1.inertia + R2_cross_n ** 2 / B2.inertia
        I = normal * ((1 + e) * vel_along_normal / (inv_mass_sum + inv_inertia_sum))
        B1.force(LINE(contact_point, contact_point + I))
        B2.force(LINE(contact_point, contact_point - I))
        correction = normal * depth * collision_correction_factor / (1.0 / B1.mass + 1.0 / B2.mass)
        B1.polygon -= correction / B1.mass
        B2.polygon += correction / B2.mass

class PHYSICS:
    def __init__(self):
        self.objs = []
        self.framecount = 0
    def additem(self, item):
        self.objs.append(item)
    
        
