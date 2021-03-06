//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/ml.hpp>
#include <opencv/cv.hpp>
#include <opencv/highgui.h>
#include <opencv/ml.h>



typedef unsigned char BYTE;

using namespace cv;
using namespace cv::ml;

#define MAX_NUM_IMAGES	60000

class DigitRecognizer
{
public:
	DigitRecognizer();
	~DigitRecognizer();
	
	bool train(char* trainPath, char* labelsPath);
	int classify(Mat img);

private:
	cv::Mat preprocessImage(Mat img);
	int readFlippedInteger(FILE *fp);

private:
	Ptr<ml::KNearest> knn;
	int numRows, numCols, numImages;
};