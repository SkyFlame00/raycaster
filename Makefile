main:
	g++ -g main.cpp -o ray -lSDL2

main_release:
	g++ main.cpp -o ray_release -lSDL2

math_test: math_test.cpp
	g++ -g math_test.cpp -o math_test

tags:
	ctags -R .
