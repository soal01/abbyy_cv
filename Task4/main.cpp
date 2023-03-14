#include <stdio.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <opencv2/opencv.hpp>
using namespace cv;


void add_paper(Mat &mat, Mat &paper) {
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            Vec3b& p = mat.at<Vec3b>(i, j);
            Vec3b& p_paper = paper.at<Vec3b>(i, j);
            for (int channel = 0; channel < 3; ++channel) {
                p[channel] = (p[channel] + p_paper[channel]) / 2;
            }
        }
    }
    //std::cout << mat.rows << " " << mat.cols << std::endl;

}

void add_horizontal_white_line(Mat &mat, int row_number, int width) {
    for (int i = 0; i < mat.rows; ++i) {
        if (i >= row_number && i < row_number + width) {
            for (int j = 0; j < mat.cols; ++j) {
                Vec3b& p = mat.at<Vec3b>(i, j);
                for (int channel = 0; channel < 3; ++channel) {
                    //p[channel] = p[channel] * 10;
                    p[channel] = 250;
                }
            }
        }
    }
}

void process_image(Mat &mat, Mat &paper) {

    add_horizontal_white_line(mat, mat.rows / 2 - 200, 12);
    add_horizontal_white_line(mat, 100, 12);
    add_horizontal_white_line(mat, mat.rows / 2 + 251, 12);
    add_horizontal_white_line(mat, mat.rows - mat.rows / 3 + 25, 12);
    add_paper(mat, paper);
    
}



int main(int argc, char** argv )
{
    
    if ( argc != 3 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat oldImage, image, paper;
    image = imread( argv[1], 1);
    paper = imread(argv[2], 1);
    //cvtColor(oldImage, image, cv::COLOR_GRAY2BGR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    double start = clock();
    process_image(image, paper);
    double end = clock();

    imwrite("output.bmp", image);
    
    std::cout << "Время работы: " << (end - start) / CLOCKS_PER_SEC << "s" << std::endl;
    std::cout << "PSNR: " << PSNR << std::endl;
    return 0;
}