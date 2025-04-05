#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header1.h"

SDL_Surface *completePuzzleImage = NULL;
SDL_Rect MISSING_PIECE_POSITION;

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
    
    TTF_Font *font = TTF_OpenFont("font.ttf", 36);
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
    int tolerance = 50;
    SDL_Rect position = getMissingPiecePosition();
    SDL_Rect piecePos = piece->position;

    // Check if the piece is within tolerance range of the target position
    int xDiff = abs(piecePos.x - position.x);
    int yDiff = abs(piecePos.y - position.y);

    if (xDiff > tolerance || yDiff > tolerance) {
        printf("Position mismatch: xDiff=%d, yDiff=%d, tolerance=%d\n", xDiff, yDiff, tolerance);
        return 0; // La pièce est trop loin de la position cible
    }

    if (estPieceBonne(piece)) {
        afficherMessage(screen, "Félicitations ! Vous avez trouvé la bonne pièce !", 1);
        return 1; // La pièce est à la bonne place
    } else {
        afficherMessage(screen, "Essayez encore !", 0);
        return 0; // La pièce n'est pas à la bonne place
    }
}
