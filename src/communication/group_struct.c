#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "communication/group_struct.h"

group_t * group_initialize(int group_id, const u8 *group_name, int id_admin)
{
    return group_fill(group_id, group_name, id_admin, NULL, 0, (u8[16]){0});
}

group_t * group_fill(int group_id, const u8 *group_name, int id_admin, int *members_id, int mdiff_port, u8 mdiff_addr[16])
{
    group_t *group = malloc(sizeof(group_t));

    if (group == NULL)
    {
        perror("malloc group fill");
        return NULL;
    }

    group->group_id = group_id;

    group -> group_name = malloc(strlen((char*) group_name) + 1);
    memcpy(group->group_name, group_name, strlen((char*) group_name) + 1);

    group->id_admin = id_admin;
    group->members_id = members_id;
    group->mdiff_port = mdiff_port;
    memcpy(group->mdiff_addr, mdiff_addr, 16);

    return group;
}

group_t * group_fill_from_files(int group_id)
{
    // TODO
    printf("%d", group_id);
    return NULL;
}

void group_struct_destroy(group_t* group)
{
    if (group != NULL)
    {
        if(group->group_name != NULL)
        {
            free(group->group_name);
        }

        if(group->members_id != NULL)
        {
            free(group->members_id);
        }
        free(group);
    }
}