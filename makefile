CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

SRC_DIR = ./src/
OBJ_DIR = ./obj/
INC_DIR = -I ./includes/

# Cibles principales
TARGET_CLIENT = client
TARGET_SERVER = server

# Fichiers sources

CLIENT_SRCS = $(wildcard $(SRC_DIR)*.c)
SERVER_SRCS = $(wildcard $(SRC_DIR)*.c)

# Fichiers objets
CLIENT_OBJS = $(CLIENT_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SERVER_OBJS = $(SERVER_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Règles principales
all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_SERVER): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(INC_DIR) -o $@ $^
	@echo " $(TARGET_SERVER) Server compiled"

$(TARGET_CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(INC_DIR) -o $@ $^
	@echo " $(TARGET_CLIENT) Client compiled"

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@


clean:
	rm -rf $(OBJ_DIR) $(TARGET_CLIENT) $(TARGET_SERVER)  

.PHONY: all clean

