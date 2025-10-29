// Team Members Include: 
// Fiyinfoluwa Afolayan, Ifeoluwakiitan Ayandosu, Inioluwa Ayo-Lawal

#include "server.h"

#define DEFAULT_ROOM "Lobby"

extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;
extern struct node *head;
extern struct room_node *rooms_head;
extern const char *server_MOTD;

char *trimwhitespace(char *str)
{
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  end[1] = '\0';
  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // socket

   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  
   char tmpbuf[MAXBUFF];  
   char cmd[MAXBUFF], username[30];
   char *arguments[80];

   pthread_mutex_lock(&rw_lock);
   sprintf(username,"guest%d", client);
   head = insertFirstU(head, client , username);
   addUserToRoom(rooms_head, DEFAULT_ROOM, username);
   pthread_mutex_unlock(&rw_lock);

   send(client, server_MOTD, strlen(server_MOTD), 0);

   while (1) {
      if ((received = (int)read(client , buffer, MAXBUFF)) > 0) {
            buffer[received] = '\0';
            strcpy(cmd, buffer);
            strcpy(sbuffer, buffer);

            // Tokenize input
            i = 0;
            arguments[i] = strtok(cmd, delimiters);
            while(arguments[i] != NULL) {
               arguments[i] = trimwhitespace(arguments[i]);
               i++;
               arguments[i] = strtok(NULL, delimiters);
            }

            if(i == 0) {
               // empty line
               sprintf(buffer, "\nchat>");
               send(client , buffer , strlen(buffer) , 0 );
               continue;
            }

            // Command handling
            if(strcmp(arguments[0], "create") == 0) {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Usage: create <room>\nchat>");
                  send(client, buffer, strlen(buffer), 0);
                  continue;
               }
               pthread_mutex_lock(&rw_lock);
               rooms_head = createRoom(rooms_head, arguments[1]);
               pthread_mutex_unlock(&rw_lock);
               sprintf(buffer, "Room %s created.\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "join") == 0) {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Usage: join <room>\nchat>");
                  send(client, buffer, strlen(buffer), 0);
                  continue;
               }
               pthread_mutex_lock(&rw_lock);
               struct node *u = findSocketUser(head, client);
               if(u != NULL) {
                  if(findRoom(rooms_head, arguments[1]) != NULL) {
                     addUserToRoom(rooms_head, arguments[1], u->username);
                     sprintf(buffer, "Joined room %s.\nchat>", arguments[1]);
                  } else {
                     sprintf(buffer, "Room %s does not exist.\nchat>", arguments[1]);
                  }
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "leave") == 0) {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Usage: leave <room>\nchat>");
                  send(client, buffer, strlen(buffer), 0);
                  continue;
               }
               pthread_mutex_lock(&rw_lock);
               struct node *u = findSocketUser(head, client);
               if(u != NULL) {
                  if(removeUserFromRoom(rooms_head, arguments[1], u->username)) {
                      sprintf(buffer, "You have left room %s.\nchat>", arguments[1]);
                  } else {
                      sprintf(buffer, "You are not in room %s.\nchat>", arguments[1]);
                  }
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "connect") == 0) {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Usage: connect <user>\nchat>");
                  send(client, buffer, strlen(buffer), 0);
                  continue;
               }
               pthread_mutex_lock(&rw_lock);
               struct node *currentUser = findSocketUser(head, client);
               struct node *targetUser = findU(head, arguments[1]);
               if(currentUser == NULL || targetUser == NULL) {
                  sprintf(buffer, "User not found.\nchat>");
               } else {
                  addDirectConnection(currentUser, targetUser->username);
                  sprintf(buffer, "You are now directly connected to %s.\nchat>", targetUser->username);
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "disconnect") == 0) {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Usage: disconnect <user>\nchat>");
                  send(client, buffer, strlen(buffer), 0);
                  continue;
               }
               pthread_mutex_lock(&rw_lock);
               struct node *currentUser = findSocketUser(head, client);
               if(currentUser == NULL) {
                  sprintf(buffer, "User not found.\nchat>");
               } else {
                  if(removeDirectConnection(currentUser, arguments[1])) {
                     sprintf(buffer, "Disconnected from %s.\nchat>", arguments[1]);
                  } else {
                     sprintf(buffer, "No direct connection with %s.\nchat>", arguments[1]);
                  }
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "rooms") == 0) {
                // read lock
                pthread_mutex_lock(&mutex);
                numReaders++;
                if(numReaders == 1)
                    pthread_mutex_lock(&rw_lock);
                pthread_mutex_unlock(&mutex);

                struct room_node *r = rooms_head;
                strcpy(buffer, "Rooms:\n");
                while(r != NULL) {
                    strcat(buffer, r->roomname);
                    strcat(buffer, "\n");
                    r = r->next;
                }
                strcat(buffer, "chat>");

                pthread_mutex_lock(&mutex);
                numReaders--;
                if(numReaders == 0)
                    pthread_mutex_unlock(&rw_lock);
                pthread_mutex_unlock(&mutex);

                send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "users") == 0) {
                // read lock
                pthread_mutex_lock(&mutex);
                numReaders++;
                if(numReaders == 1)
                    pthread_mutex_lock(&rw_lock);
                pthread_mutex_unlock(&mutex);

                struct node *u = head;
                strcpy(buffer, "Users:\n");
                while(u != NULL) {
                  strcat(buffer, u->username);
                  strcat(buffer, "\n");
                  u = u->next;
                }
                strcat(buffer, "chat>");

                pthread_mutex_lock(&mutex);
                numReaders--;
                if(numReaders == 0)
                    pthread_mutex_unlock(&rw_lock);
                pthread_mutex_unlock(&mutex);

                send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "login") == 0) {
               if(arguments[1] == NULL) {
                  sprintf(buffer, "Usage: login <username>\nchat>");
                  send(client, buffer, strlen(buffer), 0);
                  continue;
               }
               pthread_mutex_lock(&rw_lock);
               struct node *u = findSocketUser(head, client);
               if(u != NULL) {
                   if(findU(head, arguments[1]) == NULL) {
                       char oldname[30];
                       strcpy(oldname, u->username);
                       strcpy(u->username, arguments[1]);
                       removeUserFromAllRooms(rooms_head, oldname);
                       addUserToRoom(rooms_head, DEFAULT_ROOM, u->username);
                       sprintf(buffer, "You are now logged in as %s\nchat>", u->username);
                   } else {
                       sprintf(buffer, "Username %s is already taken.\nchat>", arguments[1]);
                   }
               } else {
                   sprintf(buffer, "Error: User not found.\nchat>");
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "help") == 0 ) {
                sprintf(buffer, "Commands:\nlogin <username>\ncreate <room>\njoin <room>\nleave <room>\nusers\nrooms\nconnect <user>\ndisconnect <user>\nexit\nchat>");
                send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
               pthread_mutex_lock(&rw_lock);
               struct node *currentUser = findSocketUser(head, client);
               if(currentUser != NULL) {
                   removeUserFromAllRooms(rooms_head, currentUser->username);
                   head = removeUser(head, currentUser->username);
               }
               pthread_mutex_unlock(&rw_lock);
               close(client);
               pthread_exit(NULL);
            }
            else {
                 // Normal message broadcast
                 pthread_mutex_lock(&mutex);
                 numReaders++;
                 if(numReaders == 1) pthread_mutex_lock(&rw_lock);
                 pthread_mutex_unlock(&mutex);

                 struct node *currentUser = findSocketUser(head, client);
                 if(currentUser != NULL) {
                     sprintf(tmpbuf,"\n::%s> %s\nchat>", currentUser->username, sbuffer);
                     strcpy(sbuffer, tmpbuf);

                     int *recipients = getRecipients(head, rooms_head, currentUser->username);
                     for(int j=0; recipients[j] != -1; j++) {
                         if(recipients[j] != client) {
                             send(recipients[j], sbuffer, strlen(sbuffer), 0);
                         }
                     }
                     free(recipients);
                 }

                 pthread_mutex_lock(&mutex);
                 numReaders--;
                 if(numReaders == 0) pthread_mutex_unlock(&rw_lock);
                 pthread_mutex_unlock(&mutex);
            }

         memset(buffer, 0, sizeof(buffer));
         memset(sbuffer, 0, sizeof(sbuffer));
      } else {
         // Client disconnected unexpectedly
         pthread_mutex_lock(&rw_lock);
         struct node *currentUser = findSocketUser(head, client);
         if(currentUser != NULL) {
             removeUserFromAllRooms(rooms_head, currentUser->username);
             head = removeUser(head, currentUser->username);
         }
         pthread_mutex_unlock(&rw_lock);
         close(client);
         pthread_exit(NULL);
      }
   }
   return NULL;
}