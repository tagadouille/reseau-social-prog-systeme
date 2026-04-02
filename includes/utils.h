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


#endif
