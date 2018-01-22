echo 请确保你已经填好相关配置文件！
pause
call D:\QT\RUN\5.4\mingw491_32\bin\qtenv2.bat
pause
cd /d %~dp0
windeployqt packages/com.warofplanes.root/data/warofplanes.exe
pause
D:\QT\QtIFW2.0.0\bin\binarycreator --offline-only -c config/config.xml -p packages installer
pause