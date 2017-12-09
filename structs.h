#ifndef STRUCTS_H
#define STRUCTS_H

#define USR_TAM     50              //defines user string sizes
#define USR_LINE    1024            //defines whole line max size
#define LVL_W       20              //map width
#define LVL_H       30              //map height
#define S_PIPE      "/tmp/sPipe"    //Path and filename of server pipe
#define C_PIPE      "/tmp/cPipe"    //Path and file prefix of a client pipe

typedef struct {                    //sent from client to server with auth request
    int     pid;                    //creates a unique pipe to callback
    char    user[USR_TAM];
    char    passwd[USR_TAM];
    int     authOK;
}user;

typedef struct {                    //used to create an array usernames and password
    char    user[USR_TAM];
    char    passwd[USR_TAM];
}db;

typedef struct{
    int     x_pos;      //bomb x position
    int     y_pos;      //bomb y position
    int     bomb_power; //2 or 4 squares of range minreq
    unsigned long fuse; //timestamp of bomb placement, fuse goes off after 2s
}bomb;

typedef struct{         //enemy only has x y position
    int     x_pos;
    int     y_pos;
}enemy;

typedef struct{
    char    user[USR_TAM];
    int     x_pos;      //player x position
    int     y_pos;      //player y position
    int     n_bombs;    //number of bombs
    int     n_bobombs;  //number of big bombs
    int     points;     //player points
    int     pid;
    char    pipename[USR_TAM];
    int     fd;
}bomber;

typedef struct{         //object only has x y position
    int     x_pos;
    int     y_pos;
}object;

typedef struct{
    char    terrain[LVL_W][LVL_W];
    int     n_obj;      //number of tokens to collect
    int     n_enemies;  //number of enemies
    int     exit[2];    //exit x y position, enabled when n_obj=0
}level;

typedef struct{
    int     sPipeFd;
    int     userdb_size;
    db      userdb[100];
    int     n_players;
    bomber  player[20];
}database;


#endif /* STRUCTS_H */