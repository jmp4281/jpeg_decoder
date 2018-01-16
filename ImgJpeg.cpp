#include "stdafx.h"
#include "ImgJpeg.h"
#include <stdlib.h>

ImgJpeg::ImgJpeg(uint8_t *buffer)
{
	_buffer = buffer;
	ProcessFile();
}


ImgJpeg::~ImgJpeg()
{
	free(_buffer);
}

void ImgJpeg::ProcessSOI(uint8_t *_dqtBuffer)
{

}

void ImgJpeg::ProcessDQT(uint8_t *_dqtBuffer)
{
	_dqtTable.length_of_segment = _dqtBuffer[3];
	_dqtTable.precision = _dqtBuffer[4] >> 4;
	_dqtTable.table_id = _dqtBuffer[4] << 4;
	//Esta primera tabla es la de Luminancia
	_dqtTable.TABLES.push_back(CopyZigZag(&_dqtBuffer[5]));
	//Esta otra corresponde a la crominancia
	_dqtTable.TABLES.push_back(CopyZigZag(&_dqtBuffer[69]));
 }

Table* ImgJpeg::CopyZigZag(uint8_t *dqt)
{
	Table* elements = new Table();
	memset(&elements->rawData, 0, 64);
	int r = 0;
	int z = 0;
	int y = 0;
	uint8_t startX = 1;
	int debug_num = 37;

	do
	{
		y = 0;
		for (int x = z; x >= 0; x--)
		{
			uint8_t val = *dqt++;
			if (z % 2 == 0)
				elements->rawData[x][y] = val;
			else
				elements->rawData[y][x] = val;
			y++;
		}
		z++;
	} while (z < 8);

	z = 7;
	do
	{
		y = 7;
		for (int x = startX; x <= 7; x++)
		{
			uint8_t val = *dqt++;
			if (z % 2 != 0)
				elements->rawData[y][x] = val;
			else
				elements->rawData[x][y] = val;
			y--;
		}
		startX++;
		z--;
	} while (z >= 0);

	return elements;
}

void ImgJpeg::ProcessFile()
{
	for (int i = 0; ; i++)
	{
		if (_buffer[i] == 0xFF)
		{
			switch (_buffer[i + 1])
			{
			case  MARKER_SOI:
				printf("Found marker: 0x%02x   SOI\n", _buffer[i + 1]);
				ProcessSOI(&_buffer[i]);
				continue;
				break;
			case  MARKER_APP0:
				printf("Found marker: 0x%02x  APP0\n", _buffer[i + 1]);
				break;
			
			case  MARKER_APP1:
			case  MARKER_APPF:
				printf("Found marker: 0x%02x  APPn\n", _buffer[i + 1]);
				break;
			
			case  MARKER_DQT:
				printf("Found marker: 0x%02x   DQT\n", _buffer[i + 1]);
				ProcessDQT(&_buffer[i]);
				break;

			case  MARKER_SOF0:
				printf("Found marker: 0x%02x  SOF0\n", _buffer[i + 1]);
				ProcessSOF0(&_buffer[i]);
				break;
			case  MARKER_DHT:
				printf("Found marker: 0x%02x   DH\n", _buffer[i + 1]);
				ProcessDHT(&_buffer[i]);
				break;
			case  MARKER_SOS:
				printf("Found marker: 0x%02x   SOS\n", _buffer[i + 1]);
				ProcessSOS(&_buffer[i]);
				break;
			case  MARKER_EOI:
				printf("Found marker: 0x%02x   EOI\n", _buffer[i + 1]);
				PrintDQTTable();
				PrintSOF0();
				PrintDHT();
				PrintSOS();
				return;
				break;

			default: continue; break;
			}
		}
	}
}

void ImgJpeg::ProcessDHT(uint8_t *dhtBuffer)
{
	DHTTABLE _dhtTable;
	_dhtTable.length      = dhtBuffer[3];
	_dhtTable.information = dhtBuffer[4];
	memcpy(_dhtTable.numSymbols, &dhtBuffer[5], 16);
	uint8_t *buff = &dhtBuffer[21];
	_dhtTable.numElements = 0;

	for(int x = 0; x < 16; x++)
	{
		std::vector<uint8_t> vals;
		for (int i = 0; i < _dhtTable.numSymbols[x]; i++)
		{
			_dhtTable.numElements++;
			uint8_t val = *buff++;
			vals.push_back(val);
		}
		_dhtTable.symbols.push_back(vals);
	}
	
	_dhtTables.push_back(_dhtTable);
}

void ImgJpeg::ProcessSOF0(uint8_t *sof0Buffer)
{
	_sof0Table.length = sof0Buffer[3];
	_sof0Table.bpp = sof0Buffer[4];
	_sof0Table.height = sof0Buffer[5] << 8 | sof0Buffer[6];
	_sof0Table.width = sof0Buffer[7] << 8 | sof0Buffer[8];
	_sof0Table.component = sof0Buffer[9];

	for (int i = 0; i < 3; i++)
	{
		_sof0Table.components[i].id = sof0Buffer[10 + i * 3];
		_sof0Table.components[i].samplingFactor = sof0Buffer[11 + i * 3];
		_sof0Table.components[i].quantizationTableNumer = sof0Buffer[12 + i * 3];
	}
}

void ImgJpeg::ProcessSOS(uint8_t *sosBuffer)
{
	_sosTable.length = sosBuffer[3];
	_sosTable.numComponents = sosBuffer[4];
	_sosTable.comp1ID = sosBuffer[5];

	//bit 0..3 : AC table(0..3)
    //bit 4..7 : DC table(0..3)

	_sosTable.comp1HT_DC = (sosBuffer[6] & 0xf0) >> 4;
	_sosTable.comp1HT_AC = (sosBuffer[6] & 0x0f);
	_sosTable.comp2ID = sosBuffer[7];
	_sosTable.comp2HT_DC = (sosBuffer[8] & 0xf0) >> 4;
	_sosTable.comp2HT_AC = (sosBuffer[8] & 0x0f);
	_sosTable.comp3ID = sosBuffer[9];
	_sosTable.comp3HT_DC = (sosBuffer[10] & 0xf0) >> 4;
	_sosTable.comp3HT_AC = (sosBuffer[10] & 0x0f);
	//3 bytes a ignorar
}

void ImgJpeg::PrintSOF0()
{
	printf(" ********************* SOF0 Table *****************\n");
	printf("Length:    %d\n", _sof0Table.length);
	printf("Bpp:       %d\n", _sof0Table.bpp);
	printf("Height:    %d\n", _sof0Table.height);
	printf("Width:     %d\n", _sof0Table.width);
	printf("Component: %d\n", _sof0Table.component);
	for (int i = 0; i < 3; i++) 
	{
		printf("Component[%d] ID:           0x%02x ",  i,  _sof0Table.components[i].id);
		printf("Component[%d] SampleFactor: 0x%02x ",  i,  _sof0Table.components[i].samplingFactor);
		printf("Component[%d] Quant.TableN: 0x%02x\n", i,  _sof0Table.components[i].quantizationTableNumer);
	}
}

void ImgJpeg::PrintDHT()
{
	for(int i = 0 ; i < _dhtTables.size() ; i++)
	{
		printf(" ********************* DHT Table %d *****************\n", i);
		printf("Length:      %d\n", _dhtTables[i].length);
		printf("Information: %d\n", _dhtTables[i].information);
	
		for (int j = 0; j < 16; j++)
		{
			printf("DHT Elements[%d] ID:  %d", j, _dhtTables[i].numSymbols[j]);
			for (int x = 0; x < _dhtTables[i].symbols[j].size(); x++)
			{
				printf(" %02x", _dhtTables[i].symbols[j][x]);
			}

			printf("\n");
		}
		printf("numElements: %d\n", _dhtTables[i].numElements);
	}
}

void ImgJpeg::PrintHeader()
{
	printf("SOI: 0x%02x 0x%02x\n", _header.SOI[0], _header.SOI[1]);

	//	FF E0
	printf("APP0: 0x%02x 0x%02x\n", _header.APP0[0], _header.APP0[1]);
	//Length of segment excluding APP0 marker
	printf("APP0 Length: 0x%02x 0x%02x\n", _header.Length[0], _header.Length[1]);

	//4A 46 49 46 00 = "JFIF" in ASCII
	printf("Identifier: %s\n", _header.Identifier);

	//First byte for major version, second byte for minor version
	printf("Version: 0x%02x 0x%02x\n", _header.Version[0], _header.Version[1]);

	// Density units	1	Units for the following pixel density fields
	//00 : No units; width:height pixel aspect ratio = Ydensity : Xdensity
	//01 : Pixels per inch(2.54 cm)
	//02 : Pixels per centimeter

	printf("Units: 0x%02x\n", _header.Units);

	//First byte for major version, second byte for minor version(01 02 for 1.02)
	printf("Xdensity : 0x%02x 0x%02x\n", _header.Xdensity[0], _header.Xdensity[1]);
	printf("Ydensity: 0x%02x 0x%02x\n", _header.Ydensity[0], _header.Ydensity[1]);

	//Si existe thumbnail tamaño en X y en Y
	printf("XThumbnail: %d\n", _header.XThumbnail);
	printf("YThumbnail: %d\n", _header.YThumbnail);


	//Si la version es menor que la 1.02 no habrá app0 marker segment
	//El app0 marker segment permite embeber un thumbnail en difernetes fordqtTable.elementsos, jpeg, paletizado, rgb
}

void ImgJpeg::PrintDQTTable()
{
	printf(" ********************* DQT Table *****************\n");
	
	for(int i = 0 ; i < _dqtTable.TABLES.size(); i++)
	{
		Table * elements = _dqtTable.TABLES[i];

		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				//Imprimo en decimal para comparar con JPEG snoop
				printf("%02d ", elements->rawData[x][y]);

				if (y == 7)
					printf("\n");
			}
		}
		printf(" *************************************************\n");
	}
	return;
}

void ImgJpeg::PrintSOS()
{
	printf(" ********************* SOS Table *****************\n");
	printf("Length:           %d\n", _sosTable.length);
	printf("numComponents:    %d\n", _sosTable.numComponents);
	
	printf("Component 1 ID:       %d\n", _sosTable.comp1ID);
	printf("Component 1 HT DC:    %d\n", _sosTable.comp1HT_DC);
	printf("Component 1 HT AC:    %d\n", _sosTable.comp1HT_AC);

	printf("Component 2 ID:       %d\n", _sosTable.comp2ID);
	printf("Component 2 HT DC:    %d\n", _sosTable.comp2HT_DC);
	printf("Component 2 HT AC:    %d\n", _sosTable.comp2HT_AC);

	printf("Component 3 ID:       %d\n", _sosTable.comp3ID);
	printf("Component 3 HT DC:    %d\n", _sosTable.comp3HT_DC);
	printf("Component 3 HT AC:    %d\n", _sosTable.comp3HT_AC);
}
