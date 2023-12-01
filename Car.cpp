#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


void main()
{

	// Gets the video feed from the pi camera
	VideoCapture cap(0);
	Mat img;
	Mat imgGray, imgBlur, imgLines, imgDil, imgErode, imgResize;

	Mat kernel;

	while (true) 
	{

		// Reads the current feed and saves the current frame
		cap.read(img);
		cout << img.size() << endl;
		resize(img, imgResize, Size(244,244))

		cvtColor(img, outimg, COLOR_BGR2GRAY);

		GaussianBlur(img, imgBlur, Size(3, 3), 3, 0);
		Canny(imgBlur, imgLines, 25, 75);

		kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
		dilate(imgLines, imgDil, kernel);

		
		imshow("image", imgDil);

		waitKey(1);
	}

	

	

}