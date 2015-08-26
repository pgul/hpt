@echo off
setlocal enableextensions enabledelayedexpansion

rem �஭�� � areastat �� �������� ����୨��� HPT.
rem �� ᯨ᪠ �宠਩ HPT ᮧ���� ���䨣 areastat.
rem ���४⭮ �ய�᪠�� ������ ��, � ����� ᯮ�몠���� areastat. ����᪠��
rem ᠬ areastat � �� �⮣�� ��� ࠡ��� ��ந� ��ᨢ�� ⠡���� ����⨪�
rem 宦����� �� �� 㪠����� ��ਮ�, ������ � ����� ��⮬ � ���, � ⠪ �� ᯨ᮪
rem ��, � ����� �� 㪠����� ��ਮ� �� �뫮 �� ������ ���쬠.
rem
rem C ����ᠬ� �������� � ����� Stas Mishchenkov 2:460/58.0
rem ������࠭���� As Is, ᢮����� � ��ᯫ�⭮. ��� �����, �ࠢ�� �� ���஢�.
rem ���஢��� � ᢮�� �ਯ�� �����, �� ����⥫쭮 � 㪠������ ���筨��. ;)
rem 
rem �������� ��� ��ࠬ���: ���� (�� ��易⥫��) - ��ਮ�, �� �����
rem �������� ����⨪� � �ଠ� ���䨣� areastat. �� 㬮�砭�� - 1 ����.
rem ���� ��ࠬ��஬ ����� ���� 䫠� "dead" - � �⮬ ��砥 �㤥� ��ࠢ��� � ���
rem ᯨ᮪ ������ ��.

REM ������ ��� 䠩�� ���䨣��樨 HPT, � ���஬ ���ᠭ� �宠ਨ.
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ����, ���, ��७��).
rem ���ࠧ㬥������, �� � ��� �ࠧ� �� ������ 䠩�� �宮����� 㪠��� ⨯ ����!
set areascfg=D:\Fido\hpt\areas.cfg

rem ������ ��� 䠩��, � ���஬ ������ ���䨣 ��� areastat.
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ����, ���, ��७��).
set areaSTATcfg=D:\Fido\hpt\areastat.cfg

rem ����� ���� � ���४�ਨ, � ���ன ᮧ������ ����⨪�.
set StatDIR=d:\fido\logs\stat

rem ������ ��� �ᯮ��塞��� 䠩�� �ॠ���.
set areastatexe=D:\Fido\hpt\bin\areastat.exe

rem ��� �ᯮ��塞��� 䠩�� HPT.
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ���� ���, ��७��).
set hptexe=D:\Fido\hpt\bin\hpt.exe

rem ��� 䠩�� ���䨣��樨 HPT (fidoconfig).
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ���� ���, ��७��).
set hptcfg=D:\Fido\hpt\hpt.cfg


rem ��易⥫쭮 �� � ����窠�.
rem �� ������ ����� ������...
set NameFrom="Evil Robot"
rem �㤠 ������ ����⨪�
set RobotEchoArea="crimea.robots"
set Subj="����⨪� 宦����� �宪���७権"
set TEARLINE="Evil Robot"
set Origin="Lame Users Breading, Crimea."


rem Period for statistics
rem Stat_Period [m][w]<age>
rem Examples:
rem Stat_Period 60 - statistics for 60 days
rem Stat_Period w2 - statistics for 2 weeks
rem Stat_Period m1 - statistics for 1 month
if _%~1_==__ ( set statperiod=1) else ( set statperiod=%~1)

del /f/q %areaSTATcfg%
rem                  a b c d e f
FOR /F "eol=# tokens=1,2,3,4,5* delims= " %%a in (%areascfg%) do if /i not %%c==passthrough echo Area %%b %%e %%c %%b.stt>>%areaSTATcfg%
rem ;  Area <name> <type> <path> <out_file>
echo Stat_Period !statperiod!>>%areaSTATcfg%

del /f/q %StatDIR%\*
%StatDIR:~0,2%
cd %StatDIR%
%areastatexe% %areaSTATcfg%

set outfile=%StatDIR%\stat.tpl
set deadtpl=%StatDIR%\dead.tpl

set headertop=�����������������������������������������������������������������������������Ŀ
set    header=�                     ����⨪� 宦����� �宪���७権.                     �
set   header1=�����������������������������������������������������������������������������Ĵ
set   header2=�              EchoArea                                      �  Msgs  � Users �
set   header3=�����������������������������������������������������������������������������Ĵ
set    footer=�������������������������������������������������������������������������������


echo RealName: %NameFrom%>%outfile%
echo Created %DATE% %TIME:~0,-3%>>%outfile%
echo %headertop%>>%outfile%
echo %header%>>%outfile%
echo %header1%>>%outfile%
echo %header2%>>%outfile%
echo %header3%>>%outfile%
echo RealName: %NameFrom%>%deadtpl%
echo Statperiod: %statperiod%>>%deadtpl%
echo  * Dead Areas *>>%deadtpl%
echo.>>%deadtpl%

Echo C��⠢�塞 ᯨ᮪ 䠩���...
set /a stem.0=0
set /a fn=1
for %%i in (%statdir%\*.stt) do set stem.!fn!=%%i&set /a stem.0=!fn!&set /a fn+=1
echo �ᥣ� ������� 䠩���: %stem.0%.

rem �᫨ ��祬�-� 䠩�� ����⨪� �� �������, � � ������ ����� ��祣�.
if %stem.0%==0 exit

set /a fn=1
set /a grandtotal=0
:S1

   FOR /F "eol=; tokens=1,2,3* delims=: " %%i in (!stem.%fn%!) do if /i %%i==Area (
       set stem.!fn!.area=%%j
      ) else if /i %%i==Messages (
       set /a stem.!fn!.Messages=%%j
      ) else if /i %%i==Total (
       set /a stem.!fn!.users=%%k
      ) else if /i %%i==Period (
       set stem.!fn!.Period=%%j %%k %%l
      )

   rem ����窠 �� �ॠ��� � ���⮩ �宩.
   if not defined stem.!fn!.area set /a fn+=1&goto S1

   set areaname=!stem.%fn%.area!                                                          ;
   set msgs=          !stem.%fn%.Messages! 
   set tusers=          !stem.%fn%.users! 
   set str=�!areaname:~0,60!�!msgs:~-8!�!tusers:~-7!�
   if !stem.%fn%.Messages! NEQ 0 (
      echo !str!
      echo !str!>>%outfile%
      echo %header3%>>%outfile%
      set /a grandtotal=!grandtotal!+!stem.%fn%.Messages!
   ) else echo !areaname:~0,40!>>%deadtpl%

   if !fn!==!stem.0! goto E1
   set /a fn+=1
   goto S1
:E1
   set msgs=          !grandtotal! 
   set FootStr=�                                                     �ᥣ�: �!msgs:~-8!�       �
   echo %FootStr%>>%outfile%
   echo %footer%>>%outfile%
   echo %FootStr%
   echo %footer%

if /i .%~2.==.dead. (
  %hptexe% -c %hptcfg% post -nf %NameFrom% -s " * Dead Areas *" -e %RobotEchoArea% -z %TEARLINE% -o %Origin% -x -f loc %deadtpl%
) else %hptexe% -c %hptcfg% post -nf %NameFrom% -s %Subj% -e %RobotEchoArea% -z %TEARLINE% -o %Origin% -x -f loc %outfile%

exit

