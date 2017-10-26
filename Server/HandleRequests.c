#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include <string.h>

void DieWithError(char *errorMessage); /* Error handling function */

int HandleRequests(int clntSocket)
{
	int i;
	char echoBuffer[256]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */
	char *str;
	char response[256];
	char holder[18];
	FILE *fp;

	memset(&echoBuffer, 0, sizeof(echoBuffer));
	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, 256, 0)) < 0)
		DieWithError("recv() failed");

	switch (echoBuffer[0]) {
	//Check if valid login and password
	case '0':
		//Prepare str to get text file name
		str = strtok(echoBuffer, "-");
		str = strtok(NULL, "\n");

		for (i = 0; i < strlen(echoBuffer); i++) {
			echoBuffer[i] = echoBuffer[i + 1];
		}
		echoBuffer[strlen(echoBuffer)] = 0;
		//Open text file
		fp = fopen("users.txt", "r");
		//If file not found
		if (fp == NULL) {
			printf("File not found\n");
			memset(&response, 0, sizeof(response));
			strcat(response, "False");
		}
		else {
			memset(&response, 0, sizeof(response));
			strcat(response, "False");

			//Check for valid username, then check for valid password
			while (fscanf(fp, "%s", holder) != EOF) {
				if (strcmp(echoBuffer, holder) == 0) {
					memset(&holder, 0, sizeof(holder));
					fscanf(fp, "%s", holder);
					if (strcmp(str, holder) == 0) {
						memset(&response, 0, sizeof(response));
						strcat(response, "True");
						printf("User %s logged in.\n", echoBuffer);
					}
				}
				else {
					fscanf(fp, "%d");
				}
			}
			//Close file and return result of search
			fclose(fp);
		}
		if (send(clntSocket, response, sizeof(response), 0) != sizeof(response))
			DieWithError("send() failed");
		
		break;
	//Display user list
	case '1':
		//Open file
		fp = fopen("users.txt", "r");
		if (fp == NULL) {
			printf("File not found.\n");
		}
		else {
			memset(&response, 0, sizeof(response));
			printf("Displaying user list.\n");
			//Display users until EOF is reached
			while (fscanf(fp, "%s", response) != EOF) {
				fscanf(fp, "%d");
				if (send(clntSocket, response, sizeof(response), 0) != sizeof(response))
					DieWithError("send() failed");
				memset(&response, 0, sizeof(response));
			}
			//Close file and send "End" to signal end of users
			fclose(fp);
			memset(&response, 0, sizeof(response));
			strcat(response, "End");
			if (send(clntSocket, response, sizeof(response), 0) != sizeof(response))
				DieWithError("send() failed");
		}
		break;
	//Send message to a user
	case '2':
		//Prepare username for text file
		str = strtok(echoBuffer, "-");
		str = strtok(NULL, "\n");
		for (i = 0; i < strlen(echoBuffer); i++) {
			echoBuffer[i] = echoBuffer[i + 1];
		}
		echoBuffer[strlen(echoBuffer)] = 0;
		printf("Leaving message for %s: %s\n", echoBuffer, str);
		memset(&holder, 0, sizeof(holder));
		strcat(holder, echoBuffer);
		strcat(holder, ".txt");

		fp = fopen(holder, "a");
		if (fp == NULL) {
			printf("File not found\n");
		}
		else {
			//Write message to username
			fprintf(fp, "%s\n", str);

			fclose(fp);
		}
		break;
	//Retrieve messages
	case '3':
		for (i = 0; i < strlen(echoBuffer); i++) {
			echoBuffer[i] = echoBuffer[i + 1];
		}
		echoBuffer[strlen(echoBuffer)] = 0;
		printf("Retrieving messages for %s.\n", echoBuffer);
		strcat(echoBuffer, ".txt");
		fp = fopen(echoBuffer, "r");
		if (fp == NULL) {
			printf("File not found\n");
			memset(&response, 0, sizeof(response));
			strcat(response, "End");
			if (send(clntSocket, response, sizeof(response), 0) != sizeof(response))
				DieWithError("send() failed");
		}
		else {
			memset(&response, 0, sizeof(response));
			//Get messages until EOF
			while (fgets(response, 255, fp) != NULL) {
				if (send(clntSocket, response, sizeof(response), 0) != sizeof(response))
					DieWithError("send() failed");
				memset(&response, 0, sizeof(response));
			}
			//Send "End" to signal no more messages
			memset(&response, 0, sizeof(response));
			strcat(response, "End");
			if (send(clntSocket, response, sizeof(response), 0) != sizeof(response))
				DieWithError("send() failed");
			//Close file
			fclose(fp);
		}
		break;
	//Mesage used to close socket when a user disconnects
	case '9':
		printf("User disconnected.\n");
		close(clntSocket);
		return 0;
		break;
	}
	return 1;
}
