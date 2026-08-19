#ifndef CONSTH
#define CONSTH
#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <netdb.h>
#include <string>
#else
#include <sys/stat.h>
#include <process.h>
//#include <dir.h>
#include <direct.h>
#include <io.h>
#include <winsock.h>
#include <time.h>
#include <sys/types.h>
#endif

#define DELIMITER          "/"
#define USERAGENT          "Birdland/LutinoV1.20"
#define HTTP_SERVER_PORT   (80)
#define MACADDR            (MAC_ADDR)

#ifdef MAINVAR
char MAC_ADDR[256] = {};
#else
extern char MAC_ADDR[256];
#endif



//CONSTANCES
#ifdef __linux__
typedef int HANDLE;
typedef int SOCKET;
typedef unsigned int DWORD;
#define strncmpi strncasecmp
#define O_BINARY 0
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SERROR(x) ((x) < 0 )

#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>

#else

typedef unsigned int pid_t;
typedef unsigned int ssize_t;
typedef int socklen_t;
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define strcasecmp strcmpi
#define strncasecmp strnicmp
#define strtoull strtoul
#define SERROR(x) ((x) == INVALID_SOCKET)
#endif

// ======================
// define いろいろ
// ======================
//WINSOCK2より
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
#define SERVER_NAME     "Lutino "
#define SERVER_DETAIL   "Application Server Daemon."
#ifndef TRUE
#define TRUE    (1)
#endif
#ifndef FALSE
#define FALSE   (0)
#endif

//#ifndef IGNORE_PARAMETER
//#define IGNORE_PARAMETER(n) ((void)n)
//#endif
//#ifndef FILENAME_MAX
//#define FILENAME_MAX 1024
//#endif
#ifndef QUERY_MAX
#define QUERY_MAX 1024
#endif


#define LISTEN_BACKLOG                  (32)
#define SEND_BUFFER_SIZE                (1024*128)
#define DEFAULT_SERVER_PORT             (8000)
#define DEFAULT_SERVER_TLS_PORT         (8443)
#define DEFAULT_DOCUMENT_ROOT           "/"
#define DEFAULT_FLAG_DAEMON             TRUE
//#define DEFAULT_FLAG_AUTO_DETECT        TRUE
#define DEFAULT_MIME_TYPE               "text/plain"

#ifdef __linux__
#define DEFAULT_CONF_FILENAME1          "./ltn.conf"
#else
#define DEFAULT_CONF_FILENAME1          "ltn.conf"
#endif
#define	DEFAULT_CONF_FILENAME2	        "/usr/local/lutino/ltn.conf"
#define	DEFAULT_CONF_FILENAME3	        "/etc/ltn.conf"

#define	DEFAULT_FLAG_DEBUG_LOG_OUTPUT	FALSE
#ifdef __linux__
#define	DEFAULT_DEBUG_LOG_FILENAME	    "/tmp/ltn_debug.log"
#else
#define DEFAULT_DEBUG_LOG_FILENAME      "ltn_debug.log"
#endif
#define DEFAULT_FLAG_USE_SKIN           TRUE

#define DEFAULT_SKINDATA_ROOT           "./skin"
#define DEFAULT_SKINDATA_NAME           "default"
#define DEFAULT_WORKROOT_NAME           "./work"
#define MAX_EVENTS                      100
#define MAX_BATCH_JOBS                  99
// execute path for CGI
#define	DEFAULT_PATH	"/usr/bin:/bin:/usr/sbin:/usr/bin"
#define	DEFAULT_FLAG_EXECUTE_CGI	TRUE
#define	DEFAULT_FLAG_ALLOW_PROXY	TRUE
// ltn.conf 変更対応
#define DEFAULT_ALLOW_USER_AGENT	"Mozilla"
#define DEFAULT_PC_PLAYLIST_OUT		FALSE
#define CODE_AUTO					(0)
#define CODE_SJIS					(1)
#define CODE_EUC					(2)
#define CODE_UTF8					(3)
#define CODE_UTF16					(4)
#define HTTP_USER_AGENT				"User-agent:"
#define HTTP_RANGE					"Range:"
#define HTTP_HOST					"Host:"
#define HTTP_CONTENT_LENGTH1	    "Content-Length:"
#define HTTP_CONTENT_TYPE1	        "Content-Type:"
#define HTTP_COOKIE					"Cookie:"
#define HTTP_REFERER				"Referer:"
#define HTTP_SEC_FETCH_DEST			"Sec-Fetch-Dest:"
#define HTTP_DELIMITER				"\r\n\r\n"

#define	HTTP_OK 	        		"HTTP/1.0 200 OK\r\n"
#define	HTTP_NOT_FOUND 				"HTTP/1.0 404 File Not Found\r\n"
#define HTTP_NOT_FOUND1     		"HTTP/1.x 404 Not Found\r\n"
//#ifdef __linux__
#define HTTP_CONTENT_LENGTH			"Content-Length: %zu\r\n"
//#else
//#define HTTP_CONTENT_LENGTH			"Content-Length: %lu\r\n"
//#endif
#define	HTTP_ACCEPT_RANGES			"Accept-Ranges: bytes\r\n"
#define HTTP_CONTENT_TYPE 			"Content-Type: %s\r\n"
#define	HTTP_SERVER_NAME			"Server: %s\r\n"
#define	HTTP_CONNECTION				"Connection: Close\r\n"
#define HTTP_ACCEPTRANGE			"Accept-Ranges: bytes\r\n"
#define HTTP_DATE				    "Date: %s\r\n"
#define HTTP_END				    "\r\n"

//MULTIPART
#define CONTENT_DISPOSITION          "Content-Disposition: form-data;"

//chashe
#define CACHE_CONTROL			     "Cache-Control: no-store,no-cache,must-revalidate,max-age=0\r\nPragma: no-cache\r\nExpires: -1\r\nVary: *\r\n"

// アクセスコントロール 登録可能数
#define		ACCESS_ALLOW_LIST_MAX		(32)
#define		ALLOW_USER_AGENT_LIST_MAX	(32)
// MIME_LIST_T.stream_type 用
enum class STREAM_TYPE
{
	TYPE_STREAM = 0,
	TYPE_NO_STREAM = 1,
	NOT_DEFINED = -1,
};
// MIME_LIST.menu_file_type用
enum class MIME_TYPE
{
	TYPE_UNKNOWN = 0,
	TYPE_DIRECTORY = 1,
	TYPE_MOVIE = 2,
	TYPE_MUSIC = 3,
	TYPE_IMAGE = 4,
	TYPE_DOCUMENT = 5,
	TYPE_PLAYLIST = 6,
	TYPE_MUSICLIST = 7,
	TYPE_URL = 8,
	TYPE_SCRIPT = 9,
	NOT_MIME = -1,
};

#define		NO_RESPONSE_TIMEOUT		(300)

#define		MULTI_ACCESS_MAX		(5)
#define		TIMEOUT_SECOND			(300)
#define		MAX_COUNT_ALIAS			(3)
#define     MAXTHREAD               (3)
extern int                      loop_flag;
//extern int                      ready_flag;                                   //Accept完了フラグ
extern SOCKET                   listen_socket;                                // 待ち受けSocket
extern SOCKET                   listen_socket_tls;                            // HTTPS待ち受けSocket
#endif
