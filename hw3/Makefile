main:
	g++ -pthread server.cpp main.cpp -o server -lssl -lcrypto
	g++ client.cpp -o client

clean:
	rm server client