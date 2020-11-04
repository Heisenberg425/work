#!/bin/lua

require("lfs")

hosts_content = '127.0.0.1    localhost\n127.0.0.1    sylixos\n'

function cpfile(srcFile, dstFile)
    local srcFileExistFlg = lfs.attributes(srcFile)
    if  srcFileExistFlg == nil then
        print("File "..srcFile.." Not Exist.")
        return  false
    elseif srcFileExistFlg ~= nil then
        os.execute("chmod 774 "..srcFile)
        
        local srcFileExistFlg = lfs.attributes(dstFile)
        if srcFileExistFlg ~= nil then
            local sameFlag = os.execute("cmp "..srcFile.." "..dstFile)
            if sameFlag ~= nil then
                print("file need not update!")
                return  true
            elseif sameFlag == nil then
                os.execute("chmod 774 "..dstFile)
                os.execute("rm "..dstFile)
            end
            
            os.execute("sync")
        end
        
        os.execute("cp ./"..srcFile.." "..dstFile)
        os.execute("sync")
    end        

    return  true
end

function mkdirectory(path)
    local existDirFlag = lfs.attributes(path)
    if existDirFlag == nil then
        os.execute("mkdir "..path)
    elseif existDirFlag ~= nil then
        local existToolFlag = lfs.attributes("/bin/rm_tool")
        if existToolFlag == nil then
            print("can not delete "..path)
            return  false
        elseif existToolFlag ~= nil then
            os.execute("rm_tool -r "..path)
            
            os.execute("mkdir "..path)
            print("make dir "..path.." success")
        end        
    end
    
    return  true
end

function bspFileCopy()
    -- LS3A3000
    --cpfile("OpenRT-Packet/image/bspls3x.elf", "/boot/bspls3x.elf")
    
    --FT1500A
    --cpfile("OpenRT-Packet/image/bspftxxxx.bin", "/boot/bspftxxxx.bin")
    
    --x86
    --[[
    cpfile("OpenRT-Packet/image/bspx86.elf", "/boot/bspx86.elf")
    cpfile("OpenRT-Packet/image/grldr", "/boot/grldr")
    cpfile("OpenRT-Packet/image/menu.lst", "/boot/menu.lst")
    --]]
end

print("OpenRT setup detect...")

local tarExistFlag = lfs.attributes("./OpenRT-Packet.tar.gz")
if tarExistFlag == nil then
    print("OpenRT Image File Not Exist.")
    
    return false
end
    
existFlag = lfs.attributes("/tmp/OpenRT-Packet")
if existFlag == nil then    
    print("OpenRT setup start!")
    
    os.execute("mkdir /tmp/OpenRT-Packet")
    os.execute("untar ./OpenRT-Packet.tar.gz /tmp")
    
    print("untar success!")
    
    -- tools
    cpfile("OpenRT-Packet/tools/rm_tool", "/bin/rm_tool")
    
    -- kernel module
    cpfile("OpenRT-Packet/arpSpoofDefense/arpSpoofDefense.ko", "/lib/modules/arpSpoofDefense.ko")
    cpfile("OpenRT-Packet/arpSpoofDefense/arpSpoofDefense.ini", "/etc/arpSpoofDefense.ini")
    cpfile("OpenRT-Packet/tmstat/trafficStats.ko", "/lib/modules/trafficStats.ko")
    cpfile("OpenRT-Packet/domainAccess/domainAccess.ko", "/lib/modules/domainAccess.ko")
    
    -- DHCP server
    mkdirectory("/apps/dhcpserver")
    mkdirectory("/var/dhcplease")
    
    cpfile("OpenRT-Packet/dhcpserver/dhcpserver", "/apps/dhcpserver/dhcpserver")
    cpfile("OpenRT-Packet/dhcpserver/dhcpdconfig.ini", "/etc/dhcpdconfig.ini")
    cpfile("OpenRT-Packet/dhcpserver/dhcpd.lease", "/var/dhcplease/dhcpd.leases")
    
    -- KidVPN
    mkdirectory("/etc/OpenRT")
    mkdirectory("/etc/kidvpn")
    
    cpfile("OpenRT-Packet/kidvpn/kidvpn", "/sbin/kidvpn")
    cpfile("OpenRT-Packet/kidvpn/ipPos.datx", "/etc/OpenRT/ipPos.datx")
    
    -- quagga
    mkdirectory("/etc/quagga")
    
    cpfile("OpenRT-Packet/quagga/bin/zebra",  "/bin/zebra")
    cpfile("OpenRT-Packet/quagga/bin/bgpd",   "/bin/bgpd")
    cpfile("OpenRT-Packet/quagga/bin/isisd",  "/bin/isisd")
    cpfile("OpenRT-Packet/quagga/bin/pimd",   "/bin/pimd")
    cpfile("OpenRT-Packet/quagga/bin/ripd",   "/bin/ripd")
    cpfile("OpenRT-Packet/quagga/bin/ospfd",  "/bin/ospfd")
    cpfile("OpenRT-Packet/quagga/bin/ripngd", "/bin/ripngd")
    cpfile("OpenRT-Packet/quagga/bin/vtysh",  "/bin/vtysh")

    cpfile("OpenRT-Packet/quagga/conf/bgpd.conf",    "/etc/quagga/bgpd.conf")
    cpfile("OpenRT-Packet/quagga/conf/isisd.conf",   "/etc/quagga/isisd.conf")
    cpfile("OpenRT-Packet/quagga/conf/ospf6d.conf",  "/etc/quagga/ospf6d.conf")
    cpfile("OpenRT-Packet/quagga/conf/ospfd.conf",   "/etc/quagga/ospfd.conf")
    cpfile("OpenRT-Packet/quagga/conf/pimd.conf",    "/etc/quagga/pimd.conf")
    cpfile("OpenRT-Packet/quagga/conf/ripd.conf",    "/etc/quagga/ripd.conf")
    cpfile("OpenRT-Packet/quagga/conf/ripngd.conf",  "/etc/quagga/ripngd.conf")
    cpfile("OpenRT-Packet/quagga/conf/vtysh.conf",   "/etc/quagga/vtysh.conf")
    cpfile("OpenRT-Packet/quagga/conf/zebra.conf",   "/etc/quagga/zebra.conf")

    cpfile("OpenRT-Packet/quagga/so/libzebra.so",   "/lib/libzebra.so")

    -- udial
    cpfile("OpenRT-Packet/udial/udial", "/bin/udial")
    
    -- OpenRT
    mkdirectory("/apps/OpenRTServer")
    mkdirectory("/apps/goahead")
    
    cpfile("OpenRT-Packet/OpenRT/CommunicateServer/libcomm.so", "/lib/libcomm.so")
    cpfile("OpenRT-Packet/OpenRT/CommunicateServer/libnethandle.so", "/lib/libnethandle.so")

    cpfile("OpenRT-Packet/OpenRT/OpenRTServer/OpenRTServer", "/apps/OpenRTServer/OpenRTServer")

    cpfile("OpenRT-Packet/OpenRT/WebServer/goahead", "/apps/goahead/goahead")
    cpfile("OpenRT-Packet/OpenRT/WebServer/webcomp", "/apps/goahead/webcomp")
    cpfile("OpenRT-Packet/OpenRT/WebServer/gopass", "/apps/goahead/gopass")

    cpfile("OpenRT-Packet/OpenRT/WebServer/auth.txt", "/apps/goahead/auth.txt")
    cpfile("OpenRT-Packet/OpenRT/WebServer/route.txt", "/apps/goahead/route.txt")
    cpfile("OpenRT-Packet/OpenRT/WebServer/self.crt", "/apps/goahead/self.crt")
    cpfile("OpenRT-Packet/OpenRT/WebServer/self.key", "/apps/goahead/self.key")

    cpfile("OpenRT-Packet/OpenRT/WebServer/libgoahead.so", "/lib/libgoahead.so")

    cpfile("OpenRT-Packet/OpenRT/conf/OpenRTCFGDB", "/etc/OpenRT/OpenRTCFGDB")
    
    print("web")
    
    -- web
    mkdirectory("/apps/goahead/web")
    
    os.execute("untar ./OpenRT-Packet/web.tar.gz /apps/goahead")
    
    print("OpenSSL")
    
    -- OpenSSL
    mkdirectory("/usr/lib/ssl")
    mkdirectory("/usr/lib/ssl/lib")
    mkdirectory("/usr/lib/ssl/lib/engines")
    
    cpfile("OpenRT-Packet/image/library/openssl/libcrypto.so",  "/lib/libcrypto.so")
    cpfile("OpenRT-Packet/image/library/openssl/libssl.so",     "/lib/libssl.so")

    cpfile("OpenRT-Packet/image/library/openssl/lib4758cca.so",  "/usr/lib/ssl/lib/engines/lib4758cca.so")
    cpfile("OpenRT-Packet/image/library/openssl/libaep.so",      "/usr/lib/ssl/lib/engines/libaep.so")
    cpfile("OpenRT-Packet/image/library/openssl/libatalla.so",   "/usr/lib/ssl/lib/engines/libatalla.so")
    cpfile("OpenRT-Packet/image/library/openssl/libcapi.so",     "/usr/lib/ssl/lib/engines/libcapi.so")
    cpfile("OpenRT-Packet/image/library/openssl/libchil.so",     "/usr/lib/ssl/lib/engines/libchil.so")
    cpfile("OpenRT-Packet/image/library/openssl/libcswift.so",   "/usr/lib/ssl/lib/engines/libcswift.so")
    cpfile("OpenRT-Packet/image/library/openssl/libgmp.so",      "/usr/lib/ssl/lib/engines/libgmp.so")
    cpfile("OpenRT-Packet/image/library/openssl/libnuron.so",    "/usr/lib/ssl/lib/engines/libnuron.so")
    cpfile("OpenRT-Packet/image/library/openssl/libpadlock.so",  "/usr/lib/ssl/lib/engines/libpadlock.so")
    cpfile("OpenRT-Packet/image/library/openssl/libsureware.so", "/usr/lib/ssl/lib/engines/libsureware.so")
    cpfile("OpenRT-Packet/image/library/openssl/libubsec.so",    "/usr/lib/ssl/lib/engines/libubsec.so")

    cpfile("OpenRT-Packet/image/library/openssl/openssl",     "/usr/bin/openssl")
    cpfile("OpenRT-Packet/image/library/openssl/openssl.cnf", "/usr/lib/ssl/openssl.cnf")

    cpfile("OpenRT-Packet/image/library/readline/libreadline.so", "/lib/libreadline.so")

    cpfile("OpenRT-Packet/image/library/sqlite3/libsqlite3.so", "/lib/libsqlite3.so")
    cpfile("OpenRT-Packet/image/library/sqlite3/sqlite3",       "/bin/sqlite3")

    os.execute("echo "..hosts_content.." >/etc/hosts") 
    cpfile("OpenRT-Packet/etc/startup.sh",  "/etc/startup.sh")
    cpfile("OpenRT-Packet/etc/ifparam.ini", "/etc/ifparam.ini")

    print("image")
    
    -- image
    --cpfile("OpenRT-Packet/image/library/cextern/libcextern.so", "/lib/libcextern.so")
    cpfile("OpenRT-Packet/image/library/libpcap/libpcap.so", "/lib/libpcap.so")

    --cpfile("OpenRT-Packet/image/library/libsylixos/env", "/usr/bin/env")

    --cpfile("OpenRT-Packet/image/library/libsylixos/libmbedcrypto.so", "/lib/libmbedcrypto.so")
    --cpfile("OpenRT-Packet/image/library/libsylixos/libmbedtls.so", "/lib/libmbedtls.so")
    --cpfile("OpenRT-Packet/image/library/libsylixos/libmbedx509.so", "/lib/libmbedx509.so")

    -- board support packet update.
    bspFileCopy()
    
    --cpfile("OpenRT-Packet/image/library/libsylixos/libvpmpdm.so", "/lib/libvpmpdm.so")
    
    os.execute("sleep 1")
    
    os.execute("sync")
       
    os.execute("sleep 1")
    
    os.execute("reboot")
    
elseif existFlag ~= nil then
    print("OpenRT already setup!")
end

