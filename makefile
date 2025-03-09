CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

SRCS = main.c source.c
OBJS = $(SRCS:.c=.o)
EXEC = jeu

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC)
