import matplotlib.pyplot as plt
import numpy as np

MAX_SIZE = 5

adj = np.zeros((MAX_SIZE, 90000))
with open("write_size.txt", "r") as f:
    for i, line in enumerate(f):
        cnt = [0] * MAX_SIZE
        s = list(map(int, line.split(' ')))
        for j in range(len(s)):
            cnt[s[j] - 1] += 1
        for j in range(MAX_SIZE):
            adj[j][i] = cnt[j]


def moving_average(data, window_size):
    return np.convolve(data, np.ones(window_size) / window_size, mode='valid')

fig, axes = plt.subplots(4, 4, figsize=(20, 20))  # 4行4列，总共16张图
fig.suptitle('5 Variables Over Time', fontsize=16)


for i in range(MAX_SIZE):
    row = i // 4  # 子图的行索引
    col = i % 4   # 子图的列索引
    ax = axes[row, col]  # 获取当前子图
    
    # 获取当前行的数据
    data = adj[i]
    
    # 创建掩码，过滤掉值为 -1 的点
    mask = data != -1
    x_values = np.arange(len(data))[mask]  # x 轴为时间步长
    y_values = data[mask]  # y 轴为有效数据
    
    # # 对数据进行滑动平均（窗口大小为100）
    window_size = 100
    if len(y_values) > window_size:  # 确保数据足够长
        y_values = moving_average(y_values, window_size)
        x_values = x_values[:len(y_values)]  # 调整 x 轴长度
    else:
        y_values = y_values
        x_values = x_values
    
    # 绘制平滑后的曲线
    ax.plot(x_values, y_values, label=f'Variable {i}')
    ax.set_title(f'Variable {i}')
    ax.set_xlabel('Time Step')
    ax.set_ylabel('Value')
    ax.legend()


# 调整子图间距
# plt.tight_layout()
# plt.show()

cnt = [0] * MAX_SIZE
for i in range(MAX_SIZE):
    cnt[i] = sum(adj[i])


tot = 0
for x in cnt:
    tot += x

mx = 1e9
best = []
for j in range(1, 1000):
    cur = [0] * MAX_SIZE
    for i in range(MAX_SIZE):
        cur[i] = j * cnt[i] / tot
    cost = 0
    for k in range(MAX_SIZE):
        cost += abs(round(cur[k]) - cur[k])
    if cost < mx:
        mx = cost
        best = cur

for i in range(MAX_SIZE):
    print(best[i], end=', ')
print()
for i in range(MAX_SIZE):
    print(round(best[i]), end=', ')
print()

