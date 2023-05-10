import matplotlib.pyplot as plt
import math
import sys

if(len(sys.argv) != 2):
    print("Pass path to data file")
    exit(0)

filepath = sys.argv[1]

pre_lines = []
lines = []
with open(filepath, 'r') as f:
    pre_lines = f.read().splitlines()
    for line in pre_lines:
        if line[0].isnumeric():
            lines.append(line)

N = 0
for line in lines:
    N += int(line.split(' ')[1])

xdata = []
ydata = []
cur = 0
for line in lines:
    p = line.split()
    x, y = int(p[0]), int(p[1])
    cur += y
    xdata.append(math.log10(x))
    ydata.append(cur/N)

plt.plot(xdata, ydata)
plt.title(filepath.split('.')[0])
plt.xlabel('Log of distance')
plt.ylabel('Cumulative Density Function')
plt.tight_layout()
plt.savefig(f'{filepath.split(".")[0]}.svg', format='svg')