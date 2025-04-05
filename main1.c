#include <stdio.h>
#include <stdlib.h>
#include "header1.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1440;
    const int screenHeight = 900;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Erreur d'initialisation de SDL : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "Erreur d'initialisation de SDL_ttf : %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
TTF_Font* font = TTF_OpenFont("arial.ttf", 36);
if (!font) {
    fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    TTF_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
}

// Validate piece dimensions
SDL_Surface *correctPiece = chargerImage("assets/correct.png");
if (correctPiece && (correctPiece->w != 261 || correctPiece->h != 277)) {
    fprintf(stderr, "Incorrect piece dimensions. Expected 261x277, got %dx%d\n", correctPiece->w, correctPiece->h);
    SDL_FreeSurface(correctPiece);
    TTF_Quit();
    SDL_Quit();
    return EXIT_FAILURE;
}
Mix_Chunk *successSound = Mix_LoadWAV("explo.wav");
if (!successSound) {
    fprintf(stderr, "Failed to load success sound! SDL_mixer Error: %s\n", Mix_GetError());
}
Mix_Chunk *failureSound = Mix_LoadWAV("gay.wav");
if (!failureSound) {
    fprintf(stderr, "Failed to load failure sound! SDL_mixer Error: %s\n", Mix_GetError());
}

    SDL_Surface *screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE);
    if (!screen) {
        fprintf(stderr, "Erreur de création de la fenêtre : %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

// After SDL_SetVideoMode and before loading images
    
    // Clear screen to white
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
    
    // Display welcome message
    TTF_Font* welcomeFont = TTF_OpenFont("arial.ttf", 48); // Bigger font size
    if (welcomeFont) {
        SDL_Color textColor = {0, 0, 255}; // Blue color
        SDL_Surface* welcomeText = TTF_RenderText_Blended(font, 
            "Hello! Choose the right piece and place it in the correct spot!", textColor);
        
        if (welcomeText) {
            // Create background
            SDL_Rect textBg = {
                (screenWidth - welcomeText->w) / 2 - 20,
                40,
                welcomeText->w + 40,
                welcomeText->h + 20
            };
            
            // Draw black background
            SDL_FillRect(screen, &textBg, SDL_MapRGB(screen->format, 0, 0, 0));
            
            // Center text
            SDL_Rect messagePos = {
                (screenWidth - welcomeText->w) / 2,
                50,
                welcomeText->w,
                welcomeText->h
            };
            
            // Draw text and update screen
            SDL_BlitSurface(welcomeText, NULL, screen, &messagePos);
            SDL_Flip(screen);
            
            // Important: Wait so message is visible
            SDL_Delay(3000);  // Show for 3 seconds
            
            SDL_FreeSurface(welcomeText);
        }
    }
    
    // Clear screen again before starting game
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
    SDL_Flip(screen);
        
    // Declare completePuzzleImage as global
    SDL_Surface* completePuzzleImage = NULL;
    

    // Load images
    completePuzzleImage = chargerImage("assets/final.png");
    SDL_Surface *imagePuzzle = chargerImage("assets/missing.png");

    // Initialize missing piece position
    initializeMissingPiecePosition();

    // Initialize pieces with correct positions and flags
    Piece propositions[3] = {
        {chargerImage("assets/wrong1.png"), {100, 600, 0, 0}, 0},
        {chargerImage("assets/wrong2.png"), {300, 600, 0, 0}, 0},
        {correctPiece, {500, 600, 0, 0}, 1}
    };

    // Set piece dimensions
    for (int i = 0; i < 3; i++) {
        propositions[i].position.w = propositions[i].image->w;
        propositions[i].position.h = propositions[i].image->h;
    }

    melangerPropositions(propositions, 3);

    int pieceSelectionnee = -1;
    int mouseOffsetX = 0;
    int mouseOffsetY = 0;
    int continuer = 1;
    int puzzleComplete = 0;
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
                                mouseOffsetX = event.button.x - pos->x;
                                mouseOffsetY = event.button.y - pos->y;
                                break;
                            }
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (pieceSelectionnee != -1) {
                        printf("\nChecking piece %d\n", pieceSelectionnee);
                        
                        int isCorrect = verifierPosition(screen, &propositions[pieceSelectionnee]);
                        printf("Verification result: %d\n", isCorrect);
                        
                        // Open new window for feedback
                        SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                        if (newWindow) {
                            // Fill new window with white background
                            SDL_FillRect(newWindow, NULL, SDL_MapRGB(newWindow->format, 255, 255, 255));
                            
                            SDL_Color textColor;
                            const char* message;
                            
                            if (isCorrect) {
                                puzzleComplete = 1;
                                textColor = (SDL_Color){0, 255, 0}; // Green
                                message = "Félicitations! Puzzle Complété!";
                                if (successSound) {
                                    Mix_PlayChannel(-1, successSound, 0);
                                    printf("Playing success sound\n");
                                }
                            } else {
                                textColor = (SDL_Color){255, 0, 0}; // Red
                                message = "Mauvais placement! Essayez encore!";
                                if (failureSound) {
                                    Mix_PlayChannel(-1, failureSound, 0);
                                    printf("Playing failure sound\n");
                                }
                            }
                            
                            SDL_Surface* messageText = TTF_RenderText_Solid(font, message, textColor);
                            if (messageText) {
                                SDL_Rect textPos = {
                                    (800 - messageText->w) / 2,
                                    (600 - messageText->h) / 2,
                                    messageText->w,
                                    messageText->h
                                };
                                SDL_BlitSurface(messageText, NULL, newWindow, &textPos);
                                SDL_Flip(newWindow);
                                
                                SDL_Delay(isCorrect ? 3000 : 2000);
                                SDL_FreeSurface(messageText);
                            }
                            
                            // Return to main window
                            screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE);
                            if (screen) {
                                if (puzzleComplete) {
                                    SDL_BlitSurface(completePuzzleImage, NULL, screen, NULL);
                                } else {
                                    SDL_BlitSurface(imagePuzzle, NULL, screen, NULL);
                                    // Draw pieces
                                    for (int i = 0; i < 3; i++) {
                                        afficherImage(screen, propositions[i].image, propositions[i].position);
                                    }
                                }
                                SDL_Flip(screen);
                            }
                        }
                        pieceSelectionnee = -1;
                    }
                    break;
                    
                case SDL_MOUSEMOTION:
                    if (pieceSelectionnee != -1) {
                        propositions[pieceSelectionnee].position.x = 
                            event.motion.x - mouseOffsetX;
                        propositions[pieceSelectionnee].position.y = 
                            event.motion.y - mouseOffsetY;
                    }
                    break;
                    case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        // Check all pieces positions when Enter is pressed
                        for (int i = 0; i < 3; i++) {
                            if (verifierPosition(screen, &propositions[i])) {
                                puzzleComplete = 1;
                                if (successSound) {
                                    Mix_PlayChannel(-1, successSound, 0);
                                }
                                // Force screen update
                                SDL_Flip(screen);
                                break;
                            }
                        }
                    }
                    break;
            }
        }

        // Draw background
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
        
                // Draw puzzle with missing piece
                SDL_Rect puzzlePos = {0, 0, 0, 0};
// Replace the congratulation message section with this:
if (puzzleComplete) {
    // Draw complete puzzle
    afficherImage(screen, completePuzzleImage, puzzlePos);
    
    // Create larger font for congratulation message
    TTF_Font* largeFont = TTF_OpenFont("arial.ttf", 72); // Larger size font
    if (!largeFont) {
        fprintf(stderr, "Failed to load large font: %s\n", TTF_GetError());
    } else {
        // Draw larger congratulation message at bottom
        SDL_Color textColor = {0, 255, 0}; // Green color
        SDL_Surface* congratsText = TTF_RenderText_Solid(largeFont, "Congratulations!", textColor);
        if (congratsText) {
            // Create a background rectangle for the text
            SDL_Rect textBackground = {
                0,
                screenHeight - 150, // Position from bottom
                screenWidth,
                100
            };
            // Fill background with semi-transparent black
            SDL_FillRect(screen, &textBackground, SDL_MapRGB(screen->format, 0, 0, 0));
            
            // Center text in the background
            SDL_Rect textPos = {
                screenWidth/2 - congratsText->w/2,
                screenHeight - 125, // Adjust vertical position
                congratsText->w,
                congratsText->h
            };
            SDL_BlitSurface(congratsText, NULL, screen, &textPos);
            SDL_FreeSurface(congratsText);
        } else {
            fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        }
        TTF_CloseFont(largeFont); // Close the large font
    }
}
        else {
            // Draw puzzle with missing piece
            afficherImage(screen, imagePuzzle, puzzlePos);
        }

        // Draw pieces
        for (int i = 0; i < 3; i++) {
            afficherImage(screen, propositions[i].image, propositions[i].position);
        }

        // Update screen
        SDL_Flip(screen);
    }
    // Cleanup
    for (int i = 0; i < 3; i++) {
        if (propositions[i].image) {
            SDL_FreeSurface(propositions[i].image);
        }
    }
    if (successSound) {
        Mix_FreeChunk(successSound);
    }
    Mix_CloseAudio();
    
    if (imagePuzzle) {
        SDL_FreeSurface(imagePuzzle);
    }
    if (completePuzzleImage) {
        SDL_FreeSurface(completePuzzleImage);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;

}

