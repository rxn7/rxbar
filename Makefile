OUT := rxbar
CC := g++
DIR_SRC := src
INC := -Isrc -I.
CFLAGS := -std=c++17
SRC := $(wildcard $(addsuffix /*.cpp, $(DIR_SRC)))
OBJ := $(patsubst %.cpp, %.o, $(SRC))
LIBS := -lX11

.PHONY: all clean

all: $(OBJ) $(OUT)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) $(OBJ) -o $@

clean:
	rm $(OBJ) $(OUT)

install:
	make
	sudo install -c ./rxbar /usr/bin/rxbar
