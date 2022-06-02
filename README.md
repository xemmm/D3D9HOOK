# UniversalD3D9 Hooking
 Allows you to to hook ImGui into any 32-bit DX9 game.
 
 To change to 64 bit game change to release X64
 -> Reinclude Windows SDK Library/ProjectDir includes & rebuild Detours library in x64(https://github.com/Nukem9/detours) and throw it in our $(ProjectDir)lib
 -> In our mainThread function change oEndScene to Detours::X64 library.  
