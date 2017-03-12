# Multiplayer Chess Game

A multiplayer Chess Game built by using C socket. User can login, send invite, accept invite and play game. After finish a game, both user receive a log about the game: duration, moves, game result.

**Note: ** this program only support Linux platform, because network socket written for Linux platform. 

List user: 
user: truongnm 	pass: 123456

user: dungta 	pass: 123abc

user: 1 		pass: 1

user: 2			pass: 1


## Compile

For server: go to realease/server and compile: gcc -o server server.c serverfunction.c linkedlist.c

For server: go to realease/client and compile: gcc -o client client.c clientfunction.c

## Usage

cd to release folder. 

For server, cd to server and run: ./server

For client, cd to client and run: ./client 

*Note: ** for multiple client you have to run in different client folder. Just copy paste to a new client folder and run it. I have create 4 client folder in advance.