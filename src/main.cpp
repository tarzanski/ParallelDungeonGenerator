

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

    int roomNum = 50;

    rectangle_t *rooms = generate(roomNum, 25);

    display disp;

    printf("*****STARTING GUI*****\n");

    int ecode = disp.OnExecute(rooms, roomNum);

    printf("***** CLOSING GUI*****\n");

    for (int i = 0; i < roomNum; i++) {
        free(rooms[i].neighbors);
    }
    free(rooms);

    return ecode;
}


/*
 * Class constructor
 */
display::display() {
    running = true;
    currRoomNumber = 0;
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

    gScreenSurface = SDL_GetWindowSurface(sdlwindow);
    SDL_Renderer* renderer;
    if ((renderer = SDL_CreateSoftwareRenderer(gScreenSurface)) == NULL) {
        printf("Error with creating software renderer");
        return false;
    }

    SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);
    SDL_RenderClear(renderer);

    return true;
}

/*
 * Generating the background image
 */
void display::genBackround() {
    gRoom = SDL_LoadBMP("assets/room_proto_2.bmp");
    if (gRoom == NULL) {
        printf("Error loading bmp file (room)\n");
    }

    gSides = SDL_LoadBMP("assets/turq_square.bmp");
    if (gSides == NULL) {
        printf("Error loading bmp file (sides)\n");
    }

    gGrey = SDL_LoadBMP("assets/grey_square.bmp");
    if (gGrey == NULL) {
        printf("Error loading bmp file (grey)\n");
    }

    gRed = SDL_LoadBMP("assets/red_square.bmp");
    if (gRed == NULL) {
        printf("Error loading bmp file (red)\n");
    }

    // SDL_BlitSurface(gRed, NULL, gScreenSurface, NULL);

    for (int x = 0; x < SCREEN_WIDTH; x += PIX_PER_UNIT) {
        SDL_Rect moverect;

        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }

    for (int y = 0; y < SCREEN_HEIGHT; y += PIX_PER_UNIT) {
        SDL_Rect moverect;

        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }

    SDL_UpdateWindowSurface(sdlwindow);
}

/*
 * Execution Loop
 */
int display::OnExecute(rectangle_t *rooms, int roomNum) {
    if (OnInit() == false)
        return -1;

    genBackround();

    SDL_Event Event;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }
        if (currRoomNumber < roomNum) {

            OnRender(rooms, roomNum);

            OnLoop();
        }
    }

    OnCleanup();

    return 0;
}

/*
 * Event handler function
 */
void display::OnEvent(SDL_Event* Event) {
    if (Event->type == SDL_QUIT) {
        running = false;
    }
    // if (Event->type == SDL_KEYDOWN) {
    //     currRoomNumber++;
    // }
}

/*
 * Extra loop function that tutorial had
 */
void display::OnLoop() {
    currRoomNumber++;
    //SDL_Delay(125);
}

/*
 * Render fuction
 */
void display::OnRender(rectangle_t *rooms, int roomNum) {

    SDL_Rect roomRect;

    float roomW = rooms[currRoomNumber].width;
    float roomH = rooms[currRoomNumber].height;

    // taking abs of width and height
    roomW = (roomW < 0) ? (roomW * -1) : roomW;
    roomH = (roomH < 0) ? (roomH * -1) : roomH;

    roomRect.h = (int)roomH * PIX_PER_UNIT;
    roomRect.w = (int)roomW * PIX_PER_UNIT;

    // calculating the room centers based on the number of pixels per unit
    float units_x = rooms[currRoomNumber].center.x;
    float units_y = rooms[currRoomNumber].center.y;

    roomRect.x = ((int)(units_x) * PIX_PER_UNIT) - (((roomRect.w/2) / PIX_PER_UNIT) * PIX_PER_UNIT);
    roomRect.y = ((int)( units_y) * PIX_PER_UNIT) - (((roomRect.h/2) / PIX_PER_UNIT) * PIX_PER_UNIT);

    // final alignment to move origin to middle of the window
    roomRect.x += SCREEN_WIDTH / 2;
    roomRect.y += SCREEN_HEIGHT / 2;

    SDL_BlitScaled(gRoom, NULL, gScreenSurface, &roomRect);

    // printf("****** Room # %d\n", currRoomNumber);
    // printf("top corner x: %d\n", roomRect.x);
    // printf("top corner y: %d\n", roomRect.y);
    // printf("width       : %d\n", roomRect.w);
    // printf("height      : %d\n", roomRect.h);
    // printf("real x (int): %d\n", (int)units_x);
    // printf("real y (int): %d\n", (int)units_y);


    /********* add sides to room *********/

    SDL_Rect sideRect;

    // left side
    sideRect.x = roomRect.x;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    // top side
    sideRect.h = 1;
    sideRect.w = roomRect.w;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    // bottom side
    sideRect.y = roomRect.y + roomRect.h - 1; // not sure about -1????

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    // right side
    sideRect.x = roomRect.x + roomRect.w - 1;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    SDL_Rect dotRect;

    // draw all the room centers as a small red dot up to curr room
    for (int i = 0; i < currRoomNumber; i++) {

        dotRect.h = 3;
        dotRect.w = 3;

        dotRect.x = ((int)(rooms[currRoomNumber].center.x) * PIX_PER_UNIT) + (SCREEN_WIDTH / 2) - dotRect.w/2;
        dotRect.y = ((int)(rooms[currRoomNumber].center.y) * PIX_PER_UNIT) + (SCREEN_HEIGHT / 2) - dotRect.h/2;

        SDL_BlitScaled(gRed, NULL, gScreenSurface, &dotRect);
    }

    SDL_UpdateWindowSurface(sdlwindow);

}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    SDL_FreeSurface(gRoom);

    SDL_FreeSurface(gGrey);

    SDL_FreeSurface(gRed);

    SDL_FreeSurface(gSides);

    SDL_DestroyWindow(sdlwindow);

    SDL_Quit();
}
