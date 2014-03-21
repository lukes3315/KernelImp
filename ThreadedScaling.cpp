#include <opencv2/opencv.hpp>
#include <pthread.h>
#include "ImageManipulator.hpp"

int getGrayPixel(int x, int y, cv::Mat & img)
{
  return img.data[y * img.cols + x];
}

int avgSurrounds(int x, int y, cv::Mat & img)
{
  int avg_Gray = 0;
      
  avg_Gray += getGrayPixel(x - 1, y - 1, img);
  avg_Gray += getGrayPixel(x, y - 1, img);
  avg_Gray += getGrayPixel(x + 1, y - 1, img);
  avg_Gray += getGrayPixel(x - 1, y, img);
  avg_Gray += getGrayPixel(x, y, img);
  avg_Gray += getGrayPixel(x + 1, y, img);
  avg_Gray += getGrayPixel(x - 1, y + 1, img);
  avg_Gray += getGrayPixel(x, y + 1, img);
  avg_Gray += getGrayPixel(x + 1, y + 1, img);
  avg_Gray /= 9;
  if (avg_Gray > 255)
    avg_Gray = 255;
  if (avg_Gray < 0)
    avg_Gray = 0;
  return avg_Gray;
}

void * averageImage(void * data)
{
  cv::Mat * img = (cv::Mat*)data;

  for (int y = 0 ; y < img->rows    ; ++y)
    {
      for (int x = 0; x < img->cols ; ++x)
	{
	  if (x > 0 && y > 0
	      && x < img->cols - 1
	      && y < img->rows - 1)
	    {
	      int avg = (x, y, *img);
	      img->data[y * img->cols + x] = avg;
	    }
	}
    }
  return (void*)img;
}

int main(void)
{
  cv::VideoCapture cap(0);
  int key = -1;
  cv::Mat img;
  cv::Mat *r = NULL, *g = NULL, *b = NULL;
  ImageManipulator * imageManipulator = new ImageManipulator();
  pthread_t p[3];

  if (!cap.isOpened())
    {
      std::cout << "Unable to open device at index #0" << std::endl;
      return 0;
    }
  while (key < 0)
    {
      cap >> img;
      if (img.empty())
	break;
      if (r == NULL
	  && b == NULL
	  && g == NULL)
	{
	  r = new cv::Mat(img.rows, img.cols, CV_8U);
	  g = new cv::Mat(img.rows, img.cols, CV_8U);
	  b = new cv::Mat(img.rows, img.cols, CV_8U);
	}
      imageManipulator->split(img, *r, *g, *b);
       
      if (!pthread_create(&p[0], NULL, averageImage, r)
	  && !pthread_create(&p[1], NULL, averageImage, g)
	  && !pthread_create(&p[2], NULL, averageImage, b))
	{
	  if (!pthread_join(p[0], (void**)&r)
	      && !pthread_join(p[1], (void**)&g)
	      && !pthread_join(p[2], (void**)&b))
	    {
	      imageManipulator->combine(img, *r, *g, *b);
	    }
	}
      cv::imshow("Original", img);
      key = cv::waitKey(1);
    }
  cap.release();
  return -1;
}
