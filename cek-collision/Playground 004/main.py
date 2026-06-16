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

# 添加到 main.py

class RIGID_BODY:
    """刚体类，包含位置、速度、质量等物理属性"""
    def __init__(self, polygon, mass=1.0, inertia=None):
        self.polygon = polygon          # POLYGON对象
        self.mass = mass                # 质量
        self.inertia = inertia if inertia else mass * 10  # 转动惯量（默认值）
        
        # 运动状态
        self.velocity = POINT(0, 0)     # 线速度
        self.angular_velocity = 0.0     # 角速度（弧度/秒）
        
        # 临时缓存（用于碰撞响应）
        self.force_buffer = POINT(0, 0) # 累积的力
        self.torque_buffer = 0.0        # 累积的力矩
        
    def apply_impulse(self, impulse, contact_point):
        """
        在指定点施加一个瞬间冲量
        impulse: POINT，冲量向量
        contact_point: POINT，作用点（世界坐标）
        """
        # 计算从质心到作用点的向量
        r = contact_point - self.polygon.center
        
        # 线速度变化：Δv = J / m
        self.velocity += impulse * (1.0 / self.mass)
        
        # 角速度变化：Δω = (r × J) / I
        # 2D中叉积 r × J = r.x * J.y - r.y * J.x
        torque = r.x * impulse.y - r.y * impulse.x
        self.angular_velocity += torque / self.inertia
    
    def apply_continuous_force(self, force, dt):
        """
        施加持续力（重力、摩擦力等）
        force: POINT，力向量
        dt: 时间步长
        """
        # F = ma → Δv = F * dt / m
        acceleration = force * (1.0 / self.mass)
        self.velocity += acceleration * dt
    
    def apply_continuous_torque(self, torque, dt):
        """施加持续的力矩"""
        # τ = Iα → Δω = τ * dt / I
        self.angular_velocity += torque * dt / self.inertia
    
    def update(self, dt):
        """更新位置和角度"""
        # 平移
        displacement = self.velocity * dt
        self.polygon = self.polygon + displacement
        
        # 旋转
        self.polygon = self.polygon << (self.angular_velocity * dt)
    
    def resolve_collision(self, other, collision_line, restitution=0.5):
        """
        处理两个刚体的碰撞
        collision_line: LINE，碰撞信息（起点=碰撞点，方向=法线×深度）
        restitution: 恢复系数（0=完全非弹性，1=完全弹性）
        """
        # 提取碰撞信息
        contact_point = collision_line.A
        normal = collision_line.B - collision_line.A
        depth = normal.getLen()
        
        if depth < 1e-10:
            return
        
        # 归一化法线
        normal = normal * (1.0 / depth)
        
        # 计算相对速度
        r1 = contact_point - self.polygon.center
        r2 = contact_point - other.polygon.center
        
        # 接触点的速度 = 质心速度 + 角速度引起的切向速度
        v1 = self.velocity + POINT(-self.angular_velocity * r1.y, 
                                     self.angular_velocity * r1.x)
        v2 = other.velocity + POINT(-other.angular_velocity * r2.y, 
                                      other.angular_velocity * r2.x)
        
        relative_velocity = v2 - v1
        
        # 计算法线方向上的相对速度分量
        vel_along_normal = relative_velocity | normal
        
        # 如果物体正在分离，不处理
        if vel_along_normal > 0:
            return
        
        # 计算冲量大小
        # 公式来源：https://en.wikipedia.org/wiki/Collision_response
        r1_cross_n = r1.x * normal.y - r1.y * normal.x
        r2_cross_n = r2.x * normal.y - r2.y * normal.x
        
        inv_mass_sum = (1.0 / self.mass) + (1.0 / other.mass)
        inv_inertia_sum = (r1_cross_n * r1_cross_n) / self.inertia + \
                          (r2_cross_n * r2_cross_n) / other.inertia
        
        j = -(1 + restitution) * vel_along_normal / (inv_mass_sum + inv_inertia_sum)
        
        # 应用冲量
        impulse = normal * j
        self.apply_impulse(impulse * -1, contact_point)
        other.apply_impulse(impulse, contact_point)
        
        # 位置修正（防止嵌入）
        correction_factor = 0.8  # 修正强度
        correction = normal * (depth * correction_factor / (1.0/self.mass + 1.0/other.mass))
        self.polygon = self.polygon + correction * (-1.0 / self.mass)
        other.polygon = other.polygon + correction * (1.0 / other.mass)
