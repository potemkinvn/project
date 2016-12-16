#ifndef CLIENTFUNCTION_H_INCLUDED
#define CLIENTFUNCTION_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "messagetype.h"
#include "playertype.h"
#include "piece.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

void myFlush();

/** \brief Parse Message
 *         usage: ms = ParseMessage(buff);
 *         char buff[] = "101 ~ abc" => ms.command = 101; ms.message = "abc"
 * \param char recv_data[]
 * \return message type variable
 *
 */
message ParseMessage(char recv_data[]);

/** \brief Login function
 *
 * \param
 * \return int -1 if disconnected, 0 if invalid credential, 1 if success
 *
 */
int Login();

/** \brief List all player and their status
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if list success
 *
 */
int listAllPlayerAndStatus();

/** \brief Get this player invitation list
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if list success
 *
 */
int getMyInvitationList();

/** \brief Player A send invitation to player B
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if refused, 1 if accepted
 *
 */
int sendInvitation();

/** \brief Player B send accept to player A
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if list empty, 1 if list has more than 1 user
 *
 */
int acceptInvitation();

void setup();
void printboard();
void Move(int a,int b,int c,int d);
int ValidSquare(int a,int b,int c,int d);
int CheckBlack(int *whitelist);
int *BlackMoveList(int epa,int epb);
int *WhiteMoveList(int epa,int epb);
int CheckWhite(int *blacklist);
int WhiteMove(char *cmd,int turn,int castlingcheck);
int WhiteFaultCheck(int a,int b,int c,int d);
int BlackMove(char *cmd,int turn,int castlingcheck);
int BlackFaultCheck(int a,int b,int c,int d);
int WhiteMate(int *whitelist);
int BlackMate(int *blacklist);
int PlayGame(int side);

#endif // CLIENTFUNCTION_H_INCLUDED
