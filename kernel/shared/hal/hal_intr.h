//hal_intr.h
//HAL interface - interrupt handling
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_INTR_H
#define HAL_INTR_H

#include <stdbool.h>

//Enables or disables interrupt handling. Returns whether interrupts were previously enabled.
bool hal_intr_ei(bool enable);

//Enables interrupts and halts, atomically.
void hal_intr_halt(void);

//Wakes up any cores that are halted using an interprocessor interrupt.
void hal_intr_wake(void);

//The only external interrupt system supported is MSI.
//The HAL reports a range of possible interrupt numbers to the kernel.
//The kernel then programs an MSI device to trigger one such number.
//The HAL receives the interrupt when it occurs, and calls kentry_msi.

//Returns the first interrupt number available to MSI devices.
int hal_intr_msivec_first(void);

//Returns the number of interrupt numbers available to MSI devices.
int hal_intr_msivec_count(void);



#endif //HAL_INTR_H
