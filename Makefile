include Makefile.conf

all: bin test

dirs:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

bin: dirs

test: dirs $(BIN_DIR)/board_tests $(BIN_DIR)/figure_tests

$(BIN_DIR)/board_tests: $(OBJ_DIR)/Board_t.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Figure.o
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/board_tests $(OBJ_DIR)/Board_t.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Figure.o

$(BIN_DIR)/figure_tests: $(OBJ_DIR)/Figure_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/figure_tests $(OBJ_DIR)/Figure_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o

$(OBJ_DIR)/Board_t.o: Board_t.cc Board.h Test.h Figure.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Board_t.o Board_t.cc

$(OBJ_DIR)/Board.o: Board.cc Board.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Board.o Board.cc

$(OBJ_DIR)/Figure_t.o: Figure_t.cc Figure.h Test.h Board.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Figure_t.o Figure_t.cc

$(OBJ_DIR)/Figure.o: Figure.cc Figure.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Figure.o Figure.cc

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
