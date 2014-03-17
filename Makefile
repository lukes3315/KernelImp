NAME_MAIN = main

SRC_MAINSOFT = maintemp.cpp ImageManipulator.cpp

CXX = g++
CXX_FLAGS_MAIN = -W -Wall -Werror -lopencv_imgproc -lopencv_legacy -lopencv_highgui -lopencv_core
RULES = main
RM = rm -rf

all : $(RULES)

main : $(SRC_MAIN)
	$(CXX) $(SRC_MAINSOFT) -o $(NAME_MAIN) $(CXX_FLAGS_MAIN) 

clean :
	$(RM) $(NAME_MAIN)
	$(RM) *~
	$(RM) -f *.o

re : clean all