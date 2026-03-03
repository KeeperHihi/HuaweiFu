import numpy as np

# 输入数组
ii = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
arr = [1951, 1507, 2660, 1466, 1125, 1352, 1490, 2159, 3368, 2893, 3799, 1606, 1834, 2158, 1421, 2540, ]
weight = [3278430, 3116711, 2557707, 2581907, 3061067, 2021160, 2901526, 3080000, 3033139, 2253834, 3124818, 2943453, 2787002, 2759340, 2547069, 2515808, ]

tot = np.sum(weight)
weight /= tot
weight = 1 + weight * 10

for i in range(16):
    arr[i] *= weight[i]
print(weight)
print(list(map(int, arr)))
    
# arr = [
#     arr[1] + arr[14] + arr[15],
#     arr[2] + arr[3] + arr[11],
#     arr[4] + arr[9] + arr[10] + arr[13],
#     arr[5] + arr[6] + arr[7],
#     arr[8] + arr[12] + arr[16]
# ]

# 初始化负载数组
loads = np.zeros(10)  # 10个编号
assignments = [[] for _ in range(17)]  # 每个编号对应的元素列表

# 逐个元素进行分配
for i, elem in enumerate(arr):
    # 选择当前负载最小的三个编号
    sorted_indices = np.argsort(loads)[:3]  # 选择负载最小的三个编号
    # 将当前元素分配给这三个编号
    for idx in sorted_indices:
        assignments[ii[i]].append(idx)
        loads[idx] += elem  # 更新负载

# 输出每个编号的元素和总负载
for i in range(17):
    print('{', end='')
    for x in assignments[i]:
        print(x, end=', ')
    print('},', end='\n')

# 输出每个编号的总负载
print(f"\n每个编号的总负载: {loads}")
