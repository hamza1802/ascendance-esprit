#include "header1.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_rotozoom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SDL_Surface* chargerImage(const char *chemin) {
    SDL_Surface *image = IMG_Load(chemin);
    if (!image) {
        fprintf(stderr, "Erreur de chargement de l'image %s: %s\n", chemin, SDL_GetError());
        return NULL;
    }
    SDL_Surface *optimized = SDL_DisplayFormatAlpha(image);
    SDL_FreeSurface(image);
    if (!optimized) {
        fprintf(stderr, "Erreur d'optimisation de l'image %s: %s\n", chemin, SDL_GetError());
        return NULL;
    }
    return optimized;
}

void init_puzzle_order(GameStatePuzzle *state) {
    if (!state) {
        fprintf(stderr, "Invalid GameStatePuzzle pointer\n");
        return;
    }
    for (int i = 0; i < NUM_PUZZLES; i++) {
        state->puzzle_order[i] = i;
    }
    state->current_puzzle_index = 0;
}

void load_puzzle(Puzzle *puzzle, SDL_Surface **completeImage, GameStatePuzzle *state) {
    if (!puzzle || !completeImage || !state) {
        fprintf(stderr, "Invalid pointers: puzzle=%p, completeImage=%p, state=%p\n", puzzle, completeImage, state);
        return;
    }
    if (state->current_puzzle_index >= NUM_PUZZLES) {
        state->current_puzzle_index = 0;
    }
    int puzzle_idx = state->puzzle_order[state->current_puzzle_index];
    *puzzle = state->puzzles[puzzle_idx];
    
    if (*completeImage) {
        SDL_FreeSurface(*completeImage);
        *completeImage = NULL;
    }
    *completeImage = chargerImage(puzzle->full_image_path);
    if (!*completeImage) {
        fprintf(stderr, "Failed to load complete image: %s\n", puzzle->full_image_path);
        return;
    }
    
    int target_x[NUM_PIECES] = {0, 0, 0, 180, 180, 180, 360, 360, 360};
    int target_y[NUM_PIECES] = {0, 180, 360, 0, 180, 360, 0, 180, 360};
    
    for (int i = 0; i < NUM_PIECES; i++) {
        if (puzzle->pieces[i].image) {
            SDL_FreeSurface(puzzle->pieces[i].image);
            puzzle->pieces[i].image = NULL;
        }
        
        puzzle->pieces[i].target.x = PUZZLE_X + target_x[i];
        puzzle->pieces[i].target.y = PUZZLE_Y + target_y[i];
        puzzle->pieces[i].target.w = PIECE_WIDTH;
        puzzle->pieces[i].target.h = PIECE_HEIGHT;
        
        puzzle->pieces[i].image = chargerImage(puzzle->piece_paths[i]);
        if (!puzzle->pieces[i].image) {
            fprintf(stderr, "Failed to load piece %d: %s\n", i, puzzle->piece_paths[i]);
            puzzle->pieces[i].image = SDL_CreateRGBSurface(SDL_SWSURFACE, PIECE_WIDTH, PIECE_HEIGHT, 32, 0, 0, 0, 0);
            if (puzzle->pieces[i].image) {
                SDL_FillRect(puzzle->pieces[i].image, NULL, SDL_MapRGB(puzzle->pieces[i].image->format, 255, 0, 0));
            }
            continue;
        }
        printf("Piece %d loaded: w=%d, h=%d, target=(%d,%d)\n", 
               i, puzzle->pieces[i].image->w, puzzle->pieces[i].image->h,
               puzzle->pieces[i].target.x, puzzle->pieces[i].target.y);
        
        if (i == 0) {
            puzzle->pieces[i].position.x = puzzle->pieces[i].target.x;
            puzzle->pieces[i].position.y = puzzle->pieces[i].target.y;
            puzzle->pieces[i].isPlaced = 1;
        } else {
            puzzle->pieces[i].position.x = 100 + (rand() % (SCREEN_WIDTH - PIECE_WIDTH - 100));
            puzzle->pieces[i].position.y = 600 + (rand() % (SCREEN_HEIGHT - PIECE_HEIGHT - 600));
            puzzle->pieces[i].isPlaced = 0;
        }
        puzzle->pieces[i].position.w = PIECE_WIDTH;
        puzzle->pieces[i].position.h = PIECE_HEIGHT;
    }
    state->current_puzzle_index++;
}

void afficherImage(SDL_Surface *screen, SDL_Surface *image, SDL_Rect position) {
    if (screen && image) {
        SDL_BlitSurface(image, NULL, screen, &position);
    } else {
        fprintf(stderr, "Cannot blit: screen=%p, image=%p\n", screen, image);
    }
}

int calculate_score(int elapsedTime) {
    int intervals = elapsedTime / 15;
    int score = 1000 - intervals * 150;
    return score > 100 ? score : 100;
}

void showResultPopup(SDL_Surface *parentScreen, int isSuccess, int elapsedTime, Mix_Chunk *successSound, Mix_Chunk *failureSound) {
    if (!parentScreen) {
        fprintf(stderr, "Invalid parent screen\n");
        return;
    }
    char window_pos[32];
    snprintf(window_pos, sizeof(window_pos), "SDL_VIDEO_WINDOW_POS=%d,%d", 320, 150);
    SDL_putenv(window_pos);
    
    SDL_Surface *popup = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
    if (!popup) {
        fprintf(stderr, "Couldn't create popup window: %s\n", SDL_GetError());
        return;
    }
    SDL_Surface* bgImage = chargerImage(isSuccess ? "assets/popup.jpg" : "assets/failure.jpg");
    if (!bgImage) {
        fprintf(stderr, "Failed to load background image, using fallback\n");
        SDL_FillRect(popup, NULL, SDL_MapRGB(popup->format, 255, 255, 255));
    }

    if (isSuccess && successSound) {
        Mix_PlayChannel(-1, successSound, 0);
    } else if (!isSuccess && failureSound) {
        Mix_PlayChannel(-1, failureSound, 0);
    }

    TTF_Font* popupFont = TTF_OpenFont("assets/font.ttf", 36);
    SDL_Surface* text = NULL;
    SDL_Rect textPos;
    if (popupFont) {
        SDL_Color textColor = isSuccess ? (SDL_Color){139, 28, 98} : (SDL_Color){0, 0, 0};
        char message[100];
        if (isSuccess) {
            int score = calculate_score(elapsedTime);
            snprintf(message, sizeof(message), "Completed in %d seconds! Score: %d", elapsedTime, score);
        } else {
            snprintf(message, sizeof(message), "Incorrect Placement! Try Again!");
        }
        text = TTF_RenderText_Blended(popupFont, message, textColor);
        if (text) {
            textPos.x = (800 - text->w) / 2;
            textPos.y = (600 - text->h) / 2;
            textPos.w = text->w;
            textPos.h = text->h;
        }
    }

    SDL_Surface* starImage = NULL;
    if (isSuccess) {
        starImage = chargerImage("assets/star.png");
        if (!starImage) {
            fprintf(stderr, "Failed to load star image: assets/star.png\n");
            starImage = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 50, 32, 0, 0, 0, 0);
            if (starImage) {
                SDL_FillRect(starImage, NULL, SDL_MapRGB(starImage->format, 255, 0, 0));
            }
        }
    }

    if (isSuccess && starImage) {
        Uint32 startTime = SDL_GetTicks();
        Uint32 duration = 5000;
        double angle = 0.0;
        double scale = 1.0;
        int scaleDirection = 1;
        const double rotationSpeed = 360.0 / 5.0;
        const double scaleSpeed = 0.5 / 1000.0;

        while (SDL_GetTicks() - startTime < duration) {
            if (bgImage) {
                SDL_BlitSurface(bgImage, NULL, popup, NULL);
            } else {
                SDL_FillRect(popup, NULL, SDL_MapRGB(popup->format, 255, 255, 255));
            }

            Uint32 elapsed = SDL_GetTicks() - startTime;
            angle = (rotationSpeed * elapsed) / 1000.0;
            scale += scaleDirection * scaleSpeed * elapsed;
            if (scale >= 1.5) {
                scale = 1.5;
                scaleDirection = -1;
            } else if (scale <= 0.8) {
                scale = 0.8;
                scaleDirection = 1;
            }

            SDL_Surface* rotozoomedStar = rotozoomSurface(starImage, angle, scale, 0);
            if (rotozoomedStar) {
                SDL_Rect starPos1 = {
                    (800 - rotozoomedStar->w) / 2,
                    textPos.y - rotozoomedStar->h - 20,
                    rotozoomedStar->w,
                    rotozoomedStar->h
                };
                SDL_Rect starPos2 = {
                    (800 - rotozoomedStar->w) / 2,
                    textPos.y + textPos.h + 20,
                    rotozoomedStar->w,
                    rotozoomedStar->h
                };

                SDL_BlitSurface(rotozoomedStar, NULL, popup, &starPos1);
                SDL_BlitSurface(rotozoomedStar, NULL, popup, &starPos2);
                SDL_FreeSurface(rotozoomedStar);
            } else {
                fprintf(stderr, "Failed to rotozoom star image\n");
            }

            if (text) {
                SDL_BlitSurface(text, NULL, popup, &textPos);
            }

            SDL_Flip(popup);
            SDL_Delay(16);
        }
    } else {
        if (bgImage) {
            SDL_BlitSurface(bgImage, NULL, popup, NULL);
        } else {
            SDL_FillRect(popup, NULL, SDL_MapRGB(popup->format, 255, 255, 255));
        }
        if (text) {
            SDL_BlitSurface(text, NULL, popup, &textPos);
        }
        SDL_Flip(popup);
        if (isSuccess) {
            SDL_Delay(5000);
        } else {
            SDL_Delay(1000);
        }
    }

    if (text) {
        SDL_FreeSurface(text);
    }
    if (starImage) {
        SDL_FreeSurface(starImage);
    }
    if (bgImage) {
        SDL_FreeSurface(bgImage);
    }
    if (popupFont) {
        TTF_CloseFont(popupFont);
    }
}

void draw_piece_highlight(SDL_Surface *screen, SDL_Rect *position) {
    if (screen && position) {
        SDL_Rect outline = {position->x - 2, position->y - 2, position->w + 4, position->h + 4};
        SDL_FillRect(screen, &outline, SDL_MapRGB(screen->format, 0, 255, 0));
    }
}

int check_puzzle_completion(Puzzle *puzzle) {
    if (!puzzle) return 0;
    
    int ref_x = puzzle->pieces[0].position.x;
    int ref_y = puzzle->pieces[0].position.y;
    
    int expected_x[NUM_PIECES] = {0, 0, 0, 180, 180, 180, 360, 360, 360};
    int expected_y[NUM_PIECES] = {0, 180, 360, 0, 180, 360, 0, 180, 360};
    int tolerance = 50;
    
    int isComplete = 1;
    for (int i = 0; i < NUM_PIECES; i++) {
        int expected_pos_x = ref_x + expected_x[i];
        int expected_pos_y = ref_y + expected_y[i];
        int xDiff = abs(puzzle->pieces[i].position.x - expected_pos_x);
        int yDiff = abs(puzzle->pieces[i].position.y - expected_pos_y);
        fprintf(stderr, "Piece %d: pos=(%d,%d), expected=(%d,%d), xDiff=%d, yDiff=%d\n",
                i, puzzle->pieces[i].position.x, puzzle->pieces[i].position.y,
                expected_pos_x, expected_pos_y, xDiff, yDiff);
        if (xDiff > tolerance || yDiff > tolerance) {
            isComplete = 0;
        }
    }
    
    return isComplete;
}

int show_welcome_screen(SDL_Surface *screen, Mix_Chunk *clickSound) {
    SDL_Surface *bg = chargerImage("assets/background.png");
    SDL_Surface *scaled_bg = NULL;
    if (bg) {
        printf("Welcome background loaded: w=%d, h=%d\n", bg->w, bg->h);
        double scaleX = (double)WELCOME_WIDTH / bg->w;
        double scaleY = (double)WELCOME_HEIGHT / bg->h;
        scaled_bg = rotozoomSurfaceXY(bg, 0.0, scaleX, scaleY, 0);
        SDL_FreeSurface(bg);
        if (!scaled_bg) {
            fprintf(stderr, "Failed to scale welcome background: %s\n", SDL_GetError());
        }
    } else {
        fprintf(stderr, "Failed to load welcome background: assets/background.png\n");
    }
    if (!scaled_bg) {
        scaled_bg = SDL_CreateRGBSurface(SDL_SWSURFACE, WELCOME_WIDTH, WELCOME_HEIGHT, 32, 0, 0, 0, 0);
        SDL_FillRect(scaled_bg, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
    }
    
    TTF_Font *font = TTF_OpenFont("assets/font.ttf", 36);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        SDL_FreeSurface(scaled_bg);
        return 0;
    }
    
    SDL_Surface *nextButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *nextButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(nextButtonNormal, NULL, SDL_MapRGB(screen->format, 100, 100, 100));
    SDL_FillRect(nextButtonHover, NULL, SDL_MapRGB(screen->format, 150, 150, 150));
    
    SDL_Surface *backButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *backButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(backButtonNormal, NULL, SDL_MapRGB(screen->format, 100, 100, 100));
    SDL_FillRect(backButtonHover, NULL, SDL_MapRGB(screen->format, 150, 150, 150));
    
    SDL_Color textColor = {255, 255, 255};
    SDL_Surface *nextButtonText = TTF_RenderText_Blended(font, "Next", textColor);
    SDL_Surface *backButtonText = TTF_RenderText_Blended(font, "Back", textColor);
    SDL_Rect nextTextPos = {10, 10, nextButtonText->w, nextButtonText->h};
    SDL_Rect backTextPos = {10, 10, backButtonText->w, backButtonText->h};
    SDL_BlitSurface(nextButtonText, NULL, nextButtonNormal, &nextTextPos);
    SDL_BlitSurface(nextButtonText, NULL, nextButtonHover, &nextTextPos);
    SDL_BlitSurface(backButtonText, NULL, backButtonNormal, &backTextPos);
    SDL_BlitSurface(backButtonText, NULL, backButtonHover, &backTextPos);
    
    SDL_Color msgColor = {255, 255, 255};
    const char *message = "You're about to solve a fun puzzle! Fit the pieces together to complete the image.";
    SDL_Surface *text = TTF_RenderText_Blended(font, message, msgColor);
    
    int continuer = 1;
    SDL_Event event;
    int mouseX = 0, mouseY = 0;
    int nextButtonHovered = 0, backButtonHovered = 0;
    
    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_FreeSurface(scaled_bg);
                    SDL_FreeSurface(nextButtonNormal);
                    SDL_FreeSurface(nextButtonHover);
                    SDL_FreeSurface(backButtonNormal);
                    SDL_FreeSurface(backButtonHover);
                    SDL_FreeSurface(nextButtonText);
                    SDL_FreeSurface(backButtonText);
                    SDL_FreeSurface(text);
                    TTF_CloseFont(font);
                    return 0;
                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    nextButtonHovered = (mouseX >= 625 && mouseX <= 775 && mouseY >= 600 && mouseY <= 650); // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
                    backButtonHovered = (mouseX >= 425 && mouseX <= 575 && mouseY >= 600 && mouseY <= 650); // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (mouseX >= 625 && mouseX <= 775 && mouseY >= 600 && mouseY <= 650) {
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            continuer = 0;
                        } else if (mouseX >= 425 && mouseX <= 575 && mouseY >= 600 && mouseY <= 650) {
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            SDL_FreeSurface(scaled_bg);
                            SDL_FreeSurface(nextButtonNormal);
                            SDL_FreeSurface(nextButtonHover);
                            SDL_FreeSurface(backButtonNormal);
                            SDL_FreeSurface(backButtonHover);
                            SDL_FreeSurface(nextButtonText);
                            SDL_FreeSurface(backButtonText);
                            SDL_FreeSurface(text);
                            TTF_CloseFont(font);
                            return 0;
                        }
                    }
                    break;
            }
        }
        
        SDL_BlitSurface(scaled_bg, NULL, screen, NULL);
        if (text) {
            SDL_Rect textPos = {(WELCOME_WIDTH - text->w) / 2, 250, text->w, text->h}; // Adjusted y-position for new WELCOME_HEIGHT
            SDL_BlitSurface(text, NULL, screen, &textPos);
        }
        SDL_Rect nextButtonPos = {625, 600, 150, 50}; // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
        SDL_Rect backButtonPos = {425, 600, 150, 50}; // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
        SDL_BlitSurface(nextButtonHovered ? nextButtonHover : nextButtonNormal, NULL, screen, &nextButtonPos);
        SDL_BlitSurface(backButtonHovered ? backButtonHover : backButtonNormal, NULL, screen, &backButtonPos);
        SDL_Flip(screen);
    }
    
    SDL_FreeSurface(scaled_bg);
    SDL_FreeSurface(nextButtonNormal);
    SDL_FreeSurface(nextButtonHover);
    SDL_FreeSurface(backButtonNormal);
    SDL_FreeSurface(backButtonHover);
    SDL_FreeSurface(nextButtonText);
    SDL_FreeSurface(backButtonText);
    SDL_FreeSurface(text);
    TTF_CloseFont(font);
    return 1;
}

int show_rules_screen(SDL_Surface *screen, Mix_Chunk *clickSound) {
    SDL_Surface *bg = chargerImage("assets/background.png");
    SDL_Surface *scaled_bg = NULL;
    if (bg) {
        printf("Rules background loaded: w=%d, h=%d\n", bg->w, bg->h);
        double scaleX = (double)WELCOME_WIDTH / bg->w;
        double scaleY = (double)WELCOME_HEIGHT / bg->h;
        scaled_bg = rotozoomSurfaceXY(bg, 0.0, scaleX, scaleY, 0);
        SDL_FreeSurface(bg);
        if (!scaled_bg) {
            fprintf(stderr, "Failed to scale rules background: %s\n", SDL_GetError());
        }
    } else {
        fprintf(stderr, "Failed to load rules background: assets/background.png\n");
    }
    if (!scaled_bg) {
        scaled_bg = SDL_CreateRGBSurface(SDL_SWSURFACE, WELCOME_WIDTH, WELCOME_HEIGHT, 32, 0, 0, 0, 0);
        SDL_FillRect(scaled_bg, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
    }
    
    TTF_Font *font = TTF_OpenFont("assets/font.ttf", 36);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        SDL_FreeSurface(scaled_bg);
        return 0;
    }
    
    SDL_Surface *playButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *playButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(playButtonNormal, NULL, SDL_MapRGB(screen->format, 100, 100, 100));
    SDL_FillRect(playButtonHover, NULL, SDL_MapRGB(screen->format, 150, 150, 150));
    
    SDL_Surface *backButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *backButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(backButtonNormal, NULL, SDL_MapRGB(screen->format, 100, 100, 100));
    SDL_FillRect(backButtonHover, NULL, SDL_MapRGB(screen->format, 150, 150, 150));
    
    SDL_Color textColor = {255, 255, 255};
    SDL_Surface *playButtonText = TTF_RenderText_Blended(font, "Play", textColor);
    SDL_Surface *backButtonText = TTF_RenderText_Blended(font, "Back", textColor);
    SDL_Rect playTextPos = {10, 10, playButtonText->w, playButtonText->h};
    SDL_Rect backTextPos = {10, 10, backButtonText->w, backButtonText->h};
    SDL_BlitSurface(playButtonText, NULL, playButtonNormal, &playTextPos);
    SDL_BlitSurface(playButtonText, NULL, playButtonHover, &playTextPos);
    SDL_BlitSurface(backButtonText, NULL, backButtonNormal, &backTextPos);
    SDL_BlitSurface(backButtonText, NULL, backButtonHover, &backTextPos);
    
    SDL_Color msgColor = {255, 255, 255};
    const char *message = "The puzzle is timed! Solve it as fast as possible to earn a higher score.";
    SDL_Surface *text = TTF_RenderText_Blended(font, message, msgColor);
    
    int continuer = 1;
    SDL_Event event;
    int mouseX = 0, mouseY = 0;
    int playButtonHovered = 0, backButtonHovered = 0;
    
    while (continuer) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_FreeSurface(scaled_bg);
                    SDL_FreeSurface(playButtonNormal);
                    SDL_FreeSurface(playButtonHover);
                    SDL_FreeSurface(backButtonNormal);
                    SDL_FreeSurface(backButtonHover);
                    SDL_FreeSurface(playButtonText);
                    SDL_FreeSurface(backButtonText);
                    SDL_FreeSurface(text);
                    TTF_CloseFont(font);
                    return 0;
                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    playButtonHovered = (mouseX >= 625 && mouseX <= 775 && mouseY >= 600 && mouseY <= 650); // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
                    backButtonHovered = (mouseX >= 425 && mouseX <= 575 && mouseY >= 600 && mouseY <= 650); // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (mouseX >= 625 && mouseX <= 775 && mouseY >= 600 && mouseY <= 650) {
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            continuer = 0;
                        } else if (mouseX >= 425 && mouseX <= 575 && mouseY >= 600 && mouseY <= 650) {
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            SDL_FreeSurface(scaled_bg);
                            SDL_FreeSurface(playButtonNormal);
                            SDL_FreeSurface(playButtonHover);
                            SDL_FreeSurface(backButtonNormal);
                            SDL_FreeSurface(backButtonHover);
                            SDL_FreeSurface(playButtonText);
                            SDL_FreeSurface(backButtonText);
                            SDL_FreeSurface(text);
                            TTF_CloseFont(font);
                            return 2;
                        }
                    }
                    break;
            }
        }
        
        SDL_BlitSurface(scaled_bg, NULL, screen, NULL);
        if (text) {
            SDL_Rect textPos = {(WELCOME_WIDTH - text->w) / 2, 250, text->w, text->h}; // Adjusted y-position for new WELCOME_HEIGHT
            SDL_BlitSurface(text, NULL, screen, &textPos);
        }
        SDL_Rect playButtonPos = {625, 600, 150, 50}; // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
        SDL_Rect backButtonPos = {425, 600, 150, 50}; // Adjusted for new WELCOME_WIDTH and WELCOME_HEIGHT
        SDL_BlitSurface(playButtonHovered ? playButtonHover : playButtonNormal, NULL, screen, &playButtonPos);
        SDL_BlitSurface(backButtonHovered ? backButtonHover : backButtonNormal, NULL, screen, &backButtonPos);
        SDL_Flip(screen);
    }
    
    SDL_FreeSurface(scaled_bg);
    SDL_FreeSurface(playButtonNormal);
    SDL_FreeSurface(playButtonHover);
    SDL_FreeSurface(backButtonNormal);
    SDL_FreeSurface(backButtonHover);
    SDL_FreeSurface(playButtonText);
    SDL_FreeSurface(backButtonText);
    SDL_FreeSurface(text);
    TTF_CloseFont(font);
    return 1;
}

void displayTimerBar(SDL_Surface *screen, int elapsedTime, SDL_Rect *barPos, BarTimer *barTimer) {
    if (!screen || !barPos || !barTimer) return;

    if (!barTimer->barInitialized) {
        char path[32];
        for (int i = 0; i < NUM_BAR_LEVELS; i++) {
            snprintf(path, sizeof(path), "assets/bar_%d.png", i);
            barTimer->barImages[i] = chargerImage(path);
            if (!barTimer->barImages[i]) {
                fprintf(stderr, "Failed to load bar image %s\n", path);
                barTimer->barImages[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 200, 20, 32, 0, 0, 0, 0);
                if (barTimer->barImages[i]) {
                    SDL_FillRect(barTimer->barImages[i], NULL, SDL_MapRGB(screen->format, 255, 0, 0));
                }
            }
        }
        barTimer->barInitialized = 1;
    }

    int level = elapsedTime / 15;
    if (level >= NUM_BAR_LEVELS) {
        level = NUM_BAR_LEVELS - 1;
    }

    if (barTimer->barImages[level]) {
        SDL_BlitSurface(barTimer->barImages[level], NULL, screen, barPos);
    }
}

void cleanup_bar_images(BarTimer *barTimer) {
    if (!barTimer) return;
    for (int i = 0; i < NUM_BAR_LEVELS; i++) {
        if (barTimer->barImages[i]) {
            SDL_FreeSurface(barTimer->barImages[i]);
            barTimer->barImages[i] = NULL;
        }
    }
    barTimer->barInitialized = 0;
}

void snap_piece(Piece *piece) {
    // Empty function as per original code
}

void run_puzzle_game(SDL_Surface *screen, TTF_Font *font, Mix_Chunk *clickSound, Mix_Chunk *successSound, Mix_Chunk *failureSound, Mix_Chunk *countdownSounds[], Mix_Music *bgMusic) {
    // Show welcome screen
    SDL_Surface *welcomeScreen = SDL_SetVideoMode(WELCOME_WIDTH, WELCOME_HEIGHT, 32, SDL_HWSURFACE);
    if (!welcomeScreen) {
        fprintf(stderr, "Erreur de création de la fenêtre de bienvenue: %s\n", SDL_GetError());
        return;
    }
    
    int showWelcome = 1;
    int showRules = 0;
    while (showWelcome) {
        int welcomeResult = show_welcome_screen(welcomeScreen, clickSound);
        if (welcomeResult == 0) {
            return;
        }
        showWelcome = 0;
        showRules = 1;

        while (showRules) {
            int rulesResult = show_rules_screen(welcomeScreen, clickSound);
            if (rulesResult == 0) {
                return;
            } else if (rulesResult == 2) {
                showWelcome = 1;
                showRules = 0;
            } else {
                showRules = 0;
            }
        }
    }

    // Initialize puzzle game
    SDL_Surface *gameScreen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!gameScreen) {
        fprintf(stderr, "Erreur de création de la fenêtre: %s\n", SDL_GetError());
        return;
    }
    SDL_WM_SetCaption("Puzzle Game", NULL);

    SDL_FillRect(gameScreen, NULL, SDL_MapRGB(gameScreen->format, 230, 230, 230));
    TTF_Font* puzzleFont = TTF_OpenFont("assets/font.ttf", 48);
    if (puzzleFont) {
        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* puzzleText = TTF_RenderText_Blended(puzzleFont, "Good luck!", textColor);
        if (puzzleText) {
            SDL_Surface* previewImage = chargerImage(puzzles[0].full_image_path);
            if (previewImage) {
                SDL_FillRect(gameScreen, NULL, SDL_MapRGB(gameScreen->format, 230, 230, 230));
                SDL_Rect messagePos = {
                    (SCREEN_WIDTH - puzzleText->w) / 2,
                    50,
                    puzzleText->w,
                    puzzleText->h
                };
                SDL_BlitSurface(puzzleText, NULL, gameScreen, &messagePos);
                SDL_Rect imagePos = {PREVIEW_X, PREVIEW_Y, PREVIEW_WIDTH, PREVIEW_HEIGHT};
                SDL_Surface* scaledPreview = rotozoomSurfaceXY(previewImage, 0, (double)PREVIEW_WIDTH / previewImage->w, (double)PREVIEW_HEIGHT / previewImage->h, 0);
                if (scaledPreview) {
                    SDL_BlitSurface(scaledPreview, NULL, gameScreen, &imagePos);
                    SDL_FreeSurface(scaledPreview);
                }
                SDL_Flip(gameScreen);
                SDL_Delay(2000);
                SDL_FreeSurface(previewImage);
            }
            SDL_FreeSurface(puzzleText);
        }
        TTF_CloseFont(puzzleFont);
    }

    // Play background music
    if (bgMusic) {
        Mix_PlayMusic(bgMusic, -1); // -1 means loop indefinitely
    } else {
        fprintf(stderr, "Background music not loaded\n");
    }

    GameStatePuzzle state;
    memcpy(state.puzzles, puzzles, sizeof(Puzzle) * NUM_PUZZLES);
    init_puzzle_order(&state);
    SDL_Surface *completeImage = NULL;
    Puzzle currentPuzzle;
    load_puzzle(&currentPuzzle, &completeImage, &state);

    SDL_Surface *finishButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *finishButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(finishButtonNormal, NULL, SDL_MapRGB(gameScreen->format, 100, 100, 100));
    SDL_FillRect(finishButtonHover, NULL, SDL_MapRGB(gameScreen->format, 150, 150, 150));

    SDL_Surface *restartButtonNormal = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_Surface *restartButtonHover = SDL_CreateRGBSurface(SDL_SWSURFACE, 150, 50, 32, 0, 0, 0, 0);
    SDL_FillRect(restartButtonNormal, NULL, SDL_MapRGB(gameScreen->format, 100, 100, 100));
    SDL_FillRect(restartButtonHover, NULL, SDL_MapRGB(gameScreen->format, 150, 150, 150));

    TTF_Font *fontButton = TTF_OpenFont("assets/font.ttf", 24);
    SDL_Surface *finishButtonText = NULL;
    SDL_Surface *restartButtonText = NULL;
    if (fontButton) {
        SDL_Color textColor = {255, 255, 255};
        finishButtonText = TTF_RenderText_Blended(fontButton, "Finish", textColor);
        restartButtonText = TTF_RenderText_Blended(fontButton, "Restart", textColor);
        if (finishButtonText) {
            SDL_Rect textPos = {10, 10, finishButtonText->w, finishButtonText->h};
            SDL_BlitSurface(finishButtonText, NULL, finishButtonNormal, &textPos);
            SDL_BlitSurface(finishButtonText, NULL, finishButtonHover, &textPos);
        }
        if (restartButtonText) {
            SDL_Rect textPos = {10, 10, restartButtonText->w, restartButtonText->h};
            SDL_BlitSurface(restartButtonText, NULL, restartButtonNormal, &textPos);
            SDL_BlitSurface(restartButtonText, NULL, restartButtonHover, &textPos);
        }
        TTF_CloseFont(fontButton);
    }

    int finishButtonHovered = 0;
    int restartButtonHovered = 0;
    int dragging = 0;
    int selectedPiece = -1;
    int offsetX = 0, offsetY = 0;
    int score = 1000;
    Uint32 startTime = SDL_GetTicks();
    BarTimer barTimer = { .barInitialized = 0 };
    SDL_Rect barPos = {10, 10, 200, 20};
    int continuer = 1;
    int lastCountdownPlayed = -1;

    while (continuer) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    continuer = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mouseX = event.button.x;
                        int mouseY = event.button.y;
                        if (mouseX >= PREVIEW_X && mouseX <= PREVIEW_X + 150 && mouseY >= SCREEN_HEIGHT - 100 && mouseY <= SCREEN_HEIGHT - 50) {
                            if (check_puzzle_completion(&currentPuzzle)) {
                                int elapsedTime = (SDL_GetTicks() - startTime) / 1000;
                                score = calculate_score(elapsedTime);
                                if (gameScreen) {
                                    SDL_FreeSurface(gameScreen);
                                    gameScreen = NULL;
                                }
                                SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                                if (newWindow) {
                                    showResultPopup(newWindow, 1, elapsedTime, successSound, failureSound);
                                    continuer = 0;
                                }
                            } else {
                                if (gameScreen) {
                                    SDL_FreeSurface(gameScreen);
                                    gameScreen = NULL;
                                }
                                SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                                if (newWindow) {
                                    showResultPopup(newWindow, 0, 0, successSound, failureSound);
                                    continuer = 0;
                                }
                            }
                        }
                        else if (mouseX >= PREVIEW_X + 160 && mouseX <= PREVIEW_X + 310 && mouseY >= SCREEN_HEIGHT - 100 && mouseY <= SCREEN_HEIGHT - 50) {
                            load_puzzle(&currentPuzzle, &completeImage, &state);
                            startTime = SDL_GetTicks();
                            score = 1000;
                            lastCountdownPlayed = -1;
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                        }
                        else {
                            for (int i = NUM_PIECES - 1; i >= 0; i--) {
                                if (!currentPuzzle.pieces[i].isPlaced &&
                                    mouseX >= currentPuzzle.pieces[i].position.x &&
                                    mouseX <= currentPuzzle.pieces[i].position.x + currentPuzzle.pieces[i].position.w &&
                                    mouseY >= currentPuzzle.pieces[i].position.y &&
                                    mouseY <= currentPuzzle.pieces[i].position.y + currentPuzzle.pieces[i].position.h) {
                                    selectedPiece = i;
                                    dragging = 1;
                                    offsetX = mouseX - currentPuzzle.pieces[i].position.x;
                                    offsetY = mouseY - currentPuzzle.pieces[i].position.y;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (dragging && selectedPiece != -1) {
                            snap_piece(&currentPuzzle.pieces[selectedPiece]);
                        }
                        dragging = 0;
                        selectedPiece = -1;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (dragging && selectedPiece != -1) {
                        currentPuzzle.pieces[selectedPiece].position.x = event.motion.x - offsetX;
                        currentPuzzle.pieces[selectedPiece].position.y = event.motion.y - offsetY;
                    }
                    finishButtonHovered = (event.motion.x >= PREVIEW_X && event.motion.x <= PREVIEW_X + 150 &&
                                           event.motion.y >= SCREEN_HEIGHT - 100 && event.motion.y <= SCREEN_HEIGHT - 50);
                    restartButtonHovered = (event.motion.x >= PREVIEW_X + 160 && event.motion.x <= PREVIEW_X + 310 &&
                                            event.motion.y >= SCREEN_HEIGHT - 100 && event.motion.y <= SCREEN_HEIGHT - 50);
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        if (check_puzzle_completion(&currentPuzzle)) {
                            int elapsedTime = (SDL_GetTicks() - startTime) / 1000;
                            score = calculate_score(elapsedTime);
                            if (gameScreen) {
                                SDL_FreeSurface(gameScreen);
                                gameScreen = NULL;
                            }
                            SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                            if (newWindow) {
                                showResultPopup(newWindow, 1, elapsedTime, successSound, failureSound);
                                continuer = 0;
                            }
                        } else {
                            if (gameScreen) {
                                SDL_FreeSurface(gameScreen);
                                gameScreen = NULL;
                            }
                            SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
                            if (newWindow) {
                                showResultPopup(newWindow, 0, 0, successSound, failureSound);
                                continuer = 0;
                            }
                        }
                    }
                    break;
            }
        }

        int elapsedTime = (SDL_GetTicks() - startTime) / 1000;
        if (elapsedTime >= TOTAL_TIME - 10 && elapsedTime < TOTAL_TIME) {
            int countdown = TOTAL_TIME - elapsedTime;
            int soundIndex = 10 - countdown;
            if (countdown <= 10 && countdown >= 0 && countdownSounds[soundIndex] && lastCountdownPlayed != countdown) {
                Mix_PlayChannel(-1, countdownSounds[soundIndex], 0);
                lastCountdownPlayed = countdown;
            }
        }

        score = calculate_score(elapsedTime);
        if (score <= 100) {
            if (gameScreen) {
                SDL_FreeSurface(gameScreen);
                gameScreen = NULL;
            }
            SDL_Surface *newWindow = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
            if (newWindow) {
                showResultPopup(newWindow, 0, 0, successSound, failureSound);
                continuer = 0;
            }
        }

        SDL_FillRect(gameScreen, NULL, SDL_MapRGB(gameScreen->format, 230, 230, 230));
        for (int i = 0; i < NUM_PIECES; i++) {
            if (currentPuzzle.pieces[i].image) {
                afficherImage(gameScreen, currentPuzzle.pieces[i].image, currentPuzzle.pieces[i].position);
            }
        }

        displayTimerBar(gameScreen, elapsedTime, &barPos, &barTimer);

        int remainingTime = TOTAL_TIME - elapsedTime;
        if (remainingTime < 0) remainingTime = 0;
        char timeText[32];
        snprintf(timeText, sizeof(timeText), "Time: %d", remainingTime);
        TTF_Font *fontTime = TTF_OpenFont("assets/font.ttf", 24);
        if (fontTime) {
            SDL_Color textColor = {0, 0, 0};
            SDL_Surface *timeSurface = TTF_RenderText_Blended(fontTime, timeText, textColor);
            if (timeSurface) {
                SDL_Rect timePos = {barPos.x + barPos.w + 10, barPos.y, timeSurface->w, timeSurface->h};
                SDL_BlitSurface(timeSurface, NULL, gameScreen, &timePos);
                SDL_FreeSurface(timeSurface);
            }
            TTF_CloseFont(fontTime);
        }

        SDL_Rect finishButtonPos = {PREVIEW_X, SCREEN_HEIGHT - 100, 150, 50};
        SDL_Rect restartButtonPos = {PREVIEW_X + 160, SCREEN_HEIGHT - 100, 150, 50};
        SDL_BlitSurface(finishButtonHovered ? finishButtonHover : finishButtonNormal, NULL, gameScreen, &finishButtonPos);
        SDL_BlitSurface(restartButtonHovered ? restartButtonHover : restartButtonNormal, NULL, gameScreen, &restartButtonPos);

        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
        TTF_Font *fontScore = TTF_OpenFont("assets/font.ttf", 24);
        if (fontScore) {
            SDL_Color textColor = {0, 0, 0};
            SDL_Surface *scoreSurface = TTF_RenderText_Blended(fontScore, scoreText, textColor);
            if (scoreSurface) {
                SDL_Rect scorePos = {SCREEN_WIDTH - 150, 10, scoreSurface->w, scoreSurface->h}; // Adjusted for new SCREEN_WIDTH
                SDL_BlitSurface(scoreSurface, NULL, gameScreen, &scorePos);
                SDL_FreeSurface(scoreSurface);
            }
            TTF_CloseFont(fontScore);
        }

        SDL_Flip(gameScreen);
        SDL_Delay(16);
    }

    // Stop background music when the game ends
    Mix_HaltMusic();

    cleanup_bar_images(&barTimer);
    if (completeImage) {
        SDL_FreeSurface(completeImage);
    }
    if (finishButtonNormal) {
        SDL_FreeSurface(finishButtonNormal);
    }
    if (finishButtonHover) {
        SDL_FreeSurface(finishButtonHover);
    }
    if (finishButtonText) {
        SDL_FreeSurface(finishButtonText);
    }
    if (restartButtonNormal) {
        SDL_FreeSurface(restartButtonNormal);
    }
    if (restartButtonHover) {
        SDL_FreeSurface(restartButtonHover);
    }
    if (restartButtonText) {
        SDL_FreeSurface(restartButtonText);
    }
    for (int i = 0; i < NUM_PIECES; i++) {
        if (currentPuzzle.pieces[i].image) {
            SDL_FreeSurface(currentPuzzle.pieces[i].image);
        }
    }
}
