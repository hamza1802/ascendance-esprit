#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header1.h"

SDL_Surface *completePuzzleImage = NULL;
SDL_Rect MISSING_PIECE_POSITION;
Mix_Chunk *successSound = NULL;
Mix_Chunk *failureSound = NULL;

SDL_Surface* chargerImage(const char *chemin) {
    SDL_Surface *image = IMG_Load(chemin);
    if (!image) {
        fprintf(stderr, "Erreur de chargement de l'image : %s\n", chemin);
        exit(EXIT_FAILURE);
    }
}

void initializeMissingPiecePosition(void) {
    MISSING_PIECE_POSITION.x = MISSING_PIECE_X;
    MISSING_PIECE_POSITION.y = MISSING_PIECE_Y;
    MISSING_PIECE_POSITION.w = PIECE_WIDTH;
    MISSING_PIECE_POSITION.h = PIECE_HEIGHT;
    //Add debug print to verify position
    printf("Missing piece position initialized to: x=%d, y=%d, w=%d, h=%d\n",
           MISSING_PIECE_POSITION.x, MISSING_PIECE_POSITION.y,
           MISSING_PIECE_POSITION.w, MISSING_PIECE_POSITION.h);
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

void afficherMessage(SDL_Surface *screen, const char *message, int succes) {
    SDL_Color textColor = succes ? (SDL_Color){0, 255, 0} : (SDL_Color){255, 0, 0};
    
    TTF_Font *font = TTF_OpenFont("arial.ttf", 36);
    if (!font) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        return;
    }
    
    SDL_Surface *texte = TTF_RenderText_Solid(font, message, textColor);
    if (texte) {
        SDL_Surface *texteZoom = rotozoomSurface(texte, 0, 1.5, 1);
        SDL_Rect position = {
            screen->w / 2 - texteZoom->w / 2,
            50
        };
        SDL_BlitSurface(texteZoom, NULL, screen, &position);
        SDL_FreeSurface(texte);
        SDL_FreeSurface(texteZoom);
    }
    TTF_CloseFont(font);
}

int estPieceBonne(Piece *piece) {
    // Compare piece dimensions with correct piece dimensions
    if (piece->image->w != PIECE_WIDTH || piece->image->h != PIECE_HEIGHT) {
        printf("Wrong piece dimensions: expected %dx%d, got %dx%d\n", 
               PIECE_WIDTH, PIECE_HEIGHT, piece->image->w, piece->image->h);
        return 0;
    }

    // Check position with some tolerance
    int margin = 30;
    int isCorrectPosition = (abs(piece->position.x - MISSING_PIECE_POSITION.x) < margin &&
                           abs(piece->position.y - MISSING_PIECE_POSITION.y) < margin);
    
    printf("Piece check: Position=%d, IsCorrect=%d\n", isCorrectPosition, piece->isCorrectPiece);
    return isCorrectPosition && piece->isCorrectPiece;
}

SDL_Rect getMissingPiecePosition(void) {
    return MISSING_PIECE_POSITION;
}

int verifierPosition(SDL_Surface* screen, Piece* piece) {
    // Increase tolerance for easier positioning
    int tolerance = 100;  // Increased tolerance
    
    // Get differences from target position
    int xDiff = abs(piece->position.x - MISSING_PIECE_X);
    int yDiff = abs(piece->position.y - MISSING_PIECE_Y);
    
    // Debug output
    printf("Checking piece at (%d,%d)\n", piece->position.x, piece->position.y);
    printf("Target position: (%d,%d)\n", MISSING_PIECE_X, MISSING_PIECE_Y);
    printf("Distance: xDiff=%d, yDiff=%d, isCorrectPiece=%d\n", 
           xDiff, yDiff, piece->isCorrectPiece);
    
    // Check if this is the correct piece AND it's in position
    if (piece->isCorrectPiece && xDiff < tolerance && yDiff < tolerance) {
        // Snap to exact position
        piece->position.x = MISSING_PIECE_X;
        piece->position.y = MISSING_PIECE_Y;
        printf("SUCCESS: Correct piece in right position!\n");
        return 1;
    }
    
    printf("FAILED: %s\n", piece->isCorrectPiece ? 
           "Wrong position" : "Wrong piece");
    return 0;
}
void showResultPopup(SDL_Surface *parentScreen, int isSuccess) {
    // Create a smaller window for the popup
    SDL_Surface *popup = SDL_SetVideoMode(400, 200, 32, SDL_HWSURFACE);
    if (!popup) {
        fprintf(stderr, "Couldn't create popup window: %s\n", SDL_GetError());
        return;
    }

    // Fill popup with white background
    SDL_FillRect(popup, NULL, SDL_MapRGB(popup->format, 255, 255, 255));

    // Create larger font for message
    TTF_Font* popupFont = TTF_OpenFont("assets/arial.ttf", 36);
    if (popupFont) {
        SDL_Color textColor;
        if (isSuccess) {
            textColor.r = 0;
            textColor.g = 255;
            textColor.b = 0;
        } else {
            textColor.r = 255;
            textColor.g = 0;
            textColor.b = 0;
        }

        const char* message = isSuccess ? "Félicitations!" : "Essayez encore!";

        SDL_Surface* text = TTF_RenderText_Blended(popupFont, message, textColor);
        if (text) {
            SDL_Rect textPos;
            textPos.x = (400 - text->w) / 2;
            textPos.y = (200 - text->h) / 2;
            textPos.w = text->w;
            textPos.h = text->h;

            SDL_BlitSurface(text, NULL, popup, &textPos);
            SDL_Flip(popup);

            // Play appropriate sound
            if (isSuccess && successSound) {
                Mix_PlayChannel(-1, successSound, 0);
            } else if (!isSuccess && failureSound) {
                Mix_PlayChannel(-1, failureSound, 0);
            }

            // Wait for a moment
            SDL_Delay(2000);

            SDL_FreeSurface(text);
        }
        TTF_CloseFont(popupFont);
    }
}
