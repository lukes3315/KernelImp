#ifndef IMAGE_MANIPULATOR
#define IMAGE_MANIPULATOR

#include <opencv2/opencv.hpp>
#include <cmath>

class ImageManipulator
{
public :
  enum SCALE_TYPE
    {
      LMC_NEAREST_NEIGHBORE = 0,
      LMC_AVG_SURROUNDINGS = 1
    };

  ImageManipulator();
  ImageManipulator(int size);
  ~ImageManipulator();

  void custom(cv::Mat&from,
	      cv::Mat&to,
	      char**kernel,
	      int size,
	      int divisor);

  void blur(cv::Mat&from,
	    cv::Mat&to);
  
  void edgeDetect1(cv::Mat&from,
		   cv::Mat&to);

  void edgeDetect2(cv::Mat&from,
		   cv::Mat&to);

  void edgeDetect3(cv::Mat&from,
		   cv::Mat&to);

  void sharpen(cv::Mat&from,
	       cv::Mat&to);

  void crop(cv::Mat &from, 
	    cv::Mat &to,
	    int x,
	    int y,
	    int sizex,
	    int sizey);

  void scale(cv::Mat &from,
	     cv::Mat &to,
	     SCALE_TYPE type = (SCALE_TYPE)0);

private :

  char **edgeKernel1;
  char **edgeKernel2;
  char **edgeKernel3;
  char **sharpenKernel;
  char **blurKernel;


  void allocKernel(int size, char *** kernel);
  void initialize();
  void setKernelValue(char *** kernel, int val);
  void initEdgeKernel1();
  void initEdgeKernel2();
  void initEdgeKernel3();
  void initSharpenKernel();
  void initBlurKernel();

  void releaseKernel(char *** kernel);

  int createPixelColor(int R, int G, int B);
  void setPixelColor(int x, int y, cv::Mat & img, int color);
  int getPixelColor(int x, int y, cv::Mat &img);

  int getGrayPixel(int x, int y, cv::Mat &img);
  void setGrayPixel(int x, int y, cv::Mat & img, int value);

  int getRed(int color);
  int getGreen(int color);
  int getBlue(int color);

  int getGray();

  void parseImage(char ** kernel,
		       int _size, 
		       cv::Mat & img,
		       cv::Mat & img_to,
		       int divisor = 1);

  int applyKernel3Chan(char ** kernel,
			int x,
			int y,
			int size, 
			cv::Mat & img,
			int divisor = 1);

  int applyKernel1Chan(char ** kernel,
			int x,
			int y,
			int size, 
			cv::Mat & img,
			int divisor = 1);


  int _kernelSize;
  int _middle;
};

#endif
