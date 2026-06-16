import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.transforms import Affine2D
import time

# ====================== 矩形类 ======================
class Rect:
    def __init__(self, cx, cy, w, h, angle_deg=0, angular_vel=0):
        """
        cx, cy: 中心坐标
        w, h: 宽度和高度
        angle_deg: 初始角度（度）
        angular_vel: 角速度（度/秒）
        """
        self.cx = cx
        self.cy = cy
        self.w = w
        self.h = h
        self.angle = np.deg2rad(angle_deg)   # 内部用弧度
        self.angular_vel = np.deg2rad(angular_vel)

    def get_corners(self):
        """返回当前矩形的四个角点（世界坐标）"""
        cos_a, sin_a = np.cos(self.angle), np.sin(self.angle)
        # 局部坐标下的四个角（相对于中心）
        half_w, half_h = self.w / 2, self.h / 2
        local_corners = np.array([
            [-half_w, -half_h],
            [ half_w, -half_h],
            [ half_w,  half_h],
            [-half_w,  half_h]
        ])
        # 旋转 + 平移
        world_corners = np.zeros((4, 2))
        for i in range(4):
            x = local_corners[i, 0] * cos_a - local_corners[i, 1] * sin_a + self.cx
            y = local_corners[i, 0] * sin_a + local_corners[i, 1] * cos_a + self.cy
            world_corners[i] = [x, y]
        return world_corners

    def update(self, dt):
        """按角速度更新角度"""
        self.angle += self.angular_vel * dt

# ====================== SAT碰撞检测 ======================
def project_polygon(vertices, axis):
    """将多边形所有顶点投影到轴上，返回[min, max]"""
    dots = np.dot(vertices, axis)
    return np.min(dots), np.max(dots)

def sat_overlap(proj1, proj2):
    """检查两个投影区间是否重叠"""
    return not (proj1[1] < proj2[0] or proj2[1] < proj1[0])

def check_collision(rect1, rect2):
    """
    用分离轴定理（SAT）检测两个矩形是否相交
    返回 True 表示碰撞
    """
    corners1 = rect1.get_corners()
    corners2 = rect2.get_corners()

    # 获取所有可能的分离轴（矩形的每条边的法线）
    axes = []
    for corners in [corners1, corners2]:
        for i in range(4):
            # 边向量
            edge = corners[(i+1) % 4] - corners[i]
            # 法线（垂直向量）
            normal = np.array([-edge[1], edge[0]])
            # 归一化（非必须，但有利于数值稳定）
            norm = np.linalg.norm(normal)
            if norm > 1e-8:
                normal /= norm
            axes.append(normal)

    # 去重（可选，这里简化处理）
    for axis in axes:
        proj1 = project_polygon(corners1, axis)
        proj2 = project_polygon(corners2, axis)
        if not sat_overlap(proj1, proj2):
            return False  # 存在一个分离轴 → 不相交
    return True  # 所有轴都重叠 → 相交

# ====================== 角速度离散化检测 ======================
def detect_collision_with_substeps(rect1, rect2, dt, max_step_deg=2.0):
    """
    考虑旋转的连续碰撞检测（离散化版本）
    
    dt: 总时间步长（秒）
    max_step_deg: 每子步允许的最大旋转角度（度）
    
    返回：(collided, collision_time_ratio)
        collided: 是否碰撞
        collision_time_ratio: 碰撞发生在时间步的哪个比例 [0,1]，若无碰撞则为 None
    """
    # 保存初始状态
    init_angle1 = rect1.angle
    init_angle2 = rect2.angle

    # 计算需要的子步数
    total_rotation1 = abs(rect1.angular_vel * dt)
    total_rotation2 = abs(rect2.angular_vel * dt)
    max_rotation = max(total_rotation1, total_rotation2)
    
    num_steps = max(1, int(np.ceil(max_rotation / np.deg2rad(max_step_deg))))
    sub_dt = dt / num_steps

    # 逐子步检测
    for step in range(num_steps + 1):  # +1 确保覆盖终点
        # 恢复初始状态并前进到当前子步
        rect1.angle = init_angle1 + rect1.angular_vel * (step * sub_dt)
        rect2.angle = init_angle2 + rect2.angular_vel * (step * sub_dt)

        if check_collision(rect1, rect2):
            # 发生了碰撞，返回碰撞时刻的比例
            ratio = step / num_steps
            return True, ratio

    # 没有碰撞
    return False, None

# ====================== 可视化 ======================
def draw_scene(rect1, rect2, ax):
    """绘制当前场景"""
    ax.clear()
    ax.set_xlim(-5, 15)
    ax.set_ylim(-5, 12)
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.3)

    # 绘制矩形1
    corners1 = rect1.get_corners()
    poly1 = patches.Polygon(corners1, closed=True, fill=True, 
                            facecolor='lightblue', edgecolor='blue', linewidth=2)
    ax.add_patch(poly1)

    # 绘制矩形2
    corners2 = rect2.get_corners()
    poly2 = patches.Polygon(corners2, closed=True, fill=True, 
                            facecolor='lightcoral', edgecolor='red', linewidth=2)
    ax.add_patch(poly2)

    # 标题信息
    ax.set_title(f"Rect1 Angle: {np.rad2deg(rect1.angle):.1f}° | "
                 f"Rect2 Angle: {np.rad2deg(rect2.angle):.1f}°")

    plt.draw()
    plt.pause(0.01)

# ====================== 主程序 ======================
def main():
    # 创建两个矩形
    # 矩形1：静止，位于左侧
    rect1 = Rect(cx=2, cy=2, w=3, h=2, angle_deg=0, angular_vel=0)
    # 矩形2：向右平移 + 旋转
    rect2 = Rect(cx=8, cy=3, w=2, h=3, angle_deg=0, angular_vel=60)  # 60°/秒

    # 初始化绘图
    plt.ion()
    fig, ax = plt.subplots(figsize=(10, 8))

    # 时间参数
    dt = 0.02  # 每帧20ms（50 FPS）
    total_time = 0
    max_time = 10  # 最多运行10秒

    print("开始模拟...")
    print("矩形1：静止")
    print("矩形2：以 60°/秒 的速度顺时针旋转")
    print("检测到碰撞后将自动暂停\n")

    while total_time < max_time:
        # 保存当前角度（用于回退）
        prev_angle2 = rect2.angle

        # 更新矩形2的角度（全步长）
        rect2.update(dt)

        # 使用离散化检测是否发生碰撞
        collided, ratio = detect_collision_with_substeps(rect1, rect2, dt, max_step_deg=2.0)

        if collided:
            # 回退到碰撞前的状态
            rect2.angle = prev_angle2 + rect2.angular_vel * (dt * ratio)
            print(f"💥 碰撞！时间: {total_time + dt*ratio:.3f}秒")
            print(f"   矩形2角度: {np.rad2deg(rect2.angle):.2f}°")
            
            # 显示最终状态
            draw_scene(rect1, rect2, ax)
            plt.ioff()
            plt.show()
            break

        # 绘制当前状态
        draw_scene(rect1, rect2, ax)

        total_time += dt
        time.sleep(dt)  # 控制帧率

    if total_time >= max_time:
        print("模拟结束，未检测到碰撞")

if __name__ == "__main__":
    main()
