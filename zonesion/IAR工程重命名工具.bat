@echo off
@title IAR��������������1.0

setlocal enabledelayedexpansion
setlocal EnableExtensions
for /f %%i in ('dir /b *.eww') do (
set a=%%~ni
)
echo.  ��ǰ������Ϊ��%a%
echo.  �������µĹ�����
set /p newfile=
echo.  ���ڽ��й��������������Ե�...

for /f "tokens=* delims=��" %%l in ('type %a%.eww') do (
set line=%%l
set line=!line:%a%=%newfile%!
echo !line!>>%newfile%.eww
)

for /f "tokens=* delims=��" %%l in ('type %a%.dep') do (
set line=%%l
set line=!line:%a%=%newfile%!
echo !line!>>%newfile%.dep
)

for /f "tokens=* delims=��" %%l in ('type %a%.ewd') do (
set line=%%l
set line=!line:%a%=%newfile%!
echo !line!>>%newfile%.ewd
)

for /f "tokens=* delims=��" %%l in ('type %a%.ewp') do (
set line=%%l
set line=!line:%a%=%newfile%!
echo !line!>>%newfile%.ewp
)

for /f "tokens=* delims=��" %%l in ('type %a%.ewt') do (
set line=%%l
set line=!line:%a%=%newfile%!
echo !line!>>%newfile%.ewt
)

del %a%.*

echo
exit