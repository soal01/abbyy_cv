#include <stdio.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace cv;

int p_round(int x) {
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

int get_signal(const uchar& vec) {
    // double w[3] = {0.2126, 0.7152, 0.0722};
    // return vec[0]*w[0] + vec[1]*w[1] + vec[2]*w[2];
    return vec;
}


long long** calc_sums(Mat &img, int xmin, int xmax) {
    int W = img.cols;
    int H = img.rows;
    long long** res = new long long*[H];
    for (int i = 0; i < H; ++i) {
        res[i] = new long long[xmax - xmin];
    }

    if (xmax - xmin == 1) {
        for (int i = 0 ; i < H; ++i) {
            res[i][0] = get_signal(img.at<uchar>(i, xmin));
            // if (res[i][0] > 0) {
            //     std::cout << res[i][0] << std::endl;
            // }
        }
    } else {
        int mid = (xmin + xmax) / 2;
        long long** ans1 = calc_sums(img, xmin, mid);
        long long** ans2 = calc_sums(img, mid, xmax);
        for (int x = 0; x < H; ++x) {
            for (int shift = 0; shift < xmax - xmin; ++shift) {
                res[x][shift] = (ans1[x][shift / 2] + ans2[(x + shift / 2 + shift % 2) % H][shift / 2]);
            }
        }
        for (int i = 0; i < H; ++i) {
            delete ans1[i];
            delete ans2[i];
        }
        delete ans1;
        delete ans2;
    }
    return res;
}

void clip_res(long long** res, int h, int w) {
    long long max_res = 1;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            max_res = std::max(max_res, res[i][j]);
        }
    }

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            res[i][j] = (res[i][j] * 255 / max_res);
        }
    }
}


double calcVar(long long* arr, int n) {
    double e2 = 0;
    for (int i = 0; i < n; ++i) {
        // if (arr[i] > 0) {
        //     std::cout << arr[i] << " ";
        // }
        e2 += arr[i] * arr[i];
    }
    //std::cout << std::endl;
    e2 /= n;

    double e = 0;
    for (int i = 0; i < n; ++i) {
        e += arr[i];
    }
    e /= n;

    return e2 - e*e;
}


std::pair<double, double> findAngle(Mat &img) {
    int W = img.cols;
    int H = img.rows;

    double start = clock();

    long long** res = calc_sums(img, 0, W);
    
    double end = clock();
   
    double work_time = (end - start) / CLOCKS_PER_SEC;
    
    clip_res(res, H, W);
    // for (int i = 0; i < W; ++i) {
    //     for (int j = 0; j < W; ++j) {
    //         if (res[i][j] > 0) {
    //             //std::cout << res[i][j] << std::endl; 
    //             counter++;
    //         }
    //     }
    // }
    // for (int i = 0; i < W; ++i) {
    //     for (int j = 0; j < W; ++j) {
    //         if (res[i][j] > 0) {
    //             //std::cout << res[i][j] << std::endl; 
    //             counter++;
    //         }
    //         std::cout << res[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    //std::cout << counter << std::endl;
    cv::Mat hough(H, W, CV_8U);
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            uchar& p = hough.at<uchar>(i, j);
            p = res[i][j];
        }
    }
    imwrite("hough.bmp", hough);


    int imax = 0;
    double maxVar = 0;
    for (int i = 0; i < H; ++i) {
        double var = calcVar(res[i], W);
        
        if (var > maxVar) {
            imax = i;
            maxVar = var;
        }
        //std::cout << var << std::endl;
    }
    std::cout << "imax " << imax << std::endl;
    //double angle = (H / 2.0 - imax) / (H / 2.0) * 45.0;

    return {(H / 2.0 - imax) / (H / 2.0) * 45.0 / 2, work_time};
}

// void my_rotate(Mat &src, Mat &res, double angle) {
//     cv::Point2f center((src.cols-1)/2.0, (src.rows-1)/2.0);
//     cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
//     // determine bounding rectangle, center not relevant
//     cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
//     // adjust transformation matrix
//     rot.at<double>(0,2) += bbox.width/2.0 - src.cols/2.0;
//     rot.at<double>(1,2) += bbox.height/2.0 - src.rows/2.0;

//     cv::warpAffine(src, res, rot, bbox.size());
//     // cv::imwrite("rotated_im.png", dst);
// }

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
            // if (p[0] > 0) {
            //     std::cout << (int)p[0] << " " << (int)p[1] << " " << (int)p[2] << std::endl;
            // }
            //x_src = floor()
        }
    }
}

void applySobel(Mat& src, Mat& res) {
    int ddepth = CV_16S;
    int ksize = 3;
    int scale = 1;
    int delta = 0;
    cv::Mat src_gray;
    cvtColor(src, src_gray, COLOR_BGR2GRAY);
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;
    Sobel(src_gray, grad_x, ddepth, 1, 0, ksize, scale, delta, BORDER_DEFAULT);
    Sobel(src_gray, grad_y, ddepth, 0, 1, ksize, scale, delta, BORDER_DEFAULT);
    // converting back to CV_8U
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, res);
}


int main(int argc, char** argv ) {
   
    Mat image1, image2, image3, oldImage, result;
    char* inputFile = argv[1];
    char* outputFile = argv[2];
    int type_interpol = strtoll(argv[3], NULL, 10);

    oldImage = imread(inputFile, 1);
    image1 = imread(inputFile, 1);
    result = imread(inputFile, 1);
   
    linearMedianFilter(oldImage, image1, 20);

    applySobel(image1, image2);

    int new_W = 1;
    while (new_W < image2.cols) {
        new_W *= 2;
    } 
    int new_H = 1;
    while (new_H < image2.rows) {
        new_H *= 2;
    }

    cv::Mat big_sobel(new_H, new_W, CV_8U);
    for (int i = 0; i < new_H; ++i) {
        for (int j = 0; j < new_W; ++j) {
            uchar& p = big_sobel.at<uchar>(i, j);
            if (i < image2.rows && j < image2.cols) {
                p = image2.at<uchar>(i, j);
            } else {
                p = 0;
            }
        }
    }

    auto ans = findAngle(big_sobel);
    double angle = ans.first;
    double work_time = ans.second;
    
    std::cout << "Размеры изображения: " << oldImage.rows << " " << oldImage.cols << std::endl;
    
    my_rotate(oldImage, result, -angle, type_interpol);
    
    std::cout << "Время работы: " << work_time << "s" << std::endl;
    imwrite(outputFile, result);
    return 0;
}