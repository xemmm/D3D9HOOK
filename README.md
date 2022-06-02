# UniversalD3D9 Hooking
 Allows you to to hook ImGui into any 32-bit DX9 game.
 
 To swap to a 64 bit game change to release X64
 -> Reinclude Windows SDK Library/ProjectDir includes & rebuild Detours library in x64, and throw it in our $(ProjectDir)lib
 -> The Library can be found here: (https://github.com/Nukem9/detours)
 -> In our mainThread function change oEndScene to Detours::X64 library.  
