#Makefile
#Makefile part for Pathetix system program
#Bryan E. Topp <betopp@betopp.com> 2021

#Definitions for this program
PROGRAM_BINNAME = ls
PROGRAM_SRCFILES = $(shell find src/ -name *.c)
PROGRAM_CFLAGS = 
PROGRAM_LIBS = 

#Include common program Makefile for the target platform
ifndef PROGRAM_PLATFORM
$(error PROGRAM_PLATFORM not defined)
endif

include $(PROGRAM_PLATFORM)

