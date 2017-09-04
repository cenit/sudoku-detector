// Copyright Utkarsh Sinha (2010)
// Modified Stefano Sinigardi (2017)

#include "sudoku-tools.h"


int main()
{
  // We load the image in grayscale mode. We don't want to bother with the colour information, so just skip it. 
  Mat sudoku = imread("../data/sudoku-original.jpg", 0);

  // we create a blank image of the same size. This image will hold the actual outer box of puzzle:
  Mat original = sudoku.clone();

  // Create a duplicate. We'll try to extract grid lines in this image
  Mat outerBox = Mat(sudoku.size(), CV_8UC1);

  // Blur the image a little. This smooths out the noise a bit and makes extracting the grid lines easier.
  GaussianBlur(sudoku, sudoku, Size(11, 11), 0);

  // With the noise smoothed out, we can now threshold the image. 
  // The image can have varying illumination levels, so a good choice for a thresholding algorithm would be an adaptive threshold.
  // It calculates a threshold level several small windows in the image. 
  // This threshold level is calculated using the mean level in the window, so it keeps things illumination independent.
  // It calculates a mean over a 5x5 window and subtracts 2 from the mean. This is the threshold level for every pixel.
  adaptiveThreshold(sudoku, outerBox, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5, 2);

  // Since we're interested in the borders, and they are black, we invert the image outerBox. 
  // Then, the borders of the puzzles are white (along with other noise).
  bitwise_not(outerBox, outerBox);

  // This thresholding operation can disconnect certain connected parts (like lines). 
  // So dilating the image once will fill up any small "cracks" that might have crept in.
  // Note that I've used a plus shaped structuring element here (the kernel matrix).
  Mat kernel = (Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
  dilate(outerBox, outerBox, kernel);


  // Finding the biggest blob
  // First, I use the floodfill command. This command returns a bounding rectangle of the pixels it filled. 
  // We've assumed the biggest thing in the picture to be the puzzle. So the biggest blob should have be the puzzle. 
  // Since it is the biggest, it will have the biggest bounding box as well. 
  // So we find the biggest bounding box, and save the location where we did the flood fill.
  // We iterate through the image. The >=128 condition is to ensure that only the white parts are flooded. 
  // Whenever we encounter such a part, we flood it with a dark gray colour (gray level 64). 
  // So in the future, we won't be reflooding these blobs. And whenever we encounter a big blob, we note the current point and the area it has.
  int count = 0;
  int max = -1;
  Point maxPt;
  Mat cloneOuterBox = outerBox.clone();
  for (int y = 0; y < outerBox.size().height; y++)
  {
    uchar *row = outerBox.ptr(y);
    for (int x = 0; x < outerBox.size().width; x++)
    {
      if (row[x] >= 128)
      {
        int area = floodFill(outerBox, Point(x, y), CV_RGB(0, 0, 64));

        if (area > max)
        {
          maxPt = Point(x, y);
          max = area;
        }
      }
    }
  }

  // Now, we have several blobs filled with a dark gray colour (level 64). 
  // And we also know the point what produces a blob with maximum area. So we floodfill that point with white
  floodFill(outerBox, maxPt, CV_RGB(255, 255, 255));

  // Now, the biggest blob is white. We need to turn the other blobs black. We do that here.
  // Wherever a dark gray point is enountered, it is flooded with black, effectively "hiding" it.
  for (int y = 0; y < outerBox.size().height; y++)
  {
    uchar *row = outerBox.ptr(y);
    for (int x = 0; x < outerBox.size().width; x++)
    {
      if (row[x] == 64 && x != maxPt.x && y != maxPt.y)
      {
        int area = floodFill(outerBox, Point(x, y), CV_RGB(0, 0, 0));
      }
    }
    //printf("Current row: %d\n", y);
  }

  // Because we had dilated the image earlier, we'll "restore" it a bit by eroding it:
  erode(outerBox, outerBox, kernel);
  //imshow("thresholded", outerBox);


  // Detecting lines
  // At this point, we have a single blob. Now its time to find lines. This is done with the Hough transform. 
  // OpenCV comes with it. So a line of code is all that's needed.
  vector<Vec2f> lines;
  HoughLines(outerBox, lines, 1, CV_PI / 180, 200);

  // Each physical line has several possible approximations. This is usually because the physical line is thick. 
  // So, just these lines aren't enough for figuring out where the puzzle is located. We'll have to do some math with these lines.
  // One way to fix this is to "merge" lines that are close by. We wrote mergeRelatedLines to do it!
  mergeRelatedLines(&lines, sudoku);

  // Then we draw our lines in the image
  printf("Size of lines: %d\n", ((int)lines.size()));
  for (int i = 0; i < lines.size(); i++)
  {
    drawLine(lines[i], outerBox, CV_RGB(0, 0, 128));
  }

  imshow("thresholded", outerBox);

  // We'll try and detect lines that are nearest to the top edge, bottom edge, right edge and the left edge. 
  // These will be the outer boundaries of the sudoku puzzle. We start by adding these lines after the mergeRelatedLines call.
  // The initial values of each edge is initially set to a ridiculous value. This will ensure it gets to the proper edge later on. 
  Vec2f topEdge = Vec2f(1000, 1000);	      double topYIntercept = 100000, topXIntercept = 0;
  Vec2f bottomEdge = Vec2f(-1000, -1000);	double bottomYIntercept = 0, bottomXIntercept = 0;
  Vec2f leftEdge = Vec2f(1000, 1000);	    double leftXIntercept = 100000, leftYIntercept = 0;
  Vec2f rightEdge = Vec2f(-1000, -1000);		double rightXIntercept = 0, rightYIntercept = 0;

  // Now we loop over all lines:
  for (int i = 0; i < lines.size(); i++)
  {
    Vec2f current = lines[i];
    // We store the rho and theta values. 
    float p = current[0];
    float theta = current[1];

    // If we encounter a "merged" line, we simply skip it
    if (p == 0 && theta == -100) continue;

    // Now we use the normal form of line to calculate the x and y intercepts (the place where the lines intersects the X and Y axis)
    double xIntercept, yIntercept;
    xIntercept = p / cos(theta);
    yIntercept = p / (cos(theta)*sin(theta));

    // We will ignore any line that has slope different from horizontal or vertical. 
    // Vertical case:
    if (theta > CV_PI * 80 / 180 && theta < CV_PI * 100 / 180)
    {
      if (p < topEdge[0])
        topEdge = current;

      if (p > bottomEdge[0])
        bottomEdge = current;
    }
    // Horizontal case:
    else if (theta<CV_PI * 10 / 180 || theta>CV_PI * 170 / 180)
    {
      if (xIntercept > rightXIntercept)
      {
        rightEdge = current;
        rightXIntercept = xIntercept;
      }
      else if (xIntercept <= leftXIntercept)
      {
        leftEdge = current;
        leftXIntercept = xIntercept;
      }
    }
  }

  // Just for visualizing it, we'll draw those lines on the original image:
  drawLine(topEdge, sudoku, CV_RGB(0, 0, 0));
  drawLine(bottomEdge, sudoku, CV_RGB(0, 0, 0));
  drawLine(leftEdge, sudoku, CV_RGB(0, 0, 0));
  drawLine(rightEdge, sudoku, CV_RGB(0, 0, 0));


  // Next, we'll calculate the intersections of these four lines. First, we find two points on each line. 
  // Then using some math, we can calculate exactly where any two particular lines intersect.
  // The code below finds two points on a line. The right and left edges need the "if" construct. 
  // These edges are vertical. They can have infinite slope, something a computer cannot represent. 
  // So I check if they have infinite slope or not. If it does, calculate two points using a "safe" method. 
  // Otherwise, the normal method can be used.
  Point left1, left2, right1, right2, bottom1, bottom2, top1, top2;

  int height = outerBox.size().height;
  int width = outerBox.size().width;

  if (leftEdge[1] != 0)
  {
    left1.x = 0;		left1.y = leftEdge[0] / sin(leftEdge[1]);
    left2.x = width;	left2.y = -left2.x / tan(leftEdge[1]) + left1.y;
  }
  else
  {
    left1.y = 0;		left1.x = leftEdge[0] / cos(leftEdge[1]);
    left2.y = height;	left2.x = left1.x - height*tan(leftEdge[1]);
  }

  if (rightEdge[1] != 0)
  {
    right1.x = 0;		right1.y = rightEdge[0] / sin(rightEdge[1]);
    right2.x = width;	right2.y = -right2.x / tan(rightEdge[1]) + right1.y;
  }
  else
  {
    right1.y = 0;		right1.x = rightEdge[0] / cos(rightEdge[1]);
    right2.y = height;	right2.x = right1.x - height*tan(rightEdge[1]);
  }

  bottom1.x = 0;	bottom1.y = bottomEdge[0] / sin(bottomEdge[1]);
  bottom2.x = width; bottom2.y = -bottom2.x / tan(bottomEdge[1]) + bottom1.y;

  top1.x = 0;		top1.y = topEdge[0] / sin(topEdge[1]);
  top2.x = width;	top2.y = -top2.x / tan(topEdge[1]) + top1.y;

  // This part calculates the actual intersection points
  double leftA = left2.y - left1.y;
  double leftB = left1.x - left2.x;
  double leftC = leftA*left1.x + leftB*left1.y;

  double rightA = right2.y - right1.y;
  double rightB = right1.x - right2.x;
  double rightC = rightA*right1.x + rightB*right1.y;

  double topA = top2.y - top1.y;
  double topB = top1.x - top2.x;
  double topC = topA*top1.x + topB*top1.y;

  double bottomA = bottom2.y - bottom1.y;
  double bottomB = bottom1.x - bottom2.x;
  double bottomC = bottomA*bottom1.x + bottomB*bottom1.y;

  // Intersection of left and top
  double detTopLeft = leftA*topB - leftB*topA;
  CvPoint ptTopLeft = cvPoint((topB*leftC - leftB*topC) / detTopLeft, (leftA*topC - topA*leftC) / detTopLeft);

  // Intersection of top and right
  double detTopRight = rightA*topB - rightB*topA;
  CvPoint ptTopRight = cvPoint((topB*rightC - rightB*topC) / detTopRight, (rightA*topC - topA*rightC) / detTopRight);

  // Intersection of right and bottom
  double detBottomRight = rightA*bottomB - rightB*bottomA;
  CvPoint ptBottomRight = cvPoint((bottomB*rightC - rightB*bottomC) / detBottomRight, (rightA*bottomC - bottomA*rightC) / detBottomRight);

  // Intersection of bottom and left
  double detBottomLeft = leftA*bottomB - leftB*bottomA;
  CvPoint ptBottomLeft = cvPoint((bottomB*leftC - leftB*bottomC) / detBottomLeft, (leftA*bottomC - bottomA*leftC) / detBottomLeft);

  // Now we draw the intersection points on the image
  cv::line(sudoku, ptTopRight, ptTopRight, CV_RGB(255, 0, 0), 10);
  cv::line(sudoku, ptTopLeft, ptTopLeft, CV_RGB(255, 0, 0), 10);
  cv::line(sudoku, ptBottomRight, ptBottomRight, CV_RGB(255, 0, 0), 10);
  cv::line(sudoku, ptBottomLeft, ptBottomLeft, CV_RGB(255, 0, 0), 10);

  imshow("Sudoku with edges and edge lines", sudoku);


  // Correct the perspective transform
  // We have the points. Now we can correct the skewed perspective. First, we find the longest edge of the puzzle. 
  // The new image will be a square of the length of the longest edge.
  // Simple code. We calculate the length of each edge. Whenever we find a longer edge, we store its length squared. 
  int maxLength = (ptBottomLeft.x - ptBottomRight.x)*(ptBottomLeft.x - ptBottomRight.x) + (ptBottomLeft.y - ptBottomRight.y)*(ptBottomLeft.y - ptBottomRight.y);
  int temp = (ptTopRight.x - ptBottomRight.x)*(ptTopRight.x - ptBottomRight.x) + (ptTopRight.y - ptBottomRight.y)*(ptTopRight.y - ptBottomRight.y);
  if (temp > maxLength) maxLength = temp;
  // 
  temp = (ptTopRight.x - ptTopLeft.x)*(ptTopRight.x - ptTopLeft.x) + (ptTopRight.y - ptTopLeft.y)*(ptTopRight.y - ptTopLeft.y);
  if (temp > maxLength) maxLength = temp;

  temp = (ptBottomLeft.x - ptTopLeft.x)*(ptBottomLeft.x - ptTopLeft.x) + (ptBottomLeft.y - ptTopLeft.y)*(ptBottomLeft.y - ptTopLeft.y);
  if (temp > maxLength) maxLength = temp;
  // And finally when we have the longest edge, we do a square root to get its exact length.
  maxLength = sqrt((double)maxLength);

  // We create source and destination points
  // The top left point in the source is equivalent to the point (0,0) in the corrected image. And so on.
  Point2f src[4], dst[4];
  src[0] = ptTopLeft;		    dst[0] = Point2f(0, 0);
  src[1] = ptTopRight;	    dst[1] = Point2f(maxLength - 1, 0);
  src[2] = ptBottomRight;		dst[2] = Point2f(maxLength - 1, maxLength - 1);
  src[3] = ptBottomLeft;		dst[3] = Point2f(0, maxLength - 1);

  // We create a new image and do the undistortion
  Mat undistorted = Mat(Size(maxLength, maxLength), CV_8UC1);
  cv::warpPerspective(original, undistorted, cv::getPerspectiveTransform(src, dst), Size(maxLength, maxLength));

  //imshow("Lines", outerBox);

  Mat undistortedThreshed = undistorted.clone();

  // Show this sample
  //threshold(undistorted, undistortedThreshed, 128, 255, CV_THRESH_BINARY_INV);
  adaptiveThreshold(undistorted, undistortedThreshed, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 101, 1);

  imshow("undistorted", undistortedThreshed);
  waitKey(0);

  return 0;
}

