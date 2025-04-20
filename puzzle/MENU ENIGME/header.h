#ifndef HEADER_H
#define HEADER_H
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>

#define SCREEN_W 1280
#define SCREEN_H 720
#define MAX_QUESTIONS 100
#define MAX_LINE_LENGTH 512

// Define a Button structure
typedef struct {
    SDL_Surface* surface;
    SDL_Rect rect;
    int active;
} Button;

// Define a Quiz structure
typedef struct {
    const char* question;
    const char* answerA;
    const char* answerB;
    const char* answerC;
    int correctAnswer; // 1 for A, 2 for B, 3 for C
} Quiz;

// Define a structure for SDL_Point
typedef struct {
    int x;
    int y;
} SDL_Point;

// Define game states
typedef int GameState;
#define MAIN_MENU 0
#define QUIZ 1
#define PUZZLE 2

// Declare global variables
extern Mix_Chunk* hoverSound;
extern Mix_Chunk* clickSound;
extern Mix_Music* bgMusic;
extern GameState currentState;
extern int currentQuizIndex;

void init_SDL();
int SDL_PointInRect(const SDL_Point* p, const SDL_Rect* r);
SDL_Surface* load_image(const char* filename);
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* dest);
void load_quizzes(const char* filename);  // Added declaration
void load_assets(SDL_Surface** background, SDL_Surface** quizBtnImg, SDL_Surface** puzzleBtnImg, SDL_Surface** optionA, SDL_Surface** optionB, SDL_Surface** optionC, TTF_Font** font);
void handle_events(int* running, Button buttons[], int buttonCount);
void render(SDL_Surface* screen, SDL_Surface* background, Button buttons[], int buttonCount, TTF_Font* font);
void cleanup(SDL_Surface* background, SDL_Surface* quizBtnImg, SDL_Surface* puzzleBtnImg, SDL_Surface* optionA, SDL_Surface* optionB, SDL_Surface* optionC, TTF_Font* font);
void show_score_window(int score);

#endif // HEADER_H
