::		Custom Build Script 2.4
::
::       Custom library support
::
::     Set Compiler Settings Here


:: Compiler Specification And Build Settings
set CPP=c++
set GPP=g++
set GCC=gcc
set WINDRES=windres
set OUTPUT=CapFlag.exe
set DEBUGMODE=0
set COMMANDLINE=0

set VERBOSE=0

set AUTO_REBUILD=1
set REBUILD_SOURCE_DIRECTORIES=0
set REBUILD_SOURCE_LIBRARIES=0
set ASYNC_BUILD=1

set LINK_ONLY=0

:: Configure Source For Compiling And Additional Custom Library Directories / Names
set SOURCE_DIRECTORIES=src game library\pathgrid library\sha1
set INCLUDE_DIRECTORIES=src game library\engine library\pathgrid library\sha1 library\SFML-2.5.1\include
set LIBRARY_DIRECTORIES=
set LIBRARY_NAMES=pthread engine64-s portaudio64-s sfml-graphics-s sfml-window-s sfml-system-s sfml-network-s setupapi winmm opengl32 gdi32 freetype ws2_32 comdlg32

:: Additional Compiler Flags And Configuration Settings
set CPP_COMPILER_FLAGS=-std=c++20 -DSFML_STATIC
set C_COMPILER_FLAGS=
set OBJECT_DIRECTORY=.objs


:: Advanced / Extra Command Line Settings For Building / Linking
set ADDITIONAL_INCLUDEDIRS=
set ADDITIONAL_LIBRARIES=-static-libstdc++ -static-libgcc -static
set ADDITIONAL_LIBDIRS=-Llibrary -Llibrary\SFML-2.5.1\lib64


:: Custom Library Support Directory Names
set LIBRARY_DIRECTORY_NAME=lib\windows
set INCLUDE_DIRECTORY_NAME=include
set SOURCE_DIRECTORY_NAME=src


:: Recursive Directories
set RECURSIVE_INCLUDES=0
set RECURSIVE_SOURCE=0

:: Single File Compilation
set SINGLE_FILE=%1

:: Add Optimization To Release and a define for debug mode
if %DEBUGMODE% EQU 0 (
	set CPP_COMPILER_FLAGS=%CPP_COMPILER_FLAGS% -O3 -Os
) else (
	set CPP_COMPILER_FLAGS=%CPP_COMPILER_FLAGS% -DDEBUGMODE -DDEBUG_MODE
)

@echo off

:: ---------- Build Script Start -----------

cls
:: Force current directory to program directory

echo Starting Build Process...

pushd "%~dp0"

setlocal enabledelayedexpansion

:: Calculate a timestamp
for /f "tokens=2,3,4 delims=/ " %%x in ("%DATE%") do set _timestamp=%%x/%%y/%%z
for /f "useback tokens=*" %%b in (`time /T`) do set _HR=%%b & set _timestamp=!_timestamp! !_HR:~0,2!& set _PM=!_HR:~6,2!
for /f "tokens=2,3 delims=:. " %%x in ("%TIME%") do set _timestamp=!_timestamp!:%%x:%%y
set _timestamp=!_timestamp! !_PM!

:: Iterate Include Directories
if %RECURSIVE_INCLUDES% GTR 0 (
	set TEMP_DIRS=!INCLUDE_DIRECTORIES!
	for /f "tokens=*" %%D IN ('DIR %INCLUDE_DIRECTORIES% /AD /B /S') do (
		set TEMP_DIRS=!TEMP_DIRS! %%D
	)
	set INCLUDE_DIRECTORIES=!TEMP_DIRS!
)

:: Iterate Source Directories
if %RECURSIVE_SOURCE% GTR 0 (
	set TEMP_DIRS=!SOURCE_DIRECTORIES!
	for /f "tokens=*" %%D IN ('DIR %SOURCE_DIRECTORIES% /AD /B /S') do (
		if not "%%~nxD" == "%OBJECT_DIRECTORY%" (
			set TEMP_DIRS=!TEMP_DIRS! %%D
		)
	)
	set SOURCE_DIRECTORIES=!TEMP_DIRS!
)

:: Configure Raw MinGW Command Line From Custom Settings
(for %%D in (%INCLUDE_DIRECTORIES%) do (
	set ADDITIONAL_INCLUDEDIRS=!ADDITIONAL_INCLUDEDIRS! -I%%D
))

:: Source directories are separated for libraries
set LIBRARY_SOURCE_DIRECTORIES=

(for %%D in (%LIBRARY_DIRECTORIES%) do (
	set ADDITIONAL_INCLUDEDIRS=!ADDITIONAL_INCLUDEDIRS! -I%%D\!INCLUDE_DIRECTORY_NAME!
	set LIBRARY_SOURCE_DIRECTORIES=!LIBRARY_SOURCE_DIRECTORIES! %%D\!SOURCE_DIRECTORY_NAME!
	set ADDITIONAL_LIBDIRS=!ADDITIONAL_LIBDIRS! -L%%D\!LIBRARY_DIRECTORY_NAME!
))

(for %%D in (%LIBRARY_NAMES%) do (
	set ADDITIONAL_LIBRARIES=!ADDITIONAL_LIBRARIES! -l%%D
))

:: Debugging Info
:: echo %ADDITIONAL_INCLUDEDIRS%
:: echo %ADDITIONAL_LIBDIRS%
:: echo %ADDITIONAL_LIBRARIES%
:: echo %SOURCE_DIRECTORIES%
::----------------------

:: Find The Single File To Remove When Targeting A Build File
if not [%SINGLE_FILE%] == [] (

	set REBUILD_SOURCE_DIRECTORIES=0
	set REBUILD_SOURCE_LIBRARIES=0
	set AUTO_REBUILD=0
	set LINK_ONLY=0
	set _DELETE=*_%SINGLE_FILE%.o
	
	echo Cleanup %SINGLE_FILE%...

	(for %%D in (%SOURCE_DIRECTORIES%) do (
		del /S /Q "%%D\%OBJECT_DIRECTORY%\!_DELETE!" 2>nul
	)) 

	(for %%D in (%LIBRARY_SOURCE_DIRECTORIES%) do (
		del /S /Q "%%D\%OBJECT_DIRECTORY%\!_DELETE!" 2>nul
	))
)


if %AUTO_REBUILD% GTR 0 (
	echo Searching for changes...

	set MODLIST=
	(for %%D in (%SOURCE_DIRECTORIES% %LIBRARY_SOURCE_DIRECTORIES%) do (
		if %VERBOSE% GTR 0 (
			echo ... in %%~nxD...
		)

		call :recursive_search %%D cpp %CPP%
		set MODLIST=!MODLIST! !MODIFIED_FILES!

		call :recursive_search %%D c %GCC%
		set MODLIST=!MODLIST! !MODIFIED_FILES!
	))

	(for %%F in (!MODLIST!) do (
		del /S /Q "%%~dpF%OBJECT_DIRECTORY%\%%~nF.o" 2>nul
		if %VERBOSE% GTR 0 (
			echo del /S /Q "%%~dpF%OBJECT_DIRECTORY%\%%~nF.o"
		)
	))
)

echo Cleaning Up Old Application...
taskkill /F /IM %OUTPUT% 2>nul
del %OUTPUT% 2>nul
taskkill /F /IM "ld.exe" 2>nul

if %DEBUGMODE% GTR 0 (
	set DEBUG_INFO=-ggdb -g
) else (
	set DEBUG_INFO=-s
)

if %ASYNC_BUILD% GTR 0 (
	set WAIT=
) else (
	set WAIT=/WAIT
)

set OBJECT_DIRS=

:: Delete objects from object directories / populate object directories array
(for %%D in (%SOURCE_DIRECTORIES%) do (
	if %REBUILD_SOURCE_DIRECTORIES% GTR 0 if %LINK_ONLY% EQU 0 (
		del /S /Q "%%~D\%OBJECT_DIRECTORY%\*.o" 2>nul
		del /S /Q "%%~D\%OBJECT_DIRECTORY%\*.res" 2>nul   
	)
	set OBJECT_DIRS=!OBJECT_DIRS! %%D\!OBJECT_DIRECTORY!
))

(for %%D in (%LIBRARY_SOURCE_DIRECTORIES%) do (
	if %REBUILD_SOURCE_LIBRARIES% GTR 0 if %LINK_ONLY% EQU 0 (
		del /S /Q "%%D\%OBJECT_DIRECTORY%\*.o" 2>nul
	)
	set OBJECT_DIRS=!OBJECT_DIRS! %%D\!OBJECT_DIRECTORY!
))

if %LINK_ONLY% GTR 0 (
	goto linker
)

:: Create Object Directory Structure
(for %%D in (%SOURCE_DIRECTORIES%) do (
	if exist %%D\ (
		if not exist %%D\%OBJECT_DIRECTORY% (
			echo Creating Object Directory Structure...
			mkdir %%D\%OBJECT_DIRECTORY%
		)
	)
))
(for %%D in (%LIBRARY_SOURCE_DIRECTORIES%) do (
	if exist %%D\ (
		if not exist %%D\%OBJECT_DIRECTORY% (
			echo Creating Object Directory Structure...
			mkdir %%D\%OBJECT_DIRECTORY%
		)
	)
))

echo Begin Building...

(for %%D in (%LIBRARY_SOURCE_DIRECTORIES%) do (
	echo Building Library Files For %%D...
	if exist %%D\ (
		call :compile_function %%D cpp %CPP% "%CPP_COMPILER_FLAGS%"
		call :compile_function %%D c %GCC% "%C_COMPILER_FLAGS%"
	) else (
		echo Skipping non-existent directory...
	)
))

(for %%D in (%SOURCE_DIRECTORIES%) do (
	echo Building Source Files For %%D...
	if exist %%D\ (
		call :compile_function %%D cpp %CPP% "%CPP_COMPILER_FLAGS%"
		call :compile_function %%D c %GCC% "%C_COMPILER_FLAGS%"
		call :resource_function %%D rc %WINDRES%
	) else (
		echo Skipping non-existent directory...
	)
))

goto linker

:: ---------- Compiler Function -----------
::	SourceDirctory FileExtention Compiler CompilerFlags
:compile_function
	set OBJ_DIR=%1\%OBJECT_DIRECTORY%
	for %%F in (%1\*.%2) do (
		if not exist !OBJ_DIR!\%~n3_%%~nF.o (
			echo Building %~n3_%%~nF.o
			start /B %WAIT% "%%~nF.o" %3 %ADDITIONAL_INCLUDEDIRS% %~4 %DEBUG_INFO% -c %%F -o !OBJ_DIR!\%~n3_%%~nF.o

			if %VERBOSE% GTR 0 (
				echo %3 %ADDITIONAL_INCLUDEDIRS% %~4 %DEBUG_INFO% -c %%F -o !OBJ_DIR!\%~n3_%%~nF.o
			)
			if %ASYNC_BUILD% GTR 0 (
				call :async_wait
			)
		)
	)
goto close
:resource_function
	set OBJ_DIR=%1\%OBJECT_DIRECTORY%
	for %%F in (%1\*.%2) do (
		if not exist !OBJ_DIR!\%~n3_%%~nF.res (
			echo Building %~n3_%%~nF.res
			start /B %WAIT% "%%~nF.res" %3 %%F -O coff -o !OBJ_DIR!\%~n3_%%~nF.res

			if %VERBOSE% GTR 0 (
				echo %3 %%F -O coff -o !OBJ_DIR!\%~n3_%%~nF.res
			)
			if %ASYNC_BUILD% GTR 0 (
				call :async_wait
			)
		)
	)
goto close

:: -------- Asnychronous Wait ---------
:: 	Block until the hardware concurrency is matched
:async_wait
	set /A count=0
	for /f %%G in ('tasklist ^| find /c "%CPP%"') do ( set /A count+=%%G )
	for /f %%G in ('tasklist ^| find /c "%GCC%"') do ( set /A count+=%%G )
	for /f %%G in ('tasklist ^| find /c "%GPP%"') do ( set /A count+=%%G )

	if %count% LSS %NUMBER_OF_PROCESSORS% (
		goto close
	) else (
		timeout /t 1 /nobreak>nul
		goto async_wait
	)
goto close

:: 	Block until there are no more build units running
:async_wait_all
	set /A count=0
	for /f %%G in ('tasklist ^| find /c "%CPP%"') do ( set /A count+=%%G )
	for /f %%G in ('tasklist ^| find /c "%GCC%"') do ( set /A count+=%%G )
	for /f %%G in ('tasklist ^| find /c "%GPP%"') do ( set /A count+=%%G )

	if %count% EQU 0 (
		goto close
	) else (
		timeout /t 1 /nobreak>nul
		goto async_wait_all
	)
goto close
::--------------------------------------

::	Modified File Searcher

:recursive_search

set MODIFIED_FILES=

set _list=
for %%F in (%1\*.%2) do (
	if %VERBOSE% GTR 0 (
		echo Check %%~nF.%2
	)
	set _list=!_list! %%F
)

call :find_modified_files _list %3

goto close

:find_modified_files

for %%F in (!%1!) do (
	set moddate=%%~tF

	pushd %%~dpF
	set f="%%~nF%%~xF"
	for /f "useback tokens=*" %%b in (`forfiles /M !f! /C "cmd /c echo @ftime"`) do (
		for /f "tokens=1,2,3 delims=: " %%x in ("%%b") do set hr=%%x&set min=%%y&set SECONDS=%%z
	)
	popd
	set pm=!moddate:~17,2!
	set moddate=!moddate:~0,-3!:!SECONDS! !pm!


	call :setdate "!moddate!" CURRENT

	call :loaddata %%F

	call :setdate "!datecode!" LASTCOMPILED

	call :check_dates %%F %2
)
goto close

:check_dates
	set /A LASTCOMPILED_SEC=!LASTCOMPILED_SEC!+5

	if !CURRENT_YEAR! GTR !LASTCOMPILED_YEAR! goto changed
	if !CURRENT_YEAR! EQU !LASTCOMPILED_YEAR! goto check_month
	goto skip

	:check_month
	if !CURRENT_MONTH! GTR !LASTCOMPILED_MONTH! goto changed
	if !CURRENT_MONTH! EQU !LASTCOMPILED_MONTH! goto check_day
	goto skip

	:check_day
	if !CURRENT_DAY! GTR !LASTCOMPILED_DAY! goto changed
	if !CURRENT_DAY! EQU !LASTCOMPILED_DAY! goto check_hour
	
	goto skip
	:check_hour
	if !CURRENT_HOUR! GTR !LASTCOMPILED_HOUR! goto changed
	if !CURRENT_HOUR! EQU !LASTCOMPILED_HOUR! goto check_min
	
	goto skip
	:check_min
	if !CURRENT_MIN! GTR !LASTCOMPILED_MIN! goto changed
	if !CURRENT_MIN! EQU !LASTCOMPILED_MIN! goto check_sec

	goto skip
	:check_sec
	if !CURRENT_SEC! GTR !LASTCOMPILED_SEC! goto changed
	goto skip
:changed

	if %VERBOSE% GTR 0 (
		echo File Changed: %~n1%~x1
		echo !moddate! is newer than !datecode!
		echo Record - !_timestamp!
	)

	set MODIFIED_FILES=!MODIFIED_FILES! "%~dp1%~n2_%~n1%~x1"
	echo !_timestamp!>%1:datecode

:skip
goto close

:setdate
	set _date=%1
	for /f "useback tokens=*" %%a in ('%_date%') do set _date=%%~a

	set /A "%2_YEAR=0"
	set /A "%2_MONTH=0"
	set /A "%2_DAY=0"
	set /A "%2_HOUR=0"
	set /A "%2_MIN=0"
	set /A "%2_SEC=0"

	for /f "tokens=* delims=0" %%a in ("!_date:~6,4!") do (
		set _tn=
		set /a _tn=%%a 2>nul
		if {!_tn!}=={%%a} (
			if not {%%a}=={} ( set /A "%2_YEAR=!_tn!" )
		)
	)
	for /f "tokens=* delims=0" %%a in ("!_date:~0,2!") do (
		set _tn=
		set /a _tn=%%a 2>nul
		if {!_tn!}=={%%a} (
			if not {%%a}=={} ( set /A "%2_MONTH=!_tn!" )
		)
	)
	for /f "tokens=* delims=0" %%a in ("!_date:~3,2!") do (
		set _tn=
		set /a _tn=%%a 2>nul
		if {!_tn!}=={%%a} (
			if not {%%a}=={} ( set /A "%2_DAY=!_tn!" )
		)
	)
	for /f "tokens=* delims=0" %%a in ("!_date:~11,2!") do (
		set _tn=
		set /a _tn=%%a 2>nul
		if {!_tn!}=={%%a} (
			if not {%%a}=={} ( set /A "%2_HOUR=!_tn!" )
		)
	)
	for /f "tokens=* delims=0" %%a in ("!_date:~14,2!") do (
		set _tn=
		set /a _tn=%%a 2>nul
		if {!_tn!}=={%%a} (
			if not {%%a}=={} ( set /A "%2_MIN=!_tn!" )
		)
	)
	for /f "tokens=* delims=0" %%a in ("!_date:~17,2!") do (
		set _tn=
		set /a _tn=%%a 2>nul
	
		if {!_tn!}=={%%a} (
			if not {%%a}=={} ( set /A "%2_SEC=!_tn!" )
		)
	)
	if !%2_HOUR! LSS 12 (
		for /f "tokens=* delims=0" %%a in ("!_date:~20,2!") do (
			set _ts=%%a
			if {!_ts!}=={%%a} (
				if {!_ts!} == {PM} ( set /A "%2_HOUR=!%2_HOUR!+12" )
			)
		)
	)

goto close

:loaddata
	for /f "useback tokens=*" %%a in ('%1') do set _path="%%~a:datecode"

	set datecode=00/00/0000 00:00:00
	for /f "useback delims=" %%A in (`"more<!_path!"`) do (
		set datecode=%%A
	)

	if "!datecode!" == "00/00/0000 00:00:00" (
		echo !datecode!>!_path!
	)

goto close









::--------------------------------------
:linker

:: Wait for building process to finish all tasks before linking
call :async_wait_all

set "files="

:: Find All Object Files
(for %%D in (%OBJECT_DIRS%) do (
	if exist %%D\ (
		for /f "delims=" %%A in ('dir /b /a-d "%%D\*.o" ') do set "files=!files! %%D\%%A"
		for /f "delims=" %%A in ('dir /b /a-d "%%D\*.res" ') do set "files=!files! %%D\%%A"
	)
))

:link
echo Linking Executable...

if %COMMANDLINE% GTR 0 (
	set MWINDOWS=
) else (
	set MWINDOWS=-mwindows
)

if %VERBOSE% GTR 0 (
	echo %GPP% %ADDITIONAL_LIBDIRS% -o %OUTPUT% %files% %ADDITIONAL_LIBRARIES% %MWINDOWS%
)

%GPP% %ADDITIONAL_LIBDIRS% -o %OUTPUT% %files% %ADDITIONAL_LIBRARIES% %MWINDOWS%

:finish
if exist .\%OUTPUT% (
	echo Build Success!
) else (
	echo Build Failed!
)


:: This control is for batch processing functions to return
:close