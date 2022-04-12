#pragma once

#include <array>
#include <vector>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

enum class Image
{
	ORIGINAL,
	GRAY,
	BLUR,
	CANNY,
	DILATE,
	CONTOUR,
	ALL_CONCATINATE
};

enum class DocumentFormat
{
	VERTICAL_A4,
	HORISONTAL_A4,
	SQUARE
};

using AspectRatio = cv::Size2d;

class DocumentScanner
{
	cv::Mat imgOrig, imgGray, imgBlur, imgCan, imgDil, imgCont;
	cv::Mat imgWarp;
	cv::Mat imgDoc;

	AspectRatio aspectRatio;
	cv::Size documentSize;
	int cropBorder;

	std::array<cv::Point, 4> documentContour;
	
	bool documentDetected;

	double lineLength(cv::Point point1, cv::Point point2);

public:

	DocumentScanner();
	void readImage(std::string path);
	void setCropBorder(int cropBorder);

	void showImage(Image image);
	cv::Mat& getDocument();
	void showDocument();
	void setAspectRatio(double x, double y);
	void setAspectRatio(DocumentFormat documentFormat);

	void detectDocument();
	bool idDocumentDetected();
};