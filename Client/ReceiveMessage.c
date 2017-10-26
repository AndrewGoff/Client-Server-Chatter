#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <string.h>

void DieWithError(char *errorMessage); /* Error handling function */

int ReceiveMessage(int socket)
{
	char msgBuffer[256];
	char *str;
	int msgLength = 0;
	int i;

	//Clear buffer, receive message
	memset(&msgBuffer, 0, sizeof(msgBuffer));
	while (msgLength == 0) {
		if ((msgLength = recv(socket, msgBuffer, 256, 0)) < 0) {
			DieWithError("recv() failed");
		}
	}
	//Display message
	printf("%s", msgBuffer);
	strtok(msgBuffer, " ");
	str = strtok(NULL, "\n");
	//If message is bye, return 1 to exit loop, otherwise return 0
	if (strcmp(str, "Bye") == 0) {
		return 1;
	}
	else {
		return 0;
	}
}