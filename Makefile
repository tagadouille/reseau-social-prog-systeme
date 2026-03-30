# --- CONFIGURATION ---
CC := gcc
SRC_DIR := src
INC_DIR := includes
OBJ_DIR := obj

# --- CIBLES ---
CLIENT_EXEC := client
SERVER_EXEC := server

# --- OPTIONS ---
CFLAGS := -I $(INC_DIR) -Wall -Wextra -Werror -g -MMD -MP

# --- FICHIERS ---
# On récupère tout ce qui est commun sauf client.c et server.c
COMMON_SRC := $(filter-out $(SRC_DIR)/client.c $(SRC_DIR)/server.c, \
               $(shell find $(SRC_DIR) -name "*.c"))

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
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d)

.PHONY: clean re

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC)
	@echo "Nettoyage terminé."

re: clean all