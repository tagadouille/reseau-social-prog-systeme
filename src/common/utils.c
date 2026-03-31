#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../../includes/utils.h"

/*
 * Lit exactement 'len' octets sur le réseau.
 * On utilise une boucle car TCP coupe parfois les messages en plusieurs morceaux.
 * Retourne le nombre d'octets lus, ou -1 si erreur/déconnexion.
 */
int recv_all(int sock, char *buf, int len)
{
    int received = 0;
    while (received < len)
    {
        // On décale le pointeur (buf + received) pour remplir la suite du tableau
        // On demande uniquement les octets manquants (len - received)
        int n = recv(sock, buf + received, len - received, 0);

        if (n < 0)
        {
            perror("Erreur recv");
            return -1;
        }
        if (n == 0)
        {
            printf("Connexion coupée par le client.\n");
            return -1;
        }
        received += n;
    }
    return received;
}

/*
 * Envoie exactement 'len' octets sur le réseau.
 * On boucle pour garantir que tout le buffer part sur le réseau.
 */
int send_all(int sock, const char *buf, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int n = send(sock, buf + sent, len - sent, 0);

        if (n < 0)
        {
            perror("Erreur send");
            return -1;
        }
        if (n == 0)
        {
            return -1;
        }
        sent += n;
    }
    return sent;
}

/**
 * Permet de lire et renvoit le codereq d'un buffer
 * RETURN VALUE: -1 si problème recv_all sinon le codereq du buffer.
 */
int read_codereq(int sock, u8 *buf_header)
{
    if (recv_all(sock, (char *)buf_header, 2) < 0)
        return -1;

    uint16_t code;
    memcpy(&code, buf_header, 2);
    code = ntohs(code);

    return (code >> 11) & 0x1F; // 5 bits de tête
}
