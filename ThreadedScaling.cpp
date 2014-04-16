#include <opencv2/opencv.hpp>
#include <pthread.h>
#include "ImageManipulator.hpp"

int getGrayPixel(int x, int y, cv::Mat & img)
{
  return img.data[y * img.cols + x];
}

int avgSurrounds(int x, int y, cv::Mat & img)
{
  //int avg_Gray = 0;
  char kernel[3][3];

  for (int i = 0 ; i < 3 ; ++i)
    {
      memset(kernel[i], -1, 3);
    }
  //kernel[0][0] = 1;
  //kernel[0][2] = 1;
  //kernel[2][0] = 1;
  //kernel[2][2] = 1;
  kernel[1][1] = 8;
  int avg = 0;
  int curr_x = 0, curr_y = 0;
  int middle = 1;//floor(3 / 2);
  for (int i = 0 ; i < 3 ; ++i)
    {
      for (int j = 0 ; j < 3 ; ++j)
	{
	  if (i != middle)
	    {
	      curr_y = y - (i - middle);
	    }
	  else if (i == middle)
	    {
	      curr_y = y;
	    }
	  if (j != middle)
	    {
	      curr_x = x - (j - middle);
	    }
	  else if (j == middle)
	    {
	      curr_x = x;
	    }
	  avg += getGrayPixel(curr_x, curr_y, img) * kernel[i][j];
	}
    }
  avg /= 1;
  if (avg > 255)
    avg = 255;
  if (avg < 0)
    avg = 0;
  /*
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
  */
  return avg;
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
	      int avg = avgSurrounds(x, y, *img);
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
