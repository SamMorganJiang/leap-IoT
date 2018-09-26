#include "Timer.h"
#include "HC89F0650.h"
#include "intrins.h"

void delay(int data ms) 
{
//int ti;
	
while(ms--)
{
_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
//for(ti=0;ti<112;ti++){}//112
}
}