# Makefile pour le projet de jeu avec ennemis

# Compilateur et options
CC = gcc
CFLAGS = -Wall -g -Wno-switch
SDL_FLAGS = $(shell sdl-config --cflags --libs) -lSDL_ttf -lSDL_mixer -lSDL_image -lm

# Fichiers source et objets
SRCS = main.c source.c
OBJS = $(SRCS:.c=.o)

# Nom de l'exécutable
EXEC = game

# Règle principale
all: $(EXEC)

# Compilation de l'exécutable
$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(SDL_FLAGS)

# Règle générique pour les fichiers objets
%.o: %.c header.h
	$(CC) -o $@ -c $< $(CFLAGS) $(SDL_FLAGS)

# Nettoyage
clean:
	rm -f $(OBJS) $(EXEC)

# Exécution du programme
run: $(EXEC)
	./$(EXEC)

# Pour éviter les conflits avec des fichiers du même nom
.PHONY: all clean run 