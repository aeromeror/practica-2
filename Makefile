generator: generador.c
	gcc generador.c -o gen
client: p2-dogClient.c
	gcc p2-dogClient.c -o client
server: p2-dogServer.c
	gcc p2-dogServer.c -o server -lpthread

