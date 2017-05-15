#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "client.h"
#include "Item.h"
#include "engine.h"
#include "ft_SDL.h"
#include "player.h"
#include "sprite.h"
#include "Packet.h"

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

static Packet read_server(SOCKET sock, SOCKADDR_IN *sin)
{
    int n = 0;
    size_t sinsize = sizeof *sin;
    Packet pck;
    if((n = recvfrom(sock, &pck, sizeof(pck), 0, (SOCKADDR *) sin, &sinsize)) < 0)
    {
        perror("recvfrom()");
        exit(errno);
    }
  // buffer[n] = 0;
    return pck;
}

static void write_server(SOCKET sock, SOCKADDR_IN *sin,Packet pck)
{
   if(sendto(sock, &pck, sizeof(pck), 0, (SOCKADDR *) sin, sizeof *sin) < 0)
   {
      perror("sendto()");
      exit(errno);
   }
}


// TEXT
SDL_Window *ecran = NULL;
SDL_Surface *text = NULL;
TTF_Font *font = NULL;
SDL_Color colorWhite = {255, 255, 255};
SDL_Surface *fontSurface = NULL;
char message[20];

time_t lastTime = 0, lastTimeAnim = 0;
int const SleepTime = 30;
int const SleepTimeAnim = 200;
bool tour=true;
Engine _engine;
Player mainPlayer;
Uint8 *keystate=NULL;
SDL_Point mousePosition;

// Screen




 Player  enemiPlayer;

 void *NetworkThreadingListening(void *arg)
{
      char host[] = "127.0.0.1";
    char pseudo[] = "client";

    init();
    SOCKADDR_IN sin = {0};
    SOCKET sock = init_connection(host, &sin);
    Packet w;
    strcpy(w.name,"Quentin");
    write_server(sock, &sin,  w);
   while(true)
   {
    usleep(100);


            Packet pck;
            strcpy(pck.name,"Jack");

            pck.Y = mainPlayer.Pos.y;
            pck.X = mainPlayer.Pos.x;
            pck.state = mainPlayer.state;
            pck.fire = mainPlayer.fire;
            pck.walk = mainPlayer.walk;
           write_server(sock, &sin,pck);



            Packet p;
            p = read_server(sock, &sin);

            enemiPlayer.Pos.x = p.X;
            enemiPlayer.Pos.y = p.Y;
            enemiPlayer.state = p.state;
            enemiPlayer.fire = p.fire;
            enemiPlayer.walk = p.walk;
              enemiPlayer.Pos.x =   enemiPlayer.Pos.x - _engine.camera.x + _engine.WIDTH/2 - 16;
            enemiPlayer.Pos.y = enemiPlayer.Pos.y - _engine.camera.y + _engine.HEIGHT/2 - 16;
   }
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{


    pthread_t NwkThread;
    enemiPlayer.state = DOWN;
    enemiPlayer.step = 0;
    enemiPlayer.Pos.w = 32;
    enemiPlayer.Pos.h = 32;
      enemiPlayer.Pos.x = _engine.WIDTH/2 - 16;
    enemiPlayer.Pos.y = _engine.HEIGHT/2 - 16;
    _engine.fullscreen = 0;
    _engine.WIDTH = 400;
    _engine.HEIGHT = 300;

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
    font = TTF_OpenFont("res/verdana.ttf", 20);



    keystate=SDL_GetKeyboardState(NULL);
    _engine.window = SDL_CreateWindow("Wargame #AFTEC",
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        _engine.WIDTH, _engine.HEIGHT,
                        0);
    _engine.screenRenderer = SDL_CreateRenderer(_engine.window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    _engine.menuSurface =               IMG_LoadTexture(_engine.screenRenderer, "res/menu.png");
    //menu();
    _engine.characterSurface =          IMG_LoadTexture(_engine.screenRenderer, "res/character.png");
    _engine.characterEnnemiSurface =    IMG_LoadTexture(_engine.screenRenderer, "res/ennemi.png");
    _engine.mapSurface =                IMG_LoadTexture(_engine.screenRenderer, "res/background.png");
    _engine.fogSurface =                IMG_LoadTexture(_engine.screenRenderer, "res/fog.png");

    fontSurface = SDL_GetWindowSurface(_engine.window);


    mainPlayer.health = 100;
    mainPlayer.state = DOWN;
    mainPlayer.step = 0;
    mainPlayer.Pos.x = _engine.WIDTH/2 - 16;
    mainPlayer.Pos.y = _engine.HEIGHT/2 - 16;
    mainPlayer.Pos.w = 32;
    mainPlayer.Pos.h = 32;
    SDL_Rect pCenter;
    pCenter.x = _engine.WIDTH/2 - 16;
    pCenter.y = _engine.HEIGHT/2 - 16;
    pCenter.w = 32;
    pCenter.h = 32;
    _engine.camera.x = 0;
    _engine.camera.y = 0;
    _engine.camera.w = _engine.WIDTH;
    _engine.camera.h = _engine.HEIGHT;

    if(pthread_create(&NwkThread, NULL, NetworkThreadingListening, NULL) == -1) {
        perror("pthread_create");
	return EXIT_FAILURE;
    }


        char buffer[BUF_SIZE];
        char s_buffer[BUF_SIZE];


        while (GetKeyPressEvent())
        {


            _engine.camera.x = mainPlayer.Pos.x;
            _engine.camera.y = mainPlayer.Pos.y;
            ft_GetPlayerOrientation(&mainPlayer);
            ft_GetPlayerOrientation(&enemiPlayer);
             sprintf(message, "%i,%i", mainPlayer.Pos.x, mainPlayer.Pos.y);
            text = TTF_RenderText_Blended(font, message, colorWhite);

            SDL_Rect posText = {0, 0, text->w, text->h};
            SDL_Texture *texture = SDL_CreateTextureFromSurface(_engine.screenRenderer, text);

            SDL_RenderClear(_engine.screenRenderer);
            SDL_RenderCopy(_engine.screenRenderer, _engine.mapSurface, &_engine.camera, NULL);
            SDL_RenderCopy(_engine.screenRenderer, _engine.characterSurface , &mainPlayer.sprite, &pCenter);
            SDL_RenderCopy(_engine.screenRenderer,  _engine.characterEnnemiSurface , &enemiPlayer.sprite, &enemiPlayer.Pos);
            SDL_RenderCopy(_engine.screenRenderer, _engine.fogSurface, NULL, NULL);
            SDL_RenderCopy(_engine.screenRenderer,texture, NULL, &posText);
           // SDL_RenderCopy(_engine.screenRenderer, _engine.menuSurface, NULL, NULL);
            SDL_RenderPresent(_engine.screenRenderer);
        }
        s_buffer[0] == '\0';
      //  write_server(sock, &sin,s_buffer);
        TTF_CloseFont(font);
        SDL_FreeSurface(text);
        SDL_DestroyTexture(_engine.mapSurface);
        SDL_DestroyTexture(_engine.characterSurface);
        SDL_DestroyTexture(_engine.fogSurface);
        SDL_DestroyRenderer(_engine.screenRenderer);
        SDL_DestroyWindow(_engine.window);
        end();

        SDL_Quit();
        return EXIT_SUCCESS;

    }


// KEY CODE https://wiki.libsdl.org/SDL_ScancodeAndKeycode?highlight=%28SDL_SCANCODE%29

void ft_getCharactSprite(Player *player, State state,int step)
{
    player->sprite.x = 32 * step + (step + 1);
    player->sprite.y = 32 * state + (state + 1);
    player->sprite.h = 32;
    player->sprite.w = 32;
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
            {
                return 0;
            }

        }
        mainPlayer.fire = false;
        mainPlayer.walk = false;
        if (keystate[SDL_SCANCODE_SPACE] )
          mainPlayer.fire = true;
        else
        {
            if (keystate[SDL_SCANCODE_LEFT] &   mainPlayer.Pos.x >= 50)
            {
            mainPlayer.Pos.x -= 2;
                mainPlayer.state = LEFT;
                mainPlayer.walk = true;
            }
            if (keystate[SDL_SCANCODE_RIGHT] & mainPlayer.Pos.x <= 750)
            {
                mainPlayer.Pos.x += 2;
                mainPlayer.state = RIGHT;
                mainPlayer.walk = true;
            }
            if (keystate[SDL_SCANCODE_UP] &   mainPlayer.Pos.y >= 50 )
            {
                mainPlayer.Pos.y -= 2;
                mainPlayer.state = UP;
                mainPlayer.walk = true;
            }
            if (keystate[SDL_SCANCODE_DOWN] &   mainPlayer.Pos.y <= 750 )
            {
                mainPlayer.Pos.y += 2;
                mainPlayer.state = DOWN;
                mainPlayer.walk = true;
            }

  }
    if (keystates[SDL_SCANCODE_LALT] && keystates[SDL_SCANCODE_RETURN] )
        {
            /*if (_engine.fullscreen == 1)
            {*/
                SDL_SetWindowFullscreen(_engine.window,SDL_WINDOW_FULLSCREEN);
               /* _engine.fullscreen = 0;
            }
            else
            {
               SDL_SetWindowFullscreen(_engine.window,0);
               _engine.fullscreen = 1;
            }*/
        }
    SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
        mainPlayer.fire = true;
    return 1;
}


bool AnimDelay(Player *player)
{
    int ActualTimeAnim = SDL_GetTicks();
    if (ActualTimeAnim - player->lastAnim > SleepTimeAnim)
    {
           player->lastAnim = ActualTimeAnim;
           return true;
    }
    else
    {
         return false;
    }

}
int FrameDelay()
{
   time_t ActualTime = time(0); // Get the system time;
    if (ActualTime - lastTime > SleepTime )
  {
       lastTime = ActualTime;
        return 1;
  }
    else
       return 0;
}

/*void MousePosition(int lastx, int lasty)
{

}*/
