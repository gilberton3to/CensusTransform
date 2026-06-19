#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    const char* imgName = "input_04.png";

    Mat imgIn = imread(imgName, IMREAD_GRAYSCALE);
    if (imgIn.empty())
    {
        cerr << "Erro: nao foi possivel abrir a imagem " << imgName << endl;
        return 1;
    }

    imshow("input_04", imgIn);

    Size imgSize = imgIn.size();
    Mat imgTemp = Mat::zeros(imgSize, CV_8U);

    int x, y;

    // Loop unrolling: varredura da imagem ignorando as bordas
    for (x = 1; x < imgSize.height - 1; x++)
    {
        for (y = 1; y < imgSize.width - 1; y++)
        {
            uchar center = imgIn.at<uchar>(x, y);
            unsigned int census = 0;

            // Lógica desenrolada idêntica à do STM32 (ordem de bits exata)
            census |= (imgIn.at<uchar>(x - 1, y - 1) >= center) << 7;
            census |= (imgIn.at<uchar>(x - 1, y)     >= center) << 6;
            census |= (imgIn.at<uchar>(x - 1, y + 1) >= center) << 5;
            census |= (imgIn.at<uchar>(x, y - 1)     >= center) << 4;
            census |= (imgIn.at<uchar>(x, y + 1)     >= center) << 3;
            census |= (imgIn.at<uchar>(x + 1, y - 1) >= center) << 2;
            census |= (imgIn.at<uchar>(x + 1, y)     >= center) << 1;
            census |= (imgIn.at<uchar>(x + 1, y + 1) >= center);

            imgTemp.at<uchar>(x, y) = static_cast<uchar>(census);
        }
    }

    imshow("output", imgTemp);
    waitKey(0);

    return 0;
}