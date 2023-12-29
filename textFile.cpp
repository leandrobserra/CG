#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;


static int readShader(char *fileName, char *shaderText, int size) // read a shader to shaderText.
{
	FILE *fh;
	char name[100];
	int count;

	fh = fopen(fileName, "r");
	if (!fh)
		return -1;
	fseek(fh, 0, SEEK_SET);
	count = (int)fread(shaderText, 1, size, fh);
	shaderText[count] = '\0';
	if (ferror(fh))
		count = 0;
	fclose(fh);
	return count;
}


int file_size(char* filename) // Get the size of a shader file.
{
	FILE *fp = fopen(filename, "r");
	if (!fp) return -1;
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}


char *textFileRead(char *fn) // Read and return the content of a file.
{
	FILE *fp;
	char *content = NULL;
	char * buffer = NULL;
	int count = 0;

	if (fn != NULL)
	{
		int size = file_size(fn);
		char * shaderSource = new char[size];
		readShader(fn, shaderSource, size);
		return shaderSource;

	}

	return buffer;
}

