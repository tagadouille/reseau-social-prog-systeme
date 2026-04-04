/**
 * @file utils.h
 * @brief Fonctions utilitaires communes pour le projet Paroles.
 *
 * Ce fichier déclare des fonctions d'aide globales, notamment des raccourcis de notations de
 * structures prédifinies en C afin de faciliter l'écriture de code et la lecture
 */
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>

#define BUF_SIZE 512

// Masques pour limiter la taille des données (le nombre de bits à 1)
#define MASK_11_BITS 0x07FF // Binaire: 0000 0111 1111 1111

#define MASK_5_BITS 0x1F // Binaire: 0000 0000 0001 1111

#define MASK_2_BITS 0x03 // Binaire: 0000 0000 0000 0011

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// --- Fonctions réseau (Garantissent qu'on reçoit/envoie TOUT) inspiré du cours ---
int recv_all(int sock, char *buf, int len);
int send_all(int sock, const char *buf, int len);

int read_codereq(int sock, u8 *buf_header);

/**
 * @brief permet de lire l'id du groupe ou 
 * de l'utilisateur contenu dans le buffer
 * de requête du client et de le renvoyer
 */
int read_id(u8 * buffer);

/**
 * @brief fonction qui converti une adresse IPV6
 * en char*. IMPORTANT : Il faut libérer la chaîne retournée !
 * 
 * @param addr l'adresse qui est dans un tableau
 * de uint8_t
 * 
 * @return NULL si il y a eu une erreur, le string de 
 * l'adresse sinon
 */
char* IPV6_addr_to_string(u8 * addr);

#endif
