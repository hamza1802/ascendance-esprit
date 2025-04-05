#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header.h"

SDL_Rect CORRECT_POSITION = {300, 200, 0, 0};
SDL_Surface *completePuzzleImage = NULL;
SDL_Rect MISSING_PIECE_POSITION;

SDL_Surface* chargerImage(const char *chemin) {
    SDL_Surface *image = IMG_Load(chemin);
    if (!image) {
        fprintf(stderr, "Erreur de chargement de l'image : %s\n", chemin);
        exit(EXIT_FAILURE);
    }
    printf("Image %s dimensions: %dx%d\n", chemin, image->w, image->h);
    return image;
}
void initializeMissingPiecePosition(void) {
    MISSING_PIECE_POSITION.x = MISSING_PIECE_X;
    MISSING_PIECE_POSITION.y = MISSING_PIECE_Y;
    MISSING_PIECE_POSITION.w = PIECE_WIDTH;
    MISSING_PIECE_POSITION.h = PIECE_HEIGHT;
}


void melangerPropositions(Piece propositions[], int taille) {
    srand(time(NULL));
    for (int i = 0; i < taille; i++) {
        int j = rand() % taille;
        Piece temp = propositions[i];
        propositions[i] = propositions[j];
        propositions[j] = temp;
    }
}

void afficherImage(SDL_Surface *screen, SDL_Surface *image, SDL_Rect position) {
    SDL_BlitSurface(image, NULL, screen, &position);
}

void afficherChronometre(SDL_Surface *screen, int tempsRestant, int tempsTotal, SDL_Rect position) {
    SDL_Rect barre = position;
    barre.w = (tempsRestant * position.w) / tempsTotal;
    SDL_FillRect(screen, &barre, SDL_MapRGB(screen->format, 255, 0, 0));
}

void afficherMessage(SDL_Surface *screen, const char *message, int succes) {
    SDL_Color textColor;
    if (succes) {
        textColor = (SDL_Color){0, 255, 0};  // Green for success
    } else {
        textColor = (SDL_Color){255, 0, 0};  // Red for failure
    }
    
    TTF_Font *font = TTF_OpenFont("font.ttf", 36);
    if (!font) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        return;
    }
    
    SDL_Surface *texte = TTF_RenderText_Solid(font, message, textColor);
    if (texte) {
        SDL_Rect position = {
            screen->w / 2 - texte->w / 2,    // Center horizontally
            50                               // Fixed position near the top
        };
        SDL_BlitSurface(texte, NULL, screen, &position);
        SDL_FreeSurface(texte);
    }
    
    TTF_CloseFont(font);
}
int estPieceBonne(Piece *piece) {
    int margin = 15;  // Small margin for easier placement
    return (abs(piece->position.x - MISSING_PIECE_POSITION.x) < margin &&
            abs(piece->position.y - MISSING_PIECE_POSITION.y) < margin &&
            piece->isCorrectPiece);
}

void verifierPosition(SDL_Surface *screen, Piece *piece) {
    if (abs(piece->position.x - MISSING_PIECE_POSITION.x) < 30 &&
        abs(piece->position.y - MISSING_PIECE_POSITION.y) < 30) {
        
        if (piece->isCorrectPiece) {
            // Snap to exact position
            piece->position.x = MISSING_PIECE_POSITION.x;
            piece->position.y = MISSING_PIECE_POSITION.y;
            
            afficherMessage(screen, "Bravo! Piece bien placee!", 1);
            SDL_Flip(screen);
            SDL_Delay(1000);

            // Show completed puzzle
            if (completePuzzleImage) {
                SDL_Rect fullPos = {0, 0, 0, 0};
                SDL_BlitSurface(completePuzzleImage, NULL, screen, &fullPos);
                SDL_Flip(screen);
                SDL_Delay(2000);
            }
        } else {
            afficherMessage(screen, "Mauvaise piece!", 0);
            SDL_Flip(screen);
            SDL_Delay(1000);
            // Move wrong piece back to starting position
            piece->position.x = piece->position.x + 50;
            piece->position.y = piece->position.y + 50;
        }
    }
}