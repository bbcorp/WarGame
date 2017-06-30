#ifndef PACKET_H
#define PACKET_H
#include "ft_sprite.h"
#include "main.h"
#include "ft_bullet.h"

typedef struct
{
	int Y;
	int X;
	State state;
	char name[256];
	bool walk;
	bool fire;
	int clientNum;
}ClientPacket;

typedef struct
{
	SDL_Rect players[16];
	SDL_Rect bullets[250];
}ServerPacket;

#endif // PACKET_H
