$VS_TOOLS_DIR = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools"

$VS_TOOLS_DIR\VsDevCmd.bat

mkdir obj
mkdir bin

del .\obj\*.obj
del /Q .\bin\*.*

nmake

move *.obj .\obj
move XM7.exe .\bin
move XM7.map .\bin
move w32_res.res .\bin
copy .\Roms\*.* .\bin


