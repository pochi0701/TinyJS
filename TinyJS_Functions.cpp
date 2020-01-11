#define _CRT_SECURE_NO_WARNINGS
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
 //#include "dregex.h"
using namespace std;
// ----------------------------------------------- Actual Functions
void js_print(CScriptVar* v, void* userdata) {
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

void scTrace(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(c);
	IGNORE_PARAMETER(userdata);
	CTinyJS* js = (CTinyJS*)userdata;
	js->root->trace();
}

void scObjectDump(CScriptVar* c, void* userdata) {
	c->getParameter("this")->trace("> ");
}

void scObjectClone(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	CScriptVar* obj = c->getParameter("this");
	c->getReturnVar()->copyValue(obj);
}
void scKeys(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString list = c->getParameter("obj")->trace2();
	CScriptVar* result = c->getReturnVar();
	result->setArray();
	int length = 0;
	int count = list.getLines();
	for (int i = 0; i < count; i++) {
		result->setArrayIndex(length++, new CScriptVar(list.GetListString(i)));
	}
}


void scMathRand(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	c->getReturnVar()->setDouble((double)rand() / RAND_MAX);
}

void scMathRandInt(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	int min = c->getParameter("min")->getInt();
	int max = c->getParameter("max")->getInt();
	int val = min + (int)(rand() % (1 + max - min));
	c->getReturnVar()->setInt(val);
}
/////////////////////////////////////////////////////////////////////////

void scPrint(CScriptVar* c, void*) {
	wString val = c->getParameter("val")->getString();
	printf("%s", val.c_str());
	c->getReturnVar()->setString(val);
}
/////////////////////////////////////////////////////////////////////////
void scTrim(CScriptVar* c, void*) {
	wString val = c->getParameter("this")->getString();
	c->getReturnVar()->setString(val.Trim());
}
//
void scRTrim(CScriptVar* c, void*) {
	wString val = c->getParameter("this")->getString();
	c->getReturnVar()->setString(val.RTrim());
}
//
void scLTrim(CScriptVar* c, void*) {
	wString val = c->getParameter("this")->getString();
	c->getReturnVar()->setString(val.LTrim());
}
/////////////////////////////////////////////////////////////////////////
void scCharToInt(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("ch")->getString();
	int val = 0;
	if (str.length() > 0) {
		val = (int)str.c_str()[0];
	}
	c->getReturnVar()->setInt(val);
}

void scStringIndexOf(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString search = c->getParameter("search")->getString();
	int  p = str.find(search);
	int val = (p == wString::npos) ? -1 : p;
	c->getReturnVar()->setInt(val);
}
//Substring
void scStringSubstring(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int lo = c->getParameter("lo")->getInt();
	int hi = c->getParameter("hi")->getInt();

	int l = hi - lo;
	if (l > 0 && lo >= 0 && lo + l <= (int)str.length())
		c->getReturnVar()->setString(str.substr(lo, l));
	else
		c->getReturnVar()->setString("");
}
//SubStr
void scStringSubstr(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int lo = c->getParameter("lo")->getInt();
	int hi = c->getParameter("hi")->getInt();

	if (hi > 0 && lo >= 0 && lo + hi <= (int)str.length())
		c->getReturnVar()->setString(str.substr(lo, hi));
	else
		c->getReturnVar()->setString("");
}
//AT
void scStringCharAt(CScriptVar* c, void* userdata) {
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

void scStringCharCodeAt(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int p = c->getParameter("pos")->getInt();
	if (p >= 0 && p < (int)str.length())
		c->getReturnVar()->setInt(str.at(p));
	else
		c->getReturnVar()->setInt(0);
}

void scStringSplit(CScriptVar* c, void* userdata) {
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
void scStringReplace(CScriptVar* c, void* userdata) {
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
void scPregStringReplace(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString result;
	wString str = c->getParameter("this")->getString();
	CScriptVar* arrp = c->getParameter("pattern");
	vector<wString> patterns;
	vector<wString> replaces;
	int pn = arrp->getArrayLength();
	int rn;
	if (pn) {
		CScriptVar* arrr = c->getParameter("replace");
		rn = arrr->getArrayLength();
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
	//dregex::replace(&result, str, patterns, replaces);
	c->getReturnVar()->setString(result);
}
//AddShashes
void scAddShashes(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	c->getReturnVar()->setString(str.addSlashes());
}
//getLocalAddress
void scGetLocalAddress(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	//c->getReturnVar()->setString(wString::GetLocalAddress());
}
void scStringFromCharCode(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	char str[2] = { 0 };
	str[0] = (char)c->getParameter("char")->getInt();
	c->getReturnVar()->setString(str);
}

void scIntegerParseInt(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("str")->getString();
	int val = strtol(str.c_str(), 0, 0);
	c->getReturnVar()->setInt(val);
}

void scIntegerValueOf(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("str")->getString();

	int val = 0;
	if (str.length() == 1)
		val = str[0];
	c->getReturnVar()->setInt(val);
}
//
void scIntegerToDateString(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	//wString times = c->getParameter("this")->getString();
	//wString format = c->getParameter("format")->getString();
	//char s[128] = {0};
	//long time = atol(times.c_str());
	//struct tm *timeptr;
	//timeptr = localtime(&time);
	//strftime(s, 128, format.c_str(), timeptr);
	//c->getReturnVar()->setString(s);
}
void scStringDate(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	time_t t = time(NULL);
	char s[128];
	sprintf(s, "%ld", t);
	c->getReturnVar()->setString(s);
}
void scNKFConv(CScriptVar* c, void* userdata) {
#ifdef WEB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString format = c->getParameter("format")->getString();
	wString temp = str.nkfcnv(format);
	c->getReturnVar()->setString(temp);
#endif
}

void scDBConnect(CScriptVar* c, void* userdata) {
#ifdef DB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	str = _DBConnect(str);
	if (str.Length()) {
		c->getParameter("this")->setString(str);
		c->getReturnVar()->setString(str);
	}
	else {
		c->getReturnVar()->setString("");
	}
#endif
}

void scDBDisConnect(CScriptVar* c, void* userdata) {
#ifdef DB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	int ret = _DBDisConnect(str);
	c->getReturnVar()->setInt(ret);
#endif
}

void scDBSQL(CScriptVar* c, void* userdata) {
#ifdef DB
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	wString sql = c->getParameter("sqltext")->getString();
	wString ret = _DBSQL(str, sql);
	c->getReturnVar()->setString(ret);
#endif
}


void scJSONStringify(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString result;
	c->getParameter("obj")->getJSON(result);
	c->getReturnVar()->setString(result.c_str());
}

void scExec(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	CTinyJS* tinyJS = (CTinyJS*)userdata;
	wString str = c->getParameter("jsCode")->getString();
	tinyJS->execute(str);
}

void scEval(CScriptVar* c, void* userdata) {

	CTinyJS* tinyJS = (CTinyJS*)userdata;
	c->setReturnVar(tinyJS->evaluateComplex(c->getParameter("jsCode")->getString()).var);
}

void scArrayContains(CScriptVar* c, void* userdata) {
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

void scArrayRemove(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	CScriptVar* obj = c->getParameter("obj");
	vector<int> removedIndices;
	CScriptVarLink* v;
	// remove
	v = c->getParameter("this")->firstChild;
	while (v) {
		if (v->var->equals(obj)) {
			removedIndices.push_back(v->getIntName());
		}
		v = v->nextSibling;
	}
	// renumber
	v = c->getParameter("this")->firstChild;
	while (v) {
		int n = v->getIntName();
		int newn = n;
		for (size_t i = 0; i < removedIndices.size(); i++)
			if (n >= removedIndices[i])
				newn--;
		if (newn != n)
			v->setIntName(newn);
		v = v->nextSibling;
	}
}

void scArrayJoin(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString sep = c->getParameter("separator")->getString();
	CScriptVar* arr = c->getParameter("this");

	wString sstr;
	int l = arr->getArrayLength();
	for (int i = 0; i < l; i++) {
		if (i > 0) {
			sstr += sep;
		}
		sstr += arr->getArrayIndex(i)->getString();
	}

	c->getReturnVar()->setString(sstr.c_str());
}
//ファイル存在チェック
void scFileExists(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int flag = wString::FileExists(path);
	c->getReturnVar()->setInt(flag);
}
//ディレクトリ存在チェック
void scDirExists(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int flag = wString::DirectoryExists(path);
	c->getReturnVar()->setInt(flag);
}
//htmlspecialchars
void scHtmlSpecialChars(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = uri.htmlspecialchars();
	c->getReturnVar()->setString(uri);
}
//encodeURI
void scEncodeURI(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = uri.uri_encode();
	c->getReturnVar()->setString(uri);
}
//dirname
void scDirname(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	while (uri.length() > 0 && uri[uri.length() - 1] != '/') {
		uri = uri.SubString(0, uri.length() - 1);
	}
	if (uri.length() > 0) {
		uri = uri.SubString(0, uri.length() - 1);
	}
	else {
		uri = "/";
	}
	c->getReturnVar()->setString(uri);
}
//base
void scBasename(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	int len = uri.length() - 1;
	while (len >= 0 && uri[len] != '/') {
		len--;
	}
	uri = uri.SubString(len + 1, uri.length() - len - 1);
	c->getReturnVar()->setString(uri);
}

//ScanDir
void scScanDir(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = wString::EnumFolderjson(uri);
	c->getReturnVar()->setString(uri);
}
//ExtractFileExt
void scExtractFileExt(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString uri = c->getParameter("uri")->getString();
	uri = wString::ExtractFileExt(uri);
	c->getReturnVar()->setString(uri);
}
//toLowerCase
void scToLowerCase(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	char* String = str.c_str();
	for (unsigned int i = 0; i < str.length(); i++) {
		//String[i] = (unsigned char)tolower(String[i]);
	}
	c->getReturnVar()->setString(str);
}
//toUpperCase
void scToUpperCase(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString str = c->getParameter("this")->getString();
	char* String = str.c_str();
	for (unsigned int i = 0; i < str.length(); i++) {
		//String[i] = (unsigned char)toupper(String[i]);
	}
	c->getReturnVar()->setString(str);
}
//ファイル属性など
void scFileStats(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString json = wString::FileStats(path);
	c->getReturnVar()->setString(json);
}
//ファイル属性など
void scFileDate(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString json = wString::FileStats(path, 1);
	c->getReturnVar()->setString(json);
}
//ファイル内容取得
void scLoadFromFile(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString data;
	data.LoadFromFile(path);
	c->getReturnVar()->setString(data);
}
//CSV内容取得
void scLoadFromCSV(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString data;
	data.LoadFromCSV(path);
	c->getReturnVar()->setString(data);
}
//ファイル削除
void scUnlink(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int res = _unlink(path.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//ファイル作成
void scTouch(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int ret = true;
	int fd = _open(path.c_str(), O_CREAT | O_APPEND | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE);
	if (fd < 0) {
		ret = false;
	}
	else {
		_close(fd);
	}
	c->getReturnVar()->setInt(ret);
}
//リネーム
void scRename(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString pathf = c->getParameter("pathf")->getString();
	wString patht = c->getParameter("patht")->getString();
	int ret = wString::RenameFile(pathf, patht);
	c->getReturnVar()->setInt(ret);
}
//フォルダ作成
void scMkdir(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int ret = wString::CreateDir(path);
	c->getReturnVar()->setInt(ret);
}
//フォルダ削除
void scRmdir(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	int res = _rmdir(path.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//ファイル保存
void scSaveToFile(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString path = c->getParameter("path")->getString();
	wString data = c->getParameter("data")->getString();
	int res = data.SaveToFile(path);
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//command実行
void scCommand(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString ppath = c->getParameter("path")->getString();
	int res = system(ppath.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
//Header
void scHeader(CScriptVar* c, void* userdata) {
#ifdef WEB
	CTinyJS* js = (CTinyJS*)userdata;
	headerCheckPrint(js->socket, &(js->printed), js->headerBuf, 0);
	wString str = c->getParameter("str")->getString();
	int res = js->headerBuf->header(str.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
#endif
}
//SessionStart
void scSessionStart(CScriptVar* c, void* userdata) {
#ifdef WEB
	const static char material[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	CTinyJS* js = (CTinyJS*)userdata;
	int ret = 0;
	//sidある？
	wString jssessid = js->evaluate("JSSESSID");
	if (jssessid != "undefined") {
		if (session.count(jssessid) > 0) {
			wString data = session[jssessid];
			js->execute("var _SESSION=" + data + ";", 1);
		}
		else {
			js->execute("var _SESSION={};", 1);
		}
	}
	else {
		srand((unsigned)time(NULL));
		char work[27] = { 0 };
		while (1) {
			for (int i = 0; i < 26; i++) {
				work[i] = material[rand() % (sizeof(material) - 1)];
			}
			//同じモノはダメ
			if (session.count(work) == 0) break;
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
//SetCookie
void scSetCookie(CScriptVar* c, void* userdata) {
#ifdef WEB
	CTinyJS* js = (CTinyJS*)userdata;
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
#endif
}
void scFileCopy(CScriptVar* c, void* userdata) {
	IGNORE_PARAMETER(userdata);
	wString pathf = c->getParameter("pathf")->getString();
	wString patht = c->getParameter("patht")->getString();
	int res = wString::FileCopy(pathf.c_str(), patht.c_str());
	int ret = (res == 0) ? true : false;
	c->getReturnVar()->setInt(ret);
}
// ----------------------------------------------- Register Functions
void registerFunctions(CTinyJS* tinyJS) {
	tinyJS->addNative("function exec(jsCode)", scExec, tinyJS); // execute the given code
	tinyJS->addNative("function eval(jsCode)", scEval, tinyJS); // execute the given wString (an expression) and return the result
	tinyJS->addNative("function trace()", scTrace, tinyJS);
	tinyJS->addNative("function Object.dump()", scObjectDump, tinyJS);
	tinyJS->addNative("function Object.clone()", scObjectClone, 0);
	tinyJS->addNative("function Object.keys(obj)", scKeys, 0);
	tinyJS->addNative("function charToInt(ch)", scCharToInt, 0); //  convert a character to an int - get its value
	tinyJS->addNative("function command(path)", scCommand, 0);
	tinyJS->addNative("function header(str)", scHeader, tinyJS);
	tinyJS->addNative("function session_start()", scSessionStart, tinyJS);
	tinyJS->addNative("function setCookie(name,value,expire)", scSetCookie, tinyJS);
	tinyJS->addNative("function Math.rand()", scMathRand, 0);
	tinyJS->addNative("function Math.randInt(min, max)", scMathRandInt, 0);
	tinyJS->addNative("function Integer.parseInt(str)", scIntegerParseInt, 0); // wString to int
	tinyJS->addNative("function Integer.valueOf(str)", scIntegerValueOf, 0); // value of a single character
	tinyJS->addNative("function encodeURI(uri)", scEncodeURI, 0);
	tinyJS->addNative("function dirname(uri)", scDirname, 0);
	tinyJS->addNative("function basename(uri)", scBasename, 0);
	tinyJS->addNative("function String.indexOf(search)", scStringIndexOf, 0); // find the position of a wString in a string, -1 if not
	tinyJS->addNative("function String.substring(lo,hi)", scStringSubstring, 0);
	tinyJS->addNative("function String.substr(lo,hi)", scStringSubstr, 0);
	tinyJS->addNative("function String.charAt(pos)", scStringCharAt, 0);
	tinyJS->addNative("function String.charCodeAt(pos)", scStringCharCodeAt, 0);
	tinyJS->addNative("function String.fromCharCode(char)", scStringFromCharCode, 0);
	tinyJS->addNative("function String.split(separator)", scStringSplit, 0);
	tinyJS->addNative("function String.replace(before,after)", scStringReplace, 0);
	tinyJS->addNative("function String.preg_replace(pattern,replace)", scPregStringReplace, 0);
	//tinyJS->addNative("function String.preg_match(pattern)",scPregStringMatch, 0 );
	tinyJS->addNative("function String.addSlashes()", scAddShashes, 0);
	tinyJS->addNative("function getLocalAddress()", scGetLocalAddress, 0);
	tinyJS->addNative("function String.toLowerCase()", scToLowerCase, 0);
	tinyJS->addNative("function String.toUpperCase()", scToUpperCase, 0);
	tinyJS->addNative("function String.toDateString(format)", scIntegerToDateString, 0); // time to strng format
	tinyJS->addNative("function Date()", scStringDate, 0); // time to strng
	tinyJS->addNative("function String.nkfconv(format)", scNKFConv, 0); // language code convert
	tinyJS->addNative("function String.Connect()", scDBConnect, 0); // Connect to DB
	tinyJS->addNative("function String.DisConnect()", scDBDisConnect, 0); // DisConnect to DB
	tinyJS->addNative("function String.SQL(sqltext)", scDBSQL, 0); // Execute SQL

//    tinyJS->addNative("function JSON.mp3id3tag(path)",                scMp3Id3Tag,           0 );
	tinyJS->addNative("function JSON.stringify(obj, replacer)", scJSONStringify, 0); // convert to JSON. replacer is ignored at the moment

	// JSON.parse is left out as you can (unsafely!) use eval instead
	tinyJS->addNative("function Array.contains(obj)", scArrayContains, 0);
	tinyJS->addNative("function Array.remove(obj)", scArrayRemove, 0);
	tinyJS->addNative("function Array.join(separator)", scArrayJoin, 0);
	tinyJS->addNative("function encodeURI(url)", scEncodeURI, 0);
	tinyJS->addNative("function String.trim()", scTrim, 0);
	tinyJS->addNative("function String.rtrim()", scRTrim, 0);
	tinyJS->addNative("function String.ltrim()", scLTrim, 0);
	tinyJS->addNative("function print(text)", js_print, tinyJS);
	tinyJS->addNative("function htmlspecialchars(uri)", scHtmlSpecialChars, 0);
	tinyJS->addNative("function file_exists(path)", scFileExists, 0);
	tinyJS->addNative("function dir_exists(path)", scDirExists, 0);
	tinyJS->addNative("function scandir(uri)", scScanDir, 0);
	tinyJS->addNative("function extractFileExt(uri)", scExtractFileExt, 0);
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
}
