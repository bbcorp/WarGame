#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "Item.h"
#include "engine.h"
#include "ft_SDL.h"
#include "player.h"
#include "sprite.h"

int lastTime = 0, lastTimeAnim = 0,ActualTime = 0,ActualTimeAnim = 0;
int const SleepTime = 5;
int const SleepTimeAnim = 200;
bool tour=true;
Engine _engine;
Player mainPlayer;
 Uint8 *keystate=NULL;
int main(int argc, char *argv[])
{
    _engine.WIDTH = 400;
    _engine.HEIGHT = 300;
    keystate=SDL_GetKeyboardState(NULL);
    //SDL_Color couleurNoire = {0, 0, 0}, couleurBlanche = {255, 255, 255};
    if(SDL_Init(SDL_INIT_VIDEO)== -1)
    {
        fprintf(stderr, "Erreur d'initialisation de SDL_Init : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if(TTF_Init() == -1)
    {
        fprintf(stderr, "Erreur d'initialisation de TTF_Init : %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }


        _engine.window = SDL_CreateWindow("Wargame #AFTEC",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        _engine.WIDTH, _engine.HEIGHT,
                        0);
        int h=0;
        int w=0;
        SDL_GetWindowSize(&_engine.window, &w, &h);
/* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
        _engine.screenRenderer = SDL_CreateRenderer(_engine.window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
/* Give us time to see the window. */


    mainPlayer.health = 100;
    mainPlayer.characterSurface =  IMG_LoadTexture(_engine.screenRenderer, "res/character.png");
    mainPlayer.state = DOWN;
    mainPlayer.step = 0;
    _engine.mapSurface =  IMG_LoadTexture(_engine.screenRenderer, "res/background.png");
    _engine.fogSurface = IMG_LoadTexture(_engine.screenRenderer, "res/fog_260.png");
    int running = 1;
  SDL_SetWindowFullscreen(_engine.window,SDL_WINDOW_FULLSCREEN);
    // _engine.bombSurface = IMG_LoadTexture(_engine.screenRenderer, "res/000.png");
    _engine.mapRect.x = _engine.mapSurface->w/2;
    _engine.mapRect.y = _engine.mapSurface->h/2;
    _engine.mapRect.w = _engine.WIDTH;
    _engine.mapRect.h = _engine.HEIGHT;

    mainPlayer.characterScreenRect.x = 200 - 16;
    mainPlayer.characterScreenRect.y = 150 - 16;
    mainPlayer.characterScreenRect.w = 32;
    mainPlayer.characterScreenRect.h = 32;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    // A REMETTRE SDL_EnableKeyRepeat(10, 5);

    SDL_QueryTexture(_engine.mapSurface, NULL, NULL, &w, &h);
    SDL_Rect texr; texr.x = 0; texr.y = 0; texr.w = w; texr.h = h;

    while (GetKeyPressEvent())
    {

        ft_getCharactSprite(&mainPlayer);
        SDL_RenderClear(_engine.screenRenderer);
        SDL_RenderCopy(_engine.screenRenderer, _engine.mapSurface, &_engine.mapRect, NULL);
        SDL_RenderCopy(_engine.screenRenderer, mainPlayer.characterSurface , &mainPlayer.spriteRect, &mainPlayer.characterScreenRect);
      //  SDL_RenderCopy(_engine.screenRenderer, _engine.fogSurface, NULL, NULL);
        SDL_RenderPresent(_engine.screenRenderer);
    }
    SDL_DestroyTexture(_engine.mapSurface);
    SDL_DestroyTexture(mainPlayer.characterSurface);
    SDL_DestroyTexture(_engine.fogSurface);
	SDL_DestroyRenderer(_engine.screenRenderer);
	SDL_DestroyWindow(_engine.window);

    SDL_Quit();
    return EXIT_SUCCESS;
}

// KEY CODE https://wiki.libsdl.org/SDL_ScancodeAndKeycode?highlight=%28SDL_SCANCODE%29

int GetKeyPressEvent()
{
    Uint8 *keystates = SDL_GetKeyboardState(NULL);
     if (SDL_PollEvent(&_engine.event))//close the window
        {
            if (_engine.event.type == SDL_QUIT)
              return 0;
        }
           mainPlayer.fire = false;
        mainPlayer.walk = false;
        if (keystate[SDL_SCANCODE_SPACE] )
          mainPlayer.fire = true;
        else
        {
        if (keystate[SDL_SCANCODE_LEFT] )
        {
          if(_engine.mapRect.x <= 0) return 1;
          _engine.mapRect.x -= 2;
          mainPlayer.state = LEFT;
          mainPlayer.walk = true;
        }
        if (keystate[SDL_SCANCODE_RIGHT] )
        {
            if(_engine.mapRect.x >= 720) return 1;
            _engine.mapRect.x += 2;
            mainPlayer.state = RIGTH;
            mainPlayer.walk = true;
        }
        if (keystate[SDL_SCANCODE_DOWN] )
        {
            if(_engine.mapRect.y >= 720) return 1;
            _engine.mapRect.y += 2;
            mainPlayer.state = DOWN;
            mainPlayer.walk = true;
        }
        if (keystate[SDL_SCANCODE_UP] )
        {
            if(_engine.mapRect.y <= 0) return 1;
            _engine.mapRect.y -= 2;
            mainPlayer.state = UP;
            mainPlayer.walk = true;
        }

         }

    return 1;
}


bool AnimDelay()
{
    ActualTimeAnim = SDL_GetTicks();
    if (ActualTimeAnim - lastTimeAnim > SleepTimeAnim)
    {
           lastTimeAnim = ActualTimeAnim;
           return true;
    }
    else
    {
         return false;
    }

}
void FrameDelay()
{
    ActualTime = SDL_GetTicks();
    if (ActualTime - lastTime > SleepTime )
        lastTime = ActualTime;
    else
        SDL_Delay(SleepTime - (ActualTime - lastTime));
}
