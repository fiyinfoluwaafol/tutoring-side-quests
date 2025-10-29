// Team Members Include: 
// Fiyinfoluwa Afolayan, Ifeoluwakiitan Ayandosu, Inioluwa Ayo-Lawal


#include "server.h"

int chat_serv_sock_fd;
int numReaders = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  

struct node *head = NULL;
struct room_node *rooms_head = NULL;

const char *server_MOTD = "Thanks for connecting to the BisonChat Server.\nType 'help' for commands.\n\nchat>";

int main(int argc, char **argv) {
   signal(SIGINT, sigintHandler);

   pthread_mutex_lock(&rw_lock);
   rooms_head = createRoom(rooms_head, DEFAULT_ROOM);
   pthread_mutex_unlock(&rw_lock);

   chat_serv_sock_fd = get_server_socket();
   if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
      printf("start server error\n");
      exit(1);
   }

   printf("Server Launched! Listening on PORT: %d\n", PORT);

   while(1) {
      int new_client = accept_client(chat_serv_sock_fd);
      if(new_client != -1) {
         pthread_t new_client_thread;
         pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
         pthread_detach(new_client_thread);
      }
   }

   close(chat_serv_sock_fd);
}

int get_server_socket() {
    int opt = TRUE;   
    int master_socket;
    struct sockaddr_in address; 
    
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   

    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons(PORT);   

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   

   return master_socket;
}

int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      printf("socket listen error\n");
   }
   return status;
}

int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;

   if ((reply_sock_fd = accept(serv_sock,(struct sockaddr *)&client_addr, &sin_size)) == -1) {
      printf("socket accept error\n");
   }
   return reply_sock_fd;
}

void sigintHandler(int sig_num) {
   printf("Server shutting down gracefully...\n");
   pthread_mutex_lock(&rw_lock);

   struct node *u = head;
   while(u != NULL) {
      close(u->socket);
      u = u->next;
   }

   freeAllUsers(head);
   head = NULL;
   freeAllRooms(rooms_head);
   rooms_head = NULL;

   pthread_mutex_unlock(&rw_lock);

   close(chat_serv_sock_fd);
   exit(0);
}