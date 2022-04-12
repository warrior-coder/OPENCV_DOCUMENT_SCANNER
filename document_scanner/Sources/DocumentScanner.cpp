#include "DocumentScanner.hpp"


double DocumentScanner::lineLength(cv::Point point1, cv::Point point2)
{
	return sqrt(
		(point2.x - point1.x) * (point2.x - point1.x) + (point2.y - point1.y) * (point2.y - point1.y)
	);
}

DocumentScanner::DocumentScanner()
	: cropBorder(0)
	, documentDetected(false)
	, aspectRatio(0.0, 0.0)
	, documentSize(0, 0)
{}

void DocumentScanner::readImage(std::string path)
{
	imgOrig = cv::imread(path);
}

void DocumentScanner::setCropBorder(int cropBorder)
{
	this->cropBorder = cropBorder;
}

void setShowScale(double fx, double fy)
{
	
}

void DocumentScanner::showImage(Image image)
{
	cv::Mat imgShow;
	double fx = 0.1;
	double fy = 0.1;

	switch (image)
	{
	case Image::ORIGINAL:
	{
		if (imgOrig.empty()) throw std::exception("no_image_to_show");

		cv::resize(imgOrig, imgShow, cv::Size(), fx, fy, cv::INTER_AREA);
		imshow("Image Original", imgShow);

		break;
	}
	case Image::BLUR:
	{
		if (imgBlur.empty()) throw std::exception("no_image_to_show");

		cv::resize(imgBlur, imgShow, cv::Size(), fx, fy, cv::INTER_AREA);
		imshow("Image Blur", imgShow);

		break;
	}
	case Image::GRAY:
	{
		if (imgGray.empty()) throw std::exception("no_image_to_show");

		cv::resize(imgGray, imgShow, cv::Size(), fx, fy, cv::INTER_AREA);
		imshow("Image Gray", imgShow);

		break;
	}
	case Image::CANNY:
	{
		if (imgCan.empty()) throw std::exception("no_image_to_show");

		cv::resize(imgCan, imgShow, cv::Size(), fx, fy, cv::INTER_AREA);
		imshow("Image Canny", imgShow);

		break;
	}
	case Image::DILATE:
	{
		if (imgDil.empty()) throw std::exception("no_image_to_show");

		cv::resize(imgDil, imgShow, cv::Size(), fx, fy, cv::INTER_AREA);
		imshow("Image Dilate", imgShow);

		break;
	}
	case Image::CONTOUR:
	{
		if (imgCont.empty()) throw std::exception("no_image_to_show");

		cv::resize(imgCont, imgShow, cv::Size(), fx, fy, cv::INTER_AREA);
		imshow("Image Cntour", imgShow);

		break;
	}
	case Image::ALL_CONCATINATE:
	{
		/*cv::Mat imgConcatTop;


		const std::vector<cv::Mat> hConcat = {
			imgOrig, imgBlur, imgCan
		};

		for (auto& mat : hConcat)
		{
			cv::resize(mat, mat, cv::Size(100, 100));
		}

		cv::hconcat(hConcat, imgConcatTop);
		//cv::hconcat(imgConcatTop, imgGray, imgConcatTop);

		//cv::hconcat(,imgCan, imgDil, imgConcatBottom);
		//cv::hconcat(imgConcatBottom, imgBlur, imgConcatBottom);


		//cv::resize(imgConcatBottom, imgConcatBottom, cv::Size(), 0.3, 0.3);
		//cv::resize(imgConcatTop, imgConcatTop, cv::Size(), 0.3, 0.3);
		//cv::vconcat(imgConcatTop, imgConcatBottom, imgConcat);
		cv::imshow("Images All", imgConcatTop);*/
	}
	}
}

cv::Mat& DocumentScanner::getDocument()
{
	if (!documentDetected) throw std::exception("no_document_to_get");
	
	return imgDoc;
}

void DocumentScanner::showDocument()
{
	if (!documentDetected) throw std::exception("no_document_to_show");
	cv::Mat imgShow;
	cv::resize(imgDoc, imgShow, cv::Size(), 0.3, 0.3, cv::INTER_AREA);

	cv::imshow("Document", imgShow);
}

void DocumentScanner::setAspectRatio(double x, double y)
{
	this->aspectRatio = { x, y };
}

void DocumentScanner::setAspectRatio(DocumentFormat documentFormat)
{
	switch (documentFormat)
	{
	case DocumentFormat::VERTICAL_A4:
	{
		this->aspectRatio = { 1.0, 1.41421356237 };
		break;
	}
	case DocumentFormat::HORISONTAL_A4:
	{
		this->aspectRatio = { 1.41421356237, 1.0 };
		break;
	}
	case DocumentFormat::SQUARE:
	{
		this->aspectRatio = { 1.0, 1.0 };
		break;
	}
	}
}

void DocumentScanner::detectDocument()
{
	if (imgOrig.empty()) throw std::exception("no_image_to_scan");


	/* preprocess image */
	cv::GaussianBlur(imgOrig, imgBlur, cv::Size(3, 3), 3, 0);	// blur the image

	cvtColor(imgBlur, imgGray, cv::COLOR_BGR2GRAY);				// convert color from BGR to GRAY pallete

	cv::Canny(imgGray, imgCan, 25, 75);							// use Canny edge detector

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	dilate(imgCan, imgDil, kernel);


	/* detect document contour  */
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Point> approxedContour;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(imgDil, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	double length, area, maxArea = 0.0;
	documentDetected = false;

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
				documentDetected = true;

				for (int i = 0; i < 4; i++)
				{
					documentContour[i] = approxedContour[i];
				}
			}
		}
	}

	if (documentDetected)
	{
		/* reorder document points */
		int sumPoints[4], subPoints[4];

		for (int i = 0; i < 4; i++)
		{
			sumPoints[i] = documentContour[i].x + documentContour[i].y;
			subPoints[i] = documentContour[i].x - documentContour[i].y;
		}

		documentContour = {
			documentContour[std::min_element(sumPoints, sumPoints + 4U) - sumPoints],
			documentContour[std::max_element(subPoints, subPoints + 4U) - subPoints],
			documentContour[std::min_element(subPoints, subPoints + 4U) - subPoints],
			documentContour[std::max_element(sumPoints, sumPoints + 4U) - sumPoints]
		};

		/* calculate document size */
		if (aspectRatio.width * aspectRatio.height == 0.0) throw std::exception("aspect_ratio_not_set");

		double line01Length = lineLength(documentContour[0], documentContour[1]);
		double line23Length = lineLength(documentContour[2], documentContour[3]);
		double minLength = std::min(line01Length, line23Length);

		documentSize = {
			static_cast<int>(minLength),
			static_cast<int>(minLength / aspectRatio.width * aspectRatio.height)
		};

		/* get warp perspective */
		const cv::Point2f srcPoints[] = {
			documentContour[0],
			documentContour[1],
			documentContour[2],
			documentContour[3]
		};
		const cv::Point2f dstPoints[] = {
			{ 0.0f, 0.0f },
			{ (float)documentSize.width, 0.0f },
			{ 0.0f, (float)documentSize.height },
			{ (float)documentSize.width, (float)documentSize.height }
		};
		cv::Mat perspectiveTransform = getPerspectiveTransform(srcPoints, dstPoints);

		warpPerspective(
			imgOrig,
			imgWarp,
			perspectiveTransform,
			documentSize
		);

		cv::Rect roi(cropBorder, cropBorder, documentSize.width - cropBorder * 2, documentSize.height - cropBorder * 2);

		imgDoc = imgWarp(roi);

		/* draw document contour */
		imgOrig.copyTo(imgCont);

		cv::line(imgCont, documentContour[0], documentContour[1], cv::Scalar(0, 0, 255), 15);
		cv::line(imgCont, documentContour[1], documentContour[3], cv::Scalar(0, 0, 255), 15);
		cv::line(imgCont, documentContour[2], documentContour[0], cv::Scalar(0, 0, 255), 15);
		cv::line(imgCont, documentContour[3], documentContour[2], cv::Scalar(0, 0, 255), 15);

		for (int i = 0; i < 4; i++)
		{
			cv::circle(imgCont, documentContour[i], 30, cv::Scalar(0, 255, 0), cv::FILLED);
			cv::putText(imgCont, std::to_string(i), documentContour[i], cv::FONT_HERSHEY_PLAIN, 15, cv::Scalar(255, 0, 0), 15);
		}
	}
}

bool DocumentScanner::idDocumentDetected()
{
	return documentDetected;
}
