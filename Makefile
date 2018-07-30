include Makefile.conf

all: bin test

dirs:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)

bin: dirs game uci_engine

test: dirs $(BIN_DIR)/board_tests $(BIN_DIR)/figure_tests $(BIN_DIR)/engine_tests

game: $(BIN_DIR)/game

uci_engine: $(BIN_DIR)/uci_engine

$(BIN_DIR)/board_tests: $(OBJ_DIR)/Board_t.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Utils.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/board_tests $(OBJ_DIR)/Board_t.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Utils.o

$(BIN_DIR)/figure_tests: $(OBJ_DIR)/Figure_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Utils.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/figure_tests $(OBJ_DIR)/Figure_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Utils.o

$(BIN_DIR)/engine_tests: $(OBJ_DIR)/Engine_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/SocketLog.o $(OBJ_DIR)/Socket.o $(OBJ_DIR)/Utils.o $(OBJ_DIR)/Logger.o Board.h Figure.h Field.h Engine.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/engine_tests $(OBJ_DIR)/Engine_t.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/SocketLog.o $(OBJ_DIR)/Socket.o $(OBJ_DIR)/Utils.o $(OBJ_DIR)/Logger.o

$(BIN_DIR)/game: $(OBJ_DIR)/Game.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/PgnCreator.o $(OBJ_DIR)/SocketLog.o $(OBJ_DIR)/Socket.o $(OBJ_DIR)/Logger.o $(OBJ_DIR)/Utils.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/game $(OBJ_DIR)/Game.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/PgnCreator.o $(OBJ_DIR)/SocketLog.o $(OBJ_DIR)/Socket.o $(OBJ_DIR)/Utils.o $(OBJ_DIR)/Logger.o

$(BIN_DIR)/uci_engine: $(OBJ_DIR)/UCIEngine.o $(OBJ_DIR)/UCIHandler.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/SocketLog.o $(OBJ_DIR)/Socket.o $(OBJ_DIR)/Utils.o $(OBJ_DIR)/Logger.o Board.h Figure.h Field.h
	$(CXX) $(CFLAGS) -o $(BIN_DIR)/uci_engine $(OBJ_DIR)/UCIEngine.o $(OBJ_DIR)/UCIHandler.o $(OBJ_DIR)/Engine.o $(OBJ_DIR)/Figure.o $(OBJ_DIR)/Board.o $(OBJ_DIR)/SocketLog.o $(OBJ_DIR)/Socket.o $(OBJ_DIR)/Utils.o $(OBJ_DIR)/Logger.o

$(OBJ_DIR)/Game.o: Game.cc Engine.h Board.h Figure.h Field.h PgnCreator.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Game.o Game.cc

$(OBJ_DIR)/UCIEngine.o: UCIEngine.cc UCIHandler.h Engine.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/UCIEngine.o UCIEngine.cc

$(OBJ_DIR)/UCIHandler.o: UCIHandler.cc UCIHandler.h Board.h Figure.h Field.h Logger.h utils/Utils.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/UCIHandler.o UCIHandler.cc

$(OBJ_DIR)/Engine.o: Engine.cc Engine.h Board.h Figure.h Field.h Logger.h utils/Utils.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Engine.o Engine.cc

$(OBJ_DIR)/Board_t.o: Board_t.cc Board.h utils/Test.h utils/Mock.h Figure.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Board_t.o Board_t.cc

$(OBJ_DIR)/Board.o: Board.cc Board.h Field.h Figure.h utils/Utils.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Board.o Board.cc

$(OBJ_DIR)/Figure_t.o: Figure_t.cc Figure.h utils/Test.h utils/Mock.h Board.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Figure_t.o Figure_t.cc

$(OBJ_DIR)/Engine_t.o: Engine_t.cc Figure.h utils/Test.h utils/Mock.h Board.h Field.h Engine.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Engine_t.o Engine_t.cc

$(OBJ_DIR)/Figure.o: Figure.cc Figure.h Field.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Figure.o Figure.cc

$(OBJ_DIR)/PgnCreator.o: PgnCreator.cc PgnCreator.h Figure.h Board.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/PgnCreator.o PgnCreator.cc

$(OBJ_DIR)/Logger.o: Logger.cc Logger.h utils/SocketLog.h utils/Utils.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Logger.o Logger.cc

$(OBJ_DIR)/Socket.o: utils/Socket.cc utils/Socket.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Socket.o utils/Socket.cc

$(OBJ_DIR)/SocketLog.o: utils/SocketLog.cc utils/SocketLog.h utils/Socket.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/SocketLog.o utils/SocketLog.cc

$(OBJ_DIR)/Utils.o: utils/Utils.cc utils/Utils.h
	$(CXX) $(CFLAGS) -c -o $(OBJ_DIR)/Utils.o utils/Utils.cc

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJ_DIR)/*
