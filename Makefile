BIN=./bin
SRC=./src
INCLUDE=./include/
LIB=./lib/

MAIN=main
OBJECTS=$(BIN)/$(MAIN).o $(BIN)/glad.o
LIBS=-lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lassimp -lXi -ldl -lXinerama -lXcursor

all: clean compile run

compile: dirs $(OBJECTS)
	@g++ -I$(LIB) -I$(INCLUDE) $(OBJECTS)  -o $(BIN)/$(MAIN) -std=c++11 -Wall $(LIBS)


$(BIN)/$(MAIN).o: $(SRC)/$(MAIN).cpp
	@gcc -c $(SRC)/$(MAIN).cpp -o $(BIN)/$(MAIN).o

$(BIN)/glad.o: $(SRC)/glad.c
	@gcc -c $(SRC)/glad.c -o $(BIN)/glad.o

dirs:
	@mkdir -p $(BIN)

run:
	@$(BIN)/$(MAIN)

clean:
	@rm -rf $(BIN)