#include <iostream>
#include "DocumentScanner.hpp"

int main()
{
	DocumentScanner docScan;

	docScan.ReadImage("Resources/img_02.jpg");

	if (docScan.IsEmpty())
	{
		std::cout << "Image load failed" << std::endl;
		return 1;
	}

	docScan.SetAspectRatio(DocumentFormat::VERTICAL_A4);
	docScan.SetCropBorder(20);
	docScan.SetShowScale(0.2);
	docScan.DetectDocument();

	if (!docScan.IsDocumentDetected())
	{
		std::cout << "Document detect failed" << std::endl;
		return 2;
	}

	docScan.ShowDocument();
	docScan.ShowImage(Image::ORIGINAL);
	docScan.ShowImage(Image::BLUR);
	docScan.ShowImage(Image::GRAY);
	docScan.ShowImage(Image::CANNY);
	docScan.ShowImage(Image::DILATE);
	docScan.ShowImage(Image::CONTOUR);

	docScan.WaitKey();

	return 0;
}