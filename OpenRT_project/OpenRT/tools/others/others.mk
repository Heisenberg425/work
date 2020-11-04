mkdir:
ifneq (./arpSpoofDefense/$(OUTDIR), $(wildcard ./arpSpoofDefense/$(OUTDIR)))
	mkdir ./arpSpoofDefense/$(OUTDIR) 
endif
ifneq (./dhcpserver/$(OUTDIR), $(wildcard ./dhcpserver/$(OUTDIR)))
	mkdir ./dhcpserver/$(OUTDIR) 
endif
ifneq (./domainAccess/$(OUTDIR), $(wildcard ./domainAccess/$(OUTDIR)))
	mkdir ./domainAccess/$(OUTDIR) 
endif
ifneq (./quagga/$(OUTDIR), $(wildcard ./quagga/$(OUTDIR)))
	mkdir ./quagga/$(OUTDIR) 
endif
ifneq (./rm_tool/$(OUTDIR), $(wildcard ./rm_tool/$(OUTDIR)))
	mkdir ./rm_tool/$(OUTDIR) 
endif
ifneq (./libsqlite3/$(OUTDIR), $(wildcard ./libsqlite3/$(OUTDIR)))
	mkdir ./libsqlite3/$(OUTDIR) 
endif
ifneq (./libpcap/$(OUTDIR), $(wildcard ./libpcap/$(OUTDIR)))
	mkdir ./libpcap/$(OUTDIR) 
endif
ifneq (./libreadline/$(OUTDIR), $(wildcard ./libreadline/$(OUTDIR)))
	mkdir ./libreadline/$(OUTDIR) 
endif
ifneq (./tmstat/$(OUTDIR), $(wildcard ./tmstat/$(OUTDIR)))
	mkdir ./tmstat/$(OUTDIR) 
endif
ifneq (./udial/$(OUTDIR), $(wildcard ./udial/$(OUTDIR)))
	mkdir ./udial/$(OUTDIR) 
endif
ifneq (./openssl/$(OUTDIR), $(wildcard ./openssl/$(OUTDIR)))
	mkdir ./openssl/$(OUTDIR) 
endif
ifneq (./kidvpn/$(OUTDIR), $(wildcard ./kidvpn/$(OUTDIR)))
	mkdir ./kidvpn/$(OUTDIR) 
endif

cp_bin_file:
	@echo "PLATFORM is " $(PLATFORM)
	@cp -r ./arpSpoofDefense/platform/$(PLATFORM)/arpSpoofDefense.ko ./arpSpoofDefense/$(OUTDIR)/arpSpoofDefense.ko
	@cp -r ./dhcpserver/platform/$(PLATFORM)/dhcpserver ./dhcpserver/$(OUTDIR)/dhcpserver
	@cp -r ./domainAccess/platform/$(PLATFORM)/domainAccess.ko ./domainAccess/$(OUTDIR)/domainAccess.ko
	@cp -r ./quagga/platform/$(PLATFORM)/bgpd ./quagga/$(OUTDIR)/bgpd
	@cp -r ./quagga/platform/$(PLATFORM)/isisd ./quagga/$(OUTDIR)/isisd
	@cp -r ./quagga/platform/$(PLATFORM)/libzebra.so ./quagga/$(OUTDIR)/libzebra.so
	@cp -r ./quagga/platform/$(PLATFORM)/ospfd ./quagga/$(OUTDIR)/ospfd
	@cp -r ./quagga/platform/$(PLATFORM)/pimd ./quagga/$(OUTDIR)/pimd
	@cp -r ./quagga/platform/$(PLATFORM)/ripd ./quagga/$(OUTDIR)/ripd
	@cp -r ./quagga/platform/$(PLATFORM)/ripngd ./quagga/$(OUTDIR)/ripngd
	@cp -r ./quagga/platform/$(PLATFORM)/vtysh ./quagga/$(OUTDIR)/vtysh
	@cp -r ./quagga/platform/$(PLATFORM)/zebra ./quagga/$(OUTDIR)/zebra
	@cp -r ./rm_tool/platform/$(PLATFORM)/rm_tool ./rm_tool/$(OUTDIR)/rm_tool
	@cp -r ./libsqlite3/platform/$(PLATFORM)/libsqlite3.so ./libsqlite3/$(OUTDIR)/libsqlite3.so
	@cp -r ./libsqlite3/platform/$(PLATFORM)/sqlite3 ./libsqlite3/$(OUTDIR)/sqlite3
	@cp -r ./libpcap/platform/$(PLATFORM)/libpcap.so ./libpcap/$(OUTDIR)/libpcap.so
	@cp -r ./libreadline/platform/$(PLATFORM)/libreadline.so ./libreadline/$(OUTDIR)/libreadline.so
	@cp -r ./tmstat/platform/$(PLATFORM)/trafficStats.ko ./tmstat/$(OUTDIR)/trafficStats.ko
	@cp -r ./udial/platform/$(PLATFORM)/udial ./udial/$(OUTDIR)/udial
	@cp -r ./openssl/platform/$(PLATFORM)/lib4758cca.so  ./openssl/$(OUTDIR)/lib4758cca.so
	@cp -r ./openssl/platform/$(PLATFORM)/libaep.so      ./openssl/$(OUTDIR)/libaep.so
	@cp -r ./openssl/platform/$(PLATFORM)/libatalla.so   ./openssl/$(OUTDIR)/libatalla.so
	@cp -r ./openssl/platform/$(PLATFORM)/libcapi.so     ./openssl/$(OUTDIR)/libcapi.so
	@cp -r ./openssl/platform/$(PLATFORM)/libchil.so     ./openssl/$(OUTDIR)/libchil.so
	@cp -r ./openssl/platform/$(PLATFORM)/libcrypto.so   ./openssl/$(OUTDIR)/libcrypto.so
	@cp -r ./openssl/platform/$(PLATFORM)/libcswift.so   ./openssl/$(OUTDIR)/libcswift.so
	@cp -r ./openssl/platform/$(PLATFORM)/libgmp.so      ./openssl/$(OUTDIR)/libgmp.so
	@cp -r ./openssl/platform/$(PLATFORM)/libnuron.so    ./openssl/$(OUTDIR)/libnuron.so
	@cp -r ./openssl/platform/$(PLATFORM)/libpadlock.so  ./openssl/$(OUTDIR)/libpadlock.so
	@cp -r ./openssl/platform/$(PLATFORM)/libssl.so      ./openssl/$(OUTDIR)/libssl.so
	@cp -r ./openssl/platform/$(PLATFORM)/libsureware.so ./openssl/$(OUTDIR)/libsureware.so
	@cp -r ./openssl/platform/$(PLATFORM)/libubsec.so    ./openssl/$(OUTDIR)/libubsec.so
	@cp -r ./openssl/platform/$(PLATFORM)/openssl        ./openssl/$(OUTDIR)/openssl
	@cp -r ./openssl/platform/$(PLATFORM)/openssl.cnf    ./openssl/$(OUTDIR)/openssl.cnf
	@cp -r ./kidvpn/platform/$(PLATFORM)/kidvpn          ./kidvpn/$(OUTDIR)/kidvpn

removedir:
	@if [ -f "Release" ]; then \
	rm  -r "Release"; fi
	@if [ -f "Debug" ]; then \
	rm  -r "Debug"; fi

TARGETS += mkdir cp_bin_file removedir

clean_tool:
	@if [ -d "./arpSpoofDefense/$(OUTDIR)" ]; then \
	rm -rf ./arpSpoofDefense/$(OUTDIR); fi
	@if [ -d "./dhcpserver/$(OUTDIR)" ]; then \
	rm -rf ./dhcpserver/$(OUTDIR); fi
	@if [ -d "./domainAccess/$(OUTDIR)" ]; then \
	rm -rf ./domainAccess/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./quagga/$(OUTDIR)" ]; then \
	rm -rf ./quagga/$(OUTDIR); fi
	@if [ -d "./rm_tool/$(OUTDIR)" ]; then \
	rm -rf ./rm_tool/$(OUTDIR); fi
	@if [ -d "./libpcap/$(OUTDIR)" ]; then \
	rm -rf ./libpcap/$(OUTDIR); fi
	@if [ -d "./libsqlite3/$(OUTDIR)" ]; then \
	rm -rf ./libsqlite3/$(OUTDIR); fi
	@if [ -d "./libsqlite3/$(OUTDIR)" ]; then \
	rm -rf ./libsqlite3/$(OUTDIR); fi
	@if [ -d "./libreadline/$(OUTDIR)" ]; then \
	rm -rf ./libreadline/$(OUTDIR); fi	
	@if [ -d "./tmstat/$(OUTDIR)" ]; then \
	rm -rf ./tmstat/$(OUTDIR); fi
	@if [ -d "./udial/$(OUTDIR)" ]; then \
	rm -rf ./udial/$(OUTDIR); fi
	@if [ -d "./openssl/$(OUTDIR)" ]; then \
	rm -rf ./openssl/$(OUTDIR); fi
	@if [ -d "./kidvpn/$(OUTDIR)" ]; then \
	rm -rf ./kidvpn/$(OUTDIR); fi
#*********************************************************************************************************
# End
#*********************************************************************************************************





