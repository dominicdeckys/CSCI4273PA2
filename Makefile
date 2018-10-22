all : server.c
	gcc server.c -o server

clean:
	$(RM) *.o client server
