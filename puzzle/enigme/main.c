#include <stdio.h>
#include <stdlib.h>
#include "header.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1440;
    const int screenHeight = 900;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Erreur d'initialisation de SDL : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "Erreur d'initialisation de SDL_ttf : %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Surface *screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE);
    if (!screen) {
        fprintf(stderr, "Erreur de création de la fenêtre : %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }
    initializeMissingPiecePosition();
    SDL_WM_SetCaption("Jeu de Puzzle", NULL);

    completePuzzleImage = chargerImage("assets/final.png");
    SDL_Surface *imagePuzzle = chargerImage("assets/missing.png");
    SDL_Surface *correctPiece = chargerImage("assets/correct.png");
     // Initialize missing piece position
     initializeMissingPiecePosition();
    Piece propositions[3] = {
        {chargerImage("assets/wrong1.png"), {100, 400, 0, 0}},
        {chargerImage("assets/wrong2.png"), {300, 400, 0, 0}},
        {correctPiece, {500, 800, 0, 0}, 1}
    };

    // Initialize piece dimensions
    for (int i = 0; i < 3; i++) {
        propositions[i].position.w = propositions[i].image->w;
        propositions[i].position.h = propositions[i].image->h;
    }

    melangerPropositions(propositions, 3);

    int pieceSelectionnee = -1;
    int continuer = 1;
    int mouseOffsetX = 0;
    int mouseOffsetY = 0;
    SDL_Event event;
    
    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    continuer = 0;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        for (int i = 0; i < 3; i++) {
                            SDL_Rect *pos = &propositions[i].position;
                            if (event.button.x >= pos->x && 
                                event.button.x <= pos->x + pos->w &&
                                event.button.y >= pos->y && 
                                event.button.y <= pos->y + pos->h) {
                                pieceSelectionnee = i;
                                // Store offset between mouse and piece position
                                mouseOffsetX = event.button.x - pos->x;
                                mouseOffsetY = event.button.y - pos->y;
                                break;
                            }
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (pieceSelectionnee != -1) {
                        verifierPosition(screen, &propositions[pieceSelectionnee]);
                        pieceSelectionnee = -1;
                    }
                    break;
                    
                case SDL_MOUSEMOTION:
                    if (pieceSelectionnee != -1) {
                        // Use stored offset for smooth dragging
                        propositions[pieceSelectionnee].position.x = 
                            event.motion.x - mouseOffsetX;
                        propositions[pieceSelectionnee].position.y = 
                            event.motion.y - mouseOffsetY;
                    }
                    break;
            }
        }

        // Draw background
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
       
        // Draw puzzle base
        SDL_Rect puzzlePos = {200, 100, 0, 0};
        afficherImage(screen, imagePuzzle, puzzlePos);
        
        // Draw pieces
        for (int i = 0; i < 3; i++) {
            afficherImage(screen, propositions[i].image, propositions[i].position);
        }

        // Draw timer
        afficherChronometre(screen, 50, 100, (SDL_Rect){50, 50, 200, 20});
        
        SDL_Flip(screen);
    }

    // Cleanup
    SDL_FreeSurface(imagePuzzle);
    SDL_FreeSurface(completePuzzleImage);
    for (int i = 0; i < 3; i++) {
        SDL_FreeSurface(propositions[i].image);
    }

    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}