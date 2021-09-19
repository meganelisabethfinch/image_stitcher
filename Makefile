CFLAGS = -std=c++11 -I./headers
LDFLAGS = `pkg-config --cflags --libs opencv4`
SRC = ./src/*.cpp

app:
	g++ $(LDFLAGS) $(CFLAGS) $(SRC) -o ./build/ImageStitcher


run:
	./build/ImageStitcher -i ./images/NewnLeft.jpeg -i ./images/NewnRight.jpeg -o ./images/result.jpeg