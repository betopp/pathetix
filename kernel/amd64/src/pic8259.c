//pic8259.c
//Code for controlling 8259 interrupt controllers on PC
//Bryan E. Topp <betopp@betopp.com> 2021

#include "pic8259.h"
#include "amd64.h"

#define PIC_MASTER_BASE (0x20)
#define PIC_SLAVE_BASE (0xA0)
#define PIC_MASTER_COMMAND (PIC_MASTER_BASE + 0)
#define PIC_MASTER_DATA (PIC_MASTER_BASE + 1)
#define PIC_SLAVE_COMMAND (PIC_SLAVE_BASE + 0)
#define PIC_SLAVE_DATA (PIC_SLAVE_BASE + 1)

void pic8259_init()
{
	outb(PIC_MASTER_COMMAND, 0x11); //Start init
	outb(PIC_SLAVE_COMMAND, 0x11);
	
	outb(PIC_MASTER_DATA, 32); //Master offset
	outb(PIC_SLAVE_DATA, 40); //Slave offset
	
	outb(PIC_MASTER_DATA, 4); //Set master IRQ2 as a cascade input
	outb(PIC_SLAVE_DATA, 2); //Set slave as cascaded on IRQ2

	outb(PIC_MASTER_DATA, 1); //8086 mode
	outb(PIC_SLAVE_DATA, 1);
	
	//Decide which interrupts we want
	uint8_t interrupts_master = 0;
	//interrupts_master |= (1<<0); //IRQ0 = PIT
	interrupts_master |= (1<<1); //IRQ1 = keyboard
	interrupts_master |= (1<<2); //IRQ2 = chained IRQs from slave
	
	uint8_t interrupts_slave = 0;
	
	outb(PIC_MASTER_DATA, ~interrupts_master);
	outb(PIC_SLAVE_DATA, ~interrupts_slave);
	
}

/*static uint16_t pic8259_get_irr()
{
	outb(PIC_MASTER_COMMAND, 0x0A);
	outb(PIC_SLAVE_COMMAND, 0x0A);
	
	uint8_t master_irr = inb(PIC_MASTER_COMMAND);
	uint8_t slave_irr = inb(PIC_SLAVE_COMMAND);
	
	return master_irr | (slave_irr << 8);
}*/

static uint16_t pic8259_get_isr()
{
	outb(PIC_MASTER_COMMAND, 0x0B);
	outb(PIC_SLAVE_COMMAND, 0x0B);
	
	uint8_t master_isr = inb(PIC_MASTER_COMMAND);
	uint8_t slave_isr = inb(PIC_SLAVE_COMMAND);
	
	return master_isr | (slave_isr << 8);	
}

static void pic8259_eoi_master()
{
	outb(PIC_MASTER_COMMAND, 0x20);
}

static void pic8259_eoi_slave()
{
	outb(PIC_SLAVE_COMMAND, 0x20);
}

void pic8529_pre_iret(int irq)
{
	if(pic8259_get_isr())
	{
		if(irq > 7)
			pic8259_eoi_slave();
		
		pic8259_eoi_master();
	}
}
