CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: TCP_Sender TCP_Receiver

TCP_Sender: TCP_Sender.c
	$(CC) $(CFLAGS) TCP_Sender.c -o TCP_Sender

TCP_Receiver: TCP_Receiver.c
	$(CC) $(CFLAGS) TCP_Receiver.c -o TCP_Receiver

clean:
	rm -f TCP_Sender TCP_Receiver