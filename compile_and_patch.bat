cd template
call .\build.bat
cd ..
cd Calend
call .\build.bat
cd ..
.\ResPack -a out.res .\original_res\Mili_chaohu_11205_lite_packed.res .\template\template.elf .\Calend\Calend.elf
pause