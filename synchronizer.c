/*
 * synchronizer.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */

#include "synchronizer.h"

void Synchronizer_Master(int my_rank, int tag, int p, int message_size) {
	printf("Master started execution.. \n");
	int i, requestsCounter = 0, terminateRequestsCounter = 0;
	int waitingRequestsCounter = p;

	Messages_Group** messages_groups = (Messages_Group**) malloc(sizeof(Messages_Group*) * p);
	for (i = 0; i < p; ++i) {
		messages_groups[i] = createGroupOfMessages(p + 1, 0);
	}
	int clock = 0, total_msg = 0;
	while (1) {
		int sender, dest;
		Messages_Group* receiveGroup = receiveGroupOfMessages(tag, &sender);
		for (i = 0; i < receiveGroup->numberOfElements; ++i) {
			Message* message = &(receiveGroup->messages[i]);
			dest = message->dest;
			if (dest >= 0) {
				++total_msg;
				message->dest = sender;
				addMessageToGroup(messages_groups[dest], message);
			} else if (dest == BREAK_FLAG) {
				++terminateRequestsCounter;
			}
		}
		if (terminateRequestsCounter == waitingRequestsCounter) {
			printf("Algorithm terminated in %d steps by sending a total of %d messages \n", clock, total_msg);
			printf("terminating %d \n", my_rank);
			break;
		}
		if (++requestsCounter == waitingRequestsCounter) {
			++clock;
			for (i = 0; i < p; ++i) {
				if (messages_groups[i]->numberOfElements == 0) {
					int msg[] = { 0, 0, 0, 0 };
					Message *msg1 = createMessage(-1, message_size, msg);
					addMessageToGroup(messages_groups[i], msg1);
				}
				sendGroupOfMessages(messages_groups[i], i, tag);
				deallocateGroupOfMessages(messages_groups[i]);
				messages_groups[i] = createGroupOfMessages(p, 0);
			}
			requestsCounter = 0;
			waitingRequestsCounter -= terminateRequestsCounter;
			terminateRequestsCounter = 0;
		}
	}
}
