#include <iostream>
#include <opencv2/opencv.hpp>
#include "ImageManipulator.hpp"

int main(void)
{
  ImageManipulator *manipulator = new  ImageManipulator();
  cv::VideoCapture cap(0);
  //    cv::VideoCapture cap("video.mp4");
  int key = -1;
  cv::Mat img, img2, img3;
  cv::Mat r, g, b;
  cv::Mat r_edge, g_edge, b_edge;
  

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
	  img3.create(img.rows, img.cols, CV_8U);
	  r_edge.create(img.rows, img.cols, CV_8U);
	  g_edge.create(img.rows, img.cols, CV_8U);
	  b_edge.create(img.rows, img.cols, CV_8U);
	}
      cv::cvtColor(img, img3, CV_RGB2GRAY);
      //manipulator->split(img, r, g, b);
      //manipulator->edgeDetect3(img,
      //		       img2);
      /*      
	      manipulator->edgeDetect2(g,
	      g_edge);
	      manipulator->edgeDetect3(b,
	      b_edge);
	      manipulator->combine(img, r_edge, g_edge, b_edge);*/
      
      manipulator->blur(img3, r_edge);
      manipulator->sharpen(r_edge, img3);
      cv::imshow("original", img3);
      cv::imshow("blur", r_edge);
      //cv::imshow("Green", g_edge);
      //cv::imshow("Blue", b_edge);
      key = cv::waitKey(1);
    }
  cap.release();
  img.release();
  delete manipulator;
  return 0;
}
