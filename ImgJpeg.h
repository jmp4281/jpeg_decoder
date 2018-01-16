#pragma once
#include <stdint.h>
#include <vector>

//Marker Name     Marker Identifier   Description
#define MARKER_SOI  0xd8             //Start of Image
#define MARKER_APP0 0xe0             //JFIF application segment
#define MARKER_APP1 0xE1             //Other APP segments
#define MARKER_APP2 0xE2             //Other APP segments
// ..........
#define MARKER_APPF 0xEF             //Other APP segments
#define MARKER_DQT  0xdb             //Quantization Table
#define MARKER_SOF0 0xc0             //Start of Frame
#define MARKER_DHT  0xc4             //Huffman Table
#define MARKER_SOS  0xda             //Start of Scan
#define MARKER_EOI  0xd9             //End of Image


typedef struct _JFIFHeader
{
	uint8_t SOI[2];          /* 00h  Start of Image Marker     */
	uint8_t APP0[2];         /* 02h  Application Use Marker    */
	uint8_t Length[2];       /* 04h  Length of APP0 Field      */
	uint8_t Identifier[5];   /* 06h  "JFIF" (zero terminated) Id String */
	uint8_t Version[2];      /* 07h  JFIF FordqtTable.elements Revision      */
	uint8_t Units;           /* 09h  Units used for Resolution */
	uint8_t Xdensity[2];     /* 0Ah  Horizontal Resolution     */
	uint8_t Ydensity[2];     /* 0Ch  Vertical Resolution       */
	uint8_t XThumbnail;      /* 0Eh  Horizontal Pixel Count    */
	uint8_t YThumbnail;      /* 0Fh  Vertical Pixel Count      */
	uint8_t *ThumbnailData; //Tamaño =>	3 × n, siendo n = Xthumbnail x Ythumbnail
} JFIFHEAD;


/*
Marker Identifier          2 bytes        0xff, 0xdb identifies DQT
Length                     2 bytes        This gives the length of QT.
QT infordqtTable.elementsion            1 byte          bit 0..3: number of QT (0..3, otherwise error)
bit 4..7: precision of QT, 0 = 8 bit, otherwise 16 bit
Bytes                           n bytes         This gives QT values, n = 64*(precision+1)
Remarks:
·         A single DQT segment may contain multiple QTs, each with its own infordqtTable.elementsion byte.
·         For precision=1 (16 bit), the order is high-low for each of the 64 words.
*/

typedef 
class Table {
	public:
		uint8_t rawData[8][8];
		Table() {};
		~Table() {};
};

typedef struct _DQT
{
	uint8_t length_of_segment;
	uint8_t precision;
	uint8_t table_id;
	std::vector<Table*> TABLES;
}DQTTABLE;

typedef struct _SOF0Component
{
	//ID => Y=1/ Cb=2 /Cr=3/ I=5/Q=5
	uint8_t id;
	//0..3 Vertical/ 4..7 horizontal
	uint8_t samplingFactor;
	uint8_t quantizationTableNumer;

}SOF0Component;

typedef struct _SOF0
{
	//length = 8+componentes*3
	uint16_t length;
	uint8_t bpp;
	uint8_t dataPrecision;
	uint16_t height;
	uint16_t width;
	//normalmente 1 = gris scaled, 3 = color ycbcr o yiq, 4 color cmyk
	uint8_t component;
	SOF0Component components[3];
}SOF0TABLE;

typedef struct _DHT
{
	uint16_t length;
	uint8_t information;
	uint8_t numSymbols[16];
	int numElements;
	std::vector<std::vector<uint8_t> > symbols;
}DHTTABLE;


typedef struct _SOS
{
	uint8_t length;
	uint8_t numComponents;
	uint8_t comp1ID;
	uint8_t comp1HT_DC;
	uint8_t comp1HT_AC;
	uint8_t comp2ID;
	uint8_t comp2HT_DC;
	uint8_t comp2HT_AC;
	uint8_t comp3ID;
	uint8_t comp3HT_DC;
	uint8_t comp3HT_AC;

}SOSTABLE;

class ImgJpeg
{

private:
	uint8_t *_buffer;
	JFIFHEAD _header;
	DQTTABLE _dqtTable;
	SOF0TABLE _sof0Table;
	SOSTABLE _sosTable;
	std::vector<DHTTABLE> _dhtTables;
	void PrintDQTTable();
	void PrintHeader();
	void PrintSOF0();
	void PrintDHT();
	void PrintSOS();

	void ProcessSOI(uint8_t *buffer);
	void ProcessDQT(uint8_t *buffer);
	void ProcessDHT(uint8_t *buffer);
	void ProcessSOF0(uint8_t *buffer);
	void ProcessSOS(uint8_t *buffer);

	void ProcessFile();
	
	
	Table* CopyZigZag(uint8_t *_dqtBuffer);
public:
	ImgJpeg(uint8_t *buffer);
	~ImgJpeg();
};

