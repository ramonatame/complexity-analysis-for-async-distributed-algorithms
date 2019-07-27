/*
 * hirschberg_sinclair_standard.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */

#include "hirschberg_sinclair_standard.h"

void HS_Standard(int my_rank, int p, int tag, int *ids) {

	int left = (my_rank - 1 + p) % p;
	int right = (my_rank + 1) % p;
	int leader;
	int receiveCnt = 0;

	int msg[SIZE];
	msg[0] = PROBE;
	msg[1] = ids[my_rank];
	msg[2] = 0; // phase
	msg[3] = 1; //hops

	//phase 1: send PROBE to both directions
	Messages_Group *group = createGroupOfMessages(p + 1, 0);

	Message *msg1 = createMessage(left, SIZE, msg);
	addMessageToGroup(group, msg1);

	Message *msg2 = createMessage(right, SIZE, msg);
	addMessageToGroup(group, msg2);

	//send group
	sendGroupOfMessages(group, p, tag);

	//delete group and any internal pointer
	deallocateGroupOfMessages(group);

	int shouldBreak = 0;

	while (!shouldBreak) {
		//receive group or message
		int sender_master;
		Messages_Group* receivedGroup = receiveGroupOfMessages(tag, &sender_master);

		Messages_Group *group = createGroupOfMessages(p + 1, 0);

		int messageIdx = 0;
		for (; messageIdx < receivedGroup->numberOfElements; ++messageIdx) {
			Message* message = &(receivedGroup->messages[messageIdx]);
			int sender = message->dest;

			if (sender == -1) {
				assert(receivedGroup->numberOfElements == 1);
				break;
			}

			int i;
			for (i = 0; i < message->length; ++i) {
				msg[i] = message->content[i];
			}

			if (msg[0] == PROBE) {
				if (msg[1] == ids[my_rank]) {
					msg[0] = LEADER;
					msg[2] = -1;
					msg[3] = -1;
					Message *msg1 = createMessage(left, SIZE, msg);
					addMessageToGroup(group, msg1);
				} else if (msg[1] > ids[my_rank] && msg[3] < (1 << msg[2])) {
					msg[3] = msg[3] + 1;
					if (sender == right) {
						Message *msg1 = createMessage(left, SIZE, msg);
						addMessageToGroup(group, msg1);
					} else {
						Message *msg1 = createMessage(right, SIZE, msg);
						addMessageToGroup(group, msg1);
					}
				} else if (msg[1] > ids[my_rank] && msg[3] == (1 << msg[2])) {
					msg[0] = REPLY;
					msg[3] = -1;
					if (sender == left) {
						Message *msg1 = createMessage(left, SIZE, msg);
						addMessageToGroup(group, msg1);
					} else {
						Message *msg1 = createMessage(right, SIZE, msg);
						addMessageToGroup(group, msg1);
					}
				}
			} else if (msg[0] == REPLY) {
				if (msg[1] != ids[my_rank]) {
					if (sender == right) {
						Message *msg1 = createMessage(left, SIZE, msg);
						addMessageToGroup(group, msg1);
					} else {
						Message *msg1 = createMessage(right, SIZE, msg);
						addMessageToGroup(group, msg1);
					}
				} else {
					if (++receiveCnt == 2) {
						receiveCnt = 0;
						msg[0] = PROBE;
						msg[2] = msg[2] + 1;
						msg[3] = 1;
						Message *msg1 = createMessage(left, SIZE, msg);
						addMessageToGroup(group, msg1);
						Message *msg2 = createMessage(right, SIZE, msg);
						addMessageToGroup(group, msg2);
					}
				}
			} else {
				if (msg[0] == LEADER) {
					leader = msg[1];
					if (msg[1] != ids[my_rank]) {
						Message *msg1 = createMessage(left, SIZE, msg);
						addMessageToGroup(group, msg1);
					}
					printf("I am process %d and my leader is %d\n", ids[my_rank], leader);
					Message *msg3 = createMessage(BREAK_FLAG, SIZE, msg);
					addMessageToGroup(group, msg3);
					shouldBreak = 1;
					printf("proces %d terminating... \n", my_rank);
					break;
				}
			}
		}
		if (group->numberOfElements == 0) {
			Message *msg1 = createMessage(-1, SIZE, msg);
			addMessageToGroup(group, msg1);
		}
		sendGroupOfMessages(group, p, tag);
		deallocateGroupOfMessages(group);
	}
}
