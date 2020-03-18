# Build the Purim project

FLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
CC = gcc
CFILES = main.c about.c css.c purimUtil.c csv.c databox.c calculate.c msgbox.c
TARGET = purim
INCLUDES = -I .
PROJ_VERSION = "\"2.1\""
MAX_PUPULATION = 2500

all: $(CFILES)
	$(CC) $(INCLUDES) -w  $(FLAGS) -export-dynamic -DPROJ_VERSION=$(PROJ_VERSION) \
	-DMAX_PUPULATION=$(MAX_PUPULATION) -o $(TARGET) $(CFILES)
