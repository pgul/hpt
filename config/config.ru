# main fidoconfig file
#
name Dummy point station
sysop Ivan Durak
location Moscow, Russia
address 2:5020/9999.99
# ��������
# �������� ��� ����������� ������
inbound /home/username/fido/inbound/insecure
# ���������� - ��� ��������� ������
protinbound /home/username/fido/inbound
# ��������� �����
outbound /home/username/fido/outbound
# ��������� �������� ��� �������� � �������
tempinbound /home/username/fido/tmp.inbound
tempoutbound /home/username/fido/tmp.outbound
# ���; ���� ���-�� �� �������� - ������ �����
logfiledir /home/username/fido/log
# ��������
dupehistorydir /home/username/fido/dupebase
# ������� ������ �����
nodelistdir /home/username/fido/etc
# ���� ���������
msgbasedir /home/username/fido/msgbase
# ���������� ������ �������, ����� �������������� ������� �����������
echotosslog /home/username/fido/log/toss.log
importlog /home/username/fido/log/import.log

# ������� ������� ������� ������ ��� ����� ���������
linkwithimportlog kill
# ��������� �������� ��� ������
separatebundles yes
# �� ������� ������ ��
disablepid yes
disabletid yes
# Perl-�����������; ��������� �������� - � ����� perlhooks
#hptperlfile /home/username/fido/lib/hptfunctions.pl
# ��������� �����������; �������������� ������� - � ����� packers
pack zip zip -9 -j -q $a $f
unpack "unzip -j -Loqq $a $f -d $p" 0 504b0304

# ���� ����� ������������ ��������� ��� � ������������
carbonto Ivan Durak
carboncopy PERSONAL.MAIL

# ��������� ��� ������� ����� ����������� � ������������ �����
robotsarea NETMAIL

# ���������, ����� ��� ���� �������
robot default
# ������� ��������� � ���������
killrequests yes
# �������� ��� ��������� � ��������
reportsattr loc pvt k/s npd

# ��������� ������ ��� ����� ������
robot areafix
fromname Areafix robot
robotorigin Areafix robot

# ���������, ����� ��� ���� ������; ��������� - � ����� links
linkdefaults begin
# ���� ����� ������� �� ���������� ������ - � ��� ����� �� ���� ������
allowemptypktpwd secure
# ��������� �� ���������; ��� ��������� ������ ����� ��������� ������
packer zip
# �������������� �������� �����������;
# ������������� �������� ����� off, � ��� ��������� ������ �������� ����
areafixautocreate off
# ��������� ������������
areafixautocreatedefaults -b squish -dupecheck del -dupehistory 14
# ���� � ���������� �����������
areafixautocreatefile /home/username/fido/etc/areas
# �� ��������� ���������� ������ ����, �� ��������� �������� ����������
echomailflavour direct
# ��������� ��������� �������� � �������
forwardrequests off
linkdefaults end

# �������� ������
include /home/username/fido/etc/links
# � ������������� ��������
include /home/username/fido/etc/route

# �������� �����������
# �������� ����������� ����������� - ������ �������������� �����������
netmailarea NETMAIL       /home/username/fido/msgbase/netmail       -b squish
# ���� ������ ���������, ������� ������ �� ����� ����������
badarea     BAD           /home/username/fido/msgbase/bad           -b squish
# ���� ������ ���� ��� ������������� dupecheck move
dupearea    DUPE          /home/username/fido/msgbase/dupe          -b squish
# ���� ������������� ����� ���������, ������������ ���
localarea   PERSONAL.MAIL /home/username/fido/msgbase/personal.mail -b squish

# ��������������
include /home/username/fido/etc/areas
