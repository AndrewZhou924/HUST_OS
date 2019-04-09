@Echo Off
Set /p yearmonthday=ÇëÊäÈë£º ³öÉúÄêÔÂÈÕ £¨ÀıÈç20000731£©£º

Set year=%yearmonthday:~0,4%
Set monthday=%yearmonthday:~4,4%
Set /a mod=%year%%%12

if %mod%==0 set /p="ÄãÊôºï£¬"<nul
if %mod%==1 set /p="ÄãÊô¼¦£¬"<nul
if %mod%==2 set /p="ÄãÊô¹·£¬"<nul
if %mod%==3 set /p="ÄãÊôÖí£¬"<nul
if %mod%==4 set /p="ÄãÊôÊó£¬"<nul
if %mod%==5 set /p="ÄãÊôÅ££¬"<nul
if %mod%==6 set /p="ÄãÊô»¢£¬"<nul
if %mod%==7 set /p="ÄãÊôÍÃ£¬"<nul
if %mod%==8 set /p="ÄãÊôÁú£¬"<nul
if %mod%==9 set /p="ÄãÊôÉß£¬"<nul
if %mod%==10 set /p="ÄãÊôÂí£¬"<nul
if %mod%==11 set /p="ÄãÊôÑò£¬"<nul
 
if "%monthday%" LEQ "0119" echo Ä§Ğ«×ù
if "%monthday%" GEQ "0120" if "%monthday%" LEQ "0218" echo Ë®Æ¿×ù
if "%monthday%" GEQ "0219" if "%monthday%" LEQ "0320" echo Ë«Óã×ù
if "%monthday%" GEQ "0321" if "%monthday%" LEQ "0419" echo °×Ñò×ù
if "%monthday%" GEQ "0420" if "%monthday%" LEQ "0520" echo ½ğÅ£×ù
if "%monthday%" GEQ "0521" if "%monthday%" LEQ "0621" echo Ë«×Ó×ù
if "%monthday%" GEQ "0622" if "%monthday%" LEQ "0722" echo ¾ŞĞ·×ù
if "%monthday%" GEQ "0723" if "%monthday%" LEQ "0822" echo Ê¨×Ó×ù
if "%monthday%" GEQ "0823" if "%monthday%" LEQ "0922" echo ´¦Å®×ù
if "%monthday%" GEQ "0923" if "%monthday%" LEQ "1023" echo Ìì³Ó×ù
if "%monthday%" GEQ "1024" if "%monthday%" LEQ "1122" echo ÌìĞ«×ù
if "%monthday%" GEQ "0321" if "%monthday%" LEQ "0419" echo °×Ñò×ù
if "%monthday%" GEQ "1222" if "%monthday%" LEQ "1231" echo Ä§Ğ«×ù
if "%monthday%" GEQ "1232" echo ÄúÊäÈëµÄÔÂ·İÓĞÎó

%0 :: ÓÃÓÚÑ­»·

Pause