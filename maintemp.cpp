#include <iostream>
#include <opencv2/opencv.hpp>
#include "ImageManipulator.hpp"

int main(void)
{
  ImageManipulator *manipulator = new  ImageManipulator();
  cv::VideoCapture cap("video.mp4");
  int key = -1;
  cv::Mat img, img2, img3;

  if (!cap.isOpened())
    {
      std::cout << "Failed to open" << std::endl;
      return -1;
    }
  while (key < 0)
    {
      cap >> img;
      if (img.empty())
	break;
      if (img2.data == NULL)
	{
	  img2.create(img.rows, img.cols, img.type());
	  img3.create(img.rows, img.cols, 1);
	}
      manipulator->edgeDetect2(img, img2);
      cv::imshow("Image", img2);
      key = cv::waitKey(1);
    }
  cap.release();
  img.release();
  delete manipulator;
  return 0;
}
