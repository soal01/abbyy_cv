#include <stdio.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <opencv2/opencv.hpp>
using namespace cv;


bool isFirstMin(int a, int b, int c, int d) {
    return (a <= b) && (a <= c) && (a <= d);
}

int hue_transit(int16_t L1, int16_t L2, int16_t L3,
                int16_t V1, int16_t V3) {
    
    if ((L1 < L2 && L2 < L3) || (L1 > L2 && L2 > L3)) {
        return V1 + ((int)V3 - V1) * (L2 - L1) / ((int)L3 - L1);
    } else {
        return (V1 + V3) / 2.0 + (L2 - (L1 + L3) / 2.0) / 2.0;
    }
}

u_int8_t p_round(int x) {
    if (x > 255) {
        return 255;
    }
    if (x < 0) {
        return 0;
    }
    return x;
}


int getGreen(int16_t P1, int16_t P3, int16_t P5,
             int16_t P11, int16_t P13, int16_t P15,
             int16_t P21, int16_t P23, int16_t P25,
             int16_t G8, int16_t G12, int16_t G14, int16_t G18) {

    int dN = std::abs((int)P13 - P3)*2 + std::abs((int)G18 - G8);
    int dE = std::abs((int)P13 - P15)*2 + std::abs((int)G12 - G14);
    int dW = std::abs((int)P13 - P11)*2 + std::abs((int)G14 - G12);
    int dS = std::abs((int)P13 - P23)* 2 + std::abs((int)G8 - G18);
    if (isFirstMin(dN, dE, dW, dS)) {
        return (G8*3 + G18 + P13 - P3) / 4;
    }
    if (isFirstMin(dE, dN, dW, dS)) {
        return (G14*3 + G12 + P13 - P15) / 4;
    }
    if (isFirstMin(dW, dE, dN, dS)) {
        return (G12*3 + G14 + P13 - P11) / 4;
    }
    if (isFirstMin(dS, dE, dW, dN)) {
        return (G18*3 + G8 + P13 - P23) / 4;
    }
}

int getBlue(int16_t R1, int16_t R5, int16_t R13, int16_t R21, int16_t R25,
            int16_t B7, int16_t B9, int16_t B17, int16_t B19, 
            int16_t G7, int16_t G9, int16_t G13, int16_t G17, int16_t G19) {

    int dNE = std::abs(B9 - B17) + std::abs(R5 - R13) + std::abs(R13 - R21) + std::abs(G9 - G13) + std::abs(G13 - G17);
    int dNW = std::abs(B7 - B19) + std::abs(R1 - R13) + std::abs(R13 - R25) + std::abs(G7 - G13) + std::abs(G13 - G19);
    if (dNE < dNW) {
        return hue_transit(G9, G13, G17, B9, B17);
    } else {
        return hue_transit(G7, G13, G19, B7, B19);
    }
}

int getRed(int16_t B1, int16_t B5, int16_t B13, int16_t B21, int16_t B25,
            int16_t R7, int16_t R9, int16_t R17, int16_t R19, 
            int16_t G7, int16_t G9, int16_t G13, int16_t G17, int16_t G19) {

    int dNE = std::abs(R9 - R17) + std::abs(B5 - B13) + std::abs(B13 - B21) + std::abs(G9 - G13) + std::abs(G13 - G17);
    int dNW = std::abs(R7 - R19) + std::abs(B1 - B13) + std::abs(B13 - B25) + std::abs(G7 - G13) + std::abs(G13 - G19);
    if (dNE < dNW) {
        return hue_transit(G9, G13, G17, R9, R17);
    } else {
        return hue_transit(G7, G13, G19, R7, R19);
    }
}

void PPG(Mat &mat)
{
    CV_Assert(mat.channels() == 3);
    char** colors;
    colors = new char*[mat.rows];
    for (int i = 0; i < mat.rows; ++i) {
        colors[i] = new char[mat.cols];
    }
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            Vec3b& p = mat.at<Vec3b>(i, j);
            if ((i + j) % 2 == 1) {
                colors[i][j] = 'g';
            } else {
                if (i % 2 == 0) {
                    colors[i][j] = 'r';
                } else {
                    colors[i][j] = 'b';
                }
            }
            
        }
    }

    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            if (!((i >= 2 && i < mat.rows - 2) && (j >= 2 && j < mat.cols - 2))) {
                continue;
            }
            Vec3b& p = mat.at<Vec3b>(i, j);
            if (colors[i][j] == 'r') {
                p[1] = p_round(
                        getGreen(mat.at<Vec3b>(i - 2, j - 2)[2], mat.at<Vec3b>(i - 2, j)[2], mat.at<Vec3b>(i - 2, j + 2)[2],
                                mat.at<Vec3b>(i, j - 2)[2], mat.at<Vec3b>(i, j)[2], mat.at<Vec3b>(i, j + 2)[2],
                                mat.at<Vec3b>(i + 2, j - 2)[2], mat.at<Vec3b>(i + 2, j)[2], mat.at<Vec3b>(i + 2, j + 2)[2],
                                mat.at<Vec3b>(i - 1, j)[1], mat.at<Vec3b>(i, j - 1)[1], mat.at<Vec3b>(i, j + 1)[1], mat.at<Vec3b>(i+1, j)[1])
                ); 
            }
            if (colors[i][j] == 'b') {
                p[1] = p_round(
                        getGreen(mat.at<Vec3b>(i - 2, j - 2)[0], mat.at<Vec3b>(i - 2, j)[0], mat.at<Vec3b>(i - 2, j + 2)[0],
                                mat.at<Vec3b>(i, j - 2)[0], mat.at<Vec3b>(i, j)[0], mat.at<Vec3b>(i, j + 2)[0],
                                mat.at<Vec3b>(i + 2, j - 2)[0], mat.at<Vec3b>(i + 2, j)[0], mat.at<Vec3b>(i + 2, j + 2)[0],
                                mat.at<Vec3b>(i - 1, j)[1], mat.at<Vec3b>(i, j - 1)[1], mat.at<Vec3b>(i, j + 1)[1], mat.at<Vec3b>(i+1, j)[1])
                ); 
            }
        }
    }

    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            if (!((i >= 1 && i < mat.rows - 1) && (j >= 1 && j < mat.cols - 1))) {
                continue;
            }
            Vec3b& p = mat.at<Vec3b>(i, j);
            if (colors[i][j] == 'g') {
                if (colors[i][j-1] == 'b') {
                    p[0] = p_round(
                            hue_transit(mat.at<Vec3b>(i, j - 1)[1], mat.at<Vec3b>(i, j)[1], mat.at<Vec3b>(i, j + 1)[1],
                                       mat.at<Vec3b>(i, j - 1)[0], mat.at<Vec3b>(i, j + 1)[0])
                    );
                    p[2] = p_round(
                            hue_transit(mat.at<Vec3b>(i - 1, j)[1], mat.at<Vec3b>(i, j)[1], mat.at<Vec3b>(i + 1, j)[1],
                                       mat.at<Vec3b>(i - 1, j)[2], mat.at<Vec3b>(i + 1, j)[2])
                    );
                } else {
                    p[2] = p_round(
                            hue_transit(mat.at<Vec3b>(i, j - 1)[1], mat.at<Vec3b>(i, j)[1], mat.at<Vec3b>(i, j + 1)[1],
                                       mat.at<Vec3b>(i, j - 1)[2], mat.at<Vec3b>(i, j + 1)[2])
                    );
                    p[0] = p_round(
                            hue_transit(mat.at<Vec3b>(i - 1, j)[1], mat.at<Vec3b>(i, j)[1], mat.at<Vec3b>(i + 1, j)[1],
                                       mat.at<Vec3b>(i - 1, j)[0], mat.at<Vec3b>(i + 1, j)[0])
                    );
                }
            }
        }
    }

    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            if (!((i >= 2 && i < mat.rows - 2) && (j >= 2 && j < mat.cols - 2))) {
                continue;
            }
            Vec3b& p = mat.at<Vec3b>(i, j);
            if (colors[i][j] == 'g') {
                continue;
            }
            if (colors[i][j] == 'r') {
            //     int16_t B1, int16_t B5, int16_t B13, int16_t B21, int16_t B25,
            // int16_t R7, int16_t R9, int16_t R17, int16_t R19, 
            // int16_t G7, int16_t G9, int16_t G13, int16_t G17, int16_t G19
                p[0] = p_round(
                        getBlue(mat.at<Vec3b>(i - 2, j - 2)[2], mat.at<Vec3b>(i - 2, j + 2)[2], mat.at<Vec3b>(i, j)[2],
                                mat.at<Vec3b>(i + 2, j - 2)[2], mat.at<Vec3b>(i + 2, j + 2)[2],
                                mat.at<Vec3b>(i - 1, j - 1)[0], mat.at<Vec3b>(i - 1, j + 1)[0], 
                                mat.at<Vec3b>(i + 1, j - 1)[0], mat.at<Vec3b>(i + 1, j + 1)[0],
                                mat.at<Vec3b>(i - 1, j - 1)[1], mat.at<Vec3b>(i - 1, j + 1)[1],
                                mat.at<Vec3b>(i, j)[1], mat.at<Vec3b>(i + 1, j - 1)[1], mat.at<Vec3b>(i + 1, j + 1)[1])
                );
            }
            if (colors[i][j] == 'b') {
            //     int16_t B1, int16_t B5, int16_t B13, int16_t B21, int16_t B25,
            // int16_t R7, int16_t R9, int16_t R17, int16_t R19, 
            // int16_t G7, int16_t G9, int16_t G13, int16_t G17, int16_t G19
                p[2] = p_round(
                        getRed(mat.at<Vec3b>(i - 2, j - 2)[0], mat.at<Vec3b>(i - 2, j + 2)[0], mat.at<Vec3b>(i, j)[0],
                                mat.at<Vec3b>(i + 2, j - 2)[0], mat.at<Vec3b>(i + 2, j + 2)[0],
                                mat.at<Vec3b>(i - 1, j - 1)[2], mat.at<Vec3b>(i - 1, j + 1)[2], 
                                mat.at<Vec3b>(i + 1, j - 1)[2], mat.at<Vec3b>(i + 1, j + 1)[2],
                                mat.at<Vec3b>(i - 1, j - 1)[1], mat.at<Vec3b>(i - 1, j + 1)[1],
                                mat.at<Vec3b>(i, j)[1], mat.at<Vec3b>(i + 1, j - 1)[1], mat.at<Vec3b>(i + 1, j + 1)[1])
                );
            }

        }
    }

    for (int i = 0; i < mat.rows; ++i) {
        delete colors[i];
    }
    delete colors;
}

double calculatePSNR(Mat& image1, Mat& image2) {
    double MSE = 0;
    for (int i = 0; i < image1.rows; ++i) {
        for (int j = 0; j < image1.cols; ++j) {
            Vec3b& p1 = image1.at<Vec3b>(i, j);
            Vec3b& p2 = image2.at<Vec3b>(i, j);
            double w[3] = {0.2126, 0.7152, 0.0722};
            MSE += pow((int)p1[0] - p2[0], 2)*w[2] + pow((int)p1[1] - p2[1], 2)*w[1] + pow((int)p1[2] - p2[2], 2)*w[0];
        }
    }
    MSE /= image1.rows * image1.cols;
    return 10 * log10(255*255 / MSE);
}


int main(int argc, char** argv )
{
    
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat oldImage, image;
    oldImage = imread( argv[1], cv::IMREAD_GRAYSCALE);
    cvtColor(oldImage, image, cv::COLOR_GRAY2BGR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    double start = clock();
    PPG(image);
    double end = clock();

    imwrite("output.bmp", image);
    Mat resImage;
    resImage = imread( "Original.bmp", 1 );
    double PSNR = calculatePSNR(image, resImage);
    std::cout << "Время работы: " << (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    std::cout << "PSNR: " << PSNR << std::endl;
    return 0;
}