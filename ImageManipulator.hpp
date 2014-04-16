#ifndef IMAGE_MANIPULATOR
#define IMAGE_MANIPULATOR

#include <opencv2/opencv.hpp>
#include <cmath>
#include <map>
#include <cstdio>
#include <exception>
#include <thread>

class ImageManipulator;

typedef int (ImageManipulator::*LMC_KERNEL_APPLY_METHODS)(char**,
							  int,
							  int,
							  int,
							  cv::Mat&,
							  cv::Mat&,
							  int);
typedef void (ImageManipulator::*LMC_FUNCTION_POINTER)(cv::Mat&, cv::Mat&);

class ImageManipulator
{
public :
  enum SCALE_TYPE
    {
      LMC_NEAREST_NEIGHBORE = 0,
      LMC_AVG_SURROUNDINGS = 1
    };

  struct threadData
  {
    LMC_KERNEL_APPLY_METHODS method;
    char **kernel;
    cv::Mat * img_from;
    cv::Mat * img_to;
    int divisor;
    int kernelSize;
    int x, y;
    int lim_x, lim_y;
    std::mutex * mutex;
    int _id;
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

  void split(cv::Mat &from,
	     cv::Mat &toR,
	     cv::Mat&toG,
	     cv::Mat&toB);

  void combine(cv::Mat &from,
	     cv::Mat &toR,
	     cv::Mat&toG,
	     cv::Mat&toB);

  void scale(cv::Mat &from,
	     cv::Mat &to,
	     SCALE_TYPE type = (SCALE_TYPE)0);

  void * averageImage(void*);

private :

  struct _threadData
  {
    char ** kernel;
    cv::Mat * from_img;
    cv::Mat * to_img;
    int size;
    int divisor;
  };

  void * applyKernel(void *);

  char **_edgeKernel1;
  char **_edgeKernel2;
  char **_edgeKernel3;
  char **_sharpenKernel;
  char **_blurKernel;

  int _kernelSize;
  int _middle;

  std::map<int, LMC_FUNCTION_POINTER> LMC_FUNCTIONS;
  std::map<int, LMC_KERNEL_APPLY_METHODS> LMC_KERNEL_METHODS;

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
		       cv::Mat & to_img,
		       int divisor = 1);

  int applyKernel1Chan(char ** kernel,
		       int x,
		       int y,
		       int size, 
		       cv::Mat & img,
		       cv::Mat & to_img,
		       int divisor = 1);
  
  void LMC_nearestNeighbore(cv::Mat &from,
			    cv::Mat &to);

  int averageSurrounds(int x,
		       int y,
		       cv::Mat&img);

  void LMC_avgSurroundings(cv::Mat &from,
			   cv::Mat &to);
};

#endif
