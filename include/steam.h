#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>
#include <time.h>



#define MAX_LIBRARIES 10
#define MAX_BUFFER 256

typedef struct SteamGame{
  char Title[MAX_BUFFER];
  char Location[MAX_BUFFER];
  char AppIDstr[MAX_BUFFER];
  char SizeOnDiskstr[MAX_BUFFER];
  char LastPlayedstr[MAX_BUFFER];
  unsigned int AppID;
  uint64_t SizeOnDisk;
  uint64_t LastPlayed;
  bool Selected;
} SteamGame;

typedef struct SteamData{
  char SteamLocation[256];
  char LibraryLocations[MAX_LIBRARIES][256];
  unsigned int LibraryCount;
  SteamGame* Games;
  unsigned int GameCount;

} SteamData;


typedef struct {
    int column;
    bool ascending;
} SortParams;

bool FindLibraries(char libraries[MAX_LIBRARIES][256], int* library_count, char steamfolder[256]);
bool FindSteamInstall(char filepath[], size_t size);
void FindGames(SteamData* SteamData);
void byte_to_human(uint64_t size_in_bytes, char *output, size_t output_size);
int compareGames(void* context, const void* a, const void* b);
void sortSteamData(int column, bool ascending, SteamData* steamData);