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





SDL_Window *window = NULL;

int main(int argc, char* argv[])
{
  //allow printf
  setbuf(stdout, NULL);


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
      "Steam De-Clutter", 0, 0, 800, 600,
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

  igStyleColorsDark(NULL);
  //ImFontAtlas_AddFontDefault(io.Fonts, NULL);

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
  } else {
    printf("Unable to locate steam installation. Please provide install location\n");
  }
  FindGames(&SteamData);



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
    

    igBegin("SteamSweep!", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    igGetContentRegionAvail(&windowspace);

    if (igBeginTable("ItemTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | 
      ImGuiTableFlags_Sortable, (ImVec2){0, ioptr->DisplaySize.y - 150}, 0)) {
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

      for (int i = 0; i < SteamData.GameCount-1; i++) {
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
    

    igEnd();


    // render
    igRender();
    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

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

