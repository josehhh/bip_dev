ECHO const char *words[] = { > "test_file.txt"


::Get number of lines in csv
:: Add number of lines in csv
SET /A Lines = 0 
for /f  %%a in (csv_of_words.csv) do (set /a Lines+=1)

:: Read csv to array
@echo off
setlocal enabledelayedexpansion

set var1=0

for /F "tokens=1* delims=," %%a in (csv_of_words.csv) do (
    set var2=0
    set array[!var1!][!var2!]=%%a
    set /a var2+=1
    set array[!var1!][!var2!]=%%b
    set /a var1+=1
)

:: print all values in csv
Setlocal EnableDelayedExpansion
echo off

set /a Lines-=1
FOR /L %%X IN (0,1,%Lines%) DO (
   echo "!array[%%X][0]!","!array[%%X][1]!", >> "test_file.txt"
)
set /a Lines+=1
endlocal



ECHO  }; >> "test_file.txt"

echo|set /p="const int N_WORDS=" >> "test_file.txt"






echo|set /p=%Lines% >> "test_file.txt"

ECHO ; >> "test_file.txt"