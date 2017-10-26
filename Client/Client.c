#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

void DieWithError(char *errorMessage); /* Error handling function */
int SendMessage(int socket, char *username);
int ReceiveMessage(int socket);

int main(int argc, char *argv[])
{
    int sock; /* Socket descriptor */
	int clientSock;
	int connectionStatus;
    struct sockaddr_in targetServAddr; /* Echo server address */
	struct sockaddr_in clientServAddr;
    unsigned short targetServPort; /* Echo server port */
	unsigned short clientServPort;
	unsigned int clientLen;
	char username[12];		/*target username*/
	char password[12];		/*password*/
	char myUsername[12];	/*own username*/
    char servlP[34];		/* Server IP address (dotted quad) */
	char port[34];			/* Server port*/
	char selection[2];		/*Menu Option*/
	char message[256];		/*Message being sent out*/
	char response[256];		/*Message being received*/
	char msg[240];			/*Message pre-concatenation*/
	int responselen;
	int i, count;


	connectionStatus = 0;
	while (1) {
		//Display options
		printf("\n0. Connect to the server\n");
		printf("1. Display user list\n");
		printf("2. Leave a user a message\n");
		printf("3. Retrieve my messages\n");
		printf("4. Initiate a chat with my friend\n");
		printf("5. Chat with my friend\n");
		printf("Please make a selection: ");
		fgets(selection, 3, stdin);

		switch (atoi(selection)) {
		//Conect to Server
		case 0:
			//If already connected, disconnect first
			if (connectionStatus) {
				memset(&message, 0, sizeof(message));
				strcat(message, "9");
				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}
				connectionStatus = 0;
				close(sock);
			}
			//Read in IP and Port
			printf("\nPlease enter the server IP: ");
			fgets(servlP, 256, stdin);
			printf("Please enter the port number: ");
			fgets(port, 256, stdin);
			targetServPort = atoi(port);

			/* Create a reliable, stream socket using TCP */
			if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
				DieWithError(" socket () failed");

			/* Construct the server address structure */
			memset(&targetServAddr, 0, sizeof(targetServAddr)); /* Zero out structure */
			targetServAddr.sin_family = AF_INET; /* Internet address family */
			targetServAddr.sin_addr.s_addr = inet_addr(servlP); /* Server IP address */
			targetServAddr.sin_port = htons(targetServPort); /* Server port */

			 /* Establish the connection to the echo server */
			if (connect(sock, (struct sockaddr *) &targetServAddr, sizeof(targetServAddr)) < 0) {
				DieWithError(" connect () failed");
			}
			//Upon successful connection
			else {
				printf("Connected!\n\n");
				//Get Username and Password
				memset(&myUsername, 0, sizeof(myUsername));
				memset(&password, 0, sizeof(password));
				printf("Please enter your username: ");
				fgets(myUsername, 12, stdin);
				strtok(myUsername, "\n");
				printf("Please enter your password: ");
				fgets(password, 12, stdin);

				//Prepare message
				memset(&message, 0, sizeof(message));
				strcat(message, "0");
				strcat(message, myUsername);
				strcat(message, "-");
				strcat(message, password);

				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}
				//If message was sent sucessfully, check for response
				else {
					i = 0;
					while (i == 0) {
						if ((responselen = recv(sock, response, 256, 0)) > 0) {
							i = 1;
						}
					}
					//Username and Password are valid, set status
					if (strcmp(response, "True") == 0) {
						connectionStatus = 1;
						printf("Welcome!\n");
					}
					//Username and Password are invalid, disconnect and close socket
					else {
						printf("Invalid username or password.\n");
						memset(&message, 0, sizeof(message));
						strcat(message, "9");
						if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
							DieWithError("send() failed");
						}
						connectionStatus = 0;
						close(sock);
					}
				}
			}
			break;
		//Get User List
		case 1:
			//If connected
			if (connectionStatus) {
				memset(&message, 0, sizeof(message));
				memset(&response, 0, sizeof(response));
				//Send message of 1 for User list
				strcat(message, "1");
				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}

				//On successful message being sent, receive list until end is received
				else {
					printf("\nHere are the users:\n");
					i = 0;
					while (i == 0) {
						if ((responselen = recv(sock, response, 256, 0)) > 0) {
							if (strcmp(response, "End") == 0) {
								i = 1;
							}
							else {
								printf("%s\n", response);
							}
							memset(&response, 0, sizeof(response));
						}
					}
				}
			}
			//If not logged in, display error
			else {
				printf("Please log into the server.\n");
			}
			break;
		//Leave a user a message
		case 2:
			//If connected
			if (connectionStatus) {
				memset(&username, 0, sizeof(username));
				memset(&msg, 0, sizeof(msg));
				//Get target's username and the message to be sent
				printf("Please enter the recipient's username: ");
				fgets(username, 12, stdin);
				strtok(username, "\n");
				printf("Please enter your message: ");
				fgets(msg, 240, stdin);

				//Prepare the message
				memset(&message, 0, sizeof(message));
				strcat(message, "2");
				strcat(message, username);
				strcat(message, "-");
				strcat(message, msg);

				//Send the message
				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}
				else {
					printf("Message sent!\n");
				}
			}
			//If not logged in, display error
			else {
				printf("Please log into the server.\n");
			}
			break;
		//Retrieve messages
		case 3:
			//If connected
			if (connectionStatus) {
				memset(&message, 0, sizeof(message));
				memset(&response, 0, sizeof(response));
				strcat(message, "3");
				strcat(message, myUsername);
				//Send a message of 3 to signal retrieve messages along with user's username
				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}
				//On success, display messages until End is received
				else {
					printf("\nHere are your messages:\n");
					i = 0;
					count = 1;
					while (i == 0) {
						if ((responselen = recv(sock, response, 256, 0)) > 0) {
							if (strcmp(response, "End") == 0) {
								i = 1;
							}
							else {
								printf("%d. %s", count, response);
								count++;
							}
							memset(&response, 0, sizeof(response));
						}
					}
				}
			}
			//If not logged in, display error
			else {
				printf("Please log into the server.\n");
			}
			break;
		//Start listen server
		case 4:
			//If connected
			if (connectionStatus) {
				//Disconnect from server first and close socket
				memset(&message, 0, sizeof(message));
				strcat(message, "9");
				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}
				connectionStatus = 0;
				close(sock);
				printf("-----Disconnected-----\n");

				//Get desired port number and start server
				printf("Please enter a port number: ");
				fgets(port, 256, stdin);
				clientServPort = atoi(port);
				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError("socket () failed");

				memset(&clientServAddr, 0, sizeof(clientServAddr)); /* Zero out structure */
				clientServAddr.sin_family = AF_INET; /* Internet address family */
				clientServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
				clientServAddr.sin_port = htons(clientServPort); /* Local port */

				if (bind(sock, (struct sockaddr *)&clientServAddr, sizeof(clientServAddr)) < 0) {
					DieWithError("bind () failed");
				}
				else {
					if (listen(sock, 5) < 0)
						DieWithError("listen() failed");
					
					//Wait for messages
					printf("Waiting for messages.\n\n");
					i = 0;
					while(i == 0) //Run until "Bye" is received
					{
						clientLen = sizeof(targetServAddr);
						if ((clientSock = accept(sock, (struct sockaddr *) &targetServAddr, &clientLen)) < 0)
							DieWithError("accept() failed");
						else {
							while(i == 0) {
								i = ReceiveMessage(clientSock);
								if (i == 0) {
									i = SendMessage(clientSock, myUsername);
								}
							}
						}
					}
					//Close sockets once finished receiving messages
					close(sock);
				}
			}
			//If not logged in, display error
			else {
				printf("Please log into the server.\n");
			}
			break;
		//Connect to a friend to message them
		case 5:
			//If connected
			if (connectionStatus) {
				//Disconnect from server first and close socket
				memset(&message, 0, sizeof(message));
				strcat(message, "9");
				if (send(sock, message, sizeof(message), 0) != sizeof(message)) {
					DieWithError("send() failed");
				}
				connectionStatus = 0;
				close(sock);
				printf("-----Disconnected-----\n");

				//Get desired IP and port number
				printf("Please enter the server IP: ");
				fgets(servlP, 256, stdin);
				printf("Please enter the port number: ");
				fgets(port, 256, stdin);
				targetServPort = atoi(port);

			/* Create a reliable, stream socket using TCP */
				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError(" socket () failed");

				/* Construct the server address structure */
				memset(&targetServAddr, 0, sizeof(targetServAddr)); /* Zero out structure */
				targetServAddr.sin_family = AF_INET; /* Internet address family */
				targetServAddr.sin_addr.s_addr = inet_addr(servlP); /* Server IP address */
				targetServAddr.sin_port = htons(targetServPort); /* Server port */

			/* Establish the connection to the echo server */
				if (connect(sock, (struct sockaddr *) &targetServAddr, sizeof(targetServAddr)) < 0) {
					DieWithError(" connect () failed");
				}
				else {
					//Continue messaging until "Bye" is received
					printf("Connected!\n\n");
					i = 0;
					while (i == 0) {
						i = SendMessage(sock, myUsername);
						if (i == 0) {
							i = ReceiveMessage(sock);
						}
					}
					close(sock);
				}
			}
			//If not logged in, display error
			else {
				printf("Please log into the server.\n");
			}
			break;
		//Default message for invalid choices
		default:
			printf("Please enter a valid selection\n");
		}
	}
}
