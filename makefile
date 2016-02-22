++ = g++
FLAGS = -lpthread

all: multiThreaded client udpserver udpclient clean

multiThreaded: multiThreaded.o
	$(++) -o $@ $^ $(FLAGS)

client: client.o
	$(++) -o $@ $^


udpserver: udpserver.o 
	$(++) -o $@ $^ $(FLAGS)

udpclient: udpclient.o
	$(++) -o $@ $^

clean:
	rm *.o
