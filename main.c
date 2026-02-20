/*
---------------------
   gb-overclock-rpi
   By AtexBg @ 2026
---------------------

/!\ PS: Tested on RPi 3B+ with a GB Pocket, but
behaviour may differ depending on the devices used
*/

#include <stdio.h>
#include <termios.h> //terminal stuff
#include <unistd.h>
#include <pigpio.h> //GPIO interactions

#define GPCLK_PIN 4 //pin used for generating clock, physical pin 7
#define BASE_FREQ 4194304 //Gameboy's original CPU speed is 4194304 Hz

int RUNNING=1;

void enableRawMode(struct termios *orig) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, orig); //backup current terminal instance
    raw = *orig;
    raw.c_lflag &= ~(ICANON | ECHO); //enable raw no-echo mode 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios *orig) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

void displayHelpMenu(){
    printf("nothing here yet... sorry ");
    getchar();
}

int main() {
    struct termios orig;
    int freq = 4194304; //original Gameboy CPU frequency
    int steps = 10000; //amount of Hz to change for each arrow press
    char c;

    if(gpioInitialise() < 0){return 1;} //init gpio's

    enableRawMode(&orig);
    gpioHardwareClock(GPCLK_PIN, freq);
    //setvbuf(stdout, NULL, _IOFBF, 4096); //disable stdout buffer auto-flushing

    while(RUNNING){
        //i know i could've just used ncurses, but i didn't wanted to bother me with it
    
        printf("\x1b[H\x1b[J"); //clear screen

        //this long line is the whole interface, good luck to maintain that lmao
        printf("\x1b[36mgb-overclock-pi by AtexBg @ 2026\n\x1b[36m-------------------------------------------\x1b[0m\n\x1b[36m|            Press \x1b[33mL\x1b[36m for help             |\x1b[0m\n\x1b[36m-------------------------------------------\x1b[0m\n\x1b[36m|         CURRENT CPU FREQUENCY :         |\x1b[0m\n\x1b[36m|         \x1b[32m%4.1f\x1b[32m MHz / %8d\x1b[36m Hz          |\x1b[0m\n\x1b[36m-------------------------------------------\x1b[0m\n\x1b[36m|        \x1b[32m%6.2f%%\x1b[36m of base frequency        |\x1b[0m\n\x1b[36m-------------------------------------------\x1b[0m\n\x1b[36m|            STEP : \x1b[32m%5d\x1b[36m Hz              |\x1b[0m\n\x1b[36m-------------------------------------------\x1b[0m\n\x1b[36m|   \x1b[33mD\x1b[36m: Default frequency | \x1b[33mH\x1b[36m: Halt CPU    |\x1b[0m\n\x1b[36m-------------------------------------------\x1b[0m\n", ((float)freq/1e6), freq, (float)freq / BASE_FREQ * 100, steps);

        //TODO : add percentage bar here
        //printf("\x1b[36m-------------------------------------------\x1b[0m\n");
        
        if(freq == 0){printf("\x1b[31mCPU HALTED\x1b[0m");}
        
        //input handler starts here
        c = getchar(); //wait for a key to be pressed
		switch(c){
        case 'q': RUNNING=0; break;
        case 'd': freq = BASE_FREQ; break;
        case 'h': freq = 0; break; //0Hz halt/freeze the CPU
        case 'l': displayHelpMenu(); break; //help menu not implemented yet
        
        case '\033': //ANSI sequence for arrow keys
            getchar(); // skip the '['
            c = getchar();
            
	        if(c == 'A'){steps += 500; break;}
	        else if(c == 'B'){steps -= 500; break;}
            else if(c == 'C'){freq += steps; usleep(32000); break;}
            else if(c == 'D'){freq -= steps; usleep(32000); break;}
		}

        //clamp values if needed
        if(freq > 16000000){freq=16000000;} //frequencies above 16MHz are very unstable and make the console crash after the bootROM is executed
        if(freq < 0){freq = 0;}
        if(steps > 95500){steps=95500;} //steps above 100K will cause display issues and are more likely to crash the console while changing frequency
        if(steps < 500){steps = 500;}

        gpioHardwareClock(GPCLK_PIN, freq);
    }

    printf("\x1b[H\x1b[J"); //clear screen
    gpioHardwareClock(GPCLK_PIN, 0); //reset freq to 0
    disableRawMode(&orig);
    gpioTerminate();
    return 0;
}

