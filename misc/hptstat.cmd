@echo off
rem ����� �ਯ� ᮧ���� ⠡���� � ����⨪�� 宦����� �� �� ���� HPT.
rem �� �ᥬ ����ᠬ �������� � ����� Stas Mischenkov 2:460/58.0
rem ������࠭���� As Is, ᢮����� � ��ᯫ�⭮. ��� �����, �ࠢ�� �� ���஢�.
rem ���஢��� � ᢮�� �ਯ� �����, ����⥫쭮 � 㪠������ ���筨��. ;)

setlocal enableextensions enabledelayedexpansion

rem ��� 䠩�� ���� HPT.
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ���� ���, ��७��).
set hptlog=D:\fido\logs\hpt.log

rem ��� 䠩��, � ���஬ �㤥� ᮧ���� ⠡��窠 ����⨪�.
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ���� ���, ��७��).
set hptstat=D:\fido\logs\hptstat.tpl

rem ��� �ᯮ��塞��� 䠩�� HPT.
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ���� ���, ��७��).
set hptexe=D:\Fido\hpt\bin\hpt.exe

rem ��� 䠩�� ���䨣��樨 HPT (fidoconfig).
rem �ࠩ�� ४���������� 㪠�뢠�� ������ ��� 䠩�� (���, ���� ���, ��७��).
set hptcfg=D:\Fido\hpt\hpt.cfg

rem ��易⥫쭮 ��� ����祪.
set Subj=����⨪� 宦����� �宪���७権

rem ��易⥫쭮 �� � ����窠�.
set NameFrom="Evil Robot"
set RobotEchoArea="crimea.robots"
set TEARLINE="Evil Robot"
set Origin="Lame Users Breading, Crimea."


set headertop=�����������������������������������������������������������������������������Ŀ
set    header=�                     ����⨪� 宦����� �宪���७権                      �
set   header1=�����������������������������������������������������������������������������Ĵ
set   header2=�    EchoArea                                                        �  Msgs  �
set   header3=�����������������������������������������������������������������������������Ĵ
set    footer=�������������������������������������������������������������������������������

echo !headertop!>!hptstat!
echo !header!>>!hptstat!
echo !header1!>>!hptstat!
echo !header2!>>!hptstat!


set /a ww=0
set /a areaslist.0=0
rem                  a b c d e f g h i j 
FOR /F "eol=; tokens=1,2,3,4,5,6,7,8,9,* delims== " %%a in (%hptlog%) do if /i _%%d_==_area_ (
     if /i _%%c_==_echo_ (
        set t=%%e
        set t=!t:-=�!
        if not defined msgs.!t! (
           set /a ww=!ww!+1
           set /a msgs.!t!=%%g
           set areaslist.!ww!=!t!
        ) else (
           set /a msgs.!t!=msgs.!t!+%%g
          )
     )
    ) else if /i _%%a_==_----------_ (
            if not defined startperiod set startperiod=%%b %%c %%d %%e
            set endperiod=%%b %%c %%d %%e
           )
set /a areaslist.0=!ww!

echo * !areaslist.0! EchoAreas found.

set /a ww=1
set /a grandtotal=0
:s1

   set mm=!areaslist.%ww%!
   set areatag=!mm:�=-!

   set /a grandtotal=!grandtotal!+msgs.!mm!
   set areaname=� !areatag!                                                                ;
   set msgs=             !msgs.%mm%! �
   set str=!areaname:~0,69!�!msgs:~-9!

   echo !str!
   echo !header3!>>!hptstat!
   echo !str!>>!hptstat!

   if !ww!==%areaslist.0% goto e1
   set /a ww=!ww!+1
   goto s1
:e1
set msgs=             !grandtotal!
set str=�                             (!startperiod:,= !- !endperiod:,=) !�ᥣ�: �!msgs:~-7! �
echo !header3!>>!hptstat!
echo !str!>>!hptstat!
echo !footer!>>!hptstat!

%hptexe% -c %hptcfg% post -nf %NameFrom% -s "%Subj% (!startperiod:,= !- !endperiod:,=)!" -e %RobotEchoArea% -z %TEARLINE% -o %Origin% -x -f loc %hptstat%

exit
