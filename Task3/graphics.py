import subprocess
import sys
import numpy as np
import matplotlib.pyplot as plt
from tqdm import tqdm

time = []
size = []
for i in tqdm(range(1, 11)):
    cmd = f"./Rotate input/{i}.jpg output_nearest/{i}.jpg 1"
    output = subprocess.check_output(cmd.split()).decode()
    print(output.split())
    newTime = float(output.split()[-1][:-1])
    time.append(newTime)

    newSize = float(output.split()[-4]) * float(output.split()[-5])
    size.append(newSize)

    cmd = f"./Rotate input/{i}.jpg output_linear/{i}.jpg 2"
    output = subprocess.check_output(cmd.split()).decode()
    print(output.split())
    


time = np.array(time)
size = np.array(size)
plt.figure(figsize=(12, 8))
plt.title('Зависимость времени работы быстрого преобразования Хаффа от размера изображения')
plt.scatter(size/1000000, time*1000)
plt.xlabel('мегапиксель')
plt.ylabel('мсек')
plt.grid(ls=':')
plt.show()