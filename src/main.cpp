

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <random>

#include "generate.h"
#include "main.h"
#include <SDL.h>


int main(int argc, char** argv) {

    // implement argument parsing

    // initialize SDL interface if using a GUI

    // set up any timing before calling algorithm functions

    // parse arguments if using a gui

    int roomNum = 500;

    rectangle_t *rooms = generate(roomNum, 25);

    display disp;

    printf("*****STARTING GUI*****\n");

    int ecode = disp.OnExecute(rooms, roomNum);

    printf("***** CLOSING GUI*****\n");

    free(rooms);

    return ecode;
}


/*
 * Class constructor
 */
display::display() {
    running = true;
    currRoomNumber = 0;
    pixPerUnit = 5;
    x_offset = 0;
    y_offset = 0;
}

/*
 * Initialization function
 */
bool display::OnInit() {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Error with initing SDL\n");
        return false;
    }

    if((sdlwindow = SDL_CreateWindow("Dungeon Generator Visualizer",
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT,
                                     SDL_WINDOW_OPENGL)) == NULL) {
        printf("Error with creating window\n");
        return false;
    }

    if ((renderer = SDL_CreateRenderer(sdlwindow, -1, 0)) == NULL) {
        printf("Error with creating software renderer: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer); // <-- render update function 

    return true;
}

SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer) {
    SDL_Texture* tex = NULL;

    SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
    if (loadedSurface == NULL) {
        printf("Unable to load image %s :: Error %s\n",path.c_str(), SDL_GetError());
    }

    // converting surface to texture for rendering
    tex = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (tex == NULL) {
        printf("Unable to create texture for image %s\n", path.c_str());
    }

    SDL_FreeSurface(loadedSurface);
    return tex;
}

void display::loadAssets() {
    gRoom = loadTexture("assets/room_proto_2.bmp", renderer);

    gSides = loadTexture("assets/turq_square.bmp", renderer);

    gGrey = loadTexture("assets/grey_square.bmp", renderer);

    gRed = loadTexture("assets/red_square.bmp", renderer);
}

/*
 * Execution Loop
 */
int display::OnExecute(rectangle_t *rooms, int roomNum) {
    if (OnInit() == false)
        return -1;

    loadAssets();

    SDL_Event Event;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }

        OnRender(rooms, roomNum);

    //     OnLoop();
    }

    OnCleanup();

    return 0;
}

/*
 * Event handler function
 */
void display::OnEvent(SDL_Event* event) {
    if (event->type == SDL_QUIT) {
        running = false;
    }

    if (event->type == SDL_MOUSEWHEEL) {
        if (event->wheel.y > 0) { // scroll up
            pixPerUnit += 1;
            //printf("Scrolling up by: %d ppu: %d\n", event->wheel.y, pixPerUnit);
        }
        if (event->wheel.y < 0 && pixPerUnit != 1) { // scroll down
            pixPerUnit -= 1;
            //printf("Scrolling down by: %d ppu: %d\n", event->wheel.y, pixPerUnit);
        }
    }

    // if (Event->type == SDL_KEYDOWN) {
    // }
}

/*
 * Extra loop function that tutorial had
 */
void display::OnLoop() {
    //currRoomNumber++;
    //SDL_Delay(125);
}

/*
 * Generating the background image
 */
void display::genBackround() {
    // going right from origin inclusive
    int originx = SCREEN_WIDTH / 2;
    int originy = SCREEN_HEIGHT / 2;

    SDL_Rect moverect;
    // right from origin inclusive
    for (int x = originx; x < SCREEN_WIDTH; x += pixPerUnit) {
        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }
    // left from origin
    for (int x = originx - pixPerUnit; x > 0; x -= pixPerUnit) {
        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }


    // going down from origin inclusive
    for (int y = originy; y < SCREEN_HEIGHT; y += pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }

    // going down from origin inclusive
    for (int y = originy - pixPerUnit; y > 0; y -= pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }
}

/*
 * Render fuction
 */
void display::OnRender(rectangle_t *rooms, int roomNum) {
    // clearing old frame
    SDL_RenderClear(renderer);

    // first generate background grid
    genBackround();

    for (int room_inc = 0; room_inc < roomNum; room_inc++) {

        SDL_Rect roomRect;

        float roomW = rooms[room_inc].width;
        float roomH = rooms[room_inc].height;

        // taking abs of width and height
        roomW = (roomW < 0) ? (roomW * -1) : roomW;
        roomH = (roomH < 0) ? (roomH * -1) : roomH;

        roomRect.h = (int)roomH * pixPerUnit;
        roomRect.w = (int)roomW * pixPerUnit;

        // calculating the room centers based on the number of pixels per unit
        int units_x = (int)rooms[room_inc].center.x + x_offset;
        int units_y = (int)rooms[room_inc].center.y + y_offset;

        roomRect.x = ((int)(units_x) * pixPerUnit) - (((roomRect.w/2) / pixPerUnit) * pixPerUnit);
        roomRect.y = ((int)( units_y) * pixPerUnit) - (((roomRect.h/2) / pixPerUnit) * pixPerUnit);

        // final alignment to move origin to middle of the window
        roomRect.x += SCREEN_WIDTH / 2;
        roomRect.y += SCREEN_HEIGHT / 2;

        SDL_RenderCopy(renderer, gRoom, NULL, &roomRect);

        /********* add sides to room *********/

        SDL_Rect sideRect;

        // left side
        sideRect.x = roomRect.x;
        sideRect.y = roomRect.y;
        sideRect.h = roomRect.h;
        sideRect.w = 1;

        SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

        // top side
        sideRect.h = 1;
        sideRect.w = roomRect.w;

        SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

        // bottom side
        sideRect.y = roomRect.y + roomRect.h - 1;

        SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

        // right side
        sideRect.x = roomRect.x + roomRect.w - 1;
        sideRect.y = roomRect.y;
        sideRect.h = roomRect.h;
        sideRect.w = 1;

        SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

        SDL_Rect dotRect;

        // drawing red dot for center of room
        dotRect.h = 3;
        dotRect.w = 3;

        dotRect.x = ((int)(rooms[room_inc].center.x) * pixPerUnit) + (SCREEN_WIDTH / 2) - dotRect.w/2;
        dotRect.y = ((int)(rooms[room_inc].center.y) * pixPerUnit) + (SCREEN_HEIGHT / 2) - dotRect.h/2;

        SDL_RenderCopy(renderer, gRed, NULL, &dotRect);
        
    }
    SDL_RenderPresent(renderer);
}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    // SDL_FreeSurface(gRoom);
    SDL_DestroyTexture(gRoom);

    // SDL_FreeSurface(gGrey);
    SDL_DestroyTexture(gGrey);

    // SDL_FreeSurface(gRed);
    SDL_DestroyTexture(gRed); 
 
    // SDL_FreeSurface(gSides);
    SDL_DestroyTexture(gSides);

    SDL_DestroyWindow(sdlwindow);

    SDL_DestroyRenderer(renderer);

    SDL_Quit();
}
