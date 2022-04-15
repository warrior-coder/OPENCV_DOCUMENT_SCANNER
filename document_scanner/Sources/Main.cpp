#include <iostream>
#include "DocumentScanner.hpp"

int main()
{
	DocumentScanner docScan;

	docScan.ReadImage("Resources/Image1.jpg");

	if (docScan.IsEmpty())
	{
		std::cout << "Image load failed" << std::endl;
		return 1;
	}

	docScan.SetAspectRatio(DocumentFormat::VERTICAL_A4);
	docScan.SetCropBorder(30);
	docScan.SetShowScale(0.25);
	docScan.DetectDocument();

	if (!docScan.IsDocumentDetected())
	{
		std::cout << "Document detect failed" << std::endl;
		return 2;
	}

	auto doc = docScan.GetDocument();
	cv::rotate(doc, doc, cv::ROTATE_90_COUNTERCLOCKWISE);
	cv::resize(doc, doc, cv::Size(), 0.65, 0.65, cv::INTER_AREA);
	cv::imshow("Document", doc);

	docScan.ShowImage(Image::ORIGINAL);
	docScan.ShowImage(Image::BLUR);
	docScan.ShowImage(Image::GRAY);
	docScan.ShowImage(Image::CANNY);
	docScan.ShowImage(Image::DILATE);
	docScan.ShowImage(Image::CONTOUR);

	docScan.WaitKey();

	return 0;
}