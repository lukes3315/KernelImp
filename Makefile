NAME_LIB = Image.so
NAME_MAIN = main

SRC_LIBRARY = Image.hpp
SRC_MAINSOFT = main.cpp

CXX = g++
CXX_FLAGS_LIB = -fPIC -shared
CXX_FLAGS_MAIN = -W -Wall -Werror -lopencv_imgproc -lopencv_legacy -lopencv_highgui -lopencv_core -debug
RULES = lib main
RM = rm -rf

all : $(RULES)

lib : $(SRC_LIBRARY)
	$(CXX) $(CXX_FLAGS_LIB) $(SRC_LIBRARY) -o $(NAME_LIB)

main : $(SRC_MAIN)
	$(CXX) $(SRC_MAINSOFT) -o $(NAME_MAIN) $(CXX_FLAGS_MAIN) 

clean :
	$(RM) $(NAME_MAIN)
	$(RM) $(NAME_LIB)
	$(RM) *~
	$(RM) -f *.o

re : clean all