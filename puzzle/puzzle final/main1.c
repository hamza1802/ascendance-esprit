#include <stdio.h>
#include <stdlib.h>
#include "header1.h"

int main(int argc, char *argv[]) {
    const int screenWidth = 1440;
    const int screenHeight = 900;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Erreur d'initialisation de SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize: %s\n", Mix_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    if (TTF_Init() == -1) {
        fprintf(stderr, "Erreur d'initialisation de SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    TTF_Font* font = TTF_OpenFont("assets/arial.ttf", 36);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Initialize audio resources locally
    Mix_Chunk *successSound = Mix_LoadWAV("assets/explo.wav");
    if (!successSound) {
        fprintf(stderr, "Failed to load success sound: %s\n", Mix_GetError());
    }
    Mix_Chunk *failureSound = Mix_LoadWAV("assets/fail.wav");
    if (!failureSound) {
        fprintf(stderr, "Failed to load failure sound: %s\n", Mix_GetError());
    }
    Mix_Chunk *clickSound = Mix_LoadWAV("assets/clic.wav");
    if (!clickSound) {
        fprintf(stderr, "Failed to load click sound: %s\n", Mix_GetError());
    }
    Mix_Music *backgroundMusic = Mix_LoadMUS("assets/background.wav");
    if (!backgroundMusic) {
        fprintf(stderr, "Failed to load background music: %s\n", Mix_GetError());
    }
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }

    // Load countdown audio files (ten.wav to zero.wav)
    Mix_Chunk* countdownSounds[11]; // For 10 to 0
    const char* countdownFiles[11] = {
        "assets/ten.wav", "assets/nine.wav", "assets/eight.wav", "assets/seven.wav",
        "assets/six.wav", "assets/five.wav", "assets/four.wav", "assets/three.wav",
        "assets/two.wav", "assets/one.wav", "assets/zero.wav"
    };
    for (int i = 0; i < 11; i++) {
        countdownSounds[i] = Mix_LoadWAV(countdownFiles[i]);
        if (!countdownSounds[i]) {
            fprintf(stderr, "Failed to load countdown sound %s: %s\n", countdownFiles[i], Mix_GetError());
        }
    }

    SDL_Surface *welcomeScreen = SDL_SetVideoMode(WELCOME_WIDTH, WELCOME_HEIGHT, 32, SDL_HWSURFACE);
    if (!welcomeScreen) {
        fprintf(stderr, "Erreur de création de la fenêtre de bienvenue: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    int showWelcome = 1;
    int showRules = 0;
    while (showWelcome) {
        int welcomeResult = show_welcome_screen(welcomeScreen, clickSound);
        if (welcomeResult == 0) {
            TTF_CloseFont(font);
            Mix_CloseAudio();
            TTF_Quit();
            SDL_Quit();
            return EXIT_FAILURE;
        }
        showWelcome = 0;
        showRules = 1;

        while (showRules) {
            int rulesResult = show_rules_screen(welcomeScreen, clickSound);
            if (rulesResult == 0) {
                TTF_CloseFont(font);
                Mix_CloseAudio();
                TTF_Quit();
                SDL_Quit();
                return EXIT_FAILURE;
            } else if (rulesResult == 2) {
                showWelcome = 1;
                showRules = 0;
            } else {
                showRules = 0;
            }
        }
    }

    SDL_Surface *screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        fprintf(stderr, "Erreur de création de la fenêtre: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_WM_SetCaption("Puzzle Game", NULL);

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 230, 230, 230));
    TTF_Font* puzzleFont = TTF_OpenFont("assets/arial.ttf", 48);
    if (puzzleFont) {
        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* puzzleText = TTF_RenderText_Blended(puzzleFont, "Good luck!", textColor);
        if (puzzleText) {
            SDL_Surface* previewImage = chargerImage("assets/puzzle2_full.png");
            if (previewImage) {
                SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 230, 230, 230));
                SDL_Rect messagePos = {
                    (screenWidth - puzzleText->w) / 2,
                    50,
                    puzzleText->w,
                    puzzleText->h
                };
                SDL_BlitSurface(puzzleText, NULL, screen, &messagePos);
                SDL_Rect imagePos = {
                    PUZZLE_X,
                    PUZZLE_Y,
                    PUZZLE_WIDTH,
                    PUZZLE_HEIGHT
                };
                SDL_BlitSurface(previewImage, NULL, screen, &imagePos);
                SDL_Flip(screen);
                SDL_Delay(2000);
                SDL_FreeSurface(previewImage);
            }
            SDL_FreeSurface(puzzleText);
        }
        TTF_CloseFont(puzzleFont);
    }

    // Initialize puzzles array locally
    Puzzle puzzles[NUM_PUZZLES] = {
        {
            "assets/puzzle2_full.png",
            {
                "assets/puzzle2_piece0.png", "assets/puzzle2_piece1.png", "assets/puzzle2_piece2.png",
                "assets/puzzle2_piece3.png", "assets/puzzle2_piece4.png", "assets/puzzle2_piece5.png",
                "assets/puzzle2_piece6.png", "assets/puzzle2_piece7.png", "assets/puzzle2_piece8.png"
            }
        }
    };

    // Initialize game state
    GameState gameState = {0};
    for (int i = 0; i < NUM_PUZZLES; i++) {
        gameState.puzzles[i] = puzzles[i];
    }
    init_puzzle_order(&gameState);

    SDL_Surface *completePuzzleImage = NULL;
    Puzzle currentPuzzle = {0};
    load_puzzle(&currentPuzzle, &completePuzzleImage, &gameState);
    if (!completePuzzleImage) {
        fprintf(stderr, "Failed to initialize puzzle\n");
        TTF_CloseFont(font);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Surface *restartButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *restartButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *finishButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *finishButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(restartButtonNormal, NULL, SDL_MapRGB(screen->format, 100, 100, 100));
    SDL_FillRect(restartButtonHover, NULL, SDL_MapRGB(screen->format, 150, 150, 150));
    SDL_FillRect(finishButtonNormal, NULL, SDL_MapRGB(screen->format, 100, 100, 100));
    SDL_FillRect(finishButtonHover, NULL, SDL_MapRGB(screen->format, 150, 150, 150));

    SDL_Color buttonTextColor = {255, 255, 255};
    SDL_Surface *restartText = TTF_RenderText_Blended(font, "Restart", buttonTextColor);
    SDL_Surface *finishText = TTF_RenderText_Blended(font, "Finish", buttonTextColor);
    SDL_Rect restartTextPos = {10, 10, restartText->w, restartText->h};
    SDL_Rect finishTextPos = {10, 10, finishText->w, finishText->h};
    SDL_BlitSurface(restartText, NULL, restartButtonNormal, &restartTextPos);
    SDL_BlitSurface(restartText, NULL, restartButtonHover, &restartTextPos);
    SDL_BlitSurface(finishText, NULL, finishButtonNormal, &finishTextPos);
    SDL_BlitSurface(finishText, NULL, finishButtonHover, &finishTextPos);

    int pieceSelectionnee = -1;
    int mouseOffsetX = 0;
    int mouseOffsetY = 0;
    int continuer = 1;
    SDL_Event event;
    Uint32 startTime = SDL_GetTicks() / 1000;
    int mouseX = 0, mouseY = 0;
    int restartHovered = 0, finishHovered = 0;
    SDL_Rect puzzleArea = {PUZZLE_X, PUZZLE_Y, PUZZLE_WIDTH, PUZZLE_HEIGHT};
    SDL_Rect barPos = {20, 100, 0, 0}; // Below timer and score
    int lastCountdownSecond = -1; // Track the last second we played a countdown sound

    // Initialize bar timer
    BarTimer barTimer = {0};

    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    continuer = 0;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouseX = event.button.x;
                        mouseY = event.button.y;
                        fprintf(stderr, "Mouse down at x=%d, y=%d\n", mouseX, mouseY);
                        if (mouseX >= 50 && mouseX <= 200 && mouseY >= 800 && mouseY <= 850) {
                            fprintf(stderr, "Restart button clicked\n");
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            load_puzzle(&currentPuzzle, &completePuzzleImage, &gameState);
                            startTime = SDL_GetTicks() / 1000;
                            lastCountdownSecond = -1; // Reset countdown on restart
                            if (!completePuzzleImage) {
                                fprintf(stderr, "Failed to load puzzle\n");
                                continuer = 0;
                            }
                            continue;
                        }
                        if (mouseX >= 1240 && mouseX <= 1390 && mouseY >= 800 && mouseY <= 850) {
                            fprintf(stderr, "Finish button clicked\n");
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            int elapsedTime = (SDL_GetTicks() / 1000) - startTime;
                            // Close the main game window before showing the popup
                            if (screen) {
                                SDL_FreeSurface(screen);
                                screen = NULL;
                            }
                            if (check_puzzle_completion(&currentPuzzle)) {
                                SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                                if (newWindow) {
                                    showResultPopup(newWindow, 1, elapsedTime, successSound, failureSound);
                                    // Exit the game after showing the success (congrats) popup
                                    continuer = 0;
                                }
                            } else {
                                SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                                if (newWindow) {
                                    showResultPopup(newWindow, 0, 0, successSound, failureSound);
                                    // Exit the game after showing the failure popup
                                    continuer = 0;
                                }
                            }
                            continue;
                        }
                        for (int i = NUM_PIECES - 1; i >= 0; i--) {
                            if (currentPuzzle.pieces[i].image) {
                                SDL_Rect *pos = &currentPuzzle.pieces[i].position;
                                if (mouseX >= pos->x &&
                                    mouseX <= pos->x + pos->w &&
                                    mouseY >= pos->y &&
                                    mouseY <= pos->y + pos->h) {
                                    pieceSelectionnee = i;
                                    mouseOffsetX = mouseX - pos->x;
                                    mouseOffsetY = mouseY - pos->y;
                                    fprintf(stderr, "Selected piece %d at (%d, %d)\n", i, mouseX, mouseY);
                                    if (clickSound) {
                                        Mix_PlayChannel(-1, clickSound, 0);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT && pieceSelectionnee != -1) {
                        fprintf(stderr, "Mouse up, piece %d dropped at (%d, %d)\n", pieceSelectionnee,
                                currentPuzzle.pieces[pieceSelectionnee].position.x,
                                currentPuzzle.pieces[pieceSelectionnee].position.y);
                        pieceSelectionnee = -1;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    fprintf(stderr, "Mouse motion: x=%d, y=%d\n", mouseX, mouseY);
                    restartHovered = (mouseX >= 50 && mouseX <= 200 && mouseY >= 800 && mouseY <= 850);
                    finishHovered = (mouseX >= 1240 && mouseX <= 1390 && mouseY >= 800 && mouseY <= 850);
                    if (pieceSelectionnee != -1) {
                        currentPuzzle.pieces[pieceSelectionnee].position.x = mouseX - mouseOffsetX;
                        currentPuzzle.pieces[pieceSelectionnee].position.y = mouseY - mouseOffsetY;
                    }
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        fprintf(stderr, "Enter pressed, checking completion...\n");
                        int elapsedTime = (SDL_GetTicks() / 1000) - startTime;
                        // Close the main game window before showing the popup
                        if (screen) {
                            SDL_FreeSurface(screen);
                            screen = NULL;
                        }
                        if (check_puzzle_completion(&currentPuzzle)) {
                            SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                            if (newWindow) {
                                showResultPopup(newWindow, 1, elapsedTime, successSound, failureSound);
                                // Exit the game after showing the success (congrats) popup
                                continuer = 0;
                            }
                        } else {
                            SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                            if (newWindow) {
                                showResultPopup(newWindow, 0, 0, successSound, failureSound);
                                // Exit the game after showing the failure popup
                                continuer = 0;
                            }
                        }
                    }
                    break;
            }
        }

        if (!screen) {
            fprintf(stderr, "Screen surface lost\n");
            break;
        }

        // Clear the screen with the background color
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 230, 230, 230));

        // Draw puzzle pieces (non-selected first)
        for (int i = 0; i < NUM_PIECES; i++) {
            if (i != pieceSelectionnee && currentPuzzle.pieces[i].image) {
                afficherImage(screen, currentPuzzle.pieces[i].image, currentPuzzle.pieces[i].position);
            }
        }

        // Draw the selected piece last (without highlight)
        if (pieceSelectionnee != -1 && currentPuzzle.pieces[pieceSelectionnee].image) {
            afficherImage(screen, currentPuzzle.pieces[pieceSelectionnee].image, currentPuzzle.pieces[pieceSelectionnee].position);
        }

        // Draw UI elements
        int elapsedTime = (SDL_GetTicks() / 1000) - startTime;

        // Countdown audio between 80 and 90 seconds
        if (elapsedTime >= 80 && elapsedTime <= 90) {
            int countdown = 90 - elapsedTime; // 10 at 80s, 9 at 81s, ..., 0 at 90s
            int currentSecond = elapsedTime - 80; // 0 to 10
            if (currentSecond != lastCountdownSecond) {
                int soundIndex = 10 - countdown; // Map countdown (10 to 0) to array index (0 to 10)
                if (countdownSounds[soundIndex]) {
                    Mix_PlayChannel(-1, countdownSounds[soundIndex], 0);
                    fprintf(stderr, "Playing countdown sound %d at elapsedTime=%d\n", countdown, elapsedTime);
                }
                lastCountdownSecond = currentSecond;
            }
        }

        // Check if score reaches 100 to trigger failure
        int score = calculate_score(elapsedTime);
        if (score <= 100) {
            // Close the main game window before showing the popup
            if (screen) {
                SDL_FreeSurface(screen);
                screen = NULL;
            }
            SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
            if (newWindow) {
                showResultPopup(newWindow, 0, 0, successSound, failureSound);
                // Exit the game after showing the failure popup due to score
                continuer = 0;
            }
        }

        // Skip rendering if screen is NULL (after popup)
        if (!screen) {
            continue;
        }

        // Numeric timer
        char timerText[20];
        snprintf(timerText, sizeof(timerText), "Time: %ds", elapsedTime);
        SDL_Color timerColor = {64, 64, 64};
        SDL_Surface *timerSurface = TTF_RenderText_Blended(font, timerText, timerColor);
        if (timerSurface) {
            SDL_Rect timerPos = {20, 20, timerSurface->w, timerSurface->h};
            SDL_BlitSurface(timerSurface, NULL, screen, &timerPos);
            SDL_FreeSurface(timerSurface);
        }

        // Score
        char scoreText[20];
        snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
        SDL_Surface *scoreSurface = TTF_RenderText_Blended(font, scoreText, timerColor);
        if (scoreSurface) {
            SDL_Rect scorePos = {20, 60, scoreSurface->w, scoreSurface->h};
            SDL_BlitSurface(scoreSurface, NULL, screen, &scorePos);
            SDL_FreeSurface(scoreSurface);
        }

        // Bar timer
        displayTimerBar(screen, elapsedTime, &barPos, &barTimer);

        // Draw buttons
        SDL_Rect restartButtonPos = {50, 800, 150, 50};
        SDL_Rect finishButtonPos = {1240, 800, 150, 50};
        SDL_BlitSurface(restartHovered ? restartButtonHover : restartButtonNormal, NULL, screen, &restartButtonPos);
        SDL_BlitSurface(finishHovered ? finishButtonHover : finishButtonNormal, NULL, screen, &finishButtonPos);

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    // Cleanup
    for (int i = 0; i < NUM_PIECES; i++) {
        if (currentPuzzle.pieces[i].image) {
            SDL_FreeSurface(currentPuzzle.pieces[i].image);
            currentPuzzle.pieces[i].image = NULL;
        }
    }
    if (completePuzzleImage) {
        SDL_FreeSurface(completePuzzleImage);
        completePuzzleImage = NULL;
    }
    if (restartButtonNormal) SDL_FreeSurface(restartButtonNormal);
    if (restartButtonHover) SDL_FreeSurface(restartButtonHover);
    if (finishButtonNormal) SDL_FreeSurface(finishButtonNormal);
    if (finishButtonHover) SDL_FreeSurface(finishButtonHover);
    if (restartText) SDL_FreeSurface(restartText);
    if (finishText) SDL_FreeSurface(finishText);
    if (successSound) Mix_FreeChunk(successSound);
    if (failureSound) Mix_FreeChunk(failureSound);
    if (clickSound) Mix_FreeChunk(clickSound);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    // Free countdown sounds
    for (int i = 0; i < 11; i++) {
        if (countdownSounds[i]) Mix_FreeChunk(countdownSounds[i]);
    }
    if (font) TTF_CloseFont(font);
    // Cleanup bar timer images
    cleanup_bar_images(&barTimer);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
