#include "layout.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <fstream>
#include <string>

/**
 * @brief Hard coded layout data
 *
 */
const std::string IMGUI_LAYOUT_STR = R"(
[Window][Debug##Default]
Pos=119,48
Size=400,400
Collapsed=0

[Window][Dear ImGui Demo]
Pos=661,20
Size=550,680
Collapsed=0

[Window][Hello, world!]
Pos=60,61
Size=339,180
Collapsed=0

[Window][Editor]
Pos=975,19
Size=305,480
Collapsed=0
DockId=0x00000004,0

[Window][Gizmo]
Pos=310,199
Size=929,548
Collapsed=0

[Window][Other controls]
Pos=87,156
Size=861,478
Collapsed=0

[Window][Graph Editor]
Pos=624,126
Size=624,447
Collapsed=0

[Window][DockSpaceViewport_11111111]
Pos=0,19
Size=1280,701
Collapsed=0

[Window][Scene Window]
Pos=413,0
Size=750,509
Collapsed=0
DockId=0x00000003,0

[Window][Example: Property editor]
Pos=223,155
Size=430,450
Collapsed=0

[Window][###AnimatedTitle]
Pos=100,300
Size=247,48
Collapsed=0

[Window][Same title as another window##2]
Pos=125,391
Size=451,61
Collapsed=0

[Window][Same title as another window##1]
Pos=131,497
Size=451,61
Collapsed=0

[Window][DockSpace Demo]
Pos=0,19
Size=1280,701
Collapsed=0

[Window][Delete?]
Pos=484,307
Size=311,105
Collapsed=0

[Window][Stacked 1]
Pos=435,274
Size=409,172
Collapsed=0

[Window][Scene graph]
Pos=0,531
Size=1470,275
Collapsed=0
DockId=0x00000002,0

[Window][Example: Simple layout]
Pos=51,238
Size=500,440
Collapsed=0

[Window][Example: Console]
Pos=198,55
Size=520,600
Collapsed=0

[Window][Example: Log]
Pos=255,35
Size=500,400
Collapsed=0

[Window][Dear ImGui Style Editor]
Pos=569,11
Size=353,695
Collapsed=0

[Window][Outliner]
Pos=0,501
Size=272,219
Collapsed=0
DockId=0x00000007,0

[Window][Session Layer]
Pos=274,501
Size=1006,219
Collapsed=0
DockId=0x00000008,0

[Window][Session Layer Trace]
Pos=273,661
Size=1197,145
Collapsed=0
DockId=0x0000000A,0

[Window][Script Editor]
Pos=1113,511
Size=357,295
Collapsed=0
DockId=0x00000008,0

[Window][flatten Stage]
Pos=275,511
Size=1195,295
Collapsed=0
DockId=0x00000008,1

[Window][Viewport]
Pos=0,19
Size=973,480
Collapsed=0
DockId=0x00000003,0

[Window][Choose File##LoadFile]
Pos=358,135
Size=582,434
Collapsed=0

[Window][Choose File##ExportFile]
Pos=358,135
Size=582,434
Collapsed=0

[Table][0xD0F0C6E3,2]
Column 0  Weight=1.0000
Column 1  Weight=1.0000

[Table][0x8DFA6E86,2]
Column 0  Weight=1.0000
Column 1  Weight=1.0000

[Table][0xFABAAEF7,2]
Column 0  Weight=1.0000
Column 1  Weight=1.0000

[Table][0x49F8DCEA,3]
RefScale=13
Column 0  Weight=1.0000
Column 1  Width=84
Column 2  Width=126

[Table][0x45A0E60D,7]
RefScale=13
Column 0  Width=49
Column 1  Width=112
Column 2  Width=112
Column 3  Width=112
Column 4  Width=112
Column 5  Width=112
Column 6  Width=112

[Table][0x64418101,3]
RefScale=13
Column 0  Width=63
Column 1  Width=63
Column 2  Width=63

[Table][0xA43C3885,3]
RefScale=13
Column 0  Width=56
Column 1  Width=56
Column 2  Width=101

[Table][0x834DD975,4]
RefScale=13
Column 0  Sort=0v

[Table][0xF74273FF,4]
RefScale=13
Column 0  Sort=0v

[Docking][Data]
DockSpace       ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,19 Size=1280,701 Split=Y
  DockNode      ID=0x00000005 Parent=0x8B93E3BD SizeRef=1470,480 Split=Y
    DockNode    ID=0x00000001 Parent=0x00000005 SizeRef=1470,529 Split=X
      DockNode  ID=0x00000003 Parent=0x00000001 SizeRef=750,806 CentralNode=1 Selected=0x13926F0B
      DockNode  ID=0x00000004 Parent=0x00000001 SizeRef=305,806 Selected=0x9F27EDF6
    DockNode    ID=0x00000002 Parent=0x00000005 SizeRef=1470,275 Selected=0xF5BE1C77
  DockNode      ID=0x00000006 Parent=0x8B93E3BD SizeRef=1470,219 Split=Y Selected=0xA2B48D87
    DockNode    ID=0x00000009 Parent=0x00000006 SizeRef=1197,148 Split=X Selected=0xA2B48D87
      DockNode  ID=0x00000007 Parent=0x00000009 SizeRef=272,295 Selected=0x94263E27
      DockNode  ID=0x00000008 Parent=0x00000009 SizeRef=1006,295 Selected=0xA2B48D87
    DockNode    ID=0x0000000A Parent=0x00000006 SizeRef=1197,145 Selected=0xDAFD3032
)";

void LoadDefaultLayout()
{
    const char* layoutData = IMGUI_LAYOUT_STR.c_str();
    const int layoutDataSize = IMGUI_LAYOUT_STR.size();
    ImGui::LoadIniSettingsFromMemory(layoutData, layoutDataSize);
}

void LoadDefaultOrCustomLayout()
{
    std::ifstream f("imgui.ini");
    if (!f.good()) LoadDefaultLayout();
}