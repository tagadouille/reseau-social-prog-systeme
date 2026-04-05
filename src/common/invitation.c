#include <stdio.h>
#include <stdlib.h>

#include "invitation.h"

invitation_t *invitation_create(int group_id, int user_id)
{
    invitation_t *invitation = malloc(sizeof(invitation_t));

    if (invitation == NULL)
    {
        perror("malloc invitation create");
        return NULL;
    }

    invitation->group_id = group_id;
    invitation->user_id = user_id;

    return invitation;
}

void invitation_destroy(invitation_t *invitation)
{
    if (invitation != NULL)
    {
        free(invitation);
    }
}