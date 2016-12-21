#ifndef MESSAGETYPE_H_INCLUDED
#define MESSAGETYPE_H_INCLUDED

typedef struct message {
    int command;
    char message[1024];
} message;

#endif // MESSAGETYPE_H_INCLUDED
