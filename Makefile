NAME_MAIN = main
NAME_THREADED = threadedscale

SRC_MAINSOFT = maintemp.cpp ImageManipulator.cpp
SRC_SCALING = ThreadedScaling.cpp ImageManipulator.cpp

CXX = g++
CXX_FLAGS_MAIN = -W -Wall -Werror -lopencv_imgproc -lopencv_legacy -lopencv_highgui -lopencv_core -std=c++0x
RULES = main
RM = rm -rf

all : $(RULES)

main : $(SRC_MAIN)
	$(CXX) $(SRC_MAINSOFT) -o $(NAME_MAIN) $(CXX_FLAGS_MAIN) 

threadedscale : $(SRC_SCALING)
		$(CXX) $(SRC_SCALING) -o $(NAME_THREADED) $(CXX_FLAGS_MAIN)

clean :
	$(RM) $(NAME_MAIN)
	$(RM) $(NAME_THREADED)
	$(RM) *~
	$(RM) -f *.o

re : clean all