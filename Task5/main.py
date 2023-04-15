import cv2
import numpy as np
import matplotlib.pyplot as plt
import time
from tqdm import tqdm

img_names = ['01.tif', '02.tif', '03.tif', '04.tif',
             '05.tif', '06.tif', '07.tif', '08.tif',
             '09.tif', '10.tif', '11.tif', '12.tif']

global_time = {
    'Harris' : [0, 0],
    'SIFT' : [0, 0],
    'BRISK' : [0, 0]
}
i=0
def match_shift_points(points1, points2, treshold=4):
    points1 = np.array(points1)
    points2 = np.array(points2)

    mean_points1 = np.mean(points1, axis=0)
    mean_points2 = np.mean(points2, axis=0)
    dx = mean_points2[0] - mean_points1[0]
    dy = mean_points2[1] - mean_points1[1]
    delta = np.array([dx, dy])
    match_points = 0
    for i in range(len(points1)):
        dists = np.sum(np.abs(points2 - (points1[i, :] + delta)), axis=1)
        if np.min(dists) <= treshold:
            match_points += 1
    return match_points


def harris_detector(img, k=0.04):
    img = np.float32(cv2.cvtColor(img, cv2.COLOR_BGR2GRAY))

    start = time.time()
    dst = cv2.cornerHarris(img, 2, 3, k)
    end = time.time()

    points = np.argwhere( dst > 0.01*dst.max())
    global_time['Harris'][0] += end - start
    global_time['Harris'][1] += len(points)

    return points

def sift_detector(img):
    img = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    global i
    
    start = time.time()
    sift = cv2.SIFT_create()
    kp = sift.detect(img, None)
    end = time.time()

    points = [p.pt for p in kp]
    global_time['SIFT'][0] += end - start
    global_time['SIFT'][1] += len(points)
    return points

def brisk_detector(img):
    img = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    global i
    
    start = time.time()
    brisk = cv2.BRISK_create()
    kp = brisk.detect(img, None)
    end = time.time()

    points = [p.pt for p in kp]
    global_time['BRISK'][0] += end - start
    global_time['BRISK'][1] += len(points)
    return points


def get_pair_matching(image1, image2, detector):
    
    points1 = detector(image1)
    points2 = detector(image2)
    
    matched_points = match_shift_points(points1, points2)
    return matched_points / len(points1)



def get_repeatability(detector):
    images_rep = []
    for i, img1 in tqdm(enumerate(img_names)):
        img_rep = 0
        for j, img2 in enumerate(img_names):
            if i == j:
                continue
            
            image1 = cv2.imread(f'images/{img1}')
            image2 = cv2.imread(f'images/{img2}')

            img_rep += get_pair_matching(image1, image2, detector)
        
        images_rep.append(img_rep / 11)
    
    return images_rep



def main():
    
    detectors = [harris_detector, sift_detector, brisk_detector]
    detectors_names = ['Harris', 'SIFT', 'BRISK']
    results = []
    for detector in detectors:
        results.append(get_repeatability(detector))
    for i, name in enumerate(detectors_names):
        plt.figure(figsize=(12, 8))
        plt.title(f'Воспроизводимость алгоритма {name}')
        plt.plot(np.arange(0, 12)+1, results[i])
        plt.ylabel('repeatability')
        plt.xlabel('image')
        plt.grid(ls=':')
        plt.show()


    for method, metrics in global_time.items():
        print(f'{method}: {metrics[0]*10**6 / metrics[1]}')

if __name__ == "__main__":
    main()