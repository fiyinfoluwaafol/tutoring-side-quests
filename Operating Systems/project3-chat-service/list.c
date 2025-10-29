// Team Members Include: 
// Fiyinfoluwa Afolayan, Ifeoluwakiitan Ayandosu, Inioluwa Ayo-Lawal

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

/*****************************************
 * USER LIST FUNCTIONS
 *****************************************/
struct node* insertFirstU(struct node *head, int socket, char *username) {
   if(findU(head,username) == NULL) {
       struct node *link = (struct node*) malloc(sizeof(struct node));
       link->socket = socket;
       strcpy(link->username,username);
       link->dms = NULL;
       link->next = head;
       head = link;
   }
   return head;
}

struct node* findU(struct node *head, char* username) {
   struct node* current = head;
   while(current != NULL) {
      if(strcmp(current->username, username) == 0)
         return current;
      current = current->next;
   }
   return NULL;
}

struct node* findSocketUser(struct node *head, int socket) {
    struct node *current = head;
    while(current != NULL) {
        if(current->socket == socket)
            return current;
        current = current->next;
    }
    return NULL;
}

struct node* removeUser(struct node *head, char *username) {
    struct node *current = head;
    struct node *prev = NULL;
    while(current != NULL) {
        if(strcmp(current->username, username) == 0) {
            if(prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            removeAllDirectConnections(current);
            free(current);
            return head;
        }
        prev = current;
        current = current->next;
    }
    return head;
}

void freeAllUsers(struct node *head) {
    struct node *current = head;
    while(current != NULL) {
        struct node *temp = current;
        current = current->next;
        removeAllDirectConnections(temp);
        free(temp);
    }
}

/*****************************************
 * DIRECT CONNECTIONS (DM)
 *****************************************/
void addDirectConnection(struct node *u, char *targetname) {
    struct dm_node *d = u->dms;
    while(d != NULL) {
        if(strcmp(d->username, targetname) == 0) return;
        d = d->next;
    }
    struct dm_node *newdm = malloc(sizeof(struct dm_node));
    strcpy(newdm->username, targetname);
    newdm->next = u->dms;
    u->dms = newdm;
}

bool removeDirectConnection(struct node *u, char *targetname) {
    struct dm_node *current = u->dms;
    struct dm_node *prev = NULL;
    while(current != NULL) {
        if(strcmp(current->username, targetname) == 0) {
            if(prev == NULL) {
                u->dms = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

void removeAllDirectConnections(struct node *u) {
    struct dm_node *d = u->dms;
    while(d != NULL) {
        struct dm_node *temp = d;
        d = d->next;
        free(temp);
    }
    u->dms = NULL;
}

/*****************************************
 * ROOM FUNCTIONS
 *****************************************/
struct room_node* createRoom(struct room_node *head, char *roomname) {
    struct room_node *r = findRoom(head, roomname);
    if(r != NULL) return head;

    struct room_node *newroom = malloc(sizeof(struct room_node));
    strcpy(newroom->roomname, roomname);
    newroom->users = NULL;
    newroom->next = head;
    head = newroom;
    return head;
}

struct room_node* findRoom(struct room_node *head, char *roomname) {
    struct room_node *r = head;
    while(r != NULL) {
        if(strcmp(r->roomname, roomname) == 0) return r;
        r = r->next;
    }
    return NULL;
}

bool addUserToRoom(struct room_node *head, char *roomname, char *username) {
    struct room_node *r = findRoom(head, roomname);
    if(r == NULL) return false;

    struct room_user_node *u = r->users;
    while(u != NULL) {
        if(strcmp(u->username, username) == 0) return true;
        u = u->next;
    }

    struct room_user_node *newu = malloc(sizeof(struct room_user_node));
    strcpy(newu->username, username);
    newu->next = r->users;
    r->users = newu;
    return true;
}

bool removeUserFromRoom(struct room_node *head, char *roomname, char *username) {
    struct room_node *r = findRoom(head, roomname);
    if(r == NULL) return false;

    struct room_user_node *current = r->users;
    struct room_user_node *prev = NULL;
    while(current != NULL) {
        if(strcmp(current->username, username) == 0) {
            if(prev == NULL) {
                r->users = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

void removeUserFromAllRooms(struct room_node *head, char *username) {
    struct room_node *r = head;
    while(r != NULL) {
        removeUserFromRoom(head, r->roomname, username);
        r = r->next;
    }
}

void freeAllRooms(struct room_node *head) {
    while(head != NULL) {
        struct room_node *temp = head;
        head = head->next;

        struct room_user_node *u = temp->users;
        while(u != NULL) {
            struct room_user_node *tu = u;
            u = u->next;
            free(tu);
        }

        free(temp);
    }
}

bool userInRoom(struct room_node *rooms_head, char *username, char *roomname) {
    struct room_node *r = findRoom(rooms_head, roomname);
    if(r == NULL) return false;
    struct room_user_node *u = r->users;
    while(u != NULL) {
        if(strcmp(u->username, username) == 0) return true;
        u = u->next;
    }
    return false;
}

/*****************************************
 * HELPER FUNCTIONS
 *****************************************/
int* getRecipients(struct node *head, struct room_node *rooms_head, char *username) {
    struct node *sender = findU(head, username);
    if(!sender) {
        int *arr = malloc(sizeof(int)*1);
        arr[0] = -1;
        return arr;
    }

    int cap = 100;
    int count = 0;
    int *sockets = malloc(sizeof(int)*cap);

    // Add DM connections
    struct dm_node *d = sender->dms;
    while(d != NULL) {
        struct node *u = findU(head, d->username);
        if(u != NULL) {
            bool found = false;
            for(int i=0; i<count; i++){
                if(sockets[i] == u->socket) {found = true; break;}
            }
            if(!found) {
                if(count == cap) {
                    cap *= 2;
                    sockets = realloc(sockets, sizeof(int)*cap);
                }
                sockets[count++] = u->socket;
            }
        }
        d = d->next;
    }

    // Add room members
    struct room_node *r = rooms_head;
    while(r != NULL) {
        if(userInRoom(rooms_head, username, r->roomname)) {
            struct room_user_node *ru = r->users;
            while(ru != NULL) {
                if(strcmp(ru->username, username) != 0) {
                    struct node *u = findU(head, ru->username);
                    if(u != NULL) {
                        bool found = false;
                        for(int i=0; i<count; i++){
                            if(sockets[i] == u->socket) {found = true; break;}
                        }
                        if(!found) {
                            if(count == cap) {
                                cap *= 2;
                                sockets = realloc(sockets, sizeof(int)*cap);
                            }
                            sockets[count++] = u->socket;
                        }
                    }
                }
                ru = ru->next;
            }
        }
        r = r->next;
    }

    if(count == cap) {
        sockets = realloc(sockets, sizeof(int)*(cap+1));
    }
    sockets[count] = -1;
    return sockets;
}