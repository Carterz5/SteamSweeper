#include "steam.h"


bool FindLibraries(char libraries[MAX_LIBRARIES][256], int* library_count, char steamfolder[256]){
    FILE* file = fopen(steamfolder, "r");
  
    if (!file) {
      printf("error opening libraryfolders.vdf\n");
      return false;
    }
  
    char line[512];
    *library_count = 0;
  
    while (fgets(line, sizeof(line), file)) {
      char *path_pos = strstr(line, "\"path\""); // Find the line with "path"
      if (path_pos) {
          char *start = strchr(path_pos, '"');  // First quote
          if (start) start = strchr(start + 1, '"'); // Second quote
          if (start) start = strchr(start + 1, '"'); // Third quote (actual path starts after this)
  
          if (start) {
              start += 1;  // Move past the quote
              char *end = strchr(start, '"'); // Find the closing quote
  
              if (end) {
                  *end = '\0'; // Terminate string at the closing quote
  
                  strncpy(libraries[*library_count], start, 255);
                  libraries[*library_count][255] = '\0'; // Ensure null-termination
  
                  (*library_count)++;
                  if (*library_count >= MAX_LIBRARIES) {
                      break;
                  }
              }
          }
      }
  }
  
    fclose(file);
  
    return true;
  
  
  
  }
  
  
  bool FindSteamInstall(char filepath[], size_t size){
  
  
  
    const char* subKey = "SOFTWARE\\WOW6432Node\\Valve\\Steam\\NSIS";
    const char* valueName = "Path";
    DWORD dataSize = (DWORD)size;
    DWORD dataType;
    LONG result = RegGetValue(HKEY_LOCAL_MACHINE, subKey, valueName, RRF_RT_REG_SZ, &dataType, filepath, &dataSize);
  
  
    if (result == ERROR_SUCCESS){
      printf("Registry Value: %s\n", filepath);
    } else {
      printf("Failed to read registry. Error code: %ld\n", result);
      return false;
    }
    if (strlen(filepath) + strlen("\\steamapps\\libraryfolders.vdf") < size){
      strcat(filepath, "\\steamapps\\libraryfolders.vdf");
    } else {
      printf("not enough space to append the path\n");
      return false;
    }
  
  
    printf("After strcat: %s\n", filepath);
  
    return true;
  
  }
  
  void FindGames(SteamData* SteamData) {
    // First pass to count the number of games
    SteamData->GameCount = 0;
  
    for (int i = 0; i < SteamData->LibraryCount; i++) {
        WIN32_FIND_DATAA findData;
        HANDLE hFind;
  
        // Construct the search pattern
        char searchPattern[MAX_PATH];
        snprintf(searchPattern, MAX_PATH, "%s\\steamapps\\appmanifest_*.acf", SteamData->LibraryLocations[i]);
        printf("search pattern is %s\n", searchPattern);
  
        // Start the file search
        hFind = FindFirstFileA(searchPattern, &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            printf("No matching files found or error occurred.\n");
            continue;  // Use continue instead of return so it checks other libraries
        }
  
        do {
            printf("Found: %s\n", findData.cFileName);
            SteamData->GameCount++;
        } while (FindNextFileA(hFind, &findData)); // Continue searching
  
        FindClose(hFind);
    }
  
    printf("Total games found: %d\n", SteamData->GameCount);
  
    // Ensure we have games before allocating memory
    if (SteamData->GameCount == 0) {
        printf("No games found, skipping allocation.\n");
        return;
    }
  
    SteamData->Games = malloc(SteamData->GameCount * sizeof(SteamGame));
    if (!SteamData->Games) {
        printf("Memory allocation failed for Games.\n");
        return;
    }
    for (int i = 0; i < SteamData->GameCount; i++){
      SteamData->Games[i].AppID = 0;
      SteamData->Games[i].LastPlayed = 0; 
      SteamData->Games[i].Selected = FALSE; 
      SteamData->Games[i].SizeOnDisk = 0; 
  
    }
    
  
    // Second pass to populate game data
    unsigned int gamecounter = 0;
  
    for (int i = 0; i < SteamData->LibraryCount; i++) {
        WIN32_FIND_DATAA findData;
        HANDLE hFind;
  
        // Construct the search pattern again
        char searchPattern[MAX_PATH];
        snprintf(searchPattern, MAX_PATH, "%s\\steamapps\\appmanifest_*.acf", SteamData->LibraryLocations[i]);
  
        // Start the file search
        hFind = FindFirstFileA(searchPattern, &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            printf("No matching files found or error occurred.\n");
            continue;
        }
  
        do {
            printf("Found: %s\n", findData.cFileName);
            char currentgame[MAX_PATH] = {'\0'};
            snprintf(currentgame, MAX_PATH, "%s\\steamapps\\%s", SteamData->LibraryLocations[i], findData.cFileName);
            printf("Current game file: %s\n", currentgame);
  
            FILE* file = fopen(currentgame, "r");
            if (!file) {
                printf("Error opening file: %s\n", currentgame);
                continue;
            }
  
            char line[512] = {'\0'};
            while (fgets(line, sizeof(line), file)) {
                if (sscanf(line, "%*[\t ]\"name\" \"%[^\"]\"", SteamData->Games[gamecounter].Title) == 1) {
                    printf("\t\tFound name: %s\n", SteamData->Games[gamecounter].Title);
                    
                }
                if (sscanf(line, "%*[\t ]\"installdir\" \"%[^\"]\"", SteamData->Games[gamecounter].Location) == 1){
                  char placeholder[MAX_BUFFER] = {'\0'};
                  snprintf(placeholder, MAX_PATH, "%s\\steamapps\\common\\%s", SteamData->LibraryLocations[i], SteamData->Games[gamecounter].Location);
                  strcpy(SteamData->Games[gamecounter].Location, placeholder);
                  printf("\t\tfound path: %s\n", placeholder);
                }
                if (sscanf(line, "%*[\t ]\"appid\" \"%[^\"]\"", SteamData->Games[gamecounter].AppIDstr) == 1){
                    printf("\t\tfound appid: %s\n", SteamData->Games[gamecounter].AppIDstr);
                }
                if (sscanf(line, "%*[\t ]\"SizeOnDisk\" \"%[^\"]\"", SteamData->Games[gamecounter].SizeOnDiskstr) == 1){
                    printf("\t\tfound size_on_disk: %s\n", SteamData->Games[gamecounter].SizeOnDiskstr);
                }
                if (sscanf(line, "%*[\t ]\"LastPlayed\" \"%[^\"]\"", SteamData->Games[gamecounter].LastPlayedstr) == 1){
                    printf("\t\tfound last_played: %s\n", SteamData->Games[gamecounter].LastPlayedstr);
                }
            }
            fclose(file);
            char* end;
            SteamData->Games[gamecounter].AppID = atoi(SteamData->Games[gamecounter].AppIDstr);
            SteamData->Games[gamecounter].SizeOnDisk = strtoull(SteamData->Games[gamecounter].SizeOnDiskstr, &end, 10);
            SteamData->Games[gamecounter].LastPlayed = strtoull(SteamData->Games[gamecounter].LastPlayedstr, &end, 10);
            printf("last played is %llu\n", SteamData->Games[gamecounter].LastPlayed);
            printf("size on disk is %llu\n", SteamData->Games[gamecounter].SizeOnDisk);
            if (SteamData->Games[gamecounter].LastPlayed == 0){
                strcpy(SteamData->Games[gamecounter].LastPlayedstr, "No data\0");
            } else {
                struct tm* WorkingDate;
                WorkingDate = localtime(&SteamData->Games[gamecounter].LastPlayed);
                strftime(SteamData->Games[gamecounter].LastPlayedstr, sizeof(SteamData->Games[gamecounter].LastPlayedstr), "%m/%d/%Y", WorkingDate);
            }
            



            byte_to_human(SteamData->Games[gamecounter].SizeOnDisk, SteamData->Games[gamecounter].SizeOnDiskstr, sizeof(SteamData->Games[gamecounter].SizeOnDiskstr));

            gamecounter++;  // Increment after successfully processing a file
        } while (FindNextFileA(hFind, &findData));
  
        FindClose(hFind);
    }
    
    printf("Final game counter: %d\n", gamecounter);
  }



  void byte_to_human(uint64_t size_in_bytes, char *output, size_t output_size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit_index = 0;
    double size = (double)size_in_bytes;

    // Convert to the largest appropriate unit
    while (size >= 1024.0 && unit_index < 5) {
        size /= 1024.0;
        unit_index++;
    }

    // Format the result with a whole number if possible
    if (size - (int)size == 0) {
        snprintf(output, output_size, "%d %s", (int)size, units[unit_index]);
    } else {
        snprintf(output, output_size, "%.1f %s", size, units[unit_index]);
    }
}


// Comparator function for qsort_s
int compareGames(void* context, const void* a, const void* b) {
    SteamGame* gameA = (SteamGame*)a;
    SteamGame* gameB = (SteamGame*)b;
    SortParams* params = (SortParams*)context;

    int result = 0;


    switch (params->column) {
        case 0: // Sort by AppID (int comparison is fine)
            result = (gameA->AppID > gameB->AppID) - (gameA->AppID < gameB->AppID);
            break;

        case 1: // Sort by Name (string comparison)
            result = strcmp(gameA->Title, gameB->Title);
            break;

        case 2: // Sort by Size (uint64_t comparison, avoid subtraction overflow)
            result = (gameA->SizeOnDisk > gameB->SizeOnDisk) - (gameA->SizeOnDisk < gameB->SizeOnDisk);
            break;

        case 3: // Sort by Last Played (uint64_t comparison)
            
            result = (gameA->LastPlayed > gameB->LastPlayed) - (gameA->LastPlayed < gameB->LastPlayed);
            break;

        case 4: // Sort by Path (string comparison)
            result = strcmp(gameA->Location, gameB->Location);
            break;
    }

    return params->ascending ? result : -result;
}

// Sorting function using qsort_s
void sortSteamData(int column, bool ascending, SteamData* steamData) {
    SortParams params = { column, ascending };
    qsort_s(steamData->Games, steamData->GameCount, sizeof(SteamGame), compareGames, &params);
}