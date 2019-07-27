/*
 * peterson.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */

#include "peterson.h"

int max(int a, int b) {
	return a > b ? a : b;
}

void Peterson(int my_rank, int p, int tag, int ids[]) {
	int left = (my_rank - 1 + p) % p;
//	int right = (my_rank + 1) % p;

	int msg[SIZE];

	int first_id = -1, second_id = -1;
	int max_id = ids[my_rank];
	int relay = FALSE;
	int leader = -1;

	msg[0] = NEW_ID; //new_id / leader
	msg[1] = max_id; //id
	msg[2] = -1; //doesn't matter

	Messages_Group *group = createGroupOfMessages(p + 1, 0);

	Message *msg1 = createMessage(left, SIZE, msg);
	addMessageToGroup(group, msg1);

	//send group
	sendGroupOfMessages(group, p, tag);
	//delete group and any internal pointer
	deallocateGroupOfMessages(group);

	int shouldBreak = 0;

	while (!shouldBreak) {
		//receive group or message
		int sender;
		Messages_Group* receivedGroup = receiveGroupOfMessages(tag, &sender);
		Messages_Group *group = createGroupOfMessages(p + 1, 0); // * receivedGroup->numberOfElements), 0); //CHANGE p

		int messageIdx = 0;
		for (; messageIdx < receivedGroup->numberOfElements; ++messageIdx) {
			Message* message = &(receivedGroup->messages[messageIdx]);
			int currentSender = message->dest;

			if (currentSender == -1) {
				Message *msg1 = createMessage(-1, SIZE, msg);
				addMessageToGroup(group, msg1);
				break;
			}

			int i;
			for (i = 0; i < message->length; ++i) {
				msg[i] = message->content[i];
			}

			//case of receiving new id from right
			if (msg[0] == NEW_ID) {
				if (relay == TRUE) {
					Message *msg1 = createMessage(left, SIZE, msg);
					addMessageToGroup(group, msg1);
				} else {
					if (first_id == -1) {
						first_id = msg[1];
						if (max_id == first_id) {
							msg[0] = LEADER;
							msg[1] = max_id;
							msg[2] = my_rank;
							Message *msg1 = createMessage(left, SIZE, msg);
							addMessageToGroup(group, msg1);
						} else {
							Message *msg1 = createMessage(left, SIZE, msg);
							addMessageToGroup(group, msg1);
						}
					} else {
						second_id = msg[1];
						if (first_id > max(max_id, second_id)) {
							max_id = first_id;
							msg[1] = max_id;

							first_id = -1;
							second_id = -1;
							Message *msg1 = createMessage(left, SIZE, msg);
							addMessageToGroup(group, msg1);
						} else {
							relay = TRUE;
						}
					}
				}
			} else {
				if (msg[0] == LEADER) {
					leader = msg[1];
					if (msg[2] != my_rank) {
						Message *msg1 = createMessage(left, SIZE, msg);
						addMessageToGroup(group, msg1);
					}
					printf("I am process %d (%d) and i recognize as leader the process with id %d\n", my_rank, ids[my_rank], leader);
					printf("Process %d terminating \n", my_rank);
					Message *msg3 = createMessage(BREAK_FLAG, 4, msg);
					addMessageToGroup(group, msg3);
					shouldBreak = 1;
					break;
				}
			}
		}
		//here
		if (group->numberOfElements == 0) {
			Message *msg1 = createMessage(-1, SIZE, msg);
			addMessageToGroup(group, msg1);
		}
		sendGroupOfMessages(group, p, tag);
		deallocateGroupOfMessages(group);
	}
}
