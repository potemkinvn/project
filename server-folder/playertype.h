#ifndef PLAYERTYPE_H_INCLUDED
#define PLAYERTYPE_H_INCLUDED

typedef struct player {
    char    username[20];
    int     sockdes;
    char    ipAddress[10];
    int     opponentSockdes;
    int     challenge;
    int     isAvailable; // 0: busy; 1: available
    int     isLogged; // 0: not logged, 1: logged
    int     inviteSockdes;
} playerType;


#endif // PLAYERTYPE_H_INCLUDED
