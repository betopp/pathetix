#Makefile
#Makefile part for Pathetix system program
#Bryan E. Topp <betopp@betopp.com> 2021

#Definitions for this program
PROGRAM_BINNAME = oksh
PROGRAM_SRCFILES = $(shell find src/ -name *.c)
PROGRAM_CFLAGS = -DVI -DEMACS -Wno-unused-parameter
PROGRAM_LIBS = 

#Include common program Makefile for the target platform
ifndef PROGRAM_PLATFORM
$(error PROGRAM_PLATFORM not defined)
endif

include $(PROGRAM_PLATFORM)

