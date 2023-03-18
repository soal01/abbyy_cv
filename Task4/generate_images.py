import subprocess
import sys
from tqdm import tqdm

papers = ['crumpled-white-paper.jpg', 'white-texture.jpg', 'bumaga3.jpg', 'bumaga4.jpg', 'bumaga5.jpg']

for i in tqdm(range(1, 11)):
    for p_number, paper in enumerate(papers):
        cmd = f"./ProcessImage ./images/{i}.png ./paper/{paper} ./augment_img/{i}/{p_number*2}.bmp"
        output = subprocess.check_output(cmd.split()).decode()

        cmd = f"./ProcessImage ./images/{i}.png ./paper/{paper} ./augment_img/{i}/{p_number*2+1}.bmp"
        output = subprocess.check_output(cmd.split()).decode()
    
    
