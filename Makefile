CC = g++-7
CFLAGS = -std=c++17 -Wall -Wextra

BIN_DIR = ./bin

all: build

BOOST_INCLUDE_PATH =                                    \
    -I./third_party/boost/asio/include/                 \
    -I./third_party/boost/config/include/               \
    -I./third_party/boost/system/include/               \
    -I./third_party/boost/system/include/               \
    -I./third_party/boost/throw_exception/include/      \
    -I./third_party/boost/assert/include/               \
    -I./third_party/boost/date_time/include/            \
    -I./third_party/boost/smart_ptr/include/            \
    -I./third_party/boost/core/include/                 \
    -I./third_party/boost/predef/include/               \
    -I./third_party/boost/utility/include/              \
    -I./third_party/boost/static_assert/include/        \
    -I./third_party/boost/type_traits/include/          \
    -I./third_party/boost/mpl/include/                  \
    -I./third_party/boost/preprocessor/include/         \
    -I./third_party/boost/numeric_conversion/include/   \
    -I./third_party/boost/bind/include/                 \
    -I./third_party/boost/regex/include/

JSON_INCLUDE_PATH =                                     \
    -I./third_party/json/include/

LOGGER_INCLUDE_PATH =                                   \
    -I./common/logger

COMM_MSG_INCLUDE_PATH =                                 \
    -I./common/comm_msg

SERVICE_BIN_DIR = $(BIN_DIR)/service
SERVICE_SRC_DIR = ./service
SERVICE_INCLUDE_PATH =                                   \
    $(LOGGER_INCLUDE_PATH)                               \
    $(BOOST_INCLUDE_PATH)                                \
    $(COMM_MSG_INCLUDE_PATH)                             \
    $(JSON_INCLUDE_PATH)
_SERVICE_OBJ = main.o native_receiver_comm.o native_receiver_channel.o requests.o signatures.o
SERVICE_OBJ = $(patsubst %,$(SERVICE_BIN_DIR)/%,$(_SERVICE_OBJ))

NATIVE_RECEIVER_BIN_DIR = $(BIN_DIR)/native-receiver
NATIVE_RECEIVER_SRC_DIR = ./native-receiver
NATIVE_RECEIVER_INCLUDE_PATH =                           \
    $(LOGGER_INCLUDE_PATH)                               \
    $(BOOST_INCLUDE_PATH)                                \
    $(COMM_MSG_INCLUDE_PATH)                             \
    $(JSON_INCLUDE_PATH)
_NATIVE_RECEIVER_OBJ = main.o extension_comm.o service_comm.o
NATIVE_RECEIVER_OBJ = $(patsubst %,$(NATIVE_RECEIVER_BIN_DIR)/%,$(_NATIVE_RECEIVER_OBJ))

LOGGER_BIN_DIR = $(BIN_DIR)/logger
LOGGER_SRC_DIR = ./common/logger
_LOGGER_OBJ = logger.o
LOGGER_OBJ = $(patsubst %,$(LOGGER_BIN_DIR)/%,$(_LOGGER_OBJ))

prepare_env:
	mkdir -p $(BIN_DIR)
	mkdir -p $(LOGGER_BIN_DIR)
	mkdir -p $(NATIVE_RECEIVER_BIN_DIR)
	mkdir -p $(SERVICE_BIN_DIR)

$(SERVICE_BIN_DIR)/%.o: $(SERVICE_SRC_DIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(SERVICE_INCLUDE_PATH)

$(NATIVE_RECEIVER_BIN_DIR)/%.o: $(NATIVE_RECEIVER_SRC_DIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(NATIVE_RECEIVER_INCLUDE_PATH)

$(LOGGER_BIN_DIR)/%.o: $(LOGGER_SRC_DIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

build_logger:
	ar rcs $(LOGGER_BIN_DIR)/liblogger.a $(LOGGER_OBJ)

build_native_receiver:
	$(CC) $(NATIVE_RECEIVER_OBJ) -L$(LOGGER_BIN_DIR) -llogger -o $(NATIVE_RECEIVER_BIN_DIR)/native_receiver -lpthread

build_service:
	$(CC) $(SERVICE_OBJ) -L$(LOGGER_BIN_DIR) -llogger -o $(SERVICE_BIN_DIR)/service -lpthread

build: prepare_env $(SERVICE_OBJ) $(NATIVE_RECEIVER_OBJ) $(LOGGER_OBJ) build_logger build_native_receiver build_service

clean:
	rm -f -r $(BIN_DIR)
