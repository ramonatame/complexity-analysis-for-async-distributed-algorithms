/*
 * message_type.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */

#include "message_type.h"

Message* createMessage(int dest, int length, int* content) {
	Message *message = (Message*) malloc(sizeof(Message));
	message->dest = dest;
	message->length = length;
	int* copy = (int*) malloc(sizeof(int) * length);
	memcpy(copy, content, length * sizeof(int));
	message->content = copy;
	return message;
}

void printMessage(Message *message) {
	printf("Length %d, dest/sender =  %d: \n", message->length, message->dest);
	int i = 0;
	for (; i < message->length; ++i) {
		printf("%d, ", message->content[i]);
	}
	printf("\n");
}

void deallocateMessage(Message* message) {
	free(message->content);
	free(message);
}

int* serializeMessage(Message* message) {
	int *serializedMessage = (int*) malloc(sizeof(int) * (message->length + 2));
	serializedMessage[0] = message->dest;
	serializedMessage[1] = message->length;
	int i;
	for (i = 0; i < message->length; ++i) {
		serializedMessage[i + 2] = message->content[i];
	}
	return serializedMessage;
}

Message* deserializeMessage(int* serializedMessage) {
	Message* message;
	if (serializedMessage != NULL) {
		message = createMessage(serializedMessage[0], serializedMessage[1], &serializedMessage[2]);
	}
	return message;
}

void printSerializedMessage(int* message) {
	if (message != NULL) {
		printf("Message for dest %d, length = %d; Elements: ", message[0], message[1]);
		int i;
		for (i = 0; i < message[1]; ++i) {
			printf("%d ", message[i + 2]);
		}
		printf("\n");
	}
}
