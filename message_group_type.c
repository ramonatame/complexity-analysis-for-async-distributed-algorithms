/*
 * message_group_type.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */
#include "message_group_type.h"

Messages_Group* createGroupOfMessages(int length, int numberOfElements) {
	Messages_Group *group = (Messages_Group*) malloc(sizeof(Messages_Group));
	group->length = length;
	group->numberOfElements = numberOfElements;
	group->messages = malloc(sizeof(Message) * length);
	return group;
}

void addMessageToGroup(Messages_Group* group, Message *message) {
	if (group->numberOfElements < group->length) {
		group->messages[group->numberOfElements] = *message;
		++(group->numberOfElements);
	}
}

void printGroupOfMessage(Messages_Group *group) {
	printf("New group of message of length %d: \n", group->numberOfElements);
	int i = 0;
	for (; i < group->numberOfElements; ++i) {
		printMessage(&group->messages[i]);
	}
	printf("\n");
}

int getLengthOfMessages_Group(Messages_Group *group) {
	int i, groupLength = 0;
	for (i = 0; i < group->numberOfElements; ++i) {
		groupLength += (group->messages[i].length + 2);
	}
	return groupLength + 1;
}

int* serializeGroupOfMessages(Messages_Group *group) {
	int i, j, groupLength = 0, lastIndex = 0;
	for (i = 0; i < group->numberOfElements; ++i) {
		groupLength += (group->messages[i].length + 2);
	}
	int* serializedGroup = (int*) malloc(sizeof(int) * (groupLength + 1));
	serializedGroup[0] = group->numberOfElements;
	++lastIndex;
	for (i = 0; i < group->numberOfElements; ++i) {
		int* messageSerialized = serializeMessage(&(group->messages[i]));
		for (j = 0; j < group->messages[i].length + 2; ++j) {
			serializedGroup[lastIndex] = messageSerialized[j];
			++lastIndex;
		}
	}
	return serializedGroup;
}

Messages_Group* deserializeGroupOfMessages(int * serializedGroup) {
	Messages_Group* group;
	if (serializedGroup != NULL) {
		group = createGroupOfMessages(serializedGroup[0], 0);
		int i, lastIndex = 1;
		for (i = 0; i < serializedGroup[0]; ++i) {
			Message* message = deserializeMessage(&serializedGroup[lastIndex]);
			lastIndex += serializedGroup[lastIndex + 1] + 2;
			addMessageToGroup(group, message);
		}
	}
	return group;
}

void printSerializedGroupOfMessages(int* group) {
	if (group != NULL) {
		int numberOfMessages = group[0];
		int i, lastIndex = 1;
		for (i = 0; i < numberOfMessages; ++i) {
			printSerializedMessage(&group[lastIndex]);
			//lastIndex + length of previous msg + 2 for dest & length elem of previous msg
			lastIndex += group[lastIndex + 1] + 2;
		}
	}
}

void deallocateGroupOfMessages(Messages_Group* group) {
	free(group);
}
