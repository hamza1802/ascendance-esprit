prog: main.o source.o fonction1.o
	gcc main.o source.o fonction1.o -o prog -lSDL -lSDL_image -lSDL_gfx -lSDL_mixer -lSDL_ttf -g

main.o: main.c header.h
	gcc -c main.c -g

source.o: source.c header.h
	gcc -c source.c -g

fonction1.o: fonction1.c header1.h
	gcc -c fonction1.c -g

clean:
	rm -f *.o prog
