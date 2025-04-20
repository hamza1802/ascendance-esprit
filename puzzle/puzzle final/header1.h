#ifndef HEADER_H
#define HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_mixer.h>

// Constants
#define SCREEN_WIDTH 1440
#define SCREEN_HEIGHT 900
#define PUZZLE_WIDTH 540
#define PUZZLE_HEIGHT 540
#define PIECE_WIDTH 180
#define PIECE_HEIGHT 180
#define NUM_PIECES 9
#define NUM_PUZZLES 1
#define PUZZLE_X (SCREEN_WIDTH - PUZZLE_WIDTH) / 2  // 450
#define PUZZLE_Y (SCREEN_HEIGHT - PUZZLE_HEIGHT) / 2  // 180 (centered vertically)
#define WELCOME_WIDTH 1000
#define WELCOME_HEIGHT 750
#define PREVIEW_X 50
#define PREVIEW_Y 50
#define PREVIEW_WIDTH 200
#define PREVIEW_HEIGHT 200
#define TOTAL_TIME 60
#define NUM_BAR_LEVELS 6

typedef struct {
    SDL_Surface *image;
    SDL_Rect position;
    SDL_Rect target;
    int isPlaced;
} Piece;

typedef struct {
    char *full_image_path;
    char *piece_paths[NUM_PIECES];
    Piece pieces[NUM_PIECES];
} Puzzle;

// Struct to manage puzzle selection state
typedef struct {
    Puzzle puzzles[NUM_PUZZLES];
    int puzzle_order[NUM_PUZZLES];
    int current_puzzle_index;
} GameState;

// Struct to manage bar timer images
typedef struct {
    SDL_Surface *barImages[NUM_BAR_LEVELS];
    int barInitialized;
} BarTimer;

// Function declarations
SDL_Surface* chargerImage(const char *chemin);
void init_puzzle_order(GameState *state);
void load_puzzle(Puzzle *puzzle, SDL_Surface **completeImage, GameState *state);
void afficherImage(SDL_Surface *screen, SDL_Surface *image, SDL_Rect position);
void showResultPopup(SDL_Surface *parentScreen, int isSuccess, int elapsedTime, Mix_Chunk *successSound, Mix_Chunk *failureSound);
void draw_piece_highlight(SDL_Surface *screen, SDL_Rect *position);
int check_puzzle_completion(Puzzle *puzzle);
void snap_piece(Piece *piece);
int show_welcome_screen(SDL_Surface *screen, Mix_Chunk *clickSound);
int show_rules_screen(SDL_Surface *screen, Mix_Chunk *clickSound);
int calculate_score(int elapsedTime);
void displayTimerBar(SDL_Surface *screen, int elapsedTime, SDL_Rect *barPos, BarTimer *barTimer);
void cleanup_bar_images(BarTimer *barTimer);

#endif
