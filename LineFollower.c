/*
 * File:   LineFollower.c
 * Author: Utkarsh Verma
 * Created on March 20, 2018
 * Description: Hi-Tech C line follower  program for PIC16F84A.
 */
//===================================================================================
/*CONFIG*/
#pragma config FOSC = HS        // HS oscillator
#pragma config WDTE = OFF       // Watchdog Timer disabled
#pragma config PWRTE = ON       // Power-up Timer enabled
#pragma config CP = OFF         // Code protection disabled
//-------------------------------------------------------------
#include <htc.h>
#define _XTAL_FREQ 4000000      //Using a 4MHz Crystal Oscillator
//-------------------------------------------------------------
/*Pin definitions*/
#define MBtn RA0                //Mode button
#define lBtn RA1                //Left motor button
#define rBtn RA2                //Right motor button
#define MLED RA3                //Mode LED

#define lMtrB RB0               //Left motor backward
#define lMtrF RB1               //Left motor forward
#define rMtrB RB2               //Right motor backward
#define rMtrF RB3               //Right motor forward

#define fSen RB4                //Front sensor
#define rSen RB5                //Right sensor
#define mSen RB6                //Middle sensor
#define lSen RB7                //Left Sensor
//-------------------------------------------------------------
/*PWM duty cycles*/
#define normPWM 170     //PWM duty while moving forward
#define slowPWM 180     //PWM duty while turning slowly
#define fastPWM 220     //PWM duty motors while turning fast
//-------------------------------------------------------------
/*Global variables*/
int phase = 1;          //PWM duty phase
int lFwd = 0, rFwd = 0;     //Motion states
int rFirst = 1, lFirst = 1; //Check for first iteration
int lColour = 1;        //Stores line colour, 1 - White, 0 - Black
int count;          //Counts the number of PWM enabled outputs
int PWM;
//===================================================================================
/*PWM generation*/
void interrupt ISR(void)
{
    if(T0IF)
    {
        if(phase)    //HIGH phase
        {
            TMR0 = PWM;    //Time the duty phase
            if(lFwd && rFwd)    //Time the duty phase second time
                TMR0 = PWM;
            if(lFwd)lMtrF = 0;
            if(rFwd)rMtrF = 0;
            phase=0;    //Switch to LOW phase
        }
        else     //LOW phase
        {
            TMR0 = 255 - PWM;  //Time the rest of the phase
            if(lFwd && rFwd)   //Time the rest of the phase second time
                TMR0 = 255 - PWM;
            if(lFwd)lMtrF = 1;
            if(rFwd)rMtrF = 1;
            phase=1;   //Switch to HIGH phase
        }
        T0IF = 0;     //Clear the interrupt
    }
}
//===================================================================================
/*PWM implementation to pins*/
void updatePWM(void)
{
    if(rFwd && lFwd && (lFirst || rFirst)) T0IE = 1;
    if(lFwd)
    {
        if(lFirst)    //If first iteration
        {
            if(!rFwd)  //If no rMtrF PWM generation
                T0IE = 1;   //Enable timer0 interrupt
            lFirst = 0;    //The later iterations won't be the first
        }
    }
    else
    {
        if(!lFirst)   //If not first iteration
        {
            lMtrF = 0; //Set lMtrF back to LOW
            lFwd = 0;  //Disable further PWM generation for rMtrF
            if(!rFwd)  //If no rMtrF PWM generation
                T0IE = 0;   //Enable timer0 interrupt
            lFirst = 1;    //The next iteration will be the first
        }
    }
    if(rFwd)
    {
        if(rFirst)    //If first iteration
        {
            if(!lFwd)  //If no lMtrF PWM generation
                T0IE = 1;   //Enable timer0 interrupt
            rFirst = 0;    //The later iterations won't be the first
        }
    }
    else
    {
        if(!rFirst)   //If not first iteration
        {
            rMtrF = 0; //Set rMtrF back to LOW
            rFwd = 0;  //Disable further PWM generation for rMtrF
            if(!lFwd)  //If no lMtrF PWM generation
                T0IE = 0;   //Disable timer0 interrupt
            rFirst = 1;    //The next iteration will be the first
        }
    }
}
//===================================================================================
/*Sets the motor outputs to LOW*/
void flushMotor(int n)
{
    switch(n)
    {
        case 0:   //Flush backward outputs
            lMtrB = 0; rMtrB = 0;
            break;
        case 1:   //Flush forward outputs
            lMtrF = 0; rMtrF = 0;
            break;
        case 2:   //Flush all outputs
            lMtrB = 0; rMtrB = 0;
            lMtrF = 0; rMtrF = 0;
    }
}
//===================================================================================
/*Updates the motor outputs*/
void updateMotor(int n)
{
    switch(n)
    {
        case 0:   //Update left motor
            lMtrF = (((int)eeprom_read(2)) != 1)?1:0;
            lMtrB = (((int)eeprom_read(2)) != 0)?1:0;
            break;
        case 1:   //Update right motor
            rMtrF = (((int)eeprom_read(1)) != 1)?1:0;
            rMtrB = (((int)eeprom_read(1)) != 0)?1:0;
            break;
        case 2:   //Update both motors
            lMtrF = (((int)eeprom_read(2)) != 1)?1:0;
            lMtrB = (((int)eeprom_read(2)) != 0)?1:0;
            rMtrF = (((int)eeprom_read(1)) != 1)?1:0;
            rMtrB = (((int)eeprom_read(1)) != 0)?1:0;
    }
}
//===================================================================================
/*The main set of tasks*/
void main()
{
    lColour = mSen;
    /*Set pin type*/
    TRISA = 0b10111;
    TRISB = 0b11110000;

    /*PWM initialization*/
    OPTION_REG &= 0xC0;     //Set prescaler to 1:2
    GIE = 1;            //Enable global interrupts

    /*Restore last state*/
    MLED = (int)eeprom_read(0);
    if(MLED)
        updateMotor(2);
    else
    {
        PWM = normPWM;
        lFwd = 1; rFwd = 1;
        updatePWM();
    }

    /*Motor button push counters*/
    int lmbCount, rmbCount;

    while(1)    //Infinite loop
    {
        /*Default mode*/
        if(!MLED)
        {
            if(fSen == mSen)
            {
                flushMotor(0);    //No backward motion
                if(lSen == rSen)  //Move forward
                {
                    PWM = normPWM;
                    lFwd = 1;
                    rFwd = 1;
                }
                else  //Robot straying off track
                {
                    PWM = (mSen == lColour)?slowPWM:fastPWM; //Speed adjustment based on type of turn
                    if(rSen == lColour)    //Turn right
                    {
                        lFwd = 1;
                        rFwd = 0;
                    }
                    else   //Turn left
                    {
                        lFwd = 0;
                        rFwd = 1;
                    }
                }
            }
            /*Extreme turns*/
            else
            {
                lFwd = 0; rFwd = 0;
                if(rSen == lColour)   //Turn right
                {
                    lMtrF = 1; rMtrF = 0;
                    lMtrB = 0; rMtrB = 1;
                }
                else  //Turn left
                {
                    lMtrF = 0; rMtrF = 1;
                    lMtrB = 1; rMtrB = 0;
                }
            }
            updatePWM();
        }
        //-------------------------------------------------------------
        /*Mode switching*/
        if(MBtn)
        {
            /*Change mode*/
            MLED = !MLED;
            eeprom_write(0,MLED);
            if(MLED)    //If turning custom mode on
            {
                T0IE = 0;    //Stop PWM
                /*Load last states*/
                rmbCount = (int)eeprom_read(1);
                lmbCount = (int)eeprom_read(2);

                updateMotor(2);  //Change both motor states
            }
            else    //If turning custom mode off
            {
                flushMotor(2);
                PWM = normPWM;
                lFirst = 1; rFirst = 1;  //For updatePWM()
                lFwd = 1; rFwd = 1;
            }
            __delay_ms(500);    //Delay for button press
        }
        //-------------------------------------------------------------
        /*Motor button actions*/
        if(MLED)
        {
            if(lBtn)    //Left button is pressed
            {
                lmbCount++;
                lmbCount%=3;
                eeprom_write(2,lmbCount);   //Save last states
                updateMotor(0);     //Change left motor state
                __delay_ms(500);    //Delay for button press
            }
            if(rBtn)    //Right button is pressed
            {
                rmbCount++;
                rmbCount%=3;
                eeprom_write(1,rmbCount);   //Save last state
                updateMotor(1);     //Change right motor state
                __delay_ms(500);    //Delay for button press
            }
        }
        //-------------------------------------------------------------
    }
}
//===================================================================================
