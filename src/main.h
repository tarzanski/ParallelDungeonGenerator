
#include <SDL.h>

// one extra pixel so all lines can be drawn
#define SCREEN_WIDTH 1280

#define SCREEN_HEIGHT 1024

// #define PIX_PER_UNIT 5

class display {
    private:

        // SDL runtime variables
        bool running;

        SDL_Window* sdlwindow;

        SDL_Renderer* renderer;

        SDL_Texture* gRoom;

        SDL_Texture* gSides;

        SDL_Texture* gGrey;

        SDL_Texture* gRed;

        // ui distance depencencies
        int currRoomNumber;

        int pixPerUnit;

        int x_offset;

        int y_offset;

    public:
        // constructor
        display();

        int OnExecute(rectangle_t* rooms, int roomNum);
 
        bool OnInit();
 
        void OnEvent(SDL_Event* Event);
 
        void OnLoop();
 
        void OnRender(rectangle_t *rooms, int roomNum);
 
        void OnCleanup();

        // other functions

        void genBackround();

        void loadAssets();
};