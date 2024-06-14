This was my final project for my embedded systems class at UCR in spring 2024. Below is most of my project report.

# Project Overview

Final Demo Video: https://youtu.be/Xe39ehWLJDo

My project is a video game similar in gameplay to the arcade game Space Invaders. The game is displayed on a small screen and controlled by 3 buttons and a joystick. 
When the game boots up initially, it opens on a title screen and indicates the button for starting a new game. When the button is pressed it begins the game. 
The gameplay consists of moving from side to side on the bottom of the screen and shooting upward to destroy the enemies that descend from the top of the screen. Bullets shot by the player move upward and can destroy one enemy. Each enemy destroyed gives the player one point, and if an enemy reaches the player or the bottom of the screen, the game ends. The enemies start sparse, and become more numerous as time proceeds. During gameplay, the player can pause the game, which brings up the pause menu. From this menu they can see options to resume the game, restart the game (start a new game with enemies, player, and score reset), their current score, and the all-time high-score (persists when the game is turned off). When the player dies, they can see their finishing score, the highscore (which turns yellow to indicate a new high score if applicable), and the option to restart. 
The system plays sound effects for various events such as the intro, shooting, pressing a button, dying, etc.

<img height="300" align="left" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/8b76b663-22bf-430a-b818-9ec3f0ce37dc">
<img height="300" align="left" alt="Screen Shot 2024-06-14 at 10 43 55 AM" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/55fb8af8-d6fe-4140-8292-d7ef4584ca6c">
<img height="300" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/d9b25eb2-7ec3-4fd3-9b6b-087f422063e6">


# I/O

Inputs: The user uses the joystick to move left and right while in gameplay. There are also three buttons, which are used to shoot, pause/unpause, and restart the game. They do nothing if the current state of the game is not appropriate (ex. while paused, the shoot button does nothing).
Outputs: The system uses the color LCD display to show visuals to the user, and the passive buzzer to produce sound effects.

# Features

__128x128 Color LCD Display:__

Demo Video: https://youtube.com/watch/6bHaW13nGtw 

I used the screen to display all of the game information to the player. Communication from the microcontroller to the display (commands and data) works using SPI communication.
In my code, I have divided up the LCD display into an 8x8 grid of 16x16 squares, since my player and enemy sprites are each 16x16 pixels. In order to draw a sprite, I select a rectangle of these squares of the appropriate size and then transmit the colors of the desired sprite. The sprite color data is contained in arrays that are incremented through and each color sent to the display. 
The way that the LCD screen receives data meant that some features I wanted to implement ended up being much more complex than they would seem. In the beginning I intended to completely draw each frame of the game, but the data communication turned out to be too slow for this to be feasible. This is why I settled on my 8x8 grid, because it made it easier to redraw only the areas that had updates each tick instead of the whole screen. I also had to give up on trying to implement animations or allowing the player character to move smoothly instead of snapping between grid squares.

__Passive Buzzer:__

Demo Video: https://youtu.be/x4YBtBqNnFo

I used the passive buzzer to produce sound effects for various events in the game. Changing the tone of the buzzer is done by changing the period of the atmega’s built-in PWM.  
My implementation works by storing all of the sounds as arrays of tones which are incremented through to produce a sound. In order to make music, I had to do some conversions: sheet music notes to Hz, and then Hz to PWM input. The different sound effects are ordered by priority so that only a higher (or equal) priority sound will cut off the currently-playing sound. 

__EEPROM:__

Demo Video: https://youtu.be/uqDETViLaB0

I used the atmega’s EEPROM to store the all-time high score that has been achieved. When the system starts, it reads the EPROM and stores its value. Since the score is an unsigned short, two 1-byte reads are required to get the two bytes. Whenever the player dies, the game checks if their score is higher than the highscore, and if it is the EEPROM is overwritten with the new value (and the player is informed via the high score display turning yellow). 

__Software:__

The current state of the game in regards to player and enemy locations is stored in an 2D array representing the 8x8 grid. This array is updated by functions which are called from other parts of the program, like the player moving or the enemies advancing. Whenever this happens, a second 2D array is set to 1 at that location to indicate that an update has occurred and the rendering task needs to redraw that square. The bullets shot by the player are stored as structs in a separate 1D array, since they are smaller than 16x16 and so their location could not be represented by the array explained above. Thus the enemies and bullets also advance at different rates. 
Inputs that control the state of the game are stored in a queue. This includes player input as well as signals from other tasks, for example the “die” input. I could not find a queue library that would work so I made my own.
Implementing sprites was tricky. I ran into several hurdles when trying to figure out how to extract the color data from the art that I made. The method that I settled on was to first color everything in a very limited color palette and export the art as a .ppm file (a simple image format with no compression). I then transcribed the colors from those files, using a separate program that I wrote, into a text file, with the formatting of a 2D array. Each color in the palette is represented by a char. I can then paste these arrays into my data header file, where my program reads through them and translates them back into the correct colors and then into data for the display. I drew all of the sprites in my game and implemented them with this method.

# Challenges / Difficulties:

A big problem that I ran into later and that was daunting at first was the lack of RAM on the atmega. A single sprite (the opening title) was four times the size of the entire RAM available. Trying to run the program in this condition produced some interesting corruption of the game.  However I managed to solve this problem and still largely accomplish what I wanted. Firstly, I decreased the amount of memory needed for a sprite. Initially I was storing each pixel as an array of three values for red, green, and blue, and so I could get virtually any color I liked. I sacrificed detail and shading and switched to the color palette method explained above, so each pixel was only 1 number. Then, I realized the atmega also had flash memory, and a lot more of it than RAM. I stored all of my sprites in flash memory, and ended up using 95% of it. 

<a href="url"><img src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/8ee9554a-be1b-48fa-9bc9-98e7b632db19" width="500" ></a> 
Corrupted gameplay


<img height="200" alt="oldQuintesson" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/20d22e5b-dfc6-4ac3-bbc0-fc08ee7a5fdc" aligh = "left">
<img height="200" alt="quintesson" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/22e7053c-3cad-4de8-8c4f-046c322fe713" align="left">
<img height="200" alt="Screen Shot 2024-06-13 at 5 19 46 PM" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/a81334f7-7276-4352-800b-c9a9a908bb1a">

Enemies before and after the switch. 


One more unexpected challenge was sticking to the SM best practices that I learned in class. The main issue was I couldn’t think of practical ways to not have multiple tasks writing to the same variables. I was not completely successful, but I think in most cases it is still not nondeterministic. For example many tasks can trigger a sound, and are therefore calling the function that writes to the sound variables. However since the sound is not blindly changed and instead only higher priority sounds can overwrite, the result will be the same at the end of a tick no matter which order the tasks were called in. 


# Task Diagrams
<img height="600" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/a85587ba-0983-4e5b-8ec7-1453fad9d1a2">

note: timerISR.h turned out to have an error, so in reality the periods are twice as fast.

__State Machine Diagrams__

PauseButton

<img height="200" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/142c5f3b-6786-4421-a898-0b42bb83a6ac">

RestartButton

<img height="200" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/8a8569e2-a857-4b5a-809e-f8e69300ed2f">

ShootButton

<img height="200" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/ff4a2fc9-655e-4f42-ab65-3f264e47d682">

Joystick

<img height="350" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/d876a739-a085-46fa-9e92-9c1a80dbd990">

GameUpdate

<img height="600" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/439307a9-a2aa-4f70-9a93-28a77d2faa04">

LCDUpdate

<img height="300" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/4a6adf16-f421-4b4c-b438-10d6dc70ae00">

EnemyAdvance

<img height="200" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/4e671d1a-ad72-4d66-8e83-748e1e543b2d">

BulletAdvance

<img height="150" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/49fcd0aa-28f1-4ccd-a3b9-a48e4a15c4e6">

Buzzer

<img height="300" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/78e44bf1-0a7e-44fd-8cbf-2bc56ca6f2dd">

Increment Difficulty

<img height="150" src="https://github.com/andersAsch0/CS120B_Embedded_Systems_Project/assets/84699083/ab8befee-b8e6-4cec-ab21-0ab55c545217">


 

# External References

TA-provided Headers:
- "spiAVR.h"
- "helper.h"
- “periph.h”
- "timerISR.h"
- "serialATmega.h" (debugging only)


Libraries:
- <stdlib.h>
- <avr/pgmspace.h>
- <avr/eeprom.h>

References:
- Previous labs (ex. for the passive buzzer)
- TA-provided slides on the LCD Display
- [LCD datasheet](https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf)
- [Website explaining how to use flash memory](https://wellys.com/posts/avr_c_progmem/)
- [avr/eeprom.h documentation](https://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html)
- [music note to frequency chart](https://mixbutton.com/mixing-articles/music-note-to-frequency-chart/)
- [ppm file reader source](https://github.com/sol-prog/Perlin_Noise) (used ppm.h and ppm.cpp only)

