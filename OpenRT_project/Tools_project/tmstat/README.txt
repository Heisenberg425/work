1��ʹ��help tm�鿴����ͳ�ư���
2��add, delete statistics item(tm_type should be one of term,line,app,lineapp),and show traffic statistics.
eg: tm add term 192.168.1.100 ����һ���ն��û��������
    tm add line 192.168.1.100 en4  ����һ��·����������ӿ��������
    tm show term [ip] �鿴�ն��û�����ʹ�����
    tm show line [ip] �鿴·��������������ʹ�����
    tm show app  [ip] �鿴ÿ��app����ʹ�����
    tm show lineapp [ip] �鿴·����������ÿ��app����ʹ�����
    tm disable ȥʹ������ͳ�ƹ���
	
tm add | del tm_type ip [mac] [ifname] [tcp | udp] [port]. 
tm show [tm_type]. 
tm disable | enable. 
