#ifndef LIST_H
#define LIST_H

// Team Members Include: 
// Fiyinfoluwa Afolayan, Ifeoluwakiitan Ayandosu, Inioluwa Ayo-Lawal

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/************************************
 * User Linked List
 ************************************/
struct dm_node {
   char username[30];
   struct dm_node *next;
};

struct node {
   char username[30];
   int socket;
   struct dm_node *dms;
   struct node *next;
};

/************************************
 * Room User Linked List
 ************************************/
struct room_user_node {
    char username[30];
    struct room_user_node *next;
};

/************************************
 * Room Linked List
 ************************************/
struct room_node {
    char roomname[30];
    struct room_user_node *users;
    struct room_node *next;
};

/*****************************************
 * USER LIST FUNCTIONS
 *****************************************/
struct node* insertFirstU(struct node *head, int socket, char *username);
struct node* findU(struct node *head, char* username);
struct node* findSocketUser(struct node *head, int socket);
struct node* removeUser(struct node *head, char *username);
void freeAllUsers(struct node *head);

/* DM Functions */
void addDirectConnection(struct node *u, char *targetname);
bool removeDirectConnection(struct node *u, char *targetname);
void removeAllDirectConnections(struct node *u);

/*****************************************
 * ROOM LIST FUNCTIONS
 *****************************************/
struct room_node* createRoom(struct room_node *head, char *roomname);
struct room_node* findRoom(struct room_node *head, char *roomname);
bool addUserToRoom(struct room_node *head, char *roomname, char *username);
bool removeUserFromRoom(struct room_node *head, char *roomname, char *username);
void removeUserFromAllRooms(struct room_node *head, char *username);
void freeAllRooms(struct room_node *head);
bool userInRoom(struct room_node *rooms_head, char *username, char *roomname);

/*****************************************
 * HELPER FUNCTIONS
 *****************************************/
int* getRecipients(struct node *head, struct room_node *rooms_head, char *username);

#endif