/*
 * hirschberg_sinclair_optimized.c
 *
 *  Created on: Jun 5, 2017
 *      Author: ramona
 */

#include "hirschberg_sinclair_optimized.h"

void HS_Optimized(int my_rank, int p, int tag, int *ids) {
	int leaderAtPhase = -1;
	int possibleLeaderID = -1;
	int lastSender = -1;

	int electedLeader;
	int leaderPerDirectionCounter = 0;

	int leftProcess = (my_rank - 1 + p) % p;
	int rightProcess = (my_rank + 1) % p;

	int msg[SIZE];
	msg[0] = PROBE;
	msg[1] = ids[my_rank];
	msg[2] = 0; // phase
	msg[3] = 1; //hops

	int maxKnownID = ids[my_rank];

	//phase 1: send PROBE to both directions
	Messages_Group *group = createGroupOfMessages(p + 1, 0);

	Message *msg1 = createMessage(leftProcess, SIZE, msg);
	addMessageToGroup(group, msg1);

	Message *msg2 = createMessage(rightProcess, SIZE, msg);
	addMessageToGroup(group, msg2);

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

			if (msg[0] == LEADER) {
				electedLeader = msg[1];
				//if i am not the leader, set leader and forward
				if (msg[1] != ids[my_rank]) {
					if (currentSender == rightProcess) {
						Message *msg1 = createMessage(leftProcess, SIZE, msg);
						addMessageToGroup(group, msg1);
					} else {
						Message *msg1 = createMessage(rightProcess, SIZE, msg);
						addMessageToGroup(group, msg1);
					}
				}
				printf("I am process %d and my electedLeader is %d\n", ids[my_rank], electedLeader);
				Message *msg3 = createMessage(BREAK_FLAG, SIZE, msg);
				addMessageToGroup(group, msg3);
				shouldBreak = 1;
				break;
			} else if (msg[1] >= maxKnownID) {
				if (msg[0] == PROBE) {
					if (currentSender != lastSender && possibleLeaderID == msg[1]) {
						msg[0] = LEADER;
						msg[2] = -1;
						msg[3] = -1;

						Message *msg1 = createMessage(leftProcess, SIZE, msg);
						addMessageToGroup(group, msg1);
						Message *msg2 = createMessage(rightProcess, SIZE, msg);
						addMessageToGroup(group, msg2);

						electedLeader = msg[1];
						printf("I am process %d and my electedLeader is %d\n", ids[my_rank], electedLeader);
						Message *msg3 = createMessage(BREAK_FLAG, SIZE, msg);
						addMessageToGroup(group, msg3);
						shouldBreak = 1;
						break;
					} else {
						possibleLeaderID = msg[1];
						lastSender = currentSender;
					}

					if (msg[1] == ids[my_rank]) { //electedLeader
						msg[0] = LEADER;
						msg[2] = -1;
						msg[3] = -1;
						Message *msg1 = createMessage(leftProcess, SIZE, msg);
						addMessageToGroup(group, msg1);
						//not on border
					} else if (msg[3] < (1 << msg[2])) { //forward
						maxKnownID = msg[1];
						//if this process was leader at a previous phase test if it can reply instead of forward
						if (leaderAtPhase != -1 && (1 << msg[2]) - msg[3] <= (1 << leaderAtPhase)) {
							//printf("process id %d leader at phase %d SAVING %d hops for id %d\n", ids[my_rank], leaderAtPhase, (1 << msg[2]) - msg[3], msg[1]);
							msg[0] = REPLY;
							msg[3] = -1;
							if (currentSender == leftProcess) {
								Message *msg1 = createMessage(leftProcess, SIZE, msg);
								addMessageToGroup(group, msg1);
							} else {
								Message *msg1 = createMessage(rightProcess, SIZE, msg);
								addMessageToGroup(group, msg1);
							}
							//not leader at any previous phase and not on border, just forward
						} else {
							msg[3] = msg[3] + 1;
							if (currentSender == rightProcess) {
								Message *msg1 = createMessage(leftProcess, SIZE, msg);
								addMessageToGroup(group, msg1);
							} else {
								Message *msg1 = createMessage(rightProcess, SIZE, msg);
								addMessageToGroup(group, msg1);
							}
						}
						//if message on border and >= max, reply
					} else { //pow(2,phase)-hops <= pow(2,leaderAtPhase)
						maxKnownID = msg[1];
						msg[0] = REPLY;
						msg[3] = -1;
						if (currentSender == leftProcess) {
							Message *msg1 = createMessage(leftProcess, SIZE, msg);
							addMessageToGroup(group, msg1);
						} else {
							Message *msg1 = createMessage(rightProcess, SIZE, msg);
							addMessageToGroup(group, msg1);
						}
					}

				} else { //REPLY
						 //if the message is not for this process, forward
					if (msg[1] != ids[my_rank]) {
						if (currentSender == rightProcess) {
							Message *msg1 = createMessage(leftProcess, SIZE, msg);
							addMessageToGroup(group, msg1);
						} else {
							Message *msg1 = createMessage(rightProcess, SIZE, msg);
							addMessageToGroup(group, msg1);
						}
						//the message is for this process
					} else {
						//increment the response counter and if it received answers from both directions,
						//then it's leader at this phase and starts next phase
						if (++leaderPerDirectionCounter == 2) {
							leaderPerDirectionCounter = 0;
							++leaderAtPhase;
							//printf("###process id %d electedLeader at phase %d \n", ids[my_rank], leaderAtPhase);
							msg[0] = PROBE;
							msg[2] = msg[2] + 1;
							msg[3] = 1;
							Message *msg1 = createMessage(leftProcess, SIZE, msg);
							addMessageToGroup(group, msg1);
							Message *msg2 = createMessage(rightProcess, SIZE, msg);
							addMessageToGroup(group, msg2);
						} else {
							//HIDDEN. SIDE
							Message *msg1 = createMessage(-1, SIZE, msg);
							addMessageToGroup(group, msg1);
						}
					}
				}
			} else {
				//SWALLOW CASE
				Message *msg1 = createMessage(-1, SIZE, msg);
				addMessageToGroup(group, msg1);
			}
		}
		sendGroupOfMessages(group, p, tag);
		//delete group and any internal pointer
		deallocateGroupOfMessages(group);
	}
}
