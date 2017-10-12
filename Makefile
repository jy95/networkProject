# See gcc/clang manual to understand all flags
#CFLAGS += -std=c99 # Define which version of the C standard to use #WARNING : getopt ne fonctionne pas en 99 mais en 11
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS += -rdynamic
LDFLAGS += -lz
LDFLAGS += -lcunit

# external libs
# par défaut les chemins classiques
INCLUDES += -I$(HOME)/local/include
INCLUDES += -L$(HOME)/local/lib

# Default compiler
CC=gcc

# folders
SOURCE_FOLDER=src
TESTS_FOLDER=tests
PAQUET_FOLDER=paquet
SEND_RECEIVE_DATA_FOLDER=sendAndReceiveData
CLIENT_FOLDER=client
SERVER_FOLDER=server

# folder of sources
PAQUET_FULL_PATH=$(SOURCE_FOLDER)/$(PAQUET_FOLDER)
SEND_RECEIVE_DATA_FULL_PATH=$(SOURCE_FOLDER)/$(SEND_RECEIVE_DATA_FOLDER)
CLIENT_FULL_PATH=$(SOURCE_FOLDER)/$(CLIENT_FOLDER)
SERVER_FULL_PATH=$(SOURCE_FOLDER)/$(SERVER_FOLDER)

# sources files
# On prend tout
PACKET_SOURCES = $(wildcard $(PAQUET_FULL_PATH)/*.c)
SEND_RECEIVE_DATA_SOURCES = $(wildcard $(SEND_RECEIVE_DATA_FULL_PATH)/*.c)
CLIENT_SOURCES = $(wildcard $(CLIENT_FULL_PATH)/*.c)
SERVER_SOURCES = $(wildcard $(SERVER_FULL_PATH)/*.c)
TESTS_SOURCES = $(wildcard $(TESTS_FOLDER)/*.c)

# objects

PACKET_OBJECTS=$(PACKET_SOURCES:.c=.o)
SEND_RECEIVE_DATA_OBJECTS=$(SEND_RECEIVE_DATA_SOURCES:.c=.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS=$(SERVER_SOURCES:.c=.o)

# another things

# Default target
all: clean server client

# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean

# compile them and link the resulting *.o's into an executable named database
#database: record.o database.o

client: $(CLIENT_OBJECTS) $(PACKET_OBJECTS) $(SEND_RECEIVE_DATA_OBJECTS); \
		$(CC) $(CFLAGS) $(CLIENT_OBJECTS) $(LDFLAGS) -o client;

server: $(SERVER_OBJECTS) $(PACKET_OBJECTS) $(SEND_RECEIVE_DATA_OBJECTS); \
		$(CC) $(CFLAGS) $(SERVER_OBJECTS) $(LDFLAGS) -o server;

tests: $(PACKET_OBJECTS) $(TESTS_SOURCES); \
		$(CC) $(CFLAGS) -lcunit $(TESTS_SOURCES) $(LDFLAGS) $(INCLUDES) -o testsScript;

.PHONY: clean

clean:
	@rm -f $(PACKET_OBJECTS) $(SEND_RECEIVE_DATA_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_OBJECTS)