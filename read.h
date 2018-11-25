struct Note{
	unsigned int instrument;
	double velocity;
	double frequency;
	double start, end;
};

void *fileopen(char *);
char readin(void *);
