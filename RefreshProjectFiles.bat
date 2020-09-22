@echo off

:: Setup, change these as necessary
set PathToEngine=C:\Unreal\EngineInstalls\UE_4.24
set ProjectName=CoopGame
:: End Setup

@echo Removing .vs folder
cd %~dp0 
if exist .vs RD /s /q "%cd%\.vs"

@echo Removing vs solution
del /q "*.sln"

:: Note that I am not simply removing the entirety of the build/intermediate directories
:: as in some cases it is useful to symbolic link folders. By deleting only the contents of the 
:: folders, we maintain the link

@echo Removing Binaries
cd %~dp0\Binaries
for /r %%a in (*) do (
	del /q /s "%%a"
)

for /d %%a in (*) do (
	rmdir /q /s "%%a"
)

@echo Removing Intermediates
cd %~dp0\Intermediate
for /r %%a in (*) do (
	del /q /s "%%a"
)

for /d %%a in (*) do (
	rmdir /q /s "%%a"
)

@echo Generating New Project Files
set UProjectPath=%~dp0\%ProjectName%.uproject
"%PathToEngine%\Engine\Binaries\DotNET\UnrealBuildTool.exe" -projectfiles -project=%UProjectPath% -game -rocket -progress