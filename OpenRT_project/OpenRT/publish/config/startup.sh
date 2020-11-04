shfile /etc/usrFroScript.sh
sleep 3
modulereg /lib/modules/arpSpoofDefense.ko
modulereg /lib/modules/trafficStats.ko
modulereg /lib/modules/domainAccess.ko

gadd quagga 1000
uadd quagga 123 1 1000 1000 "router user" /home/quagga
mkdir /var/run

rtmssadj 1   
ipforward 1 
natipfrag UDP 1 
natipfrag ICMP 1

sleep 1
/apps/OpenRTServer/OpenRTServer&

sleep 3                    
shfile /etc/usrAftScript.sh


