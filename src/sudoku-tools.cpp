
#include "sudoku-tools.h"


void drawLine(Vec2f line, Mat &img, Scalar rgb)
{
  if (line[1] != 0)
  {
    float m = -1 / tan(line[1]);
    float c = line[0] / sin(line[1]);

    cv::line(img, Point(0, c), Point(img.size().width, m*img.size().width + c), rgb);
  }
  else
  {
    cv::line(img, Point(line[0], 0), Point(line[0], img.size().height), rgb);
  }
}

void mergeRelatedLines(vector<Vec2f> *lines, Mat &img)
{
  // The iterator helps traverse the array list. Each element of the list contains 2 things: rho and theta (the normal form of a line).
  // During the merging process, certain lines will fuse together. 
  vector<Vec2f>::iterator current;
  vector<Vec4i> points(lines->size());
  for (current = lines->begin(); current != lines->end(); current++)
  {
    // We'll need to mark lines that have been fused (so they aren't considered for other things). 
    // This is done by setting the rho to zero and theta to -100 (an impossible value). 
    // So whenever we encounter such a line, we simply skip it:
    if ((*current)[0] == 0 && (*current)[1] == -100) continue;

    // Now, we store the rho and theta for the current line in two variables:
    float p1 = (*current)[0];
    float theta1 = (*current)[1];

    // With these two values, we find two points on the line
    // If the is horizontal (theta is around 90 degrees), we find a point at the extreme left (x=0) 
    // and one at the extreme right (x=img.width). If not, we find a point at the extreme top (y=0) 
    // and one at extreme bottom (y=img.height). All the calculations are done based on the normal form of a line.
    Point pt1current, pt2current;
    if (theta1 > CV_PI * 45 / 180 && theta1 < CV_PI * 135 / 180)
    {
      pt1current.x = 0;
      pt1current.y = p1 / sin(theta1);

      pt2current.x = img.size().width;
      pt2current.y = -pt2current.x / tan(theta1) + p1 / sin(theta1);
    }
    else
    {
      pt1current.y = 0;
      pt1current.x = p1 / cos(theta1);

      pt2current.y = img.size().height;
      pt2current.x = -pt2current.y / tan(theta1) + p1 / cos(theta1);
    }

    // We start iterating over the lines again
    // With this loop, we can compare every line with every other line. 
    vector<Vec2f>::iterator	pos;
    for (pos = lines->begin(); pos != lines->end(); pos++)
    {
      // If we find that the line current is the same as the line pos, we continue. No point fusing the same line.
      if (*current == *pos)
        continue;

      // Now we check if the lines are within a certain distance of each other:
      if (fabs((*pos)[0] - (*current)[0]) < 20 && fabs((*pos)[1] - (*current)[1]) < CV_PI * 10 / 180)
      {
        // If they are, we store the rho and theta for the line pos.
        float p = (*pos)[0];
        float theta = (*pos)[1];

        // And again, we find two points on the line pos:
        Point pt1, pt2;
        if ((*pos)[1] > CV_PI * 45 / 180 && (*pos)[1] < CV_PI * 135 / 180)
        {
          pt1.x = 0;
          pt1.y = p / sin(theta);

          pt2.x = img.size().width;
          pt2.y = -pt2.x / tan(theta) + p / sin(theta);
        }
        else
        {
          pt1.y = 0;
          pt1.x = p / cos(theta);

          pt2.y = img.size().height;
          pt2.x = -pt2.y / tan(theta) + p / cos(theta);
        }

        // If endpoints of the line pos and the line current are close to each other, we fuse them:
        if (((double)(pt1.x - pt1current.x)*(pt1.x - pt1current.x) + (pt1.y - pt1current.y)*(pt1.y - pt1current.y) < 64 * 64) && ((double)(pt2.x - pt2current.x)*(pt2.x - pt2current.x) + (pt2.y - pt2current.y)*(pt2.y - pt2current.y) < 64 * 64))
        {
          printf("Merging\n");
          // Merge the two
          (*current)[0] = ((*current)[0] + (*pos)[0]) / 2;
          (*current)[1] = ((*current)[1] + (*pos)[1]) / 2;

          (*pos)[0] = 0;
          (*pos)[1] = -100;
        }
      }
    }
  }

  //return lines;
}

