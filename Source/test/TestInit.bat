@echo off

echo -------------------------^^*_*^^-------------------------------------------
echo If you use qmake project to build thaf.lib,
echo Please setting the build dirrectory to point to "bin.noversion/Libs/[debug|release]" dirrectory
echo Please make sure all the output binary will be placed on this dirrectory "bin.noversion" as well
echo -------------------------^^*_*^^-------------------------------------------
if not exist bin.noversion mkdir bin.noversion
if not exist bin.noversion\Libs mkdir bin.noversion\Libs

cmd /K
