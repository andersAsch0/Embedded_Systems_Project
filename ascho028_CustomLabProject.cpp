#include "timerISR.h"
#include "helper.h" //setbit and getbit
#include "periph.h" //ADC
// #include "serialATmega.h"
#include "LCDControl.h"
#include "gameOperations.h"
#include "queue.h"
// #include <avr/eeprom.h> 
// #include "spiAVR.h"
// #include "gameData.h"

#define NUM_TASKS 10

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

const unsigned long GCD_PERIOD = 50;//TODO:Set the GCD Period
const unsigned char ButtonPeriod = 50;
const unsigned char JoystickPeriod = 50;
const unsigned char LCDUpdatePeriod = 50; 
const unsigned char GameUpdatePeriod = 50;
const unsigned short EnemyAdvancePeriod = 2000;
const unsigned char BulletAdvancePeriod = 100;
const unsigned char BuzzerPeriod = 100;
const unsigned short IncDifficultyPeriod = 5000;

task tasks[NUM_TASKS]; // declared task array with 5 tasks
unsigned long ICR1Value = 16000000/ (50 * 8) - 1;     // 50 = 1/0.02 = 1/ 20 ms
//global communication vars
uint8_t count = 0;
enum InputEnum {RIGHTINPUT, LEFTINPUT, SHOOTINPUT, PAUSEINPUT, RESTARTINPUT, DIEINPUT, NOINPUT};
queue inputs(5);
int currGameState;

void buzzerInit(){
    TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the prescaler to be 8
    //WGM11, WGM12, WGM13 set timer to fast pwm mode
    ICR1 = 39999; //20ms pwm period
    OCR1A = 19999; //50% duty cycle (% of above)
    TCCR1B = (TCCR1B & 0xF8); //buzzer disabled 
}

void setBuzzer(unsigned char enable){
    if(enable) TCCR1B = (TCCR1B & 0xF8) | 0x02; //set prescaler to 8
    else TCCR1B = (TCCR1B & 0xF8); //buzzer disabled 
}

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                       // Reset the elapsed time for the next tick
        }
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


//tick functions for each task
enum JoystickStates{JSLEFT, JSRIGHT, JSCENTER};
int JoystickTick(int currState){
    //mealy
    switch (currState)
    {
    case JSCENTER:
        if( ADC_read(4) < 400) {
            currState = JSRIGHT;
            inputs.push(RIGHTINPUT);
        }
        else if (ADC_read(4) > 600) {
            currState = JSLEFT;
            inputs.push(LEFTINPUT);
        }
        else currState = JSCENTER;
        break;
    case JSLEFT:
        if(( ADC_read(4) <= 600)) currState = JSCENTER;
        else currState = JSLEFT;
        break;
    case JSRIGHT: 
        if(( ADC_read(4) >= 400)) currState = JSCENTER;
        else currState = JSRIGHT;
        break;
    default:
        break;
    }
    return currState;
}
enum ButtonStates{BUNPRESSED, BPRESSED};
int PauseButtonTick(int currState){
    //mealy
    switch (currState)
    {
    case BUNPRESSED:
        if(ADC_read(3) > 100) {
            currState = BPRESSED;
            inputs.push(PAUSEINPUT);  
        }
        break;
    case BPRESSED:
        if(ADC_read(3) <= 100){
            currState = BUNPRESSED;
        }
    default:
        break;
    }
    return currState;
}
int RestartButtonTick(int currState){
    switch (currState)
    {
    case BUNPRESSED:
        if(ADC_read(2) > 100) {
            currState = BPRESSED;
            inputs.push(RESTARTINPUT); 
        }
        break;
    case BPRESSED:
        if(ADC_read(2) <= 100){
            currState = BUNPRESSED;
        }
    default:
        break;
    }
    return currState;
}
int ShootButtonTick(int currState){
    switch (currState)
    {
    case BUNPRESSED:
        if(ADC_read(5) > 100) {
            currState = BPRESSED;
            inputs.push(SHOOTINPUT);    
        }
        break;
    case BPRESSED:
        if(ADC_read(5) <= 100){
            currState = BUNPRESSED;
        }
    default:
        break;
    }
    return currState;
}


enum GAMEUPDATESTATES{GStart, GIntro, GPaused, GDead, GGameplay, GDeathPause};
int GameUpdateTick(int currState){
    if(inputs.isEmpty()) return(currState); 
    static unsigned char input;
    input = inputs.pop();   

    //transitions
    switch (currState)
    {
    case GGameplay:
        if((input == PAUSEINPUT) || (input == RESTARTINPUT)) {
            triggerSound(BUTTONSOUND);
            currState = GPaused;
            drawPauseScreen();
            }
        else if(input == DIEINPUT){
            triggerSound(DEATHSOUND);
            drawExplosion(7, getPlayerLocation());
            currState = GDeathPause;
            //draw explosion
        }
        break;
    case GPaused:
        if(input == PAUSEINPUT){
            triggerSound(BUTTONSOUND);
            currState = GGameplay;
            drawFullGameScreen();
        }
        else if(input == RESTARTINPUT){
            triggerSound(BUTTONSOUND);
            resetGame();
            currState = GGameplay;
            drawFullGameScreen();
        }
        break;
    case GDead:
        if(input == RESTARTINPUT){
            triggerSound(BUTTONSOUND);
            resetGame();
            currState = GGameplay;
            drawFullGameScreen();
        }
        break;
    case GIntro:
        if((input == PAUSEINPUT) || (input == RESTARTINPUT)){
            triggerSound(BUTTONSOUND);
            resetGame();
            currState = GGameplay;
            drawFullGameScreen();
        }
        break;
    case GDeathPause:
        if(input == RESTARTINPUT){ 
            triggerSound(BUTTONSOUND);
            currState = GDead;
            if(highScore > currScore){
                drawDeathScreen(0);
            }
            else{
                highScore = currScore;
                setHighScore(highScore);
                drawDeathScreen(1);
            }
        }
        break;
    case GStart:
        triggerSound(INTROSOUND);
        currState = GIntro;
        drawIntroScreen();
        break;
    default:
        break;
    }
    
    //actions
    switch (currState)
    {
        case GGameplay:
            if(input == RIGHTINPUT) movePlayer(1);
            else if (input == LEFTINPUT) movePlayer(-1);
            else if(input == SHOOTINPUT) {addBullet(); triggerSound(SHOOTSSOUND);}
        default:
            break;
    }
    

    currGameState = currState;
    return currState;
}

enum LCDUPDATESTATES{LCDU};
int LCDUpdateTick(int currState){
    if(currGameState == GGameplay){
        for(unsigned char row = 0; row < 8; row++){
            for(unsigned char col = 0; col < 8; col++){
                if(needsUpdate[row][col] != 0){
                    LCDDrawGridSquare(row, col);
                    needsUpdate[row][col] = 0;
                }
                
            }
        }
        LCDDrawBullets();
    }
    return currState;
}

enum ENEMYADVANCESTATES{EA};
int EnemyAdvanceTick(int currState){
    if(currGameState != GGameplay) return currState; //do nothing if paused, dead, etc

    if(advanceEnemies()) inputs.push(DIEINPUT);
    for(unsigned char i = 0; i < 8; i++) if(flipCoin(currDifficulty)) spawnEnemy(i);

    return currState;
}
enum BULLETADVANCESTATES{BA};
int BulletAdvanceTick(int currState){
    if(currGameState != GGameplay) return currState; //do nothing if paused, dead, etc
    advanceBullets();
    return currState;
}

enum BUZZERSTATES {Buzz};
int BuzzerTick(int currState){
    if((soundProgress < SOUNDLENGTH) && (soundsArray[currSound][soundProgress] != -1)){
        if(!soundsArray[currSound][soundProgress])setBuzzer(0); //if its 0, buzzer should be silent
        else setBuzzer(1);
        ICR1 = 16000000 / (soundsArray[currSound][soundProgress] * 8) -1; //translate the Hz
        OCR1A = ICR1 / 2; //50% duty cycle (% of above)
        soundProgress++;
    }
    else{
        setBuzzer(0);
        if (soundsArray[currSound][soundProgress] == -1) soundProgress = SOUNDLENGTH; //indicate that this sound has ended
    }
    return currState;
}

enum INCDIFFSTATES{Inc};
int IncDifficultyTick(int currState){
    if(currGameState == GGameplay) currDifficulty += 2;
    return currState;
}

int main(void) {
    //initialize all your inputs and ouputs
    DDRC = 0x00; PORTC = 0xFF; //analogs all inputs
    DDRB = 0b11101111; PORTB = 0b00010000; //all outputs except MISO (not used anyway)
    DDRD = 0xff; PORTD = 0x00; //pins 0-7 all outputs


    bulletArrayInit();
    ADC_init();
    // serial_init(9600);
    buzzerInit();


    // Initialize tasks here
    unsigned char i = 0;
    tasks[i].period = JoystickPeriod;
    tasks[i].state = JSCENTER;
    tasks[i].elapsedTime = JoystickPeriod;
    tasks[i].TickFct = &JoystickTick;
    i++;
    tasks[i].period = ButtonPeriod;
    tasks[i].state = BUNPRESSED;
    tasks[i].elapsedTime = ButtonPeriod;
    tasks[i].TickFct = &PauseButtonTick;
    i++;
    tasks[i].period = ButtonPeriod;
    tasks[i].state = BUNPRESSED;
    tasks[i].elapsedTime = ButtonPeriod;
    tasks[i].TickFct = &RestartButtonTick;
    i++;
    tasks[i].period = ButtonPeriod;
    tasks[i].state = BUNPRESSED;
    tasks[i].elapsedTime = ButtonPeriod;
    tasks[i].TickFct = &ShootButtonTick;
    i++;
    tasks[i].period = LCDUpdatePeriod;
    tasks[i].state = LCDU;
    tasks[i].elapsedTime = LCDUpdatePeriod;
    tasks[i].TickFct = &LCDUpdateTick;
    i++;
    tasks[i].period = GameUpdatePeriod;
    tasks[i].state = GStart;
    tasks[i].elapsedTime = GameUpdatePeriod;
    tasks[i].TickFct = &GameUpdateTick;
    i++;
    tasks[i].period = EnemyAdvancePeriod;
    tasks[i].state = EA;
    tasks[i].elapsedTime = EnemyAdvancePeriod;
    tasks[i].TickFct = &EnemyAdvanceTick;
    i++;
    tasks[i].period = BulletAdvancePeriod;
    tasks[i].state = BA;
    tasks[i].elapsedTime = BulletAdvancePeriod;
    tasks[i].TickFct = &BulletAdvanceTick;
    i++;
    tasks[i].period = BuzzerPeriod;
    tasks[i].state = Buzz;
    tasks[i].elapsedTime = BuzzerPeriod;
    tasks[i].TickFct = &BuzzerTick;
    i++;
    tasks[i].period = IncDifficultyPeriod;
    tasks[i].state = Inc;
    tasks[i].elapsedTime = IncDifficultyPeriod;
    tasks[i].TickFct = &IncDifficultyTick;
    i++;

    //LDC: CS = PORTD 7  - set low to "choose" this device
    //reset: PORTD 5 - set low, then high to reset
    //A0: PORTD 6 - 0 means data, 1 means command
    //MOSI and SCLK are hooked up to PORTB 3 and 5, SS has no connection but must b output (2)

    PORTD= SetBit(PORTD, 7, 0);
    _delay_ms(1);
    //hardware reset
    PORTD = SetBit(PORTD, 5, 1);
    _delay_ms(200);
    PORTD = SetBit(PORTD, 5, 0);
    _delay_ms(200);
    PORTD = SetBit(PORTD, 5, 1);
    _delay_ms(200);

    SPI_INIT();
    sendSPICommand(0x01); //sw reset
    _delay_ms(1);
    sendSPICommand(0x11); //slpout
    sendSPICommand(0x3A); //COLMOD
    sendSPIData(0b00000110); //18 bit color
    sendSPICommand(0x29); //display on / DISPON

    clearLCD();
    currGameState = GIntro; //start on title screen
    // setHighScore(0);
    highScore = getHighScore();




    setBuzzer(1);
    inputs.push(NOINPUT); //used to trigger the gameupdate to process

    TimerSet(GCD_PERIOD);
    TimerOn();


    while (1) {

    }

    return 0;
}
