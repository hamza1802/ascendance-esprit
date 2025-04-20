#ifndef HEADER1_H
#define HEADER1_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

#define SCREEN_WIDTH 1400  // Increased from 1280 to 1400
#define SCREEN_HEIGHT 900  // Increased from 720 to 900
#define WELCOME_WIDTH 1000  // Increased from 800 to 1000
#define WELCOME_HEIGHT 750  // Increased from 600 to 750
#define PREVIEW_WIDTH 200
#define PREVIEW_HEIGHT 200
#define PREVIEW_X 50
#define PREVIEW_Y 300
#define PUZZLE_X 300
#define PUZZLE_Y 50
#define PIECE_WIDTH 180
#define PIECE_HEIGHT 180
#define NUM_PIECES 9
#define NUM_PUZZLES 1
#define TOTAL_TIME 90
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

typedef struct {
    Puzzle puzzles[NUM_PUZZLES];
    int puzzle_order[NUM_PUZZLES];
    int current_puzzle_index;
} GameStatePuzzle;

typedef struct {
    SDL_Surface *barImages[NUM_BAR_LEVELS];
    int barInitialized;
} BarTimer;

extern Puzzle puzzles[];

SDL_Surface* chargerImage(const char *chemin);
void init_puzzle_order(GameStatePuzzle *state);
void load_puzzle(Puzzle *puzzle, SDL_Surface **completeImage, GameStatePuzzle *state);
void afficherImage(SDL_Surface *screen, SDL_Surface *image, SDL_Rect position);
int calculate_score(int elapsedTime);
void showResultPopup(SDL_Surface *parentScreen, int isSuccess, int elapsedTime, Mix_Chunk *successSound, Mix_Chunk *failureSound);
void draw_piece_highlight(SDL_Surface *screen, SDL_Rect *position);
int check_puzzle_completion(Puzzle *puzzle);
int show_welcome_screen(SDL_Surface *screen, Mix_Chunk *clickSound);
int show_rules_screen(SDL_Surface *screen, Mix_Chunk *clickSound);
void displayTimerBar(SDL_Surface *screen, int elapsedTime, SDL_Rect *barPos, BarTimer *barTimer);
void cleanup_bar_images(BarTimer *barTimer);
void snap_piece(Piece *piece);
void run_puzzle_game(SDL_Surface *screen, TTF_Font *font, Mix_Chunk *clickSound, Mix_Chunk *successSound, Mix_Chunk *failureSound, Mix_Chunk *countdownSounds[], Mix_Music *bgMusic);

#endif
