# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS = -rdynamic

# external libs
# par défaut les chemins classiques

LDFLAGS += -I$(HOME)/local/include
LDFLAGS += -L$(HOME)/local/lib

# Default compiler
CC=gcc

# folders
SOURCE_FOLDER=src
TESTS_FOLDER=tests
PAQUET_FOLDER=paquet
SEND_RECEIVE_DATA_FOLDER=sendAndReceiveData
CLIENT_FOLDER=client
SERVER_FOLDER=server

# sources files
# On prend tout

PACKET_SOURCES = $(wildcard $(SOURCE_FOLDER)/$(PAQUET_FOLDER)/**/*.c)
SEND_RECEIVE_DATA_SOURCES = $(wildcard $(SOURCE_FOLDER)/$(SEND_RECEIVE_DATA_FOLDER)/**/*.c)
CLIENT_SOURCES = $(wildcard $(SOURCE_FOLDER)/$(CLIENT_FOLDER)/**/*.c)
SERVER_SOURCES = $(wildcard $(SOURCE_FOLDER)/$(SERVER_FOLDER)/**/*.c)

# objects

PACKET_OBJECTS=$(PACKET_SOURCES:.c=.o)
SEND_RECEIVE_DATA_OBJECTS=$(SEND_RECEIVE_DATA_SOURCES:.c=.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS=$(SERVER_SOURCES:.c=.o)

# Default target
all: clean server client

# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean

# We use an implicit rule: look for the files record.c/database.c,
# compile them and link the resulting *.o's into an executable named database
#database: record.o database.o

client: $(CLIENT_OBJECTS) $(PACKET_OBJECTS); \
		$(CC) $(CFLAGS) $(CLIENT_OBJECTS) $(LDFLAGS)

server: $(SERVER_OBJECTS) $(PACKET_OBJECTS); \
		$(CC) $(CFLAGS) $(SERVER_OBJECTS) $(LDFLAGS)

$(SOURCE_FOLDER)/paquet: $(PACKET_OBJECTS); \
   		$(CC) $(CFLAGS) -lz $(PACKET_OBJECT) $(LDFLAGS)

tests: $(PACKET_OBJECTS) $(TESTS_OBJECTS); \
		$(CC) $(CFLAGS) -lcunit $(LDFLAGS)

.PHONY: clean

clean:
	@rm -f *.o