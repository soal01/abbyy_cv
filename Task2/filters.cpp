#include <stdio.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace cv;


int getMedian(std::vector<int>& pixels) {
    sort(pixels.begin(), pixels.end());
    return pixels[pixels.size() / 2];
}

int getActualIndex(int ind, int low, int high) {
    if (ind < low) {
        return low - ind;
    }
    if (ind > high) {
        return high - (ind - high);
    }
    return ind;
}

void SimpleMedianFilter(Mat &mat, Mat &res, int r) {
    CV_Assert(mat.channels() == 3);
    
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            Vec3b& p = res.at<Vec3b>(i, j);
            for (int channel = 0; channel < 3; channel++) {
                std::vector<int> pixels;
                for (int ip = i - r; ip <= i + r; ++ip) {
                    for (int jp = j - r; jp <= j + r; ++jp) {
                        int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                        int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                        pixels.push_back(mat.at<Vec3b>(actualIp, actualJp)[channel]);
                        
                    }
                }
                p[channel] = getMedian(pixels);
            }
        }
    }
}


void linearMedianFilter(Mat &mat, Mat &res, int r) {
    CV_Assert(mat.channels() == 3);
    
    
    for (int i = 0; i < mat.rows; ++i) {
        uint8_t hist[3][256];
        uint8_t median[3];
        int leftSum[3] = {0, 0, 0};
        int targetLeftSum = (2*r + 1)*(2*r + 1) / 2;
        for (int channel = 0; channel < 3; ++channel) {
            for (int val = 0; val < 256; ++val) {
                hist[channel][val] = 0;
                
            }
            median[channel] = 0;
        }
        for (int j = 0; j < mat.cols; ++j) {
            Vec3b& p = res.at<Vec3b>(i, j);
            if (j == 0) {
                for (int channel = 0; channel < 3; channel++) {
                    for (int ip = i - r; ip <= i + r; ++ip) {
                        for (int jp = j - r; jp <= j + r; ++jp) {
                            int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                            int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                            hist[channel][mat.at<Vec3b>(actualIp, actualJp)[channel]]++;
                        }
                    }
                    while (leftSum[channel] + hist[channel][median[channel]] <= targetLeftSum) {
                        leftSum[channel] += hist[channel][median[channel]];
                        median[channel]++;
                    }
                    p[channel] = median[channel];
                }
            } else {
                for (int channel = 0; channel < 3; channel++) {
                    for (int ip= i - r; ip <= i + r; ++ip) {
                        int jp = j - 1 - r;
                        int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                        int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                        int val = mat.at<Vec3b>(actualIp, actualJp)[channel];
                        hist[channel][val]--;
                        if (val < median[channel]) {
                            leftSum[channel]--;
                        }
                    }


                    for (int ip= i - r; ip <= i + r; ++ip) {
                        int jp = j + r;
                        int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                        int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                        int val = mat.at<Vec3b>(actualIp, actualJp)[channel];
                        hist[channel][val]++;
                        if (val < median[channel]) {
                            leftSum[channel]++;
                        } 
                    }
                   
                    if (leftSum[channel] > targetLeftSum) {
                        do {
                            median[channel]--;
                            leftSum[channel] -= hist[channel][median[channel]];
                        }while (leftSum[channel] > targetLeftSum);
                    } else {
                        while (leftSum[channel] + hist[channel][median[channel]] <= targetLeftSum) {
                            leftSum[channel] += hist[channel][median[channel]];
                            median[channel]++;
                        }
                    }
                
                    p[channel] = median[channel];
                }
            }
        }
    }
}

void constantMedianFilter(Mat &mat, Mat &res, int r) {
    CV_Assert(mat.channels() == 3);
    
    for (int channel = 0; channel < 3; ++channel) {
        int** columns_hist = new int*[mat.cols];
        for (int i = 0; i < mat.cols; ++i) {
            columns_hist[i] = new int[256];
        }
        int targetLeftSum = (2*r + 1)*(2*r + 1) / 2;
        
        for (int i = 0; i < mat.cols; ++i) {
            for(int j = 0; j < 256; ++j) {
                columns_hist[i][j] = 0;
            }
        }
        for (int j = 0; j < mat.cols; ++j) {
            int i = -1;
            for (int ip = i - r; ip <= i + r; ++ip) {
                int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                columns_hist[j][mat.at<Vec3b>(actualIp, j)[channel]]++;
            }
        }
        
        for (int i = 0; i < mat.rows; ++i) {
            int leftSum = 0;
            int median = 0;
            int hist[256];
            for (int val = 0; val < 256; ++val) {
                hist[val] = 0;
            }

            for (int j = 0; j < mat.cols; ++j) {
                Vec3b& p = res.at<Vec3b>(i, j);

                if (j == 0) {
                    for (int jp = j; jp <= j + r; ++jp) {
                        int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                        int old_i = getActualIndex(i - r - 1, 0, mat.rows - 1);
                        int new_i = getActualIndex(i + r, 0, mat.rows - 1);
                        columns_hist[actualJp][mat.at<Vec3b>(old_i, actualJp)[channel]]--;
                        
                        columns_hist[actualJp][mat.at<Vec3b>(new_i, actualJp)[channel]]++;
                    }

                    for (int jp = j - r; jp <= j + r; ++jp) {
                        int actualIp = getActualIndex(i, 0, mat.rows - 1);
                        int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                        for (int val = 0; val < 256; ++val) {
                            hist[val] += columns_hist[actualJp][val];
                        }
                    }



                    while (leftSum + hist[median] <= targetLeftSum) {
                        leftSum += hist[median];
                        median++;
                    }

                    p[channel] = median;
                } else {
                    int actualJp = getActualIndex(j + r, 0, mat.cols - 1);
                    if (actualJp > r && j + r < mat.cols) {
                        int old_i = getActualIndex(i - r - 1, 0, mat.rows - 1);
                        int new_i = getActualIndex(i + r, 0, mat.rows - 1);
                        columns_hist[actualJp][mat.at<Vec3b>(old_i, actualJp)[channel]]--;
                        
                        columns_hist[actualJp][mat.at<Vec3b>(new_i, actualJp)[channel]]++;
                    }

                    for (int val = 0; val < 256; ++val) {
                        int old_j = getActualIndex(j - 1 - r, 0, mat.cols - 1);
                        int new_j = getActualIndex(j + r, 0, mat.cols - 1);
                        
                        hist[val] -= columns_hist[old_j][val];
                        
                        if (val < median) {
                            leftSum -= columns_hist[old_j][val];
                        }
                        hist[val] += columns_hist[new_j][val];
                        if (val < median) {
                            leftSum += columns_hist[new_j][val];
                        }
                    }

                    if (leftSum > targetLeftSum) {
                        do {
                            median--;
                            leftSum -= hist[median];
                        }while (leftSum > targetLeftSum);
                    } else {
                        while (leftSum + hist[median] <= targetLeftSum) {
                            leftSum += hist[median];
                            median++;
                        }
                    }
                
                    p[channel] = median;
                }
            }
        }
        for (int i = 0; i < mat.cols; ++i) {
            delete columns_hist[i];
        }
        delete columns_hist;
    }
}



bool checkImages(Mat &img1, Mat &img2) {
    
    CV_Assert(img1.channels() == 3);
    bool flag = true;
    for (int i = 0; i < img1.rows; ++i) {
        for (int j = 0; j < img2.cols; ++j) {
            Vec3b& p1 = img1.at<Vec3b>(i, j);
            Vec3b& p2 = img2.at<Vec3b>(i, j);
            for (int channel = 0; channel < 3; channel++) {
                if (p1[channel] != p2[channel]) {
                    return false;
                }
            }
        }
        if (!flag) {
            break;
        }
    }
    return flag;
}


int main(int argc, char** argv )
{
    
    Mat resImage, oldImage;
    oldImage = imread(argv[1], 1);
    resImage = imread(argv[1], 1);
    
    int r = strtoll(argv[3], NULL, 10);
    int type = strtoll(argv[2], NULL, 10);
    double start = clock();
    if (type == 0) {
        std::cout << "simple filter" << std::endl;
        SimpleMedianFilter(oldImage, resImage, r);
    }
    if (type == 1) {
        std::cout << "linear filter" << std::endl;
        linearMedianFilter(oldImage, resImage, r);
    }
    if (type == 2) {
        std::cout << "constant filter" << std::endl;
        constantMedianFilter(oldImage, resImage, r);
    }
    double end = clock();
    
    std::cout << "Время работы: " << (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    imwrite(std::string("./outputs/r_") + std::to_string(r) + std::string(".bmp"), resImage);
    return 0;
}