#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/uio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

#define SERVER_PORT  12345

#define TRUE             1
#define FALSE            0
#define PORT            5550   /* Port that will be opened */ 
#define BACKLOG         2   /* Number of allowed connections */
#define BUFF_SIZE       1024

/*
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
   return (port > 0) && (port <= 65535);
}
/*
* Check valid messages
* @param char* mess
* @return 1 if valid, else return 0
*/
int validMessages(char* mess) {
   int i;
   for(i = 0; i < strlen(mess); i++) {
      if((isalpha(mess[i]) == 0) && (isdigit(mess[i]) == 0)) {
         return 0;
      }
   }
   return 1;
}
/*
* get char from messages
* @param char* mess, char* str
* @return void
*/
void charFromMessage(char* mess, char* str) {
   int i;
   int j = 0;
   for(i = 0; i < strlen(mess); i++) {
      if(isalpha(mess[i]) != 0) {
         str[j++] = mess[i];
      }
   }
}
/*
* get number from messages
* @param char* mess, char* str
* @return void
*/
void numberFromMessage(char* mess, char* str) {
   int i;
   int j = 0;
   for(i = 0; i < strlen(mess); i++) {
      if(isdigit(mess[i]) != 0) {
         str[j++] = mess[i];
      }
   }
}

int main (int argc, char *argv[])
{
   int    i, len, rc, on = 1;
   int    listen_sd, max_sd, new_sd;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   char   buffer[80];
   struct sockaddr_in   addr;
   struct timeval       timeout;
   struct fd_set        master_set, working_set;
   int bytes_sent, bytes_received;

   int port_number;
   if(argc != 2) {
      perror(" Error Parameter! Please input only port number\n ");
      exit(0);
   }
   if((port_number = atoi(argv[1])) == 0) {
      perror(" Please input port number\n");
      exit(0);
   }
   if(!validPortNumber(port_number)) {
      perror("Invalid Port Number!\n");
      exit(0);
   }

   listen_sd = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(0);
   }

   /*************************************************************/
   /* Set socket to be nonblocking. All of the sockets for    */
   /* the incoming connections will also be nonblocking since  */
   /* they will inherit that state from the listening socket.   */
   /*************************************************************/
   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(0);
   }

   memset(&addr, 0, sizeof(addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port        = htons(port_number);
   rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(0);
   }

   rc = listen(listen_sd, 32);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(0);
   }

   /*************************************************************/
   /* Initialize the master fd_set                              */
   /*************************************************************/
   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);

   /*************************************************************/
   /* Initialize the timeval struct to 3 minutes.  If no        */
   /* activity after 3 minutes this program will end.           */
   /*************************************************************/
   timeout.tv_sec  = 3 * 60;
   timeout.tv_usec = 0;

   /*************************************************************/
   /* Loop waiting for incoming connects or for incoming data   */
   /* on any of the connected sockets.                          */
   /*************************************************************/
   do
   {
      memcpy(&working_set, &master_set, sizeof(master_set));

      /**********************************************************/
      /* Call select() and wait 3 minutes for it to complete.   */
      /**********************************************************/
      printf("Waiting on select()...\n");
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }

      /**********************************************************/
      /* Check to see if the 3 minute time out expired.         */
      /**********************************************************/
      if (rc == 0)
      {
         printf("  select() timed out.  End program.\n");
         break;
      }

      desc_ready = rc;
      for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
      {
         if (FD_ISSET(i, &working_set))
         {
            desc_ready -= 1;

            /****************************************************/
            /* Check to see if this is the listening socket     */
            /****************************************************/
            if (i == listen_sd)
            {
               printf("Listening socket is readable\n");
               do
               {
                  /**********************************************/
                  /* Accept each incoming connection.  If       */
                  /* accept fails with EWOULDBLOCK, then we     */
                  /* have accepted all of them.  Any other      */
                  /* failure on accept will cause us to end the */
                  /* server.                                    */
                  /**********************************************/
                  new_sd = accept(listen_sd, NULL, NULL);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                     break;
                  }

                  printf("  New incoming connection - %d\n", new_sd);
                  FD_SET(new_sd, &master_set);
                  if (new_sd > max_sd)
                     max_sd = new_sd;

                  } while (new_sd != -1);
            }

            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               /*************************************************/
               /* Receive all incoming data on this socket      */
               /* before we loop back and call select again.    */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Receive data on this connection until the  */
                  /* recv fails with EWOULDBLOCK.  If any other */
                  /* failure occurs, we will close the          */
                  /* connection.                                */
                  /**********************************************/
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  buffer[rc - 1] = '\0';
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }

                  if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     close_conn = TRUE;
                     break;
                  }

                  /**********************************************/
                  /* Data was received                          */
                  /**********************************************/
                  len = rc;
                  printf("  %d bytes received\n", len);
                  char number[BUFF_SIZE];
                  char character[BUFF_SIZE];

                  /**********************************************/
                  /* Echo the data back to the client           */
                  /**********************************************/
                  if(!validMessages(buffer)) {
                     char error[100] = "Message has invalid character!!";
                     char empty[100] = "error";
                     struct iovec iov[2];
                     char buf0[50];
                     char buf1[60];
                     iov[0].iov_base = buf0;
                     iov[1].iov_base = buf1;
                     strcpy(iov[0].iov_base, error);
                     iov[0].iov_len = sizeof(buf0);
                     strcpy(iov[1].iov_base, empty);
                     iov[1].iov_len = sizeof(buf1);
                     bytes_sent = writev(i, iov, 2);
                     if (bytes_sent <= 0){
                        printf("\nConnection closed");
                        break;
                     }
                     continue;
                  } else {
                     numberFromMessage(buffer, number);// get number from message save in number variable
                     charFromMessage(buffer, character);// get char from message save in character
                     struct iovec iov[2];
                     char buf0[50];
                     char buf1[60];
                     iov[0].iov_base = buf0;
                     iov[1].iov_base = buf1;
                     strcpy(iov[0].iov_base, character);
                     iov[0].iov_len = sizeof(buf0);
                     strcpy(iov[1].iov_base, number);
                     iov[1].iov_len = sizeof(buf1);
                     bytes_sent = writev(i, iov, 2);
                     if (bytes_sent <= 0){
                        printf("\nConnection closed");
                        close_conn = TRUE;
                        break;
                     }
                  }

               } while (TRUE);

               if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }
            } /* End of existing connection is readable */
         } /* End of if (FD_ISSET(i, &working_set)) */
      } /* End of loop through selectable descriptors */

   } while (end_server == FALSE);

   /*************************************************************/
   /* Clean up all of the sockets that are open                  */
   /*************************************************************/
   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
   return 0;
}