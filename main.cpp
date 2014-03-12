#include <opencv2/opencv.hpp>
#include <iostream>

#define KERNEL_SIZE 3
#define IDENTITY 0
#define SHARPENING 1
#define EDGE_DETECTION_1 2
#define EDGE_DETECTION_2 3
#define EDGE_DETECTION_3 4


#define KERNEL_WANTED EDGE_DETECTION_2

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

int getAverage(int x, int y, cv::Mat & img, char kernel[][3])
{
  int avg_R = 0;
  int avg_G = 0;
  int avg_B = 0;
  int avg = 0;
  int curr_y = 0;
  int curr_x = 0;
  int color = 0;
  
  for (int i = 0 ; i < 3 ; ++i)
    {
      for (int j = 0 ; j < 3 ; ++j)
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

int main(void)
{
  cv::VideoCapture cap(0);
  int key = -1;
  cv::Mat img;
  cv::Mat img2;

  if (!cap.isOpened())
    return -1;

  char kernel[3][3];
  memset(kernel[0], 0, 3);
  memset(kernel[1], 0, 3);
  memset(kernel[2], 0, 3);

#if KERNEL_WANTED == SHARPENING
  kernel[0][0] = 0;
  kernel[0][1] = -1;
  kernel[0][2] = 0;
  kernel[1][0] = -1;
  kernel[1][1] = 5;
  kernel[1][2] = -1;
  kernel[2][0] = 0;
  kernel[2][1] = -1;
  kernel[2][2] = 0;
  std::cout << "Sharpen kernel" << std::endl;
#elif KERNEL_WANTED == IDENTITY
  kernel[1][1] = 1;
  std::cout << "Original kernel" << std::endl;
#elif KERNEL_WANTED == EDGE_DETECTION_1
  kernel[0][1] = 1;
  kernel[1][0] = 1;
  kernel[1][1] = -4;
  kernel[1][2] = 1;
  kernel[2][1] = 1;
  std::cout << "Edge detection #1 kernel" << std::endl;
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
#elif KERNEL_WANTED == EDGE_DETECTION_3
  memset(kernel[0], -1, KERNEL_SIZE);
  memset(kernel[1], -1, KERNEL_SIZE);
  memset(kernel[2], -1, KERNEL_SIZE);
  kernel[1][1] = 8;
  std::cout << "Edge detection #3 kernel" << std::endl;
#endif


  unsigned char * data = NULL;


  cv::Vec3f colors;
  while (key < 0)
    {
      cap >> img;
      if (img.empty())
	break;
      if (data == NULL)
	{
	  data = new unsigned char[(img.cols * img.rows) * img.channels()];
	  img2.create(img.rows, img.cols, img.type());
	}
      for (int y = 1 ; y < (img.rows) - 1; ++y)
	{
	  for (int x = 1 ; x < (img.cols) - 1 ; ++x)
	    {
	      
	      
	      setPixelColor(x, y, img2, getAverage(x, y, img, kernel));
	      //setPixel(x, y, img, 0xFFFFFF);

	      //char pixels[3][9];
	      /*
	      colors = img.at<cv::Vec3f>(y - 1, x - 1);
	      avg_R += kernel[0][0] * colors.val[0];
	      avg_G += kernel[0][0] * colors.val[1];
	      avg_B += kernel[0][0] * colors.val[2];

	      colors = img.at<cv::Vec3f>(y - 1, x);
	      avg_R += kernel[0][1] * colors.val[0];
	      avg_G += kernel[0][1] * colors.val[1];
	      avg_B += kernel[0][1] * colors.val[2];
	      
	      colors = img.at<cv::Vec3f>(y - 1, x + 1);
	      
	      avg_R += kernel[0][2] * colors.val[0];
	      avg_G += kernel[0][2] * colors.val[1];
	      avg_B += kernel[0][2] * colors.val[2];

	      colors = img.at<cv::Vec3f>(y, x - 1);
	      avg_R += kernel[1][0] * colors.val[0];
	      avg_G += kernel[1][0] * colors.val[1];
	      avg_B += kernel[1][0] * colors.val[2];

	      colors = img.at<cv::Vec3f>(y, x);
	      avg_R += kernel[1][1] * colors.val[0];
	      avg_G += kernel[1][1] * colors.val[1];
	      avg_B += kernel[1][1] * colors.val[2];
	      
	      colors = img.at<cv::Vec3f>(y, x + 1);
	      avg_R += kernel[1][2] * colors.val[0];
	      avg_G += kernel[1][2] * colors.val[1];
	      avg_B += kernel[1][2] * colors.val[2];

	      colors = img.at<cv::Vec3f>(y - 1, x - 1);
	      avg_R += kernel[2][0] * colors.val[0];
	      avg_G += kernel[2][0] * colors.val[1];
	      avg_B += kernel[2][0] * colors.val[2];

	      colors = img.at<cv::Vec3f>(y + 1, x);
	      avg_R += kernel[2][1] * colors.val[0];
	      avg_G += kernel[2][1] * colors.val[1];
	      avg_B += kernel[2][1] * colors.val[2];

	      colors = img.at<cv::Vec3f>(y + 1, x + 1);
	      avg_R += kernel[2][2] * colors.val[0];
	      avg_G += kernel[2][2] * colors.val[1];
	      avg_B += kernel[2][2] * colors.val[2];

	      avg_R /= 9;
	      avg_G /= 9;
	      avg_B /= 9;

	      cv::Vec3f t;
	      t.val[0] = avg_R;
	      t.val[1] = avg_G;
	      t.val[2] = avg_B;
	      */
	      //img2.data[(y*img2.channels()) * img2.cols + (x * img2.channels())] = 255;
	      //img2.data[(y*img2.channels()) * img2.cols + (x * img2.channels()) + 1] = 255;
	      //img2.data[(y*img2.channels()) * img2.cols + (x * img2.channels()) + 2] = 255;
	      /*
	      cv::Vec3f t;
	      t.val[0] = 255;
	      t.val[1] = 255;
	      t.val[2] = 255;
	      //t.val[3] = 255;
	      img2.at<cv::Vec3f>(y, x) = t;*/




	      /*
	      //Top row pixels      
	      pixels[0][0] = img.data[(y - 3) * img.cols + (x - 3)];
	      pixels[0][1] = img.data[(y - 3) * img.cols + (x - 2)];
	      pixels[0][2] = img.data[(y - 3) * img.cols + (x - 1)];

	      pixels[0][3] = img.data[(y - 3) * img.cols + (x)];
	      pixels[0][4] = img.data[(y - 3) * img.cols + (x + 1)];
	      pixels[0][5] = img.data[(y - 3) * img.cols + (x + 2)];

	      pixels[0][6] = img.data[(y - 3) * img.cols + (x + 3)];
	      pixels[0][7] = img.data[(y - 3) * img.cols + (x + 4)];
	      pixels[0][8] = img.data[(y - 3) * img.cols + (x + 5)];

	      //middle row pixels
	      pixels[1][0] = img.data[(y) * img.cols + (x - 3)];
	      pixels[1][1] = img.data[(y) * img.cols + (x - 2)];
	      pixels[1][2] = img.data[(y) * img.cols + (x - 1)];

	      pixels[1][3] = img.data[(y) * img.cols + (x)];
	      pixels[1][4] = img.data[(y) * img.cols + (x + 1)];
	      pixels[1][5] = img.data[(y) * img.cols + (x + 2)];

	      pixels[1][6] = img.data[(y) * img.cols + (x + 3)];
	      pixels[1][7] = img.data[(y) * img.cols + (x + 4)];
	      pixels[1][8] = img.data[(y) * img.cols + (x + 5)];

	      //Bottom row pixels
	      pixels[2][0] = img.data[(y + 3) * img.cols + (x - 3)];
	      pixels[2][1] = img.data[(y + 3) * img.cols + (x - 2)];
	      pixels[2][2] = img.data[(y + 3) * img.cols + (x - 1)];

	      pixels[2][3] = img.data[(y + 3) * img.cols + (x)];
	      pixels[2][4] = img.data[(y + 3) * img.cols + (x + 1)];
	      pixels[2][5] = img.data[(y + 3) * img.cols + (x + 2)];

	      pixels[2][6] = img.data[(y + 3) * img.cols + (x + 3)];
	      pixels[2][7] = img.data[(y + 3) * img.cols + (x + 4)];
	      pixels[2][8] = img.data[(y + 3) * img.cols + (x + 5)];

	      int avg_R = 0, avg_G = 0, avg_B = 0;

	      avg_R = (pixels[0][0] * kernel[0][0]) + (pixels[0][3] * kernel[0][1]) + (pixels[0][6] * kernel[0][2]);
	      avg_G = (pixels[0][1] * kernel[0][0]) + (pixels[0][4] * kernel[0][1]) + (pixels[0][7] * kernel[0][2]);
	      avg_B = (pixels[0][2] * kernel[0][0]) + (pixels[0][5] * kernel[0][1]) + (pixels[0][8] * kernel[0][2]);

	      avg_R += (pixels[1][0] * kernel[1][0]) + (pixels[1][3] * kernel[1][1]) + (pixels[1][6] * kernel[1][2]);
	      avg_G += (pixels[1][1] * kernel[1][0]) + (pixels[1][4] * kernel[1][1]) + (pixels[1][7] * kernel[1][2]);
	      avg_B += (pixels[1][2] * kernel[1][0]) + (pixels[1][5] * kernel[1][1]) + (pixels[1][8] * kernel[1][2]);

	      avg_R += (pixels[2][0] * kernel[2][0]) + (pixels[2][3] * kernel[2][1]) + (pixels[2][6] * kernel[2][2]);
	      avg_G += (pixels[2][1] * kernel[2][0]) + (pixels[2][4] * kernel[2][1]) + (pixels[2][7] * kernel[2][2]);
	      avg_B += (pixels[2][2] * kernel[2][0]) + (pixels[2][5] * kernel[2][1]) + (pixels[2][8] * kernel[2][2]);

	      avg_R /= 9;
	      avg_G /= 9;
	      avg_B /= 9;
	      */
	    }
	}
      cv::imshow("VideoBase", img);
      cv::imshow("Video", img2);
      key = cv::waitKey(1);
    }

  return 0;
}
