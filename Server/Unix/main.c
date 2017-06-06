#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "client.h"

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

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   fd_set rdfs;

   while(1)
   {
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
	 perror("select()");
	 exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
	 /* stop process when type on keyboard */
	 break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
	 /* new client */
	 SOCKADDR_IN csin = { 0 };
     Packet packet;
	 /* a client is talking */
	 read_client(sock, &csin,&packet);

	 if(check_if_client_exists(clients, &csin, actual) == 0)
	 {
	    if(actual != MAX_CLIENTS)
	    {
	       Client c = { csin };
	       strcpy(c.name, packet.name);
	       printf("%s \n",c.name);
	       clients[actual] = c;
	       actual++;
	    }
	 }
	 else
	 {
	    Client *client = get_client(clients, &csin, actual);
	    if(client == NULL) continue;
	    if(packet.name[0] == '\0')
        {
            int pos = get_client_pos(clients, &csin, actual);
            array_remove(clients, MAX_CLIENTS, pos, 1);
               actual--;
        }
        else
            send_message_to_all_clients(sock, clients, client, actual, packet, 0);
	 }
      }
   }

   end_connection(sock);
}

static int check_if_client_exists(Client *clients, SOCKADDR_IN *csin, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr
	    && clients[i].sin.sin_port == csin->sin_port)
      {
	 return 1;
      }
   }

   return 0;
}

static Client* get_client(Client *clients, SOCKADDR_IN *csin, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr
	    && clients[i].sin.sin_port == csin->sin_port)
      {
	 return &clients[i];
      }
   }

   return NULL;
}


static int get_client_pos(Client *clients, SOCKADDR_IN *csin, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr
	    && clients[i].sin.sin_port == csin->sin_port)
      {
        return i;
      }
   }

   return 0;
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
    free(&clients[to_remove]);
 //  memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void array_remove (Client* arr, size_t size, size_t index, size_t rem_size)
{
  int* begin = arr+index;                        // beginning of segment to remove
  int* end   = arr+index+rem_size;               // end of segment to remove
  size_t trail_size = size-index-rem_size;       // size of the trailing items after segment

  memcpy(begin,                                  // move data to beginning
         end,                                    // from end of segment
         trail_size*sizeof(Client));

  memset(begin+trail_size,                       // from the new end of the array
         0,                                      // set everything to zero
         rem_size*sizeof(Client));

}

static void send_message_to_all_clients(int sock, Client *clients, Client *sender, int actual, const Packet packet, char from_server)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender != &clients[i])
      {
        write_client(sock, &clients[i].sin, packet);
      }
   }
}

static int init_connection(void)
{
   /* UDP so SOCK_DGRAM */
   SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, SOCKADDR_IN *sin, Packet *packet)
{
   int n = 0;
   size_t sinsize = sizeof *sin;

   if((n = recvfrom(sock, packet, sizeof(*packet), 0, (SOCKADDR *) sin, &sinsize)) < 0)
   {
      perror("recvfrom()");
      exit(errno);
   }
   return n;
}

static void write_client(SOCKET sock, SOCKADDR_IN *sin,Packet packet)
{
   if(sendto(sock, &packet, sizeof(packet), 0, (SOCKADDR *) sin, sizeof *sin) < 0)
   {
      perror("send()");
      //exit(errno);
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}