#include <wchar.h>
#include "stdafx.h"
#include <Windows.h>
#include <SetupAPI.h>
#include <cstdlib>
#include "conio.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <stdlib.h>

// local global variables
static BOOL TableBuilt = FALSE;
static UINT32 CRC32Table[256];
#define CRC32_POLYNOMIAL 0xEDB88320L

UINT16 crc16_compute(const UCHAR * p_data, UINT32 size, const UINT16 * p_crc)
{
	UINT32 i;
	UINT16 crc = (p_crc == NULL) ? 0xffff : *p_crc;
	for (i = 0; i < size; i++)
	{
		crc = (unsigned char)(crc >> 8) | (crc << 8);
		crc ^= p_data[i];
		crc ^= (unsigned char)(crc & 0xff) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xff) << 4) << 1;
	}

	return crc;
}

// for CRC-32 via table lookup (faster than calc)
void BuildCRCTable(void) {
	int i;
	int j;
	unsigned long crc;
	for (i = 0; i < 256; i++)
	{
		crc = i;
		for (j = 0; j<8; j++)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
			else
				crc >>= 1;
		}
		CRC32Table[i] = crc;
	}
	TableBuilt = TRUE;
	return;
}

UINT32 crc32_compute(const UCHAR * p_data, UINT32 size) {
	UINT32 temp1;
	UINT32 temp2;
	UINT32 retval;
	ULONGLONG iBlockSize = size;
	ULONGLONG iBytesRemaining = size;
	ULONGLONG iBlockCount;
	UINT32 crc32 = 0xFFFFFFFFL;

	if (!TableBuilt)
		BuildCRCTable();
	try
	{
		for (iBlockCount = 0; iBlockCount < iBlockSize; iBlockCount++) 
		{
			temp1 = (crc32 >> 8) & 0x00FFFFFFL;
			temp2 = CRC32Table[((int)crc32 ^ p_data[iBlockCount]) & 0xFF];
			crc32 = temp1 ^ temp2; 
			//printf("CRC %d;%08x\n", iBlockCount, crc32);
		}

		retval = crc32;
	}
	catch (...)
	{
		printf("Couldn't calculate.\n");
		retval = 0;
	}

	return(retval);
}


UINT32 CalcCRC(wchar_t *filename, int CRCType) {
	int				hfw;
	errno_t			err;
	struct _stat32	fileinfo;
	UCHAR			*fw;
	unsigned int	size;
	UINT32			CRC;

	err = _tsopen_s(&hfw, filename, _O_BINARY | _O_RDONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (err) {
		wprintf(L"Can't open %s", filename);
		perror(": ");
		return false;
	}
	err = _tstat32(filename, &fileinfo);
	size = fileinfo.st_size;
	//printf("File size: %d bytes\n", size);
	fw = new UCHAR[size];
	if (!fw) {
		printf("Couldn't allocate buffer for image\n");
		_close(hfw);
		return false;
	}
	do {
		if (_read(hfw, fw, size) != size) {
			printf("Couldn't read %d bytes from file\n", size);
			_close(hfw);
			break;
		}
		_close(hfw);
		if (CRCType == 16) {
			CRC = crc16_compute(fw, size, NULL);
			printf("CRC:%04x\n", CRC);
		}
		else if (CRCType == 32) {
			CRC = crc32_compute(fw, size);
			printf("CRC:%08x\n", CRC);
		}
	} while (false);

	delete[](UCHAR *)fw;
	return CRC;
}

int _tmain(int argc, _TCHAR* argv[])
{

	if (argc < 1) {
		_tprintf(L"%s <firmware file> [32/16]", argv[1]);
		exit(-1);
	}

	if (argc < 2) {
		printf("Must specify a filename or the calculation type.\n");
		exit(-1);
	}

	try
	{
		UINT32 CRC = CalcCRC(argv[1], _ttoi(argv[2]));
		return CRC;
	}
	catch (errno_t err)
	{
		printf("Error:%s", err);
		exit(-2);
	}
}
