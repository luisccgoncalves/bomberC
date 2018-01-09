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

void openSpipe(char *pipename){

    if(access(pipename, F_OK)==-1)  //If pipe isn't created, server is offline
        error(-1, 0, "ERROR - Server is offline");
    sPipeFd=open(pipename,O_RDWR);
    if(sPipeFd==0)
        error(-1,0,"ERROR - Could not open pipe.");
}

void openCpipe(char *pipename){

    if(access(pipename, F_OK)==-1)
        if(mkfifo(pipename, 0777)<0)
            error(-1,0,"ERROR - Could not create pipe.");
    cPipeFd=open(pipename, O_RDWR);
    if(cPipeFd==0)
        error(-1,0,"ERROR - Could not open pipe.");
}

void openpipe(char *pipename){

    if(access(pipename, F_OK)==-1)
        if(!strcmp(S_PIPE,pipename))    //If server pipe isn't created, quits
            error(-1, 0, "ERROR - Server is offline");

        else if(mkfifo(pipename, 0777)<0) //If client pipe isn't created, create it
            error(-1,0,"ERROR - Could not create pipe.");

    if(!strcmp(S_PIPE,pipename)){

        sPipeFd=open(pipename,O_RDWR);
        if(sPipeFd==0)
            error(-1,0,"ERROR - Could not open pipe.");
    }
    else {

        cPipeFd=open(pipename, O_RDWR);
        if(cPipeFd==0)
            error(-1,0,"ERROR - Could not open pipe.");
    }

}

user login(user newUser){

    newUser.authOK=0;
    printf("bomberC\nPlease login.\n");
    while(newUser.authOK==0){
        printf("user:");
        scanf(" %49[^\n]s",newUser.user);
        printf("pass:");
        scanf(" %49[^\n]s",newUser.passwd);
        newUser.pid=getpid();

        write(sPipeFd,&newUser, sizeof(user));

        read(cPipeFd,&newUser,sizeof(user));

        ServerPID=newUser.pid;

        if(newUser.authOK==0)
            printf("Wrong username or password.\n");
        else
            printf("Welcome %s.\n",newUser.user);
    }

    return newUser;
}

int main(int argc, char** argv) {

    user        newUser;
    int         running=1;
    int         arg_n;              //Custom shell argc equivalent
    char        uinput[USR_LINE];
    char        args[3][USR_TAM];   //Custom shell argv equivalent
    char        buffer[USR_TAM];

    setbuf(stdout, NULL);
    signal(SIGINT, signal_handler);


    openpipe(S_PIPE);    //Open Server Pipe

    sprintf(buffer,"%s_%d",C_PIPE,getpid());
    openpipe(buffer);    //Open Client Pipe

    newUser=login(newUser);

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
