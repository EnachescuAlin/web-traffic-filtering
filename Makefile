CC = g++
CFLAGS = -std=c++14 -Wall -Wextra

BIN_DIR = ./bin

all: build

NATIVE_RECEIVER_BIN_DIR = $(BIN_DIR)/native-receiver
NATIVE_RECEIVER_SRC_DIR = ./native-receiver
NATIVE_RECEIVER_INCLUDE_PATH = -I./common/logger
_NATIVE_RECEIVER_OBJ = main.o extension_comm.o
NATIVE_RECEIVER_OBJ = $(patsubst %,$(NATIVE_RECEIVER_BIN_DIR)/%,$(_NATIVE_RECEIVER_OBJ))

LOGGER_BIN_DIR = $(BIN_DIR)/logger
LOGGER_SRC_DIR = ./common/logger
_LOGGER_OBJ = logger.o
LOGGER_OBJ = $(patsubst %,$(LOGGER_BIN_DIR)/%,$(_LOGGER_OBJ))

prepare_env:
	mkdir -p $(BIN_DIR)
	mkdir -p $(LOGGER_BIN_DIR)
	mkdir -p $(NATIVE_RECEIVER_BIN_DIR)


$(NATIVE_RECEIVER_BIN_DIR)/%.o: $(NATIVE_RECEIVER_SRC_DIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(NATIVE_RECEIVER_INCLUDE_PATH)

$(LOGGER_BIN_DIR)/%.o: $(LOGGER_SRC_DIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

build_logger:
	ar rcs $(LOGGER_BIN_DIR)/liblogger.a $(LOGGER_OBJ)

build_native_receiver:
	$(CC) $(NATIVE_RECEIVER_OBJ) -L$(LOGGER_BIN_DIR) -llogger -o $(NATIVE_RECEIVER_BIN_DIR)/native_receiver

build: prepare_env $(NATIVE_RECEIVER_OBJ) $(LOGGER_OBJ) build_logger build_native_receiver

clean:
	rm -f -r $(BIN_DIR)
