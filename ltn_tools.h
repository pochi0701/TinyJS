// ==========================================================================
//code=UTF8	tab=4
//
// Lutino:	Application SErver.
//
// 		ltn_tools.h
//		$Revision: 1.7 $
//		$Date: 2004/03/10 04:47:55 $
//
// ==========================================================================
#ifndef	_LTN_TOOLS_H
#define	_LTN_TOOLS_H
#ifdef __linux__
#define O_BINARY                0
#define INVALID_SOCKET          (-1)
#define SOCKET_ERROR            (-1)
typedef int SOCKET;
#else
// socket不使用のためwinsock.hをインクルードしない
//#include <winsock.h>
#include <time.h>
#include <sys/types.h>
#include <io.h>
#include <fcntl.h>
#if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
typedef unsigned int SOCKET;
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)(~0))
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#endif
#define		OUTPUT_LOG_ROTATE_SIZE		(1024*1024*1900)	/* 1.9Gbyte */
#define		OUTPUT_LOG_ROTATE_MAX		5					/* 何回のrotateを許すか */
#include        "ltn_String.h"

// ==========================================================================
// 汎用オープン(OS/文字コード差異吸収)
// ==========================================================================
int myopen(const wString& filename, int amode, int option = 0);
// uriエンコード／デコード
//extern int      uri_decode(char* dst, unsigned int dst_len, const char* src, unsigned int src_len);

// テキスト処理イロイロ。
extern char*    seekCRLFCRLF(char* start, char* end);
extern void*    memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);
extern char*    cut_after_character(char* sentence, char cut_char);
extern void     cut_after_last_character(char* sentence, char cut_char);
extern void     ltrim(char* sentence, char cut_char = ' ');
extern void     rtrim(char* sentence, char cut_char = ' ');
extern int      sentence_split(char* sentence, char cut_char, char* split1, char* split2);
extern void     duplex_character_to_unique(char* sentence, char unique_char);
extern void     replace_character(char* sentence, const char* key, const char* rep);
//extern void     replace_character_first(char* sentence, const char* key, const char* rep);
extern void     make_datetime_string(char* sentence);
extern void     filename_to_extension(char* filename, char* extension_buf, unsigned int extension_buf_size);
//extern wString  insertStr(int pos, const wString& fill);
extern int      nkf(const char* in, char* out, size_t len, const char* options);
//extern int      HTTPDownload(char* src, char* dst, off_t offset);
// path から ./ と ../ を取り除く。dir に結果が格納される。
extern char*    path_sanitize(char* dir, size_t dir_size);
// DebugLog 出力
extern void     debug_log_initialize();
extern void     debug_log_output(const char* fmt, ...);

//システム処理（socket不使用のためコメントアウト）
extern int      sClose(SOCKET& socket);
//extern bool     transport_attach_tls(SOCKET socket);
//extern bool     transport_attach_tls_client(SOCKET socket, const char* server_name);
//extern void     transport_shutdown_all(void);
//extern int      transport_send(SOCKET socket, const char* buffer, unsigned int length, int mode);
//extern int      transport_recv(SOCKET socket, char* buffer, unsigned int length, int mode);
//extern int      transport_close(SOCKET& socket);
extern wString  GetAuthorization(const wString& AuthorizedString);

#ifdef __linux__
extern int      getTargetFile(const char* LinkFile, char* TargetFile);
extern void     Sleep(unsigned int milliseconds);

#endif
//char* mymalloc(size_t size);
//char* mycalloc(size_t size1, int num);
//void  myfree(char* ptr);
bool  split(const char* cut_char, wString& split1, wString& split2);
/// <summary>クラスmp3</summary>
class mp3 {
private:
	unsigned char   mp3_id3v1_flag;         // MP3 タグ 存在フラグ
	unsigned char   mp3_id3v1_title[128];   // MP3 曲名
	unsigned char   mp3_id3v1_album[128];   // MP3 アルバム名
	unsigned char   mp3_id3v1_artist[128];  // MP3 アーティスト
	unsigned char   mp3_id3v1_year[128];    // MP3 制作年度
	unsigned char   mp3_id3v1_comment[128]; // MP3 コメント
public:
	//int init;
	mp3() {
		mp3_id3v1_flag = 0;
		*mp3_id3v1_title = 0;
		*mp3_id3v1_album = 0;
		*mp3_id3v1_artist = 0;
		*mp3_id3v1_year = 0;
		*mp3_id3v1_comment = 0;
	};
	wString mp3_id3_tag(const char* filename);
	wString mp3_id3_tag(const wString& filename);
	int     mp3_id3_tag_read(const char* mp3_filename);
	int     mp3_id3v1_tag_read(const char* mp3_filename);
	int     mp3_id3v2_tag_read(const char* mp3_filename);
	unsigned int id3v2_len(const unsigned char* buf);
};
extern struct tm* gmtime_r(const time_t* timer, struct tm* tmbuf);
extern struct tm* localtime_r(const time_t* timer, struct tm* tmbuf);
extern wString* asctimew(const struct tm* tm);
extern wString* ctimew(const time_t* timer);
#endif
