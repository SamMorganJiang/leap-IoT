#include "Timer.h"

void delay(int data ms) // ?????(AT89C52 @ 11.0592MHz)
{
    int ti;
    while(ms--)
    {
        for(ti=0;ti<112;ti++){}//112
    }
}
