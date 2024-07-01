#ifndef GAME_OPERATIONS_H
#define GAME_OPERATIONS_H

#include "gameData.h"

/// @return gets players y location (leftmost = 0)
unsigned char getPlayerLocation(){
    for(unsigned char i = 0; i < 8; i++){
        if (gameData[7][i] == 1) return i;
    }
    return 8; //should never reacht this
}

/// @param dir -1 is left, 1 is right, anything else does nothing
void movePlayer(signed char dir){
    if((dir < -1 )||( dir > 1)) return;
    unsigned char playerLocation = getPlayerLocation();
    if((playerLocation+dir < 8) && (playerLocation+dir > -1)){
        gameData[7][playerLocation] = 0;        needsUpdate[7][playerLocation] = 1;
        gameData[7][playerLocation + dir] = 1;  needsUpdate[7][playerLocation + dir] = 1;
    }
}

signed char triggerSound(SoundsEnum soundEnum){
    if((soundProgress == SOUNDLENGTH) || (soundEnum <= currSound)){ //only override currently playing noise if its already finished OR new sound is higher priority
        currSound = soundEnum;
        soundProgress = 0;
        return 1;
    }
    return 0;
}

void spawnEnemy(unsigned char col){
    if(gameData[0][col] != -1) currNumEnemies++; //dont add to the count if there already an enemy in this position
    gameData[0][col] = -1;
    needsUpdate[0][col] = 1;
}
void removeEnemy(unsigned char row, unsigned char col){
    if((row>=0)&&(row<8)&&(col>=0)&&(col<8) && (gameData[row][col] ==-1)) {
        gameData[row][col] = 0;
        needsUpdate[row][col] = 1;
        }
}
unsigned char advanceEnemies(){ //returns 1 if player has died
    for(signed char row = 7; row >= 0; row--){
        for(signed char col = 7; col >= 0; col--){
            if(gameData[row][col] == -1){ //move enemies down
                if((row == 7) || (gameData[row+1][col] == 1)) {removeEnemy(row,col); return 1;} //player has died
                else{
                    removeEnemy(row, col);
                    gameData[row+1][col] = -1;
                    needsUpdate[row+1][col] = 1;
                }
            }
            else if (gameData[row][col] == -2){ //remove explosions
                gameData[row][col] = 0;
                needsUpdate[row][col] = 1;
            }
        }
    }
    return 0;
}


void addBullet(){ //does nothing if array is full
    for(unsigned char i = 0; i < 16; i++){
        if(bulletArray[i].row == -1){
            bulletArray[i].row = 7*4;
            bulletArray[i].col = getPlayerLocation()*4 + 1;
            needsUpdate[6][getPlayerLocation()] = 1;
            return;
        }
    }
}
void removeBullet(unsigned char bulletArrayIndex){
    if(sizeof(bulletArray)/sizeof(unsigned char) > bulletArrayIndex){
        bulletArray[bulletArrayIndex].row = -1;
        bulletArray[bulletArrayIndex].col = -1;
        }
    return;
}
void advanceBullets(){
    for(unsigned char i = 0; i < 16; i++){
        if(bulletArray[i].row != -1){
            needsUpdate[bulletArray[i].row / 4][bulletArray[i].col / 4] = 1; //update square its leaving
            bulletArray[i].row--; //will naturally return to -1 when it leaves screen, col will still b pos but who cares
            if((gameData[bulletArray[i].row / 4][bulletArray[i].col / 4] == -1)) { //if its hit an enemy
                triggerSound(ENEMYSOUND);
                removeEnemy(bulletArray[i].row / 4, bulletArray[i].col / 4);
                gameData[bulletArray[i].row / 4][ bulletArray[i].col / 4] = -2; 
                removeBullet(i);
                currScore++;
                currNumEnemies--;
                }
            else if((gameData[bulletArray[i].row / 4 + 1][bulletArray[i].col / 4] == -1)){ //this is to catch the edge case when the bullet & enemy pass over eachother in the same tick
                triggerSound(ENEMYSOUND);
                removeEnemy(bulletArray[i].row / 4 + 1, bulletArray[i].col / 4);
                gameData[bulletArray[i].row / 4 + 1][ bulletArray[i].col / 4] = -2; 
                removeBullet(i);
                currScore++; 
                currNumEnemies--;
            }
        }
    }
    return;
}


void resetPlayerAndEnemy(){
    for(unsigned char i = 0; i < 8; i++){
        for(unsigned char j = 0; j < 8; j++){
            gameData[i][j] = 0;
        }
    }
    gameData[7][3] = 1;
    currNumEnemies = 0;
}

void resetGame(){
    resetPlayerAndEnemy();
    bulletArrayInit();
    currScore = 0;
    currDifficulty = 0;
}
/// @brief returns a 1 or zero w probability depending on probof
/// @param probOf1 probability of 1 (in percentage), negative means 0%, >100 means 100
unsigned char flipCoin(unsigned char probOf1){
    return ((rand()%100) < (probOf1));
}

unsigned char shouldSpawnEnemy(unsigned char difficulty){
    static unsigned char spawnTypeFlip = 0; // 0 means spawn based on curr number to try to get close to difficulty map, 1 means spawn based on chance
    spawnTypeFlip = (rand() % 2);
    if(difficulty == 0) return 0;
    if(spawnTypeFlip){ //prob based
        if(difficulty == 1) return(flipCoin(10/8));
        else if(difficulty <= 3) return(flipCoin(12/8));
        else if(difficulty <= 5) return(flipCoin(20/8));
        else if(difficulty <= 10) return(flipCoin(40/8));
        else if(difficulty <= 80) return(flipCoin(100/8));
        else return(flipCoin(100/4));
    }
    else{
        if(difficulty <= 5) return(currNumEnemies < 1); //yes spawn iff there are less than 1 enemies currently
        else if(difficulty <= 15) return(currNumEnemies <= 2);
        else if(difficulty <= 25) return(currNumEnemies <= 3);
        else if(difficulty <= 40) return(currNumEnemies <= 5);
        else if(difficulty <= 65) return(currNumEnemies <= 6);
        else if(difficulty <= 80) return(currNumEnemies <= 7);
        else return(currNumEnemies < 10);
    }



}

#endif