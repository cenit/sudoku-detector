#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

void drawLine(Vec2f line, Mat &img, Scalar rgb = CV_RGB(0,0,255));
void mergeRelatedLines(vector<Vec2f> *lines, Mat &img);

