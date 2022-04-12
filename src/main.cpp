

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

    //generate(5, 25);

    display disp;

    return disp.OnExecute();

    return 0;
}


/*
 * Class constructor
 */
display::display() {
    running = true;
}

/*
 * Execution Loop
 */
int display::OnExecute() {
    if (OnInit() == false)
        return -1;
    
    SDL_Event Event;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }

        OnLoop();
        OnRender();
    } 

    OnCleanup();

    return 0;
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
                                     640, 480,
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

    SDL_SetRenderDrawColor(renderer,0x38,0x38,0x38,0xFF);
    SDL_RenderClear(renderer);
    
    return true;
}

/*
 * Event handler function
 */
void display::OnEvent(SDL_Event* Event) {
    if (Event->type == SDL_QUIT) {
        running = false;
    }
}

/*
 * Extra loop function that tutorial had
 */
void display::OnLoop() {
    
}

/*
 * Render fuction
 */
void display::OnRender() {

    // each pixel represents one unit of distance in both x and y


    gRed = SDL_LoadBMP("assets/red_square.bmp");
    if (gRed == NULL) {
        printf("Error loading bmp file\n");
    }

    SDL_BlitSurface(gRed, NULL, gScreenSurface, NULL);

    SDL_UpdateWindowSurface(sdlwindow);

}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    SDL_FreeSurface(gRed);

    SDL_DestroyWindow(sdlwindow);

    SDL_Quit();
}