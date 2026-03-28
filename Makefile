# --- CONFIGURATION ---
CC := gcc
SRC_DIR := src
INC_DIR := includes
OBJ_DIR := obj

# --- CIBLES ---
CLIENT_EXEC := client
SERVER_EXEC := server

# --- OPTIONS ---
CFLAGS := -I $(INC_DIR) -Wall -Wextra -Werror -g

# --- FICHIERS ---
# On récupère tout ce qui est commun
COMMON_SRC := $(wildcard $(SRC_DIR)/common/*.c)
HEADERS := $(wildcard $(INC_DIR)/*.h)

# Sources spécifiques
CLIENT_SRC := $(SRC_DIR)/client.c $(COMMON_SRC)
SERVER_SRC := $(SRC_DIR)/server.c $(COMMON_SRC)

# Transformation magique : src/chemin/fichier.c -> obj/chemin/fichier.o
CLIENT_OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRC))
SERVER_OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRC))

# --- RÈGLES ---

all: $(CLIENT_EXEC) $(SERVER_EXEC)

# Règle pour l'exécutable Client
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Règle pour l'exécutable Serveur
$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Règle générique pour les objets dans le dossier obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean re

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC)
	@echo "Nettoyage terminé."

re: clean all