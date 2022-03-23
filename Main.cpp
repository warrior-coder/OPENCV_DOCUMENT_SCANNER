#include <opencv2/opencv.hpp>

#include <array>
#include <vector>

using namespace cv;
using namespace std;


int main()
{
	Mat img = imread("Resources/img_01.jpg");
	if (img.empty()) return 1;
	resize(img, img, Size(), 0.3, 0.3);

	Mat imgGray, imgBlur, imgCan, imgDil;		// processing stage images
	Mat imgWarp;						// warped document image

	vector<vector<Point>> contours;		// array of all image contours
	vector<Vec4i> hierarchy;

	double area, maxArea, length;		// contour properties
	vector<Point> approxedContour(4);	// array of approxed contour points
	array<Point, 4> documentContour;	// array of document contour points

	bool documentDetected;

	int sumPoints[4], subPoints[4];

	Size documentSize(283, 400);
	Rect roi(5, 5, documentSize.width - 5 * 2, documentSize.height - 5 * 2);

	Point2f srcPoints[4];
	const Point2f dstPoints[] = {
		{ 0.0f, 0.0f },
		{ (float)documentSize.width, 0.0f },
		{ 0.0f, (float)documentSize.height },
		{ (float)documentSize.width, (float)documentSize.height }
	};


	// create output windows
	//namedWindow("Camera", WINDOW_AUTOSIZE);
	//namedWindow("Document", WINDOW_AUTOSIZE);


	/* preprocess image */

	GaussianBlur(img, imgBlur, Size(5, 5), 5, 0);	// blur the image
	imshow("Image Blur", imgBlur);

	cvtColor(imgBlur, imgGray, COLOR_BGR2GRAY);				// convert color from BGR to GRAY pallete
	imshow("Image Gray", imgGray);

	//threshold(imgGray, imgThre, );

	Canny(imgGray, imgCan, 25, 75);						// use Canny edge detector
	imshow("Image Canny", imgCan);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCan, imgDil, kernel);
	imshow("Image Dilate", imgDil);


	/* detect document contour  */
	findContours(imgDil, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	maxArea = 0.0;
	documentDetected = false;

	for (const auto& contour : contours)
	{
		area = contourArea(contour);

		if (area > 25000.0)
		{
			length = arcLength(contour, true);
			approxPolyDP(contour, approxedContour, 0.02 * length, true);

			if (area > maxArea && approxedContour.size() == 4)
			{
				maxArea = area;
				for (int i = 0; i < 4; i++)
				{
					documentContour[i] = approxedContour[i];
				}
				documentDetected = true;
			}
		}
	}

	if (documentDetected)
	{
		/* reorder document points */
		for (int i = 0; i < 4; i++)
		{
			sumPoints[i] = documentContour[i].x + documentContour[i].y;
			subPoints[i] = documentContour[i].x - documentContour[i].y;
		}

		documentContour = {
			documentContour[min_element(sumPoints, sumPoints + 4U) - sumPoints],
			documentContour[max_element(subPoints, subPoints + 4U) - subPoints],
			documentContour[min_element(subPoints, subPoints + 4U) - subPoints],
			documentContour[max_element(sumPoints, sumPoints + 4U) - sumPoints]
		};


		/* get warp perspective */
		srcPoints[0] = documentContour[0];
		srcPoints[1] = documentContour[1];
		srcPoints[2] = documentContour[2];
		srcPoints[3] = documentContour[3];

		warpPerspective(
			img,
			imgWarp,
			getPerspectiveTransform(srcPoints, dstPoints),
			documentSize
		);


		// show scanned documnet
		imshow("Document", imgWarp(roi));


		/* draw document contour */
		line(img, documentContour[0], documentContour[1], Scalar(255, 0, 255), 2);
		line(img, documentContour[1], documentContour[3], Scalar(255, 0, 255), 2);
		line(img, documentContour[2], documentContour[0], Scalar(255, 0, 255), 2);
		line(img, documentContour[3], documentContour[2], Scalar(255, 0, 255), 2);

		for (int i = 0; i < 4; i++)
		{
			circle(img, documentContour[i], 8, Scalar(0, 255, 0), FILLED);
			putText(img, to_string(i), documentContour[i], FONT_HERSHEY_PLAIN, 4, Scalar(0, 255, 0), 3);
		}

	}

	// show camera image
	imshow("Image", img);

	// wait 20 ms and continue if no key pressed
	waitKey(0);

	// destroy output windows
	//destroyAllWindows();

	return 0;
}