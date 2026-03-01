#CFLAGS = -g -fdiagnostics-color=always -Iexternal -Isrc
CFLAGS = -g -Iexternal -Isrc -std=c++17
BUILD = build
SRC = src
BIN = bin

$(BUILD):
	mkdir -p build
	mkdir -p bin

$(BIN):
	mkdir -p bin

OBJS = $(BUILD)/main.o \
	   $(BUILD)/math.o \
	   $(BUILD)/game_algorithms.o \
	   $(BUILD)/Level.o \
	   $(BUILD)/Platform.o \
	   $(BUILD)/Window.o \
	   $(BUILD)/stb_image_impl.o \
       $(BUILD)/TextureManager.o

$(BUILD)/math.o: $(BUILD) $(SRC)/math/math.cpp
	g++ $(CFLAGS) -c $(SRC)/math/math.cpp -o $(BUILD)/math.o

$(BUILD)/game_algorithms.o: $(BUILD) $(SRC)/game_algorithms.cpp
	g++ $(CFLAGS) -c $(SRC)/game_algorithms.cpp -o $(BUILD)/game_algorithms.o

$(BUILD)/Level.o: $(BUILD) $(SRC)/Level.cpp
	g++ $(CFLAGS) -c $(SRC)/Level.cpp -o $(BUILD)/Level.o

$(BUILD)/Platform.o: $(BUILD) $(SRC)/Platform.cpp
	g++ $(CFLAGS) -c $(SRC)/Platform.cpp -o $(BUILD)/Platform.o

$(BUILD)/Window.o: $(BUILD) $(SRC)/Window.cpp
	g++ $(CFLAGS) -c $(SRC)/Window.cpp -o $(BUILD)/Window.o

$(BUILD)/stb_image_impl.o: $(BUILD) $(SRC)/stb_image_impl.cpp
	g++ $(CFLAGS) -c $(SRC)/stb_image_impl.cpp -o $(BUILD)/stb_image_impl.o

$(BUILD)/TextureManager.o: $(BUILD) $(SRC)/TextureManager.cpp
	g++ $(CFLAGS) -c $(SRC)/TextureManager.cpp -o $(BUILD)/TextureManager.o

$(BUILD)/main.o: $(BUILD) main.cpp
	g++ $(CFLAGS) -c ./main.cpp -o $(BUILD)/main.o

# $^ - for prerequisites
program: $(OBJS)
	g++ $(CFLAGS) $^ -o $(BIN)/ray -lSDL2

main_release:
	g++ main.cpp -o $(BIN)/ray_release -lSDL2

math_test: math_test.cpp
	g++ $(CFLAGS) math_test.cpp -o $(BIN)/math_test

tags:
	ctags -R .

clean:
	rm -rf $(BUILD)
