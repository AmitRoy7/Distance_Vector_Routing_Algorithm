CC =g++
BIN_FILE = dv_routing
SRC_FILE = dv_routing.cpp


all:
	$(CC) -o $(BIN_FILE) $(SRC_FILE)
	
clean:
	rm  $(BIN_FILE)
