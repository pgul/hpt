#!/usr/bin/perl
#
 
# makeavl.pl&readlist.pm by Alexander Reznikov 2:4600/220@fidonet, 
#                                              99:2003/110@webnet, 
#                                              homebrewer@yandex.ru
#
#
# ��� �ਯ� �।�����祭 ��� ᮧ����� ������� ᯨ᪠ ��� ��, ����㯭��
# �� 㧫�. � ����⢥ ��室��� ������ �ᯮ������� ᯨ᪨ �� �������� � �ଠ�
# EchoList. ����� ��樮���쭮 ����� ���� �ᯮ�짮��� ᢮� ᮡ�⢥���� �宫���
# � ⮬ �� �ଠ�. � ��砥 hpt ��� ����� ᮧ���� �� ����� �ਯ� 
# fconf2na.pl. ����� �ਯ� ��⠥��� ���� ���ᠭ�� �� �� echolist.txt 
# (䠩��� ECHOLIST) � echo5020.lst, �᫨ ��� ���� � ⥪�饩 ��४�ਨ.

# ����騩 �宫��� � �ଠ� Echolist,
# �᫨ �� ����� (���������஢��) - �� �ᯮ������
# ��� ����� ������� �� ����� fidoconfig/fconf2na.pl
$echolist = 'echolist.fe';

# ��� १������饣� ᯨ᪠ �� "��� �㯮�"
$avlname = '11f800dc.fwd';

# ���᮪ avail-䠩��� � 䮬�� Echolist, �� ������ �ନ����� १������騩 
# ᯨ᮪
@fwdlists = ('fwd126.txt', 'fwd113.txt', 'fwd103.txt');

#########
use readlist;
InitEchoList();

read_echolist($echolist) if (defined $echolist)&&($echolist ne '');

foreach $i (@fwdlists)
{
 read_echolist($i);
}

open FILE, ">$avlname";

foreach $i (sort keys(%echo))
{
 $descr = GetEchoListDescr($i) || $echo{$i} || '';
# $descr = $echo{$i} if length($descr)==0;

 $descr =~ tr/�/H/;

 print FILE "$i".(length($descr)>0? " $descr": '')."\n";
}

close(FILE);

sub read_echolist
{
 my $filename = shift;
 if (!open FILE, "<$filename")
 {
  warn("Can not open \'$filename\' ($!)\n");
  return 0; 
 }

 my ($echoid, $descr);

 while (<FILE>)
 {
  chomp;

  if (/^([^ ]+)\s*\"?(.*?)\"?$/)
  {
   $echoid = uc($1);
   $descr = $2;

   $echo{$echoid} = $descr if (!exists $echo{$echoid})||(length($echo{$echoid})==0);
  }
 }
 close(FILE);
 return 1;
}
