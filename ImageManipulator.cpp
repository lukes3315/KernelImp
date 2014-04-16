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
  releaseKernel(&_edgeKernel1);
  releaseKernel(&_edgeKernel2);
  releaseKernel(&_edgeKernel3);
  releaseKernel(&_sharpenKernel);
  releaseKernel(&_blurKernel);
}

void ImageManipulator::initialize()
{
  allocKernel(_kernelSize, &_edgeKernel1);
  allocKernel(_kernelSize, &_edgeKernel2);
  allocKernel(_kernelSize, &_edgeKernel3);
  allocKernel(_kernelSize, &_sharpenKernel);
  allocKernel(_kernelSize, &_blurKernel);

  _middle = floor(_kernelSize/2);
  initEdgeKernel1();
  initEdgeKernel2();
  initEdgeKernel3();
  initSharpenKernel();
  initBlurKernel();

  LMC_FUNCTIONS.insert(std::pair<int, LMC_FUNCTION_POINTER>(0, &ImageManipulator::LMC_nearestNeighbore));
  LMC_FUNCTIONS.insert(std::pair<int, LMC_FUNCTION_POINTER>(1, &ImageManipulator::LMC_avgSurroundings));

  LMC_KERNEL_METHODS.insert(std::pair<int, LMC_KERNEL_APPLY_METHODS>(1, &ImageManipulator::applyKernel1Chan));
  LMC_KERNEL_METHODS.insert(std::pair<int, LMC_KERNEL_APPLY_METHODS>(3, &ImageManipulator::applyKernel3Chan));
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
  setKernelValue(&_edgeKernel1, 0);
  _edgeKernel1[0][_middle] = 1;
  _edgeKernel1[_middle][0] = 1;
  _edgeKernel1[_middle][_kernelSize - 1] = 1;
  _edgeKernel1[_kernelSize - 1][_middle] = 1;
  _edgeKernel1[_middle][_middle] = -4;
}

void ImageManipulator::initEdgeKernel2()
{
  setKernelValue(&_edgeKernel2, -2);
  _edgeKernel2[_middle][_middle] = 4;
  _edgeKernel2[0][0] = 1;
  _edgeKernel2[0][_kernelSize - 1] = 1;
  _edgeKernel2[_kernelSize - 1][0] = 1;
  _edgeKernel2[_kernelSize - 1][_kernelSize - 1] = 1;
}

void ImageManipulator::initEdgeKernel3()
{
  setKernelValue(&_edgeKernel3, -1);
  _edgeKernel3[_middle][_middle] = 8;
}

void ImageManipulator::initSharpenKernel()
{
  setKernelValue(&_sharpenKernel, -1);
  _sharpenKernel[_middle][_middle] = 9;
}

void ImageManipulator::initBlurKernel()
{
  setKernelValue(&_blurKernel, 1);
}

void * ImageManipulator::applyKernel(void * data)
{
  threadData * _data = (threadData*)data;

  for (int y = _data->y; y < _data->lim_y; ++y)
    {
      for (int x = _data->x; x < _data->lim_x; ++x)
	{
	  LMC_KERNEL_APPLY_METHODS f = _data->method;

	  if (x > 0 
	      && y > 0
	      && x < _data->img_from->cols - 1
	      && y < _data->img_from->rows - 1)
	    {
	      if ((x < _data->x + 1 || y < _data->y + 1
		   || x > _data->lim_x  - 1|| y > _data->lim_y - 1))
		{
		  if (_data->mutex->try_lock())
		    {
		      ((this)->*f)(_data->kernel,
				   x,
				   y,
				   _data->kernelSize,
				   *_data->img_from,
				   *_data->img_to,
				   _data->divisor);
		      _data->mutex->unlock();
		    }
		  else
		    {
		    trylock:
		      if (_data->mutex->try_lock())
			{
			  ((this)->*f)(_data->kernel,
				       x,
				       y,
				       _data->kernelSize,
				       *_data->img_from,
				       *_data->img_to,
				       _data->divisor);
			  _data->mutex->unlock();
			}
		      else
			goto trylock;
		    }
	      
		}
	      else
		{
		  ((this)->*f)(_data->kernel,
			       x,
			       y,
			       _data->kernelSize,
			       *_data->img_from,
			       *_data->img_to,
			       _data->divisor);
		}
	    }
	}
    }
  return NULL;
}

void ImageManipulator::parseImage(char ** kernel, 
				  int _size, 
				  cv::Mat & img,
				  cv::Mat & to_img,
				  int divisor)
{
  if (to_img.data != NULL)
    {
      std::map<int, LMC_KERNEL_APPLY_METHODS>::iterator result = 
	LMC_KERNEL_METHODS.find(img.channels());
      
      #define NB_THREADS 4
      
      threadData data[NB_THREADS];
      int nb = NB_THREADS / 2;
      std::mutex _m;
      std::thread threads[NB_THREADS];
      
      if(result != LMC_KERNEL_METHODS.end())
	{
	  for (int i = 0 ; i < NB_THREADS ; ++i)
	    {
	      data[i].kernel = kernel;
	      data[i].kernelSize = _size;
	      data[i].method = result->second;
	      data[i].img_from = &img;
	      data[i].mutex = &_m;
	      data[i].img_to = &to_img;
	      data[i].divisor = divisor;
	      data[i]._id = i;
	      if (i < nb)
		{
		  data[i].x = i * (img.cols / nb);
		  data[i].y = 0;
		  data[i].lim_x = data[i].x + (img.cols / nb);
		  data[i].lim_y = data[i].y + (img.rows / nb);
		}
	      else
		{
		  data[i].x = (i - nb) * (img.cols / nb);
		  data[i].y = img.rows / nb;
		  data[i].lim_x = data[i].x + (img.cols / nb);
		  data[i].lim_y = img.rows;////data[i].y + (img.rows / nb);		  
		}
	      threads[i] = std::thread(&ImageManipulator::applyKernel,
				       this, &data[i]);
	    }
	  for (int i = 0; i < NB_THREADS; ++i)
	    {
	      if (threads[i].joinable())
		{
		  threads[i].join();
		}
	    }
	}
      
      //for (int i 
      
      /*if (!pthread_join(threads[0], NULL)
	  && !pthread_join(threads[1], NULL)
	  && !pthread_join(threads[2], NULL)
	  && !pthread_join(threads[3], NULL))
	  {}*/
	  
			//	    	      if (!pthread_join(p[0], (void**)&threaddataR)


      /*	  
      if (img.channels() == 3)
	{
	//std::thread
	  std::cout << "Wut ? " << std::endl;
	  struct _threadData * threaddataR = new struct _threadData();
	  struct _threadData * threaddataG = new struct _threadData();
	  struct _threadData * threaddataB = new struct _threadData();
	  pthread_t p[3];
	  std::cout << "Wut ? " << std::endl;

	  threaddataR->kernel = kernel;
	  threaddataR->size = _size;
	  threaddataR->from_img = &img;
	  threaddataR->to_img = new cv::Mat(img.rows, img.cols, CV_8U);
	  
	  std::cout << "Wut ?1 " << std::endl;

	  threaddataG->kernel = kernel;
	  threaddataG->size = _size;
	  threaddataG->from_img = &img;
	  threaddataG->to_img = new cv::Mat(img.rows, img.cols, CV_8U);
	  std::cout << "Wut ?2 " << std::endl;

	  threaddataB->kernel = kernel;
	  threaddataB->size = _size;
	  threaddataB->from_img = &img;
	  threaddataB->to_img = new cv::Mat(img.rows, img.cols, CV_8U);
	  std::cout << "Wut ?3 " << std::endl;

	  split(img, *(threaddataR->to_img), *(threaddataG->to_img), *(threaddataB->to_img));

	  if (!pthread_create(&p[0], NULL, applyKernel, &threaddataR)
	      && !pthread_create(&p[1], NULL, applyKernel, &threaddataG)
	      && !pthread_create(&p[2], NULL, applyKernel, &threaddataB))
	    {
	    	      if (!pthread_join(p[0], (void**)&threaddataR)
		  && !pthread_join(p[1], (void**)&threaddataG)
		  && !pthread_join(p[2], (void**)&threaddataB))
		{
		  combine(to_img,
			  *(threaddataR->to_img),
			  *(threaddataG->to_img),
			  *(threaddataB->to_img));
		  //delete 
		}
	  
	    }
	  int color;

	  for (int y = 1 ; y < img.rows - 1; ++y)
	    {
	      for (int x = 1 ; x < img.cols - 1 ; ++x)
		{
		  color = applyKernel3Chan(kernel,
					   x,
					   y, _size, img, divisor);
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
	}*/
    }
}

int ImageManipulator::applyKernel3Chan(char **kernel,
				   int x, int y,
				   int _size,
				   cv::Mat & img,
				   cv::Mat & to_img,
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
  int color =  createPixelColor(avg_R, avg_G, avg_B);
  setPixelColor(x, y, to_img, color);
  return color;
}

int ImageManipulator::applyKernel1Chan(char **kernel,
				   int x, int y,
				   int _size,
				   cv::Mat & img,
				   cv::Mat & to_img,
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
  avg /= divisor;
  if (avg < 0)
    avg = 0;
  setGrayPixel(x, y, to_img, avg);
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
  parseImage(_blurKernel, _kernelSize, from, to, 9);
}

void ImageManipulator::edgeDetect1(cv::Mat &from, cv::Mat &to)
{
  parseImage(_edgeKernel1, _kernelSize, from, to, 1);
}

void ImageManipulator::edgeDetect2(cv::Mat &from, cv::Mat &to)
{
  parseImage(_edgeKernel2, _kernelSize, from, to, 1);
}

void ImageManipulator::edgeDetect3(cv::Mat &from, cv::Mat &to)
{
  parseImage(_edgeKernel3, _kernelSize, from, to, 1);
}

void ImageManipulator::sharpen(cv::Mat &from, cv::Mat &to)
{
  parseImage(_sharpenKernel, _kernelSize, from, to, 1);
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



void ImageManipulator::split(cv::Mat & from,
			     cv::Mat & toR,
			     cv::Mat & toG,
			     cv::Mat & toB)
{
  if (toR.data == NULL)
    toR.create(from.rows, from.cols, CV_8U);

  if (toG.data == NULL)
    toG.create(from.rows, from.cols, CV_8U);

  if (toB.data == NULL)
    toB.create(from.rows, from.cols, CV_8U);

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

void ImageManipulator::combine(cv::Mat & from,
			       cv::Mat & toR,
			       cv::Mat & toG,
			       cv::Mat & toB)
{
  if (toR.data == NULL
      || toG.data == NULL
      || toB.data == NULL)
    {
      std::cout << "You need all three channels for proper image fusion" << std::endl;
      return;
    }
  if (from.data == NULL)
    {
      from.create(toR.rows, toR.cols, CV_32S);
    }
  for (int y = 0 ; y < from.rows ; ++y)
    {
      for (int x = 0 ; x < from.cols ; ++x)
	{
	  setPixelColor(x,
			y,
			from,
			createPixelColor(
					 getGrayPixel(x, y, toR),
					 getGrayPixel(x, y, toG),
					 getGrayPixel(x, y, toB)));
	}
    }
}

void ImageManipulator::LMC_nearestNeighbore(cv::Mat &from, cv::Mat &to)
{
  (void)from;
  (void)to;
  if (to.data != NULL)
    {
    }
}

int ImageManipulator::averageSurrounds(int x, int y, cv::Mat &img)
{
  if (img.channels() == 3)
    {
      int avg_R = 0, avg_G = 0, avg_B = 0;

      avg_R += getRed(getPixelColor(x - 1, y - 1, img));
      avg_G += getGreen(getPixelColor(x - 1, y - 1, img));
      avg_B += getBlue(getPixelColor(x - 1, y - 1, img));
  
      avg_R += getRed(getPixelColor(x, y - 1, img));
      avg_G += getGreen(getPixelColor(x, y - 1, img));
      avg_B += getBlue(getPixelColor(x, y - 1, img));

      avg_R += getRed(getPixelColor(x + 1, y - 1, img));
      avg_G += getGreen(getPixelColor(x + 1, y - 1, img));
      avg_B += getBlue(getPixelColor(x + 1, y - 1, img));

      avg_R += getRed(getPixelColor(x - 1, y, img));
      avg_G += getGreen(getPixelColor(x - 1, y, img));
      avg_B += getBlue(getPixelColor(x - 1, y, img));

      avg_R += getRed(getPixelColor(x, y, img));
      avg_G += getGreen(getPixelColor(x, y, img));
      avg_B += getBlue(getPixelColor(x, y, img));

      avg_R += getRed(getPixelColor(x + 1, y, img));
      avg_G += getGreen(getPixelColor(x + 1, y, img));
      avg_B += getBlue(getPixelColor(x + 1, y, img));

      avg_R += getRed(getPixelColor(x - 1, y + 1, img));
      avg_G += getGreen(getPixelColor(x - 1, y + 1, img));
      avg_B += getBlue(getPixelColor(x - 1, y + 1, img));

      avg_R += getRed(getPixelColor(x, y + 1, img));
      avg_G += getGreen(getPixelColor(x, y + 1, img));
      avg_B += getBlue(getPixelColor(x, y + 1, img));

      avg_R += getRed(getPixelColor(x + 1, y + 1, img));
      avg_G += getGreen(getPixelColor(x + 1, y + 1, img));
      avg_B += getBlue(getPixelColor(x + 1, y + 1, img));
  
      avg_R /= 9;
      avg_G /= 9;
      avg_B /= 9;
      return createPixelColor(avg_R, avg_G, avg_B);
    }
  else if (img.channels() == 1)
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
  return 0;
}

/*void * ImageManipulator::averageImage(void * data)
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
	      int avg = averageSurrounds(x, y, *img);
	      setGrayPixel(x, y, *img, avg);
	    }
	}
    }
  return (void*)img;
  }*/

void ImageManipulator::LMC_avgSurroundings(cv::Mat &from, cv::Mat &to)
{
  float _scale = (float)from.rows / (float)to.rows;

  if (to.data != NULL)
    {
      if (to.channels() == 3)
	{
	  //cv::Mat * r = new cv::Mat(from.rows, from.cols, CV_8U);
	  //split

	  try
	    {
	      for (int y = 0 ; y < to.rows ; ++y)
		{
		  for (int x = 0 ; x < to.cols ; ++x)
		    {	
		      if (x > 1
			  && y > 1
			  && x < to.cols - 1
			  && y < to.rows - 1)
			{
			  setPixelColor(x, 
					y,
					to,
					averageSurrounds(floor(x * _scale), 
							 floor(y * _scale), 
							 from));
			}
		      else
			{
			  setPixelColor(x,
					y,
					to,
					getPixelColor(floor(x * _scale),
						      floor(y * _scale),
						      from));
			}
		    }
		}
	    }
	  catch (std::exception & e)
	    {
	      std::cout << "3 channels LMC_averageSurroundings" << e.what() << std::endl;
	    }
	}
    }
  if (to.channels() == 1)
    {
      try
	{
	  for (int y = 0 ; y < to.rows ; ++y)
	    {
	      for (int x = 0 ; x < to.cols ; ++x)
		{	
		  if (x > 1
		      && y > 1
		      && x < to.cols - 1
		      && y < to.rows - 1)
		    {
		      setGrayPixel(x, 
				   y,
				   to,
				   averageSurrounds(floor(x * _scale),
						    floor(y * _scale),
						    from));
		    }
		  else
		    {
		      setGrayPixel(x,
				   y,
				   to,
				   getGrayPixel(floor(x * _scale),
				   		floor(y * _scale),
						from));
		    }
		}
	    }
	}
      catch (std::exception & e)
	{
	  std::cout << "Exception in 1 channel LMC_averageSurroundings" << e.what() << std::endl;
	}
    }
}

void ImageManipulator::scale(cv::Mat &from, cv::Mat &to, SCALE_TYPE type)
{
  std::map<int, LMC_FUNCTION_POINTER>::iterator result = 
    LMC_FUNCTIONS.find((int)type);

  if(result != LMC_FUNCTIONS.end())
    {
      (this->*(result->second))(from, to);
    }
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
