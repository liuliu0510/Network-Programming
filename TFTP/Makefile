all: server 

server: server.cpp packets.cpp
    g++ -c packets.cpp -o packets.o
    g++ server.cpp packets.o -o server