#include <netinet/in.h> // Structure for storing address information 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h> // For socket APIs 
#include <sys/types.h> 
#include <unistd.h>     // For close()
#include "list.h"

#define PORT 9001
#define ACK "ACK"
      
int main(int argc, char const* argv[]) 
{ 
    int n, val, idx;
    // Create server socket 
    int servSockD = socket(AF_INET, SOCK_STREAM, 0); 

    // Buffers for receiving and sending data 
    char buf[1024];
    char sbuf[1024];
    char* token;

    // Define server address 
    struct sockaddr_in servAddr; 

    // Linked list 
    list_t *mylist;

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(PORT); 
    servAddr.sin_addr.s_addr = INADDR_ANY; 

    // Bind socket to the specified IP and port 
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections 
    if (listen(servSockD, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept a client socket 
    int clientSocket = accept(servSockD, NULL, NULL); 
    if (clientSocket < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    // Create the linked list 
    mylist = list_alloc();

    while(1){
        // Receive messages from client socket 
        n = recv(clientSocket, buf, sizeof(buf), 0);
        if (n <= 0) {
            perror("recv failed or connection closed");
            break;
        }
        buf[n] = '\0';
        memset(sbuf, '\0', sizeof(sbuf)); // Clear the send buffer

        // Tokenize the input command 
        token = strtok(buf, " ");
        
        if(strcmp(token,"exit") == 0){
            list_free(mylist);
            sprintf(sbuf, "Exiting...");
            send(clientSocket, sbuf, sizeof(sbuf), 0);
            close(clientSocket);
            close(servSockD);
            exit(0);
        }
        else if(strcmp(token,"get_length") == 0){
            // Call list_length() 
            val = list_length(mylist);
            sprintf(sbuf,"%s%d", "Length = ", val);
        }
        else if(strcmp(token,"add_front") == 0){
            token = strtok(NULL, " ");  // Get the value 
            if (token != NULL) {
                val = atoi(token);
                list_add_to_front(mylist,val);
                sprintf(sbuf,"%s", ACK);
            } else {
                sprintf(sbuf, "Error: Missing value");
            }
        }
        else if(strcmp(token,"add_back") == 0){
            token = strtok(NULL, " ");  // Get the value 
            if (token != NULL) {
                val = atoi(token);
                list_add_to_back(mylist,val);
                sprintf(sbuf,"%s", ACK);
            } else {
                sprintf(sbuf, "Error: Missing value");
            }
        }
        else if(strcmp(token,"add_position") == 0){
            token = strtok(NULL, " ");  // Get the index 
            if (token != NULL) {
                idx = atoi(token);
                token = strtok(NULL, " ");  // Get the value 
                if (token != NULL) {
                    val = atoi(token);
                    list_add_at_index(mylist,val, idx);
                    sprintf(sbuf,"%s", ACK);
                } else {
                    sprintf(sbuf, "Error: Missing value");
                }
            } else {
                sprintf(sbuf, "Error: Missing index");
            }
        }
        else if(strcmp(token,"remove_back") == 0){
            // Call list_remove_from_back() 
            val = list_remove_from_back(mylist);
            if (val != -1)
                sprintf(sbuf,"%s%d", "Removed = ", val);
            else
                sprintf(sbuf,"Error: List is empty");
        }
        else if(strcmp(token,"remove_front") == 0){
            // Call list_remove_from_front() 
            val = list_remove_from_front(mylist);
            if (val != -1)
                sprintf(sbuf,"%s%d", "Removed = ", val);
            else
                sprintf(sbuf,"Error: List is empty");
        }
        else if(strcmp(token,"remove_position") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                idx = atoi(token);
                // Call list_remove_at_index() 
                val = list_remove_at_index(mylist,idx);
                if (val != -1)
                    sprintf(sbuf,"%s%d", "Removed = ", val);
                else
                    sprintf(sbuf,"Error: Invalid index");
            } else {
                sprintf(sbuf, "Error: Missing index");
            }
        }
        else if(strcmp(token,"get") == 0){
            token = strtok(NULL, " ");  // Get the index 
            if (token != NULL) {
                idx = atoi(token);
                // Call list_get_elem_at() 
                val = list_get_elem_at(mylist, idx);
                if (val != -1)
                    sprintf(sbuf,"%s%d", "Value = ", val);
                else
                    sprintf(sbuf,"Error: Invalid index");
            } else {
                sprintf(sbuf, "Error: Missing index");
            }
        }
        else if(strcmp(token,"print") == 0){
            // Call listToString() 
            sprintf(sbuf,"%s", listToString(mylist));
        }
        else {
            sprintf(sbuf, "Invalid command");
        }

        // Send response to client socket 
        send(clientSocket, sbuf, sizeof(sbuf), 0);

        memset(buf, '\0', sizeof(buf)); // Clear the receive buffer
    }

    // Clean up in case of loop exit 
    list_free(mylist);
    close(clientSocket);
    close(servSockD);

    return 0; 
}


// #include <netinet/in.h> //structure for storing address information 
// #include <stdio.h> 
// #include <string.h>
// #include <stdlib.h> 
// #include <sys/socket.h> //for socket APIs 
// #include <sys/types.h> 
// #include "list.h"

// #define PORT 9001
// #define ACK "ACK"
  
// int main(int argc, char const* argv[]) 
// { 
  
// 	  int n, val, idx;
//     // create server socket similar to what was done in 
//     // client program 
//     int servSockD = socket(AF_INET, SOCK_STREAM, 0); 
  
//     // string store data to recv/send to/from client 
//     char buf[1024];
// 		char sbuf[1024];
//     char* token;

//     // define server address 
//     struct sockaddr_in servAddr; 
  
// 	  // list
// 		list_t *mylist;


//     servAddr.sin_family = AF_INET; 
//     servAddr.sin_port = htons(PORT); 
//     servAddr.sin_addr.s_addr = INADDR_ANY; 
  
//     // bind socket to the specified IP and port 
//     bind(servSockD, (struct sockaddr*)&servAddr, 
//          sizeof(servAddr)); 
  
//     // listen for connections 
//     listen(servSockD, 1); 
  
//     // integer to hold client socket. 
//     int clientSocket = accept(servSockD, NULL, NULL); 

//     mylist = list_alloc();  // create the list

//     while(1){
//         // recvs messages from client socket 
//         n = recv(clientSocket, buf, sizeof(buf), 0);
// 				buf[n] = '\0';

//         if(n > 0){
//             token = strtok(buf, " ");
    
// 						if(strcmp(token,"exit") == 0){
// 							  list_free(mylist);
// 							  exit(1);
// 						}
// 						else if(strcmp(token,"get_length") == 0){
// 								// Make a Call
// 								val = list_length(mylist);
// 								sprintf(sbuf,"%s%d", "Length = ", val);
// 						}
// 						else if(strcmp(token,"add_front") == 0){
// 							  token = strtok(NULL, " ");  // get next token (value)
// 								val = atoi(token);
// 								// Make a Call
// 								sprintf(sbuf,"%s%d", ACK, val);
// 								list_add_to_front(mylist,val);
// 						}
// 						else if(strcmp(token,"remove_position") == 0){
// 							  token = strtok(NULL, " ");
// 								idx = atoi(token);
// 								// Make a Call
// 								val = list_remove_at_index(mylist,idx);
// 								sprintf(sbuf,"%s%d", ACK, val);
// 						}
// 						else if(strcmp(token,"print") == 0){
// 							  sprintf(sbuf,"%s", listToString(mylist));
// 						}
// 						// ADD THE OTHER OPERATIONS


//                 // send's messages to client socket 
//             send(clientSocket, sbuf, sizeof(sbuf), 0);
//         }
// 				memset(buf, '\0', 1024);
//     }
  
//     return 0; 
// }
