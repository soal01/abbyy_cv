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

void add_vertical_white_line(Mat &mat, int column_number, int width) {
    for (int i = 0; i < mat.rows; ++i) {
        
        for (int j = 0; j < mat.cols; ++j) {
            if (j >= column_number && j < column_number + width) {
                Vec3b& p = mat.at<Vec3b>(i, j);
                for (int channel = 0; channel < 3; ++channel) {
                    //p[channel] = p[channel] * 10;
                    p[channel] = 250;
                }
            }
        }
    }
    
}

void add_vertical_black_line(Mat &mat, int column_number, int width) {
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            if (j >= column_number && j < column_number + width) {
                Vec3b& p = mat.at<Vec3b>(i, j);
                for (int channel = 0; channel < 3; ++channel) {
                    //p[channel] = p[channel] * 10;
                    p[channel] = 5;
                }
            }
        }
    }
    
}

uint8_t clip_p(int x) {
    if (x > 255) {
        return 255;
    }
    if (x < 0) {
        return 0;
    }
    return x;
}

void add_gauss_noise(Mat &mat) {
    // float** noise = new float*[mat.rows];
    // for (int i = 0; i < mat.rows; ++i) {
    //     noise[i] = new float[mat.cols];
    // }
    cv::Mat noise(mat.size(), mat.type());
    float m = (0, 0, 0);
    float sigma = (15, 15, 15);
    cv::randn(noise, m, sigma);
    for (int i = 0; i < mat.rows; ++i) {
        for (int j = 0; j < mat.cols; ++j) {
            Vec3b& p = mat.at<Vec3b>(i, j);
            Vec3b& p_noise = noise.at<Vec3b>(i, j);
            for (int channel = 0; channel < 3; ++channel) {
                p[channel] = clip_p(p[channel] + p_noise[0]);
            }
        }
    }
    cv::Mat blured_noise;
    GaussianBlur(noise, blured_noise, Size(5, 5), 0);
    mat += blured_noise;
}

std::pair<double, double> simple_rotate(double x, double y, double angle) {
    double x_ans = x * cos(angle) - y * sin(angle);
    double y_ans = x * sin(angle) + y * cos(angle);
    return {x_ans, y_ans};
}

double linear_inter(double x1, double x3, double f1, double f3, double x2) {
    double k = (f1 - f3) / (x1 - x3);
    double b = f1 - k*x1;
    return k*x2 + b;
}

Vec3b get_f(Mat &img, int x, int y) {
    if (x >= 0 && x < img.cols && y >= 0  && y < img.rows) {
        return img.at<Vec3b>(y, x);
    } else {
        return Vec3b({0, 0, 0});
    }
}

Vec3b get_pixel(Mat &src, const std::pair<double, double>& point, int type_interpol) {
    int xl = floor(point.first);
    int xr = ceil(point.first);
    int yb = floor(point.second);
    int yu = ceil(point.second);
    if (type_interpol == 1) {
        int x_src = (abs(xl - point.first) < abs(xr - point.first))? xl : xr;
        int y_src = (abs(yb - point.second) < abs(yu - point.second))? yb : yu;
        if (x_src >= 0 && x_src < src.cols && y_src >= 0 && y_src < src.rows) {
            return src.at<Vec3b>(y_src, x_src);
        } else {
            return Vec3b({0, 0, 0});
        }
    } else {
        Vec3b ans({0, 0, 0});
        Vec3b f_lb = get_f(src, xl, yb);
        Vec3b f_lu = get_f(src, xl, yu);
        Vec3b f_rb = get_f(src, xr, yb);
        Vec3b f_ru = get_f(src, xr, yu);

        for (int channel = 0; channel < 3; ++channel) {
            double f1 = linear_inter(yb, yu, f_lb[channel], f_lu[channel], point.second);
            double f2 = linear_inter(yb, yu, f_rb[channel], f_ru[channel], point.second);
            ans[channel] = linear_inter(xl, xr, f1, f2, point.first);
        }
        return ans;
        //return Vec3b({0, 0, 0});
    }
}

void my_rotate(Mat &src, Mat &res, double angle, int type_interpol=1) {
    double x_center = src.cols / 2.0;
    double y_center = src.rows / 2.0;
    angle = angle * 3.14159 / 180;
    for (int y = 0; y < res.rows; ++y) {
        for (int x = 0; x < res.cols; ++x) {
            Vec3b& p = res.at<Vec3b>(y, x);
            double x_src = x - x_center;
            double y_src = y_center - y;
            auto src_point = simple_rotate(x_src, y_src, -angle);
            src_point.first += x_center;
            src_point.second = y_center - src_point.second;
            p = get_pixel(src, src_point, type_interpol);
        }
    }
}


void process_image(Mat &mat, Mat &paper, Mat &result) {
    srand(time(0));
    int row = rand() % mat.rows;
    add_horizontal_white_line(mat, row, 12);
    row = rand() % mat.rows;
    add_horizontal_white_line(mat, row, 12);
    row = rand() % mat.rows;
    add_horizontal_white_line(mat, row, 12);
    row = rand() % mat.rows;
    add_horizontal_white_line(mat, row, 12);
    int column = rand() % mat.cols;
    add_vertical_black_line(mat, column, 5);
    column = rand() % mat.cols;
    add_vertical_black_line(mat, column, 5);
    add_paper(mat, paper);
    add_gauss_noise(mat);
    Mat blured_mat;
    GaussianBlur(mat, blured_mat, Size(15, 15), 0);
    int angle = rand() % 7 - 3;
    my_rotate(blured_mat, result, angle);
}



int main(int argc, char** argv )
{
    
    if ( argc != 4 )
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
    
    Mat result(image.size(), image.type());
    process_image(image, paper, result);
    imwrite(argv[3], result);
    
    return 0;
}