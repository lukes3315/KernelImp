#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

#define KERNEL_SIZE 3
#define IDENTITY 0
#define SHARPENING 1
#define EDGE_DETECTION_1 2
#define EDGE_DETECTION_2 3
#define EDGE_DETECTION_3 4
#define BLUR 5

#define KERNEL_WANTED EDGE_DETECTION_1

#define R_2_GRAY 0.2126
#define G_2_GRAY 0.7152
#define B_2_GRAY 0.0722

int divisor = 1;

int getGrayPixel(int R, int G, int B);
void convertRGBToGray(cv::Mat & from, cv::Mat & to);
int createPixelColor(int R, int G, int B);
void setPixelColor(int x, int y, cv::Mat & img, int color);
int getPixelColor(int x, int y, cv::Mat & img);
int getRed(int color);
int getGreen(int color);
int getBlue(int color);

int getGrayPixel(int R, int G, int B)
{
  int gray = R*R_2_GRAY + G*G_2_GRAY + B*B_2_GRAY;
  return gray;
}

void convertRGBToGray(cv::Mat & from, cv::Mat & to)
{
  if (to.data == NULL)
    to.create(from.rows, from.cols, 1);

  for (int y = 0; y < from.rows ; ++y)
    {
      for (int x = 0 ; x < from.cols ; ++x)
	{
	  int color = getPixelColor(x, y, from);
	  to.data[y * from.cols + x] = 
	    getGrayPixel(getRed(color), getGreen(color), getBlue(color));
	}
    }
}

void split(cv::Mat & from, cv::Mat & toR, cv::Mat & toG, cv::Mat & toB)
{
  if (toR.data == NULL)
    toR.create(from.rows, from.cols, 1);

  if (toG.data == NULL)
    toG.create(from.rows, from.cols, 1);

  if (toB.data == NULL)
    toB.create(from.rows, from.cols, 1);

  for (int y = 0 ; y < from.rows ; ++y)
    {
      for (int x = 0 ; x < from.cols ; ++x)
	{
	  int color = getPixelColor(x, y, from);
	  toR.data[y * from.cols + x] = getRed(color);
	  toG.data[y * from.cols + x] = getGreen(color);
	  toB.data[y * from.cols + x] = getBlue(color);
	}
    }
}

int createPixelColor(int R, int G, int B)
{
  R <<= 16;
  G <<= 8;
  int color = R | G | B;
  return color;
}

void setPixelColor(int x, int y, cv::Mat & img, int color)
{
  img.data[(y * img.channels()) * img.cols + (x * img.channels())] = (color >> 16) & 0xFF;
  img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 1] = (color >> 8) & 0xFF;
  img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 2] = (color) & 0xFF;
}

int getPixelColor(int x, int y, cv::Mat & img)
{
  int r = img.data[(y * img.channels()) * img.cols + (x * img.channels())];
  int g = img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 1];
  int b = img.data[(y * img.channels()) * img.cols + (x * img.channels()) + 2];
  return createPixelColor(r, g, b);
}

int getRed(int color)
{
  return (color >> 16) & 0xFF;
}

int getGreen(int color)
{
  return (color >> 8) & 0xFF;
}

int getBlue(int color)
{
  return (color & 0xFF);
}

int get_X(int x, int curr_index)
{
  if (curr_index == 0)
    {
      return x - 1;
    }
  if (curr_index == 1)
    {
      return x;
    }
  if (curr_index == 2)
    {
      return x + 1;
    }
  return 0;
}

void copy(cv::Mat & from, cv::Mat & to)
{
  for (int i = 0 ; i < (to.cols * to.rows) * to.channels() ; ++i)
    {
      to.data[i] = from.data[i];
    }
}

int getColorAverage(int x, int y, cv::Mat & from)
{
  int avg_R = 0, avg_G = 0, avg_B = 0;

  avg_R += getRed(getPixelColor(x - 1, y - 1, from));
  avg_G += getGreen(getPixelColor(x - 1, y - 1, from));
  avg_B += getBlue(getPixelColor(x - 1, y - 1, from));
  
  avg_R += getRed(getPixelColor(x, y - 1, from));
  avg_G += getGreen(getPixelColor(x, y - 1, from));
  avg_B += getBlue(getPixelColor(x, y - 1, from));

  avg_R += getRed(getPixelColor(x + 1, y - 1, from));
  avg_G += getGreen(getPixelColor(x + 1, y - 1, from));
  avg_B += getBlue(getPixelColor(x + 1, y - 1, from));

  avg_R += getRed(getPixelColor(x - 1, y, from));
  avg_G += getGreen(getPixelColor(x - 1, y, from));
  avg_B += getBlue(getPixelColor(x - 1, y, from));

  avg_R += getRed(getPixelColor(x, y, from));
  avg_G += getGreen(getPixelColor(x, y, from));
  avg_B += getBlue(getPixelColor(x, y, from));

  avg_R += getRed(getPixelColor(x + 1, y, from));
  avg_G += getGreen(getPixelColor(x + 1, y, from));
  avg_B += getBlue(getPixelColor(x + 1, y, from));

  avg_R += getRed(getPixelColor(x - 1, y + 1, from));
  avg_G += getGreen(getPixelColor(x - 1, y + 1, from));
  avg_B += getBlue(getPixelColor(x - 1, y + 1, from));

  avg_R += getRed(getPixelColor(x, y + 1, from));
  avg_G += getGreen(getPixelColor(x, y + 1, from));
  avg_B += getBlue(getPixelColor(x, y + 1, from));

  avg_R += getRed(getPixelColor(x + 1, y + 1, from));
  avg_G += getGreen(getPixelColor(x + 1, y + 1, from));
  avg_B += getBlue(getPixelColor(x + 1, y + 1, from));
  
  avg_R /= 9;
  avg_G /= 9;
  avg_B /= 9;

  return createPixelColor(avg_R, avg_G, avg_B);
}

void scale(cv::Mat & from, cv::Mat & to, float _scale)
{
  int _n_rows = from.rows * _scale;
  int _n_cols = from.cols * _scale;

  if (to.data == NULL)
    {
      to.create(_n_rows, _n_cols, from.type());
    }
  else if (to.rows != _n_rows
	   || to.cols != _n_cols)
    {
      to.release();
      to.create(_n_rows, _n_cols, from.type());
    }
  if (_scale > 1.f || _scale < 1.f)
    {
      for (int y = 0 ; y < to.rows ; ++y)
	{
	  for (int x = 0 ; x < to.cols ; ++x)
	    {	
	      if (x > 0 && y > 0 && x < to.cols - 1 && y < to.rows - 1)
		{
		  setPixelColor(x, y, to, getColorAverage((int)x/_scale, (int)y/_scale, from));
		}
	      else
		{
		  setPixelColor(x, y, to,
				getPixelColor((int)x/_scale, (int)y/_scale, from));
		}
	    }
	}
    }
  else if (_scale == 1.f)
    {
      copy(from, to);
    }
  std::cout << _scale<<std::endl;
}

int getY(int64_t coords)
{
  return coords & 0xFFFFFFFF;
}

int getX(int64_t coords)
{
  return (coords >> 32);
}

int64_t getCoords(int x, int y)
{
  int64_t coords = 0;

  coords = x;//coords = x
  coords <<= 32;//push 32 '0' into 64bit integer so x value located at the begining
  coords |= y;//apply OR mask to add y value to 64bit long integer
  return coords;
}

void crop(cv::Mat & from, cv::Mat & to, int64_t start_coords, int64_t end_coords)
{
  int x = getX(start_coords);
  int y = getY(start_coords);
  int _end_x = getX(end_coords);
  int _end_y = getY(end_coords);

  std::cout << "x " << x << std::endl;
  std::cout << "y " << y << std::endl;
  std::cout << "endx " << _end_x << std::endl;
  std::cout << "endy " << _end_y << std::endl;
  int cols = _end_x - x;
  int rows = _end_y - y;

  if (to.data == NULL)
    to.create(rows, cols, from.type());
  if (to.cols != cols
      || to.rows != rows)
    {
      to.release();
      to.create(rows, cols, from.type());
    }
  else
    {
      int _crop_x = 0, _crop_y = 0;
      for (int _y = y; _y < _end_y ; ++_y && ++_crop_y)
	{
	  _crop_x = 0;
	  for (int _x = x ; _x < _end_x ; ++_x && ++_crop_x)
	    {
	      setPixelColor(_crop_x, _crop_y, to, getPixelColor(_x, _y, from));
	    }
	}
    }
}

int applyKernel(int x, int y, cv::Mat & img, char kernel[][3])
{
  int avg_R = 0;
  int avg_G = 0;
  int avg_B = 0;
  int avg = 0;
  int curr_y = 0;
  int curr_x = 0;
  int color = 0;

  for (int i = 0 ; i < KERNEL_SIZE ; ++i)
    {
      for (int j = 0 ; j < KERNEL_SIZE ; ++j)
	{
	  if (i == 0)
	    {
	      curr_y = y - 1;
	      curr_x = get_X(x, j);
	      color = getPixelColor(curr_x, curr_y, img);
	    }
	  else if (i == 1)
	    {
	      curr_y = y;
	      curr_x = get_X(x, j);
	      color = getPixelColor(curr_x, curr_y, img);
	    }
	  else if (i == 2)
	    {
	      curr_y = y + 1;
	      curr_x = get_X(x, j);
	      color = getPixelColor(curr_x, curr_y, img);
	    }
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
  else if (avg_R > 255)
    avg_R = 255;
  if (avg_G < 0)
    avg_G = 0;
  else if (avg_G > 255)
    avg_G = 255;
  if (avg_B < 0)
    avg_B = 0;
  else if (avg_B > 255)
    avg_B = 255;
  avg = createPixelColor(avg_R, avg_G, avg_B);
  return avg;
}

int64_t checkNeighBorhood(int x, int y, cv::Mat & img)
{
  for (int _y = y - 1 ; _y < y + 1 ; ++_y)
    {
      for (int _x = x - 1; _x < x + 1 ; ++_x)
	{
	  if (_x != x && _y == y)
	    {
	      if (getPixelColor(_x, _y, img) > 0)
		{
		  return getCoords(_x, _y);
		}
	    }
	}
    }
  return 0;
}

int64_t mooresNeighbor(int64_t start_coord, cv::Mat & img)
{
  int x = getX(start_coord);
  int y = getY(start_coord);

  if (x > 0 && y > 0
      && x < img.cols - 1 && y < img.rows - 1)
    {
      int coords = 1;

      while (coords > 0)
	{
	  coords = checkNeighBorhood(x, y, img);
	  if (coords > 0)
	    {
	      int _x = getX(coords);
	      int _y = getY(coords);
	      setPixelColor(_x, _y, img, createPixelColor(255, 0, 255));
	      x = _x;
	      y = _y;
	    }
	}
    }
  return 0;
}

int main(void)
{
  cv::VideoCapture cap("video.mp4");
  int key = -1;
  cv::Mat img;
  cv::Mat img2;
  cv::Mat gray;
  cv::Mat scaling;
  cv::Mat scaling_sharp;
  cv::Mat scaling_sharpened;
  cv::Mat cropped;
  cv::Mat edges;
  cv::Mat eT, eT2;

  if (!cap.isOpened())
    return -1;
  char kernel[3][3];
  memset(kernel[0], 0, 3);
  memset(kernel[1], 0, 3);
  memset(kernel[2], 0, 3);
#if KERNEL_WANTED == SHARPENING
  kernel[0][0] = -1;
  kernel[0][1] = -1;
  kernel[0][2] = -1;
  kernel[1][0] = -1;
  kernel[1][1] = 9;
  kernel[1][2] = -1;
  kernel[2][0] = -1;
  kernel[2][1] = -1;
  kernel[2][2] = -1;
  std::cout << "Sharpen kernel" << std::endl;
  divisor = 1;
#elif KERNEL_WANTED == IDENTITY
  kernel[1][1] = 1;
  std::cout << "Original kernel" << std::endl;
  divisor = 1;
#elif KERNEL_WANTED == EDGE_DETECTION_1
  kernel[0][1] = 1;
  kernel[1][0] = 1;
  kernel[1][1] = -4;
  kernel[1][2] = 1;
  kernel[2][1] = 1;
  std::cout << "Edge detection #1 kernel" << std::endl;
  divisor = 1;
#elif KERNEL_WANTED == EDGE_DETECTION_2
  memset(kernel[0], -2, KERNEL_SIZE);
  memset(kernel[1], -2, KERNEL_SIZE);
  memset(kernel[2], -2, KERNEL_SIZE);
  kernel[0][0] = 1;
  kernel[0][2] = 1;
  kernel[2][0] = 1;
  kernel[2][2] = 1;
  kernel[1][1] = 4;
  std::cout << "Edge detection #2 kernel" << std::endl;
  divisor = 1;
#elif KERNEL_WANTED == EDGE_DETECTION_3
  memset(kernel[0], -1, KERNEL_SIZE);
  memset(kernel[1], -1, KERNEL_SIZE);
  memset(kernel[2], -1, KERNEL_SIZE);
  kernel[1][1] = 8;
  std::cout << "Edge detection #3 kernel" << std::endl;
  divisor = 1;
#elif KERNEL_WANTED == BLUR
  memset(kernel[0], 1, KERNEL_SIZE);
  memset(kernel[1], 1, KERNEL_SIZE);
  memset(kernel[2], 1, KERNEL_SIZE);
  std::cout << "Blur kernel" << std::endl;
  divisor = 9;
#endif
  getCoords(100, 20);

  while (key < 0)
    {
      cap >> img;
      if (img.empty())
	break;
      if (img2.data == NULL)
	{
	  img2.create(img.rows, img.cols, img.type());
	  edges.create(img.rows, img.cols, img.type());
	  eT.create(img.rows, img.cols, img.type());
	  eT2.create(img.rows, img.cols, img.type());
	  std::cout << "Edges threshed channels = " << eT.channels()<< std::endl;
	}
      memset(eT.data, 0, img.rows * img.cols * img.channels());
      memset(eT2.data, 0, img.rows * img.cols * img.channels());
      for (int y = 1 ; y < (img.rows) - 1; ++y)
	{
	  for (int x = 1 ; x < (img.cols) - 1 ; ++x)
	    {
	      int color = applyKernel(x, y, img, kernel);
	      int r = getRed(color);
	      int g = getGreen(color);
	      int b = getBlue(color);
	      if (r > 30 || b > 30 || g > 30)
		{
		  setPixelColor(x, y, eT, createPixelColor(255, 255, 255));
		}
	      setPixelColor(x, y, edges, color);
	    }
	}
      for (int y = 0 ; y < img.rows ; ++y)
	{
	  for (int x = 0 ; x < img.cols ; ++x)
	    {
	      int color = getPixelColor(x, y, eT);
	      
	      if (color > 0)
		{
		  mooresNeighbor(getCoords(x, y), eT);
		}
	    }
	}

      cv::imshow("GrayEdges", eT);
      cv::imshow("VideoBase", img);
      cv::imshow("Edges", edges);
      key = cv::waitKey(1);
    }
  img.release();
  img2.release();
  gray.release();
  cap.release();
  return 0;
}
