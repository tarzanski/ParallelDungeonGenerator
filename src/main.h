
#include <SDL.h>

class display {
    private:
        bool running;

        SDL_Window* sdlwindow;

        SDL_Surface* gScreenSurface;

        SDL_Surface* gRed;

    public:
        // constructor
        display();

        int OnExecute();
 
        bool OnInit();
 
        void OnEvent(SDL_Event* Event);
 
        void OnLoop();
 
        void OnRender();
 
        void OnCleanup();
};