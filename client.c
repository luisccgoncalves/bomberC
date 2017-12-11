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

//############################################ GLOBAL VARIABLES

int sPipeFd, cPipeFd;
int ServerPID;

//############################################

void gracefullexit(){

    char clt_pipe[USR_TAM];

    sprintf(clt_pipe,"%s_%d",C_PIPE,getpid());

    close(sPipeFd);
    close(cPipeFd);
    unlink(clt_pipe);
}

void signal_handler(int signum){

    if(signum==SIGINT){
        gracefullexit();
        printf("\nSHUTTING DOWN.\n");
        exit(0);
    }
}

void *pingpong(void *ptr){

    while(1){
        if(access(S_PIPE, F_OK)==-1) { //If pipe isn't created, server is offline
            gracefullexit();
            error(-1, 0, "ERROR - Server is offline");
        }
        sleep(5);
    }
}

int main(int argc, char** argv) {

    user        newUser;
    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    char        buffer[USR_TAM];
    pthread_t   keepalive;

    setbuf(stdout, NULL);
    signal(SIGINT, signal_handler);


//=================================================== OPEN SERVER PIPE

    if(access(S_PIPE, F_OK)==-1)  //If pipe isn't created, server is offline
        error(-1, 0, "ERROR - Server is offline");
    sPipeFd=open(S_PIPE,O_RDWR);
    if(sPipeFd==0)
        error(-1,0,"ERROR - Could not open pipe.");

//====================================================================


    newUser.authOK=0;
    printf("bomberC\nPlease login.\n");
    while(newUser.authOK==0){
        printf("user:");
        scanf(" %49[^\n]s",newUser.user);
        printf("pass:");
        scanf(" %49[^\n]s",newUser.passwd);
        newUser.pid=getpid();

        write(sPipeFd,&newUser, sizeof(user));
        sprintf(buffer,"%s_%d",C_PIPE,newUser.pid);


//=================================================== CREATES CLIENT PIPE

        if(access(buffer, F_OK)==-1)
            if(mkfifo(buffer, 0777)<0)
                error(-1,0,"ERROR - Could not create pipe.");
        cPipeFd=open(buffer, O_RDWR);
        if(cPipeFd==0)
            error(-1,0,"ERROR - Could not open pipe.");

//====================================================================


        read(cPipeFd,&newUser,sizeof(user));

        ServerPID=newUser.pid;
        //Creates a thread to send/receive keepalive signals
        if(pthread_create(&keepalive,NULL, pingpong, NULL)!=0)
            error(-1,0,"ERROR - Error creating thread");

        if(newUser.authOK==0) {
            running=0;
            printf("Wrong username or password.\n");
        }
        else
            printf("Welcome %s.\n",newUser.user);
    }

    while(running) {
        printf("bomber#>");
        scanf(" %1023[^\n]s", uinput);
        arg_n = sscanf(uinput, "%s %s %s", args[0], args[1], args[2]);

        if (!strcmp(args[0], "exit"))
            running = 0;
    }

    gracefullexit();

    return (EXIT_SUCCESS);
}
