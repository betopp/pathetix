//pic8259.h
//Code for controlling 8259 interrupt controllers on PC
//Bryan E. Topp <betopp@betopp.com> 2018

#pragma once
#include <stdint.h>

//Initializes master and slave PICs
void pic8259_init();

//Call at end of interrupt
void pic8259_pre_iret(int irq);
