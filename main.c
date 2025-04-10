// Win32 Resource Extractor v1.0
// (c) 2025 emil.apps

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

#define MAX_RESOURCE_NAME 256

BOOL SaveResourceToFile(HMODULE hModule, LPCSTR lpType, LPCSTR lpName) {
	HRSRC hResInfo = FindResourceA(hModule, lpName, lpType);
	if (hResInfo == NULL) {
		fprintf(stderr, "Error: Could not find resource %s of type %s\n", lpName, lpType);
		return FALSE;
	}

	HGLOBAL hRes = LoadResource(hModule, hResInfo);
	if (hRes == NULL) {
		fprintf(stderr, "Error: Could not load resource %s of type %s\n", lpName, lpType);
		return FALSE;
	}

	LPVOID lpResData = LockResource(hRes);
	if (lpResData == NULL) {
		fprintf(stderr, "Error: Could not lock resource %s of type %s\n", lpName, lpType);
		FreeResource(hRes);
		return FALSE;
	}

	DWORD dwSize = SizeofResource(hModule, hResInfo);

	// Create a filename without slashes
	char szFilename[MAX_PATH];
	strncpy(szFilename, lpName, MAX_PATH - 1);
	szFilename[MAX_PATH - 1] = '\0'; // Ensure null termination
	for (char *p = szFilename; *p; ++p) {
		if (*p == '\\' || *p == '/') {
			*p = '_'; // Replace slashes with underscores
		}
	}

	FILE *pFile = fopen(szFilename, "wb");
	if (pFile == NULL) {
		fprintf(stderr, "Error: Could not open file %s for writing\n", szFilename);
		UnlockResource(hRes);
		FreeResource(hRes);
		return FALSE;
	}

	if (fwrite(lpResData, 1, dwSize, pFile) != dwSize) {
		fprintf(stderr, "Error: Could not write resource data to %s\n", szFilename);
		fclose(pFile);
		UnlockResource(hRes);
		FreeResource(hRes);
		return FALSE;
	}

	fclose(pFile);
	UnlockResource(hRes);
	FreeResource(hRes);

	printf("Resource %s of type %s saved to %s\n", lpName, lpType, szFilename);
	return TRUE;
}

BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) {
	// lpName can be either a string pointer or an integer ID
	if (IS_INTRESOURCE(lpName)) {
		char szName[MAX_RESOURCE_NAME];
		sprintf(szName, "#%d", (WORD)lpName); // Convert integer ID to string
		SaveResourceToFile(hModule, lpType, szName);
	}
	else {
		SaveResourceToFile(hModule, lpType, lpName);
	}
	return TRUE;
}

BOOL CALLBACK EnumResTypeProc(HMODULE hModule, LPSTR lpType, LONG_PTR lParam) {
	EnumResourceNamesA(hModule, lpType, EnumResNameProc, 0);
	return TRUE;
}

int main(int argc, char *argv[]) {
	printf("Win32 Resource Extractor\n");
	printf("(c) 2025 emil.apps\n\n");

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <module_path>\n", argv[0]);
		return 1;
	}

	LPCSTR modulePath = argv[1];
	HMODULE hModule = LoadLibraryExA(modulePath, NULL, LOAD_LIBRARY_AS_DATAFILE);

	if (hModule == NULL) {
		fprintf(stderr, "Error: Could not load module %s\n", modulePath);
		return 1;
	}

	printf("Extracting resources from: %s\n", modulePath);

	EnumResourceTypesA(hModule, EnumResTypeProc, 0);

	FreeLibrary(hModule);
	printf("Resource extraction complete.\n");

	return 0;
}
