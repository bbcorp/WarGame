#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "client.h"
#include "Item.h"
#include "engine.h"
#include "ft_SDL.h"
#include "player.h"
#include "sprite.h"

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static int init_connection(const char *address, SOCKADDR_IN *sin)
{
   /* UDP so SOCK_DGRAM */
   SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   struct hostent *hostinfo;

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   hostinfo = gethostbyname(address);
   if (hostinfo == NULL)
   {
      fprintf (stderr, "Unknown host %s.\n", address);
      exit(EXIT_FAILURE);
   }

   sin->sin_addr = *(IN_ADDR *) hostinfo->h_addr;
   sin->sin_port = htons(PORT);
   sin->sin_family = AF_INET;

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_server(SOCKET sock, SOCKADDR_IN *sin, char *buffer)
{
   int n = 0;
   size_t sinsize = sizeof *sin;

   if((n = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (SOCKADDR *) sin, &sinsize)) < 0)
   {
      perror("recvfrom()");
      exit(errno);
   }

   buffer[n] = 0;

   return n;
}

static void write_server(SOCKET sock, SOCKADDR_IN *sin, const char *buffer)
{
   if(sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *) sin, sizeof *sin) < 0)
   {
      perror("sendto()");
      exit(errno);
   }
}



SDL_Color textColor = { 255, 255, 255, 255 }; // white
int lastTime = 0, lastTimeAnim = 0,ActualTime = 0,ActualTimeAnim = 0;
int const SleepTime = 5;
int const SleepTimeAnim = 200;
bool tour=true;
Engine _engine;
Player mainPlayer;
Uint8 *keystate=NULL;
SDL_Color couleurNoire = {0, 0, 0};
SDL_Texture* SurfaceToTexture( SDL_Surface* surf );
SDL_Point mousePosition;
int main(int argc, char *argv[])
{

    char host[] = "172.24.9.63";
    char pseudo[] = "client";
    _engine.fullscreen = 0;
    _engine.WIDTH = 400;
    _engine.HEIGHT = 300;
    TTF_Font *police = NULL;
    SDL_Surface *texte = NULL;
    SDL_Texture* blendedTexture;
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


    keystate=SDL_GetKeyboardState(NULL);
    police = TTF_OpenFont("res\verdana.ttf", 15 );
    _engine.window = SDL_CreateWindow("Wargame #AFTEC",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        _engine.WIDTH, _engine.HEIGHT,
                        0);
        //int h=0;
        //int w=0;
    //SDL_GetWindowSize(&_engine.window, &w, &h);
    _engine.screenRenderer = SDL_CreateRenderer(_engine.window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    mainPlayer.health = 100;
    _engine.characterSurface =  IMG_LoadTexture(_engine.screenRenderer, "res/character.png");
    _engine.characterEnemiSurface =  IMG_LoadTexture(_engine.screenRenderer, "res/enemi.png");
    mainPlayer.state = DOWN;
    mainPlayer.step = 0;
    _engine.mapSurface =  IMG_LoadTexture(_engine.screenRenderer, "res/background.png");
    _engine.fogSurface = IMG_LoadTexture(_engine.screenRenderer, "res/fog.png");
    _engine.mapRect.x = 0;
    _engine.mapRect.y = 0;
    _engine.mapRect.w = _engine.WIDTH;
    _engine.mapRect.h = _engine.HEIGHT;

    mainPlayer.characterScreenRect.x = _engine.WIDTH/2 - 16;
    mainPlayer.characterScreenRect.y = _engine.HEIGHT/2 - 16;
    mainPlayer.characterScreenRect.w = 32;
    mainPlayer.characterScreenRect.h = 32;
    SDL_Rect pCenter;
    pCenter.x = _engine.WIDTH/2 - 16;
    pCenter.y = _engine.HEIGHT/2 - 16;
    pCenter.w = 32;
    pCenter.h = 32;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    // A REMETTRE SDL_EnableKeyRepeat(10, 5);
//SDL_ShowCursor(SDL_DISABLE);



   //SDL_Surface* solid = TTF_RenderText_Blended( police, "plop", couleurNoire );

	//blendedTexture = SurfaceToTexture( solid );
        init();
        SOCKADDR_IN sin = { 0 };
        SOCKET sock = init_connection(host, &sin);
        char buffer[BUF_SIZE];
        char s_buffer[BUF_SIZE];
        write_server(sock, &sin, pseudo);
        while (GetKeyPressEvent())
        {
            //fprintf(stderr, "_engine.mapRect.x : %d\n", _engine.mapRect.x);
            //fprintf(stderr, "_engine.mapRect.y : %d\n", _engine.mapRect.y);
            /*SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
            MousePosition(mousePosition.x, mousePosition.y);*/

            fd_set rdfs;
            FD_ZERO(&rdfs);
            FD_SET(sock, &rdfs);
            //sprintf (s_buffer, "%d %d", _engine.mapRect.x - _engine.WIDTH/2 - 16, _engine.mapRect.y - _engine.HEIGHT/2 - 116);
            sprintf (s_buffer, "%d %d", mainPlayer.characterScreenRect.x, mainPlayer.characterScreenRect.y);
            write_server(sock, &sin,s_buffer);
            if(FD_ISSET(sock, &rdfs))
            {
                int n = read_server(sock, &sin, buffer);
                if(n == 0)
                {
                    printf("Server disconnected !\n");
                    break;
                }
                sscanf (buffer,"%d %d",&_engine.mapRectEnemi.x,&_engine.mapRectEnemi.y);

            }
            _engine.mapRectEnemi.w = 32;
            _engine.mapRectEnemi.h = 32;
            printf ("Player.x = %d, Player.y = %d\n", mainPlayer.characterScreenRect.x, mainPlayer.characterScreenRect.y);
            printf ("Ennemi.x = %d, Ennemi.y = %d\n", _engine.mapRectEnemi.x, _engine.mapRectEnemi.y);
            printf ("map.x = %d, map.y = %d\n", _engine.mapRect.x, _engine.mapRect.y);
            ft_GetPlayerOrientation(&mainPlayer);
            SDL_RenderClear(_engine.screenRenderer);
            _engine.mapRect.x =  mainPlayer.characterScreenRect.x ;
            _engine.mapRect.y = mainPlayer.characterScreenRect.y;
            SDL_RenderCopy(_engine.screenRenderer, _engine.mapSurface, &_engine.mapRect, NULL);




            SDL_RenderCopy(_engine.screenRenderer, _engine.characterSurface , &_engine.spriteRect, &pCenter);
            SDL_RenderCopy(_engine.screenRenderer,  _engine.characterEnemiSurface , &_engine.spriteRect, &_engine.mapRectEnemi);
            SDL_RenderCopy(_engine.screenRenderer, _engine.fogSurface, NULL, NULL);
            SDL_RenderPresent(_engine.screenRenderer);
        }
        SDL_DestroyTexture(_engine.mapSurface);
        SDL_DestroyTexture(_engine.characterSurface);
        SDL_DestroyTexture(_engine.fogSurface);
        SDL_DestroyRenderer(_engine.screenRenderer);
        SDL_DestroyWindow(_engine.window);

        SDL_Quit();
        return EXIT_SUCCESS;

    }


// KEY CODE https://wiki.libsdl.org/SDL_ScancodeAndKeycode?highlight=%28SDL_SCANCODE%29

void ft_getCharactSprite(State state,int step)
{
    _engine.spriteRect.x = 32 * step + (step + 1);
    _engine.spriteRect.y = 32 * state + (state + 1);
    _engine.spriteRect.h = 32;
    _engine.spriteRect.w = 32;

}


SDL_Texture* SurfaceToTexture( SDL_Surface* surf )
{
	SDL_Texture* text;

	text = SDL_CreateTextureFromSurface( _engine.screenRenderer, surf );

	SDL_FreeSurface( surf );

	return text;
}

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
         // if(_engine.mapRect.x <= 48) return 1;
         mainPlayer.characterScreenRect.x -= 2;
          mainPlayer.state = LEFT;
          mainPlayer.walk = true;
        }
        if (keystate[SDL_SCANCODE_RIGHT] )
        {
           // if(_engine.mapRect.x >= 752) return 1;
         mainPlayer.characterScreenRect.x += 2;
            mainPlayer.state = RIGHT;
            mainPlayer.walk = true;
        }

        if (keystate[SDL_SCANCODE_UP] )
        {
           // if(_engine.mapRect.y <= 48) return 1;
           mainPlayer.characterScreenRect.y -= 2;
            mainPlayer.state = UP;
            mainPlayer.walk = true;
        }
      if (keystate[SDL_SCANCODE_DOWN] )
        {
           // if(_engine.mapRect.y <= 48) return 1;
           mainPlayer.characterScreenRect.y += 2;
            mainPlayer.state = DOWN;
            mainPlayer.walk = true;
        }
  }
    if (keystates[SDL_SCANCODE_LALT] && keystates[SDL_SCANCODE_RETURN] )
        {
            if (_engine.fullscreen == 1)
            {
                SDL_SetWindowFullscreen(_engine.window,SDL_WINDOW_FULLSCREEN);
                _engine.fullscreen = 0;
            }
            else
            {
               SDL_SetWindowFullscreen(_engine.window,0);
               _engine.fullscreen = 1;
            }
        }
        SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
 //  SDL_Log("Mouse position : x=%i y=%i", mousePosition.x, mousePosition.y);
   // SDL_Log("fullscreen : %i", _engine.fullscreen);

    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
        mainPlayer.fire = true;

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

/*void MousePosition(int lastx, int lasty)
{

}*/
