#include "header.h"
#include "header1.h"
#include <SDL/SDL.h>
#include <time.h>

// Function to check if a point is inside a rectangle (already in main.c)
int SDL_PointInRect(const SDL_Point* p, const SDL_Rect* r) {
    return (p->x >= r->x && p->x < (r->x + r->w) &&
            p->y >= r->y && p->y < (r->y + r->h));
}

int main(int argc, char* argv[]) {
    srand(time(NULL)); // From main1.c for puzzle game randomization
    init_SDL();

    // Load quizzes for the quiz game
    load_quizzes("quizz.txt");

    // Initialize screen for the main menu
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "SDL_SetVideoMode Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_WM_SetCaption("Game Menu", NULL);

    // Load assets for the quiz game
    SDL_Surface *background, *quizBtnImg, *puzzleBtnImg, *optionA, *optionB, *optionC;
    TTF_Font* font;
    load_assets(&background, &quizBtnImg, &puzzleBtnImg, &optionA, &optionB, &optionC, &font);

    // Initialize buttons for the main menu
    Button buttons[5] = {
        {quizBtnImg, {300, SCREEN_H / 2, 200, 100}, 0},     // Quiz button
        {puzzleBtnImg, {600, SCREEN_H / 2, 200, 100}, 0},   // Puzzle button
        {optionA, {0, 0, optionA->w, optionA->h}, 0},       // Option A
        {optionB, {0, 0, optionB->w, optionB->h}, 0},       // Option B
        {optionC, {0, 0, optionC->w, optionC->h}, 0}        // Option C
    };

    int running = 1;
    while (running) {
        handle_events(&running, buttons, 5);
        render(screen, background, buttons, 5, font);
    }

    // Cleanup resources
    cleanup(background, quizBtnImg, puzzleBtnImg, optionA, optionB, optionC, font);
    return 0;
}
