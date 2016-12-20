#ifndef SERVERFUNCTION_H_INCLUDED
#define SERVERFUNCTION_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>

#include "linkedlist.h"

#include "messagetype.h"
#include "playertype.h"

/** \brief Reset this player info, due to this player disconnected
 *
 * \param playerType *pl this player pointer
 * \return
 *
 */
void ResetPlayer(playerType *pl);

void myFlush();

/** \brief Push this user to user list from file
 *
 * \param FILE *input :file descriptor
 * \param node **top :user list
 * \return
 *
 */
void GetUserInfo(FILE *input, node **top);

/** \brief Find node user pointer on user name
 *
 * \param node **top :user list
 * \param char buff[] :user name
 * \return user node pointer on match name or null if none
 *
 */
node* FindNodeOnUsername(node **top, char buff[]);

/** \brief Parse Message
 *         usage: ms = ParseMessage(buff);
 *         char buff[] = "101 ~ abc" => ms.command = 101; ms.message = "abc"
 * \param char recv_data[]
 * \return message type variable
 *
 */
message ParseMessage(char recv_data[]);

/** \brief Get connected player index on user name
 *
 * \param char username[] :user name to find
 * \param playerType player[30] :array of all player in server
 * \return
 *
 */
int GetPlayerIndexOnUsername(char username[], playerType player[30]);

/** \brief Get connected player index on sockdes
 *
 * \param int sockdes :sockdes to find
 * \param playerType player[30] :array of all player in server
 * \return
 *
 */
int GetPlayerIndexOnSockdes(int sockdes, playerType player[30]);

void SendOpponentQuit(int opponentIndex);

/** \brief Test username function
 *
 * \param
 * \return
 *
 */
void TestUsername();

/** \brief Retry login more than 3 times, disconnect this player
 *
 * \param
 * \param
 * \return
 *
 */
void RetryUsernameExceed();

/** \brief Send (to user) and process response (from user)
 *
 * \param
 * \param
 * \return send to user 107 if password match, 108 if password not match
 *
 */
void SendAndProcessResponse();

/** \brief Receive client A send invitation request client B: 200 ~ [B's name]
 *
 * \param
 * \param
 * \return result: log clientA.inviteSockes is client B's sockdes
 *          or (clientB not exist or clientA send request to clientA) send A 209
 *
 */
void ReceiveInvitationRequest();

/** \brief Receive B accept A's invite: 202 ~ [A's name]
 *          start step 2 in 3 ways handshake:
 *          forward it (the info that B accepted) to A: 203 ~ [B's name]
 * \param
 * \param
 * \return
 *
 */
void StartStep2In3WaysHandshake();

/** \brief Start step 3 in 3 ways handshake:
 *          receive A ready (ack) status: 206 ~ [B's name]
 *          forward it to B: 206 ~ [A's name]
 * \param
 * \param
 * \return
 *
 */
void StartStep3In3WaysHandshake();

/** \brief List all players
 *
 * \param
 * \param
 * \return
 *
 */
void ListAllPlayers();

/** \brief Get this player invitation list
 *
 * \param
 * \param
 * \return
 *
 */
void GetInvitationList();

/** \brief Receive move from playerX: 300 ~ [cordinate]
 *          forward to playerY: 301 ~ [cordinate]
 *          or if playerY disconnected, send playerX to main menu
 * \param
 * \param
 * \return
 *
 */
void GetMoveAndForwardMove();

/** \brief Receive game result: 302
 *
 * \param
 * \param
 * \return
 *
 */
void ProcessGameResult();

/** \brief Game has started, create log file, set player[i].logFileName
 *
 * \param
 * \param
 * \return
 *
 */
void InitLog();

/** \brief Game has ended, send log to two players
 *
 * \param
 * \param
 * \return
 *
 */
void SendLog();

#endif // SERVERFUNCTION_H_INCLUDED
