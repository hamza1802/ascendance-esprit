#include "header.h"
#include <SDL/SDL.h> // Include SDL header for SDL_Point

#define SCREEN_W 1280
#define SCREEN_H 720

// Function to check if a point is inside a rectangle
int SDL_PointInRect(const SDL_Point* p, const SDL_Rect* r) {
    return (p->x >= r->x && p->x < (r->x + r->w) &&
            p->y >= r->y && p->y < (r->y + r->h));
}

int main(int argc, char* argv[]) {
    init_SDL();

    // Charger les questions depuis le fichier texte
    load_quizzes("quizz.txt");

    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "SDL_SetVideoMode Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_WM_SetCaption("Game Menu", NULL);

    SDL_Surface *background, *quizBtnImg, *puzzleBtnImg, *optionA, *optionB, *optionC;
    TTF_Font* font;
    load_assets(&background, &quizBtnImg, &puzzleBtnImg, &optionA, &optionB, &optionC, &font);

    Button buttons[5] = {
        {quizBtnImg, {300, SCREEN_H / 2, 200, 100}, 0},
        {puzzleBtnImg, {600, SCREEN_H / 2, 200, 100}, 0},
        {optionA, {0, 0, optionA->w, optionA->h}, 0},
        {optionB, {0, 0, optionB->w, optionB->h}, 0},
        {optionC, {0, 0, optionC->w, optionC->h}, 0}
    };

    int running = 1;
    while (running) {
        handle_events(&running, buttons, 5);
        render(screen, background, buttons, 5, font);
    }

    cleanup(background, quizBtnImg, puzzleBtnImg, optionA, optionB, optionC, font);
    return 0;
}
