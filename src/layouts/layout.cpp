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
[Window][Editor]
Pos=865,19
Size=415,93
Collapsed=0
DockId=0x0000000D,0

[Window][Scene Window]
Pos=413,0
Size=750,509
Collapsed=0
DockId=0x00000003,0

[Window][Scene graph]
Pos=0,531
Size=1470,275
Collapsed=0
DockId=0x00000002,0

[Window][Outliner]
Pos=0,19
Size=205,485
Collapsed=0
DockId=0x00000007,0

[Window][Session Layer]
Pos=865,114
Size=415,390
Collapsed=0
DockId=0x0000000E,0

[Window][Session Layer Trace]
Pos=273,661
Size=1197,145
Collapsed=0
DockId=0x0000000A,0

[Window][Script Editor]
Pos=1113,511
Size=357,295
Collapsed=0
DockId=0x0000000B,0

[Window][flatten Stage]
Pos=275,511
Size=1195,295
Collapsed=0
DockId=0x0000000B,1

[Window][Viewport]
Pos=207,19
Size=656,485
Collapsed=0
DockId=0x00000008,0

[Window][Choose File##LoadFile]
Pos=277,155
Size=624,351
Collapsed=0

[Window][Choose File##ExportFile]
Pos=277,155
Size=624,351
Collapsed=0

[Window][Choose File##DomeLightFile]
Pos=277,155
Size=624,351
Collapsed=0

[Window][DockSpaceViewport_11111111]
Pos=0,19
Size=1280,701
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][Scene Index View]
Pos=0,506
Size=860,214
Collapsed=0
DockId=0x00000011,0

[Window][Scene Index Attribute]
Pos=862,506
Size=418,214
Collapsed=0
DockId=0x00000012,0

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

[Table][0xC736EC2E,4]
RefScale=13
Column 0  Sort=0v

[Table][0x56106076,4]
RefScale=13
Column 0  Sort=0v

[Table][0x9A17D9B2,4]
RefScale=13
Column 0  Sort=0v

[Table][0xDE26929E,4]
RefScale=13
Column 0  Sort=0v

[Table][0x744DC63C,4]
RefScale=13
Column 0  Sort=0v

[Table][0xEDB79678,4]
RefScale=13
Column 0  Sort=0v

[Table][0x41ACBBE1,4]
RefScale=13
Column 0  Sort=0v

[Table][0xECD54214,4]
RefScale=13
Column 0  Sort=0v

[Table][0x40CE6F8D,4]
RefScale=13
Column 0  Sort=0v

[Docking][Data]
DockSpace             ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,19 Size=1280,701 Split=Y
  DockNode            ID=0x00000005 Parent=0x8B93E3BD SizeRef=1470,491 Split=Y
    DockNode          ID=0x00000001 Parent=0x00000005 SizeRef=1470,529 Split=X
      DockNode        ID=0x00000003 Parent=0x00000001 SizeRef=1040,806 Split=Y Selected=0x13926F0B
        DockNode      ID=0x0000000F Parent=0x00000003 SizeRef=642,485 Split=X Selected=0x13926F0B
          DockNode    ID=0x00000013 Parent=0x0000000F SizeRef=863,512 Split=X Selected=0x13926F0B
            DockNode  ID=0x00000007 Parent=0x00000013 SizeRef=205,512 Selected=0x94263E27
            DockNode  ID=0x00000008 Parent=0x00000013 SizeRef=656,512 CentralNode=1 Selected=0x13926F0B
          DockNode    ID=0x00000014 Parent=0x0000000F SizeRef=415,512 Split=Y Selected=0x9F27EDF6
            DockNode  ID=0x0000000D Parent=0x00000014 SizeRef=319,93 Selected=0x9F27EDF6
            DockNode  ID=0x0000000E Parent=0x00000014 SizeRef=319,390 Selected=0xA2B48D87
        DockNode      ID=0x00000010 Parent=0x00000003 SizeRef=642,214 Split=X Selected=0x07D9D459
          DockNode    ID=0x00000011 Parent=0x00000010 SizeRef=860,187 Selected=0x07D9D459
          DockNode    ID=0x00000012 Parent=0x00000010 SizeRef=418,187 Selected=0x3ADDE625
      DockNode        ID=0x00000004 Parent=0x00000001 SizeRef=428,806 Selected=0x9F27EDF6
    DockNode          ID=0x00000002 Parent=0x00000005 SizeRef=1470,275 Selected=0xF5BE1C77
  DockNode            ID=0x00000006 Parent=0x8B93E3BD SizeRef=1470,208 Split=Y Selected=0xA2B48D87
    DockNode          ID=0x00000009 Parent=0x00000006 SizeRef=1197,148 Split=X Selected=0xA2B48D87
      DockNode        ID=0x0000000B Parent=0x00000009 SizeRef=630,219 Selected=0x07D9D459
      DockNode        ID=0x0000000C Parent=0x00000009 SizeRef=648,219 Selected=0x3ADDE625
    DockNode          ID=0x0000000A Parent=0x00000006 SizeRef=1197,145 Selected=0xDAFD3032
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