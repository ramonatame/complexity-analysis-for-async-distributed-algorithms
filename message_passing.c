/*
 * message_passing.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */

#include "message_passing.h"

void sendGroupOfMessages(Messages_Group* group, int dest, int tag) {
	int* serializedGroup = serializeGroupOfMessages(group);
	int groupLength = getLengthOfMessages_Group(group);
	MPI_Send(serializedGroup, groupLength, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

int getMessageCount(int tag, int *sender) {
	int count;
	MPI_Status status;
	MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_INT, &count);
	*sender = status.MPI_SOURCE;
	return count;
}

Messages_Group* receiveGroupOfMessages(int tag, int* sender) {
	MPI_Status status;
	int sendr;
	int count = getMessageCount(tag, &sendr);
	int* serializedGroup = (int*) malloc(sizeof(int) * count);
	MPI_Recv(serializedGroup, count, MPI_INT, sendr, tag, MPI_COMM_WORLD, &status);
	Messages_Group* group = deserializeGroupOfMessages(serializedGroup);
	*sender = status.MPI_SOURCE;
	return group;
}
