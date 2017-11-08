/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/

#define APP_NAME "AMING"
#define APP_VERSION "1.0"
#define APP_YEAR "2017"
#define APP_COPYRIGHT "ANYKS LLC"
#define APP_DOMAIN "anyks.com"
#define APP_SITE "http://anyks.com"
#define APP_EMAIL "info@anyks.com"
#define APP_SUPPORT "support@anyks.com"
#define APP_AUTHOR "Yuriy Lobarev"
#define APP_TELEGRAM "@forman"
#define APP_PHONE "+7(910)983-95-90"


#define OPT_NULL 0x00		
#define OPT_CONNECT 0x01	
#define OPT_UPGRADE 0x02	
#define OPT_AGENT 0x04		
#define OPT_GZIP 0x08		
#define OPT_SMART 0x10		
#define OPT_KEEPALIVE 0x20	
#define OPT_LOG 0x40		
#define OPT_PGZIP 0x80		
#define OPT_DEBLOCK 0x100	


#define AMING_EMPTY ""				
#define AMING_CLEAR {}				
#define AMING_NULL 0x00				
#define AMING_IP 0x01				
#define AMING_NETWORK 0x02			
#define AMING_IPV4 0x03				
#define AMING_IPV6 0x04				
#define AMING_DOMAIN 0x05			
#define AMING_ADDRESS 0x06			
#define AMING_MAC 0x07				
#define AMING_USER 0x08				
#define AMING_GROUP 0x09			
#define AMING_HTTP_ACTION 0x10		
#define AMING_HTTP_METHOD 0x11		
#define AMING_HTTP_TRAFFIC 0x12		

#define AMING_TYPE_HTTP 0x01		
#define AMING_TYPE_SOCKS 0x02		
#define AMING_TYPE_REDIRECT 0x04	

#define AMING_AUTH_BASIC 0x01
#define AMING_AUTH_BEARER 0x02
#define AMING_AUTH_DIGEST 0x04
#define AMING_AUTH_HMAC 0x08
#define AMING_AUTH_HOBA 0x10
#define AMING_AUTH_MUTUAL 0x20
#define AMING_AUTH_AWS4HMACSHA256 0x40


#define AUSERS_TYPE_FILE 0x01								
#define AUSERS_TYPE_PAM 0x02								
#define AUSERS_TYPE_LDAP 0x04								
#define AUSERS_TYPE_FILE_PAM (0x01 | 0x02)					
#define AUSERS_TYPE_FILE_LDAP (0x01 | 0x04)					
#define AUSERS_TYPE_PAM_LDAP (0x02 | 0x04)					
#define AUSERS_TYPE_FILE_PAM_LDAP (0x01 | 0x02 | 0x04)		


#define PROXY_TYPE "http"
#define PROXY_USER "nobody"
#define PROXY_GROUP "nogroup"
#define PROXY_NAME "anyks"
#define PROXY_CONFIGS "none"
#define PROXY_TRANSFER false
#define PROXY_REVERSE false
#define PROXY_FORWARD true
#define PROXY_OPTIMOS false
#define PROXY_DEBUG false
#define PROXY_DAEMON false
#define PROXY_IPV6ONLY true
#define PROXY_SUBNET false
#define PROXY_PIPELINING true
#define PROXY_IPV "4 -> 4"
#define PROXY_HTTP_PORT 8080
#define PROXY_SOCKS5_PORT 1080
#define PROXY_REDIRECT_PORT 1180
#define PROXY_HTTP_METHODS "OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT"


#define CACHE_DNS true
#define CACHE_RESPONSE false
#define CACHE_DTTL "1d"
#define CACHE_DIR "/var/cache"


#define PID_DIR "/var/run"


#ifdef __linux__
	#define CONFIG_DIR "/etc"
#else
	#define CONFIG_DIR "/usr/local/etc"
#endif

 
 


#define MAX_WORKERS 0

#define MMAX_WORKERS 300


#define CONNECTS_TOTAL 100
#define CONNECTS_CONNECT 100
#define CONNECTS_FDS 1024 * 10
#define CONNECTS_SIZE "5MB"


#define IPV4_INTERNAL "127.0.0.1"
#define IPV4_EXTERNAL {"0.0.0.0"}
#define IPV4_RESOLVER {"8.8.8.8", "8.8.4.4"}


#define IPV6_INTERNAL "::1"
#define IPV6_EXTERNAL {"::0"}
#define IPV6_RESOLVER {"2001:4860:4860::8888", "2001:4860:4860::8844"}


#define LDAP_VER 2
#define LDAP_SCOPE "one"
#define LDAP_USER_UID "uidNumber"
#define LDAP_USER_LOGIN "uid"
#define LDAP_USER_FIRST_NAME "givenName"
#define LDAP_USER_LAST_NAME "sn"
#define LDAP_USER_SECOND_NAME "initials"
#define LDAP_USER_DESCRIPTION "cn"
#define LDAP_USER_PASSWORD "userPassword"
#define LDAP_GROUP_GID "gidNumber"
#define LDAP_GROUP_LOGIN "cn"
#define LDAP_GROUP_DESCRIPTION "description"
#define LDAP_GROUP_PASSWORD "Password"
#define LDAP_GROUP_MEMBER "memberUid"


#define BUFFER_WRITE_SIZE "auto"
#define BUFFER_READ_SIZE "auto"


#define TIMEOUTS_READ "5s"
#define TIMEOUTS_WRITE "15s"
#define TIMEOUTS_UPGRADE "300s"


#define LOGS_ENABLED false
#define LOGS_FILES true
#define LOGS_CONSOLE true
#define LOGS_DATA false
#define LOGS_SIZE "1MB"
#define LOGS_DIR "/var/log"


#define KEEPALIVE_CNT 3
#define KEEPALIVE_IDLE 1
#define KEEPALIVE_INTVL 2


#define AUTH_GROUP_MAX_PAM 100
#define AUTH_UPDATE "10m"
#define AUTH_ENABLED false


#define GZIP_VARY false
#define GZIP_LENGTH 100
#define GZIP_CHUNK "4KB"
#define GZIP_REGEX "msie6"
#define GZIP_VHTTP {"1.0", "1.1"}
#define GZIP_PROXIED {"private", "no-cache", "no-store"}
#define GZIP_TYPES {"text/html", "text/css", "text/plain", "text/xml", "text/javascript", "text/csv"}


#define FIREWALL_MAX_TRYAUTH 10
#define FIREWALL_AUTH false
#define FIREWALL_LOOP true
#define FIREWALL_BLACK_LIST false
#define FIREWALL_WHITE_LIST false
#define FIREWALL_BANDLIMIN false
#define FIREWALL_TIME_LOOP "60s"
