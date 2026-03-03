import matplotlib.pyplot as plt
import numpy as np

MAX_SIZE = 6
MAX_TAG = 17

cnt = np.zeros((MAX_SIZE, MAX_TAG))
with open("write_tag_with_size.txt", "r") as f:
    for i, line in enumerate(f):
        if line == '':
            continue
        s = list(map(int, line.split(' ')))
        tag = s[0]
        size = s[1]
        cnt[size][tag] += 1

for i in range(1, MAX_TAG):
    print('{', end='')
    for j in range(1, MAX_SIZE):
        print(int(cnt[j][i]), end=', ')
    print('},')
