@echo off

:: Setup, change these as necessary
set PathToEngine=C:\Unreal\EngineInstalls\UE_4.24
set ProjectName=CoopGame
:: End Setup

if exist %~dp0\.vs (
	@echo Removing .vs folder
	RD /s /q %~dp0\.vs
) else (
	@echo Cannot Remove .vs Directory - No .vs Directory
)

if exist %ProjectName%.sln (
	@echo Removing vs solution
	del /q %ProjectName%.sln
) else ( 
	@echo Count Not Delete .sln - %ProjectName%.sln Does Not Exist.
)

:: Note that I am not simply removing the entirety of the build/intermediate directories
:: as in some cases it is useful to symbolic link folders. By deleting only the contents of the 
:: folders, we maintain the link

if exist %~dp0\Binaries (
	@echo Removing Binaries
	cd %~dp0\Binaries
	for /r %%a in (*) do (
		del /q /s "%%a"
	)
	for /d %%a in (*) do (
		rmdir /q /s "%%a"
	)
) else (
	@echo Cannot Clear Binaries - No Binaries Directories
)

if exist %~dp0\Intermediate (
	@echo Removing Intermediate
	cd %~dp0\Intermediate
	for /r %%a in (*) do (
		del /q /s "%%a"
	)
	for /d %%a in (*) do (
		rmdir /q /s "%%a"
	)
) else (
	@echo Cannot Clear Intermediates - No Intermiedates Directories
)

set UProjectPath=%~dp0%ProjectName%.uproject
if exist %UProjectPath% (
	@echo Generating New Project Files
	"%PathToEngine%\Engine\Binaries\DotNET\UnrealBuildTool.exe" -projectfiles -project=%UProjectPath% -game -rocket -progress
) else (
	@echo %UProjectPath% does not exist, cannot regenerate project files.
)

pause