main:
	g++ -pthread server.cpp server_main.cpp -o server -lssl -lcrypto
	g++ client.cpp client_main.cpp -o client -lssl -lcrypto

clean:
	rm server client