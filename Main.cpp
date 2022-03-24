#include "DocumentScanner.hpp"


int main()
{
	DocumentScanner docScan;

	docScan.readImage("Resources/img_01.jpg");
	docScan.setAspectRatio(DocumentFormat::VERTICAL_A4);
	docScan.setCropBorder(10);

	docScan.detectDocument();

	if (docScan.idDocumentDetected())
	{
		docScan.showDocument();

		docScan.showImage(Image::ORIGINAL);
		docScan.showImage(Image::BLUR);
		docScan.showImage(Image::GRAY);
		docScan.showImage(Image::CANNY);
		docScan.showImage(Image::DILATE);
		docScan.showImage(Image::CONTOUR);
	}
	
	cv::waitKey(0);

	return 0;
}