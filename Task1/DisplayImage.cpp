#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;


void paintAlphaMat(Mat &mat)
{
    CV_Assert(mat.channels() == 3);
    for (int i = 0; i < mat.rows; ++i)
    {
        for (int j = 0; j < mat.cols; ++j)
        {
            Vec3b& bgra = mat.at<Vec3b>(i, j);
            // bgra[0] = UCHAR_MAX; // Blue
            // bgra[1] = saturate_cast<uchar>((float (mat.cols - j)) / ((float)mat.cols) * UCHAR_MAX); // Green
            // bgra[2] = saturate_cast<uchar>((float (mat.rows - i)) / ((float)mat.rows) * UCHAR_MAX); // Red
            // bgra[3] = saturate_cast<uchar>(0.5 * (bgra[1] + bgra[2])); // Alpha
            // if (i % 100 < 5 && j % 100 < 5) {
            //     std::cout << (int)bgra[0] << " " << (int)bgra[1] << " " << (int)bgra[2] << std::endl;
            // }
            if (bgra[0] != 0) {
                bgra[1] = bgra[2] = bgra[0];
                continue;
            }
            if (bgra[1] != 0) {
                bgra[0] = bgra[2] = bgra[1];
                continue;
            }
            if (bgra[2] != 0) {
                bgra[1] = bgra[0] = bgra[2];
                continue;
            }
        }
    }
}


int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat image;
    image = imread( argv[1], 1 );
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    paintAlphaMat(image);
    //namedWindow("Display Image", WINDOW_AUTOSIZE );
    //imshow("Display Image", image);
    //waitKey(0);
    imwrite("example.bmp", image);


    return 0;
}