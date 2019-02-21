all: simulator

simulator: simulator.c
	gcc -Wall -Werror -fsanitize=address simulator.c -o simulator -lm

clean:
	-rm -rf simulator

bug:
	gcc -Wall  -Werror -fsanitize=address simulator.c -o simulator -g -lm
