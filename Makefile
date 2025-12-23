BUILD = build
SRC = src
BIN = bin

$(BUILD):
	mkdir -p build
	mkdir -p bin

$(BIN):
	mkdir -p bin

OBJS = $(BUILD)/main.o $(BUILD)/math.o $(BUILD)/game_algorithms.o $(BUILD)/Level.o

$(BUILD)/math.o: $(BUILD)
	g++ -g -c $(SRC)/math/math.cpp -o $(BUILD)/math.o

$(BUILD)/game_algorithms.o: $(BUILD)
	g++ -g -c $(SRC)/game_algorithms.cpp -o $(BUILD)/game_algorithms.o

$(BUILD)/Level.o: $(BUILD)
	g++ -g -c $(SRC)/Level.cpp -o $(BUILD)/Level.o

$(BUILD)/main.o: $(BUILD)
	g++ -g -c ./main.cpp -o $(BUILD)/main.o

# $^ - for prerequisites
program: $(OBJS)
	g++ -g $^ -o ray -lSDL2

main_release:
	g++ main.cpp -o $(BIN)/ray_release -lSDL2

math_test: math_test.cpp
	g++ -g math_test.cpp -o $(BIN)/math_test

tags:
	ctags -R .

clean:
	rm -rf $(BUILD)
