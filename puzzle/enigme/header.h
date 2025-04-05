#ifndef HEADER_H
#define HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>

// Constants for missing piece position and dimensions
#define MISSING_PIECE_X 445  // Distance from left
#define MISSING_PIECE_Y 187  // Distance from top
#define PIECE_WIDTH 261      // Width of the correct piece
#define PIECE_HEIGHT 277     // Height of the correct piece

typedef struct {
    SDL_Surface *image;
    SDL_Rect position;
    int isCorrectPiece;
} Piece;

// External declarations
extern SDL_Surface *completePuzzleImage;
extern SDL_Rect MISSING_PIECE_POSITION;

// Function declarations
SDL_Surface* chargerImage(const char *chemin);
void initializeMissingPiecePosition(void);
void melangerPropositions(Piece propositions[], int taille);
void afficherImage(SDL_Surface *screen, SDL_Surface *image, SDL_Rect position);
void afficherChronometre(SDL_Surface *screen, int tempsRestant, int tempsTotal, SDL_Rect position);
void afficherMessage(SDL_Surface *screen, const char *message, int succes);
int estPieceBonne(Piece *piece);
void verifierPosition(SDL_Surface *screen, Piece *piece);

#endif