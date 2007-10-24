# Adapted from http://www.cs.oberlin.edu/~jwalker/makeTmpl/
# general project structure
SRC:=src
OUT:=out
OBJS:=$(OUT)/obj

# whether or not to generate the run link (yes/no)
GEN_RUN:=yes

# flags for different builds 
BASE_FLAGS:=-Wall
BASE_LFLAGS:=-lglut -lGL -lGLU -lm
DEBUG_FLAGS:=-g
DEBUG_LFLAGS:=
FINAL_FLAGS:=-O3
FINAL_LFLAGS:=

#t he executable to build to
TARGET:=$(OUT)/main.exe

include Makefile-core
