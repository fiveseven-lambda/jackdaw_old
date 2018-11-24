struct Note{
	unsigned int instrument;
	double velocity;
	double height;
	double start, end;
};

void *fileopen(char *);
char readin(void *);
