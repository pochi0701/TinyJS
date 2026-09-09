#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
 *
 * - Useful language functions
 *
 * Authored By Gordon Williams <gw@pur3.co.uk>
 *
 * Copyright (C) 2009 Pur3 Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "TinyJS_Functions.h"
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include "dregex.h"
//#include "ltn.h"      // socket不使用のためコメントアウト
#include "ltn_tools.h"
#include "define.h"
//#include "Lutino.h"   // socket不使用のためコメントアウト
 //#include "unit1.h"
using namespace std;
//void headerCheckPrint(SOCKET mysocket, int* printed, wString* headerBuf, int flag);  // socket不使用のためコメントアウト
//wString _DBConnect(const wString& database);  // DB不使用のためコメントアウト
//int     _DBDisConnect(wString& key);
//wString _DBSQL(const wString& key, wString& sql);

//extern vector<multipart*> mp;  // socket不使用のためコメントアウト

// ----------------------------------------------- Actual Functions
void js_print(CScriptVar* v, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	//headerCheck(js->socket, &(js->printed), js->headerBuf,1);
	wString str = v->getParameter("text")->getString();
	//int num = send( js->socket, str.c_str(), str.length(), 0);
	//if( num <0 ){
	 //   debug_log_output( "Script Write Error at js_print" );
	//}
	//debug_log_output( "print %s %d", str.c_str(), num );
	printf("%s", v->getParameter("text")->getString().c_str());
}

void scTrace(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(c);
	CTinyJS* js = static_cast<CTinyJS*>(userdata);
	js->root->trace();
}

void scObjectDump(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	c->getParameter("this")->trace("> ");
}

void scObjectClone(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	CScriptVar* obj = c->getParameter("this");
	c->getReturnVar()->copyValue(obj);
}
void scKeys(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString list = c->getParameter("obj")->trace2();
	CScriptVar* result = c->getReturnVar();
	result->setArray();
	int length = 0;
	int count = list.lines();
	for (int i = 0; i < count; i++) {
		result->setArrayIndex(length++, new CScriptVar(list.get_list_string(i)));
	}
}

void scMathRand(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	//srand((unsigned int)time(NULL));
	c->getReturnVar()->setDouble((double)rand() / RAND_MAX);
}

void scMathRandInt(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	//srand((unsigned int)time(NULL));
	int min = c->getParameter("min")->getInt();
	int max = c->getParameter("max")->getInt();
	int val = min + (int)(rand() % (1 + max - min));
	c->getReturnVar()->setInt(val);
}
/////////////////////////////////////////////////////////////////////////

void scPrint(CScriptVar* c, void*)
{
	wString val = c->getParameter("val")->getString();
	printf("%s", val.c_str());
	c->getReturnVar()->setString(val);
}
/////////////////////////////////////////////////////////////////////////
void scTrim(CScriptVar* c, void*)
{
	wString val = c->getParameter("this")->getString();
	c->getReturnVar()->setString(val.trim());
}
//
void scRTrim(CScriptVar* c, void*)
{
	wString val = c->getParameter("this")->getString();
	c->getReturnVar()->setString(val.rtrim());
}
//
void scLTrim(CScriptVar* c, void*)
{
	wString val = c->getParameter("this")->getString();
	c->getReturnVar()->setString(val.ltrim());
}
/////////////////////////////////////////////////////////////////////////
void scCharToInt(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("ch")->getString();
	int val = 0;
	if (str.length() > 0) {
		val = (int)str.c_str()[0];
	}
	c->getReturnVar()->setInt(val);
}

void scStringIndexOf(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString search = c->getParameter("search")->getString();
	int p = str.find(search);
	int val = (p == wString::npos) ? -1 : p;
	c->getReturnVar()->setInt(val);
}
//Substring
void scStringSubstring(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int len = str.length();
	int lo = c->getParameter("lo")->getInt();
	// hi is optional; if omitted, use string length
	CScriptVar* args = c->getParameter("arguments");
	int hi = (args && args->getArrayLength() < 2) ? len : c->getParameter("hi")->getInt();
	if (lo < 0) lo = 0;
	if (lo > len) lo = str.length();
	if (hi < 0) hi = 0;
	if (hi > len) hi = str.length();
	if (lo > hi) {
		auto temp = hi;
		hi = lo;
		lo = temp;
	}
	int lex = hi - lo;
	if (lex > 0 && lo >= 0 && lo + lex <= (int)str.length())
		c->getReturnVar()->setString(str.substr(lo, lex));
	else
		c->getReturnVar()->setString("");
}
//SubStr
void scStringSubstr(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int lo = c->getParameter("lo")->getInt();
	int hi = c->getParameter("hi")->getInt();

	if (hi > 0 && lo >= 0 && lo + hi <= (int)str.length())
		c->getReturnVar()->setString(str.substr(lo, hi));
	else
		c->getReturnVar()->setString("");
}
//startsWith
void scStringStartsWith(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString needle = c->getParameter("lo")->getString();
	//int hi = c->getParameter ("hi")->getInt ();

	//if (hi >= 0)
	//	c->getReturnVar ()->setInt (str.starts_with (needle.c_str (), hi));
	//else
	c->getReturnVar()->setInt(str.starts_with(needle.c_str()));
}
//endsWith
void scStringEndsWith(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString needle = c->getParameter("lo")->getString();
	//int hi = c->getParameter ("hi")->getInt ();

	//if (hi >= 0)
	//	c->getReturnVar ()->setInt (str.ends_with (needle.c_str (), hi));
	//else
	c->getReturnVar()->setInt(str.ends_with(needle.c_str()));
}
////startsWith
//void scStringStartsWith2 (CScriptVar* c, void* userdata)
//{
//	IGNORE_PARAMETER (userdata);
//	wString str = c->getParameter ("this")->getString ();
//	wString needle = c->getParameter ("lo")->getString ();
//
//	c->getReturnVar ()->setInt (str.starts_with (needle.c_str ()));
//}
////endsWith
//void scStringEndsWith2 (CScriptVar* c, void* userdata)
//{
//	IGNORE_PARAMETER (userdata);
//	wString str = c->getParameter ("this")->getString ();
//	wString needle = c->getParameter ("lo")->getString ();
//
//	c->getReturnVar ()->setInt (str.ends_with (needle.c_str ()));
//}

//AT

/// <summary>
/// function String.charAt(pos)
/// pos位置の文字を取得（byte単位）
/// </summary>
/// <param name="c">引き渡しデータ</param>
/// <param name="userdata"></param>
void scStringCharAt(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int p = c->getParameter("pos")->getInt();
	if (p >= 0 && p < (int)str.length()) {
		c->getReturnVar()->setString(str.substr(p, 1));
	}
	else {
		c->getReturnVar()->setString("");
	}
}

void scStringCharCodeAt(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int p = c->getParameter("pos")->getInt();
	if (p >= 0 && p < (int)str.length())
		c->getReturnVar()->setInt(str.at(p));
	else
		c->getReturnVar()->setInt(0);
}

void scStringSplit(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString sep = c->getParameter("separator")->getString();
	CScriptVar* result = c->getReturnVar();
	result->setArray();
	int length = 0;

	//consider sepatator length;
	int inc = sep.length();
	int pos = str.find(sep);
	while (pos != wString::npos) {
		result->setArrayIndex(length++, new CScriptVar(str.substr(0, pos)));
		str = str.substr(pos + inc);
		pos = str.find(sep);
	}

	if (str.size() > 0) {
		result->setArrayIndex(length++, new CScriptVar(str));
	}
}
//Replace
void scStringReplace(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString before = c->getParameter("before")->getString();
	wString after = c->getParameter("after")->getString();
	//strの中のbeforeを探す
	int pos = str.find(before);
	if (pos != wString::npos) {
		str = str.substr(0, pos) + after + str.substr(pos + before.length());
	}
	c->getReturnVar()->setString(str);
}
//ReplaceAll
void scStringReplaceAll(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString before = c->getParameter("before")->getString();
	wString after = c->getParameter("after")->getString();
	//strの中のbeforeを探す
	int pos = str.find(before);
	while (pos != wString::npos) {
		str = str.substr(0, pos) + after + str.substr(pos + before.length());
		pos = str.find(before, pos);
	}
	c->getReturnVar()->setString(str);
}
//PregReplace
void scPregStringReplace(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString result;
	wString str = c->getParameter("this")->getString();
	CScriptVar* arrp = c->getParameter("pattern");
	vector<wString> patterns;
	vector<wString> replaces;
	int pn = arrp->getArrayLength();
	if (pn) {
		CScriptVar* arrr = c->getParameter("replace");
		auto rn = arrr->getArrayLength();
		if (pn == rn) {
			for (int i = 0; i < pn; i++) {
				patterns.push_back(arrp->getArrayIndex(i)->getString());
				replaces.push_back(arrr->getArrayIndex(i)->getString());
			}
		}
	}
	else {
		wString pattern = c->getParameter("pattern")->getString();
		wString replace = c->getParameter("replace")->getString();
		patterns.push_back(pattern);
		replaces.push_back(replace);
	}
	dregex::replace(&result, str, patterns, replaces);
	c->getReturnVar()->setString(result);
}
//Match - global regex match function
void scMatch(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString pattern = c->getParameter("pattern")->getString();
	wString text = c->getParameter("text")->getString();
	int result = dregex::match(text, pattern);
	c->getReturnVar()->setInt(result);
}
//Replace - global regex replace function
void scReplace(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString text = c->getParameter("text")->getString();
	CScriptVar* arrp = c->getParameter("pattern");
	vector<wString> patterns;
	vector<wString> replaces;

	// Check if pattern is an array
	int pn = arrp->getArrayLength();
	if (pn > 0) {
		// Pattern is an array
		CScriptVar* arrr = c->getParameter("replacement");
		int rn = arrr->getArrayLength();
		if (pn == rn) {
			for (int i = 0; i < pn; i++) {
				patterns.push_back(arrp->getArrayIndex(i)->getString());
				replaces.push_back(arrr->getArrayIndex(i)->getString());
			}
		}
	}
	else {
		// Pattern is a single string
		wString pattern = c->getParameter("pattern")->getString();
		wString replacement = c->getParameter("replacement")->getString();
		patterns.push_back(pattern);
		replaces.push_back(replacement);
	}

	// Perform replacement
	wString result;
	dregex::replace(&result, text, patterns, replaces);

	// Return the replaced text
	c->getReturnVar()->setString(result);
}
//AddShashes
void scAddShashes(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	c->getReturnVar()->setString(str.add_slashes());
}
//getLocalAddress (socket不使用のためコメントアウト)
#if 0
void scGetLocalAddress(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	c->getReturnVar()->setString(wString::get_local_address());
}
//getLocalPort
void scGetLocalPort(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	c->getReturnVar()->setInt(wString::get_local_port());
}
#endif
void scStringFromCharCode(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	char str[2] = {};
	str[0] = (char)c->getParameter("char")->getInt();
	c->getReturnVar()->setString(str);
}

void scIntegerParseInt(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("str")->getString();
	int val = strtol(str.c_str(), 0, 0);
	c->getReturnVar()->setInt(val);
}

void scIntegerValueOf(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("str")->getString();

	int val = 0;
	if (str.length() == 1)
		val = str[0];
	c->getReturnVar()->setInt(val);
}
//
void scIntegerToDateString(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString times = c->getParameter("this")->getString();
	wString format = c->getParameter("format")->getString();
	char s[128] = {};
	time_t time = atol(times.c_str());
	const struct tm* timeptr;
	timeptr = localtime(&time);
	strftime(s, 128, format.c_str(), timeptr);
	c->getReturnVar()->setString(s);
}
void scStringDate(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	auto t = time(NULL);
	char s[128];
#ifdef __linux__
	sprintf(s, "%ld", t);
#else
	sprintf(s, "%lld", t);
#endif
	c->getReturnVar()->setString(s);
}
void scNKFConv(CScriptVar* c, void* userdata)
{
#ifdef WEB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString format = c->getParameter("format")->getString();
	wString temp = str.nkfcnv(format);
	c->getReturnVar()->setString(temp);
#endif
}

void scDBConnect(CScriptVar* c, void* userdata)
{
#ifdef DB
	IGNORE_PARAMETER(userdata);
	//接続DB名
	wString str = c->getParameter("dbn")->getString();
	//DB接続文字列取得
	str = _DBConnect(str);
	if (str.length() > 0) {
		//c->getParameter("this")->setString(str);
		c->getReturnVar()->setString(str);
	}
	else {
		c->getReturnVar()->setString("");
	}
#endif
}

void scDBDisConnect(CScriptVar* c, void* userdata)
{
#ifdef DB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int ret = _DBDisConnect(str);
	c->getReturnVar()->setInt(ret);
#endif
}

void scDBSQL(CScriptVar* c, void* userdata)
{
#ifdef DB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString sql = c->getParameter("sqltext")->getString();
	wString ret = _DBSQL(str, sql);
	c->getReturnVar()->setString(ret);
#endif
}


// sqlBind: ":key" 形式のプレースホルダーを、params オブジェクトの値で
// SQL標準のエスケープ（文字列は '...' で囲み、内部の ' を '' に置換）を
// 行いながら安全に置換します。対応キーが無い場合や、文字列/数値/真偽値
// 以外の型が渡された場合は例外を投げます。
void scSqlBind(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString tmpl = c->getParameter("this")->getString();
	CScriptVar* params = c->getParameter("params");

	auto isIdentStart = [](char ch) {
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
		};
	auto isIdentChar = [](char ch) {
		return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
			(ch >= '0' && ch <= '9') || ch == '_';
		};

	wString result;
	const char* p = tmpl.c_str();
	unsigned int n = tmpl.length();
	unsigned int i = 0;
	while (i < n) {
		char ch = p[i];
		if (ch == ':' && i + 1 < n && isIdentStart(p[i + 1])) {
			unsigned int j = i + 1;
			wString key;
			while (j < n && isIdentChar(p[j])) {
				key += p[j];
				j++;
			}
			CScriptVarLink* link = params ? params->findChild(key) : NULL;
			if (link == NULL) {
				throw new CScriptException("sqlBind: no value for placeholder :" + key + " in params");
			}
			CScriptVar* v = link->var;
			if (v->isNumeric()) {
				result += v->getString();
			}
			else if (v->isString()) {
				const wString& sv = v->getString();
				result += '\'';
				const char* sp = sv.c_str();
				unsigned int sn = sv.length();
				for (unsigned int k = 0; k < sn; k++) {
					if (sp[k] == '\'') {
						result += "''";
					}
					else {
						result += sp[k];
					}
				}
				result += '\'';
			}
			else {
				throw new CScriptException("sqlBind: unsupported value type for :" + key + " (only string/number/boolean allowed)");
			}
			i = j;
		}
		else {
			result += ch;
			i++;
		}
	}
	c->getReturnVar()->setString(result);
}

void scJSONStringify(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString result;
	c->getParameter("obj")->getJSON(result);
	c->getReturnVar()->setString(result.c_str());
}

void scExec(CScriptVar* c, void* userdata)
{
	CTinyJS* tinyJS = static_cast<CTinyJS*>(userdata);
	wString str = c->getParameter("jsCode")->getString();
	tinyJS->execute(str);
}

void scEval(CScriptVar* c, void* userdata)
{

	CTinyJS* tinyJS = static_cast<CTinyJS*>(userdata);
	c->setReturnVar(tinyJS->evaluateComplex(c->getParameter("jsCode")->getString()).var);
}

void scArrayContains(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	CScriptVar* obj = c->getParameter("obj");
	CScriptVarLink* v = c->getParameter("this")->firstChild;

	bool contains = false;
	while (v) {
		if (v->var->equals(obj)) {
			contains = true;
			break;
		}
		v = v->nextSibling;
	}

	c->getReturnVar()->setInt(contains);
}

void scArrayRemove(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	CScriptVar* obj = c->getParameter("obj");
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();

	std::vector<CScriptVar*> kept;
	kept.reserve(len);
	for (int i = 0; i < len; i++) {
		CScriptVar* v = arr->getArrayIndex(i);
		if (!v->equals(obj)) {
			kept.push_back(v->deepCopy());
		}
	}

	arr->removeAllChildren();
	for (int i = 0; i < (int)kept.size(); i++) {
		arr->setArrayIndex(i, kept[i]);
	}
}

void scArrayJoin(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString sep = c->getParameter("separator")->getString();
	CScriptVar* arr = c->getParameter("this");

	wString sstr;
	int lex = arr->getArrayLength();
	for (int i = 0; i < lex; i++) {
		if (i > 0) {
			sstr += sep;
		}
		sstr += arr->getArrayIndex(i)->getString();
	}

	c->getReturnVar()->setString(sstr.c_str());
}
//ファイル存在チェック
void scFileExists(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	//int flag = wString::file_exists(path);
	int flag = path.file_exists();
	c->getReturnVar()->setInt(flag);
}
//ディレクトリ存在チェック
void scDirExists(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int flag = wString::directory_exists(path);
	c->getReturnVar()->setInt(flag);
}
//htmlspecialchars
void scHtmlSpecialChars(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = uri.htmlspecialchars();
	c->getReturnVar()->setString(uri);
}
//encodeURI
void scEncodeURI(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = uri.uri_encode();
	c->getReturnVar()->setString(uri);
}
//atob
void scAtob(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("str")->getString();
	str = str.unbase64();
	c->getReturnVar()->setString(str);
}
//btoa
void scBtoa(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("str")->getString();
	str = str.base64();
	c->getReturnVar()->setString(str);
}
//dirname
void scDirname(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	while (uri.length() > 0 && uri[uri.length() - 1] != '/') {
		uri = uri.substr(0, uri.length() - 1);
	}
	if (uri.length() > 0) {
		uri = uri.substr(0, uri.length() - 1);
	}
	else {
		uri = "/";
	}
	c->getReturnVar()->setString(uri);
}
//base
void scBasename(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	int len = uri.length() - 1;
	while (len >= 0 && uri[len] != '/') {
		len--;
	}
	uri = uri.substr(len + 1, uri.length() - len - 1);
	c->getReturnVar()->setString(uri);
}

//ScanDir
void scScanDir(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString json = wString::enum_folder_json(path);
	c->getReturnVar()->setString(json);
}

//scMimeInfo
void scMimeInfo(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	auto ret = wString::find_mime_type(uri);
	c->getReturnVar()->setString(ret);
	return;
}
//extract_file_ext
void scExtractFileExt(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = wString::extract_file_ext(uri);
	c->getReturnVar()->setString(uri);
}
//toLowerCase
void scToLowerCase(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	char* String = str.c_str();
	for (unsigned int i = 0; i < str.length(); i++) {
		String[i] = (unsigned char)tolower(String[i]);
	}
	c->getReturnVar()->setString(str);
}
//toUpperCase
void scToUpperCase(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	char* String = str.c_str();
	for (unsigned int i = 0; i < str.length(); i++) {
		String[i] = (unsigned char)toupper(String[i]);
	}
	c->getReturnVar()->setString(str);
}
//ファイル属性など
void scFileStats(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString json = wString::file_stats(path);
	c->getReturnVar()->setString(json);
}
//ファイル属性など
void scFileDate(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString json = wString::file_stats(path, 1);
	c->getReturnVar()->setString(json);
}
//ファイル内容取得
void scLoadFromFile(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString data;
	data.load_from_file(path);
	c->getReturnVar()->setString(data);
}
//CSV内容取得
void scLoadFromCSV(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString data;
	data.load_from_csv(path);
	c->getReturnVar()->setString(data);
}
//ファイル削除
void scUnlink(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
#ifdef __linux__
#else
#endif
	int res = wString::delete_file(path);
	int ret = (res == 0) ? false : true;
	c->getReturnVar()->setInt(ret);
}
//ファイル作成
void scTouch(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int ret = true;
	int fd = myopen(path, O_CREAT | O_APPEND | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);
	if (fd < 0) {
		ret = false;
	}
	else {
		close(fd);
	}
	c->getReturnVar()->setInt(ret);
}
//リネーム
void scRename(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString pathf = c->getParameter("pathf")->getString();
	wString patht = c->getParameter("patht")->getString();
	int ret = wString::rename_file(pathf, patht);
	c->getReturnVar()->setInt(ret);
}
//フォルダ作成
void scMkdir(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int ret = wString::create_dir(path);
	c->getReturnVar()->setInt(ret);
}
//フォルダ削除
void scRmdir(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
#ifdef __linux__
#else
#endif
	int res = wString::delete_folder(path);
	int ret = (res == 0) ? false : true;
	c->getReturnVar()->setInt(ret);
}
//ファイル保存
void scSaveToFile(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString data = c->getParameter("data")->getString();
	int res = data.save_to_file(path);
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//command実行
void scCommand(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString ppath = c->getParameter("path")->getString();
#ifdef __linux__
#else
	ppath = ppath.nkfcnv("Ws");
#endif
	int res = system(ppath.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//Header (socket不使用のためコメントアウト)
#if 0
void scHeader(CScriptVar* c, void* userdata)
{
	CTinyJS* js = static_cast<CTinyJS*>(userdata);
	headerCheckPrint(js->socket, &(js->printed), js->headerBuf, 0);
	wString str = c->getParameter("str")->getString();
	int res = js->headerBuf->header(str.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//SessionStart
void scSessionStart(CScriptVar* c, void* userdata)
{
#ifdef WEB
	const static char material[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	CTinyJS* js = static_cast<CTinyJS*>(userdata);
	int ret = 0;
	//sidある？
	wString jssessid = js->evaluate("JSSESSID");
	if (jssessid != "undefined") {
		if (session->count(jssessid) > 0) {
			wString data = (*session)[jssessid];
			js->execute("var _SESSION=" + data + ";", ExecuteModes::ON_SERVER);
		}
		else {
			js->execute("var _SESSION={};", ExecuteModes::ON_SERVER);
		}
	}
	else {
		// initializeへ移動
		//srand((unsigned)time(NULL));
		char work[27] = {};
		while (1) {
			for (int i = 0; i < 26; i++) {
				work[i] = material[rand() % (sizeof(material) - 1)];
			}
			//同じモノはダメ
			if (session->count(work) == 0) break;
		}
		jssessid = work;
		js->execute("var _SESSION={};var sid=\"" + jssessid + "\";");
		//新規にセッションを作る時はcookieを送出(ブラウザ閉じるまで)
		headerCheckPrint(js->socket, &(js->printed), js->headerBuf, 0);
		wString str;
		str.sprintf("Set-Cookie: sid=%s;", jssessid.c_str());
		int res = js->headerBuf->header(str.c_str());
		ret = (res == 0) ? true : false;
	}
	c->getReturnVar()->setInt(ret);
#endif
}

/// <summary>
/// ブラウザへのCookieの設定
/// </summary>
/// <param name="c"></param>
/// <param name="userdata"></param>
void scSetCookie(CScriptVar* c, void* userdata)
{
	CTinyJS* js = static_cast<CTinyJS*>(userdata);
	wString str;
	time_t timer;
	headerCheckPrint(js->socket, &(js->printed), js->headerBuf, 0);
	wString name = c->getParameter("name")->getString();
	wString value = c->getParameter("value")->getString();
	time_t  expire = c->getParameter("expire")->getInt() + time(&timer);
	str.sprintf("Set-Cookie: %s=%s; expires=%s", name.c_str(), value.c_str(), ctime(&expire));
	int res = js->headerBuf->header(str.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
#endif  // end socket不使用
void scFileCopy(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString pathf = c->getParameter("pathf")->getString();
	wString patht = c->getParameter("patht")->getString();
	int res = wString::FileCopy(pathf, patht);
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
void scMp3Id3Tag(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	mp3* mp3instance = new mp3();
	wString path = c->getParameter("path")->getString();
#ifdef __linux__
#else
	path = path.nkfcnv("Ws");
#endif
	wString res = mp3instance->mp3_id3_tag(path);
	delete mp3instance;
	c->getReturnVar()->setString(res);
}
#if 0  // socket/lutino不使用のためコメントアウト
void scShutDown(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString pw = c->getParameter("password")->getString();
	//global_param.system_passwordを設定しないと、シャットダウンできないようにする
	if (global_param.system_password.length() > 0 && pw == global_param.system_password) {
		//ループ抜ける
		loop_flag = 0;
#ifndef __linux__
		PostMessage(g_hMainWnd, WM_COMMAND, IDM_EXIT, 0);
#endif
	}
}
extern int ssdp_client(wString& str, int loops);
void scSSDP(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString str;
	int ret = ssdp_client(str, 2);
	if (ret == 0) {
		c->getReturnVar()->setString(str);
	}
}
//ファイル内容取得
void scRestful(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString method = c->getParameter("method")->getString();
	wString url = c->getParameter("url")->getString();
	wString send = c->getParameter("send")->getString();
	wString data;
	data = wString::http_rest(method, url, send);
	c->getReturnVar()->setString(data);
}
#endif  // end socket/lutino不使用
char randhex()
{
	auto value = rand() % 16;
	if (value < 10) {
		return value + '0';
	}
	else {
		return value + 'a' - 10;
	}
}

/// <summary>
/// UUID 取得
/// </summary>
/// <param name="c"></param>
/// <param name="userdata"></param>
void scRandomUUID(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString data;
	char no1 = '8';
	char no2 = '9';

	data = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
	for (auto ptr = 0U; ptr < data.length(); ptr++) {
		if (data[ptr] == 'x') {
			data[ptr] = randhex();
		}
		else if (data[ptr] == 'y')
		{
			data[ptr] = no2;
		}
	}
	c->getReturnVar()->setString(data);
}

/// <summary>
/// BIOS UUID
/// </summary>
/// <param name="c"></param>
/// <param name="userdata"></param>
void scBiosUUID(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString data;
	data = data.bios_uuid();
	c->getReturnVar()->setString(data);
}
// --- Array Methods ---
void scArrayPush(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();
	CScriptVar* val = c->getParameter("val");
	arr->setArrayIndex(len, val);
	c->getReturnVar()->setInt(len + 1);
}

void scArrayPop(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();
	if (len > 0) {
		CScriptVar* val = arr->getArrayIndex(len - 1);
		c->getReturnVar()->copyValue(val);
		arr->setArrayIndex(len - 1, new CScriptVar()); // undefined
	}
	else {
		c->getReturnVar()->setUndefined();
	}
}

void scArrayShift(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();
	if (len > 0) {
		CScriptVar* val = arr->getArrayIndex(0);
		c->getReturnVar()->copyValue(val);
		// Shift all elements left
		for (int i = 1; i < len; ++i) {
			arr->setArrayIndex(i - 1, arr->getArrayIndex(i));
		}
		arr->setArrayIndex(len - 1, new CScriptVar()); // undefined
	}
	else {
		c->getReturnVar()->setUndefined();
	}
}

void scArrayUnshift(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();
	CScriptVar* val = c->getParameter("val");
	// Shift all elements right
	for (int i = len; i > 0; --i) {
		arr->setArrayIndex(i, arr->getArrayIndex(i - 1));
	}
	arr->setArrayIndex(0, val);
	c->getReturnVar()->setInt(len + 1);
}

void scArrayIndexOf(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	CScriptVar* search = c->getParameter("val");
	int len = arr->getArrayLength();
	int found = -1;
	for (int i = 0; i < len; ++i) {
		if (arr->getArrayIndex(i)->equals(search)) {
			found = i;
			break;
		}
	}
	c->getReturnVar()->setInt(found);
}

void scArraySlice(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();
	int start = c->getParameter("start")->getInt();
	int end = c->getParameter("end")->isUndefined() ? len : c->getParameter("end")->getInt();
	if (start < 0) start = len + start;
	if (end < 0) end = len + end;
	if (start < 0) start = 0;
	if (end > len) end = len;
	if (end < start) end = start;
	CScriptVar* result = c->getReturnVar();
	result->setArray();
	int idx = 0;
	for (int i = start; i < end; ++i) {
		result->setArrayIndex(idx++, arr->getArrayIndex(i));
	}
}

void scArraySplice(CScriptVar* c, void*) {
	CScriptVar* arr = c->getParameter("this");
	int len = arr->getArrayLength();
	int start = c->getParameter("start")->getInt();
	int deleteCount = c->getParameter("deleteCount")->getInt();
	if (start < 0) start = len + start;
	if (start < 0) start = 0;
	if (start > len) start = len;
	if (deleteCount < 0) deleteCount = 0;
	if (deleteCount > len - start) deleteCount = len - start;
	// Return deleted elements
	CScriptVar* result = c->getReturnVar();
	result->setArray();
	for (int i = 0; i < deleteCount; ++i) {
		result->setArrayIndex(i, arr->getArrayIndex(start + i)->deepCopy());
	}
	// Collect new items to insert
	CScriptVar* args = c->getParameter("arguments");
	int argc = args->getArrayLength();
	int numArgs = argc - 2; // after start, deleteCount
	if (numArgs < 0) numArgs = 0;
	std::vector<CScriptVar*> newItems;
	for (int i = 0; i < numArgs; ++i) {
		wString tmp;
		tmp.sprintf("%d", i + 2);
		newItems.push_back(c->getParameter(tmp.c_str()));
	}
	// Build new array content
	std::vector<CScriptVar*> newArr;
	for (int i = 0; i < start; ++i) {
		newArr.push_back(arr->getArrayIndex(i)->deepCopy());
	}
	for (auto* v : newItems) {
		newArr.push_back(v->deepCopy());
	}
	for (int i = start + deleteCount; i < len; ++i) {
		newArr.push_back(arr->getArrayIndex(i)->deepCopy());
	}
	// Set new array content
	int newLen = (int)newArr.size();
	arr->removeAllChildren();
	for (int i = 0; i < newLen; ++i) {
		arr->setArrayIndex(i, newArr[i]);
	}
}

//死亡
void scDie(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	wString msg = c->getParameter("msg")->getString();
	throw new CScriptException(msg);
}

void scIsNaN(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	double val = c->getParameter("v")->getDouble();
	c->getReturnVar()->setInt(std::isnan(val) ? 1 : 0);
}

void scIsFinite(CScriptVar* c, void* userdata)
{
	IGNORE_PARAMETER(userdata);
	double val = c->getParameter("v")->getDouble();
	c->getReturnVar()->setInt(std::isfinite(val) ? 1 : 0);
}

// ----------------------------------------------- Register Functions
void registerFunctions(CTinyJS* tinyJS)
{
	tinyJS->addNative("function exec(jsCode)", scExec, tinyJS); // execute the given code
	tinyJS->addNative("function eval(jsCode)", scEval, tinyJS); // execute the given wString (an expression) and return the result
	tinyJS->addNative("function trace()", scTrace, tinyJS);
	tinyJS->addNative("function Object.dump()", scObjectDump, tinyJS);
	tinyJS->addNative("function Object.clone()", scObjectClone, 0);
	tinyJS->addNative("function Object.keys(obj)", scKeys, 0);
	tinyJS->addNative("function charToInt(ch)", scCharToInt, 0); //  convert a character to an int - get its value
	tinyJS->addNative("function command(path)", scCommand, 0);
	//tinyJS->addNative("function header(str)", scHeader, tinyJS);       // socket不使用のためコメントアウト
	//tinyJS->addNative("function session_start()", scSessionStart, tinyJS);  // socket不使用
	//tinyJS->addNative("function setCookie(name,value,expire)", scSetCookie, tinyJS);  // socket不使用
	tinyJS->addNative("function Math.rand()", scMathRand, 0);
	tinyJS->addNative("function Math.randInt(min, max)", scMathRandInt, 0);
	tinyJS->addNative("function Integer.parseInt(str)", scIntegerParseInt, 0); // wString to int
	tinyJS->addNative("function Integer.valueOf(str)", scIntegerValueOf, 0); // value of a single character
	tinyJS->addNative("function isNaN(v)", scIsNaN, 0);
	tinyJS->addNative("function isFinite(v)", scIsFinite, 0);
	tinyJS->addNative("function encodeURI(uri)", scEncodeURI, 0);
	tinyJS->addNative("function atob(str)", scAtob, 0);
	tinyJS->addNative("function btoa(str)", scBtoa, 0);
	tinyJS->addNative("function dirname(uri)", scDirname, 0);
	tinyJS->addNative("function basename(uri)", scBasename, 0);
	tinyJS->addNative("function String.indexOf(search)", scStringIndexOf, 0); // find the position of a wString in a string, -1 if not
	tinyJS->addNative("function String.substring(lo,hi)", scStringSubstring, 0);
	tinyJS->addNative("function String.substr(lo,hi)", scStringSubstr, 0);
	//tinyJS->addNative ("function String.startsWith(lo,hi)", scStringStartsWith, 0);
	tinyJS->addNative("function String.startsWith(lo)", scStringStartsWith, 0);
	//tinyJS->addNative ("function String.endsWith(lo,hi)", scStringEndsWith, 0);
	tinyJS->addNative("function String.endsWith(lo)", scStringEndsWith, 0);
	tinyJS->addNative("function String.charAt(pos)", scStringCharAt, 0);
	tinyJS->addNative("function String.charCodeAt(pos)", scStringCharCodeAt, 0);
	tinyJS->addNative("function String.fromCharCode(char)", scStringFromCharCode, 0);
	tinyJS->addNative("function String.split(separator)", scStringSplit, 0);
	tinyJS->addNative("function String.replace(before,after)", scStringReplace, 0);
	tinyJS->addNative("function String.replaceAll(before,after)", scStringReplaceAll, 0);
	tinyJS->addNative("function String.preg_replace(pattern,replace)", scPregStringReplace, 0);
	//tinyJS->addNative("function String.preg_match(pattern)",scPregStringMatch, 0 );
	tinyJS->addNative("function match(pattern,text)", scMatch, 0);
	tinyJS->addNative("function replace(text,pattern,replacement)", scReplace, 0);
	tinyJS->addNative("function String.addSlashes()", scAddShashes, 0);
	//tinyJS->addNative("function getLocalAddress()", scGetLocalAddress, 0);  // socket不使用のためコメントアウト
	//tinyJS->addNative("function getLocalPort()", scGetLocalPort, 0);  // socket不使用
	tinyJS->addNative("function String.toLowerCase()", scToLowerCase, 0);
	tinyJS->addNative("function String.toUpperCase()", scToUpperCase, 0);
	tinyJS->addNative("function String.toDateString(format)", scIntegerToDateString, 0); // time to strng format
	tinyJS->addNative("function Date()", scStringDate, 0); // time to strng
	tinyJS->addNative("function String.nkfconv(format)", scNKFConv, 0); // language code convert
	//tinyJS->addNative("function DBConnect(dbn)", scDBConnect, 0);          // DB不使用のためコメントアウト
	//tinyJS->addNative("function String.DBDisConnect()", scDBDisConnect, 0);
	//tinyJS->addNative("function String.SQL(sqltext)", scDBSQL, 0);
    tinyJS->addNative("function String.sqlBind(params)", scSqlBind, 0); // Safely bind :key placeholders into a SQL string

	tinyJS->addNative("function JSON.mp3id3tag(path)", scMp3Id3Tag, 0);
	tinyJS->addNative("function JSON.stringify(obj, replacer)", scJSONStringify, 0); // convert to JSON. replacer is ignored at the moment

	// JSON.parse is left out as you can (unsafely!) use eval instead
	tinyJS->addNative("function Array.contains(obj)", scArrayContains, 0);
	tinyJS->addNative("function Array.remove(obj)", scArrayRemove, 0);
	tinyJS->addNative("function Array.join(separator)", scArrayJoin, 0);
	//tinyJS->addNative ("function encodeURI(url)", scEncodeURI, 0);
	tinyJS->addNative("function String.trim()", scTrim, 0);
	tinyJS->addNative("function String.rtrim()", scRTrim, 0);
	tinyJS->addNative("function String.ltrim()", scLTrim, 0);
	tinyJS->addNative("function print(text)", js_print, tinyJS);
	tinyJS->addNative("function htmlspecialchars(uri)", scHtmlSpecialChars, 0);
	tinyJS->addNative("function file_exists(path)", scFileExists, 0);
	tinyJS->addNative("function dir_exists(path)", scDirExists, 0);
	tinyJS->addNative("function scandir(path)", scScanDir, 0);
	tinyJS->addNative("function extractFileExt(uri)", scExtractFileExt, 0);
	tinyJS->addNative("function mimeInfo(uri)", scMimeInfo, 0);
	tinyJS->addNative("function file_stat(path)", scFileStats, 0);
	tinyJS->addNative("function filedate(path)", scFileDate, 0);
	tinyJS->addNative("function loadFromFile(path)", scLoadFromFile, 0);
	tinyJS->addNative("function loadFromCSV(path)", scLoadFromCSV, 0);
	tinyJS->addNative("function unlink(path)", scUnlink, 0);
	tinyJS->addNative("function touch(path)", scTouch, 0);
	tinyJS->addNative("function rename(pathf,patht)", scRename, 0);
	tinyJS->addNative("function mkdir(path)", scMkdir, 0);
	tinyJS->addNative("function rmdir(path)", scRmdir, 0);
	tinyJS->addNative("function saveToFile(path,data)", scSaveToFile, 0);
	tinyJS->addNative("function copy(pathf,patht)", scFileCopy, 0);
	//tinyJS->addNative("function shutdown(password)", scShutDown, 0);  // socket不使用のためコメントアウト
	//tinyJS->addNative("function ssdp()", scSSDP, 0);  // socket不使用
	//tinyJS->addNative("function restful(method,url,send)", scRestful, 0);  // socket不使用
	tinyJS->addNative("function randomUUID()", scRandomUUID, 0);
	tinyJS->addNative("function biosUUID()", scBiosUUID, 0);
	tinyJS->addNative("function Array.push(val)", scArrayPush, 0);
	tinyJS->addNative("function Array.pop()", scArrayPop, 0);
	tinyJS->addNative("function Array.shift()", scArrayShift, 0);
	tinyJS->addNative("function Array.unshift(val)", scArrayUnshift, 0);
	tinyJS->addNative("function Array.indexOf(val)", scArrayIndexOf, 0);
	tinyJS->addNative("function Array.slice(start,end)", scArraySlice, 0);
	tinyJS->addNative("function Array.splice(start,deleteCount)", scArraySplice, 0);
	tinyJS->addNative("function die(msg)", scDie, 0);
}
