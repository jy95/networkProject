# See gcc/clang manual to understand all flags
CFLAGS += -std=gnu99 # Define which version of the C standard to use #WARNING : gnu99 sur machine Intel
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
#INCLUDES += -I$(HOME)/local/include
#INCLUDES += -L$(HOME)/local/lib

# Default compiler
CC=gcc

# folders
SOURCE_FOLDER=src
TESTS_FOLDER=tests
PAQUET_FOLDER=paquet
SEND_RECEIVE_DATA_FOLDER=sendAndReceiveData
CLIENT_FOLDER=client
SERVER_FOLDER=server
STACK_FOLDER=packet_stack
WINDOW_SERVER_FOLDER=server_window

# folder of sources
PAQUET_FULL_PATH=$(SOURCE_FOLDER)/$(PAQUET_FOLDER)
SEND_RECEIVE_DATA_FULL_PATH=$(SOURCE_FOLDER)/$(SEND_RECEIVE_DATA_FOLDER)
CLIENT_FULL_PATH=$(SOURCE_FOLDER)/$(CLIENT_FOLDER)
SERVER_FULL_PATH=$(SOURCE_FOLDER)/$(SERVER_FOLDER)
STACK_FULL_PATH=$(SOURCE_FOLDER)/$(STACK_FOLDER)
WINDOW_SERVER_FULL_PATH=$(SOURCE_FOLDER)/$(WINDOW_SERVER_FOLDER)

# sources files
# On prend tout
PACKET_SOURCES = $(wildcard $(PAQUET_FULL_PATH)/*.c)
SEND_RECEIVE_DATA_SOURCES = $(wildcard $(SEND_RECEIVE_DATA_FULL_PATH)/*.c)
CLIENT_SOURCES = $(wildcard $(CLIENT_FULL_PATH)/*.c)
SERVER_SOURCES = $(wildcard $(SERVER_FULL_PATH)/*.c)
TESTS_SOURCES = $(wildcard $(TESTS_FOLDER)/*.c)
STACK_SOURCES = $(wildcard $(STACK_FULL_PATH)/*.c)
WINDOW_SERVER_SOURCES = $(wildcard $(WINDOW_SERVER_FULL_PATH)/*.c)

#Tous les fichiers nécessaires pour builder
# Merci l'idée de https://stackoverflow.com/a/170472/6149867
SENDER_SRC += $(PACKET_SOURCES)
SENDER_SRC += $(SEND_RECEIVE_DATA_SOURCES)
SENDER_SRC += $(CLIENT_SOURCES)

RECEIVER_SRC += $(PACKET_SOURCES)
RECEIVER_SRC += $(SEND_RECEIVE_DATA_SOURCES)
RECEIVER_SRC += $(STACK_SOURCES)
RECEIVER_SRC += $(WINDOW_SERVER_SOURCES)
RECEIVER_SRC += $(SERVER_SOURCES)

TESTS_SRC += $(PACKET_SOURCES)
TESTS_SRC += $(TESTS_SOURCES)

ALL_OBJ += $(SENDER_SRC:.c=.o)
ALL_OBJ += $(RECEIVER_SRC:.c=.o)
ALL_OBJ += $(TESTS_SRC:.c=.o)
# another things

# Default target
all: clean sender receiver

sender: $(SENDER_SRC); \
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS);

receiver: $(RECEIVER_SRC); \
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS);

#$(INCLUDES) si nécessaire
tests: $(TESTS_SRC); \
		$(CC) $(CFLAGS) $^ $(LDFLAGS) -o testsScript;

.PHONY: clean

clean:
	@rm -f $(ALL_OBJ) sender receiver testsScript