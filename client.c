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

int openPipe(char *pipename){
    int fd;

    if(access(pipename, F_OK)==-1)
        if(mkfifo(pipename, 0777)<0)
            error(-1,0,"ERROR - Could not create pipe.");
    fd=open(pipename, O_RDWR);
    if(fd==0)
        error(-1,0,"ERROR - Could not open file.");
    return fd;
}

int main(int argc, char** argv) {

    user newUser;
    int running=1;
    int sPipeFd, cPipeFd;
    char buffer[USR_TAM];

    setbuf(stdout, NULL);

    newUser.pid=0;
    printf("bomberC\nPlease login.\n");
    while(newUser.pid==0){
        printf("user:");
        scanf(" %49[^\n]s",newUser.user);
        printf("pass:");
        scanf(" %49[^\n]s",newUser.passwd);
        newUser.pid=getpid();

        sPipeFd=open(S_PIPE,O_RDWR);
            //control
        write(sPipeFd,&newUser, sizeof(user));
        sprintf(buffer,"%s_%d",C_PIPE,newUser.pid);
        printf("%s\n",buffer);

        cPipeFd=openPipe(buffer);
        //cPipeFd=open(buffer,O_RDWR);
        read(cPipeFd,&newUser,sizeof(user));
        printf("----%d\n",newUser.authOK);
        if(newUser.authOK==0)
            printf("Wrong username or password.\n");
        else
            printf("Welcome %s.\n",newUser.user);
    }

    return (EXIT_SUCCESS);
}
