// jpeg_decoder.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <vector>

#define BYTE uint8_t
using namespace std;

//JFIF IS INCOMPATIBLE WITH EXIF FORMAT

/*

A cada marcador le precede un 0xFF

Marker Name     Marker Identifier   Description
SOI                   0xd8             Start of Image
APP0                  0xe0             JFIF application segment
APPn               0xe1 – 0xef         Other APP segments
DQT                   0xdb             Quantization Table
SOF0                  0xc0             Start of Frame
DHT                   0xc4             Huffman Table
SOS                   0xda             Start of Scan
EOI                   0xd9             End of Image
*/

typedef struct _JFIFHeader
{
	BYTE SOI[2];          /* 00h  Start of Image Marker     */
	BYTE APP0[2];         /* 02h  Application Use Marker    */
	BYTE Length[2];       /* 04h  Length of APP0 Field      */
	BYTE Identifier[5];   /* 06h  "JFIF" (zero terminated) Id String */
	BYTE Version[2];      /* 07h  JFIF Format Revision      */
	BYTE Units;           /* 09h  Units used for Resolution */
	BYTE Xdensity[2];     /* 0Ah  Horizontal Resolution     */
	BYTE Ydensity[2];     /* 0Ch  Vertical Resolution       */
	BYTE XThumbnail;      /* 0Eh  Horizontal Pixel Count    */
	BYTE YThumbnail;      /* 0Fh  Vertical Pixel Count      */
	BYTE *ThumbnailData; //Tamaño =>	3 × n, siendo n = Xthumbnail x Ythumbnail
} JFIFHEAD;


/*
Marker Identifier          2 bytes        0xff, 0xdb identifies DQT
Length                     2 bytes        This gives the length of QT.
QT information            1 byte          bit 0..3: number of QT (0..3, otherwise error)
bit 4..7: precision of QT, 0 = 8 bit, otherwise 16 bit
Bytes                           n bytes         This gives QT values, n = 64*(precision+1)
Remarks:
·         A single DQT segment may contain multiple QTs, each with its own information byte.
·         For precision=1 (16 bit), the order is high-low for each of the 64 words.
*/
typedef struct _DQT 
{
	uint8_t marker_identifier; //0xff 0xDB
	uint8_t length_of_segment;
    uint8_t QT_information;
	//0..3 = numero de QT, 4..7 precision de qt 0 = 8, otra cosa = 16
	uint8_t *elements; //QT values n = 64*(precision+1)
                       //Para precision = 1(16 bit) el orden es high-low
}DQTTABLE;

JFIFHEAD header;

void ReadAllBytes()
{
	FILE *fileptr;
	uint8_t *buffer;
	long filelen;

	//fileptr = fopen("C:\\Users\\100793536\\Desktop\\GetFotoAtleta.jpg", "rb");  // Open the file in binary mode
	fopen_s(&fileptr, "C:\\Users\\Julian\\Desktop\\GetFotoAtleta.jpg", "rb");  // Open the file in binary mode

	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	filelen = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file

	buffer = (uint8_t *)malloc((filelen + 1) * sizeof(uint8_t)); // Enough memory for file + \0
	fread(buffer, filelen, 1, fileptr); // Read in the entire file
	fclose(fileptr); // Close the file

	memcpy(&header, buffer, 20);

	for (int i = 0; i < filelen; i++)
	{
		if (buffer[i] == 0xFF)
		{
			switch (buffer[i + 1])
			{
			case  0xd8:
				printf("Found marker: 0x%02x   SOI\n", buffer[i + 1]);
				continue;
				break;
			case  0xE0:
				printf("Found marker: 0x%02x  APP0 ", buffer[i + 1]);
				break;
			case  0xE1:
			case  0xEF:
				printf("Found marker: 0x%02x  APPn ", buffer[i + 1]);
				break;
			case  0xdb:
				printf("Found marker: 0x%02x   DQT ", buffer[i + 1]);
				break;
			case  0xc0:
				printf("Found marker: 0x%02x  SOF0 ", buffer[i + 1]);
				break;
			case  0xc4:
				printf("Found marker: 0x%02x   DHT ", buffer[i + 1]);
				break;
			case  0xdA:
				printf("Found marker: 0x%02x   SOS ", buffer[i + 1]);
				break;
			case  0xd9:
				printf("Found marker: 0x%02x   EOI\n", buffer[i + 1]);
				break;
			
			default: continue; break;

			}

			printf("Lenght 0x%02x 0x%02x  Dec: %d %d\n", buffer[i + 2], buffer[i + 3], buffer[i + 2], buffer[i + 3]);
		}

	}

}

void PrintHeader()
{
	printf("SOI: 0x%02x 0x%02x\n", header.SOI[0], header.SOI[1]);
	
	//	FF E0
	printf("APP0: 0x%02x 0x%02x\n", header.APP0[0], header.APP0[1]);
	//Length of segment excluding APP0 marker
	printf("APP0 Length: 0x%02x 0x%02x\n", header.Length[0], header.Length[1]);
	
    //4A 46 49 46 00 = "JFIF" in ASCII
	printf("Identifier: %s\n", header.Identifier);

	//First byte for major version, second byte for minor version
	printf("Version: 0x%02x 0x%02x\n", header.Version[0], header.Version[1]);

	// Density units	1	Units for the following pixel density fields
		//00 : No units; width:height pixel aspect ratio = Ydensity : Xdensity
		//01 : Pixels per inch(2.54 cm)
		//02 : Pixels per centimeter

	printf("Units: 0x%02x\n", header.Units);
	
	//First byte for major version, second byte for minor version(01 02 for 1.02)
	printf("Xdensity : 0x%02x 0x%02x\n", header.Xdensity[0], header.Xdensity[1]);
	printf("Ydensity: 0x%02x 0x%02x\n", header.Ydensity[0], header.Ydensity[1]);

   //Si existe thumbnail tamaño en X y en Y
	printf("XThumbnail: %d\n", header.XThumbnail );
	printf("YThumbnail: %d\n", header.YThumbnail );


	//Si la version es menor que la 1.02 no habrá app0 marker segment
	//El app0 marker segment permite embeber un thumbnail en difernetes formatos, jpeg, paletizado, rgb
}
int main()
{

	ReadAllBytes();
	PrintHeader();
	return 0;
}
