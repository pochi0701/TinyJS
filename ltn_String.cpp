// ==========================================================================
//code=UTF8	tab=4
//
// Lutino:	Application SErver.
//
// 		ltn_String.cpp
//		$Revision: 1.0 $
//		$Date: 2018/02/12 21:11:00 $
//
// ==========================================================================
//---------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/types.h>
#include <vector>
#include <algorithm>
#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#define WIN32_LEAN_AND_MEAN   // winsock2.hが自動インクルードされないようにする
#include <windows.h>
#include <stdarg.h>
#include <process.h>
#include "dirent.h"
//#include <dir.h>
#include <direct.h>
#include <io.h>
typedef int ssize_t;  // Windows用 ssize_t定義
#endif
#include "ltn_String.h"
//#include "TinyJS.h"  -- CScriptExceptionはltn_String.hで定義
//#include <stdexcept>
//#include "ltn.h"         // socket/global_param不使用のためコメントアウト
#include "ltn_tools.h"
#include <time.h>
#include "define.h"
#include <fstream>
//#include "libnkf.hpp"    // libnkf不使用のためコメントアウト
//#include <ipmib.h>       // socket不使用のためコメントアウト
//#include <iphlpapi.h>    // socket不使用のためコメントアウト

// HTTP通信関連anonymous namespaceはsocket不使用のためコメントアウト
/*
namespace {
struct HttpUrlParts {
	bool    use_tls = false;
	wString host;
	wString target;
	int     port = HTTP_SERVER_PORT;
};

bool parse_http_url(const wString& url, HttpUrlParts& info)
{
	if (url.starts_with("https://")) {
		info.use_tls = true;
		info.port = 443;
	}
	else if (url.starts_with("http://")) {
		info.use_tls = false;
		info.port = HTTP_SERVER_PORT;
	}
	else {
		return false;
	}

	int host_pos = url.Pos("://");
	if (host_pos < 0) {
		return false;
	}
	host_pos += 3;

	int path_pos = url.Pos("/", host_pos);
	wString authority;
	if (path_pos >= 0) {
		info.target = url.substr(path_pos, url.length() - path_pos);
		authority = url.substr(host_pos, path_pos - host_pos);
	}
	else {
		info.target = "/";
		authority = url.substr(host_pos, url.length() - host_pos);
	}

	int port_pos = authority.Pos(":");
	if (port_pos >= 0) {
		info.port = atoi(authority.c_str() + port_pos + 1);
		info.host = authority.substr(0, port_pos);
	}
	else {
		info.host = authority;
	}

	return info.host.length() > 0;
}

SOCKET open_http_socket(const HttpUrlParts& info)
{
	auto socket = wString::sock_connect(info.host.c_str(), info.port);
	if (SERROR(socket)) {
		return INVALID_SOCKET;
	}
	if (info.use_tls && !transport_attach_tls_client(socket, info.host.c_str())) {
		sClose(socket);
		return INVALID_SOCKET;
	}
	return socket;
}

bool read_http_response(SOCKET socket, wString& response)
{
	char buffer[4096];
	response.clear();
	while (loop_flag) {
		auto recv_len = transport_recv(socket, buffer, sizeof(buffer), 0);
		if (recv_len > 0) {
			response.set_binary(buffer, recv_len);
		}
		else if (recv_len == 0) {
			return true;
		}
		else {
			return response.length() > 0;
		}
	}
	return response.length() > 0;
}

int get_http_status_code(const wString& response)
{
	auto pos = response.Pos(" ");
	if (pos < 0) {
		return -1;
	}
	return atoi(response.c_str() + pos + 1);
}

bool split_http_response(const wString& response, wString& body)
{
	auto pos = response.Pos(HTTP_DELIMITER);
	if (pos == wString::npos) {
		return false;
	}
	body = response.substr(pos + 4);
	return true;
}
}
*/

#ifndef strrstr
///---------------------------------------------------------------------------
/// <summary>
/// strrstr()
///   文字列 から 文字列 を検索する。
///   最後 に現れた文字列の先頭を指す ポインタ を返す。
///   文字列が見つからない場合は null を返す。
///   pattern が 空文字列 の場合は常に 文字列終端 を返す。
///   文字列終端文字 '\0' は検索対象とならない。
///   半角英字 の 大文字と小文字 を区別する。
/// </summary>
/// <param name="string">検索対象となる文字列</param>
/// <param name="pattern">文字列から検索する文字列</param>
/// <returns></returns>
char* strrstr (const char* string, const char* pattern)
{
	// 文字列終端に達するまで検索を繰り返す。
	const char* last = NULL;
	for (const char* p = string; ; p++) {
		if (0 == *p) {
			return (char*)last;
		}
		p = strstr (p, pattern);
		if (p == NULL) {
			break;
		}
		last = p;
	}
	return (char*)last;
}//strrstr
#endif

/// <summary>検索失敗時の値</summary>
const int wString::npos = (int)(-1);
///---------------------------------------------------------------------------
/// <summary>
/// 文字列クラス　ディフォールトコンストラクタ
/// </summary>
wString::wString (void)
{
	//初期化
	len = 0;
	capa = 1;
	String = static_cast<char*>(new char[1]);
	*String = 0;
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列クラス文字数指定（内容は０）
/// </summary>
/// <param name="mylen">初期文字数指定</param>
wString::wString (int mylen)
{
	//初期化
	if (mylen < 0) mylen = 0;
	len = 0;
	//末尾０の分
	capa = mylen + 1;
	String = static_cast<char*>(new char[mylen + 1]);
	*String = 0;
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列クラス文字列コンストラクタ
/// </summary>
/// <param name="str">元となる文字列</param>
wString::wString (const char* str)
{
	//初期化
	len = static_cast<unsigned int>(strlen (str));
	if (len) {
		capa = len + 1;
		String = static_cast<char*>(new char[capa]);
		strcpy (String, str);
	}
	else {
		capa = 1;
		String = static_cast<char*>(new char[1]);
		String[0] = 0;
	}
}
///---------------------------------------------------------------------------
/// <summary>
/// コピーコンストラクタ
/// </summary>
/// <param name="str">コピーする文字列</param>
wString::wString (const wString& str)
{
	//初期化
	len = str.len;
	if (str.len) {
		capa = str.capa;
		String = new char[str.capa];
		memcpy (String, str.String, str.capa);
		//*String = 0;
	}
	else {
		capa = 1;
		String = static_cast<char*>(new char[1]);
		String[0] = 0;
	}
}
///---------------------------------------------------------------------------
/// <summary>
/// デストラクタ
/// </summary>
wString::~wString ()
{
	delete[] String;
}
///---------------------------------------------------------------------------
/// <summary>
/// プラスメソッドa+b
/// </summary>
/// <param name="str">プラスするwString</param>
/// <returns>プラス結果</returns>
wString wString::operator+(const wString& str) const
{
	wString temp (*this);
	temp += str;
	return temp;
}
///---------------------------------------------------------------------------
/// <summary>
/// プラスメソッドa+b
/// </summary>
/// <param name="str">プラスする文字列</param>
/// <returns>プラス結果</returns>
wString wString::operator+(const char* str) const
{
	wString temp (*this);
	temp += str;
	return temp;
}
///---------------------------------------------------------------------------
/// <summary>
/// プラスメソッド+(a,b)
/// </summary>
/// <param name="str1">元文字列</param>
/// <param name="str2">プラスするwString</param>
/// <returns>プラス結果</returns>
wString operator+(const char* str1, const wString& str2)
{
	wString temp (str1);
	temp += str2;
	return temp;
}
///---------------------------------------------------------------------------
/// <summary>
/// プラスメソッドa+=b
/// </summary>
/// <param name="str">プラスするwString</param>
void wString::operator+=(const wString& str)
{
	unsigned int newLen = len + str.len;
	resize (newLen + 1);
	memcpy (String + len, str.String, str.len);
	String[newLen] = 0;
	len = newLen;
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// プラスメソッドa+=b
/// </summary>
/// <param name="str">プラスする文字列</param>
void wString::operator+=(const char* str)
{
	auto slen = static_cast<unsigned int>(strlen (str));
	auto newLen = slen + len;
	resize (newLen);
	strcpy (String + len, str);
	len = newLen;
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// バイナリ文字列の設定a.set_binary(mem,len)
/// </summary>
/// <param name="mem">設定バイナリ</param>
/// <param name="addLen">設定長さ</param>
void wString::set_binary (const void* mem, int binaryLength)
{
	auto newLen = binaryLength + len;
	resize (newLen);
	memcpy (static_cast<void*>(String + len), mem, binaryLength);
	len = newLen;
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// 先頭文字列の比較（バイナリ非対応）
/// </summary>
/// <param name="needle">検索する文字列</param>
/// <param name="pos">検査開始位置</param>
/// <returns>先頭と一致すればtrue</returns>
bool wString::starts_with (const char* needle, int pos) const
{
	auto nlen = strlen (needle);
	if (nlen == 0 || nlen > len) {
		return false;
	}
	if (pos >= 0) {
		if (pos >= len - nlen + 1) {
			return false;
		}
		return memcmp (String + pos, needle, nlen) == 0;
	}
	else {
		return memcmp(String, needle, nlen) == 0;
	}
}
//---------------------------------------------------------------------------
/// <summary>
/// 末尾文字列の比較
/// 非バイナリ
/// </summary>
/// <param name="needle">検索する文字列</param>
/// <returns>先頭と一致すればtrue</returns>
bool wString::ends_with (const char* needle, int end_len) const
{
	/// TODO:end_lenは無意味
	int temp_len = len;
	if (end_len >= 0) {
		if (end_len == 0 || end_len > temp_len) {
			return false;
		}
		//temp_len = end_len;
	}
	auto nlen = static_cast<unsigned int>(strlen (needle));
	if (nlen == 0 || nlen > len) {
		return false;
	}
	return memcmp(String + len - nlen, needle,nlen) == 0;
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列に1バイト文字加算
/// バイナリ対応
/// </summary>
/// <param name="ch">加算する1バイト文字</param>
void wString::operator+=(const char ch)
{
	int tmpl = ((len >> 4) + 1) << 4;
	resize (tmpl);
	String[len++] = ch;
	String[len] = 0;
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// wStringとwStringの比較
/// 非バイナリ
/// </summary>
/// <param name="str">比較する文字列</param>
/// <returns>ture:同一、false:異なる場合</returns>
bool wString::operator==(const wString& str) const
{
	if (len != str.len) {
		return false;
	}
	return(strncmp (String, str.String, len) == 0);
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列の比較
/// 非バイナリ
/// </summary>
/// <param name="str">比較する文字列</param>
/// <returns>true:同一、false:不一致</returns>
bool wString::operator==(const char* str) const
{
	auto mylen = static_cast<unsigned int>(strlen (str));
	if (len != mylen) {
		return false;
	}
	return(strncmp (String, str, len) == 0);
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列の比較
/// 非バイナリ
/// </summary>
/// <param name="str">比較する文字列</param>
/// <returns>true:不一致、false:一致</returns>
bool wString::operator!=(const wString& str) const
{
	if (len != str.len) {
		return true;
	}
	return(strncmp (String, str.String, len) != 0);
}
//---------------------------------------------------------------------------
bool wString::operator!=(const char* str) const
{
	if (len != strlen (str)) {
		return true;
	}
	return(strncmp (String, str, len) != 0);
}
//---------------------------------------------------------------------------
bool wString::operator>=(const wString& str) const
{
	unsigned int maxlen = (len > str.len) ? len : str.len;
	return(strncmp (String, str.String, maxlen) >= 0);
}
//---------------------------------------------------------------------------
bool wString::operator>=(const char* str) const
{
	auto mylen = static_cast<unsigned int>(strlen (str));
	auto maxlen = (len > mylen) ? len : mylen;
	return(strncmp (String, str, maxlen) >= 0);
}
//---------------------------------------------------------------------------
bool wString::operator<=(const wString& str) const
{
	unsigned int maxlen = (len > str.len) ? len : str.len;
	return(strncmp (String, str.String, maxlen) <= 0);
}
//---------------------------------------------------------------------------
bool wString::operator<=(const char* str) const
{
	auto mylen = static_cast<unsigned int>(strlen (str));
	auto maxlen = (len > mylen) ? len : mylen;
	return(strncmp (String, str, maxlen) <= 0);
}
//---------------------------------------------------------------------------
bool wString::operator>(const wString& str) const
{
	auto maxlen = (len > str.len) ? len : str.len;
	return(strncmp (String, str.String, maxlen) > 0);
}
//---------------------------------------------------------------------------
bool wString::operator>(const char* str) const
{
	auto mylen = static_cast<unsigned int>(strlen (str));
	auto maxlen = (len > mylen) ? len : mylen;
	return(strncmp (String, str, maxlen) > 0);
}
//---------------------------------------------------------------------------
bool wString::operator<(const wString& str) const
{
	auto maxlen = (len > str.len) ? len : str.len;
	return(strncmp (String, str.String, maxlen) < 0);
}
//---------------------------------------------------------------------------
bool wString::operator<(const char* str) const
{
	auto mylen = static_cast<unsigned int>(strlen (str));
	auto maxlen = (len > mylen) ? len : mylen;
	return(strncmp (String, str, maxlen) < 0);
}
//---------------------------------------------------------------------------
void wString::operator=(const wString& str)
{
	resize (str.capa);
	memcpy (String, str.String, str.capa);
	len = str.len;
	return;
}
//---------------------------------------------------------------------------
void wString::operator=(const char* str)
{
	auto newLen = static_cast<unsigned int>(strlen (str));
	resize (newLen);
	strcpy (String, str);
	len = newLen;
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// 整数の文字列化
/// </summary>
/// <param name="num">文字列にする整数</param>
void wString::operator=(const int num)
{
	this->sprintf ("%d", num);
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// 実数の文字列化
/// </summary>
/// <param name="num">文字列にする実数</param>
void wString::operator=(const double num)
{
	this->sprintf ("%f", num);
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列の配列アクセス
/// バイナリ対応
/// </summary>
/// <param name="index">参照する添え字</param>
/// <returns>参照した1バイトの値</returns>
char wString::operator[](int index) const
{
	if (index >= 0 && index < static_cast<int>(len)) {
		return String[index];
	}
	else {
		throw new CScriptException ("Out bound.Operator[].");
		//perror("out bound");
		//return 0;
	}
}

/// <summary>
/// 文字列の配列アクセス左辺値
/// </summary>
/// <param name="index">参照する添え字</param>
/// <returns>参照した1バイトの実体</returns>
char& wString::operator[](int index)
{
	return String[index];
}

///---------------------------------------------------------------------------
/// <summary>
/// 文字列の位置指定アクセス
/// </summary>
/// <param name="index">参照する添え字</param>
/// <returns>参照した1バイトの値</returns>
char wString::at (unsigned int index) const
{
	if (index < len) {
		return String[index];
	}
	else {
		throw new CScriptException ("Out bound.at().");
		//perror ("out bound");
		//return -1;
	}
}
//---------------------------------------------------------------------------
wString& wString::set_length (const unsigned int num)
{
	resize (num);
	return *this;
}
//---------------------------------------------------------------------------
// 比較
//---------------------------------------------------------------------------
int wString::compare (const wString& str) const
{
	return strcmp (String, str.String);
}
//---------------------------------------------------------------------------
// 比較
//---------------------------------------------------------------------------
int wString::compare (const char* str) const
{
	return strcmp (String, str);
}
//---------------------------------------------------------------------------
// クリア
//---------------------------------------------------------------------------

/// <summary>
/// 文字列のクリア
/// 設定した領域は解放しない
/// バイナリ対応
/// </summary>
void  wString::clear (void)
{
	len = 0;
	if (String != nullptr) {
		*String = 0;
	}
}
//---------------------------------------------------------------------------
// 部分文字列
//---------------------------------------------------------------------------

///// <summary>
///// 部分文字列
///// バイナリ対応
///// </summary>
///// <param name="start">開始位置(0スタート)</param>
///// <param name="mylen">指定バイト数。0以下で残り全文字列</param>
///// <returns>指定した文字列</returns>
//wString  wString::SubString(int start, int mylen) const
//{
//	// TODO エラーチェック
//	if (start < 0 || start >= static_cast<int>(len)) {
//		//throw error
//	}
//	if (mylen <= 0 || start + mylen > (int)len) {
//		mylen = (int)len - start;
//		// if( mylen < 0 )
//	}
//	wString temp(mylen);
//	if (mylen > 0) {
//		memcpy(temp.String, String + start, mylen);
//		temp.String[mylen] = 0;
//		//長さ不定。数えなおす
//		temp.len = mylen;
//	}
//	return temp;
//}
//---------------------------------------------------------------------------
// substr
//---------------------------------------------------------------------------
/// <summary>
/// 部分文字列抽出
/// バイナリ対応
/// </summary>
/// <param name="start">開始バイト</param>
/// <param name="cut_len">切り取りバイト数</param>
/// <returns>抽出した文字列</returns>
wString wString::substr (int start, int cut_len) const
{
	// TODO エラーチェック
	if (start < 0 || start >= static_cast<int>(len)) {
		// TODO:throw error
	}
	if (cut_len < 0) cut_len = len;
	if (start + cut_len > (int)len) {
		cut_len = (int)len - start;
	}

	wString temp (cut_len);
	if (cut_len > 0) {
		memcpy (temp.String, String + start, cut_len);
		temp.String[cut_len] = 0;
		//長さ不定。数えなおす
		temp.len = cut_len;
	}
	return temp;
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
int wString::find (const wString& str, int index) const
{
	const char* ptr = strstr (String + index, str.String);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return static_cast<int>((ptr - String));
	}
}
///---------------------------------------------------------------------------
/// 位置
///---------------------------------------------------------------------------
/// <summary>
/// 文字列の検索
/// </summary>
/// <param name="ch">検索文字列</param>
/// <param name="index">検索開始位置</param>
/// <returns>見つかった場合先頭からの文字位置（0スタート)、見つからなかった場合wString:npos</returns>
int wString::find (const char* str, int index) const
{
	const char* ptr = strstr (String + index, str);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return static_cast<int>((ptr - String));
	}
}
/// <summary>
/// 文字の検索
/// </summary>
/// <param name="ch">検索文字</param>
/// <param name="index">検索開始位置</param>
/// <returns>見つかった場合先頭からの文字位置（0スタート)、見つからなかった場合wString:npos</returns>
int wString::find (char ch, int index) const
{
	const char* ptr = strchr (String + index, ch);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return static_cast<int>((ptr - String));
	}
}
/// <summary>
/// 行末からの文字列の検索
/// </summary>
/// <param name="ch">検索文字</param>
/// <param name="index">検索開始位置</param>
/// <returns>見つかった場合先頭からの文字位置（0スタート)、見つからなかった場合wString:npos</returns>
int wString::rfind (const wString& str, int index) const
{
	const char* ptr = strrstr (String + index, str.String);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return static_cast<int>((ptr - String));
	}
}
///---------------------------------------------------------------------------
/// <summary>
/// 行末からの検索
/// </summary>
/// <param name="str">検索文字列</param>
/// <param name="index">検索開始位置(省略可能)</param>
/// <returns>成功：文字位置 失敗:wString::npos</returns>
int wString::rfind (const char* str, int index) const
{
	const char* ptr = strrstr (String + index, str);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return static_cast<int>((ptr - String));
	}
}
///---------------------------------------------------------------------------
/// <summary>
/// 行末からの検索
/// </summary>
/// <param name="ch">検索文字</param>
/// <param name="index">検索開始位置(省略可能)</param>
/// <returns>成功：文字位置 失敗:wString::npos</returns>
int wString::rfind (char ch, int index) const
{
	const char* ptr = strrchr (String + index, ch);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return static_cast<int>((ptr - String));
	}
}

/********************************************************************************/
// sentence文字列内のunique_charが連続しているところを、unique_char1文字だけにする。
/********************************************************************************/
void wString::duplex_character_to_unique(char unique_char)
{
	for(int i = 0; i < len - 1; i++) {
		if (String[i] == unique_char && String[i + 1] == unique_char) {
			// 連続している場合、後ろの文字を削除
			memmove(String + i + 1, String + i + 2, len - i - 1);
			len--;
			String[len] = '\0'; // 終端文字を更新
			i--; // 次の文字もチェックするためにインデックスを戻す
		}
	}
	//wString fromStr;
	//wString toStr;
	//fromStr = unique_char;
	//fromStr += unique_char;
	//toStr = unique_char;
	//replace_character(fromStr, toStr);
	return;
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
//int wString::Pos(const char* pattern)
//{
//    char* ptr = strstr(String,pattern);
//    if( ptr == NULL ){
//        return npos;
//    }else{
//        return (int)(ptr-String);
//    }
//}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------

/// <summary>
/// 文字列中の位置
/// </summary>
/// <param name="pattern">存在を検査する文字列</param>
/// <param name="pos">検査開始位置</param>
/// <returns>見つからない場合wString::npos,存在する場合、先頭を０とした位置情報</returns>
int wString::Pos (const char* pattern, int pos) const
{
	const char* ptr = strstr (String + pos, pattern);
	if (ptr == NULL) {
		return npos;
	}
	else {
		return (int)(ptr - String);
	}
}

//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
int wString::Pos (const wString& pattern, int pos) const
{
	return Pos (pattern.String, pos);
}
//---------------------------------------------------------------------------
//　Size
//---------------------------------------------------------------------------

/// <summary>
/// 文字長
/// </summary>
/// <returns>文字列の長さ</returns>
unsigned int wString::size (void) const
{
	return len;
}

/// <summary>
/// 文字長
/// </summary>
/// <param name=""></param>
/// <returns>文字列の長さ</returns>
unsigned int wString::length (void) const
{
	return len;
}

/// <summary>
/// 文字列が空かどうか判定
/// </summary>
/// <param name=""></param>
/// <returns>空の場合true、それ以外false</returns>
bool wString::empty (void) const
{
	return len == 0;
}

/// <summary>
/// ファイル読み込み
/// </summary>
/// <param name="str">読み込むファイルのフルファイル名</param>
int wString::load_from_file (const char* str)
{
	return load_from_file (wString (str));
}
//---------------------------------------------------------------------------
// ファイル読み込み
//---------------------------------------------------------------------------
wString wString::replace_env(const wString& path)
{
	//文字列から%%で囲まれた部分を抽出
	wString result = path;
	int start = 0;

	while ((start = result.find('%', start)) != wString::npos) {
		int end = result.find('%', start + 1);
		if (end == wString::npos) {
			break;  // 閉じる % がない場合は終了
		}

		wString varName = result.substr(start + 1, end - start - 1);
		char* ptr  = getenv(varName.c_str());
		wString varValue = ptr;

		if (varValue.length()) {
			varValue = varValue.nkfcnv("sW");
			result.replace(start, end - start + 1, varValue);
		}
		else {
			result.replace(start, end - start + 1, "");  // 未定義の環境変数は削除
		}
	}
	return result;
}

/// <summary>
/// ファイル読み込み
/// </summary>
/// <param name="FileName">ファイル名（フルパス）またはURL</param>
/// <returns>０</returns>
int wString::load_from_file (const wString& FileName)
{
	long flen;
	int  handle;
	// http_get不使用のためHTTP読み込みブランチをコメントアウト
	//if (FileName.starts_with ("http://") || FileName.starts_with ("https://")) {
	//	wString tmp;
	//	tmp = wString::http_get (const_cast<char*>(FileName.c_str ()));
	//	*this = tmp;
	//}
	//else {
	{
		wString FileName2 = replace_env(FileName);
#ifdef __linux__
		handle = open (FileName2.c_str (), O_RDONLY | S_IREAD);
#else
		handle = myopen (FileName2, O_RDONLY | O_BINARY, S_IREAD);
#endif
		if (handle < 0) {
			debug_log_output ("%s(%d):load_from_file(%s) Error.", __FILE__, __LINE__, FileName2.c_str ());
			return -1;
		}
		flen = lseek (handle, 0, SEEK_END);
		lseek (handle, 0, SEEK_SET);
		set_length (flen + 1);
		len = read (handle, String, flen);
		close (handle);
		// アプリケーションレベルでやるべき
		//// BOM?
		//// 「BOM」は「バイトオーダーマーク」の略で、
		//// テキストファイルの文字コードが「UTF - 8」であることを示すものです。
		//// UTF - 8の場合、テキストデータの先頭に「0xEF 0xBB 0xBF」が付きます。
		//if (len >= 3) {
		//	if (this->starts_with ("\xef\xbb\xbf")) {
		//		// 先頭3バイトを削る
		//		len -= 3;
		//		memmove (String, String + 3, len);
		//	}
		//}
		String[len] = 0;
		//\0がある場合を考えればstrlenとってはいけない
		//len = strlen(String);
	}
	return 0;
}

/// <summary>
/// CSVファイル読み込み
/// </summary>
/// <param name="str">CSVファイル名</param>
void wString::load_from_csv (const char* str)
{
	load_from_csv (wString (str));
}

/// <summary>
/// 文字列の数値判断(0123456789.-+)
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
int wString::is_number (const char* str)
{
	for (auto i = static_cast<int>(strlen (str)) - 1; i >= 0; i--) {
		auto ch = static_cast<unsigned char>(str[i]);
		if ((!isdigit (ch)) && ch != '.' && ch != '-' && ch != '+') {
			return 0;
		}
	}
	return 1;
}

/// <summary>
/// CSV用のstrtok
/// </summary>
/// <param name="str">対象文字列</param>
/// <param name="ptr">切り出しポインター</param>
/// <returns>切り出し文字列</returns>
char* wString::strtok_csv (char* str, int& ptr)
{
	int start = ptr;
	char* token = str + start;
	int quoted = 0;
	if (ptr && str[ptr] == 0) {
		return NULL;
	}
	// 区切り文字または\0まで
	while (str[ptr]) {
		if (str[ptr] == '\"') {
			quoted++;
		}else if ((quoted & 1) == 0 && str[ptr] == ',') {
			break;
		}
		// 1文字増加
		ptr++;
	}

	str[ptr++] = 0;
	//先頭と末尾の"を除去
	if (token[0] == '\"' && token[ptr - start-2] == '\"') {
		token[ptr - start-2] = '\0';
		token++;
	}
	return token;
}

/// <summary>
/// fdから、１行(CRLFか、LF単独が現れるまで)受信
/// CRLFは削除する。
/// CSV用にダブルクォーテーションをエスケープする
/// 受信したサイズをreturnする。
/// \rは無視する
/// </summary>
/// <param name="fd">ファイルディスクリプタ</param>
/// <param name="line_buf_p">ラインバッファ</param>
/// <param name="line_max">最大文字数</param>
/// <returns></returns>
int wString::readLineCSV (int fd, char* line_buf_p, int line_max)
{
	char byte_buf;
	int  line_len = 0;
	int  quoted = 0;
	// １行受信実行
	while (1) {
		auto recv_len = read (fd, &byte_buf, 1);
		if (recv_len != 1) { // 受信失敗チェック
			return (-1);
		}
		// CR/LFチェック
		if (byte_buf == '\"') {
			quoted++;
		}else if ((quoted & 1) == 0) {
			if (byte_buf == '\r') {
				continue;
			}
			else if (byte_buf == '\n') {
				*line_buf_p = 0;
				break;
			}
		}

		// バッファにセット
		*line_buf_p++ = byte_buf;
		// 受信バッファサイズチェック
		if (++line_len >= line_max) {
			// バッファオーバーフロー検知
			return (-1);
		}
	}
	return line_len;
}



/// <summary>
/// CSVファイル読み込み
/// 文字コードはファイル依存
/// 1行目はタイトル行固定
/// </summary>
/// <param name="FileName">CSVファイル名</param>
bool wString::load_from_csv (const wString& FileName)
{
	int  fd;
	char s[1024] = {};
	char t[1024] = {};
	int first = 1;
	fd = myopen (FileName, O_RDONLY | O_BINARY, S_IREAD);
	if (fd < 0) {
		debug_log_output ("%s(%d):load_from_csv(%s) Error.", __FILE__, __LINE__, FileName.c_str ());
		return false;
	}

	wString work;

	work = "[";
	//１行目はタイトル
	while (true) {
		auto ret = readLineCSV (fd, s, sizeof (s));
		if (ret < 0) break;
		//分解する
		int ptr = 0;
		int ptr2 = 0;
		char* p;
		char* comma = const_cast <char*>("");
		while ((p = strtok_csv (s, ptr2)) != 0) {
			if (is_number (p)) {
				ptr += ::sprintf (t + ptr, "%s%s", comma, p);
			}
			else {
				// ""->\"
				// cr ->なし
				// lf ->"\n"
				ptr += ::sprintf (t + ptr, "%s\"%s\"", comma, p);
			}
			comma = const_cast <char*>(",");
		}
		if (first) {
			first = 0;
		}
		else {
			work += ",";
		}
		work += wString ("[") + t + "]";
	}
	work += "]";
	*this = work;
	close (fd);
	return true;
}
//---------------------------------------------------------------------------
// ファイル書き込み
//---------------------------------------------------------------------------
int wString::save_to_file (const char* str)
{
	return save_to_file (wString (str));
}
//---------------------------------------------------------------------------
// ファイル書き込み
//---------------------------------------------------------------------------
int wString::save_to_file (const wString& FileName)
{
	wString FileName2 = replace_env(FileName);
#ifdef __linux__
	int handle = myopen (FileName2, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
#else
	int handle = myopen (FileName2, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IREAD | S_IWRITE);
#endif
	if (handle < 0) {
		debug_log_output ("%s(%d):save_to_file(%s) Error.", __FILE__, __LINE__, FileName2.c_str ());
		return handle;
	}
	write (handle, String, len);
	close (handle);
	return 0;
}
//---------------------------------------------------------------------------
// 大文字化
//---------------------------------------------------------------------------
wString wString::ToUpper (void)
{
	wString temp (*this);
	for (auto i = 0U; i < length (); i++) {
		temp.String[i] = (char)toupper (temp.String[i]);
	}
	return temp;
}
//---------------------------------------------------------------------------
// 大文字化
//---------------------------------------------------------------------------
wString wString::ToLower (void)
{
	wString temp (*this);
	for (auto i = 0U; i < length (); i++) {
		temp.String[i] = (char)tolower (temp.String[i]);
	}
	return temp;
}
//---------------------------------------------------------------------------
// トリム
//---------------------------------------------------------------------------
wString wString::trim (void)
{
	wString temp (*this);
	if (temp.len) {
		//先頭の空白等を抜く
		while (temp.len && *reinterpret_cast<unsigned char*>(temp.String) <= ' ') {
#ifdef __linux__
			char* src = temp.String;
			char* dst = src + 1;
			while (*src) {
				*src++ = *dst++;
			}
#else
			strcpy (temp.String, temp.String + 1);
#endif
			temp.len--;
		}
		//末尾の空白等を抜く
		while (temp.len && reinterpret_cast<unsigned char*>(temp.String)[temp.len - 1] <= ' ') {
			temp.String[--temp.len] = 0;
		}
	}
	return temp;
}
//---------------------------------------------------------------------------
// トリム
//---------------------------------------------------------------------------
wString wString::rtrim (void)
{
	wString temp (*this);
	if (temp.len) {
		//末尾の空白等を抜く
		while (temp.len && reinterpret_cast<unsigned char*>(temp.String)[temp.len - 1] <= ' ') {
			temp.String[--temp.len] = 0;
		}
	}
	return temp;
}
///---------------------------------------------------------------------------
/// <summary>
/// sentence文字列の行末に、cut_charがあったとき、削除
/// </summary>
/// <param name="sentence">対象文字列</param>
/// <param name="cut_char">削除文字列（省略値スペース）</param>
void wString::rtrim_chr (char* sentence, unsigned char cut_char)
{
	if (sentence == NULL || *sentence == 0) return;
	char* source_p;
	auto slength = static_cast<int>(strlen (sentence));		// 文字列長Get
	source_p = sentence;
	source_p += slength;									// ワークポインタを文字列の最後にセット。
	for (auto i = 0; i < slength; i++) {					// 文字列の数だけ繰り返し。
		source_p--;											// 一文字ずつ前へ。
		if (*source_p == cut_char) {						// 削除キャラ ヒットした場合削除
			*source_p = '\0';
		}
		else {												// 違うキャラが出てきたところで終了。
			break;
		}
	}
	return;
}
///---------------------------------------------------------------------------
/// <summary>
/// 文字列の行末に、cut_charがあったとき、削除
/// </summary>
/// <param name="cut_char">対象文字列</param>
void wString::rtrim_chr (unsigned char cut_char)
{
	if (len) {
		//末尾の空白等を抜く
		while (len && String[len - 1] == cut_char) {
			String[--len] = 0;
		}
	}
	return;
}
/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
wString wString::ltrim_chr (unsigned char cut_char)
{
	wString temp (*this);
	if (temp.len) {
		//先頭の空白等を抜く
		while (temp.len && *reinterpret_cast<unsigned char*>(temp.String) == cut_char) {
			char* src = temp.String;
			char* dst = src + 1;
			while (*src) {
				*src++ = *dst++;
			}
			temp.len--;
		}
	}
	return temp;
}
//---------------------------------------------------------------------------
// トリム
//---------------------------------------------------------------------------
wString wString::ltrim (void)
{
	wString temp (*this);
	if (temp.len) {
		//先頭の空白等を抜く
		while (temp.len && *reinterpret_cast<unsigned char*>(temp.String) <= ' ') {
			char* src = temp.String;
			char* dst = src + 1;
			while (*src) {
				*src++ = *dst++;
			}
			temp.len--;
		}
	}
	return temp;
}

//--------------------------------------------------------------------
/// <summary>
/// ファイル情報
/// </summary>
/// <param name="str">ファイル名</param>
/// <param name="mode">1:ファイル情報,0:日付のみ</param>
/// <returns>ファイル情報(JSON {"permission":permittion,"size":size,"date":date}) 存在しないときundefined</returns>
wString wString::file_stats (const char* str, int mode)
{
	struct stat      stat_buf;
	wString buf = "undefined";
	wString path = str;
	if (stat (path.nkfcnv ("Ws").c_str (), &stat_buf) == 0) {
		if (mode == 0) {
			/* ファイル情報を表示 */

#ifdef __linux__
			buf.sprintf ("{\"permission\":\"%o\",\"size\":%d,\"date\":\"%s\"}", stat_buf.st_mode, stat_buf.st_size, ctime (&stat_buf.st_mtime));
#else
			wString* time_data = ctimew (&stat_buf.st_mtime);
			buf.sprintf ("{\"permission\":\"%o\",\"size\":%d,\"date\":\"%s\"}", stat_buf.st_mode, stat_buf.st_size, time_data->c_str ());
			delete time_data;
#endif
			//printf("デバイスID : %d\n",stat_buf.st_dev);
			//printf("inode番号 : %d\n",stat_buf.st_ino);
			//printf("アクセス保護 : %o\n",stat_buf.st_mode );
			//printf("ハードリンクの数 : %d\n",stat_buf.st_nlink);
			//printf("所有者のユーザID : %d\n",stat_buf.st_uid);
			//printf("所有者のグループID : %d\n",stat_buf.st_gid);
			//printf("デバイスID（特殊ファイルの場合） : %d\n",stat_buf.st_rdev);
			//printf("容量（バイト単位） : %d\n",stat_buf.st_size);
			//printf("ファイルシステムのブロックサイズ : %d\n",stat_buf.st_blksize);
			//printf("割り当てられたブロック数 : %d\n",stat_buf.st_blocks);
			//printf("最終アクセス時刻 : %s",ctime(&stat_buf.st_atime));
			//printf("最終修正時刻 : %s",ctime(&stat_buf.st_mtime));
			//printf("最終状態変更時刻 : %s",ctime(&stat_buf.st_ctime));
		}
		else {
			//date
			if (mode == 1) {
				//char s[128] = {};
				//time_t timer;
				//struct tm *timeptr;
				//timer = time(NULL);
				//timeptr = localtime(&stat_buf.st_mtime);
				//strftime(s, 128, "%Y/%m/%d %H:%M:%S", timeptr);
				buf.sprintf ("%d", stat_buf.st_mtime);
			}
		}
	}
	else {
		debug_log_output ("%s(%d):file_stats(%s) Error.", __FILE__, __LINE__, str);
	}
	return buf;
}
//---------------------------------------------------------------------------

/// <summary>
/// ファイル情報
/// </summary>
/// <param name="str">ファイル名</param>
/// <param name="mode">1:ファイル情報,0:日付のみ</param>
/// <returns>ファイル情報(JSON {"permission":permittion,"size":size,"date":date}) 存在しないときundefined</returns>
wString wString::file_stats (const wString& str, int mode)
{
	return file_stats (str.String, mode);
}

//---------------------------------------------------------------------------
int wString::file_exists (const char* str)
{
#ifdef __linux__
	struct stat send_filestat;
	int  result = stat (str, &send_filestat);
	if ((result == 0) && (S_ISREG (send_filestat.st_mode) == 1)) {
		return 1;
	}
#else
	wString tmp (str);
	wString tmp2 = tmp.nkfcnv ("Ws");
	std::ifstream file (tmp2.String);
	if (file.is_open ()) {
		return 1;
	}
#endif
	//debug_log_output("%s(%d):file_exists(%s) Error.", __FILE__, __LINE__, tmp.c_str());
	return 0;
}
//---------------------------------------------------------------------------
int wString::file_exists (const wString& str)
{
	return file_exists (str.String);
}
//---------------------------------------------------------------------------
int wString::file_exists (void)
{
	return file_exists (String);
}
//---------------------------------------------------------------------------
//パス部分を抽出
wString wString::extract_file_dir (const wString& str)
{
	//todo SJIS/EUC対応するように
	wString temp (str);
	int ptr = temp.last_delimiter (DELIMITER);
	temp.len = ptr;
	temp.String[ptr] = 0;
	return temp;
}
//---------------------------------------------------------------------------

/// <summary>
/// フォルダ作成
/// </summary>
/// <param name="str">utf-8フォルダ名</param>
/// <returns>成功:true 失敗:false</returns>
bool wString::create_dir (const wString& str)
{
	bool flag = false;
#ifdef __linux__
	//0x777ではちゃんとフォルダできない
	flag = (mkdir (str.String, 0777) != -1);
#else
	wString str2 (str);

	if (!directory_exists (str2)) {
		flag = (mkdir (str2.nkfcnv ("Ws").windows_file_name ().c_str ()) != -1);
		if (!flag) {
			debug_log_output ("%s(%d):create_dir(%s) Error.", __FILE__, __LINE__, str2.c_str ());
		}
	}
#endif
	return flag;
}
///---------------------------------------------------------------------------
/// <summary>
/// 指定文字数で文字列を初期化
/// </summary>
/// <param name="num"></param>
void wString::reset_length (unsigned int num)
{
	assert (capa > (unsigned int)num);
	String[num] = 0;
	len = num;
}
//---------------------------------------------------------------------------
char* wString::c_str (void) const
{
	return String;
}
/// <summary>
/// 文字領域キャパシティ
/// </summary>
/// <param name=""></param>
/// <returns></returns>
inline unsigned int wString::capacity (void) const
{
	return capa;
}
//---------------------------------------------------------------------------
int wString::last_delimiter (const char* delim) const
{
	auto pos = -1;
	auto dlen = (int)strlen (delim);
	for (auto i = (int)len - dlen; i > 0; i--) {
		if (strncmp (String + i, delim, dlen) == 0) {
			pos = i;
			break;
		}
	}
	return pos;
}
//---------------------------------------------------------------------------
bool wString::rename_file (const wString& src, const wString& dst)
{
	wString src2 = src;
	wString dst2 = dst;
#ifdef __linux__
#else
	src2 = src2.nkfcnv ("Ws");
	dst2 = dst2.nkfcnv ("Ws");
#endif
	if (rename (src2.c_str (), dst2.c_str ()) >= 0) {
		return true;
	}
	else {
		//debug_log_output("%s(%d):rename_file(%s,%s) Error.", __FILE__, __LINE__, src.c_str(),dst.c_str());
		return false;
	}
}
//---------------------------------------------------------------------------
unsigned long wString::file_size_by_name (char* str)
{
	unsigned long pos;
	int handle;
#ifdef __linux__
	handle = open (str, 0);
#else
	wString temp = str;
	handle = open (temp.nkfcnv ("Ws").c_str (), O_BINARY);
#endif
	pos = lseek (handle, 0, SEEK_END);
	close (handle);
	return pos;
}
//---------------------------------------------------------------------------
unsigned long wString::file_size_by_name (wString& str)
{
	return file_size_by_name (str.String);
}
//---------------------------------------------------------------------------
wString wString::extract_file_name (const char* str, const char* delim)
{
	wString tmp (str);
	return extract_file_name (tmp, delim);
}
//---------------------------------------------------------------------------
wString wString::extract_file_name (const wString& str, const char* delim)
{
	int pos = str.last_delimiter (delim);
	return str.substr (pos + 1);
}
//---------------------------------------------------------------------------
wString wString::extract_file_ext (const wString& str)
{
	int pos = str.last_delimiter (".");
	return str.substr (pos + 1);
}
//---------------------------------------------------------------------------
const char* MIME_TYPE_NAME[] = {
	"TYPE_UNKNOWN",
	"TYPE_DIRECTORY",
	"TYPE_MOVIE",
	"TYPE_MUSIC",
	"TYPE_IMAGE",
	"TYPE_DOCUMENT",
	"TYPE_PLAYLIST",
	"TYPE_MUSICLIST",
	"TYPE_URL",
	"TYPE_SCRIPT",
};
/// <summary>
/// JSON {"mineType":"mimetype","fileType":"filetype"}が返還される。
/// </summary>
/// <param name="file_name">検査する</param>
/// <returns></returns>
wString wString::find_mime_type (const wString& file_name)
{
	// mime_list不使用のためコメントアウト
	IGNORE_PARAMETER(file_name);
	return "";
	/*
	int pos = file_name.last_delimiter (".");
	wString tmp = file_name.substr (pos + 1).ToLower ();
	if (tmp.length () > 0) {
		int ptr = 0;
		while (mime_list[ptr].mime_type != NULL) {
			if (tmp == mime_list[ptr].file_extension) {
				tmp.wString::sprintf ("{\"mimeType\":\"%s\",\"fileType\":\"%s\"}", mime_list[ptr].mime_type, MIME_TYPE_NAME[static_cast<int>(mime_list[ptr].menu_file_type)]);
				return tmp;
			}
			ptr++;
		}
	}
	return "";
	*/
}

//---------------------------------------------------------------------------
wString wString::change_file_ext (const wString& str, const char* ext)
{
	int pos = str.last_delimiter (".");
	return str.substr (0, pos + 1) + ext;
}
//---------------------------------------------------------------------------
int wString::delete_file (const wString& str)
{
	int flag = 0;
#ifdef __linux__
	flag = (unlink (str.String) == 0);
#else
	wString str2 = str;
	if (file_exists (str2)) {
		char work[2048];
		// MS932に変換して実行
		strcpy (work, str2.nkfcnv ("Ws").c_str ());
		windows_file_name (work);
		if (unlink (work) == 0) {
			flag = 1;
		}
	}
#endif
	if (flag == 0) {
		debug_log_output ("%s(%d):delete_file(%s) Error.", __FILE__, __LINE__, str.c_str ());
	}
	return flag;
}
//---------------------------------------------------------------------------
int wString::delete_folder (const wString& str)
{
	int flag = 0;
#ifdef __linux__
	flag = (rmdir (str.String) == 0);
#else
	wString str2 = str;
	// str2はUTF-8
	if (directory_exists (str2)) {
		char work[2048];
		// MS932に変換して実行
		strcpy (work, str2.nkfcnv ("Ws").c_str ());
		windows_file_name (work);
		if (rmdir (work) == 0) {
			flag = 1;
		}
	}
#endif
	debug_log_output ("%s(%d):delete_file(%s) Error.", __FILE__, __LINE__, str.c_str ());
	return flag;
}
//---------------------------------------------------------------------------
int wString::directory_exists (const char* str)
{
	int flag = 0;
#ifdef __linux__
	struct stat send_filestat;
	int  result = stat (str, &send_filestat);
	if ((result == 0) && (S_ISDIR (send_filestat.st_mode) == 1)) {
		flag = 1;
	}
#else

	//char tmp[1024];
	// TODO SJIS変換
	wString tmp (str);
	wString tmp2 = tmp.nkfcnv ("Ws");

	//strcpy(tmp, str);
	// フォルダの末尾は'/'以外
	if (tmp2.ends_with ("/")) {
		tmp2 = tmp2.substr (0, tmp2.len - 1);
	}
	WIN32_FIND_DATAA fd; //検索データ
	HANDLE result = ::FindFirstFileA (tmp2.String, &fd); //最初の検索で使用する関数
	if (result != INVALID_HANDLE_VALUE) {
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
			flag = 1;
		}
		FindClose (result); //ハンドルを閉じる
	}
#endif
	//if (flag == 0)
	//{
	//	debug_log_output("%s(%d):directory_exists(%s) Error.", __FILE__, __LINE__, tmp2.c_str());
	//}
	return flag;
}
//---------------------------------------------------------------------------
int wString::directory_exists (const wString& str)
{
	return directory_exists (str.String);
}
/// <summary>
/// sentence文字列内のkey文字列を置換先文字列で置換する。
/// </summary>
/// <param name="sentence">置換対象文字列</param>
/// <param name="slen">置換対象文字列の長さ</param>
/// <param name="p">置換の位置</param>
/// <param name="klen">置換する長さ</param>
/// <param name="rep">置換先文字列</param>
void wString::replace_character_len (const char* sentence, int slen, const char* p, int klen, const char* rep)
{
	auto rlen = (int)strlen (const_cast<char*>(rep));
	int num;
	if (klen == rlen) {
		memcpy (const_cast<char*>(p), rep, rlen);
		//前詰め置換そのままコピーすればいい
	}
	else if (klen > rlen) {
		num = klen - rlen;
		strcpy (const_cast<char*>(p), p + num);
		memcpy (const_cast<char*>(p), rep, rlen);
		//置換文字が長いので後詰めする
	}
	else {
		num = rlen - klen;
		//pからrlen-klenだけのばす
		for (auto str = const_cast<char*>(sentence + slen + num); str > p + num; str--) {
			*str = *(str - num);
		}
		memcpy (const_cast<char*>(p), rep, rlen);
	}
	return;
}
/// <summary>
/// sentence文字列内のkey文字列をrep文字列で置換する。ｎ回
/// </summary>
/// <param name="sentence">置換元文字列</param>
/// <param name="key">置換文字列</param>
/// <param name="rep">置換後の文字列</param>
bool wString::replace_character(wString key, wString rep)
{
	int klen = key.length();
	int rlen = rep.length();
	if (klen == 0 || len == 0) {
		return false;
	}
	auto p = find(key);
	//前詰め置換そのままコピーすればいい
	while (p != wString::npos) {
		replace(p, klen, rep);
		p = find(key, rlen);
	}
	return true;
}
///---------------------------------------------------------------------------
/// <summary>
/// フォルダ内のファイルを列挙する
/// </summary>
/// <param name="Path">フォルダパス</param>
/// <returns>一覧のJSON文字列。コピーしてほしい</returns>
wString wString::enum_folder_json (const wString& Path)
{
#ifdef __linux__
	struct dirent** namelist;
	int n;
	wString temp;
	wString Path2;
	Path2 = Path;
	int first = 1;
	//Directoryオープン
	n = scandir (Path.String, &namelist, NULL, alphasort);
	if (n >= 0) {
		temp = "[";
		for (int i = 0; i < n; i++) {
			if (first) {
				first = 0;
			}
			else {
				temp += ",";
			}
			temp += "\"" + Path2 + DELIMITER + namelist[i]->d_name + "\"";
			free (namelist[i]);
		}
		temp += "]";
		free (namelist);
	}
	else {
		throw new CScriptException ("Directory Open Error.[" + Path2 + "]");
		//perror ("ディレクトリのオープンエラー");
		//exit (1);
	}
	return temp;
#else
	DIR* dir;
	wString              temp;
	wString              Path2;

	Path2 = Path;
	Path2 = Path2.nkfcnv ("Ws");
	//Directoryオープン
	if ((dir = opendir (Path2.String)) != NULL) {
		struct dirent* ent;
		std::vector<wString> list;
		//ファイルリスト
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp (ent->d_name, ".") != 0 &&
				strcmp (ent->d_name, "..") != 0) {
				list.push_back ("\"" + Path2 + DELIMITER + ent->d_name + "\"");
			}
		}
		closedir (dir);

		//sort
		if (list.size () > 0) {
			for (unsigned int ii = 0; ii < list.size () - 1; ii++) {
				for (unsigned int jj = ii + 1; jj < list.size (); jj++) {
					if (strcmp (list[ii].c_str (), list[jj].c_str ()) > 0) {
						std::swap (list[ii], list[jj]);
					}
				}
			}
		}        temp = "[";
		for (unsigned int i = 0; i < list.size (); i++) {
			if (i) temp += ",";
			temp += list[i];
		}
		temp += "]";
	}
	else {
		throw new CScriptException ("Directory Open Error.Full Path Required.[" + Path2 + "]");
		//perror ("ディレクトリのオープンエラー");
		//exit (1);
	}
	wString temp2 = temp.nkfcnv ("Sw");
	return temp2;
#endif
}

wString wString::get_current_dir ()
{
	char cwd[FILENAME_MAX];
	if (getcwd (cwd, sizeof (cwd)) == NULL) {
		debug_log_output ("get_current_dir: error %s", strerror (errno));
		throw new CScriptException ("get_current_dir");
		//exit(-1);
	}
#ifdef __linux__
	return wString (cwd);
#else
	return wString (cwd).nkfcnv ("Sw");
#endif
}

///---------------------------------------------------------------------------
/// <summary>
/// フォルダ内のファイルを列挙する
/// </summary>
/// <param name="Path">フォルダパス</param>
/// <returns>一覧のJSON文字列。コピーしてほしい</returns>
wString wString::enum_folder (const wString& Path)
{
#ifdef __linux__
	struct dirent** namelist;
	int n;
	wString temp;
	wString Path2;
	Path2 = Path;
	int first = 1;

	n = scandir (Path.String, &namelist, NULL, alphasort);
	if (n < 0) {
		temp = "";
	}
	else
	{
		temp = "[";
		for (int i = 0; i < n; i++) {
			if (first) {
				first = 0;
			}
			else {
				temp += ",";
			}
			temp += "\"" + Path2 + DELIMITER + namelist[i]->d_name + "\"";
			free (namelist[i]);
		}
		temp += "]";
		free (namelist);
	}
#else
	DIR* dir;
	wString              temp;
	wString              Path2;
	Path2 = Path;
	Path2 = Path2.nkfcnv ("Ws");
	//Directoryオープン
	if ((dir = opendir (Path2.String)) != NULL) {
		struct dirent* ent;
		std::vector<wString> list;
		//ファイルリスト
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp (ent->d_name, ".") != 0 &&
				strcmp (ent->d_name, "..") != 0) {
				list.push_back (Path2 + DELIMITER + ent->d_name);
			}
		}
		closedir (dir);
		if (list.size () > 0) {
			for (unsigned int ii = 0; ii < list.size () - 1; ii++) {
				for (unsigned int jj = ii + 1; jj < list.size (); jj++) {
					if (strcmp (list[ii].c_str (), list[jj].c_str ()) > 0) {
						std::swap (list[ii], list[jj]);
					}
				}
			}
		}
		for (unsigned int i = 0; i < list.size (); i++) {
			temp.Add (list[i]);
		}
	}
	else {
		throw new CScriptException ("Directory Open Error.[" + Path2 + "]");
		//perror ("ディレクトリのオープンエラー");
		//exit (1);
	}
#endif
	wString temp2 = temp.nkfcnv ("Sw");
	return temp2;
}
/// <summary>
/// 文字列（バイナリ含む）ダンプ
/// </summary>
/// <returns>ダンプした文字列</returns>
wString wString::dump () const
{
	wString tmp;
	void* ptr = String;
	int vlen = len;
	unsigned char* out = static_cast<unsigned char*>(ptr);
	for (int start = 0; start < vlen; start += 16) {
		tmp.cat_sprintf ("%04d:", start);
		for (int line = start; line < start + 16 && line < vlen; line++) {
			tmp.cat_sprintf ("%02X ", out[line]);
		}
		for (int line = start; line < start + 16 && line < vlen; line++) {
			if (out[line] >= ' ' && out[line] < 0x7f) {
				tmp.cat_sprintf ("%c", out[line]);
			}
			else {
				tmp.cat_sprintf (".");
			}
		}
		tmp.cat_sprintf ("\r\n");
	}
	return tmp;
}
#ifndef va_copy

/// <summary>
/// 可変フォーマッティング
/// </summary>
/// <param name="fmt"></param>
/// <param name="arg"></param>
/// <returns></returns>
int wString::vtsprintf (const char* fmt, va_list arg)
{
	int vlen = 0;
	int vsize = 0;
	int zeroflag, width;


	while (*fmt) {
		if (*fmt == '%') {        /* % に関する処理 */
			zeroflag = width = 0;
			fmt++;
			if (*fmt == '0') {
				fmt++;
				zeroflag = 1;
			}
			while ((*fmt >= '0') && (*fmt <= '9')) {
				width *= 10;
				width += *(fmt++) - '0';
			}

			/* printf ("zerof = %d,width = %d\n",zeroflag,width); */

			//lluもluもuも同じ
			while (*fmt == 'l' || *fmt == 'z') {
				fmt++;
			}

			switch (*fmt) {
			case 'd':        /* 10進数 */
				vsize = tsprintf_decimal (va_arg (arg, signed long), zeroflag, width);
				break;
			case 'u':        /* 10進数 */
				vsize = tsprintf_decimalu (va_arg (arg, unsigned long), zeroflag, width);
				break;
			case 'o':        /* 8進数 */
				vsize = tsprintf_octadecimal (va_arg (arg, unsigned long), zeroflag, width);
				break;
			case 'x':        /* 16進数 0-f */
				vsize = tsprintf_hexadecimal (va_arg (arg, unsigned long), 0, zeroflag, width);
				break;
			case 'X':        /* 16進数 0-F */
				vsize = tsprintf_hexadecimal (va_arg (arg, unsigned long), 1, zeroflag, width);
				break;
			case 'c':        /* キャラクター */
				vsize = tsprintf_char (va_arg (arg, int));
				break;
			case 's':        /* ASCIIZ文字列 */
				vsize = tsprintf_string (va_arg (arg, char*));
				break;
			default:        /* コントロールコード以外の文字 */
				/* %%(%に対応)はここで対応される */
				vlen++;
				*this += *fmt;
				break;
			}
			vlen += vsize;
			fmt++;
		}
		else {
			*this += *(fmt++);
			vlen++;
		}
	}
	va_end (arg);
	return (vlen);
}
/// <summary>
/// 数値 => 10進文字列変換
/// </summary>
/// <param name="val"></param>
/// <param name="zerof"></param>
/// <param name="width"></param>
/// <returns></returns>
int wString::tsprintf_decimal (signed long val, int zerof, int width)
{
	//末尾０を保証
	char  tmp[22] = {};
	char* ptmp = tmp + 20;
	int   vlen = 0;
	int   minus = 0;

	if (!val) {        /* 指定値が0の場合 */
		*(ptmp--) = '0';
		vlen++;
	}
	else {
		/* マイナスの値の場合には2の補数を取る */
		if (val < 0) {
			val = ~val;
			val++;
			minus = 1;
		}
		while (val && vlen < 19) {
			/* バッファアンダーフロー対策 */
			//if (vlen >= 19){
			//    break;
			//}

			*ptmp = (char)((val % 10) + '0');
			val /= 10;
			ptmp--;
			vlen++;
		}

	}

	/* 符号、桁合わせに関する処理 */
	if (zerof) {
		if (minus) {
			width--;
		}
		while (vlen < width) {
			*(ptmp--) = '0';
			vlen++;
		}
		if (minus) {
			*(ptmp--) = '-';
			vlen++;
		}
	}
	else {
		if (minus) {
			*(ptmp--) = '-';
			vlen++;
		}
		while (vlen < width) {
			*(ptmp--) = ' ';
			vlen++;
		}
	}
	*this += (ptmp + 1);
	return (vlen);
}
/// <summary>
/// 符号なし数値 => 10進文字列変換
/// </summary>
/// <param name="val"></param>
/// <param name="zerof">先頭がゼロサプレスなら1</param>
/// <param name="width"></param>
/// <returns></returns>
int wString::tsprintf_decimalu (unsigned long val, int zerof, int width)
{
	char tmp[22] = {};
	char* ptmp = tmp + 20;
	int vlen = 0;
	// TODO:minusが設定されない
	int minus = 0;

	if (!val) {        /* 指定値が0の場合 */
		*(ptmp--) = '0';
		vlen++;
	}
	else {
		while (val) {
			/* バッファアンダーフロー対策 */
			if (vlen >= 19) {
				break;
			}

			*ptmp = (char)((val % 10) + '0');
			val /= 10;
			ptmp--;
			vlen++;
		}
	}

	/* 符号、桁合わせに関する処理 */
	if (zerof) {
		if (minus) {
			width--;
		}
		while (vlen < width) {
			*(ptmp--) = '0';
			vlen++;
		}
		if (minus) {
			*(ptmp--) = '-';
			vlen++;
		}
	}
	else {
		while (vlen < width) {
			*(ptmp--) = ' ';
			vlen++;
		}
	}

	*this += (ptmp + 1);
	return (vlen);
}
/// <summary>
/// 数値 => 8進文字列変換
/// </summary>
/// <param name="val"></param>
/// <param name="zerof"></param>
/// <param name="width"></param>
/// <returns></returns>
int wString::tsprintf_octadecimal (unsigned long val, int zerof, int width)
{
	char tmp[22] = {};
	char* ptmp = tmp + 20;
	int vlen = 0;
	// TODO:minusが設定されない
	int minus = 0;

	if (!val) {        /* 指定値が0の場合 */
		*(ptmp--) = '0';
		vlen++;
	}
	else {
		while (val) {
			/* バッファアンダーフロー対策 */
			if (vlen >= 19) {
				break;
			}

			*ptmp = (char)((val % 8) + '0');
			val /= 8;
			ptmp--;
			vlen++;
		}
	}

	/* 符号、桁合わせに関する処理 */
	if (zerof) {
		if (minus) {
			width--;
		}
		while (vlen < width) {
			*(ptmp--) = '0';
			vlen++;
		}
		if (minus) {
			*(ptmp--) = '-';
			vlen++;
		}
	}
	else {
		while (vlen < width) {
			*(ptmp--) = ' ';
			vlen++;
		}
	}

	*this += (ptmp + 1);
	return (vlen);
}
/// <summary>
/// 数値 => 16進文字列変換
/// </summary>
/// <param name="val"></param>
/// <param name="capital"></param>
/// <param name="zerof"></param>
/// <param name="width"></param>
/// <returns></returns>
int wString::tsprintf_hexadecimal (unsigned long val, int capital, int zerof, int width)
{
	char tmp[22] = {};
	char* ptmp = tmp + 20;
	int vlen = 0;
	char str_a;

	/* A～Fを大文字にするか小文字にするか切り替える */
	if (capital) {
		str_a = 'A';
	}
	else {
		str_a = 'a';
	}

	if (!val) {        /* 指定値が0の場合 */
		*(ptmp--) = '0';
		vlen++;
	}
	else {
		while (val) {
			/* バッファアンダーフロー対策 */
			if (vlen >= 18) {
				break;
			}

			*ptmp = (char)(val % 16);
			if (*ptmp > 9) {
				*ptmp += (char)(str_a - 10);
			}
			else {
				*ptmp += '0';
			}

			val >>= 4;        /* 16で割る */
			ptmp--;
			vlen++;
		}
	}
	while (vlen < width) {
		*(ptmp--) = zerof ? '0' : ' ';
		vlen++;
	}

	*this += (ptmp + 1);
	return(vlen);
}
/// <summary>
/// 数値 => 1文字キャラクタ変換
/// </summary>
/// <param name="ch"></param>
/// <returns></returns>
int wString::tsprintf_char (int ch)
{
	*this += (char)ch;
	return(1);
}
/// <summary>
/// 数値 => ASCIIZ文字列変換
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
int wString::tsprintf_string (char* str)
{
	*this += str;
	return(static_cast<int>(strlen (str)));
}
#endif
//---------------------------------------------------------------------------
//wString 


/// <summary>
/// 可変引数フォーマッティング
/// </summary>
/// <param name="format"></param>
/// <param name=""></param>
/// <returns></returns>
int wString::sprintf (const char* format, ...)
{
#ifndef va_copy
	String[0] = 0;
	vlen = 0;
	va_list ap;
	va_start (ap, format);
	vtsprintf (format, ap);
	va_end (ap);
	return vlen;
#else
	int stat;
	//可変引数を２つ作る
	va_list ap1, ap2;
	va_start (ap1, format);
	va_copy (ap2, ap1);
	//最初はダミーで文字列長をシミュレート
	stat = vsnprintf (String, 0, format, ap1);
	set_length (stat + 1);
	//実際に出力
	stat = vsprintf (String, format, ap2);
	va_end (ap1);
	va_end (ap2);
	len = stat;
	return stat;
#endif
}
/// <summary>
/// 可変引数追加フォーマッティング
/// </summary>
/// <param name="format"></param>
/// <param name=""></param>
/// <returns></returns>
int wString::cat_sprintf (const char* format, ...)
{
#ifndef va_copy
	int status;
	va_list ap;
	va_start (ap, format);
	status = this->vtsprintf (format, ap);
	va_end (ap);
	return status;
#else
	int stat;
	//可変引数を２つ作る
	va_list ap1, ap2;
	va_start (ap1, format);
	va_copy (ap2, ap1);
	//最初はダミーで文字列長をシミュレート
	stat = vsnprintf (String, 0, format, ap1);
	set_length (stat + len + 1);
	//実際に出力
	stat = vsprintf (String + len, format, ap2);
	va_end (ap1);
	va_end (ap2);
	len += stat;
	return stat;
#endif
}
//---------------------------------------------------------------------------
/// <summary>
/// 文字列をデリミタで切って、デリミタ後の文字を返す
/// 元文字列は破壊
/// </summary>
/// <param name="delimstr">分解したTStringList文字列</param>
/// <returns>切断後の文字列。見つからない場合は長さ０の文字列</returns>
wString wString::strsplit (const char* delimstr)
{
	wString tmp;
	char* ptr = c_str ();
	auto delimlen = static_cast<int>(strlen (delimstr));
	int start = 0;
	for (auto i = 0; i < static_cast<int>(len); i++) {
		for (auto j = 0; j < delimlen; j++) {
			if (ptr[i] == delimstr[j]) {
				ptr[i] = 0;
				tmp.Add (&ptr[start]);
				start = i + 1;
				break;
			}
		}
	}
	if (start < static_cast<int>(len)) {
		tmp.Add (&ptr[start]);
	}
	return tmp;
}
/// <summary>
/// 文字列容量の変更
/// </summary>
/// <param name="newsize"></param>
void wString::resize (const int newsize)
{
	if (len >= capa) {
		wString temp;
		temp.sprintf ("Over capacity.[len:%u capa:%u]", len, capa);
		throw new CScriptException (temp);
		//printf ("not good %u %u", len, capa);
		//exit (1);
	}
	if ((int)capa <= newsize) {
		// capaは古いサイズ＋新しいサイズ。必ず１は増やす。フィボナッチと同じ
		capa += newsize + ((capa <= 0) ? 1 : 0);
		char* tmp = new char[capa];
		memcpy (tmp, String, len);
		tmp[len] = 0;
		delete[] String;
		String = tmp;
	}
	else {
		//指定サイズが元より小さいので何もしない
	}
}
///---------------------------------------------------------------------------
/// <summary>
/// HTML特殊文字に変換
/// </summary>
/// <param name=""></param>
/// <returns>HTML特殊文字</returns>
wString wString::htmlspecialchars (void)
{
	wString temp;
	// 引数チェック
	char* ptr = String;
	for (unsigned int is = 0; is < len; is++) {
		switch (*ptr) {
		case '&':temp += "&amp;"; break;
		case '\"':temp += "&quot;"; break;
			//case '\'':temp += "&#39;";break;
		case '<':temp += "&lt;";  break;
		case '>':temp += "&gt;";  break;
		default:temp += *ptr;    break;
		}
		ptr++;
	}
	return temp;
}
///---------------------------------------------------------------------------
/// <summary>
/// URIエンコードを行う
/// </summary>
/// <param name="">変換元文字列</param>
/// <returns>URIエンコードした文字列</returns>
wString wString::uri_encode (void)
{
	unsigned int is;
	wString dst;
	// 引数チェック
	for (is = 0; is < len; is++) {
		auto datum = static_cast<unsigned char>(String[is]);
		/* ' '(space) はちと特別扱いにしないとまずい */
		if (datum == ' ') {
			dst += "%20";
			/* エンコードしない文字全員集合 */
			//        }else if ( strchr("!$()*,-./:;?@[]^_`{}~", String[is]) != NULL ){
		}
		else if (strchr ("!$()*,-.:;/?@[]^_`{}~", datum) != NULL) {
			dst += datum;
			/* アルファベットと数字はエンコードせずそのまま */
		}
		else if (isalnum (datum)) {
			dst += datum;
		}
		/* \マークはエンコード */
		else if (datum == '\\') {
			dst += "%5c";
			/* それ以外はすべてエンコード */
		}
		else {
			char work[8];
			::sprintf (work, "%%%2X", datum);
			dst += work;
		}
	}
	return dst;
}

#define FILE_BUFFER 8000
/// <summary>
/// ファイルコピー
/// </summary>
/// <param name="fname_r">ファイルコピー元</param>
/// <param name="fname_w">ファイルコピー先</param>
/// <returns>成功:0,失敗:-1</returns>
int   wString::FileCopy (const wString& fname_r, const wString& fname_w)
{
	int fpr;
	int fpw;
	unsigned char buf[FILE_BUFFER];

	fpr = myopen (fname_r, O_RDONLY | O_BINARY, S_IREAD);
	if (fpr < 0) {
		return -1;
	}
	fpw = myopen (fname_w, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);
	if (fpw < 0) {
		close (fpr);
		return -1;
	}
	while (1) {
		auto vsize = read (fpr, buf, sizeof (buf));
		if (vsize <= 0) {
			break;
		}
		write (fpw, buf, vsize);
	}
	close (fpr);
	close (fpw);

	return 0;
}
///---------------------------------------------------------------------------
/// <summary>
/// QUOTE,ESCAPE,NULLをエスケープする
/// </summary>
/// <returns>エスケープした文字列</returns>
wString wString::add_slashes (void)
{
	wString tmp;
	for (unsigned int i = 0; i < len; i++) {
		if (String[i] == '\'' || String[i] == '\"' || String[i] == '\\' || String[i] == 0) {
			tmp += '\\';
		}
		tmp += String[i];
	}
	return tmp;
}
/// <summary>
/// 1文字の１６進文字を１０進数値に変換する
/// </summary>
/// <param name="x">変換する文字</param>
/// <returns>１０進の数値</returns>
unsigned char htoc (unsigned char x)
{
	if ('0' <= x && x <= '9') return (unsigned char)(x - '0');
	if ('a' <= x && x <= 'f') return (unsigned char)(x - 'a' + 10);
	if ('A' <= x && x <= 'F') return (unsigned char)(x - 'A' + 10);
	return 0;
}
/// <summary>
/// URIデコードを行います.
/// </summary>
/// <returns>デコードした文字</returns>
wString wString::uri_decode ()
{
	unsigned char   code;
	wString dst;
	// =================
	// メインループ
	// =================
	for (auto i = 0U; i < len && String[i] != '\0'; i++) {
		if (String[i] == '%') {
			if (i + 2 >= len) {
				break;
			}
			code = (unsigned char)(htoc (String[++i]) << 4);
			code += htoc (String[++i]);
			dst += code;
		}
		else if (String[i] == '+') {
			dst += ' ';
		}
		else {
			dst += String[i];
		}
	}
	return dst;
}
#if 0  // WEB不使用: init_header/header/headerPrintMem等はコメントアウト
/// <summary>
/// httpヘッダを作成する
/// </summary>
/// <param name="content_length">送出するコンテンツ長さ。０なら省略</param>
/// <param name="expire">０以外で作成。１時間で期限切れ</param>
/// <param name="mime_type">mime_type</param>
void wString::init_header (size_t content_length, int expire, const char* mime_type)
{
	sprintf ("%s", HTTP_OK);
	cat_sprintf ("%s", HTTP_CONNECTION);
	cat_sprintf (HTTP_SERVER_NAME, SERVER_NAME);
	cat_sprintf (HTTP_CONTENT_TYPE, mime_type);
	//Date
	time_t timer;
	time (&timer);
	//struct tm *utc;
	//utc = gmtime(&timer);
	struct tm utc;
	gmtime_r (&timer, &utc);
	char work[80] = {};
	const static char* dow[] = { "Sun", "Mon","Tue", "Wed", "Thu", "Fri", "Sat" };
	const static char* mon[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	::sprintf (work, "%s, %d %s %d %02d:%02d:%02d",
			   dow[utc.tm_wday], utc.tm_mday, mon[utc.tm_mon], utc.tm_year + 1900, utc.tm_hour, utc.tm_min, utc.tm_sec);
	this->cat_sprintf ("Date: %s GMT\r\n", work);
	//expire
	if (expire) {
		timer += 60 * 60;
		//utc = gmtime(&timer);
		gmtime_r (&timer, &utc);
		::sprintf (work, "%s, %d %s %d %02d:%02d:%02d",
				   dow[utc.tm_wday], utc.tm_mday, mon[utc.tm_mon], utc.tm_year + 1900, utc.tm_hour, utc.tm_min, utc.tm_sec);
		this->cat_sprintf ("Expires: %s GMT\r\n", work);
	}
	if (content_length) {
		this->cat_sprintf (HTTP_CONTENT_LENGTH, content_length);
	}
}

/// <summary>
/// ヘッダー文字列をsocketに送出（socket不使用のためコメントアウト）
/// </summary>
/*
void wString::send_header (SOCKET socket, int endflag)
{
	transport_send (socket, String, len, 0);
	if (endflag) {
		transport_send (socket, HTTP_END, (int)strlen (HTTP_END), 0);
	}
}
*/
wString wString::headerPrintMem (void)
{
	return *this + HTTP_END;
}

/// <summary>
/// BorlandのTStringList対応
/// ヘッダ作成
/// 301(恒久的)は転送先のURLが表示され、302(一時的)では元のURLが表示される
/// </summary>
/// <param name="str">header文字列</param>
/// <param name="flag">true:新規作成,false:継続作成</param>
/// <param name="status">301それ以外は302</param>
/// <returns></returns>
int wString::header (const char* str, int flag, int status)
{
	char head[80] = {};
	char body[80] = {};
	if (*str) {
		if (!flag) {
			Add (str);
			return 0;
		}
		for (auto i = 0; i < (int)strlen (str); i++) {
			if (str[i] == ' ') {
				memcpy (head, str, i + 1);
				head[i + 1] = 0;
				strcpy (body, str + i + 1);
				break;
			}
		}
		if (*head && *body) {
			if (strcmp (head, "Location: ") == 0) {
				if (status == 301) {
					header ("HTTP/1.0 301 Moved Permanetry", true);
				}
				else {
					header ("HTTP/1.0 302 Found", true);
				}
			}
			int count = lines ();
			wString str2;
			str2.sprintf ("%s%s", head, body);
			//あれば入れ替え
			for (int i = 0; i < count; i++) {
				wString tmp = get_list_string (i);
				if (strncmp (tmp.c_str (), head, strlen (head)) == 0) {
					insert_list_string (str2.c_str (), i);
					return 0;
				}
			}
			//なければ追加
			Add (str2);
			return 0;
		}
	}
	return 1;
}
#endif  // end WEB disabled block
/********************************************************************************/
// 日本語文字コード変換。
// (libnkfのラッパー関数)
//
//      サポートされている形式は以下の通り。
//      in:S:SJIS E:EUC W:UTF-8
//      out:s:SJIS w:UTF-8
/********************************************************************************/
wString wString::nkfcnv (const wString& option)
{
	// libnkf.hpp不使用のためno-op（文字列をそのまま返す）
	IGNORE_PARAMETER(option);
	return *this;
}
///---------------------------------------------------------------------------
/// <summary>
/// BorlandのTStringList対応
/// 行の追加
/// </summary>
/// <param name="str">追加する文字列</param>
void wString::Add (const char* str)
{
	*this += str;
	*this += "\r\n";
}
///---------------------------------------------------------------------------
/// <summary>
/// BorlandのTStringList対応
/// 行の追加
/// </summary>
/// <param name="str">追加する文字列</param>
void wString::Add (const wString& str)
{
	*this += str;
	*this += "\r\n";
}

///---------------------------------------------------------------------------
/// <summary>
/// BorlandのTStringList風実装
/// 格納した行数を返す
/// </summary>
/// <returns>行数</returns>
int wString::lines (void)
{
	int count = 0;
	char* ptr = String;
	while (*ptr) {
		if (*ptr == '\r') {
			ptr++;
			continue;
		}
		if (*ptr++ == '\n') {
			count++;
		}
	}
	return count;
}

//---------------------------------------------------------------------------
/// <summary>
/// BorlandのTStringList風実装
/// \r\nで区切られた文字列のpos行目にdstを挿入
/// </summary>
/// <param name="dst">挿入文字列</param>
/// <param name="pos">挿入行位置</param>
/// <returns>成功:true,失敗:false</returns>
bool wString::insert_list_string (wString& src, int pos)
{
	bool flag;
	flag = insert_list_string (src.String, pos);
	return flag;
}
///---------------------------------------------------------------------------
/// <summary>
/// BorlandのTStringList風実装
/// \r\nで区切られた文字列のpos行目にdstを挿入
/// </summary>
/// <param name="dst">挿入文字列</param>
/// <param name="pos">挿入行位置</param>
/// <returns>成功:true,失敗:false</returns>
bool wString::insert_list_string (const char* dst, int pos)
{
	//行数が多い
	std::vector<wString> work;
	int count = 0;
	int ptr = 0;
	int last = 0;
	int cchar = 0;
	while (String[ptr]) {
		switch (String[ptr]) {
		case '\r':
			ptr++;
			cchar++;
			continue;
		case '\n':
			ptr++;
			cchar++;
			count++;
			work.push_back (substr (last, ptr - last - cchar));
			cchar = 0;
			last = ptr;
			break;
		default:
			cchar = 0;
			ptr++;
		}
	}
	//行末にcrlfがなかった場合に対応
	if (ptr - last - cchar) {
		count++;
		work.push_back (substr (last, ptr - last - cchar));
	}
	//要素追加
	if (0 <= pos && pos <= count) {
		work[pos] = dst;
		//元を消す
		clear ();
		for (unsigned int i = 0; i < (unsigned int)work.size (); i++) {
			*this += work[i] + "\r\n";
		}
		return true;
	}
	else {
		//なにもしない
		return false;
	}
}
///---------------------------------------------------------------------------
/// <summary>
/// BorlandのTStringList風実装
/// \r\nで区切られた文字列リストのpos行目のデータを取得
/// </summary>
/// <param name="pos">取得行位置</param>
/// <returns>取得した文字列、失敗時区別なし</returns>
wString wString::get_list_string (int pos)
{
	int   lcount = 0;
	int   ptr = 0;
	int   curptr = 0;
	int   lstptr;
	int   spcnt = 0;//\r\nまたは\nの文字数
	while (String[ptr]) {
		if (String[ptr] == '\r') {
			ptr++;
			spcnt++;
			continue;
		}
		if (String[ptr++] == '\n') {
			lstptr = curptr;
			curptr = ptr;
			spcnt++;
			if (lcount++ == pos) {
				return substr (lstptr, curptr - lstptr - spcnt);
			}
		}
		spcnt = 0;
	}
	return wString ();
}
// HTTP通信関連はsocket不使用のためコメントアウト
#if 0
#define HTTP_STR_BUF_SIZE (1024)
/// <summary>
/// HTTPより文字列データを取得
/// </summary>
/// <param name="url">URL</param>
/// <param name="offset">読み込みオフセット</param>
/// <returns>読み込んだ文字列。失敗したときは長さ０</returns>
wString wString::http_get (const wString& url, off_t offset)
{
	wString     buf;
	wString     request;
	wString     body;
	HttpUrlParts target_info;
	SOCKET      server_socket;                  //サーバーソケット
	if (!parse_http_url(url, target_info)) {
		return wString ();
	}
	server_socket = open_http_socket(target_info);
	if (SERROR(server_socket)) {
		return wString();
	}

	request.sprintf("GET %s HTTP/1.0\r\n"
					"Accept: */*\r\n"
					"User-Agent: %s%s\r\nHost: %s\r\nRange: bytes=%llu-\r\nConnection: close\r\n\r\n",
					target_info.target.c_str(),
					USERAGENT,
					MACADDR,
					target_info.host.c_str(),
					static_cast<unsigned long long>(offset));
	if (transport_send(server_socket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
		transport_close(server_socket);
		return wString();
	}
	if (!read_http_response(server_socket, buf)) {
		transport_close(server_socket);
		return wString();
	}
	transport_close(server_socket);
	auto status = get_http_status_code(buf);
	if (status < 200 || 300 <= status) {
		return wString();
	}
	if (!split_http_response(buf, body)) {
		return wString();
	}
	return body;
}
///---------------------------------------------------------------------------
/// <summary>
/// HTTPよりRESTメソッドを指定し文字列データを取得
/// </summary>
/// <param name="methods">HEAD,GET,POST,PUT,DELETE</param>
/// <param name="url"></param>
/// <param name="data">送出データ</param>
/// <returns>読み込んだ文字列。失敗したときは長さ０</returns>
wString wString::http_rest (const wString& methods, const wString& url, const wString& data)
{
	wString     buf;
	wString     ptr;
	HttpUrlParts target_info;
	SOCKET      server_socket;                  //サーバーソケット
	if (!parse_http_url(url, target_info)) {
		return wString ();
	}
	server_socket = open_http_socket(target_info);
	if (SERROR(server_socket)) {
		return wString();
	}

	if (methods == "GET") {
		ptr.sprintf("%s %s HTTP/1.0\r\n"
					"Accept: */*\r\n"
					"User-Agent: %s%s\r\n"
					"Host: %s\r\n"
					"Connection: close\r\n\r\n",
					methods.c_str(),
					target_info.target.c_str(),
					USERAGENT,
					MACADDR,
					target_info.host.c_str());
	}
	else {
		ptr.sprintf("%s %s HTTP/1.0\r\n"
					"Accept: */*\r\n"
					"User-Agent: %s%s\r\n"
					"Host: %s\r\n"
					"Content-Length: %d\r\n"
					"Content-Type: text/plain;charset=UTF-8\r\n"
					"Connection: close\r\n\r\n",
					methods.c_str(),
					target_info.target.c_str(),
					USERAGENT,
					MACADDR,
					target_info.host.c_str(),
					data.length());
		ptr += data;
	}
	if (transport_send(server_socket, ptr.c_str(), ptr.length(), 0) == SOCKET_ERROR) {
		transport_close(server_socket);
		return wString();
	}
	if (!read_http_response(server_socket, buf)) {
		transport_close(server_socket);
		return wString();
	}
	transport_close(server_socket);
	auto status = get_http_status_code(buf);
	if (status < 200 || 300 <= status) {
		return wString();
	}
	if (!split_http_response(buf, ptr)) {
		return wString();
	}
	return ptr;
}
///---------------------------------------------------------------------------
/// <summary>
/// ソケットを作成し、相手に接続するラッパー
/// </summary>
/// <param name="host">ホスト名</param>
/// <param name="port">ポート</param>
/// <returns>成功時:ソケット番号、失敗時:-1</returns>
SOCKET wString::sock_connect (const wString& host, const int port)
{
	return sock_connect (host.String, port);
}
///---------------------------------------------------------------------------
/// <summary>
/// ソケット接続
/// </summary>
/// <param name="host">ホスト名</param>
/// <param name="port">ポート</param>
/// <returns>成功時:ソケット番号、失敗時:-1</returns>
SOCKET wString::sock_connect (const char* host, const int port)
{
	//  int sock;
	SOCKET sock;
	struct sockaddr_in sockadd = {};     //ＳＯＣＫＥＴ構造体
	struct hostent* hent;
	debug_log_output ("sock_connect: %s:%d", host, port);
	//ＳＯＣＫＥＴ作成
	if (SERROR (sock = socket (PF_INET, SOCK_STREAM, 0))) {
		debug_log_output ("sock_connect_error:");
		return INVALID_SOCKET;
	}
	//debug_log_output("sock: %d", sock);
	if (NULL == (hent = gethostbyname (host))) {
		sClose (sock);
		return INVALID_SOCKET;
	}
	debug_log_output ("hent: %p", hent);
	//ソケット構造体へアドレス設定
	memcpy (&sockadd.sin_addr, hent->h_addr, hent->h_length);
	//ソケット構造体へポート設定
	sockadd.sin_port = htons ((u_short)port);
	//ＩＰＶ４アドレスファミリを設定
	sockadd.sin_family = AF_INET;
	//接続
	if (SERROR (connect (sock, reinterpret_cast<struct sockaddr*>(&sockadd), sizeof (sockadd)))) {
		sClose (sock);
		return INVALID_SOCKET;
	}
	return sock;
}
///---------------------------------------------------------------------------
/// <summary>
/// URLがアクセス可能かチェックする
/// /形式はwww.make-it.co.jp/index.html
/// </summary>
/// <param name="url">アクセス先url</param>
/// <returns>アクセス可能ならtrue</returns>
bool wString::check_url (const wString& url)
{
	wString buf;
	bool    access_flag = false;
	HttpUrlParts target_info;
	if (url.starts_with("http://") || url.starts_with("https://")) {
		if (!parse_http_url(url, target_info)) {
			return false;
		}
	}
	else {
		auto ptr = url.Pos("/");
		target_info.host = url.substr(0, ptr);
		target_info.target = url.substr(ptr, url.length() - ptr);
		target_info.port = HTTP_SERVER_PORT;
		if (target_info.target.length() == 0) {
			target_info.target = "/";
		}
	}
	SOCKET server_socket = open_http_socket(target_info);
	if (!SERROR(server_socket)) {
		buf.sprintf ("HEAD %s HTTP/1.0\r\n"
					 "User%cAgent: "
					 USERAGENT
					 "\r\n"
					 "Host: %s\r\n"
					 "Connection: close\r\n"
					 "\r\n",
					 target_info.target.c_str (),
					 '-',
					 target_info.host.c_str ()
		);
		int dd = transport_send(server_socket, buf.c_str(), buf.length(), 0);
		if (dd != SOCKET_ERROR) {
			buf.set_length (1024);
			auto recv_len = transport_recv(server_socket, buf.c_str(), buf.capacity() - 1, 0);
			buf.reset_length (recv_len);
			//見つからない
			auto ptr = atoi (buf.String + (buf.Pos (" ") + 1));
			// 受信データありならば(ファイル有り）、データを解析する。
			if (200 <= ptr && ptr < 300) {
				access_flag = true;
			}
		}
		transport_close(server_socket);
	}
	return access_flag;
}
///---------------------------------------------------------------------------
/// <summary>
/// 対象httpのサイズ取得
/// </summary>
/// <param name="url">参照先URL</param>
/// <returns>取得した場合コンテンツ長さ、エラー時-1</returns>
int wString::http_size (const wString& url)
{
	wString     buf;
	HttpUrlParts target_info;
	int         content_length = -1;
	SOCKET      server_socket;                          //サーバーソケット
	if (!parse_http_url(url, target_info)) {
		return -1;
	}
	server_socket = open_http_socket(target_info);
	if (!SERROR(server_socket)) {
		buf.sprintf ("HEAD %s HTTP/1.0\r\n"
					 "Accept: */*\r\n"
					 "User-Agent: %s%s\r\n"
					 "Host: %s\r\n"
					 "Connection: close\r\n\r\n",
					 target_info.target.uri_encode ().c_str (),
					 USERAGENT,
					 MACADDR,
					 target_info.host.c_str ()
		);
		if (transport_send(server_socket, buf.c_str(), buf.length(), 0) != SOCKET_ERROR) {
			buf.set_length (HTTP_STR_BUF_SIZE);
			auto recv_len = transport_recv(server_socket, buf.c_str(), buf.capacity() - 1, 0);
			buf.reset_length (recv_len);      //糸止め
			auto work1 = atoi (buf.String + (buf.Pos (" ") + 1));
			int pos = buf.Pos ("Content-Length:");
			if (pos >= 0) {
				if (200 <= work1 && work1 < 300) {
					content_length = atoi (buf.c_str () + pos + 16);
				}
			}
			else if (work1 == 302) {
				//Location:----\r\n
				work1 = buf.Pos ("Location:");
				if (work1) {
					int num = buf.Pos ("\r\n", work1) - work1 - 10;
					buf = buf.substr (work1 + 10, num);
					content_length = http_size (buf);
				}
			}
		}
		transport_close(server_socket);
	}
	//終了
	return content_length;
}
#endif

bool wString::path_sanitize()
{
	if (empty()) {
		*this = "/";
		return true;
	}

	wString drive;   // Windows の "C:" を保持する
	int pos = 0;
	const size_t n = this->size();

	// --- Windows ドライブレター判定 ---
	// 例: "C:/Users/user"
	if (n >= 2 && (*this)[1] == ':' &&
		(((*this)[0] >= 'A' && (*this)[0] <= 'Z') ||
			((*this)[0] >= 'a' && (*this)[0] <= 'z')))
	{
		drive = this->substr(0, 2);  // "C:"
		pos = 2;

		// ドライブの後が "/" ならスキップ
		if (pos < n && (*this)[pos] == '/')
			pos++;
	}

	// --- パス部分の正規化 ---
	std::vector<wString> stack;

	while (pos < n) {
		// skip leading '/'
		while (pos < n && (*this)[pos] == '/') pos++;

		if (pos >= n) break;

		// extract one component
		size_t start = pos;
		while (pos < n && (*this)[pos] != '/') pos++;
		wString part = this->substr(start, pos - start);

		if (part == "..") {
			if (stack.empty()) {
				// Windows の場合は "C:" があるのでそこまでは戻れる
				if (!drive.empty()) {
					// 例: C:/.. → C:
					// ただし C: から上には行けない
					// stack が空なら何もしない
					continue;
				}

				// Linux の場合は不正
				this->clear();
				return false;
			}
			stack.pop_back();
		}
		else if (part != ".") {
			stack.push_back(part);
		}
	}

	// --- 再構築 ---
	wString out;

	if (!drive.empty()) {
		out = drive;  // "C:"
	}

	if (stack.empty()) {
		// Windows: "C:/"
		// Linux:   "/"
		out += "/";
	}
	else {
		for (auto& s : stack) {
			out += "/";
			out += s;
		}
	}

	*this = out;
	return true;
}


//---------------------------------------------------------------------------
// linux用ファイル名に変換
//---------------------------------------------------------------------------

/// <summary>
/// Windows用パス名に変更(副作用あり）
/// </summary>
/// <param name="FileName">変換するパス名</param>
/// <returns>変換した文字列</returns>
char* wString::windows_file_name (char* FileName)
{
#ifdef __linux__
	return FileName;
#else
	char* work = FileName;
	int ptr = 0;
	while (work[ptr]) {
		if (work[ptr] == '/') {
			work[ptr] = '\\';
		}
		ptr++;
	}
	return work;
#endif
}
/// <summary>
/// Windows用パス名に変更(副作用あり）
/// </summary>
/// <param name="FileName">変換するパス名</param>
/// <returns>変換した文字列</returns>
wString wString::windows_file_name ()
{
#ifdef __linux__
	return *this;
#else

	//char* work = FileName;
	for (auto i = 0U; i < len; i++) {
		if (String[i] == '/') {
			String[i] = '\\';
		}
	}
	return *this;
#endif
}
/// <summary>
/// Linux用パス名に変換（自データを変更）
/// </summary>
/// <param name="FileName">変換するパス名</param>
/// <returns>変換した文字列</returns>
char* wString::linux_file_name (char* FileName)
{
#ifdef __linux__
	return FileName;
#else
	char* work = FileName;
	int ptr = 0;
	while (work[ptr]) {
		if (work[ptr] == '\\') {
			work[ptr] = '/';
		}
		ptr++;
	}
	return work;
#endif
}
// get_local_address/get_local_port はsocket不使用のためコメントアウト
#if 0  // was: /*
/// <summary>
/// ローカルアドレスを文字列として返す
/// </summary>
/// <returns>ローカルアドレス</returns>
wString wString::get_local_address (void)
{
#ifdef __linux__
	//linux番の自IP/ホスト名を設定する
	struct ifaddrs* ifa_list;
	struct ifaddrs* ifa;
	char addrstr[256] = {};

	// 機器リスト取得
	if (getifaddrs (&ifa_list) == 0) {
		// 一覧作成
		for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr->sa_family == AF_INET) {
				// eth0の場合のみ
				if (*ifa->ifa_name == 'e') {
					inet_ntop (AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, addrstr, sizeof (addrstr));
					break;
				}
			}
		}
		/* (7) */
		freeifaddrs (ifa_list);
		return addrstr;
	}
	else {
		return "";
	}
#else
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
	//ホスト名を取得する
	char hostname[256];
	if (gethostname (hostname, sizeof (hostname)) != 0) {
		throw new CScriptException ("get_local_address:GetHostName error.");
		//perror ("ディレクトリのオープンエラー");
		//return "";
	}
	//puts(hostname);

	// 有効なルーティングを検索
	// アクセス対象はDNS1.1.1.1
	DWORD dwDestAddr = 0x1111;
	PMIB_IPFORWARDROW pBestRoute = (PMIB_IPFORWARDROW)malloc (sizeof (MIB_IPFORWARDROW));
	if (pBestRoute == NULL) {
		// TODO:Throwする
		throw new CScriptException ("get_local_address:memory access error.");
		//exit (1);
	}
	DWORD dwRetVal = GetBestRoute (dwDestAddr, 0, pBestRoute);
	if (dwRetVal == NO_ERROR) {
		dwDestAddr = pBestRoute->dwForwardNextHop;
		//printf("Interface Index: %d\n", pBestRoute->dwForwardIfIndex);
		//printf("Next Hop: %s\n", inet_ntoa(*(struct in_addr*)&pBestRoute->dwForwardNextHop));
	}
	else {
		dwDestAddr = 0;
	}
	free (pBestRoute);

	//ホスト名からIPアドレスを取得する
	HOSTENT* hostend = gethostbyname (hostname);
	if (hostend == NULL) {
		return "";
	}
	IN_ADDR inaddr = {};
	// GetBestRouteで外に出られるアドレスを正とする。
	// もし全てのインターフェースが外に出られる場合はGetBestRouteが選択したルートとする。
	////for (auto i = 0; i < hostend->h_length; i++) {
	auto i = 0;
	while (hostend->h_addr_list[i] != NULL) {
		//DWORD aaa = (Dhostend->h_addr_list[i];
		// TODO:３バイト比較。厳密にすべき。
		if (memcmp (static_cast<void*>(hostend->h_addr_list[i]), static_cast<void*>(&dwDestAddr), 3) == 0) {
			inaddr.S_un.S_un_b.s_b1 = hostend->h_addr_list[i][0];
			inaddr.S_un.S_un_b.s_b2 = hostend->h_addr_list[i][1];
			inaddr.S_un.S_un_b.s_b3 = hostend->h_addr_list[i][2];
			inaddr.S_un.S_un_b.s_b4 = hostend->h_addr_list[i][3];
			break;
		}
		i += 1;
	}
	//ip.resize(256);
	//static char ip[256];
	//strcpy(ip, inet_ntoa(inaddr));
	return inet_ntoa (inaddr);
#endif
}
extern GLOBAL_PARAM_T global_param;
/// <summary>
/// ローカルポートを文字列として返す
/// </summary>
/// <returns>ローカルポート</returns>
int wString::get_local_port (void)
{
	return global_param.server_port;
}
#endif  // was: */
//---------------------------------------------------------------------------

/// <summary>
/// コピー先文字列に元文字列のindexからslen分コピーする
/// バイナリ非対応
/// </summary>
/// <param name="str">コピー先文字列</param>
/// <param name="slen">コピーする文字数</param>
/// <param name="index">開始位置</param>
/// <returns></returns>
unsigned int wString::copy (char* str, unsigned int slen, int index) const
{
	strncpy (str, String + index, slen);
	str[slen] = 0;
	return slen;
}
///---------------------------------------------------------------------------
/// <summary>
/// 開始位置から指定文字数分変更先文字列に置換する
/// </summary>
/// <param name="index">開始位置</param>
/// <param name="slen">変更する文字数</param>
/// <param name="repstr">変更先文字列</param>
/// <returns></returns>
wString& wString::replace (int index, unsigned int slen, const wString& repstr)
{
	auto rlen = repstr.len;
	//同じ
	if (slen == rlen) {
		memcpy (String + index, repstr.String, rlen);
		//前詰め置換そのままコピーすればいい
	}
	else if (slen > rlen) {
		int num = slen - rlen;
		char* p = String + index;
		char* q = p + num;
		while (*q) {
			*p++ = *q++;
		}
		memcpy (String + index, repstr.String, rlen);
		len -= num;
		String[len] = 0;
		//置換文字が長いので後詰めする
	}
	else {
		int num = rlen - slen;
		resize (len + num + 1);
		for (char* p = (String + len + num); p > String + index + num; p--) {
			*p = *(p - num);
		}
		memcpy (String + index, repstr.String, rlen);
		len += num;
		String[len] = 0;
	}
	return *this;
}

#define ToBase64tbl "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
/// <summary>
/// BASE64エンコード
/// </summary>
/// <param name="">エンコードする文字列</param>
/// <returns>エンコードした文字列</returns>
wString wString::base64 (void)
{
	wString tmpout;
	wString work (len + 3);
	memcpy (reinterpret_cast<void*>(work.String), String, len);
	work.String[len] = 0;
	work.String[len + 1] = 0;
	work.String[len + 2] = 0;

	unsigned char* instr = reinterpret_cast<unsigned char*>(work.String);
	int count = 0;
	/*
	ABC -> 414243 -> 0100 0001  0100 0010  0100 0011
		-> 010000 010100 001001 000011
		   10 14 09 03
	*/
	/// 文字数*8ビットから1回あたり６ビットとる。残りビットがある間処理を行う
	for (int i = len * 8; i > 0; i -= 6) {
		unsigned char ch = 0;
		switch (count) {
		case 0:
			ch = (unsigned char)((*instr >> 2) & 0x3f);
			break;
		case 1:
			ch = (unsigned char)((*instr++ << 4) & 0x3f);
			ch |= (unsigned char)((*instr >> 4) & 0x3f);
			break;
		case 2:
			ch = (unsigned char)((*instr++ << 2) & 0x3f);
			ch |= (unsigned char)((*instr >> 6) & 0x3f);
			break;
		case 3:
			ch = (unsigned char)((*instr++) & 0x3f);
			break;
		}
		tmpout += ToBase64tbl[ch];
		count++;
		count %= 4;
	}
	count = (4 - count) % 4;
	while (count-- > 0) {
		tmpout += '=';
	}
	//tmpout += '\0';
	return tmpout;
}
// TODO range chack
/// <summary>
/// BASE64デコード
/// </summary>
/// <param name="">デコードする文字列</param>
/// <returns>デコードされた文字列</returns>
wString wString::unbase64 (void)
{
	const unsigned char* instr = reinterpret_cast<unsigned char*>(String);

	char FromBase64tbl[256] = {};
	wString tmpout;
	tmpout.clear ();
	//逆テーブルの作成,=も０になる
	for (unsigned int i = 0; i < sizeof (ToBase64tbl); i++) {
		FromBase64tbl[(int)ToBase64tbl[i]] = (unsigned char)i;
	}

	/*
	ABC -> 414243 -> 0100 0001  0100 0010  0100 0011
	->   010000 010100 001001 000011
	10 14 09 03
	*/
	int ll = len;
	while (ll > 0) {
		int s1 = FromBase64tbl[*instr++];
		int s2 = FromBase64tbl[*instr++];
		char ch = (char)((s1 << 2) | (s2 >> 4));
		if (ch) {
			int s3 = FromBase64tbl[*instr++];
			tmpout += ch;
			ch = (char)(((s2 & 0x0f) << 4) | (s3 >> 2));
			if (ch) {
				int s4 = FromBase64tbl[*instr++];
				tmpout += ch;
				ch = (char)(((s3 & 0x03) << 6) | s4);
				if (ch) {
					tmpout += ch;
				}
			}
		}
		ll -= 4;
	}
	//tmpout += '\0';
	return tmpout;
}

/// <summary>
/// 文字列中の引用符、エスケープを二重エスケープ
/// </summary>
/// <param name="str">エスケープする文字列</param>
/// <returns>エスケープ結果</returns>
wString wString::escape (const wString& str)
{

	wString dst;
	unsigned int len = str.length ();
	// 引数チェック
	for (auto is = 0U; is < len; is++) {
		if (str[is] == '\"') {
			dst += "\\\"";
		}
		else if (str[is] == '\\') {
			dst += "\\\\";
		}
		else {
			dst += str[is];
		}
	}
	return dst;
}
// line_receive はsocket不使用のためコメントアウト
/*
/// <summary>
/// accept_socketから、１行(CRLFか、LF単独が現れるまで)受信
/// CRLFは削除する。
/// </summary>
/// <param name="accept_socket"></param>
/// <returns>受信したサイズ,エラーは-1を返す</returns>
int wString::line_receive (SOCKET accept_socket)
{
	char 	byte_buf;
	clear ();
	// １行受信実行
	while (1) {
		int recv_len = transport_recv (accept_socket, &byte_buf, 1, 0);
		if (recv_len == 0) {
			if (len) {
				return len;
			}
			else {
				return -1;
			}
		}
		else if (recv_len < 0) { // 受信失敗チェック
#ifdef __linux__
			debug_log_output ("header read error cnt = %d error=%s\n", recv_len, strerror (errno));
#else
			debug_log_output ("header read error cnt = %d socket=%d error=%d\n", recv_len, accept_socket, WSAGetLastError ());
#endif
			return (-1);
		}
		else {
			// CR/LFチェック
			if (byte_buf == '\r') {
				continue;
			}
			else if (byte_buf == '\n') {
				break;
			}
			// バッファにセット
			*this += (char)byte_buf;
		}
	}
	return len;
}
*/

/// <summary>
/// utf8文字列の長さ
/// range       8bit       5bit    bytes
/// 0xxx - xxxx:00 - 7f -> 00 - 0f 1
/// 110x - xxxx:c0 - df -> 18 - 1b 2
/// 1110 - xxxx:e0 - ef -> 1c - 1d 3
/// 1111 - 0xxx:f0 - f7 -> 1e - 1f 4
/// </summary>
/// <param name="str">utf-8文字列</param>
/// <returns>文字数</returns>
int wString::strlen_utf8 (char* str)
{
	const static int hb[32] = {
		//0 1 2 3 4 5 6 7 8 9 A B C D E F
		  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,//0x00
		  1,1,1,1,1,1,1,1,2,2,2,2,3,3,4,1,//0x00
	};

	int cnt = 0;
	while (*str) {
		auto target = *(reinterpret_cast<unsigned char*>(str));
		auto current_len = hb[target >> 3];
		str += current_len;
		cnt++;
	}
	return cnt;
}

/// <summary>
/// sentence文字列の、cut_charが最初に出てきた所から前を削除
/// もし、cut_charがsentence文字列に入っていなかった場合、文字列全部削除
/// </summary>
/// <param name="cut_char">分離文字</param>
/// <returns></returns>
bool wString::cut_before_character (const char cut_char)
{
	// 削除対象キャラクターが最初に出てくる所を探す。
	int ptr = find (cut_char);
	if (ptr == wString::npos) {
		// 発見できなかった場合、文字列全部削除。
		clear ();
		return false;
	}
	memmove(String, String + ptr + 1, len - ptr);
	len -= (ptr + 1);
	return true;
}
/// <summary>
/// sentence文字列より、cut_charから後ろを削除
///      見つからなければ何もしない。
/// </summary>
/// <param name="cut_char">分離文字</param>
/// <returns>成功:true 失敗:false</returns>
bool wString::cut_after_character (const char cut_char)
{
	int ptr = find (cut_char);
	if (ptr == wString::npos) {
		return false;
	}
	String[ptr] = 0;
	len = ptr;
	return true;
}
//************************************************************************
// sentence文字列の、cut_charが最後に出てきた所から後ろをCUT
// もし、cut_charがsentence文字列に入っていなかった場合、文字列全部削除。
//************************************************************************
bool wString::cut_after_last_character(const char cut_char)
{
	int ptr = rfind(cut_char);
	if( ptr == wString::npos) {
		return false;
	}

	String[ptr] = 0;
	len = ptr;
	return true;
}

//*********************************************************
// sentence文字列より、最初に出て来たcut_charの前後を分割。
//
//      sentence        (IN) 分割対象の文字列を与える。
//      cut_char        (IN) 分割対象の文字を入れる。
//      split1          (OUT)カットされた前の部分が入る。sentenceと同等のサイズが望ましい。
//      split2          (OUT)カットされた後ろの部分が入る。sentenceと同等のサイズが望ましい。
//
//
// return
//              0:                      正常終了。
//              それ以外：      エラー。分割失敗などなど。
//*********************************************************
bool wString::sentence_split(char cut_char, wString& split1, wString& split2)
{
	split1.clear();
	split2.clear();
	char* p = String;
	char* ptr = nullptr;
	// エラーチェック。
	if (len == 0 ) {
		return false;       // 引数にNULLまじり。
	}
	int pos = find(cut_char);
	// 分割文字発見できず。
	if (pos == wString::npos) {
		return false;
	}
	ptr = p+pos;//分割文字の位置をポインタに変換
	//比較しながら複写
	while (*p) {
		//分割文字より前半
		if (p < ptr) {
			split1 += *p++;
			//分割文字より後半
		}
		else if (p > ptr) {
			split2 += *p++;
			//分割文字位置
		}
		else {
			p++;
		}
	}
	return 0; // 正常終了。
}









//////////////////////////////////////////////////////////////////////////
/// <summary>
/// pos位置に指定文字を挿入(バイナリ単位）
/// </summary>
/// <param name="pos">挿入位置</param>
/// <param name="fill">挿入文字</param>
/// <returns>挿入された文字</returns>
wString wString::insert (int pos, const wString& fill)
{
	wString str1;
	wString str2;
	str1 = substr (0, pos);
	str2 = substr (pos);
	return str1 + fill + str2;
}

//////////////////////////////////////////////////////////////////////////
/// <summary>
/// PNGフォーマットファイルから、画像サイズを得る。
/// </summary>
/// <param name="png_filename"></param>
/// <returns>json:{"x":</returns>
wString wString::png_size (const wString& png_filename)
{
	int         fd;
	unsigned char       buf[255] = {};
	ssize_t     read_len;
	auto x = 0;
	auto y = 0;
	fd = myopen (png_filename, O_BINARY | O_RDONLY);
	if (fd < 0) {
		return "null";
	}
	// ヘッダ+サイズ(0x18byte)  読む
	//memset(buf, 0, sizeof(buf));
	read_len = read (fd, reinterpret_cast<char*>(buf), 0x18);
	if (read_len == 0x18) {
		x = (buf[0x10] << 24) +
			(buf[0x11] << 16) +
			(buf[0x12] << 8) +
			(buf[0x13]);
		y = (buf[0x14] << 24) +
			(buf[0x15] << 16) +
			(buf[0x16] << 8) +
			(buf[0x17]);
	}
	close (fd);
	wString tmp;
	tmp.sprintf ("{\"x\":%d,\"y\":%d}", x, y);
	return tmp;
}
/// <summary>
///  GIFフォーマットファイルから、画像サイズを得る
/// </summary>
/// <param name="gif_filename"></param>
/// <returns></returns>
wString wString::gif_size (const wString& gif_filename)
{
	int         fd;
	unsigned char       buf[255] = {};
	ssize_t     read_len;
	auto x = 0;
	auto y = 0;
	fd = myopen (gif_filename, O_BINARY | O_RDONLY);
	if (fd < 0) {
		return "null";
	}
	// ヘッダ+サイズ(10byte)  読む
	//memset(buf, 0, sizeof(buf));
	read_len = read (fd, reinterpret_cast<char*>(buf), 10);
	if (read_len == 10) {
		x = buf[6] + (buf[7] << 8);
		y = buf[8] + (buf[9] << 8);
	}
	close (fd);
	wString tmp;
	tmp.sprintf ("{\"x\":%d,\"y\":%d}", x, y);
	return tmp;
}

/// <summary>
///  JPEGフォーマットファイルから、画像サイズを得る。
/// </summary>
/// <param name="jpeg_filename"></param>
/// <returns></returns>
wString wString::jpeg_size (const wString& jpeg_filename)
{
	int                 fd;
	unsigned char       buf[255];
	off_t               vlength;
	auto x = 0;
	auto y = 0;
	//debug_log_output("jpeg_size: '%s'.", jpeg_filename);
	fd = myopen (jpeg_filename, O_BINARY | O_RDONLY);
	if (fd < 0) {
		return "null";
	}
	while (1) {
		// マーカ(2byte)  読む
		auto read_len = read (fd, buf, 2);
		if (read_len != 2) {
			//debug_log_output("fraed() EOF.\n");
			break;
		}
		// Start of Image.
		if ((buf[0] == 0xFF) && (buf[1] == 0xD8)) {
			continue;
		}
		// Start of Frame 検知
		if ((buf[0] == 0xFF) && (buf[1] >= 0xC0) && (buf[1] <= 0xC3)) { // SOF 検知
			//debug_log_output("SOF0 Detect.");
			// sof データ読み込み
			memset (buf, 0, sizeof (buf));
			read_len = read (fd, buf, 0x11);
			if (read_len != 0x11) {
				debug_log_output ("fraed() error.\n");
				break;
			}
			y = (buf[3] << 8) + buf[4];
			x = (buf[5] << 8) + buf[6];
			break;
		}
		// SOS検知
		if ((buf[0] == 0xFF) && (buf[1] == 0xDA)) { // SOS 検知
			//debug_log_output("Start Of Scan.\n");
			// 0xFFD9 探す。
			while (1) {
				// 1byte 読む
				read_len = read (fd, buf, 1);
				if (read_len != 1) {
					//debug_log_output("fraed() error.\n");
					break;
				}
				// 0xFFだったら、もう1byte読む
				if (buf[0] == 0xFF) {
					buf[0] = 0;
					if (read (fd, buf, 1) != 1) {
						break;
					}
					// 0xD9だったら 終了
					if (buf[0] == 0xD9) {
						//debug_log_output("End Of Scan.\n");
						break;
					}
				}
			}
			continue;
		}
		// length 読む
		memset (buf, 0, sizeof (buf));
		if (read (fd, buf, 2) != 2) {
			close (fd);
			return "";
		}
		vlength = (buf[0] << 8) + buf[1];
		// length分とばす
		lseek (fd, vlength - 2, SEEK_CUR);
	}
	close (fd);
	wString tmp;
	tmp.sprintf ("{\"x\":%d,\"y\":%d}", x, y);
	return tmp;
}

/// <summary>
/// Get BIOS UUID in a 16 byte array.
/// </summary>
/// <returns>Return BIOS UUID string</returns>
wString wString::bios_uuid()
{
	wString tmp;
#ifndef __linux__
	unsigned char uuid[16] = {};
	int cnt = 0;
	/// <summary>
    /// SMBIOS Structure header as described at
    /// https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf 
    /// (para 6.1.2)
    /// </summary>
	struct dmi_header
	{
		BYTE type;
		BYTE length;
		WORD handle;
		BYTE data[1];
	};

	/// <summary>
    /// Structure needed to get the SMBIOS table using GetSystemFirmwareTable API.
    /// see https ://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
    /// </summary>
	struct RawSMBIOSData {
		BYTE  _Used20CallingMethod;
		BYTE  _SMBIOSMajorVersion;
		BYTE  _SMBIOSMinorVersion;
		BYTE  _DmiRevision;
		DWORD  Length;
		BYTE  SMBIOSTableData[1];
	};

	bool result = false;

	RawSMBIOSData* smb = nullptr;
	BYTE* data;

	DWORD size = 0;

	// Get size of BIOS table
	size = GetSystemFirmwareTable('RSMB', 0, smb, size);
	smb = static_cast<RawSMBIOSData*>(malloc(size));

	// Get BIOS table
	GetSystemFirmwareTable('RSMB', 0, smb, size);

	// Go through BIOS structures
	data = smb->SMBIOSTableData;
	while (data < smb->SMBIOSTableData + smb->Length)
	{
		BYTE* next;
		const dmi_header* h = reinterpret_cast<dmi_header*>(data);

		if (h->length < 4) {
			break;
		}

		//Search for System Information structure with type 0x01 (see para 7.2)
		if (h->type == 0x01 && h->length >= 0x19)
		{
			data += 0x08; //UUID is at offset 0x08

			// check if there is a valid UUID (not all 0x00 or all 0xff)
			bool all_zero = true, all_one = true;
			for (int i = 0; i < 16 && (all_zero || all_one); i++)
			{
				if (data[i] != 0x00) all_zero = false;
				if (data[i] != 0xFF) all_one = false;
			}
			if (!all_zero && !all_one)
			{
				/* As off version 2.6 of the SMBIOS specification, the first 3 fields
				of the UUID are supposed to be encoded on little-endian. (para 7.2.1) */
				uuid[cnt++] = data[3];
				uuid[cnt++] = data[2];
				uuid[cnt++] = data[1];
				uuid[cnt++] = data[0];
				uuid[cnt++] = data[5];
				uuid[cnt++] = data[4];
				uuid[cnt++] = data[7];
				uuid[cnt++] = data[6];
				for (int i = 8; i < 16; i++) {
					uuid[cnt++] = data[i];
				}

				result = true;
			}
			break;
		}

		//skip over formatted area
		next = data + h->length;

		//skip over unformatted area of the structure (marker is 0000h)
		while (next < smb->SMBIOSTableData + smb->Length && (next[0] != 0 || next[1] != 0)) {
			next++;
		}
		next += 2;

		data = next;
	}
	free(smb);


	tmp.sprintf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
		uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
#else
    const char* cmd = "sudo dmidecode -t system|grep UUID";
	char buffer[1024] = {};
	FILE* pipe = popen(cmd, "r");
	if (pipe) {
		while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
			tmp += buffer;
			memset(buffer, 0, sizeof(buffer));
		}
		// 頭をカット
		if (tmp.starts_with("UUID: ")) {
			tmp = tmp.substr(6);
		}
	}
#endif
	return tmp;
}

