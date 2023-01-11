#include <iostream>

#include <windows.h>
#include <wincon.h>
#include <winspool.h>
#include <stdio.h>
#include <stdlib.h>

// Use printReceipt printerName, paperWidth, paperLength, filePath
// printReceipt "Brother TD-4420DN" 580 2200 "C:\Users\bruno\OneDrive\Desktop\Untitled-1.pdf"

int main(int argc, char** argv)
{	
	if (argc != 5) return 1;

	char* printerName = argv[1];
	int paperWidth = atoi(argv[2]);
	int paperLength = atoi(argv[3]);
	const char* filePath = argv[4];

	PRINTER_DEFAULTS pd;
	ZeroMemory(&pd, sizeof(pd));
	pd.DesiredAccess = PRINTER_ALL_ACCESS;

	HANDLE print_handle = 0;
	auto res = OpenPrinter(printerName, &print_handle, &pd);
	if (print_handle == 0) {
		printf("Failed to open printer...\n");
		return 1;
	}

	PRINTER_INFO_2* pi2 = NULL;
	DWORD dwNeeded = 0;
	auto bFlag = GetPrinter(print_handle, 2, 0, 0, &dwNeeded);
	pi2 = (PRINTER_INFO_2*)GlobalAlloc(GPTR, dwNeeded);
	auto bFlag2 = GetPrinter(print_handle, 2, (LPBYTE)pi2, dwNeeded, &dwNeeded);
	if (pi2->pDevMode == NULL)
	{
		dwNeeded = DocumentProperties(NULL, print_handle,
			printerName,
			NULL, NULL, 0);
		if (dwNeeded <= 0)
		{
			GlobalFree(pi2);
			ClosePrinter(print_handle);
			return 1;
		}

		auto pDevMode = (DEVMODE*)GlobalAlloc(GPTR, dwNeeded);
		if (pDevMode == NULL)
		{
			GlobalFree(pi2);
			ClosePrinter(print_handle);
			return 1;
		}

		auto lFlag = DocumentProperties(NULL, print_handle,
			printerName,
			pDevMode, NULL,
			DM_OUT_BUFFER);
		if (lFlag != IDOK || pDevMode == NULL)
		{
			GlobalFree(pDevMode);
			GlobalFree(pi2);
			ClosePrinter(print_handle);
			return 1;
		}
		pi2->pDevMode = pDevMode;
	}

	pi2->pDevMode->dmFields = DM_PAPERLENGTH | DM_PAPERWIDTH;
	pi2->pDevMode->dmPaperWidth = paperWidth;
	pi2->pDevMode->dmPaperLength = paperLength;

	// Do not attempt to set security descriptor...
	pi2->pSecurityDescriptor = NULL;

	// Make sure the driver-dependent part of devmode is updated...
	auto lFlag = DocumentProperties(NULL, print_handle,
		printerName,
		pi2->pDevMode, pi2->pDevMode,
		DM_IN_BUFFER | DM_OUT_BUFFER);
	if (lFlag != IDOK)
	{
		GlobalFree(pi2);
		ClosePrinter(print_handle);
		return 1;
	}

	SetPrinter(print_handle, 2, (LPBYTE)pi2, 0);

	ShellExecute(NULL, "print", filePath, NULL, NULL, 0);
	ClosePrinter(print_handle);

	return 0;
}
