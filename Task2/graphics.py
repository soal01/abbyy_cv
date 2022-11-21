import subprocess
import sys
import numpy as np
import matplotlib.pyplot as plt
from tqdm import tqdm

r = range(0, 10, 2)
#r = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
timeSimple = []
timeLinear = []
timeConstant = []
for i in tqdm(r):
    cmd = "./MedianFilter photo.bmp 0 " + str(i)
    output = subprocess.check_output(cmd.split()).decode()
    print(output.split())
    newTime = float(output.split()[-1][:-1])
    timeSimple.append(newTime)

    cmd = "./MedianFilter photo.bmp 1 " + str(i)
    output = subprocess.check_output(cmd.split()).decode()
    print(output.split())
    newTime = float(output.split()[-1][:-1])
    timeLinear.append(newTime)

    cmd = "./MedianFilter photo.bmp 2 " + str(i)
    output = subprocess.check_output(cmd.split()).decode()
    print(output.split())
    newTime = float(output.split()[-1][:-1])
    timeConstant.append(newTime)

w = 960
h = 1280
timeSimple = np.array(timeSimple)
timeLinear = np.array(timeLinear)
timeConstant = np.array(timeConstant)
plt.figure(figsize=(12, 8))
plt.title('Зависимость времени работы медианного фильтра от размера фильтра')
plt.plot(r, timeSimple*1000 / (w*h/1000000), label='наивный алгоритм')
plt.plot(r, timeLinear*1000 / (w*h/1000000), label="линейный алгоритм")
plt.plot(r, timeConstant*1000 / (w*h/1000000), label="константный алгоритм")
plt.xlabel('Радиус фильтра')
plt.ylabel('мсек/мегапиксель')
plt.legend()
plt.grid(ls=':')
plt.show()