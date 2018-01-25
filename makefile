main:
	gcc transmitter.c -o transmitter -std=gnu99
	gcc agent.c -o agent -std=gnu99 
clean:
	rm package -f
