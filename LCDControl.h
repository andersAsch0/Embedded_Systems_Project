#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H

#include "spiAVR.h"
#include "gameData.h"
#include "helper.h"

void sendSPICommand(char command){
    PORTD = SetBit(PORTD, 6, 0); //send command
    SPI_SEND(command);
}
void sendSPIData(char data){
    PORTD = SetBit(PORTD, 6, 1); //send data
    SPI_SEND(data);

}
/// @brief gets the address of the pixel on the top right corner of a grid square (16x16)
char getLCDGridAddress(char gridNumber){
    return((128/8 * gridNumber) + 1);
}
char getLCDBulletAddress(char bulletCoord){
    return((128/32 * bulletCoord) + 1);
}

void LCDSelectGridArea(char startRow, char endRow, char startCol, char endCol){
    sendSPICommand(0x2A); //set col
    sendSPIData(0x00);
    sendSPIData(getLCDGridAddress(startCol%8) + 1); //cols start at address 2 for some reason??
    sendSPIData(0x00);
    sendSPIData(getLCDGridAddress(endCol%8)+15 + 1);


    sendSPICommand(0x2B); //set row
    sendSPIData(0x00);
    sendSPIData(getLCDGridAddress(startRow%8));
    sendSPIData(0x00);
    sendSPIData(getLCDGridAddress(endRow%8)+15);
}
void LCDSelectBulletArea(char startRow, char endRow, char startCol, char endCol){
    sendSPICommand(0x2A); //set col
    sendSPIData(0x00);
    sendSPIData(getLCDBulletAddress(startCol%32) + 1); //cols start at address 2 for some reason??
    sendSPIData(0x00);
    sendSPIData(getLCDBulletAddress(endCol%32)+3 + 1);


    sendSPICommand(0x2B); //set row
    sendSPIData(0x00);
    sendSPIData(getLCDBulletAddress(startRow%32));
    sendSPIData(0x00);
    sendSPIData(getLCDBulletAddress(endRow%32)+3);
}
/// @brief set all on the LCD pixels to black
void clearLCD(){
    LCDSelectGridArea(0,7,0,7);
    sendSPICommand(0x2C);
    for(unsigned short i = 0; i < 16384; i++){ 
        sendSPIData(0x00);
        sendSPIData(0x00);
        sendSPIData(0x00);
    }
}

/// @param spritePtr address to the location of the first element in the sprite array
void LCDSendSpriteData(const unsigned char *spritePtr, unsigned short numPixels){
    sendSPICommand(0x2C);
    for(unsigned short i = 0; i < numPixels; i ++ ){
        sendSPIData(colors[pgm_read_byte(spritePtr+i)][2]);
        sendSPIData(colors[pgm_read_byte(spritePtr+i)][1]);
        sendSPIData(colors[pgm_read_byte(spritePtr+i)][0]);
    }

}
/// @brief used only for indicating a new highscore by turning text yellow
void LCDSendSpriteDataYellow(const unsigned char *spritePtr, unsigned short numPixels){
    sendSPICommand(0x2C);
    for(unsigned short i = 0; i < numPixels; i ++ ){
        sendSPIData(0x00); //blue is always 0
        if(colors[pgm_read_byte(spritePtr+i)][0]){
            sendSPIData(0xFF); //red and green - send yellow
            sendSPIData(0xFF);
        }
        else{
            sendSPIData(0x00); //send black
            sendSPIData(0x00);
        }
    }

}

/// @brief draw the square, checks what is should be
void LCDDrawGridSquare(unsigned char row, unsigned char col){
    LCDSelectGridArea(row, row, col, col);

    switch (gameData[row%8][col%8])
    {
    case 0: //background
        sendSPICommand(0x2C);
        for(unsigned short i = 0; i < 256; i++){ //fill the grid square w background color
            sendSPIData(0x00);
            sendSPIData(0x00);
            sendSPIData(0x00);
        }
        break;
    case 1: //player
        LCDSendSpriteData(&tankSprite[0][0], 256);
        break;
    case -1: //enemy
        LCDSendSpriteData(&alienSprite[0][0], 256);
        break;
    case -2: //explosion
        LCDSendSpriteData(&explosionSprite[0][0], 256);
        break;
    default:
        break;
    }
}
void LCDDrawBullets(){
    for(unsigned char i = 0; i < 16; i++){
        if(bulletArray[i].row != -1){
            LCDSelectBulletArea(bulletArray[i].row, bulletArray[i].row, bulletArray[i].col, bulletArray[i].col);
            LCDSendSpriteData(&bulletSprite[0][0], 16);
        }
    }
}
/// @brief aka update all of the grid squares of the gameplay screen
void drawFullGameScreen(){
    for(unsigned char row = 0; row < 8; row++){
        for(unsigned char col = 0; col < 8; col++){
            LCDDrawGridSquare(row, col);
        }
    }
}
void drawNumber(unsigned char row, unsigned char startingCol, unsigned short number){
    LCDSelectGridArea(row, row, startingCol+3, startingCol+3);
    LCDSendSpriteData(numSpritesArr[number%10], 256);
    LCDSelectGridArea(row, row, startingCol+2, startingCol+2);
    LCDSendSpriteData(numSpritesArr[(number/10)%10], 256);
    LCDSelectGridArea(row, row, startingCol+1, startingCol+1);
    LCDSendSpriteData(numSpritesArr[(number/100)%10], 256);
    LCDSelectGridArea(row, row, startingCol, startingCol);
    LCDSendSpriteData(numSpritesArr[(number/1000)%10], 256);
}
void drawNumberYellow(unsigned char row, unsigned char startingCol, unsigned short number){
    LCDSelectGridArea(row, row, startingCol+3, startingCol+3);
    LCDSendSpriteDataYellow(numSpritesArr[number%10], 256);
    LCDSelectGridArea(row, row, startingCol+2, startingCol+2);
    LCDSendSpriteDataYellow(numSpritesArr[(number/10)%10], 256);
    LCDSelectGridArea(row, row, startingCol+1, startingCol+1);
    LCDSendSpriteDataYellow(numSpritesArr[(number/100)%10], 256);
    LCDSelectGridArea(row, row, startingCol, startingCol);
    LCDSendSpriteDataYellow(numSpritesArr[(number/1000)%10], 256);
}

void drawIntroScreen(){
    clearLCD();
    LCDSelectGridArea(1, 4, 0, 7);
    LCDSendSpriteData(&titleSprite[0][0], 8192);
    LCDSelectGridArea(6, 7, 2, 5);
    LCDSendSpriteData(&newGameOption[0][0], 2048);
}

void drawPauseScreen(){
    clearLCD();
    LCDSelectGridArea(0, 2, 3, 4);
    LCDSendSpriteData(&pauseIcon[0][0], 1536);
    LCDSelectGridArea(4, 5, 0, 7);
    LCDSendSpriteData(&pauseOptionsSprite[0][0], 4096);
    LCDSelectGridArea(6, 6, 0, 3);
    LCDSendSpriteData(&yourScoreText[0][0], 1024);
    LCDSelectGridArea(7, 7, 0, 3);
    LCDSendSpriteData(&highscoreText[0][0], 1024); 
    drawNumber(6, 4, currScore);
    drawNumber(7, 4, highScore);

}

void drawDeathScreen(unsigned char hasNewHighscore){
    clearLCD();
    LCDSelectGridArea(1, 3, 2, 5);
    LCDSendSpriteData(&deathScreenSprite[0][0], 2240);
    LCDSelectGridArea(4, 5, 2, 5);
    LCDSendSpriteData(&newGameOption[0][0], 2048);
    LCDSelectGridArea(6, 6, 0, 3);
    LCDSendSpriteData(&yourScoreText[0][0], 1024);
    drawNumber(6, 4, currScore);
    LCDSelectGridArea(7, 7, 0, 3);
    if(hasNewHighscore) {
        LCDSendSpriteDataYellow(&highscoreText[0][0], 1024); 
        drawNumberYellow(7, 4, highScore); 
    }
    else   {
        LCDSendSpriteData(&highscoreText[0][0], 1024); 
        drawNumber(7, 4, highScore); 
    }
}

void drawExplosion(unsigned char row, unsigned char col){
    LCDSelectGridArea(row, row, col, col);
    LCDSendSpriteData(&explosionSprite[0][0], 256);
}



#endif