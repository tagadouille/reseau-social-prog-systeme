/**
 * @file utils.c
 * @brief Implémentation des fonctions utilitaires de gestion mémoire sécurisée.
 *
 * Ce fichier fournit des versions "sécurisées" pour les fonctions
 * d'allocation mémoire standard (malloc, realloc). Ces versions
 * ("x-fonctions") terminent le programme en cas d'échec d'allocation,
 * évitant ainsi de devoir tester le retour de NULL à chaque appel.
 */

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

/**
 * @brief Alloue de la mémoire de manière sécurisée (exit on failure).
 * Tente d'allouer 'size' octets de mémoire en utilisant malloc.
 * Si l'allocation échoue (malloc retourne NULL), un message d'erreur
 * est affiché via perror et le programme se termine avec EXIT_FAILURE.
 *
 * @param size Le nombre d'octets à allouer.
 * @return void* Un pointeur vers la zone mémoire allouée.
 * @note Cette fonction ne retourne jamais NULL.
 */
void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL)
    {
        perror("Allocation mémoire échouée (xmalloc)");
        exit(EXIT_FAILURE);
    }
    return p;
}

/**
 * @brief Réalloue de la mémoire de manière sécurisée (exit on failure).
 *
 * Tente de réallouer la zone mémoire pointée par 'ptr' pour qu'elle
 * fasse 'size' octets, en utilisant realloc.
 * Si la réallocation échoue (realloc retourne NULL), un message d'erreur
 * est affiché via perror et le programme se termine avec EXIT_FAILURE.
 *
 * @param ptr Pointeur vers la zone mémoire précédemment allouée (ou NULL).
 * @param size La nouvelle taille en octets de la zone mémoire.
 * @return void* Un pointeur vers la (potentiellement) nouvelle zone mémoire.
 * @note Cette fonction ne retourne jamais NULL.
 */
void *xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (p == NULL)
    {
        perror("Réallocation mémoire échouée (xrealloc)");
        exit(EXIT_FAILURE);
    }
    return p;
}
