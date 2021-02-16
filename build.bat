@echo off
REM		CAPFLAG BUILD SCRIPT v1.7

REM Set Compiler Settings Here

set CPP=c++
set GPP=g++
set WINRES=windres
set ARCH=64
set OUTPUT=CapFlag.exe
set DEBUGMODE=0


del %OUTPUT% 2>nul

setlocal enabledelayedexpansion

if not exist .objs64 (
	echo Creating Object Directory Structure...
	mkdir .objs64
)

echo Building Dependency Libraries...
if not exist .objs64\pathgrid.o (
	start /B "%%~nF.o" %CPP% -std=c++20 -DSFML_STATIC -I.\library\SFML-2.5.1\include -I.\game -I.\library -I.\library\engine -I.\library\sha1 -I.\library\pathgrid -ICapflag\game -c library\pathgrid\pathgrid.cpp -o .objs64\pathgrid.o
)
if not exist .objs64\sha1.o (
	start /B "%%~nF.o" %CPP% -std=c++20 -DSFML_STATIC -I.\library\SFML-2.5.1\include -I.\game -I.\library -I.\library\engine -I.\library\sha1 -I.\library\pathgrid -ICapflag\game -c library\sha1\sha1.cpp -o .objs64\sha1.o
)

echo Building Game Files...
for %%F in (game/*.cpp) do (
	if not exist .objs64\%%~nF.o (
		echo Building %%~nF.o
		start /B "%%~nF.o" %CPP% -std=c++20 -DSFML_STATIC -I.\library\SFML-2.5.1\include -I.\game -I.\library -I.\library\engine -I.\library\sha1 -I.\library\pathgrid -ICapflag\game -c game\%%~nF.cpp -o .objs64\%%~nF.o
	)
)
for %%F in (*.cpp) do (
	if not exist .objs64\%%~nF.o (
		echo Building %%~nF.o
		start /B "%%~nF.o" %CPP% -std=c++20 -DSFML_STATIC -I.\library\SFML-2.5.1\include -I.\game -I.\library -I.\library\engine -I.\library\sha1 -I.\library\pathgrid -ICapflag\game -c %%F -o .objs64\%%~nF.o
	)
)

if not exist .objs64\resource.res (
	echo Building Resources...
	%WINRES% -J rc -O coff -i game\resource.rc -o .objs64\resource.res
)


REM Wait for building process to finish
:loop
for /f %%G in ('tasklist ^| find /c "%CPP%"') do ( set count=%%G )
if %count%==0 (
	goto linker
) else (
	timeout /t 2 /nobreak>nul
	goto loop
)

:linker

set "files="
for /f "delims=" %%A in ('dir /b /a-d ".objs64\%*" ') do set "files=!files! .objs64\%%A"

echo Linking Executable...
if %ARCH%==64 (
	set SFML_LIBRARY_DIR=.\library\SFML-2.5.1\lib64
	set PORTAUDIO_LIB=portaudio64-s
	set ENGINE_LIB=engine64-s
	goto link
)
if %ARCH%==32 (
	set SFML_LIBRARY_DIR=.\library\SFML-2.5.1\lib
	set PORTAUDIO_LIB=portaudio-s
	set ENGINE_LIB=engine-s
	goto link
)
echo ARCH Must be 32 or 64! Make sure ARCH matches the compiler's architecture!
goto finish
:link

if %DEBUGMODE% GTR 0 (
	set MWINDOWS=
) else (
	set MWINDOWS=-mwindows
)

%GPP% -L.\library -L%SFML_LIBRARY_DIR% -o %OUTPUT% %files% -s -static-libstdc++ -static-libgcc -static -lpthread -static-libstdc++ -static-libgcc -static -l%ENGINE_LIB% -l%PORTAUDIO_LIB% -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lsfml-network-s -lsetupapi -lwinmm -lopengl32 -lgdi32 -lfreetype -lws2_32 -lcomdlg32 %MWINDOWS%

:finish
if exist .\%OUTPUT% (
	echo Build Success!
) else (
	echo Build Failed!
)

pause>nul