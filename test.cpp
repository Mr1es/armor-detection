#if 0

#include<opencv2/opencv.hpp>
using namespace cv;
int main()
{
	Mat a = imread("C:/Users/28687/Desktop/shot/Project1/shots/shot_1.png");
	GaussianBlur(a, a, Size(5, 5), 0, 0, BORDER_DEFAULT);
	cvtColor(a, a, COLOR_BGR2HSV);
	Mat mask;
	inRange(a, Scalar(80, 70, 105), Scalar(110, 200, 255), mask);
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(mask, mask, MORPH_OPEN, element, Point(-1, -1), 1);
	morphologyEx(mask, mask, MORPH_CLOSE, element, Point(-1, -1), 1);



	waitKey(0);
	return 0;
}
#endif