set MAFIAPATH=G:\Steam\steamapps\common\Mafia III
set mypath=C:\Users\Martin\Desktop\Mafia3ScriptHook
copy /Y /B "%mypath%\build\Release\M3ScriptHook.dll" "%MAFIAPATH%\M3ScriptHook.dll" /B 
copy /Y /B "%mypath%\build\Release\plugins\ExampleDLLPlugin.dll" "%MAFIAPATH%\plugins\ExampleDLLPlugin.dll" /B 
copy /Y /B "%mypath%\build\Release\M3ScriptHookLauncher.exe" "%MAFIAPATH%\M3ScriptHookLauncher.exe" /B