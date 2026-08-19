#ifndef LTN_STRINGH
#define LTN_STRINGH
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
//#include <dirent.h>
#include <assert.h>
//#include "const.h"
//#include "ltn.h"
// SOCKET type definition is in ltn_tools.h (not needed here since socket functions are commented out)
#ifndef DELIMITER
#define DELIMITER "/"
#endif


class wString
{
private:
	unsigned int len;	//実際の長さ
	unsigned int capa;	//保存領域の長さ
	char*		 String;

	void replace_character_len(const char* sentence, int slen, const char* p, int klen, const char* rep);
	//void replace_str(const char* sentence, int slen, const char* p, int klen, const char* rep);
#ifndef va_copy
	int tsprintf_string(char* str);
	//int tsprintf_string(wString& str);
	int tsprintf_char(int ch);
	int tsprintf_decimal(signed long val, int zerof, int width);
	int tsprintf_decimalu(unsigned long val, int zerof, int width);
	int tsprintf_octadecimal(unsigned long val, int zerof, int width);
	int tsprintf_hexadecimal(unsigned long val, int capital, int zerof, int width);
	int vtsprintf(const char* fmt, va_list arg);
#endif
public:
	const static int npos;
	wString(void);
	explicit wString(int mylen);
	wString(const char* str);
	wString(const wString& str);
	~wString(void);
	void   resize(const int newSize);
	//OPERATION OVERLOAD
	wString  operator+(const wString& str) const;
	wString  operator+(const char* str) const;
	friend wString operator+(const char* str1, const wString& str2);
	void     operator=(const wString& str);
	void     operator=(const char* str);
	void     operator=(const int num);
	void     operator=(const double num);
	bool     operator==(const wString& str) const;
	bool     operator==(const char* str) const;
	bool     operator!=(const wString& str) const;
	bool     operator!=(const char* str) const;
	bool     operator>=(const wString& str) const;
	bool     operator>=(const char* str) const;
	bool     operator<=(const wString& str) const;
	bool     operator<=(const char* str) const;
	bool     operator> (const wString& str) const;
	bool     operator> (const char* str) const;
	bool     operator< (const wString& str) const;
	bool     operator< (const char* str) const;
	void     operator+=(const char ch);
	void     operator+=(const wString& str);
	void     operator+=(const char* str);
	char     operator[](int index) const;
	char&    operator[](int index);
	//STRING FUNCTION
	bool            starts_with(const char* str, int pos = -1) const;
	bool            ends_with(const char* str, int end_len = -1) const;
	void            set_binary(const void* str, int addLen);
	char            at(unsigned int index) const;
	wString         substr(int start, int mylen = -1) const;
	int             compare(const wString& str) const;
	int             compare(const char* str) const;
	void            clear(void);
	int             Pos(const wString& pattern, int pos = 0) const;
	int             Pos(const char* pattern, int pos = 0) const;
	unsigned int    copy(char* str, unsigned int slen, int index) const;
	wString&        replace(int index, unsigned int len, const wString& repstr);
	void            duplex_character_to_unique(char unique_char);
	bool            path_sanitize();
	wString         dump(void) const;

	unsigned int    size(void) const;
	unsigned int    length(void) const;
	unsigned int    capacity(void) const;
	bool            empty(void) const;
	char*			c_str(void) const;
	wString&		set_length(const unsigned int num);

	wString         ToUpper(void);
	wString         ToLower(void);
	wString         trim(void);
	wString         rtrim(void);
	wString         ltrim(void);
	void          rtrim_chr (const unsigned char cut_char);
	wString         ltrim_chr (const unsigned char cut_char);
	static void     rtrim_chr (char* sentence, const unsigned char cut_char = ' ');
	int             sprintf(const char* format, ...);
	int             cat_sprintf(const char* format, ...);
	int             last_delimiter(const char* delim) const;
///	wString         strsplit(const wString& str, const char* delimstr);
	wString         strsplit(const char* delimstr);
	int             find(const wString& str, int index = 0) const;
	int             find(const char* str, int index = 0) const;
	int             find(char ch, int index = 0) const;
	int             rfind(const wString& str, int index = 0) const;
	int             rfind(const char* str, int index = 0) const;
	int             rfind(char ch, int index = 0) const;
	int             strlen_utf8(char* str);
	//URL and encode
	static char* linux_file_name(char* FileName);
	static char* windows_file_name(char* FileName);
	//wString linux_file_name();
	wString windows_file_name();
	wString         uri_encode(void);
	wString         uri_decode(void);
	wString         htmlspecialchars(void);
	wString         add_slashes(void);
	wString         base64(void);
	wString         unbase64(void);
	static wString  escape(const wString& str);
	//ANISSTRING
	bool            insert_list_string(wString& src, int pos);
	bool            insert_list_string(const char* src, int pos);
	wString         get_list_string(const int pos);
	int             lines(void);
	void            reset_length(unsigned int num = 0);
	void            Add(const wString& str);
	void            Add(const char* str);
	//HEADER（socket不使用のためコメントアウト）
	void            init_header(size_t content_length, int expire = 1, const char* mime_type = "text/html");
	//void            send_header(SOCKET socket, int endflag);
	wString         headerPrintMem(void);
	int             header(const char* str, int flag = true, int status = 0);
	//FILE OPERATION
	static bool     rename_file(const wString& src, const wString& dst);
	static int      FileCopy(const wString& fname_r, const wString& fname_w);
	static int      delete_file(const wString& str);
	static int      delete_folder(const wString& str);
	static int      directory_exists(const char* str);
	static int      directory_exists(const wString& str);
	static wString  file_stats(const char* str, int mode = 0);
	static wString  file_stats(const wString& str, int mode = 0);
	static int      file_exists(const char* str);
	static int      file_exists(const wString& str);
	int             file_exists(void);
	static wString  extract_file_dir(const wString& str);
	static wString  extract_file_name(const char* str, const char* delim = DELIMITER);
	static wString  extract_file_name(const wString& str, const char* delim = DELIMITER);
	static wString  extract_file_ext(const wString& str);
	static wString  find_mime_type(const wString& file_name);
	static bool     create_dir(const wString& str);
	static wString  change_file_ext(const wString& str, const char* ext);
	static unsigned long file_size_by_name(char* str);
	static unsigned long file_size_by_name(wString& str);
	static wString  enum_folder(const wString& Path);
	static wString  enum_folder_json(const wString& Path);
	static wString  get_current_dir();
	static bool     check_url(const wString& url);
	static wString  png_size(const wString& png_filename);
	static wString  gif_size(const wString& gif_filename);
	static wString  jpeg_size(const wString& jpeg_filename);
	static wString  replace_env(const wString& path);
	static int		readLineCSV (int fd, char* line_buf_p, int line_max);
	wString  bios_uuid(void);
	int             is_number (const char* str);
	int             load_from_file(const char* str);
	int             load_from_file(const wString& FileName);
	bool            load_from_csv(const wString& FileName);
	void            load_from_csv(const char* str);
	int             save_to_file(const char* FileName);
	int             save_to_file(const wString& str);
	wString         nkfcnv(const wString& option);
	//int             line_receive(SOCKET accept_socket);  // socket不使用のためコメントアウト
	bool            cut_before_character(const char cut_char);
	bool            cut_after_character(const char cut_char);
	bool            cut_after_last_character(const char cut_char);
	bool            sentence_split(char cut_char, wString& split1, wString& split2);
	bool            replace_character(wString key, wString rep);
	wString         insert(int pos, const wString& fill);
	//
	char*			strtok_csv (char* str, int& ptr);
	//HTTP接続用（socket不使用のためコメントアウト）
	//static int      http_size(const wString& url);
	//static SOCKET   sock_connect(const wString& host, const int port);
	//static SOCKET   sock_connect(const char* host, const int port);
	//static wString  http_get(const wString& url, const off_t offset = 0);
	//static wString  http_rest(const wString& methods, const wString& url, const wString& data);
	//static wString  get_local_address(void);
	//static int      get_local_port (void);
};
extern wString current_dir;

/// CScriptException: TinyJS例外クラス（ltn_Stringから参照のためここで定義）
#ifndef CSCRIPTEXCEPTION_DEFINED
#define CSCRIPTEXCEPTION_DEFINED
class CScriptException {
public:
	wString text;
	explicit CScriptException(const wString& exceptionText) : text(exceptionText) {}
};
#endif

#endif
