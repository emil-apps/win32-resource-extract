// Win32 Resource Extractor v1.0
// (c) 2025 emil.apps

#pragma comment(lib, "shlwapi.lib")
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>

#define MAX_RESOURCE_NAME 256
#define MAX_FOLDER_NAME 256

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

	// Get the folder name based on the resource type
	char szFolderName[MAX_FOLDER_NAME];
	strncpy(szFolderName, lpType, MAX_FOLDER_NAME - 1);
	szFolderName[MAX_FOLDER_NAME - 1] = '\0';

	// Create the folder if it doesn't exist
	if (!PathFileExistsA(szFolderName))
	{
		if (!CreateDirectoryA(szFolderName, NULL))
		{
			fprintf(stderr, "Error: Could not create directory %s\n", szFolderName);
			UnlockResource(hRes);
			FreeResource(hRes);
			return FALSE;
		}
		
		printf("Created directory: %s\n", szFolderName);
	}

	// Create a filename without slashes
	char szFilename[MAX_PATH];
	strncpy(szFilename, lpName, MAX_PATH - 1);
	szFilename[MAX_PATH - 1] = '\0'; // Ensure null termination
	for (char *p = szFilename; *p; ++p) {
		if (*p == '\\' || *p == '/') {
			*p = '_'; // Replace slashes with underscores
		}
	}

	// Combine folder name and filename
	char szFullFilename[MAX_PATH];
	snprintf(szFullFilename, MAX_PATH, "%s\\%s", szFolderName, szFilename);

	FILE *pFile = fopen(szFullFilename, "wb");
	if (pFile == NULL) {
		fprintf(stderr, "Error: Could not open file %s for writing\n", szFullFilename);
		UnlockResource(hRes);
		FreeResource(hRes);
		return FALSE;
	}

	if (fwrite(lpResData, 1, dwSize, pFile) != dwSize) {
		fprintf(stderr, "Error: Could not write resource data to %s\n", szFullFilename);
		fclose(pFile);
		UnlockResource(hRes);
		FreeResource(hRes);
		return FALSE;
	}

	fclose(pFile);
	UnlockResource(hRes);
	FreeResource(hRes);

	printf("%s %s saved to %s\n", lpType, lpName, szFullFilename);
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

int main(int argc, char *argv[])
{
	printf("Win32 Resource Extractor\n");
	printf("(c) 2025 emil.apps\n\n");

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <path>\n", PathFindFileName(argv[0]));
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
