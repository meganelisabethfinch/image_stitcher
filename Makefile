CFLAGS = -std=c++11
LDFLAGS = `pkg-config --cflags --libs opencv4`
SRC = ./src/*.cpp

ImageStitcher:
	g++ $(LDFLAGS) $(CFLAGS) ./src/main.cpp -o ./build/ImageStitcher

run:
	./build/ImageStitcher -i ./images/NewnLeft.jpeg -i ./images/NewnRight.jpeg -o ./images/result.jpeg