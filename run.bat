@echo off
cd /d "C:\Users\user\Documents\MyCode\VisualStudio\meta_algo\NSGA_II\x64\Release"

:: 設定參數
set run=1
set func_id=1
set mnfes=25000
set dim=1
set pop_size=100
set CR=0.9
set MR=0.1

:: ========== f1 ==========
echo Running f1
NSGA_II.exe %run% %func_id% %mnfes% %dim% %pop_size% %CR% %MR%

pause