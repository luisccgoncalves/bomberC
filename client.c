#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>


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
        printf("\n%s\n%s\n%d",newUser.user,newUser.passwd, newUser.pid);
    }

    int     fd;
    //user    *buffer;

    if(access("/tmp/sPipe", F_OK)==-1)
        if(mkfifo("/tmp/sPipe", S_IRWXU)<0)
            error(-1,0,"ERROR - Could not create pipe.");
    fd=open("/tmp/sPipe", O_RDWR);
    if(fd==0)
        error(-1,0,"ERROR - Could not open file.");

    write(fd,&newUser, sizeof(user));

    return (EXIT_SUCCESS);
}
