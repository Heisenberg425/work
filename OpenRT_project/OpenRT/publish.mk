INSTALL_PATH := $(PROJECT_PATH)/publish/install-Packet
OPENRT_PATH := $(PROJECT_PATH)/publish/install-Packet/OpenRT-Packet

makePubDir:
ifneq ($(INSTALL_PATH), $(wildcard $(INSTALL_PATH)))
	mkdir $(INSTALL_PATH)
endif
ifneq ($(OPENRT_PATH), $(wildcard $(OPENRT_PATH)))
	mkdir $(OPENRT_PATH)
endif
ifneq ($(OPENRT_PATH)/arpSpoofDefense, $(wildcard $(OPENRT_PATH)/arpSpoofDefense))
	mkdir $(OPENRT_PATH)/arpSpoofDefense
endif
ifneq ($(OPENRT_PATH)/dhcpserver, $(wildcard $(OPENRT_PATH)/dhcpserver))
	mkdir $(OPENRT_PATH)/dhcpserver 
endif
ifneq ($(OPENRT_PATH)/domainAccess, $(wildcard $(OPENRT_PATH)/domainAccess))
	mkdir $(OPENRT_PATH)/domainAccess 
endif
ifneq ($(OPENRT_PATH)/etc, $(wildcard $(OPENRT_PATH)/etc))
	mkdir $(OPENRT_PATH)/etc 
endif
ifneq ($(OPENRT_PATH)/image/library/cextern, $(wildcard $(OPENRT_PATH)/image/library/cextern))
	mkdir -p $(OPENRT_PATH)/image/library/cextern
endif
ifneq ($(OPENRT_PATH)/image/library/libpcap, $(wildcard $(OPENRT_PATH)/image/library/libpcap))
	mkdir -p $(OPENRT_PATH)/image/library/libpcap
endif
ifneq ($(OPENRT_PATH)/image/library/libsylixos, $(wildcard $(OPENRT_PATH)/image/library/libsylixos))
	mkdir -p $(OPENRT_PATH)/image/library/libsylixos
endif
ifneq ($(OPENRT_PATH)/image/library/openssl, $(wildcard $(OPENRT_PATH)/image/library/openssl))
	mkdir -p $(OPENRT_PATH)/image/library/openssl
endif
ifneq ($(OPENRT_PATH)/image/library/readline, $(wildcard $(OPENRT_PATH)/image/library/readline))
	mkdir -p $(OPENRT_PATH)/image/library/readline
endif
ifneq ($(OPENRT_PATH)/image/library/sqlite3, $(wildcard $(OPENRT_PATH)/image/library/sqlite3))
	mkdir -p $(OPENRT_PATH)/image/library/sqlite3
endif
ifneq ($(OPENRT_PATH)/kidvpn, $(wildcard $(OPENRT_PATH)/kidvpn))
	mkdir $(OPENRT_PATH)/kidvpn 
endif
ifneq ($(OPENRT_PATH)/OpenRT, $(wildcard $(OPENRT_PATH)/OpenRT))
	mkdir $(OPENRT_PATH)/OpenRT 
endif
ifneq ($(OPENRT_PATH)/OpenRT/CommunicateServer, $(wildcard $(OPENRT_PATH)/OpenRT/CommunicateServer))
	mkdir $(OPENRT_PATH)/OpenRT/CommunicateServer 
endif
ifneq ($(OPENRT_PATH)/OpenRT/conf, $(wildcard $(OPENRT_PATH)/OpenRT/conf))
	mkdir $(OPENRT_PATH)/OpenRT/conf 
endif
ifneq ($(OPENRT_PATH)/OpenRT/OpenRTServer, $(wildcard $(OPENRT_PATH)/OpenRT/OpenRTServer))
	mkdir $(OPENRT_PATH)/OpenRT/OpenRTServer 
endif
ifneq ($(OPENRT_PATH)/OpenRT/WebServer, $(wildcard $(OPENRT_PATH)/OpenRT/WebServer))
	mkdir $(OPENRT_PATH)/OpenRT/WebServer 
endif
ifneq ($(OPENRT_PATH)/quagga, $(wildcard $(OPENRT_PATH)/quagga))
	mkdir $(OPENRT_PATH)/quagga 
endif
ifneq ($(OPENRT_PATH)/quagga/bin, $(wildcard $(OPENRT_PATH)/quagga/bin))
	mkdir $(OPENRT_PATH)/quagga/bin 
endif
ifneq ($(OPENRT_PATH)/quagga/conf, $(wildcard $(OPENRT_PATH)/quagga/conf))
	mkdir $(OPENRT_PATH)/quagga/conf 
endif
ifneq ($(OPENRT_PATH)/quagga/so, $(wildcard $(OPENRT_PATH)/quagga/so))
	mkdir $(OPENRT_PATH)/quagga/so 
endif
ifneq ($(OPENRT_PATH)/tmstat, $(wildcard $(OPENRT_PATH)/tmstat))
	mkdir $(OPENRT_PATH)/tmstat 
endif
ifneq ($(OPENRT_PATH)/tools, $(wildcard $(OPENRT_PATH)/tools))
	mkdir $(OPENRT_PATH)/tools 
endif
ifneq ($(OPENRT_PATH)/udial, $(wildcard $(OPENRT_PATH)/udial))
	mkdir $(OPENRT_PATH)/udial 
endif

OUTDIR := Release
copyPubFile:
	-cp -r $(PROJECT_PATH)/tools/others/arpSpoofDefense/$(OUTDIR)/arpSpoofDefense.ko   $(OPENRT_PATH)/arpSpoofDefense/arpSpoofDefense.ko
	-cp -r $(PROJECT_PATH)/tools/others/arpSpoofDefense/sample/arpSpoofDefense.ini     $(OPENRT_PATH)/arpSpoofDefense/arpSpoofDefense.ini
	-cp -r $(PROJECT_PATH)/tools/others/dhcpserver/$(OUTDIR)/dhcpserver                $(OPENRT_PATH)/dhcpserver/dhcpserver
	-cp -r $(PROJECT_PATH)/tools/others/dhcpserver/sample/dhcpd.lease                  $(OPENRT_PATH)/dhcpserver/dhcpd.lease
	-cp -r $(PROJECT_PATH)/tools/others/dhcpserver/sample/dhcpdconfig.ini              $(OPENRT_PATH)/dhcpserver/dhcpdconfig.ini
	-cp -r $(PROJECT_PATH)/tools/others/domainAccess/$(OUTDIR)/domainAccess.ko         $(OPENRT_PATH)/domainAccess/domainAccess.ko
	-cp -r $(PROJECT_PATH)/publish/config/ifparam.ini                                  $(OPENRT_PATH)/etc/ifparam.ini
	-cp -r $(PROJECT_PATH)/publish/config/startup.sh                                   $(OPENRT_PATH)/etc/startup.sh
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/lib4758cca.so                $(OPENRT_PATH)/image/library/openssl/lib4758cca.so 
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libaep.so                    $(OPENRT_PATH)/image/library/openssl/libaep.so     
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libatalla.so                 $(OPENRT_PATH)/image/library/openssl/libatalla.so  
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libcapi.so                   $(OPENRT_PATH)/image/library/openssl/libcapi.so    
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libchil.so                   $(OPENRT_PATH)/image/library/openssl/libchil.so    
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libcrypto.so                 $(OPENRT_PATH)/image/library/openssl/libcrypto.so  
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libcswift.so                 $(OPENRT_PATH)/image/library/openssl/libcswift.so  
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libgmp.so                    $(OPENRT_PATH)/image/library/openssl/libgmp.so     
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libnuron.so                  $(OPENRT_PATH)/image/library/openssl/libnuron.so   
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libpadlock.so                $(OPENRT_PATH)/image/library/openssl/libpadlock.so 
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libssl.so                    $(OPENRT_PATH)/image/library/openssl/libssl.so     
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libsureware.so               $(OPENRT_PATH)/image/library/openssl/libsureware.so
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/libubsec.so                  $(OPENRT_PATH)/image/library/openssl/libubsec.so   
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/openssl                      $(OPENRT_PATH)/image/library/openssl/openssl       
	-cp -r $(PROJECT_PATH)/tools/others/openssl/$(OUTDIR)/openssl.cnf                  $(OPENRT_PATH)/image/library/openssl/openssl.cnf
	-cp -r $(PROJECT_PATH)/tools/others/libpcap/$(OUTDIR)/libpcap.so                   $(OPENRT_PATH)/image/library/libpcap/libpcap.so
	-cp -r $(PROJECT_PATH)/tools/others/libreadline/$(OUTDIR)/libreadline.so           $(OPENRT_PATH)/image/library/readline/libreadline.so
	-cp -r $(PROJECT_PATH)/tools/others/libsqlite3/$(OUTDIR)/libsqlite3.so             $(OPENRT_PATH)/image/library/sqlite3/libsqlite3.so
	-cp -r $(PROJECT_PATH)/tools/others/libsqlite3/$(OUTDIR)/sqlite3                   $(OPENRT_PATH)/image/library/sqlite3/sqlite3
	-cp -r $(PROJECT_PATH)/tools/others/kidvpn/$(OUTDIR)/kidvpn	                       $(OPENRT_PATH)/kidvpn/kidvpn
	-cp -r $(PROJECT_PATH)/src/OpenRTServer/src/kidvpn/ipPos.datx	                   $(OPENRT_PATH)/kidvpn/ipPos.datx
	-cp -r $(PROJECT_PATH)/src/CommunicateServer/libcomm/$(OUTDIR)/strip/libcomm.so              $(OPENRT_PATH)/OpenRT/CommunicateServer/libcomm.so
	-cp -r $(PROJECT_PATH)/src/CommunicateServer/libnethandle/$(OUTDIR)/strip/libnethandle.so    $(OPENRT_PATH)/OpenRT/CommunicateServer/libnethandle.so	
	-cp -r $(PROJECT_PATH)/publish/config/OpenRTCFGDB                                            $(OPENRT_PATH)/OpenRT/conf/OpenRTCFGDB
	-cp -r $(PROJECT_PATH)/src/OpenRTServer/$(OUTDIR)/strip/OpenRTServer                         $(OPENRT_PATH)/OpenRT/OpenRTServer/OpenRTServer
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/$(OUTDIR)/strip/goahead                         $(OPENRT_PATH)/OpenRT/WebServer/goahead
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/$(OUTDIR)/strip/gopass                          $(OPENRT_PATH)/OpenRT/WebServer/gopass
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/$(OUTDIR)/strip/webcomp                         $(OPENRT_PATH)/OpenRT/WebServer/webcomp
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/$(OUTDIR)/strip/libgoahead.so                   $(OPENRT_PATH)/OpenRT/WebServer/libgoahead.so
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/embedthis-goahead/src/auth.txt                  $(OPENRT_PATH)/OpenRT/WebServer/auth.txt
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/embedthis-goahead/src/route.txt                 $(OPENRT_PATH)/OpenRT/WebServer/route.txt
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/embedthis-goahead/src/self.crt                  $(OPENRT_PATH)/OpenRT/WebServer/self.crt
	-cp -r $(PROJECT_PATH)/src/WebServer/goahead/embedthis-goahead/src/self.key                  $(OPENRT_PATH)/OpenRT/WebServer/self.key
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/bgpd                                    $(OPENRT_PATH)/quagga/bin/bgpd  
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/isisd                                   $(OPENRT_PATH)/quagga/bin/isisd 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/ospfd                                   $(OPENRT_PATH)/quagga/bin/ospfd 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/pimd                                    $(OPENRT_PATH)/quagga/bin/pimd  
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/ripd                                    $(OPENRT_PATH)/quagga/bin/ripd  
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/ripngd                                  $(OPENRT_PATH)/quagga/bin/ripngd
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/vtysh                                   $(OPENRT_PATH)/quagga/bin/vtysh 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/zebra                                   $(OPENRT_PATH)/quagga/bin/zebra 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/$(OUTDIR)/libzebra.so                             $(OPENRT_PATH)/quagga/so/libzebra.so
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/bgpd.conf                          $(OPENRT_PATH)/quagga/conf/bgpd.conf  
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/isisd.conf                         $(OPENRT_PATH)/quagga/conf/isisd.conf 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/ospf6d.conf                        $(OPENRT_PATH)/quagga/conf/ospf6d.conf
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/ospfd.conf                         $(OPENRT_PATH)/quagga/conf/ospfd.conf 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/pimd.conf                          $(OPENRT_PATH)/quagga/conf/pimd.conf  
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/ripd.conf                          $(OPENRT_PATH)/quagga/conf/ripd.conf  
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/ripngd.conf                        $(OPENRT_PATH)/quagga/conf/ripngd.conf
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/vtysh.conf                         $(OPENRT_PATH)/quagga/conf/vtysh.conf 
	-cp -r $(PROJECT_PATH)/tools/others/quagga/configurations/zebra.conf                         $(OPENRT_PATH)/quagga/conf/zebra.conf 
	-cp -r $(PROJECT_PATH)/tools/others/tmstat/$(OUTDIR)/trafficStats.ko                         $(OPENRT_PATH)/tmstat/trafficStats.ko
	-cp -r $(PROJECT_PATH)/tools/others/rm_tool/$(OUTDIR)/rm_tool                                $(OPENRT_PATH)/tools/rm_tool
	-cp -r $(PROJECT_PATH)/tools/others/udial/$(OUTDIR)/udial                                    $(OPENRT_PATH)/udial/udial

delOpenRTDir:
	rm -rf $(PROJECT_PATH)/OpenRT-Packet

tarPublish:
	tools/tarTools/tar.exe -cvf publish/install-Packet/OpenRT-Packet/web.tar.gz  web/*
	make delOpenRTDir
	@cp -r  $(PROJECT_PATH)/publish/install-Packet/OpenRT-Packet/                $(PROJECT_PATH)/
	tools/tarTools/tar.exe -cvf publish/install-Packet/OpenRT-Packet.tar.gz      OpenRT-Packet/*
	make delOpenRTDir
	@cp -r $(PROJECT_PATH)/publish/config/setup.lua                              $(PROJECT_PATH)/publish/install-Packet/setup.lua
	
publishClean:
	-rm -rf publish/install-Packet

#*********************************************************************************************************
# End
#*********************************************************************************************************





