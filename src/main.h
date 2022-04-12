
#include <SDL.h>

class display {
    private:
        bool running;

        int currRoomNumber;

        SDL_Window* sdlwindow;

        SDL_Surface* gScreenSurface;

        SDL_Surface* gRoom;

        SDL_Surface* gGrey;

        


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
};