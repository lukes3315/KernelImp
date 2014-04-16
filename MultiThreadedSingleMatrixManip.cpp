#include <iostream>
#include <cstdlib>
#include <string.h>
#include <pthread.h>

#define Y 48
#define X 64

struct threadData
{
  int x, y;
  int lim_x, lim_y;
  char *** data;
  char c;
};

void * thread_method(void * data)
{
  threadData * _data = (threadData*)data;

  for (int y = _data->y; y < _data->lim_y ; ++y)
    {
      for (int x = _data->x ; x < _data->lim_x; ++x)
	{
	  char ** k = *_data->data;
	  k[y][x] = _data->c;
	}
    }
}

void displayData(char ** k)
{
  for (int i = 0 ; i < Y ; ++i)
    {
      std::cout << k[i] << std::endl;
    }
}

int main(void)
{
  char ** k = (char**)malloc(sizeof(char*) * Y);
  for (int i = 0 ; i < Y ; ++i)
    {
      k[i] = (char*)malloc(sizeof(char) * X);
      memset(k[i], '0', X);
    }
  pthread_t pthread[4];
  threadData data[4];

  for (int i = 0 ; i < 4; ++i)
    {
      switch (i)
	{
	case 0:
	  data[i].x = 0;
	  data[i].y = 0;
	  data[i].lim_x = X / 2;
	  data[i].lim_y = Y / 2;
	  data[i].c = '1';
	  data[i].data = &k;
	  if (!pthread_create(&pthread[i], NULL, thread_method, &data[i]))
	    {
	      std::cout << "good for thread #" << i << std::endl;
	    }
	  break;
	case 1:
	  data[i].x = X / 2;
	  data[i].y = 0;
	  data[i].lim_x = X;
	  data[i].lim_y = Y / 2;
	  data[i].c = '2';
	  data[i].data = &k;
	  if (!pthread_create(&pthread[i], NULL, thread_method, &data[i]))
	    {
	      std::cout << "good for thread #" << i << std::endl;
	    }
	  break;
	case 2:
	  data[i].x = 0;
	  data[i].y = Y / 2;
	  data[i].lim_x = X / 2;
	  data[i].lim_y = Y;
	  data[i].c = '3';
	  data[i].data = &k;
	  if (!pthread_create(&pthread[i], NULL, thread_method, &data[i]))
	    {
		std::cout << "good for thread #" << i << std::endl;
	    }
	  break;
	case 3:
	  data[i].x = X / 2;
	  data[i].y = Y / 2;
	  data[i].lim_x = X;
	  data[i].lim_y = Y;
	  data[i].c = '4';
	  data[i].data = &k;
	  if (!pthread_create(&pthread[i], NULL, thread_method, &data[i]))
	      {
		std::cout << "good for thread #" << i << std::endl;
	      }
	  break;
	}
    }
  
  if (!pthread_join(pthread[0], NULL)//(void**)&threaddataR)
      && !pthread_join(pthread[1], NULL)//(void**)&threaddataG)
      && !pthread_join(pthread[2], NULL)//(void**)&threaddataB))
      && !pthread_join(pthread[3], NULL))//(void**)&threaddataB))
    {
      displayData(k);
    }
  
  for (int i = 0 ; i < Y ; ++i)
    {
      free(k[i]);
    }
  free(k);
  
  
}
