#include <iostream>
#include <time.h>
#include <string>
#include <sstream>
#include <cmath>
#include <deque>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>

// ENUM to represent directions
enum Directions {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    DEFAULT
};

// CONSTANTS
int SCREEN_WIDTH = 600;
int SCREEN_HEIGHT = 600;
int SCREEN_FPS = 60;
float SCREEN_TICKS_PER_FRAME = 1000.0f / SCREEN_FPS;
int TILE_SIZE = 24;

// SNAKE CONSTANTS
int SNAKE_SPAWN_X = (SCREEN_WIDTH / TILE_SIZE) / 2;
int SNAKE_SPAWN_Y = (SCREEN_HEIGHT / TILE_SIZE) / 2;

// FUNCTIONS SIGS
bool init();
void close();

// GLOBAL VARS
SDL_Window* mWindow = NULL;
SDL_Renderer* mRenderer = NULL;
SDL_Texture* mText = NULL;
TTF_Font* mFont = NULL;

int main(int argc, char* argv[]) {
    bool foodConsumed = true;
    bool running = true;

    bool speedShouldChange = true;
    bool collided = false;

    SDL_Event e;
    SDL_Rect mDestRect;

    SDL_Rect foodRect;
    foodRect.w = TILE_SIZE;
    foodRect.h = TILE_SIZE;
    SDL_Point foodPos;

    SDL_Rect snakeRect;
    snakeRect.w = TILE_SIZE;
    snakeRect.h = TILE_SIZE;
    SDL_Point snakeHeadPos;

    int snakeLength = 1;
    std::deque<SDL_Point> snakeSegments;
    SDL_Point snakeSegment;

    int frameCounter = 0;
    int moveCounter = 0;

    float snakeMoveSpeed = 0.2f;
    int move_speed_counter = 0;

    int foodXPos = 0;
    int foodYPos = 0;

    Directions currentDirection = DEFAULT;

    std::stringstream ss;

    if (!init()) {
        std::cout << "Failed to initialize!\n";
    }
    else {
        // Seeds the RNG
        srand(time(NULL));

        while (!collided) {

            // Starts the performance counter for framecounting
            Uint64 start = SDL_GetPerformanceCounter();

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                }      

                // USER PRESS KEY
                else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                    case SDLK_w:
                        currentDirection = UP;
                        break;
                    case SDLK_d:
                        currentDirection = RIGHT;
                        break;
                    case SDLK_s:
                        currentDirection = DOWN;
                        break;
                    case SDLK_a:
                        currentDirection = LEFT;
                        break;
                    default:
                        break;
                    }
                }
            }

            // CLEAR WINDOW
            SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
            SDL_RenderClear(mRenderer);

            // Initialize first snakeSegment and push it into the queue
            if (snakeSegments.empty()) {
                snakeRect.x = SNAKE_SPAWN_X * TILE_SIZE;
                snakeRect.y = SNAKE_SPAWN_Y * TILE_SIZE;
                snakeSegment.x = snakeRect.x;
                snakeSegment.y = snakeRect.y;
                snakeSegments.push_front(snakeSegment);
            }

            // CHOOSE RANDOM POS FOR FOOD
            if (foodConsumed) {
                bool foodPosValid = true;

                // While food IS inside snake body, choose new position for food
                do {
                    foodPos.x = 1 + rand() % ((SCREEN_WIDTH / TILE_SIZE) - 2);
                    foodPos.y = 1 + rand() % ((SCREEN_HEIGHT / TILE_SIZE) - 2);

                    // Goes through snakeSegments queue to see if foodPos is equal to a snakeSegment position
                    for (const SDL_Point point: snakeSegments) {
                        if (point.x == (foodPos.x * TILE_SIZE) && point.y == (foodPos.y * TILE_SIZE)) {
                            foodPosValid = false;
                            break;
                        }
                        else {
                            foodPosValid = true;
                        }
                    }
                } while (!foodPosValid);

                
                foodConsumed = false;
            }        

            // DRAW FOOD
            foodRect.x = foodPos.x * TILE_SIZE;
            foodRect.y = foodPos.y * TILE_SIZE;

            SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);
            SDL_RenderFillRect(mRenderer, &foodRect);

            // MOVE SNAKE
            if (moveCounter % int(snakeMoveSpeed * SCREEN_FPS) == 0 && moveCounter != 0 && currentDirection != DEFAULT) {

                // Move snake according to chosen direction
                switch (currentDirection) {
                case UP:
                    snakeRect.y -= TILE_SIZE;
                    break;
                case RIGHT:
                    snakeRect.x += TILE_SIZE;
                    break;
                case DOWN:
                    snakeRect.y += TILE_SIZE;
                    break;
                case LEFT:
                    snakeRect.x -= TILE_SIZE;
                default:
                    break;
                }

                // Resets the move counter to prepare for next move
                moveCounter = 0;

                // Update the snakeHeadPosition
                snakeHeadPos.x = snakeRect.x;
                snakeHeadPos.y = snakeRect.y;

                // Check to see if head collided with boundaries
                if (snakeHeadPos.x < TILE_SIZE ||
                    snakeHeadPos.x > SCREEN_WIDTH - (TILE_SIZE * 2)||
                    snakeHeadPos.y < TILE_SIZE ||
                    snakeHeadPos.y > SCREEN_HEIGHT - (TILE_SIZE * 2)
                   )
                {
                    std::cout << "Collided!" << std::endl;
                    collided = true;
                    break;
                }

                // Goes through snakeSegments queue to see if head collided with segment
                for (const SDL_Point point : snakeSegments) {
                    if (point.x == snakeHeadPos.x && point.y == snakeHeadPos.y) {
                        collided = true;
                    }
                }

                // If snakeHeadPos same as foodPos, eats food, grow and speed up
                // Else just shift all snake segments to simulate movement
                if (snakeHeadPos.x == foodRect.x && snakeHeadPos.y == foodRect.y) {
                    foodConsumed = true;
                    snakeLength++;

                    if (snakeMoveSpeed > 0.05f)

                    snakeMoveSpeed -= 0.001f;
                    snakeSegments.push_front(snakeHeadPos);
                }
                else {
                    snakeSegments.pop_back();
                    snakeSegments.push_front(snakeHeadPos);
                }
            }

            // Draws level boundaries
            SDL_Rect topBoundaryRect;
            topBoundaryRect.x = 0;
            topBoundaryRect.y = 0;
            topBoundaryRect.w = SCREEN_WIDTH;
            topBoundaryRect.h = TILE_SIZE;

            SDL_Rect rightBoundaryRect;
            rightBoundaryRect.x = SCREEN_WIDTH - TILE_SIZE;
            rightBoundaryRect.y = 0;
            rightBoundaryRect.w = TILE_SIZE;
            rightBoundaryRect.h = SCREEN_HEIGHT;

            SDL_Rect bottomBoundaryRect;
            bottomBoundaryRect.x = 0;
            bottomBoundaryRect.y = SCREEN_HEIGHT - TILE_SIZE;
            bottomBoundaryRect.w = SCREEN_WIDTH;
            bottomBoundaryRect.h = TILE_SIZE;

            SDL_Rect leftBoundaryRect;
            leftBoundaryRect.x = 0;
            leftBoundaryRect.y = 0;
            leftBoundaryRect.w = TILE_SIZE;
            leftBoundaryRect.h = SCREEN_HEIGHT;

            SDL_SetRenderDrawColor(mRenderer, 96, 96, 96, 255);
            SDL_RenderFillRect(mRenderer, &topBoundaryRect);
            SDL_RenderFillRect(mRenderer, &rightBoundaryRect);
            SDL_RenderFillRect(mRenderer, &bottomBoundaryRect);
            SDL_RenderFillRect(mRenderer, &leftBoundaryRect);

            // DRAW SNAKE
            if (!collided) {
                SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);

                for (int i = 0; i < snakeSegments.size(); i++) {
                    SDL_Rect snakeSegment;
                    snakeSegment.x = snakeSegments.at(i).x;
                    snakeSegment.y = snakeSegments.at(i).y;
                    snakeSegment.w = TILE_SIZE;
                    snakeSegment.h = TILE_SIZE;
                    SDL_RenderFillRect(mRenderer, &snakeSegment);
                }
            }

            // Stops the performance counter for framecounting
            Uint64 end = SDL_GetPerformanceCounter();

            // counts the time passed during the frame
            float elapsedTime = (end - start) / (float)SDL_GetPerformanceFrequency();
            float fps = 1.0f / elapsedTime;


            /// DEBUG SECTION /// 
            //ss << "frames: " << frameCounter;

            //// render the frame counter to screen
            //if (ss.str().size() > 0) {
            //    SDL_Color foreground = { 255, 255, 255 };
            //    SDL_Surface* mText_Surface = TTF_RenderText_Solid(mFont, ss.str().c_str(), foreground);

            //    if (mText_Surface == NULL) {
            //        std::cout << "Unable to RenderText! SDL_Error: " << SDL_GetError() << std::endl;
            //    }
            //    else {
            //        mText = SDL_CreateTextureFromSurface(mRenderer, mText_Surface);
            //        mDestRect.x = 20;
            //        mDestRect.y = 20;
            //        mDestRect.w = mText_Surface->w;
            //        mDestRect.h = mText_Surface->h;
            //        SDL_RenderCopy(mRenderer, mText, NULL, &mDestRect);

            //        SDL_DestroyTexture(mText);
            //        SDL_FreeSurface(mText_Surface);

            //        ss.clear();
            //        ss.str("");
            //    }
            //}

            //// Frame counting for DEBUG
            //if (frameCounter == SCREEN_FPS) {
            //    frameCounter = 0;
            //    frameCounter++;
            //}
            //else {
            //    frameCounter++;
            //}

            // Increment counter to move snake (countdown to move time)
            moveCounter++;

            // Delays the frame to limit the framerate. VERY IMPORTANT
            SDL_Delay(floor(SCREEN_TICKS_PER_FRAME - elapsedTime));

            // Render the frame
            SDL_RenderPresent(mRenderer);
        }
    }

    std::cout << "GAME OVER!" << std::endl;

    // Deallocates ressources
    close();

    return 0;
}


bool init() {
    bool successFlag = true;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Failed to initialize SDL! SDL_Error: " << SDL_GetError() << std::endl;
        successFlag = false;
    }

    if (TTF_Init() < 0) {
        std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
        successFlag = false;
    }

    mWindow = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (mWindow == NULL) {
        std::cout << "Failed to create window! SDL_Error: " << SDL_GetError() << std::endl;
        successFlag = false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mRenderer == NULL) {
        std::cout << "Failed to create renderer! SDL_Error: " << SDL_GetError() << std::endl;
        successFlag = false;
    }

    mFont = TTF_OpenFont("segoeui.ttf", 32);
    if (mFont == NULL) {
        std::cout << "Error loading font: " << TTF_GetError() << std::endl;
        successFlag = false;
    }

    SDL_StartTextInput();

    return successFlag;
}


void close() {
    SDL_StopTextInput();
    TTF_CloseFont(mFont);
    
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    mWindow = NULL;
    mRenderer = NULL;

    TTF_Quit();
    SDL_Quit();
}
