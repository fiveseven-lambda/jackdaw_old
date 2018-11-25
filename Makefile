TARGET = jackdaw.c read.c

jackdaw: $(TARGET)
	gcc $(TARGET) -ojackdaw -Wall -lm

lambda.wav: lambda.jkd
	./jackdaw lambda.jkd -olambda.wav
