#ifndef BOMBERC_COMMON_H
#define BOMBERC_COMMON_H

void print_lvl(level map){

    int i,j;
    for(i=0;i<LVL_W;i++)
        for(j=0;j<LVL_H;j++)
            printf("%c",map.terrain[i][j]);
}

#endif //BOMBERC_COMMON_H
