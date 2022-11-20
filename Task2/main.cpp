#include <stdio.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace cv;

u_int8_t p_round(int x) {
    if (x > 255) {
        return 255;
    }
    if (x < 0) {
        return 0;
    }
    return x;
}

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
                    //std::vector<int> pixels;
                    for (int ip = i - r; ip <= i + r; ++ip) {
                        for (int jp = j - r; jp <= j + r; ++jp) {
                            int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                            int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                            //pixels.push_back(mat.at<Vec3b>(actualIp, actualJp)[channel]);
                            hist[channel][mat.at<Vec3b>(actualIp, actualJp)[channel]]++;
                        }
                    }
                    while (leftSum[channel] + hist[channel][median[channel]] <= targetLeftSum) {
                        leftSum[channel] += hist[channel][median[channel]];
                        median[channel]++;
                    }
                    //p[channel] = getMedian(pixels);
                    //median[channel] = p[channel];
                    p[channel] = median[channel];
                }
            } else {
                for (int channel = 0; channel < 3; channel++) {
                    //int leftSum = (2*r + 1) * (2*r + 1) / 2;
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
                // for (int jp = j - r; jp <= j + r; ++jp) {
                //     int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                //     int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                //     //pixels.push_back(mat.at<Vec3b>(actualIp, actualJp)[channel]);
                //     columns_hist[j][mat.at<Vec3b>(actualIp, actualJp)[channel]]++;
                // }
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
                    for (int ip = i - r; ip <= i + r; ++ip) {
                        for (int jp = j - r; jp <= j + r; ++jp) {
                            int actualIp = getActualIndex(ip, 0, mat.rows - 1);
                            int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                            //pixels.push_back(mat.at<Vec3b>(actualIp, actualJp)[channel]);
                            hist[mat.at<Vec3b>(actualIp, actualJp)[channel]]++;
                        }
                    }
                    while (leftSum + hist[median] <= targetLeftSum) {
                            leftSum += hist[median];
                            median++;
                        }
                    
                    for (int jp = j; jp <= j + r; ++jp) {
                        int actualJp = getActualIndex(jp, 0, mat.cols - 1);
                        int old_i = getActualIndex(i - r - 1, 0, mat.rows - 1);
                        int new_i = getActualIndex(i + r, 0, mat.rows - 1);
                        columns_hist[actualJp][mat.at<Vec3b>(old_i, actualJp)[channel]]--;
                        if (columns_hist[actualJp][mat.at<Vec3b>(old_i, actualJp)[channel]] < 0) {
                            std::cout << i<< " " << j << " 111" << std::endl;
                        }
                        columns_hist[actualJp][mat.at<Vec3b>(new_i, actualJp)[channel]]++;
                    }
                    p[channel] = median;
                } else {
                    int actualJp = getActualIndex(j + r, 0, mat.cols - 1);
                    if (actualJp > r && j + r < mat.cols) {
                        int old_i = getActualIndex(i - r - 1, 0, mat.rows - 1);
                        int new_i = getActualIndex(i + r, 0, mat.rows - 1);
                        columns_hist[actualJp][mat.at<Vec3b>(old_i, actualJp)[channel]]--;
                        if (columns_hist[actualJp][mat.at<Vec3b>(old_i, actualJp)[channel]] < 0) {
                            std::cout << i<< " " << j << " 222" << std::endl;
                        }
                        columns_hist[actualJp][mat.at<Vec3b>(new_i, actualJp)[channel]]++;
                    }

                    for (int val = 0; val < 256; ++val) {
                        int old_j = getActualIndex(j - 1 - r, 0, mat.cols - 1);
                        int new_j = getActualIndex(j + r, 0, mat.cols - 1);
                        
                        hist[val] -= columns_hist[old_j][val];
                        if (hist[val] < 0) {
                            std::cout << i<< " " << j << " 333" << std::endl;
                        }
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
                    // std::cout << i << " " << j << " " << (int)p1[channel] << " " << (int)p2[channel] << std::endl;
                    // flag = false;
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
    
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat image1, image2, oldImage;
    oldImage = imread(argv[1], 1);
    image1 = imread(argv[1], 1);
    image2 = imread(argv[1], 1);
    if ( !image1.data )
    {
        printf("No image data \n");
        return -1;
    }
    double start = clock();
    SimpleMedianFilter(oldImage, image1, 1);
    //linearMedianFilter(oldImage, image2, 1);
    constantMedianFilter(oldImage, image2, 1);
    double end = clock();
    std::cout << checkImages(image1, image2) << std::endl;

    imwrite("output2.bmp", image2);
    imwrite("output1.bmp", image1);
    std::cout << "Время работы: " << (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    return 0;
}