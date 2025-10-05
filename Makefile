CFLAGS=-Isrc -Wall -Wextra # -g -DDEBUG
SRC=src
BIN=bin
OBJ=obj

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

EXEC := $(BIN)/aurum

.PHONY: all clean mkdirs
all: $(EXEC)

$(EXEC): $(OBJS) | mkdirs
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c | mkdirs
	$(CC) $(CFLAGS) -c $< -o $@

mkdirs:
	@mkdir -p $(BIN) $(OBJ)

clean:
	@rm -rf $(BIN)/* core* *~ $(SRC)/*~ docs/* *.dSYM $(OBJ)/*

