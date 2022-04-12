#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

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
	cv::Mat _imgOrig;
	cv::Mat _imgBlur, _imgGray, _imgCan, _imgDil, _imgCont;
	cv::Mat _imgWarp;
	cv::Mat _imgDoc;

	AspectRatio _aspectRatio;
	cv::Size _documentSize;
	int _cropBorder;
	double _showScale;

	std::array<cv::Point, 4> _documentContour;
	
	bool _documentDetected;

	double _LineLength(const cv::Point& point1, const cv::Point& point2);

public:

	DocumentScanner();
	~DocumentScanner() = default;

	void ReadImage(const std::string& filePath);
	void SetCropBorder(int cropBorder);

	void ShowImage(const Image imageFlag) const;
	void ShowDocument() const;

	cv::Mat& GetDocument();

	void SetAspectRatio(double x, double y);
	void SetAspectRatio(const DocumentFormat documentFormat);
	void SetShowScale(double showScale);

	void DetectDocument();

	bool IsEmpty() const;
	bool IsDocumentDetected() const;

	[[maybe_unused]] int WaitKey(int delay = 0) const;
};