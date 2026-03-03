import matplotlib.pyplot as plt
import numpy as np

step = {}
pre = {}
with open("write_id.txt", "r") as f:
    for i, line in enumerate(f):
        s = list(map(int, line.split(' ')))
        n = s[0]
        for j in range(1, len(s)):
            pre[s[j]] = i + 1

cnt = 0
with open("query_id.txt", "r") as f:
    for i, line in enumerate(f):
        s = list(map(int, line.split(' ')))
        n = s[0]
        cnt += n
        for j in range(1, len(s)):
            # if i + 1 - pre[s[j]] > 1000:
            #     print(pre[s[j]], i + 1)
            step[i + 1 - pre[s[j]]] = step.get(i + 1 - pre[s[j]], 0) + 1

print(len(step))

mx = 0
for key, val in step.items():
    mx = max(mx, key)

cnt = [0] * (mx + 1)
for key, val in step.items():
    cnt[key] = val

def moving_average(data, window_size):
    return np.convolve(data, np.ones(window_size) / window_size, mode='valid')

fig, ax = plt.subplots(1, 1, figsize=(20, 20))
fig.suptitle('life time', fontsize=16)

# 获取当前行的数据
data = cnt

x_values = np.arange(len(data))  # x 轴为时间步长
y_values = np.array(data)  # y 轴为有效数据
print(x_values.shape)
print(y_values.shape)

# # 对数据进行滑动平均（窗口大小为100）
window_size = 100
if len(y_values) > window_size:  # 确保数据足够长
    y_smooth = moving_average(y_values, window_size)
    x_smooth = x_values[:len(y_smooth)]  # 调整 x 轴长度
else:
    y_smooth = y_values
    x_smooth = x_values

# 绘制平滑后的曲线
ax.plot(x_smooth, y_smooth, label=f'Variable {i}')
ax.set_title(f'Variable {i}')
ax.set_xlabel('Time Step')
ax.set_ylabel('Value')
ax.legend()


# 调整子图间距
plt.tight_layout()
plt.show()