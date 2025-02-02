/*
    bit.h -- Build It Configuration Header for linux-x86-default

    This header is created by Bit during configuration. To change settings, re-run
    configure or define variables in your Makefile to override these default values.
 */


/* Settings */
#ifndef BIT_AUTHOR
    #define BIT_AUTHOR "Embedthis Software"
#endif
#ifndef BIT_BIT
    #define BIT_BIT "0.9.3"
#endif
#ifndef BIT_BUILD_NUMBER
    #define BIT_BUILD_NUMBER 0
#endif
#ifndef BIT_COMPANY
    #define BIT_COMPANY "embedthis"
#endif
#ifndef BIT_COMPATIBLE
    #define BIT_COMPATIBLE "3.3.0"
#endif
#ifndef BIT_DEBUG
    #define BIT_DEBUG 1
#endif
#ifndef BIT_DEPTH
    #define BIT_DEPTH 1
#endif
#ifndef BIT_DESCRIPTION
    #define BIT_DESCRIPTION "Embedthis GoAhead Embedded Web Server"
#endif
#ifndef BIT_DISCOVER
    #define BIT_DISCOVER "doxygen,dsi,man,man2html,pmaker,ssl,utest,zip"
#endif
#ifndef BIT_GOAHEAD_ACCESS_LOG
    #define BIT_GOAHEAD_ACCESS_LOG 0
#endif
#ifndef BIT_GOAHEAD_AUTH
    #define BIT_GOAHEAD_AUTH 1
#endif
#ifndef BIT_GOAHEAD_AUTH_STORE
    #define BIT_GOAHEAD_AUTH_STORE "file"
#endif
#ifndef BIT_GOAHEAD_AUTO_LOGIN
    #define BIT_GOAHEAD_AUTO_LOGIN 0
#endif
#ifndef BIT_GOAHEAD_CA
    #define BIT_GOAHEAD_CA ""
#endif
#ifndef BIT_GOAHEAD_CERTIFICATE
    #define BIT_GOAHEAD_CERTIFICATE "self.crt"
#endif
#ifndef BIT_GOAHEAD_CGI
    #define BIT_GOAHEAD_CGI 1
#endif
#ifndef BIT_GOAHEAD_CGI_BIN
    #define BIT_GOAHEAD_CGI_BIN "cgi-bin"
#endif
#ifndef BIT_GOAHEAD_CIPHERS
    #define BIT_GOAHEAD_CIPHERS ""
#endif
#ifndef BIT_GOAHEAD_CLIENT_CACHE
    #define BIT_GOAHEAD_CLIENT_CACHE "css,gif,ico,jpg,js,png"
#endif
#ifndef BIT_GOAHEAD_CLIENT_CACHE_LIFESPAN
    #define BIT_GOAHEAD_CLIENT_CACHE_LIFESPAN 86400
#endif
#ifndef BIT_GOAHEAD_DOCUMENTS
    #define BIT_GOAHEAD_DOCUMENTS "web"
#endif
#ifndef BIT_GOAHEAD_JAVASCRIPT
    #define BIT_GOAHEAD_JAVASCRIPT 1
#endif
#ifndef BIT_GOAHEAD_KEY
    #define BIT_GOAHEAD_KEY "self.key"
#endif
#ifndef BIT_GOAHEAD_LEGACY
    #define BIT_GOAHEAD_LEGACY 1
#endif
#ifndef BIT_GOAHEAD_LIMIT_BUFFER
    #define BIT_GOAHEAD_LIMIT_BUFFER 8192
#endif
#ifndef BIT_GOAHEAD_LIMIT_FILENAME
    #define BIT_GOAHEAD_LIMIT_FILENAME 256
#endif
#ifndef BIT_GOAHEAD_LIMIT_FILES
    #define BIT_GOAHEAD_LIMIT_FILES 0
#endif
#ifndef BIT_GOAHEAD_LIMIT_HEADER
    #define BIT_GOAHEAD_LIMIT_HEADER 2048
#endif
#ifndef BIT_GOAHEAD_LIMIT_HEADERS
    #define BIT_GOAHEAD_LIMIT_HEADERS 4096
#endif
#ifndef BIT_GOAHEAD_LIMIT_NUM_HEADERS
    #define BIT_GOAHEAD_LIMIT_NUM_HEADERS 64
#endif
#ifndef BIT_GOAHEAD_LIMIT_PARSE_TIMEOUT
    #define BIT_GOAHEAD_LIMIT_PARSE_TIMEOUT 5
#endif
#ifndef BIT_GOAHEAD_LIMIT_PASSWORD
    #define BIT_GOAHEAD_LIMIT_PASSWORD 32
#endif
#ifndef BIT_GOAHEAD_LIMIT_POST
    #define BIT_GOAHEAD_LIMIT_POST 204800000
#endif
#ifndef BIT_GOAHEAD_LIMIT_PUT
    #define BIT_GOAHEAD_LIMIT_PUT 204800000
#endif
#ifndef BIT_GOAHEAD_LIMIT_SESSION_COUNT
    #define BIT_GOAHEAD_LIMIT_SESSION_COUNT 512
#endif
#ifndef BIT_GOAHEAD_LIMIT_SESSION_LIFE
    #define BIT_GOAHEAD_LIMIT_SESSION_LIFE 1800
#endif
#ifndef BIT_GOAHEAD_LIMIT_STRING
    #define BIT_GOAHEAD_LIMIT_STRING 4096
#endif
#ifndef BIT_GOAHEAD_LIMIT_TIMEOUT
    #define BIT_GOAHEAD_LIMIT_TIMEOUT 60
#endif
#ifndef BIT_GOAHEAD_LIMIT_UPLOAD
    #define BIT_GOAHEAD_LIMIT_UPLOAD 204800000
#endif
#ifndef BIT_GOAHEAD_LIMIT_URI
    #define BIT_GOAHEAD_LIMIT_URI 2048
#endif
#ifndef BIT_GOAHEAD_LISTEN
    #define BIT_GOAHEAD_LISTEN "http://*:80,https://*:443"
#endif
#ifndef BIT_GOAHEAD_LOGFILE
    #define BIT_GOAHEAD_LOGFILE "stderr:0"
#endif
#ifndef BIT_GOAHEAD_LOGGING
    #define BIT_GOAHEAD_LOGGING 1
#endif
#ifndef BIT_GOAHEAD_PUT_DIR
    #define BIT_GOAHEAD_PUT_DIR "."
#endif
#ifndef BIT_GOAHEAD_REALM
    #define BIT_GOAHEAD_REALM "Login"
#endif
#ifndef BIT_GOAHEAD_REPLACE_MALLOC
    #define BIT_GOAHEAD_REPLACE_MALLOC 0
#endif
#ifndef BIT_GOAHEAD_STEALTH
    #define BIT_GOAHEAD_STEALTH 1
#endif
#ifndef BIT_GOAHEAD_TRACING
    #define BIT_GOAHEAD_TRACING 1
#endif
#ifndef BIT_GOAHEAD_UPLOAD
    #define BIT_GOAHEAD_UPLOAD 1
#endif
#ifndef BIT_GOAHEAD_UPLOAD_DIR
    #define BIT_GOAHEAD_UPLOAD_DIR "/tmp"
#endif
#ifndef BIT_GOAHEAD_VERIFY_ISSUER
    #define BIT_GOAHEAD_VERIFY_ISSUER 0
#endif
#ifndef BIT_GOAHEAD_VERIFY_PEER
    #define BIT_GOAHEAD_VERIFY_PEER 0
#endif
#ifndef BIT_GOAHEAD_XFRAME_HEADER
    #define BIT_GOAHEAD_XFRAME_HEADER "SAMEORIGIN"
#endif
#ifndef BIT_HAS_ATOMIC
    #define BIT_HAS_ATOMIC 0
#endif
#ifndef BIT_HAS_ATOMIC64
    #define BIT_HAS_ATOMIC64 0
#endif
#ifndef BIT_HAS_DOUBLE_BRACES
    #define BIT_HAS_DOUBLE_BRACES 0
#endif
#ifndef BIT_HAS_DYN_LOAD
    #define BIT_HAS_DYN_LOAD 1
#endif
#ifndef BIT_HAS_LIB_EDIT
    #define BIT_HAS_LIB_EDIT 0
#endif
#ifndef BIT_HAS_LIB_RT
    #define BIT_HAS_LIB_RT 1
#endif
#ifndef BIT_HAS_MMU
    #define BIT_HAS_MMU 1
#endif
#ifndef BIT_HAS_MTUNE
    #define BIT_HAS_MTUNE 1
#endif
#ifndef BIT_HAS_PAM
    #define BIT_HAS_PAM 0
#endif
#ifndef BIT_HAS_STACK_PROTECTOR
    #define BIT_HAS_STACK_PROTECTOR 1
#endif
#ifndef BIT_HAS_SYNC
    #define BIT_HAS_SYNC 1
#endif
#ifndef BIT_HAS_SYNC64
    #define BIT_HAS_SYNC64 1
#endif
#ifndef BIT_HAS_SYNC_CAS
    #define BIT_HAS_SYNC_CAS 0
#endif
#ifndef BIT_HAS_UNNAMED_UNIONS
    #define BIT_HAS_UNNAMED_UNIONS 1
#endif
#ifndef BIT_MANIFEST
    #define BIT_MANIFEST "package/manifest.bit"
#endif
#ifndef BIT_NAME
    #define BIT_NAME "goahead"
#endif
#ifndef BIT_PACKS
    #define BIT_PACKS "bits/packs"
#endif
#ifndef BIT_PREFIXES
    #define BIT_PREFIXES "embedthis-prefixes"
#endif
#ifndef BIT_PRODUCT
    #define BIT_PRODUCT "goahead"
#endif
#ifndef BIT_REQUIRES
    #define BIT_REQUIRES "compiler,lib,link"
#endif
#ifndef BIT_STATIC
    #define BIT_STATIC 0
#endif
#ifndef BIT_SYNC
    #define BIT_SYNC "bitos,est"
#endif
#ifndef BIT_TITLE
    #define BIT_TITLE "Embedthis GoAhead"
#endif
#ifndef BIT_VERSION
    #define BIT_VERSION "3.3.0"
#endif
#ifndef BIT_WARN64TO32
    #define BIT_WARN64TO32 0
#endif
#ifndef BIT_WARN_UNUSED
    #define BIT_WARN_UNUSED 1
#endif
#ifndef BIT_WITHOUT_ALL
    #define BIT_WITHOUT_ALL "doxygen,dsi,man,man2html,pmaker,ssl"
#endif

/* Prefixes */
#ifndef BIT_ROOT_PREFIX
    #define BIT_ROOT_PREFIX "/"
#endif
#ifndef BIT_BASE_PREFIX
    #define BIT_BASE_PREFIX "/usr/local"
#endif
#ifndef BIT_DATA_PREFIX
    #define BIT_DATA_PREFIX "/"
#endif
#ifndef BIT_STATE_PREFIX
    #define BIT_STATE_PREFIX "/var"
#endif
#ifndef BIT_APP_PREFIX
    #define BIT_APP_PREFIX "/usr/local/lib/goahead"
#endif
#ifndef BIT_VAPP_PREFIX
    #define BIT_VAPP_PREFIX "/usr/local/lib/goahead/3.3.0"
#endif
#ifndef BIT_BIN_PREFIX
    #define BIT_BIN_PREFIX "/usr/local/bin"
#endif
#ifndef BIT_INC_PREFIX
    #define BIT_INC_PREFIX "/usr/local/include"
#endif
#ifndef BIT_LIB_PREFIX
    #define BIT_LIB_PREFIX "/usr/local/lib"
#endif
#ifndef BIT_MAN_PREFIX
    #define BIT_MAN_PREFIX "/usr/local/share/man"
#endif
#ifndef BIT_SBIN_PREFIX
    #define BIT_SBIN_PREFIX "/usr/local/sbin"
#endif
#ifndef BIT_ETC_PREFIX
    #define BIT_ETC_PREFIX "/etc/goahead"
#endif
#ifndef BIT_WEB_PREFIX
    #define BIT_WEB_PREFIX "/var/www/goahead-default"
#endif
#ifndef BIT_LOG_PREFIX
    #define BIT_LOG_PREFIX "/var/log/goahead"
#endif
#ifndef BIT_SPOOL_PREFIX
    #define BIT_SPOOL_PREFIX "/var/spool/goahead"
#endif
#ifndef BIT_CACHE_PREFIX
    #define BIT_CACHE_PREFIX "/var/spool/goahead/cache"
#endif
#ifndef BIT_SRC_PREFIX
    #define BIT_SRC_PREFIX "goahead-3.3.0"
#endif

/* Suffixes */
#ifndef BIT_EXE
    #define BIT_EXE ""
#endif
#ifndef BIT_SHLIB
    #define BIT_SHLIB ".so"
#endif
#ifndef BIT_SHOBJ
    #define BIT_SHOBJ ".so"
#endif
#ifndef BIT_LIB
    #define BIT_LIB ".a"
#endif
#ifndef BIT_OBJ
    #define BIT_OBJ ".o"
#endif

/* Profile */
#ifndef BIT_CONFIG_CMD
    #define BIT_CONFIG_CMD "bit -d -q -platform linux-x86-default -configure . -gen make"
#endif
#ifndef BIT_GOAHEAD_PRODUCT
    #define BIT_GOAHEAD_PRODUCT 1
#endif
#ifndef BIT_PROFILE
    #define BIT_PROFILE "default"
#endif
#ifndef BIT_TUNE_SIZE
    #define BIT_TUNE_SIZE 1
#endif

/* Miscellaneous */
#ifndef BIT_MAJOR_VERSION
    #define BIT_MAJOR_VERSION 3
#endif
#ifndef BIT_MINOR_VERSION
    #define BIT_MINOR_VERSION 3
#endif
#ifndef BIT_PATCH_VERSION
    #define BIT_PATCH_VERSION 0
#endif
#ifndef BIT_VNUM
    #define BIT_VNUM 300030000
#endif

/* Packs */
#ifndef BIT_PACK_CC
    #define BIT_PACK_CC 1
#endif
#ifndef BIT_PACK_DOXYGEN
    #define BIT_PACK_DOXYGEN 1
#endif
#ifndef BIT_PACK_DSI
    #define BIT_PACK_DSI 1
#endif
#ifndef BIT_PACK_EST
    #define BIT_PACK_EST 1
#endif
#ifndef BIT_PACK_LIB
    #define BIT_PACK_LIB 1
#endif
#ifndef BIT_PACK_LINK
    #define BIT_PACK_LINK 1
#endif
#ifndef BIT_PACK_MAN
    #define BIT_PACK_MAN 1
#endif
#ifndef BIT_PACK_MAN2HTML
    #define BIT_PACK_MAN2HTML 1
#endif
#ifndef BIT_PACK_MATRIXSSL
    #define BIT_PACK_MATRIXSSL 0
#endif
#ifndef BIT_PACK_NANOSSL
    #define BIT_PACK_NANOSSL 0
#endif
#ifndef BIT_PACK_OPENSSL
    #define BIT_PACK_OPENSSL 0
#endif
#ifndef BIT_PACK_PMAKER
    #define BIT_PACK_PMAKER 1
#endif
#ifndef BIT_PACK_SSL
    #define BIT_PACK_SSL 0
#endif
#ifndef BIT_PACK_UTEST
    #define BIT_PACK_UTEST 1
#endif
#ifndef BIT_PACK_VXWORKS
    #define BIT_PACK_VXWORKS 0
#endif
#ifdef SYLIXOS
    #define BIT_PACK_SYLIXOS 1
#endif
#ifndef BIT_PACK_WINSDK
    #define BIT_PACK_WINSDK 0
#endif
#ifndef BIT_PACK_ZIP
    #define BIT_PACK_ZIP 1
#endif
