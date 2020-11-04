1、使用help tm查看流量统计帮助
2、add, delete statistics item(tm_type should be one of term,line,app,lineapp),and show traffic statistics.
eg: tm add term 192.168.1.100 增加一个终端用户流量监控
    tm add line 192.168.1.100 en4  增加一个路由器上网络接口流量监控
    tm show term [ip] 查看终端用户流量使用情况
    tm show line [ip] 查看路由器上网口流量使用情况
    tm show app  [ip] 查看每个app流量使用情况
    tm show lineapp [ip] 查看路由器上网口每个app流量使用情况
    tm disable 去使能流量统计功能
	
tm add | del tm_type ip [mac] [ifname] [tcp | udp] [port]. 
tm show [tm_type]. 
tm disable | enable. 
