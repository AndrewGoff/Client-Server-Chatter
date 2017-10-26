#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <string.h>
#include <stdlib.h>

void DieWithError(char *errorMessage); /* Error handling function */

int SendMessage(int socket, char *username)
{
	char msgBuffer[256];
	char msg[240];
	int msgLength;
	int i;

	//Clear buffers
	memset(&msgBuffer, 0, sizeof(msgBuffer));
	memset(&msg, 0, sizeof(msg));
	strtok(username, "\n");
	//Display name as "Name: "; get message
	printf("(Type \"Bye\" to end the chat.)\n");
	printf("%s: ", username);
	fgets(msg, 240, stdin);

	//Create message and send it
	strcat(msgBuffer, username);
	strcat(msgBuffer, ": ");
	strcat(msgBuffer, msg);
	msgLength = strlen(msgBuffer);
	if (send(socket, msgBuffer, msgLength, 0) != msgLength) {
		DieWithError("send() sent a different number of bytes than expected");
	}
	//If message is "Bye" return 1 to stop loop, otherwise return 0
	if (strcmp(msg, "Bye\n") == 0) {
		return 1;
	}
	else {
		return 0;
	}
}