// jpeg_decoder.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <vector>
#include "ImgJpeg.h"

uint8_t* ReadAllBytes(const char * inputFile)
{
	FILE *fileptr;
	uint8_t *buffer;
	long filelen;

	fopen_s(&fileptr, inputFile, "rb");  // Open the file in binary mode

	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	filelen = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file

	buffer = (uint8_t *)malloc((filelen + 1) * sizeof(uint8_t)); // Enough memory for file + \0
	fread(buffer, filelen, 1, fileptr); // Read in the entire file
	fclose(fileptr); // Close the file
	return buffer;
}
	

int main()
{
	ImgJpeg* _img =  new ImgJpeg(ReadAllBytes("C:\\Users\\Julian\\Desktop\\GetFotoAtleta.jpg"));
	delete _img;
	return 0;
}
