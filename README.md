# Pacman

This is a multiplayer version of pacman.

## How to build

To build the server and client applications, run

	cd src
	make all

## How to play

First start a server by running

	src/server port_number

This will start the server on the specified port number.

Clients can then connect to the server by running

	src/client server_ip_addr server_port username

First the clients will have to wait for the server
to allocate them a game.

Once a game has been allocated, a map will be visible
and players can start playing.

Use keys w, a, s, d to move up, left, down, right respectively.
