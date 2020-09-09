cd template
call .\build.bat
cd ..
cd WordLearner
call .\build.bat
cd ..
pause
cd Calend
call .\build.bat
cd ..
.\ResPack -a out_light.res .\original_res\Mili_chaohu_11205_lite_packed.res .\template\template.elf .\Calend\Calend.elf .\WordLearner\WordLearner.elf
.\ResPack -a out.res .\original_res\MNVolkov_BipOS_0.5.2_Apps_MNVolkov_BipOS_0.5.2_Apps_Mili_chaohu.res .\template\template.elf .\Calend\Calend.elf .\WordLearner\WordLearner.elf
pause