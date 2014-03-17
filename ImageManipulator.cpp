#include "ImageManipulator.hpp"

ImageManipulator::ImageManipulator()
{
  _kernelSize = 3;
  initialize();
}

ImageManipulator::ImageManipulator(int size):_kernelSize(size)
{
  initialize();
}

ImageManipulator::~ImageManipulator()
{
  releaseKernel(&edgeKernel1);
  releaseKernel(&edgeKernel2);
  releaseKernel(&edgeKernel3);
  releaseKernel(&sharpenKernel);
  releaseKernel(&blurKernel);
}

void ImageManipulator::initialize()
{
  allocKernel(_kernelSize, &edgeKernel1);
  allocKernel(_kernelSize, &edgeKernel2);
  allocKernel(_kernelSize, &edgeKernel3);
  allocKernel(_kernelSize, &sharpenKernel);
  allocKernel(_kernelSize, &blurKernel);

  _middle = floor(_kernelSize/2);
  initEdgeKernel1();
  initEdgeKernel2();
  initEdgeKernel3();
  initSharpenKernel();
  initBlurKernel();
}

void ImageManipulator::allocKernel(int size, char *** kernel)
{
  *kernel = (char**)malloc(sizeof(char *) * (size + 1));
  for (int i = 0 ; i < size + 1 ; ++i)
    {
      (*kernel)[i] = (char*)malloc(sizeof(char) * (size + 1));
      (*kernel)[i] = (char*)memset((*kernel)[i], 0, size);
    }
}

void ImageManipulator::setKernelValue(char *** kernel, int val)
{
  for (int i = 0 ; i < _kernelSize; ++i)
    {
      (*kernel)[i] = (char*)memset((*kernel)[i], val, _kernelSize);
    }
}

void ImageManipulator::initEdgeKernel1()
{
  setKernelValue(&edgeKernel1, 0);
  edgeKernel1[0][_middle] = 1;
  edgeKernel1[_middle][0] = 1;
  edgeKernel1[_middle][_kernelSize - 1] = 1;
  edgeKernel1[_kernelSize - 1][_middle] = 1;
  edgeKernel1[_middle][_middle] = -4;
}

void ImageManipulator::initEdgeKernel2()
{
  setKernelValue(&edgeKernel2, -2);
  edgeKernel2[_middle][_middle] = 4;
  edgeKernel2[0][0] = 1;
  edgeKernel2[0][_kernelSize - 1] = 1;
  edgeKernel2[_kernelSize - 1][0] = 1;
  edgeKernel2[_kernelSize - 1][_kernelSize - 1] = 1;
}

void ImageManipulator::initEdgeKernel3()
{
  setKernelValue(&edgeKernel3, -1);
  edgeKernel3[_middle][_middle] = 8;
}

void ImageManipulator::initSharpenKernel()
{
  setKernelValue(&sharpenKernel, -1);
  sharpenKernel[_middle][_middle] = 9;
}

void ImageManipulator::initBlurKernel()
{
  setKernelValue(&blurKernel, 1);
}

void ImageManipulator::parseImage(char ** kernel, 
				  int _size, 
				  cv::Mat & img,
				  cv::Mat & to_img,
				  int divisor)
{
  if (to_img.data != NULL)
    {
      if (img.channels() == 3)
	{
	  int color;
	  for (int y = 1 ; y < img.rows - 1; ++y)
	    {
	      for (int x = 1 ; x < img.cols - 1 ; ++x)
		{
		  color = applyKernel3Chan(kernel, x, y, _size, img, divisor);
		  setPixelColor(x, y, to_img, color);
		}
	    }
	}
      else if (img.channels() == 1)
	{
	  for (int y = 1 ; y < img.rows - 1; ++y)
	    {
	      for (int x = 1 ; x < img.cols - 1 ; ++x)
		{
		  setGrayPixel(x, y, to_img, applyKernel1Chan(kernel, x, y, _size, img, divisor));
		}
	    }
	}
    }
}

int ImageManipulator::applyKernel3Chan(char **kernel,
				   int x, int y,
				   int _size,
				   cv::Mat & img,
				   int divisor)
{
  if (_size % 2 == 0)
    return -1;
  int avg_R = 0, avg_G = 0, avg_B = 0;
  int curr_x = 0, curr_y = 0;
  int middle = floor(_size / 2);
  for (int i = 0 ; i < _size ; ++i)
    {
      for (int j = 0 ; j < _size ; ++j)
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
	      curr_x = x - (j - _middle);
	    }
	  else if (j == middle)
	    {
	      curr_x = x;
	    }
	  int color = getPixelColor(curr_x, curr_y, img);
	  avg_R += getRed(color) * kernel[i][j];
	  avg_G += getGreen(color) * kernel[i][j];
	  avg_B += getBlue(color) * kernel[i][j];
	}
    }
  if (divisor > 1)
    {
      avg_R /= divisor;
      avg_G /= divisor;
      avg_B /= divisor;
    }
  if (avg_R < 0)
    avg_R = 0;
  if (avg_R > 255)
    avg_R = 255;
  if (avg_G < 0)
    avg_G = 0;
  if (avg_G > 255)
    avg_G = 255;
  if (avg_B < 0)
    avg_B = 0;
  if (avg_B > 255)
    avg_B = 255;
  return createPixelColor(avg_R, avg_G, avg_B);
}

int ImageManipulator::applyKernel1Chan(char **kernel,
				   int x, int y,
				   int _size,
				   cv::Mat & img,
				   int divisor)
{
  if (_size % 2 == 0)
    return -1;
  int avg = 0;
  int curr_x = 0, curr_y = 0;
  int middle = floor(_size / 2);
  for (int i = 0 ; i < _size ; ++i)
    {
      for (int j = 0 ; j < _size ; ++j)
	{
	  if (i != middle)
	    {
	      curr_y = y - (i - _middle);
	    }
	  else if (i == middle)
	    {
	      curr_y = y;
	    }
	  if (j != middle)
	    {
	      curr_x = x - (j - _middle);
	    }
	  else if (j == middle)
	    {
	      curr_x = x;
	    }
	  avg += getGrayPixel(curr_x, curr_y, img) * kernel[i][j];
	}
    }
  avg /= divisor;
  if (avg < 0)
    avg = 0;
  return avg;
}

int ImageManipulator::createPixelColor(int R, int G, int B)
{
  R <<= 16;
  G <<= 8;
  int color = (R | G | B);
  return color;
}

void ImageManipulator::setPixelColor(int x,
				     int y,
				     cv::Mat & img,
				     int color)
{
  img.data[(y * img.channels()) * img.cols + (x * img.channels())] = (color >> 16) & 0xFF;
  img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 1] = (color >> 8) & 0xFF;
  img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 2] = (color) & 0xFF;
}

int ImageManipulator::getPixelColor(int x, int y, cv::Mat & img)
{
  int r = img.data[(y * img.channels()) * img.cols + (x * img.channels())];
  int g = img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 1];
  int b = img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 2];
  return createPixelColor(r, g, b);
}

int ImageManipulator::getRed(int color)
{
  return (color >> 16) & 0xFF;
}

int ImageManipulator::getGreen(int color)
{
  return (color >> 8) & 0xFF; 
}

int ImageManipulator::getBlue(int color)
{
  return (color & 0xFF);
}

int ImageManipulator::getGrayPixel(int x, int y, cv::Mat & img)
{
  return img.data[y * img.cols + x];
}

void ImageManipulator::setGrayPixel(int x, int y, cv::Mat & img, int value)
{
  img.data[y * img.cols + x] = value;
}

void ImageManipulator::custom(cv::Mat & from, cv::Mat & to, char **kernel, int size, int divisor)
{
  parseImage(kernel, size, from, to, divisor);
}

void ImageManipulator::blur(cv::Mat &from, cv::Mat &to)
{
  parseImage(blurKernel, _kernelSize, from, to, 9);
}

void ImageManipulator::edgeDetect1(cv::Mat &from, cv::Mat &to)
{
  parseImage(edgeKernel1, _kernelSize, from, to, 1);
}

void ImageManipulator::edgeDetect2(cv::Mat &from, cv::Mat &to)
{
  parseImage(edgeKernel2, _kernelSize, from, to, 1);
}

void ImageManipulator::edgeDetect3(cv::Mat &from, cv::Mat &to)
{
  parseImage(edgeKernel3, _kernelSize, from, to, 1);
}

void ImageManipulator::sharpen(cv::Mat &from, cv::Mat &to)
{
  parseImage(sharpenKernel, _kernelSize, from, to, 1);
}

void ImageManipulator::crop(cv::Mat & from, cv::Mat & to, 
			    int x, int y, int sizex, int sizey)
{
  if ((x < from.cols && y < from.rows)
      && (x + sizex < from.cols && y + sizey < from.rows))
    {
      int _crop_x = 0, _crop_y = 0;
      if (from.channels() == 3)
	{
	  for (int _y = y; _y < y + sizey ; ++_y && ++_crop_y)
	    {
	      _crop_x = 0;
	      for (int _x = x ; _x < x + sizex ; ++_x && ++_crop_x)
		{
		  setPixelColor(_crop_x, _crop_y, to, getPixelColor(_x, _y, from));
		}
	    }
	}
      else if (from.channels() == 1)
	{
	  for (int _y = y; _y < y + sizey ; ++_y && ++_crop_y)
	    {
	      _crop_x = 0;
	      for (int _x = x ; _x < x + sizex ; ++_x && ++_crop_x)
		{
		  setGrayPixel(_crop_x,
			       _crop_y,
			       to,
			       getGrayPixel(_x, _y, from));
		}
	    }	  
	}
    }
}

void ImageManipulator::scale(cv::Mat &from, cv::Mat &to, SCALE_TYPE type)
{
  (void)from;
  (void)to;
  (void)type;
  if (type == LMC_NEAREST_NEIGHBORE)
    {}
  else if (type == LMC_AVG_SURROUNDINGS)
    {}
}

void ImageManipulator::releaseKernel(char *** kernel)
{
  (void)kernel;
  for (int i = 0 ; i < _kernelSize + 1 ; ++i)
    {
      free((*kernel)[i]);
    }
  free(*kernel);
}
