CFLAGS = -std=c++11 -I./headers
LDFLAGS = `pkg-config --cflags --libs opencv4`
SRC = ./src/*.cpp

app:
	g++ $(LDFLAGS) $(CFLAGS) $(SRC) -o ./build/ImageStitcher


run:
	./build/ImageStitcher -i ./images/mountain1.png -i ./images/mountain2.png -o ./images/result.jpeg