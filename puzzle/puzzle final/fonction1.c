#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "header1.h"

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

void init_puzzle_order(GameState *state) {
    if (!state) {
        fprintf(stderr, "Invalid GameState pointer\n");
        return;
    }
    for (int i = 0; i < NUM_PUZZLES; i++) {
        state->puzzle_order[i] = i;
    }
    state->current_puzzle_index = 0;
}

void load_puzzle(Puzzle *puzzle, SDL_Surface **completeImage, GameState *state) {
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
            puzzle->pieces[i].position.y = 650 + (rand() % (SCREEN_HEIGHT - PIECE_HEIGHT - 650));
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
    // New score calculation: decrease by 150 every 15 seconds, minimum 100
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

    TTF_Font* popupFont = TTF_OpenFont("assets/arial.ttf", 36);
    SDL_Surface* text = NULL;
    SDL_Rect textPos;
    if (popupFont) {
        SDL_Color textColor = isSuccess ? (SDL_Color){139, 28, 98} : (SDL_Color){0, 0, 0};
        char message[100];
        if (isSuccess) {
            int score = calculate_score(elapsedTime);
            snprintf(message, sizeof(message), "Completed in %d seconds! Score: %d", elapsedTime, score);
        } else {
            snprintf(message, sizeof(message), "Game over");
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
            SDL_Delay(5000); // 5 seconds for success
        } else {
            SDL_Delay(3000); // 3 second for failure
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
    
    TTF_Font *font = TTF_OpenFont("assets/arial.ttf", 36);
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
                    nextButtonHovered = (mouseX >= 525 && mouseX <= 675 && mouseY >= 600 && mouseY <= 650);
                    backButtonHovered = (mouseX >= 325 && mouseX <= 475 && mouseY >= 600 && mouseY <= 650);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (mouseX >= 525 && mouseX <= 675 && mouseY >= 600 && mouseY <= 650) {
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            continuer = 0;
                        } else if (mouseX >= 325 && mouseX <= 475 && mouseY >= 600 && mouseY <= 650) {
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
            SDL_Rect textPos = {(WELCOME_WIDTH - text->w) / 2, 250, text->w, text->h};
            SDL_BlitSurface(text, NULL, screen, &textPos);
        }
        SDL_Rect nextButtonPos = {525, 600, 150, 50};
        SDL_Rect backButtonPos = {325, 600, 150, 50};
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
    
    TTF_Font *font = TTF_OpenFont("assets/arial.ttf", 36);
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
                    playButtonHovered = (mouseX >= 525 && mouseX <= 675 && mouseY >= 600 && mouseY <= 650);
                    backButtonHovered = (mouseX >= 325 && mouseX <= 475 && mouseY >= 600 && mouseY <= 650);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (mouseX >= 525 && mouseX <= 675 && mouseY >= 600 && mouseY <= 650) {
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            continuer = 0;
                        } else if (mouseX >= 325 && mouseX <= 475 && mouseY >= 600 && mouseY <= 650) {
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
            SDL_Rect textPos = {(WELCOME_WIDTH - text->w) / 2, 250, text->w, text->h};
            SDL_BlitSurface(text, NULL, screen, &textPos);
        }
        SDL_Rect playButtonPos = {525, 600, 150, 50};
        SDL_Rect backButtonPos = {325, 600, 150, 50};
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

    // Initialize bar images if not already done
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

    // Calculate which bar image to display (update every 15 seconds)
    int level = elapsedTime / 15; // Changed from 10 to 15 seconds
    if (level >= NUM_BAR_LEVELS) {
        level = NUM_BAR_LEVELS - 1;
    }

    // Display the appropriate bar image
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
