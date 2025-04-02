#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"
#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "steam.h"


#ifdef IMGUI_HAS_IMSTR
#define igBegin igBegin_Str
#define igSliderFloat igSliderFloat_Str
#define igCheckbox igCheckbox_Str
#define igColorEdit3 igColorEdit3_Str
#define igButton igButton_Str
#endif

void StyleCustom();



SDL_Window *window = NULL;

int main(int argc, char* argv[])
{
  //allow printf
  setbuf(stdout, NULL);
  printf("Starting up\n");
  SDL_Log("starting up\n");
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("failed to init: %s", SDL_GetError());
    return -1;
  }

  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // and prepare OpenGL stuff
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_DisplayMode current;
  SDL_GetCurrentDisplayMode(0, &current);
  
  window = SDL_CreateWindow(
      "SteamSweep", 100, 100, 1024, 768,
      SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
      );
  if (window == NULL) {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return -1;
  }

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);  // enable vsync




  // check opengl version sdl uses
  SDL_Log("opengl version: %s", (char*)glGetString(GL_VERSION));

  // setup imgui
  igCreateContext(NULL);

  //set docking
  ImGuiIO* ioptr = igGetIO();
  ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  //ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
  // ImFontAtlas* fonts = ioptr->Fonts;
  // ImFont* font = ImFontAtlas_AddFontFromFileTTF(fonts, "./bin/Roboto-Regular.ttf", 18.0f, NULL, NULL);
  // ioptr->FontDefault = font;
  
  //igStyleColorsClassic(NULL);
  //igStyleColorsDark(NULL);
  igStyleColorsLight(NULL);
  //ImFontAtlas_AddFontDefault(io.Fonts, NULL);
  StyleCustom();

  ImVec4 clearColor;
  clearColor.x = 0.45f;
  clearColor.y = 0.55f;
  clearColor.z = 0.60f;
  clearColor.w = 1.00f;

  SteamData SteamData;
  SteamData.GameCount = 0;
  SteamData.LibraryCount = 0;
  bool pathflag = FindSteamInstall(SteamData.SteamLocation, 256);

  if(pathflag == TRUE){
    FindLibraries(SteamData.LibraryLocations, &SteamData.LibraryCount, SteamData.SteamLocation);
    printf("Found %d Steam libraries:\n", SteamData.LibraryCount);
    for (int i = 0; i < SteamData.LibraryCount; i++) {
        printf("%s\n", SteamData.LibraryLocations[i]);
    }
    FindGames(&SteamData, FALSE);
  } else {
    printf("Unable to locate steam installation. Please provide install location\n");
  }
  char UserSteamPath[MAX_PATH] = {'\0'};
  float SizeSelection = 0.0f;
  int LastPlayedSelection = 0;
  bool ShowPopup = FALSE;
  //bool waitingForInput = false;
  unsigned int currentIndex = 0;

  bool quit = false;
  while (!quit)
  {
    SDL_Event e;

    // we need to call SDL_PollEvent to let window rendered, otherwise
    // no window will be shown
    while (SDL_PollEvent(&e) != 0)
    {
      ImGui_ImplSDL2_ProcessEvent(&e);
      if (e.type == SDL_QUIT)
        quit = true;
      if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == SDL_GetWindowID(window))
        quit = true;
    }
    
    // start imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    igNewFrame();


    ImVec2 windowSize = igGetIO()->DisplaySize;

        // Set the next window position to the top-left corner
    igSetNextWindowPos((ImVec2){0, 0}, ImGuiCond_Always, (ImVec2){0, 0});

    // Set the window size to match the application window
    igSetNextWindowSize(windowSize, ImGuiCond_Always);

    ImVec2 windowspace;
    if(pathflag == FALSE){
      igBegin("Uh oh!", NULL, 0);
      igText("We were unable to locate your steam installation! Please provide your steam installation location here");
      igText("Enter Steam Path e.g. C:\\Program Files (x86)\\Steam\\");
      igInputText(" ", UserSteamPath, sizeof(UserSteamPath), ImGuiInputTextFlags_EnterReturnsTrue, 0, 0);
      if(igButton("Submit", (ImVec2){100, 100})){
        printf("User path is %s\n", UserSteamPath);
        strcpy(SteamData.SteamLocation, UserSteamPath);
        strcat(SteamData.SteamLocation, "\\steamapps\\libraryfolders.vdf");
        printf("Steam install location is %s\n", SteamData.SteamLocation);
        if(FindLibraries(SteamData.LibraryLocations, &SteamData.LibraryCount, SteamData.SteamLocation)){
          pathflag = TRUE;
          FindGames(&SteamData, FALSE);
        }
      };
      igEnd();
    } else {

      igBegin("SteamSweeper!", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
      igGetContentRegionAvail(&windowspace);

      if (igBeginTable("ItemTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | 
        ImGuiTableFlags_Sortable, (ImVec2){0, ioptr->DisplaySize.y - 175}, 0)) {
        // Create table headers
        igTableSetupColumn("AppID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 100, 0);
        igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 200, 1);
        igTableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 150, 2);
        igTableSetupColumn("Last Played", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 150, 3);
        igTableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 0, 4);
        igTableHeadersRow();


              // Retrieve sorting specs
        ImGuiTableSortSpecs* sort_specs = igTableGetSortSpecs();
        if (sort_specs && sort_specs->SpecsDirty) {
            // Get the first sorting column
            ImGuiTableColumnSortSpecs* spec = sort_specs->Specs;
            if (spec) {
                int column_index = spec->ColumnUserID; // Which column is sorted
                bool ascending = (spec->SortDirection == ImGuiSortDirection_Ascending);

                // Sort SteamData based on the selected column
                sortSteamData(column_index, ascending, &SteamData);
            }
            sort_specs->SpecsDirty = false; // Mark sorting as handled
        }

        for (int i = 0; i < SteamData.GameCount; i++) {
            if(SteamData.Games[i].AppID == 228980){
              continue;
            }
            igTableNextRow(0, 0);
            igTableSetColumnIndex(0);
            igCheckbox(SteamData.Games[i].AppIDstr, &SteamData.Games[i].Selected);
            igTableSetColumnIndex(1);
            igText(SteamData.Games[i].Title);
            igTableSetColumnIndex(2);
            igText(SteamData.Games[i].SizeOnDiskstr);
            igTableSetColumnIndex(3);
            igText(SteamData.Games[i].LastPlayedstr);
            igTableSetColumnIndex(4);
            igText(SteamData.Games[i].Location);
        }
        igEndTable();
      }
      
      if(igButton("Refresh", (ImVec2){100, 100})){
        FindGames(&SteamData, TRUE);
      }
      igSameLine(0.0f, 10.0f);
      if(igButton("Select All", (ImVec2){100, 100})){
        SelectAll(&SteamData);
      }
      igSameLine(0.0f, 10.0f);
      if(igButton("Deselect All", (ImVec2){100, 100})){
        DeSelectAll(&SteamData);
      }
      igSameLine(0.0f, 10.0f);
      if(igButton("Select Filtered Games", (ImVec2){200, 100})){
        SelectFiltered(&SteamData, SizeSelection, LastPlayedSelection);
      }
      igSameLine(0.0f, 50.0f);
      if(igButton("Uninstall Selected", (ImVec2){400, 100})){
        ShowPopup = TRUE;
      }
      
      igSliderFloat("File Size (More Than)", &SizeSelection, 0.0f, 500.0f, "%.1f GB", 0);
      igSliderInt("Last Played (More Than)", &LastPlayedSelection, 0.0, 365, "%d days", 0);


      if (ShowPopup) {
                // Calculate the position to center the popup window
        ImVec2 popupPos = (ImVec2){
            (windowSize.x) * 0.5f, // X position: (screen width - popup width) / 2
            (windowSize.y) * 0.5f  // Y position: (screen height - popup height) / 2
        };

        // Set the position of the popup to center it on the screen
        igSetNextWindowPos(popupPos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
        igSetNextWindowSize((ImVec2){300.0f, 150.0f}, ImGuiCond_Always);
        igBegin("Uninstaller", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // Check if there are games to uninstall
        if (currentIndex < SteamData.GameCount) {
            SteamGame *currentGame = &SteamData.Games[currentIndex];
            
            if (currentGame->Selected) { // Only proceed if the game is selected
                igTextWrapped("Because of the way Steam works, you will need to confirm the uninstallation of each game with a steam popup. Please click uninstall when prompted by Steam.");
                igText("Uninstalling: %s", currentGame->Title);

                if (igButton("Confirm Uninstall", (ImVec2){135, 30})) {
                    UninstallGame(currentGame); // Uninstall the game
                    currentIndex++; // Move to the next game
                    //waitingForInput = false; // Reset and wait for next user input
                }
                igSameLine(0.0f, 10.0f);
                if (igButton("Cancel", (ImVec2){135, 30})) {
                  currentIndex++; // Move to the next game
                  //waitingForInput = false; // Reset and wait for next user input
              }

            } else {
                currentIndex++; // Skip games that are not selected
            }
        } else {
            igTextWrapped("All selected games have been uninstalled or skipped.");
            
            if(igButton("Close", (ImVec2){150, 30})){
              ShowPopup = FALSE;
              currentIndex = 0;
              FindGames(&SteamData, TRUE);
            }
        }
    
        igEnd();
      }

      igEnd();


    }

    


    // render
    igRender();
    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
    fflush(stdout);
    SDL_GL_SwapWindow(window);
  }

  // clean up
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  igDestroyContext(NULL);

  SDL_GL_DeleteContext(gl_context);
  if (window != NULL)
  {
    SDL_DestroyWindow(window);
    window = NULL;
  }
  SDL_Quit();
  free(SteamData.Games);
  return 0;
}


void StyleCustom() {
  ImGuiStyle* style = igGetStyle();
  // style->GrabRounding = 2.0f;
  // style->WindowRounding = 2.0f;  // Rounded window corners
  // style->FrameRounding = 2.0f;   // Rounded buttons and frames
  //style->Alpha = 1.0f;

  ImVec4* colors = style->Colors;


  // colors[ImGuiCol_Text]                  = (ImVec4){0.00f, 0.00f, 0.00f, 1.00f};
  // colors[ImGuiCol_TextDisabled]          = (ImVec4){0.60f, 0.60f, 0.60f, 1.00f};
  // colors[ImGuiCol_WindowBg]              = (ImVec4){0.94f, 0.94f, 0.94f, 0.94f};
  // //colors[ImGuiCol_ChildWindowBg]         = (ImVec4){0.00f, 0.00f, 0.00f, 0.00f};
  // colors[ImGuiCol_PopupBg]               = (ImVec4){1.00f, 1.00f, 1.00f, 0.94f};
  // colors[ImGuiCol_Border]                = (ImVec4){0.00f, 0.00f, 0.00f, 0.39f};
  // colors[ImGuiCol_BorderShadow]          = (ImVec4){1.00f, 1.00f, 1.00f, 0.10f};
  // colors[ImGuiCol_FrameBg]               = (ImVec4){1.00f, 1.00f, 1.00f, 0.94f};
  // colors[ImGuiCol_FrameBgHovered]        = (ImVec4){0.26f, 0.59f, 0.98f, 0.40f};
  // colors[ImGuiCol_FrameBgActive]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.67f};
  // colors[ImGuiCol_TitleBg]               = (ImVec4){0.96f, 0.96f, 0.96f, 1.00f};
  // colors[ImGuiCol_TitleBgCollapsed]      = (ImVec4){1.00f, 1.00f, 1.00f, 0.51f};
  // colors[ImGuiCol_TitleBgActive]         = (ImVec4){0.82f, 0.82f, 0.82f, 1.00f};
  // colors[ImGuiCol_MenuBarBg]             = (ImVec4){0.86f, 0.86f, 0.86f, 1.00f};
  // colors[ImGuiCol_ScrollbarBg]           = (ImVec4){0.98f, 0.98f, 0.98f, 0.53f};
  // colors[ImGuiCol_ScrollbarGrab]         = (ImVec4){0.69f, 0.69f, 0.69f, 1.00f};
  // colors[ImGuiCol_ScrollbarGrabHovered]  = (ImVec4){0.59f, 0.59f, 0.59f, 1.00f};
  // colors[ImGuiCol_ScrollbarGrabActive]   = (ImVec4){0.49f, 0.49f, 0.49f, 1.00f};
  // //colors[ImGuiCol_ComboBg]               = (ImVec4){0.86f, 0.86f, 0.86f, 0.99f};
  // colors[ImGuiCol_CheckMark]             = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
  // colors[ImGuiCol_SliderGrab]            = (ImVec4){0.24f, 0.52f, 0.88f, 1.00f};
  // colors[ImGuiCol_SliderGrabActive]      = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
  // colors[ImGuiCol_Button]                = (ImVec4){0.26f, 0.59f, 0.98f, 0.40f};
  // colors[ImGuiCol_ButtonHovered]         = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
  // colors[ImGuiCol_ButtonActive]          = (ImVec4){0.06f, 0.53f, 0.98f, 1.00f};
  // colors[ImGuiCol_Header]                = (ImVec4){0.26f, 0.59f, 0.98f, 0.31f};
  // colors[ImGuiCol_HeaderHovered]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.80f};
  // colors[ImGuiCol_HeaderActive]          = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
  // //colors[ImGuiCol_Column]                = (ImVec4){0.39f, 0.39f, 0.39f, 1.00f};
  // //colors[ImGuiCol_ColumnHovered]         = (ImVec4){0.26f, 0.59f, 0.98f, 0.78f};
  // //colors[ImGuiCol_ColumnActive]          = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
  // colors[ImGuiCol_ResizeGrip]            = (ImVec4){1.00f, 1.00f, 1.00f, 0.50f};
  // colors[ImGuiCol_ResizeGripHovered]     = (ImVec4){0.26f, 0.59f, 0.98f, 0.67f};
  // colors[ImGuiCol_ResizeGripActive]      = (ImVec4){0.26f, 0.59f, 0.98f, 0.95f};
  // //colors[ImGuiCol_CloseButton]           = (ImVec4){0.59f, 0.59f, 0.59f, 0.50f};
  // //colors[ImGuiCol_CloseButtonHovered]    = (ImVec4){0.98f, 0.39f, 0.36f, 1.00f};
  // //colors[ImGuiCol_CloseButtonActive]     = (ImVec4){0.98f, 0.39f, 0.36f, 1.00f};
  // colors[ImGuiCol_PlotLines]             = (ImVec4){0.39f, 0.39f, 0.39f, 1.00f};
  // colors[ImGuiCol_PlotLinesHovered]      = (ImVec4){1.00f, 0.43f, 0.35f, 1.00f};
  // colors[ImGuiCol_PlotHistogram]         = (ImVec4){0.90f, 0.70f, 0.00f, 1.00f};
  // colors[ImGuiCol_PlotHistogramHovered]  = (ImVec4){1.00f, 0.60f, 0.00f, 1.00f};
  // colors[ImGuiCol_TextSelectedBg]        = (ImVec4){0.26f, 0.59f, 0.98f, 0.35f};
  // //colors[ImGuiCol_ModalWindowDarkening]  = (ImVec4){0.20f, 0.20f, 0.20f, 0.35f};
  // Background colors
  colors[ImGuiCol_WindowBg]        = (ImVec4){ 0.10f, 0.10f, 0.08f, 1.0f }; // Dark brownish-green
  colors[ImGuiCol_ChildBg]         = (ImVec4){ 0.12f, 0.12f, 0.10f, 1.0f };
  colors[ImGuiCol_PopupBg]         = (ImVec4){ 0.15f, 0.15f, 0.12f, 1.0f };

  // Borders
  colors[ImGuiCol_Border]          = (ImVec4){ 0.30f, 0.25f, 0.20f, 1.0f };
  colors[ImGuiCol_BorderShadow]    = (ImVec4){ 0.00f, 0.00f, 0.00f, 0.0f };

  // Headers
  colors[ImGuiCol_Header]          = (ImVec4){ 0.20f, 0.22f, 0.15f, 1.0f };
  colors[ImGuiCol_TableHeaderBg]   = (ImVec4){ 0.20f, 0.22f, 0.15f, 1.0f };
  colors[ImGuiCol_HeaderHovered]   = (ImVec4){ 0.25f, 0.28f, 0.18f, 1.0f };
  colors[ImGuiCol_HeaderActive]    = (ImVec4){ 0.30f, 0.35f, 0.20f, 1.0f };

  // Buttons
  colors[ImGuiCol_Button]          = (ImVec4){ 0.18f, 0.20f, 0.12f, 1.0f };
  colors[ImGuiCol_ButtonHovered]   = (ImVec4){ 0.24f, 0.28f, 0.15f, 1.0f };
  colors[ImGuiCol_ButtonActive]    = (ImVec4){ 0.30f, 0.35f, 0.20f, 1.0f };

  // Frames
  colors[ImGuiCol_FrameBg]         = (ImVec4){ 0.16f, 0.18f, 0.12f, 1.0f };
  colors[ImGuiCol_FrameBgHovered]  = (ImVec4){ 0.22f, 0.25f, 0.15f, 1.0f };
  colors[ImGuiCol_FrameBgActive]   = (ImVec4){ 0.28f, 0.32f, 0.18f, 1.0f };

  // Text
  colors[ImGuiCol_Text]            = (ImVec4){ 0.85f, 0.85f, 0.78f, 1.0f }; // Slightly yellowish white
  colors[ImGuiCol_TextDisabled]    = (ImVec4){ 0.50f, 0.50f, 0.45f, 1.0f };

  // Scrollbar
  colors[ImGuiCol_ScrollbarBg]     = (ImVec4){ 0.12f, 0.12f, 0.10f, 1.0f };
  colors[ImGuiCol_ScrollbarGrab]   = (ImVec4){ 0.25f, 0.22f, 0.15f, 1.0f };
  colors[ImGuiCol_ScrollbarGrabHovered] = (ImVec4){ 0.30f, 0.28f, 0.20f, 1.0f };
  colors[ImGuiCol_ScrollbarGrabActive]  = (ImVec4){ 0.35f, 0.32f, 0.25f, 1.0f };

  // Checkboxes, sliders, progress bars
  colors[ImGuiCol_CheckMark]       = (ImVec4){ 0.80f, 0.80f, 0.60f, 1.0f };
  colors[ImGuiCol_SliderGrab]      = (ImVec4){ 0.65f, 0.60f, 0.40f, 1.0f };
  colors[ImGuiCol_SliderGrabActive] = (ImVec4){ 0.75f, 0.70f, 0.50f, 1.0f };
  colors[ImGuiCol_Separator]       = (ImVec4){ 0.30f, 0.25f, 0.20f, 1.0f };
  colors[ImGuiCol_SeparatorHovered] = (ImVec4){ 0.40f, 0.35f, 0.25f, 1.0f };
  colors[ImGuiCol_SeparatorActive]  = (ImVec4){ 0.50f, 0.45f, 0.30f, 1.0f };

  // Tabs
  colors[ImGuiCol_Tab]             = (ImVec4){ 0.18f, 0.20f, 0.12f, 1.0f };
  colors[ImGuiCol_TabHovered]      = (ImVec4){ 0.24f, 0.28f, 0.15f, 1.0f };
  //colors[ImGuiCol_TabActive]       = (ImVec4){ 0.30f, 0.35f, 0.20f, 1.0f };
}