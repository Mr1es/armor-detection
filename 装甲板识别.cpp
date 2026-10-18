#if 1
#include<iostream>
#include<opencv2/opencv.hpp>
#include<time.h>
using namespace std;
using namespace cv;
int main()
{
	VideoCapture cap("8.4第二场_clip_194.800_204.avi");
	if (!cap.isOpened()) { cout << "打不开视频" << endl; return -1; }
	double fps = cap.get(CAP_PROP_FPS);
	int delay = (fps > 0) ? (int)(1000.0 / fps) : 33;
	//int delay = 180;
	int hmin = 80, smin = 70, vmin = 105;
	namedWindow("mask");
	//createTrackbar("H最小", "mask", &hmin, 180);   // H 最大 110
	//createTrackbar("S最小", "mask", &smin, 255);   // S 最大 200
	//createTrackbar("V最小", "mask", &vmin, 255);   // V 最大 255
	Mat K = (Mat_<double>(3, 3) << 2355.2485, 0.0, 563.00874,
		0.0, 2355.66400, 434.82466,
		0.0, 0.0, 1.0);
	Mat dist = (Mat_<double>(1, 5) << -0.064176, -0.219573, -0.000362, -0.000117, 0.289017);
	
	while (1)
	{
		Mat frame;
		cap >> frame;
		if (frame.empty())
		{
			cout << "视频读取失败" << endl;
			return -1;
		}
		Mat hsv, gau;
		GaussianBlur(frame, gau, Size(5, 5), 0, 0, BORDER_DEFAULT);
		cvtColor(gau, hsv, COLOR_BGR2HSV);
		Mat mask;
		inRange(hsv, Scalar(80, 70, 105), Scalar(110, 200, 255), mask);
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		morphologyEx(mask, mask, MORPH_OPEN, element, Point(-1, -1), 1);
		morphologyEx(mask, mask, MORPH_CLOSE, element, Point(-1, -1), 1);
		vector<vector<Point>>contours;
		findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		Mat show = frame.clone();
		clock_t start = clock();//-----------------------------------------------------------
		for (int i = 0;i < contours.size();i++)
		{
			//drawContours(show, contours, i, Scalar(0, 255, 0), 1);
		}
		vector<RotatedRect>lights;
		vector<vector<Point>> lightPts;
		for (int i = 0;i < (int)contours.size();i++)
		{
			double area = contourArea(contours[i]);
			if (area < 30)continue;

			RotatedRect r = minAreaRect(contours[i]);
			float len = max(r.size.width, r.size.height);
			float thin = min(r.size.width, r.size.height);
			if (thin < 2.0)continue;
			if (len / thin < 3.0)continue;
			if (thin > 10)continue;
			if (len > 40)continue;

			lights.push_back(r);
			lightPts.push_back(contours[i]);
			Point2f pts[4];
			r.points(pts);
			for (int k = 0;k < 4;k++)
			{
				line(show, pts[k], pts[(k + 1) % 4], Scalar(0, 255, 0), 1);
			}
		}



		for (int i = 0;i < (int)lights.size();i++)
		{
			for (int j = i + 1;j < (int)lights.size();j++)
			{
				float len1 = max(lights[i].size.width, lights[i].size.height);
				float len2 = max(lights[j].size.width, lights[j].size.height);
				float meanlen = (len1 + len2) / 2;
				Point2f p1 = lights[i].center;
				Point2f p2 = lights[j].center;
				
				float lenDiff = fabs(len1 - len2) / max(len1, len2);
				if (lenDiff > 0.6) continue;
				float dis = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
				float ratio = dis / meanlen;
				if (ratio < 1.5 || ratio > 3.0) continue;
				float yDiff = fabs(p1.y - p2.y) / meanlen;
				if (yDiff > 0.6) continue;
				Vec4f ln1, ln2;
				fitLine(lightPts[i], ln1, DIST_L2, 0, 0.01, 0.01);
				fitLine(lightPts[j], ln2, DIST_L2, 0, 0.01, 0.01);
				Point2f ax1(ln1[0], ln1[1]);//先接住点，再接住方向
				Point2f ax2(ln2[0], ln2[1]);
				if (ax1.y > 0) ax1 = -ax1;      // 统一朝上
				if (ax2.y > 0) ax2 = -ax2;
				Point2f t1 = p1 + ax1 * (len1 / 2), b1 = p1 - ax1 * (len1 / 2);
				Point2f t2 = p2 + ax2 * (len2 / 2), b2 = p2 - ax2 * (len2 / 2);
				Point2f corner[4] = { t1, b1, b2,t2 };
				for (int k = 0; k < 4; k++)
				{
					line(show, corner[k], corner[(k + 1) % 4], Scalar(0, 0, 255), 2);
				}
				Point2f center = (p1 + p2) * 0.5f;
				circle(show, center, 3, Scalar(0, 0, 255), -1);

				
				double LIGHTBAR_LENGTH = 56e-3, SMALL_ARMOR_WIDTH = 135e-3;//pnp
				vector<Point3f>objPts;
				if (p1.x < p2.x) {                // 灯1 在左边 
					objPts.push_back(Point3f(0, SMALL_ARMOR_WIDTH / 2, LIGHTBAR_LENGTH / 2));
					objPts.push_back(Point3f(0, -SMALL_ARMOR_WIDTH / 2, LIGHTBAR_LENGTH / 2));
					objPts.push_back(Point3f(0, -SMALL_ARMOR_WIDTH / 2, -LIGHTBAR_LENGTH / 2));
					objPts.push_back(Point3f(0, SMALL_ARMOR_WIDTH / 2, -LIGHTBAR_LENGTH / 2));
				}
				else {                            // 灯1 在右边，3D 的左右跟着换
					objPts.push_back(Point3f(0, SMALL_ARMOR_WIDTH / 2, -LIGHTBAR_LENGTH / 2));
					objPts.push_back(Point3f(0, -SMALL_ARMOR_WIDTH / 2, -LIGHTBAR_LENGTH / 2));
					objPts.push_back(Point3f(0, -SMALL_ARMOR_WIDTH / 2, LIGHTBAR_LENGTH / 2));
					objPts.push_back(Point3f(0, SMALL_ARMOR_WIDTH / 2, LIGHTBAR_LENGTH / 2));
				}
				vector<Point2f>imgpts(corner,corner + 4);
				Mat rvec, tvec;
				if (solvePnP(objPts, imgpts, K, dist, rvec, tvec, false, SOLVEPNP_IPPE))
				{
					double distance = norm(tvec);
					cout << "距离 " << distance << " m" << endl;
					string s = to_string(distance) + " m";
					putText(show, s, Point(cvRound(center.x) - 30, cvRound(center.y) - 15),
						FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 255), 2);
				}
				clock_t end = clock();
				double execution_time = (double)(end - start) / CLOCKS_PER_SEC;
				printf("程序执行时间:%f 秒\n", execution_time);
				
				
			
			
			}
			
		}

		
				imshow("mask", show);
				imshow("video", frame);
				char c = waitKey(delay);
				if (c == 27)
				{
					cout << "程序结束运行";
					return 0;
				}


	}
			return 0;
}
#endif