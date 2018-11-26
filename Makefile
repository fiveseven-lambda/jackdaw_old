TARGET = jackdaw.c read.c

jackdaw: $(TARGET)
	gcc $(TARGET) -ojackdaw -Wall -lm
