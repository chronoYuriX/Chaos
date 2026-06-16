from math import sin, cos, pi


class POINT:
    def __init__(self, x = float("NaN"), y = float("NaN")):
        self.x, self.y = x, y
    def __add__(A, B):
        return POINT(A.x + B.x, A.y + B.y)
    def __sub__(A, B):
        return POINT(A.x - B.x, A.y - B.y)
    def __mul__(self, scale): # A * scale
        return POINT(self.x * scale, self.y * scale)
    def __pow__(A, B): # A x B
        return A.x * B.y - A.y * B.x
    def rotate(self, center, angle):
        offset_self = self - center
        offset_center = POINT(offset_self.x * cos(angle) - offset_self.y * sin(angle), \
                              offset_self.x * sin(angle) + offset_self.y * cos(angle))
        return center + offset_center
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
    def __and__(P1, P2):
        P1Edge = P1.getEdge()
        P2Edge = P2.getEdge()
        for P1Edge_i in P1Edge:
            for P2Edge_j in P2Edge:
                if P1Edge_i & P2Edge_j:
                    return True
        return False
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

class COLLISION_INFO:
    """存储碰撞信息的类"""
    def __init__(self):
        self.collided = False      # 是否碰撞
        self.normal = POINT()      # 碰撞法线（从P1指向P2的单位向量）
        self.depth = 0.0           # 穿透深度
        self.contact_point = POINT()  # 碰撞点（世界坐标）
        self.contact_edge_P1 = []  # P1参与碰撞的边（列表，可能为空）
        self.contact_edge_P2 = []  # P2参与碰撞的边

def get_axes(polygon):
    """获取多边形所有边的法线（作为分离轴候选）"""
    edges = polygon.getEdge()
    axes = []
    for edge in edges:
        # 边向量
        edge_vec = edge.B - edge.A
        # 法线（垂直于边，取向外方向）
        normal = POINT(-edge_vec.y, edge_vec.x)
        # 归一化
        length = (normal.x**2 + normal.y**2)**0.5
        if length > 1e-10:
            normal = POINT(normal.x/length, normal.y/length)
            axes.append(normal)
    return axes

def project_polygon(polygon, axis):
    """将多边形投影到轴上，返回[min, max]以及对应的顶点索引"""
    dots = [p.x*axis.x + p.y*axis.y for p in polygon.points]
    min_val = min(dots)
    max_val = max(dots)
    min_idx = dots.index(min_val)
    max_idx = dots.index(max_val)
    return min_val, max_val, min_idx, max_idx

def find_MTV(poly1, poly2):
    """
    找到最小平移向量（Minimum Translation Vector）
    返回：(axis, overlap, is_colliding)
    axis: 分离轴（单位向量）
    overlap: 重叠量（正数表示重叠）
    is_colliding: 是否碰撞
    """
    axes = get_axes(poly1) + get_axes(poly2)
    
    min_overlap = float('inf')
    min_axis = POINT()
    is_colliding = True
    
    for axis in axes:
        min1, max1, _, _ = project_polygon(poly1, axis)
        min2, max2, _, _ = project_polygon(poly2, axis)
        
        # 检查是否有间隔
        if max1 < min2 or max2 < min1:
            return POINT(), 0, False  # 有分离轴，不相交
        
        # 计算重叠量
        overlap = min(max1 - min2, max2 - min1)
        
        # 确保法线方向从poly1指向poly2
        center_diff = poly2.center - poly1.center
        if center_diff.x * axis.x + center_diff.y * axis.y < 0:
            axis = POINT(-axis.x, -axis.y)
        
        if overlap < min_overlap:
            min_overlap = overlap
            min_axis = axis
    
    return min_axis, min_overlap, True

def find_contact_point(poly1, poly2, mtv_axis, depth):
    """
    找到碰撞点和碰撞边
    返回 COLLISION_INFO 对象
    """
    info = COLLISION_INFO()
    info.collided = True
    info.normal = mtv_axis
    info.depth = depth
    
    # 在法线方向上找到poly1的最远点和poly2的最近点
    # 这给出了碰撞的大致位置
    dots1 = [p.x*mtv_axis.x + p.y*mtv_axis.y for p in poly1.points]
    dots2 = [p.x*mtv_axis.x + p.y*mtv_axis.y for p in poly2.points]
    
    # poly1在法线负方向的最远点（离poly2最近的点）
    idx1 = dots1.index(max(dots1))
    # poly2在法线正方向的最近点（离poly1最近的点）
    idx2 = dots2.index(min(dots2))
    
    # 碰撞点取两点中点
    info.contact_point = POINT(
        (poly1.points[idx1].x + poly2.points[idx2].x) / 2,
        (poly1.points[idx1].y + poly2.points[idx2].y) / 2
    )
    
    # 找到参与碰撞的边
    edges1 = poly1.getEdge()
    edges2 = poly2.getEdge()
    
    for i, edge in enumerate(edges1):
        # 检查这条边是否与碰撞法线大致垂直（即边与法线方向接近平行）
        edge_vec = edge.B - edge.A
        edge_normal = POINT(-edge_vec.y, edge_vec.x)
        edge_len = (edge_normal.x**2 + edge_normal.y**2)**0.5
        if edge_len > 1e-10:
            edge_normal = POINT(edge_normal.x/edge_len, edge_normal.y/edge_len)
            dot = edge_normal.x * mtv_axis.x + edge_normal.y * mtv_axis.y
            if abs(abs(dot) - 1.0) < 0.01:  # 法线方向一致
                info.contact_edge_P1.append(edge)
    
    for i, edge in enumerate(edges2):
        edge_vec = edge.B - edge.A
        edge_normal = POINT(-edge_vec.y, edge_vec.x)
        edge_len = (edge_normal.x**2 + edge_normal.y**2)**0.5
        if edge_len > 1e-10:
            edge_normal = POINT(edge_normal.x/edge_len, edge_normal.y/edge_len)
            dot = edge_normal.x * (-mtv_axis.x) + edge_normal.y * (-mtv_axis.y)
            if abs(abs(dot) - 1.0) < 0.01:
                info.contact_edge_P2.append(edge)
    
    return info

def detect_collision_with_info(poly1, poly2):
    """
    完整的碰撞检测，返回 COLLISION_INFO
    """
    mtv_axis, depth, collided = find_MTV(poly1, poly2)
    
    if not collided:
        info = COLLISION_INFO()
        info.collided = False
        return info
    
    return find_contact_point(poly1, poly2, mtv_axis, depth)

