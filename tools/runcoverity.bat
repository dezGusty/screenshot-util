@Echo off

Echo.---------------------------------------------------------------------
Echo.This batch file will build the archive containing the coverity build.
SETLOCAL EnableDelayedExpansion
SETLOCAL EnableExtensions

Rem
Rem Go to the directory where the batch is located... you know... in case we use "Run As Administrator".
Rem
PushD %~dp0

Rem
Rem Set some variables
Rem
Set directory_to_use=!CD!\..\solution
Set solution_name=screenshot-util-vs2013.sln
Set archive_name=screenshot-util-vs2013.zip

Rem
Rem Display them to the user. It's easier to see when something is wrong.
Rem
Echo.---------------------------------------------------------------------
Echo.Requirements:
Echo.directory_to_use :   !directory_to_use!
Echo.solution_name    :   !solution_name!
Echo.---------------------------------------------------------------------


Rem
Rem Ensure that coverity is found.
Rem
Echo.Attempting to use coverity directory defined in environment variable.
If "!COVDIR!"=="" (
  Echo.Unable to locate coverity environment variable [COVDIR].
  Echo.First, make sure that you have coverity installed.
  Echo.Second, create the [COVDIR] environment variable so that the next file path is valid:
  Echo.[COVDIR]\bin\cov-build.exe
  GoTo :eof
) Else (
  Echo.Found environment variable COVDIR: [!COVDIR!]
  If Not Exist "%COVDIR%\bin\cov-build.exe" (
    Echo.Could not locate coverity build executable.
    GoTo :eof
  )
)

Rem
Rem Call the Visual Studio environment variable batch.
Rem
Echo.Current directory is !CD!
Echo.Calling VSExpress variable setting batch file...
If Not Exist "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" (
  Echo.Well, I really thought that you would use Visual Studio 2013.
  Echo.Other versions of Visual Studio probably work as well, but you will 
  Echo.have to manually call the [vcvarsall.bat] script before launching this one.
  GoTo :eof
)

CALL "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" x86

Echo.Preparing build switches... 
SET MSBUILD_SWITCHES=/nologo /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true /target:Rebuild /property:Configuration="Release";Platform=Win32

Rem
Rem Go to the build folder, where the project output shall be placed.
Rem
Echo.Navigating to build folder...
If Not Exist !directory_to_use! (
  Echo.Unfortunately, this batch file expects you to use [build] subdirectory as the build directory for CMake.
  Echo.Could not find a directory "!directory_to_use!". Exiting.
  GoTo :eof
)

Rem
Rem Call the coverity build
Rem
PushD !directory_to_use!
Echo.Current directory is !CD!
"%COVDIR%\bin\cov-build.exe" --dir cov-int MSBuild "!solution_name!" %MSBUILD_SWITCHES%

Echo.Built project !solution_name!

Rem
Rem Locate 7zip.
Rem
Set SevenZipPath=

If "!SevenZipPath!"=="" (
  If Not "%SEVENZ_HOME%"=="" (
    Set SevenZipPath=%SEVENZ_HOME%
  )
)

If "!SevenZipPath!"=="" (
  If Exist "C:\Program Files\7-Zip\7z.exe" (
    Set SevenZipPath=C:\Program Files\7-Zip
  )
)

If "!SevenZipPath!"=="" (
  If Exist d:\Programe\7zip\7z.exe (
    Set SevenZipPath=d:\Programe\7zip
  )
)

If Not Exist "!SevenZipPath!\7z.exe" (
  Echo Could not find 7zip command line! Make sure you have 7zip installed. If you installed it in a location other than
  Echo the default, define the environment variable SEVENZ_HOME to point to the folder where 7z.exe can be found.
  Pause
  GoTo :eof
)

Echo Using 7zip from ... !SevenZipPath!
Echo.Compressing output...
Set _my_file_name=!archive_name!
Call "!SevenZipPath!\7z.exe" a -tzip !_my_file_name! m1=LZMA:d=21 -mmt -mx7 -r cov-int


REM Built coverity report. Compress it.
PopD
Echo.Compressed output...
If Exist "submitcoveritybuild.bat" (
  Echo.A batch for auto-submitting the coverity build has been detected and will be called.
  Call "submitcoveritybuild.bat"
) Else (
  Echo.You may now submit your build.
)

:End
PopD

Echo. & ECHO Press any key to close this window...
PAUSE >NUL
ENDLOCAL
EXIT /B