#ifndef PLAYERTYPE_H_INCLUDED
#define PLAYERTYPE_H_INCLUDED

typedef struct player {
    int     sockdes;
    char    ipAddress[10];

    char    username[20];
    int     challenge;
    int     isLogged; // 0: not logged, 1: logged

    int     isAvailable; // 0: busy; 1: available

    int     opponentSockdes;
    int     inviteSockdes;
    char    logFileName[50];

    int     sentGameResult;
} playerType;


#endif // PLAYERTYPE_H_INCLUDED
