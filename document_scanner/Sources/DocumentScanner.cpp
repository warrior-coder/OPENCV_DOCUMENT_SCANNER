#include "DocumentScanner.hpp"


double DocumentScanner::_LineLength(const cv::Point& point1, const cv::Point& point2)
{
	return sqrt(
		(point2.x - point1.x) * (point2.x - point1.x) + (point2.y - point1.y) * (point2.y - point1.y)
	);
}

DocumentScanner::DocumentScanner()
	: _cropBorder(0)
	, _documentDetected(false)
	, _aspectRatio(0.0, 0.0)
	, _documentSize(0, 0)
	, _showScale(1.0)
{}

void DocumentScanner::ReadImage(const std::string& filePath)
{
	_imgOrig = cv::imread(filePath);
}

void DocumentScanner::SetCropBorder(int cropBorder)
{
	_cropBorder = cropBorder;
}

void DocumentScanner::SetShowScale(double showScale)
{
	_showScale = showScale;
}

void DocumentScanner::ShowImage(const Image imageFlag) const
{
	cv::Mat imgShow;
	
	switch (imageFlag)
	{
	case Image::ORIGINAL:
	{
		if (_imgOrig.empty()) throw std::exception("NO_IMAGE_TO_SHOW");

		cv::resize(_imgOrig, imgShow, cv::Size(), _showScale, _showScale, cv::INTER_AREA);
		imshow("Image Original", imgShow);

		break;
	}
	case Image::BLUR:
	{
		if (_imgBlur.empty()) throw std::exception("NO_IMAGE_TO_SHOW");

		cv::resize(_imgBlur, imgShow, cv::Size(), _showScale, _showScale, cv::INTER_AREA);
		imshow("Image Blur", imgShow);

		break;
	}
	case Image::GRAY:
	{
		if (_imgGray.empty()) throw std::exception("NO_IMAGE_TO_SHOW");

		cv::resize(_imgGray, imgShow, cv::Size(), _showScale, _showScale, cv::INTER_AREA);
		imshow("Image Gray", imgShow);

		break;
	}
	case Image::CANNY:
	{
		if (_imgCan.empty()) throw std::exception("NO_IMAGE_TO_SHOW");

		cv::resize(_imgCan, imgShow, cv::Size(), _showScale, _showScale, cv::INTER_AREA);
		imshow("Image Canny", imgShow);

		break;
	}
	case Image::DILATE:
	{
		if (_imgDil.empty()) throw std::exception("NO_IMAGE_TO_SHOW");

		cv::resize(_imgDil, imgShow, cv::Size(), _showScale, _showScale, cv::INTER_AREA);
		imshow("Image Dilate", imgShow);

		break;
	}
	case Image::CONTOUR:
	{
		if (_imgCont.empty()) throw std::exception("NO_IMAGE_TO_SHOW");

		cv::resize(_imgCont, imgShow, cv::Size(), _showScale, _showScale, cv::INTER_AREA);
		imshow("Image Contour", imgShow);

		break;
	}
	}
}

cv::Mat& DocumentScanner::GetDocument()
{
	if (!_documentDetected) throw std::exception("NO_DOCUMENT_TO_GET");
	
	return _imgDoc;
}

void DocumentScanner::ShowDocument() const
{
	if (!_documentDetected) throw std::exception("NO_DOCUMENT_TO_SHOW");

	cv::Mat imgShow;

	cv::resize(_imgDoc, imgShow, cv::Size(), 0.3, 0.3, cv::INTER_AREA);
	cv::imshow("Document", imgShow);
}

void DocumentScanner::SetAspectRatio(double x, double y)
{
	_aspectRatio = { x, y };
}

void DocumentScanner::SetAspectRatio(DocumentFormat documentFormat)
{
	switch (documentFormat)
	{
	case DocumentFormat::VERTICAL_A4:
	{
		_aspectRatio = { 1.0, 1.41421356237 };
		break;
	}
	case DocumentFormat::HORISONTAL_A4:
	{
		_aspectRatio = { 1.41421356237, 1.0 };
		break;
	}
	case DocumentFormat::SQUARE:
	{
		_aspectRatio = { 1.0, 1.0 };
		break;
	}
	}
}

void DocumentScanner::DetectDocument()
{
	if (_imgOrig.empty()) throw std::exception("NO_IMAGE_TO_SCAN");

	// preprocess image
	cv::GaussianBlur(_imgOrig, _imgBlur, cv::Size(3, 3), 3, 0);	// blur the image

	cvtColor(_imgBlur, _imgGray, cv::COLOR_BGR2GRAY);				// convert color from BGR to GRAY pallete

	cv::Canny(_imgGray, _imgCan, 25, 75);							// use Canny edge detector

	const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	dilate(_imgCan, _imgDil, kernel);


	// detect document contour
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Point> approxedContour;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(_imgDil, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	double length, area, maxArea = 0.0;
	_documentDetected = false;

	for (const auto& contour : contours)
	{
		area = contourArea(contour);

		if (area > 1000.0)
		{
			length = arcLength(contour, true);
			approxPolyDP(contour, approxedContour, 0.02 * length, true);

			if (area > maxArea && approxedContour.size() == 4)
			{
				maxArea = area;
				_documentDetected = true;

				for (int i = 0; i < 4; i++)
				{
					_documentContour[i] = approxedContour[i];
				}
			}
		}
	}

	if (_documentDetected)
	{
		// reorder document points
		int sumPoints[4]{}, subPoints[4]{};

		for (int i = 0; i < 4; i++)
		{
			sumPoints[i] = _documentContour[i].x + _documentContour[i].y;
			subPoints[i] = _documentContour[i].x - _documentContour[i].y;
		}

		_documentContour = {
			_documentContour[std::min_element(sumPoints, sumPoints + 4U) - sumPoints],
			_documentContour[std::max_element(subPoints, subPoints + 4U) - subPoints],
			_documentContour[std::min_element(subPoints, subPoints + 4U) - subPoints],
			_documentContour[std::max_element(sumPoints, sumPoints + 4U) - sumPoints]
		};

		// calculate document size
		if (_aspectRatio.width * _aspectRatio.height == 0.0) throw std::exception("aspect_ratio_not_set");

		const double line01Length = _LineLength(_documentContour[0], _documentContour[1]);
		const double line23Length = _LineLength(_documentContour[2], _documentContour[3]);
		const double minLength = std::min(line01Length, line23Length);

		_documentSize = {
			static_cast<int>(minLength),
			static_cast<int>(minLength / _aspectRatio.width * _aspectRatio.height)
		};

		// get warp perspective
		const cv::Point2f srcPoints[] = {
			_documentContour[0],
			_documentContour[1],
			_documentContour[2],
			_documentContour[3]
		};
		const cv::Point2f dstPoints[] = {
			{ 0.0f, 0.0f },
			{ static_cast<float>(_documentSize.width), 0.0f },
			{ 0.0f, static_cast<float>(_documentSize.height) },
			{ static_cast<float>(_documentSize.width), static_cast<float>(_documentSize.height) }
		};
		const cv::Mat perspectiveTransform = getPerspectiveTransform(srcPoints, dstPoints);

		warpPerspective(
			_imgOrig,
			_imgWarp,
			perspectiveTransform,
			_documentSize
		);

		// crop borders
		const cv::Rect roi(
			_cropBorder,
			_cropBorder,
			_documentSize.width - _cropBorder * 2,
			_documentSize.height - _cropBorder * 2
		);

		_imgDoc = _imgWarp(roi);

		// draw document contour
		_imgOrig.copyTo(_imgCont);

		cv::line(_imgCont, _documentContour[0], _documentContour[1], cv::Scalar(0, 0, 255), 15);
		cv::line(_imgCont, _documentContour[1], _documentContour[3], cv::Scalar(0, 0, 255), 15);
		cv::line(_imgCont, _documentContour[2], _documentContour[0], cv::Scalar(0, 0, 255), 15);
		cv::line(_imgCont, _documentContour[3], _documentContour[2], cv::Scalar(0, 0, 255), 15);

		for (int i = 0; i < 4; i++)
		{
			cv::circle(_imgCont, _documentContour[i], 30, cv::Scalar(0, 255, 0), cv::FILLED);
			cv::putText(_imgCont, std::to_string(i), _documentContour[i], cv::FONT_HERSHEY_PLAIN, 15, cv::Scalar(255, 0, 0), 15);
		}
	}
}

bool DocumentScanner::IsEmpty() const
{
	return _imgOrig.empty();
}

bool DocumentScanner::IsDocumentDetected() const
{
	return _documentDetected;
}

[[maybe_unused]] int DocumentScanner::WaitKey(int delay) const
{
	return cv::waitKey(delay);
}