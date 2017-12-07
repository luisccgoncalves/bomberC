#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char** argv) {

    user newUser;
    int running=1;

    printf("bomberC\nPlease login.\n");
    while(running){
        printf("user:");
        scanf(" %49[^\n]s",newUser.user);
        printf("pass:");
        scanf(" %49[^\n]s",newUser.passwd);
        newUser.pid=getpid();
        running=0;
    }

    return (EXIT_SUCCESS);
}
