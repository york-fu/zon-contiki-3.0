
::�򿪻���
@echo on

::ɾ��*.dep�ļ�
del=/s /a /f "*.tmp"
del=/s /a /f "*.bak"
del=/s /a /f "*.dep"

::ɾ�� "Exe" "List" "Obj""settings"�ļ���
for /f "delims=" %%i in ('dir /ad /b /s "Exe" "List" "Obj" "settings"') do (
   rd /s /q "%%i"
)

::ɾ�����ļ���
for /f "tokens=*" %%i in ('dir/s/b/ad^|sort /r') do rd "%%i"

exit