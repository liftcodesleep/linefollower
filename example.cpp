#include <opencv4/opencv.hpp>

int main(void)
{
    std::cout << "OpenCV version : " << cv::CV_VERSION << endl;
    std::cout << "Major version : " << cv::CV_MAJOR_VERSION << endl;
    std::cout << "Minor version : " << cv::CV_MINOR_VERSION << endl;
    std::cout << "Subminor version : " << cv::CV_SUBMINOR_VERSION << endl;
    std::cout << cv::getBuildInformation() << std::endl;
}