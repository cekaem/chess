include Makefile.conf

all: bin test

dirs:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

bin: dirs game

test: dirs $(BIN_DIR)/board_tests $(BIN_DIR)/figure_tests $(BIN_DIR)/engine_tests

game: $(BIN_DIR)/game

$(BIN_DIR)/board_tests: $(OBJ_DIR)/Board_t.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Figure.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/board_tests $(OBJ_DIR)/Board_t.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Figure.o

$(BIN_DIR)/figure_tests: $(OBJ_DIR)/Figure_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/figure_tests $(OBJ_DIR)/Figure_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o

$(BIN_DIR)/engine_tests: $(OBJ_DIR)/Engine_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Engine.o Board.h Figure.h Field.h Engine.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/engine_tests $(OBJ_DIR)/Engine_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Engine.o

$(BIN_DIR)/game: $(OBJ_DIR)/Game.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/PgnCreator.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/game $(OBJ_DIR)/Game.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/PgnCreator.o

$(OBJ_DIR)/Game.o: Game.cc Engine.h Board.h Figure.h Field.h PgnCreator.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Game.o Game.cc

$(OBJ_DIR)/Engine.o: Engine.cc Engine.h Board.h Figure.h Field.h Log.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Engine.o Engine.cc

$(OBJ_DIR)/Board_t.o: Board_t.cc Board.h Test.h Mock.h Figure.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Board_t.o Board_t.cc

$(OBJ_DIR)/Board.o: Board.cc Board.h Field.h Figure.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Board.o Board.cc

$(OBJ_DIR)/Figure_t.o: Figure_t.cc Figure.h Test.h Mock.h Board.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Figure_t.o Figure_t.cc

$(OBJ_DIR)/Engine_t.o: Engine_t.cc Figure.h Test.h Mock.h Board.h Field.h Engine.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Engine_t.o Engine_t.cc

$(OBJ_DIR)/Figure.o: Figure.cc Figure.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Figure.o Figure.cc

$(OBJ_DIR)/PgnCreator.o: PgnCreator.cc PgnCreator.h Figure.h Board.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/PgnCreator.o PgnCreator.cc

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
