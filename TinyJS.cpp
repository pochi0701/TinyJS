/*
 * TinyJS
 *
 * A single-file Javascript-alike engine
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

 /* Version 0.1  :  (gw) First published on Google Code
	Version 0.11 :  Making sure the 'root' variable never changes
					'symbol_base' added for the current base of the sybmbol table
	Version 0.12 :  Added findChildOrCreate, changed wString passing to use references
					Fixed broken wString encoding in getJSString()
					Removed getInitCode and added getJSON instead
					Added nil
					Added rough JSON parsing
					Improved example app
	Version 0.13 :  Added tokenEnd/tokenLastEnd to lexer to avoid parsing whitespace
					Ability to define functions without names
					Can now do "var mine = function(a,b) { ... };"
					Slightly better 'trace' function
					Added findChildOrCreateByPath function
					Added simple test suite
					Added skipping of blocks when not executing
	Version 0.14 :  Added parsing of more number types
					Added parsing of wString defined with '
					Changed nil to null as per spec, added 'undefined'
					Now set variables with the correct scope, and treat unknown
							   as 'undefined' rather than failing
					Added proper (I hope) handling of null and undefined
					Added === check
	Version 0.15 :  Fix for possible memory leaks
	Version 0.16 :  Removal of un-needed findRecursive calls
					symbol_base removed and replaced with 'scopes' stack
					Added reference counting a proper tree structure
						(Allowing pass by reference)
					Allowed JSON output to output IDs, not strings
					Added get/set for array indices
					Changed Callbacks to include user data pointer
					Added some support for objects
					Added more Java-esque builtin functions
	Version 0.17 :  Now we don't deepCopy the parent object of the class
					Added JSON.stringify and eval()
					Nicer JSON indenting
					Fixed function output in JSON
					Added evaluateComplex
					Fixed some reentrancy issues with evaluate/execute
	Version 0.18 :  Fixed some issues with code being executed when it shouldn't
	Version 0.19 :  Added array.length
					Changed '__parent' to 'prototype' to bring it more in line with javascript
	Version 0.20 :  Added '%' operator
	Version 0.21 :  Added array type
					String.length() no more - now String.length
					Added extra constructors to reduce confusion
					Fixed checks against undefined
	Version 0.22 :  First part of ardi's changes:
						sprintf -> sprintf_s
						extra tokens parsed
						array memory leak fixed
					Fixed memory leak in evaluateComplex
					Fixed memory leak in FOR loops
					Fixed memory leak for unary minus
	Version 0.23 :  Allowed evaluate[Complex] to take in semi-colon separated
					  statements and then only return the value from the last one.
					  Also checks to make sure *everything* was parsed.
					Ints + doubles are now stored in binary form (faster + more precise)
	Version 0.24 :  More useful error for maths ops
					Don't dump everything on a match error.
	Version 0.25 :  Better wString escaping
	Version 0.26 :  Add CScriptVar::equals
					Add built-in array functions
	Version 0.27 :  Added OZLB's TinyJS.setVariable (with some tweaks)
					Added OZLB's Maths Functions
	Version 0.28 :  Ternary operator
					Rudimentary call stack on error
					Added String Character functions
					Added shift operators
	Version 0.29 :  Added new object via functions
					Fixed getString() for double on some platforms
	Version 0.30 :  Rlyeh Mario's patch for Math Functions on VC++
	Version 0.31 :  Add exec() to TinyJS functions
					Now print quoted JSON that can be read by PHP/Python parsers
					Fixed postfix increment operator
	Version 0.32 :  Fixed Math.randInt on 32 bit PCs, where it was broken
	Version 0.33 :  Fixed Memory leak + brokenness on === comparison

	 NOTE:
		   Constructing an array with an initial length 'Array(5)' doesn't work
		   Recursive loops of data such as a.foo = a; fail to be garbage collected
		   length variable cannot be set
		   The postfix increment operator returns the current value, not the previous as it should.
		   There is no prefix increment operator
		   Arrays are implemented as a linked list - hence a lookup time is O(n)

	 TODO:
		   Utility va-args style function in TinyJS for executing a function directly
		   Merge the parsing of expressions/statements so eval("statement") works like we'd expect.
		   Move 'shift' implementation into mathsOp

  */

#include "TinyJS.h"
#include <assert.h>
#include "define.h"

#ifndef ASSERT
#define ASSERT(X) assert(X)
#endif
  //#define TINYJS_CALL_STACK

 /// <summary>
 /// Frees the given link IF it isn't owned by anything else
 /// </summary>
 /// <param name="x"></param>
inline void CLEAN(CScriptVarLink* x)
{
	auto link = x;
	if (link && !link->owned) {
		delete link;
	}
}
/* Create a LINK to point to VAR and free the old link.
 * BUT this is more clever - it tries to keep the old link if it's not owned to save allocations */
#define CREATE_LINK(LINK, VAR) { if (!LINK || LINK->owned) LINK = new CScriptVarLink(VAR); else LINK->replaceWith(VAR); }
 // 論理オペレータOR
SCRIPTVAR_FLAGS operator|(SCRIPTVAR_FLAGS L, SCRIPTVAR_FLAGS R)
{
	return static_cast<SCRIPTVAR_FLAGS>(static_cast<int>(L) | static_cast<int>(R));
}
// 論理オペレータ&
SCRIPTVAR_FLAGS operator&(SCRIPTVAR_FLAGS L, SCRIPTVAR_FLAGS R)
{
	return static_cast<SCRIPTVAR_FLAGS>(static_cast<int>(L) & static_cast<int>(R));
}
// 論理オペレータ~
SCRIPTVAR_FLAGS operator~(SCRIPTVAR_FLAGS L)
{
	return static_cast<SCRIPTVAR_FLAGS>(~static_cast<int>(L));
}

static unsigned char cmap[256] = {
	//+0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,//00
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//10
	   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//20
	   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,//30
	   0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,//40
	   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4,//50
	   0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,//60
	   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0,//70
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//80
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//90
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//A0
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//B0
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//C0
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//D0
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//E0
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//F0
};

using namespace std;


// ----------------------------------------------------------------------------------- Utils
inline bool isWhitespace(unsigned char ch)
{
	return (cmap[ch] & 1);//(ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r');
}
////////////////////////////////////////////////////////////////////////////////
//数字チェック
inline bool isNumeric(unsigned char  ch)
{
	return (cmap[ch] & 2);//(ch>='0') && (ch<='9');
}
////////////////////////////////////////////////////////////////////////////////
//数値チェック
inline bool isNumber(const wString& str)
{
	for (auto i = 0U; i < str.size(); i++) {
		if (!isNumeric(str[i])) return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
//１６進チェック
inline bool isHexadecimal(unsigned char ch)
{
	return ((ch >= '0') && (ch <= '9')) ||
		((ch >= 'a') && (ch <= 'f')) ||
		((ch >= 'A') && (ch <= 'F'));
}
////////////////////////////////////////////////////////////////////////////////
//アルファベットチェック
inline bool isAlpha(unsigned char ch)
{
	return (cmap[ch] & 4);//((ch>='a') && (ch<='z')) || ((ch>='A') && (ch<='Z')) || ch=='_';
}


////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// エラー時1行出力
/// </summary>
/// <param name="s"></param>
/// <param name="ptr"></param>
/// <param name="end"></param>
/// <returns></returns>
wString oneLine(const char* s, int ptr, int end)
{
	wString work;
	if (end < ptr) {
		ptr = end;
	}
	//前の行に入るように
	if (ptr > 0) ptr--;
	if (ptr > 0) ptr--;
	//遡る
	while (ptr > 0 && s[ptr] != '\n') {
		ptr--;
	}
	//ptr++;
	while (s[ptr] && s[ptr] != '\n') {
		work += s[ptr++];
	}
	return work;
}
////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// convert the given wString into a quoted wString suitable for javascript
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
wString getJSString(const wString& str)
{
	wString nStr = str;
	char buffer[5] = { 0 };
	for (auto i = 0U; i < nStr.size(); i++) {
		const char* replaceWith = "";
		bool replace_flg = true;

		switch (nStr[i]) {
		case '\\': replaceWith = "\\\\"; break;
		case '\n': replaceWith = "\\n"; break;
		case '\r': replaceWith = "\\r"; break;
		case '\a': replaceWith = "\\a"; break;
		case '"':  replaceWith = "\\\""; break;
		default:
			int nCh = ((int)nStr[i]) & 0xFF;
			if (nCh < 32) {
				// \x00の４文字なので５文字目を０にする必要あり
				// 2023/04/17
				snprintf(buffer, 5, "\\x%02X", nCh);
				replaceWith = buffer;
			}
			else {
				replaceWith = const_cast<char*>("");
				replace_flg = false;
			}
			break;
		}

		if (replace_flg) {
			nStr = nStr.substr(0, i) + replaceWith + nStr.substr(i + 1);
			i += static_cast<int>(strlen(replaceWith)) - 1;
		}
	}
	return "\"" + nStr + "\"";
}

/// <summary>
/// Is the wString alphanumeric
/// 英字+[英字|数値]
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
bool isAlphaNum(const wString& str)
{
	if (str.size() == 0) {
		return true;
	}
	if (!isAlpha(str[0])) {
		return false;
	}
	for (auto i = 0U; i < str.size(); i++) {
		if (!(isAlpha(str[i]) || isNumeric(str[i]))) {
			return false;
		}
	}
	return true;
}

#ifdef web
void JSTRACE(SOCKET socket, const char* format, ...)
{
	char work[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(work, format, ap);
	va_end(ap);
	send(socket, work, static_cast<int>(strlen(work)), 0);
}
#endif
// ----------------------------------------------------------------------------------- CSCRIPTEXCEPTION
// 例外はtextに格納
CScriptException::CScriptException(const wString& exceptionText)
{
	text = exceptionText;
}

////////////////////////////////////////////////////////////////////////////////////// CSCRIPTLEX
/// <summary>
/// スクリプト語彙クラス
/// </summary>
/// <param name="input"></param>
CScriptLex::CScriptLex(const wString& input) {
	data = _strdup(input.c_str());//寿命の点からコピーする。deleteで消す
	dataOwned = true;
	dataStart = 0;
	dataEnd = static_cast<int>(strlen(data));
	reset();
}

/// <summary>
/// スクリプト語彙クラス
/// </summary>
/// <param name="owner"></param>
/// <param name="startChar"></param>
/// <param name="endChar"></param>
CScriptLex::CScriptLex(CScriptLex* owner, int startChar, int endChar) {
	data = owner->data;
	dataOwned = false;
	dataStart = startChar;
	dataEnd = endChar;
	reset();
}

/// <summary>Destructor</summary>
CScriptLex::~CScriptLex(void)
{
	if (dataOwned) {
		free(static_cast<void*>(data));
	}
}

void CScriptLex::reset()
{
	dataPos = dataStart;
	tokenStart = 0;
	tokenEnd = 0;
	tokenLastEnd = 0;
	tk = LEX_TYPES::LEX_EOF;
	tkStr = "";
	getNextCh();//currch設定 nextchは不定
	getNextCh();//currch,nextch設定
	getNextToken();//１ワード取り込んだ状態で開始
}
//期待する語をチェックして次の１トークンを先読み
//期待はずれなら例外
void CScriptLex::match(LEX_TYPES expected_tk)
{
	if (tk != expected_tk) {
		wString errorString;
		errorString.sprintf("Got %s expected %s at %s(%s)\n", getTokenStr(tk).c_str(), getTokenStr(expected_tk).c_str(), getPosition(tokenStart).c_str(), oneLine(data, dataPos, dataEnd).c_str());
		throw new CScriptException(errorString.c_str());
	}
	getNextToken();
}
#ifdef web
//グローバルで申し訳ないが最初に文字を出力する際にheaderを先に出す
void headerCheckPrint(int socket, int* printed, wString* headerBuf, int flag)
{
	if (headerBuf->length() == 0) {
		headerBuf->headerInit(0, 0);
	}
	if (flag && *printed == 0) {
		*printed = 1;
		headerBuf->headerPrint(socket, 1);
	}
}
#endif

/// <summary>
/// conver token to string.
/// </summary>
/// <param name="token">LEX_TYPES token</param>
/// <returns>wString</returns>
wString CScriptLex::getTokenStr(LEX_TYPES token)
{
	if (static_cast<int>(token) > 32 && static_cast<int>(token) < 256) {
		char buf[4] = "' '";
		buf[1] = (char)token;
		return wString(buf);
	}
	switch (token) {
	case LEX_TYPES::LEX_EOF: return "EOF";
	case LEX_TYPES::LEX_ID: return "ID";
	case LEX_TYPES::LEX_INT: return "INT";
	case LEX_TYPES::LEX_FLOAT: return "FLOAT";
	case LEX_TYPES::LEX_STR: return "STRING";
	case LEX_TYPES::LEX_EQUAL: return "==";
	case LEX_TYPES::LEX_TYPEEQUAL: return "===";
	case LEX_TYPES::LEX_NEQUAL: return "!=";
	case LEX_TYPES::LEX_NTYPEEQUAL: return "!==";
	case LEX_TYPES::LEX_LEQUAL: return "<=";
	case LEX_TYPES::LEX_LSHIFT: return "<<";
	case LEX_TYPES::LEX_LSHIFTEQUAL: return "<<=";
	case LEX_TYPES::LEX_GEQUAL: return ">=";
	case LEX_TYPES::LEX_RSHIFT: return ">>";
	case LEX_TYPES::LEX_RSHIFTUNSIGNED: return ">>>";
	case LEX_TYPES::LEX_RSHIFTEQUAL: return ">>=";
	case LEX_TYPES::LEX_PLUSEQUAL: return "+=";
	case LEX_TYPES::LEX_MINUSEQUAL: return "-=";
	case LEX_TYPES::LEX_PLUSPLUS: return "++";
	case LEX_TYPES::LEX_MINUSMINUS: return "--";
	case LEX_TYPES::LEX_ANDEQUAL: return "&=";
	case LEX_TYPES::LEX_ANDAND: return "&&";
	case LEX_TYPES::LEX_OREQUAL: return "|=";
	case LEX_TYPES::LEX_OROR: return "||";
	case LEX_TYPES::LEX_XOREQUAL: return "^=";
		// reserved words
	case LEX_TYPES::LEX_R_IF: return "if";
	case LEX_TYPES::LEX_R_ELSE: return "else";
	case LEX_TYPES::LEX_R_DO: return "do";
	case LEX_TYPES::LEX_R_WHILE: return "while";
	case LEX_TYPES::LEX_R_FOR: return "for";
	case LEX_TYPES::LEX_R_BREAK: return "break";
	case LEX_TYPES::LEX_R_CONTINUE: return "continue";
	case LEX_TYPES::LEX_R_FUNCTION: return "function";
	case LEX_TYPES::LEX_R_RETURN: return "return";
	case LEX_TYPES::LEX_R_VAR: return "var";
	case LEX_TYPES::LEX_R_LET: return "let";
	case LEX_TYPES::LEX_R_TRUE: return "true";
	case LEX_TYPES::LEX_R_FALSE: return "false";
	case LEX_TYPES::LEX_R_NULL: return "null";
	case LEX_TYPES::LEX_R_UNDEFINED: return "undefined";
	case LEX_TYPES::LEX_R_NEW: return "new";
	default:
		wString msg;
		msg.sprintf("?[%d]", static_cast<int>(token));
		return msg;
	}

}

/// <summary>
/// 次の１文字を取り込む。
/// </summary>
/// <returns>取り込み前の1文字</returns>
LEX_TYPES CScriptLex::getNextCh()
{
	currCh = nextCh;
	if (dataPos < dataEnd) {
#if 0
		//これは便利かもしれない
		if (serverExecute == ExecuteModes::ON_SERVER) {
			send(socket, " ", 1, 0);
			send(socket, data + dataPos, 1, 0);
		}
		else {
			send(socket, ".", 1, 0);
			send(socket, data + dataPos, 1, 0);
		}
#endif
		nextCh = static_cast<LEX_TYPES>(data[dataPos]);
	}
	else {
		nextCh = LEX_TYPES::LEX_EOF;
	}
	dataPos++;
	return currCh;
}
/// <summary>１トークン取得</summary>
void CScriptLex::getNextToken() {
	// octal digits
	char buf[4] = "???";
	tk = LEX_TYPES::LEX_EOF;
	tkStr.clear();
	//無駄文字読み飛ばし
	while (currCh != LEX_TYPES::LEX_EOF && isWhitespace(static_cast<unsigned char>(currCh))) {
		getNextCh();
	}
	// newline comments
	if (currCh == LEX_TYPES::LEX_DIV && nextCh == LEX_TYPES::LEX_DIV) {
		while (currCh != LEX_TYPES::LEX_EOF && currCh != LEX_TYPES::LEX_CR) {
			getNextCh();
		}
		getNextCh();
		getNextToken();
		return;
	}
	// block comments
	if (currCh == LEX_TYPES::LEX_DIV && nextCh == LEX_TYPES::LEX_MUL) {
		while (currCh != LEX_TYPES::LEX_EOF && (currCh != LEX_TYPES::LEX_MUL || nextCh != LEX_TYPES::LEX_DIV)) {
			getNextCh();
		}
		getNextCh();
		getNextCh();
		getNextToken();
		return;
	}
	// record beginning of this token(pre-read 2 chars );
	tokenStart = dataPos - 2;
	// tokens
	if (isAlpha(static_cast<unsigned char>(currCh))) { //  IDs
		while (isAlpha(static_cast<unsigned char>(currCh)) || isNumeric(static_cast<unsigned char>(currCh))) {
			tkStr += static_cast<char>(currCh);
			getNextCh();
		}
		tk = LEX_TYPES::LEX_ID;
		if (tkStr == "if")        tk = LEX_TYPES::LEX_R_IF;
		else if (tkStr == "else")      tk = LEX_TYPES::LEX_R_ELSE;
		else if (tkStr == "do")        tk = LEX_TYPES::LEX_R_DO;
		else if (tkStr == "while")     tk = LEX_TYPES::LEX_R_WHILE;
		else if (tkStr == "for")       tk = LEX_TYPES::LEX_R_FOR;
		else if (tkStr == "break")     tk = LEX_TYPES::LEX_R_BREAK;
		else if (tkStr == "continue")  tk = LEX_TYPES::LEX_R_CONTINUE;
		else if (tkStr == "function")  tk = LEX_TYPES::LEX_R_FUNCTION;
		else if (tkStr == "return")    tk = LEX_TYPES::LEX_R_RETURN;
		else if (tkStr == "var")       tk = LEX_TYPES::LEX_R_VAR;
		else if (tkStr == "let")       tk = LEX_TYPES::LEX_R_LET;
		else if (tkStr == "true")      tk = LEX_TYPES::LEX_R_TRUE;
		else if (tkStr == "false")     tk = LEX_TYPES::LEX_R_FALSE;
		else if (tkStr == "null")      tk = LEX_TYPES::LEX_R_NULL;
		else if (tkStr == "undefined") tk = LEX_TYPES::LEX_R_UNDEFINED;
		else if (tkStr == "new")       tk = LEX_TYPES::LEX_R_NEW;
	}
	else if (isNumeric(static_cast<unsigned char>(currCh))) { // Numbers
		bool isHex = false;
		if (static_cast<unsigned char>(currCh) == '0') {
			tkStr += static_cast<unsigned char>(currCh); getNextCh();
		}
		if (static_cast<unsigned char>(currCh) == 'x') {
			isHex = true;
			tkStr += static_cast<unsigned char>(currCh); getNextCh();
		}
		tk = LEX_TYPES::LEX_INT;
		while (isNumeric(static_cast<unsigned char>(currCh)) || (isHex && isHexadecimal(static_cast<unsigned char>(currCh)))) {
			tkStr += static_cast<unsigned char>(currCh);
			getNextCh();
		}
		if (!isHex && currCh == LEX_TYPES::LEX_DOT) {
			tk = LEX_TYPES::LEX_FLOAT;
			tkStr += static_cast<unsigned char>(LEX_TYPES::LEX_DOT);
			getNextCh();
			while (isNumeric(static_cast<unsigned char>(currCh))) {
				tkStr += static_cast<unsigned char>(currCh);
				getNextCh();
			}
		}
		// do fancy e-style floating point
		if (!isHex && (static_cast<unsigned char>(currCh) == 'e' || static_cast<unsigned char>(currCh) == 'E')) {
			tk = LEX_TYPES::LEX_FLOAT;
			tkStr += static_cast<unsigned char>(currCh);
			getNextCh();
			if (currCh == LEX_TYPES::LEX_MINUS) {
				tkStr += static_cast<unsigned char>(currCh);
				getNextCh();
			}
			while (isNumeric(static_cast<unsigned char>(currCh))) {
				tkStr += static_cast<unsigned char>(currCh);
				getNextCh();
			}
		}
	}
	else if (currCh == LEX_TYPES::LEX_D_QUOTE) {
		// strings...
		getNextCh();
		while (currCh != LEX_TYPES::LEX_EOF && currCh != LEX_TYPES::LEX_D_QUOTE) {
			if (currCh == LEX_TYPES::LEX_ESC) {
				getNextCh();
				switch (currCh) {
				case static_cast<LEX_TYPES>('n'): tkStr += '\n'; break;
				case static_cast<LEX_TYPES>('a'): tkStr += '\a'; break;
				case static_cast<LEX_TYPES>('r'): tkStr += '\r'; break;
				case static_cast<LEX_TYPES>('t'): tkStr += '\t'; break;
				case LEX_TYPES::LEX_D_QUOTE: tkStr += '"'; break;
				case LEX_TYPES::LEX_ESC: tkStr += '\\'; break;
				default: tkStr += static_cast<unsigned char>(currCh);
				}
			}
			else {
				tkStr += static_cast<unsigned char>(currCh);
			}
			getNextCh();
		}
		getNextCh();
		tk = LEX_TYPES::LEX_STR;
	}
	else if (currCh == LEX_TYPES::LEX_S_QUOTE) {
		// strings again...
		getNextCh();
		while (currCh != LEX_TYPES::LEX_EOF && currCh != LEX_TYPES::LEX_S_QUOTE) {
			if (currCh == LEX_TYPES::LEX_ESC) {
				getNextCh();
				switch (currCh) {
				case static_cast<LEX_TYPES>('n'): tkStr += '\n'; break;
				case static_cast<LEX_TYPES>('a'): tkStr += '\a'; break;
				case static_cast<LEX_TYPES>('r'): tkStr += '\r'; break;
				case static_cast<LEX_TYPES>('t'): tkStr += '\t'; break;
				case LEX_TYPES::LEX_S_QUOTE: tkStr += '\''; break;
				case LEX_TYPES::LEX_ESC: tkStr += '\\'; break;
				case static_cast<LEX_TYPES>('x'): { // hex digits
					getNextCh(); buf[0] = static_cast<char>(currCh);
					getNextCh(); buf[1] = static_cast<char>(currCh);
					tkStr += (char)strtol(buf, 0, 16);
				} break;
				default:
					if (static_cast<unsigned char>(currCh) >= '0' && static_cast<unsigned char>(currCh) <= '7') {
						// octal digits
						buf[0] = static_cast<char>(currCh);
						getNextCh(); buf[1] = static_cast<char>(currCh);
						getNextCh(); buf[2] = static_cast<char>(currCh);
						tkStr += (char)strtol(buf, 0, 8);
					}
					else {
						tkStr += static_cast<char>(currCh);
					}
					break;
				}
			}
			else {
				tkStr += static_cast<char>(currCh);
			}
			getNextCh();
		}
		getNextCh();
		tk = LEX_TYPES::LEX_STR;
	}
	else {
		// single chars
		tk = currCh;
		if (currCh != LEX_TYPES::LEX_EOF) getNextCh();
		if (tk == LEX_TYPES::LEX_EQ && currCh == LEX_TYPES::LEX_EQ) { // ==
			tk = LEX_TYPES::LEX_EQUAL;
			getNextCh();
			if (currCh == LEX_TYPES::LEX_EQ) { // ===
				tk = LEX_TYPES::LEX_TYPEEQUAL;
				getNextCh();
			}
		}
		else if (tk == LEX_TYPES::LEX_EXCLAMATION && currCh == LEX_TYPES::LEX_EQ) { // !=
			tk = LEX_TYPES::LEX_NEQUAL;
			getNextCh();
			if (currCh == LEX_TYPES::LEX_EQ) { // !==
				tk = LEX_TYPES::LEX_NTYPEEQUAL;
				getNextCh();
			}
		}
		else if (tk == LEX_TYPES::LEX_L_THAN && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_LEQUAL;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_L_THAN && currCh == LEX_TYPES::LEX_L_THAN) {
			tk = LEX_TYPES::LEX_LSHIFT;
			getNextCh();
			if (currCh == LEX_TYPES::LEX_EQ) { // <<=
				tk = LEX_TYPES::LEX_LSHIFTEQUAL;
				getNextCh();
			}
		}
		else if (tk == LEX_TYPES::LEX_G_THAN && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_GEQUAL;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_G_THAN && currCh == LEX_TYPES::LEX_G_THAN) {
			tk = LEX_TYPES::LEX_RSHIFT;
			getNextCh();
			if (currCh == LEX_TYPES::LEX_EQ) { // >>=
				tk = LEX_TYPES::LEX_RSHIFTEQUAL;
				getNextCh();
			}
			else if (currCh == LEX_TYPES::LEX_G_THAN) { // >>>
				tk = LEX_TYPES::LEX_RSHIFTUNSIGNED;
				getNextCh();
			}
		}
		else if (tk == LEX_TYPES::LEX_PLUS && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_PLUSEQUAL;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_MINUS && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_MINUSEQUAL;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_PLUS && currCh == LEX_TYPES::LEX_PLUS) {
			tk = LEX_TYPES::LEX_PLUSPLUS;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_MINUS && currCh == LEX_TYPES::LEX_MINUS) {
			tk = LEX_TYPES::LEX_MINUSMINUS;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_AND && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_ANDEQUAL;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_AND && currCh == LEX_TYPES::LEX_AND) {
			tk = LEX_TYPES::LEX_ANDAND;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_OR && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_OREQUAL;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_OR && currCh == LEX_TYPES::LEX_OR) {
			tk = LEX_TYPES::LEX_OROR;
			getNextCh();
		}
		else if (tk == LEX_TYPES::LEX_XOR && currCh == LEX_TYPES::LEX_EQ) {
			tk = LEX_TYPES::LEX_XOREQUAL;
			getNextCh();
		}
	}
	/* This isn't quite right yet */
	tokenLastEnd = tokenEnd;
	tokenEnd = dataPos - 3;
}
/// <summary>部分文字列を返す</summary>
wString CScriptLex::getSubString(int lastPosition)
{
	int lastCharIdx = tokenLastEnd + 1;
	if (lastCharIdx < dataEnd) {
		/* save a memory alloc by using our data array to create the
		   subistring */
		char old = data[lastCharIdx];
		data[lastCharIdx] = 0;
		wString value = &data[lastPosition];
		data[lastCharIdx] = old;
		return value;
	}
	else {
		return wString(&data[lastPosition]);
	}
}

/// <summary>
/// 部分語彙を返す
/// </summary>
/// <param name="lastPosition"></param>
/// <returns></returns>
CScriptLex* CScriptLex::getSubLex(int lastPosition)
{
	int lastCharIdx = tokenLastEnd + 1;
	if (lastCharIdx < dataEnd) {
		return new CScriptLex(this, lastPosition, lastCharIdx);
	}
	else {
		return new CScriptLex(this, lastPosition, dataEnd);
	}
}
/// <summary>指定位置を行数、列数に変換</summary>
wString CScriptLex::getPosition(int pos)
{
	if (pos < 0) pos = tokenLastEnd;
	int line = 1;
	int col = 1;
	for (int i = 0; i < pos; i++) {
		char ch;
		if (i < dataEnd) {
			ch = data[i];
		}
		else {
			ch = 0;
		}
		col++;
		if (ch == '\n') {
			line++;
			col = 1;
		}
	}
	wString buf;
	buf.sprintf("(line: %d, col: %d)", line, col);
	return buf;
}

////////////////////////////////////////////////////////////////////////////////////// CSCRIPTVARLINK
/// <summary>
/// 変数への値の設定
/// </summary>
/// <param name="var"></param>
/// <param name="myname"></param>
CScriptVarLink::CScriptVarLink(CScriptVar* var, const wString& myname)
{
	this->name = myname;
	this->nextSibling = 0;
	this->prevSibling = 0;
	this->var = var->setRef();//thisの参照を増やして返す
	this->owned = false;
}

/// <summary>
/// 変数のコピー（浅い参照）
/// </summary>
/// <param name="link"></param>
CScriptVarLink::CScriptVarLink(const CScriptVarLink& link)
{
	// Copy constructor
	this->name = link.name;
	this->nextSibling = 0;
	this->prevSibling = 0;
	this->var = link.var->setRef();
	this->owned = false;
}

/// <summary>デストラクタ</summary>
CScriptVarLink::~CScriptVarLink()
{
	var->unref();
}

void CScriptVarLink::replaceWith(CScriptVar* newVar)
{
	CScriptVar* oldVar = var;
	var = newVar->setRef();
	oldVar->unref();
}

void CScriptVarLink::replaceWith(CScriptVarLink* newVar)
{
	if (newVar)
		replaceWith(newVar->var);
	else
		replaceWith(new CScriptVar());
}
/// <summary>名前を数値に変換</summary>
int CScriptVarLink::getIntName()
{
	return atoi(name.c_str());
}
/// <summary>変数に整数を設定</summary>
void CScriptVarLink::setIntName(int n)
{
	char sIdx[64];
	snprintf(sIdx, sizeof(sIdx), "%d", n);
	name = sIdx;
}

// ----------------------------------------------------------------------------------- CSCRIPTVAR

CScriptVar::CScriptVar()
{
	refs = 0;
	init();
	flags = SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED;
}

CScriptVar::CScriptVar(const wString& str)
{
	refs = 0;
	init();
	flags = SCRIPTVAR_FLAGS::SCRIPTVAR_STRING;
	data = str;
}


CScriptVar::CScriptVar(const wString& varData, SCRIPTVAR_FLAGS varFlags)
{
	refs = 0;
	init();
	flags = varFlags;
	if ((varFlags & SCRIPTVAR_FLAGS::SCRIPTVAR_INTEGER) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED) {
		intData = strtol(varData.c_str(), 0, 0);
	}
	else if ((varFlags & SCRIPTVAR_FLAGS::SCRIPTVAR_DOUBLE) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED) {
		doubleData = strtod(varData.c_str(), 0);
	}
	else {
		data = varData;
	}
}

CScriptVar::CScriptVar(double val)
{
	refs = 0;
	init();
	setDouble(val);
}

CScriptVar::CScriptVar(int val)
{
	refs = 0;
	init();
	setInt(val);
}
CScriptVar::CScriptVar(bool val)
{
	refs = 0;
	init();
	setInt(val);
}

CScriptVar::~CScriptVar(void)
{
	removeAllChildren();
}

void CScriptVar::init()
{
	firstChild = 0;
	lastChild = 0;
	flags = SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED;
	jsCallback = 0;
	jsCallbackUserData = 0;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
}

CScriptVar* CScriptVar::getReturnVar()
{
	return getParameter(TINYJS_RETURN_VAR);
}

void CScriptVar::setReturnVar(CScriptVar* var)
{
	findChildOrCreate(TINYJS_RETURN_VAR)->replaceWith(var);
}


CScriptVar* CScriptVar::getParameter(const wString& name)
{
	return findChildOrCreate(name)->var;
}
//親変数で子供が見つかったらlinkを返す。なければ0
CScriptVarLink* CScriptVar::findChild(const wString& childName)
{
	CScriptVarLink* v = firstChild;
	while (v) {
		if (v->name.compare(childName) == 0)
			return v;
		v = v->nextSibling;
	}
	return 0;
}

CScriptVarLink* CScriptVar::findChildOrCreate(const wString& childName, SCRIPTVAR_FLAGS varFlags)
{
	CScriptVarLink* lex = findChild(childName);
	if (lex) return lex;

	return addChild(childName, new CScriptVar(TINYJS_BLANK_DATA, varFlags));
}

CScriptVarLink* CScriptVar::findChildOrCreateByPath(const wString& path)
{
	int p = path.find(static_cast<unsigned char>(LEX_TYPES::LEX_DOT));
	if (p == wString::npos)
		return findChildOrCreate(path);

	return findChildOrCreate(path.substr(0, p), SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT)->var->
		findChildOrCreateByPath(path.substr(p + 1));
}

CScriptVarLink* CScriptVar::addChild(const wString& childName, CScriptVar* child)
{
	if (isUndefined()) {
		flags = SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT;
	}
	// if no child supplied, create one
	if (!child) {
		child = new CScriptVar();
	}

	CScriptVarLink* link = new CScriptVarLink(child, childName);
	link->owned = true;
	if (lastChild) {
		lastChild->nextSibling = link;
		link->prevSibling = lastChild;
		lastChild = link;
	}
	else {
		firstChild = link;
		lastChild = link;
	}
	return link;
}

CScriptVarLink* CScriptVar::addChildNoDup(const wString& childName, CScriptVar* child)
{
	// if no child supplied, create one
	if (!child)
		child = new CScriptVar();

	CScriptVarLink* v = findChild(childName);
	if (v) {
		v->replaceWith(child);
	}
	else {
		v = addChild(childName, child);
	}

	return v;
}

void CScriptVar::removeChild(const CScriptVar* child)
{
	CScriptVarLink* link = firstChild;
	while (link) {
		if (link->var == child) {
			break;
		}
		link = link->nextSibling;
	}
	ASSERT(link);
	removeLink(link);
}

void CScriptVar::removeLink(CScriptVarLink* link)
{
	if (!link) return;
	if (link->nextSibling) {
		link->nextSibling->prevSibling = link->prevSibling;
	}
	if (link->prevSibling) {
		link->prevSibling->nextSibling = link->nextSibling;
	}
	if (lastChild == link) {
		lastChild = link->prevSibling;
	}
	if (firstChild == link) {
		firstChild = link->nextSibling;
	}
	delete link;
}

void CScriptVar::removeAllChildren()
{
	CScriptVarLink* c = firstChild;
	while (c) {
		CScriptVarLink* t = c->nextSibling;
		delete c;
		c = t;
	}
	firstChild = 0;
	lastChild = 0;
}

CScriptVar* CScriptVar::getArrayIndex(int idx)
{
	char sIdx[64] = {};
	snprintf(sIdx, sizeof(sIdx), "%d", idx);
	CScriptVarLink* link = findChild(sIdx);
	if (link) return link->var;
	else return new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_NULL); // undefined
}

void CScriptVar::setArrayIndex(int idx, CScriptVar* value)
{
	char sIdx[64] = {};
	snprintf(sIdx, sizeof(sIdx), "%d", idx);
	CScriptVarLink* link = findChild(sIdx);

	if (link) {
		if (value->isUndefined())
			removeLink(link);
		else
			link->replaceWith(value);
	}
	else {
		if (!value->isUndefined())
			addChild(sIdx, value);
	}
}

int CScriptVar::getArrayLength()
{
	int highest = -1;
	if (!isArray()) return 0;

	CScriptVarLink* link = firstChild;
	while (link) {
		if (isNumber(link->name)) {
			int val = atoi(link->name.c_str());
			if (val > highest) highest = val;
		}
		link = link->nextSibling;
	}
	return highest + 1;
}

int CScriptVar::getChildren()
{
	int n = 0;
	CScriptVarLink* link = firstChild;
	while (link) {
		n++;
		link = link->nextSibling;
	}
	return n;
}

int CScriptVar::getInt()
{
	/* strtol understands about hex and octal */
	if (isInt()) return intData;
	if (isNull()) return 0;
	if (isUndefined()) return 0;
	if (isDouble()) return (int)doubleData;
	return 0;
}

double CScriptVar::getDouble()
{
	if (isDouble()) return doubleData;
	if (isInt()) return intData;
	if (isNull()) return 0;
	if (isUndefined()) return 0;
	return 0; /* or NaN? */
}

const wString& CScriptVar::getString()
{
	/* Because we can't return a wString that is generated on demand.
	 * I should really just use char* :) */
	if (isInt()) {
		data.sprintf("%ld", intData);
		return data;
	}
	if (isDouble()) {
		data.sprintf("%f", doubleData);
		return data;
	}
	static const wString s_null = "null";
	if (isNull()) return s_null;
	static const wString s_undefined = "undefined";
	if (isUndefined()) return s_undefined;
	// are we just a wString here?
	return data;
}

void CScriptVar::setInt(int val)
{
	flags = flags & ~SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK | SCRIPTVAR_FLAGS::SCRIPTVAR_INTEGER;
	intData = val;
	doubleData = 0;
	data = TINYJS_BLANK_DATA;
}

void CScriptVar::setDouble(double val)
{
	flags = flags & ~SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK | SCRIPTVAR_FLAGS::SCRIPTVAR_DOUBLE;
	doubleData = val;
	intData = 0;
	data = TINYJS_BLANK_DATA;
}

void CScriptVar::setString(const wString& str)
{
	// name sure it's not still a number or integer
	flags = flags & ~SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK | SCRIPTVAR_FLAGS::SCRIPTVAR_STRING;
	data = str;
	intData = 0;
	doubleData = 0;
}

/// <summary>undefinedの値を設定</summary>
void CScriptVar::setUndefined()
{
	// name sure it's not still a number or integer
	flags = flags & ~SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK | SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
	removeAllChildren();
}

/// <summary>配列を設定</summary>
void CScriptVar::setArray()
{
	// name sure it's not still a number or integer
	flags = (flags & ~SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK) | SCRIPTVAR_FLAGS::SCRIPTVAR_ARRAY;
	data = TINYJS_BLANK_DATA;
	intData = 0;
	doubleData = 0;
	removeAllChildren();
}

/// <summary>
/// thisとvが同一ならtrue
/// </summary>
/// <param name="v">比較する変数</param>
/// <returns>同一なら真、さもなくば偽</returns>:
bool CScriptVar::equals(CScriptVar* v)
{
	CScriptVar* resV = mathsOp(v, LEX_TYPES::LEX_EQUAL);
	bool res = resV->getBool();
	delete resV;
	return res;
}

/// <summary>
/// this,op,bでの演算
/// </summary>
/// <param name="b">変数の値</param>
/// <param name="op">演算子</param>
/// <returns>this,op,bの演算結果true/false</returns>
CScriptVar* CScriptVar::mathsOp(CScriptVar* b, LEX_TYPES op)
{
	CScriptVar* a = this;
	// Type equality check
	if (op == LEX_TYPES::LEX_TYPEEQUAL || op == LEX_TYPES::LEX_NTYPEEQUAL) {
		// check type first, then call again to check data
		bool eql = ((a->flags & SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK) ==
			(b->flags & SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK));
		if (eql) {
			CScriptVar* contents = a->mathsOp(b, LEX_TYPES::LEX_EQUAL);
			if (!contents->getBool()) eql = false;
			if (!contents->refs) delete contents;
		}

		if (op == LEX_TYPES::LEX_TYPEEQUAL) {
			return new CScriptVar(eql);
		}
		else {
			return new CScriptVar(!eql);
		}
	}
	// do maths...
	if (a->isUndefined() && b->isUndefined()) {
		if (op == LEX_TYPES::LEX_EQUAL)       return new CScriptVar(true);
		else if (op == LEX_TYPES::LEX_NEQUAL) return new CScriptVar(false);
		else return new CScriptVar(); // undefined
	}
	else if ((a->isNumeric() || a->isUndefined()) &&
		(b->isNumeric() || b->isUndefined())) {
		if (!a->isDouble() && !b->isDouble()) {
			// use ints
			int da = a->getInt();
			int db = b->getInt();
			switch (op) {
			case LEX_TYPES::LEX_PLUS:  return new CScriptVar(da + db);
			case LEX_TYPES::LEX_MINUS: return new CScriptVar(da - db);
			case LEX_TYPES::LEX_MUL:   return new CScriptVar(da * db);
			case LEX_TYPES::LEX_DIV:   return new CScriptVar((db != 0) ? (da / db) : 0);
			case LEX_TYPES::LEX_AND:   return new CScriptVar(da & db);
			case LEX_TYPES::LEX_OR:    return new CScriptVar(da | db);
			case LEX_TYPES::LEX_XOR:   return new CScriptVar(da ^ db);
			case LEX_TYPES::LEX_MOD:   return new CScriptVar(da % db);
			case LEX_TYPES::LEX_EQUAL: return new CScriptVar(da == db);
			case LEX_TYPES::LEX_NEQUAL:return new CScriptVar(da != db);
			case LEX_TYPES::LEX_L_THAN:   return new CScriptVar(da < db);
			case LEX_TYPES::LEX_LEQUAL:return new CScriptVar(da <= db);
			case LEX_TYPES::LEX_G_THAN:   return new CScriptVar(da > db);
			case LEX_TYPES::LEX_GEQUAL:return new CScriptVar(da >= db);
			default: throw new CScriptException("Operation " + CScriptLex::getTokenStr(op) + " not supported on the Int datatype");
			}
		}
		else {
			// use doubles
			double da = a->getDouble();
			double db = b->getDouble();
			switch (op) {
			case LEX_TYPES::LEX_PLUS:      return new CScriptVar(da + db);
			case LEX_TYPES::LEX_MINUS:     return new CScriptVar(da - db);
			case LEX_TYPES::LEX_MUL:       return new CScriptVar(da * db);
			case LEX_TYPES::LEX_DIV:       return new CScriptVar((db != 0) ? (da / db) : 0);
			case LEX_TYPES::LEX_EQUAL:     return new CScriptVar(da == db);
			case LEX_TYPES::LEX_NEQUAL:    return new CScriptVar(da != db);
			case LEX_TYPES::LEX_L_THAN:    return new CScriptVar(da < db);
			case LEX_TYPES::LEX_LEQUAL:    return new CScriptVar(da <= db);
			case LEX_TYPES::LEX_G_THAN:    return new CScriptVar(da > db);
			case LEX_TYPES::LEX_GEQUAL:    return new CScriptVar(da >= db);
			default: throw new CScriptException("Operation " + CScriptLex::getTokenStr(op) + " not supported on the Double datatype");
			}
		}
	}
	else if (a->isArray()) {
		/* Just check pointers */
		switch (op) {
		case LEX_TYPES::LEX_EQUAL: return new CScriptVar(a == b);
		case LEX_TYPES::LEX_NEQUAL: return new CScriptVar(a != b);
		default: throw new CScriptException("Operation " + CScriptLex::getTokenStr(op) + " not supported on the Array datatype");
		}
	}
	else if (a->isObject()) {
		/* Just check pointers */
		switch (op) {
		case LEX_TYPES::LEX_EQUAL: return new CScriptVar(a == b);
		case LEX_TYPES::LEX_NEQUAL: return new CScriptVar(a != b);
		default: throw new CScriptException("Operation " + CScriptLex::getTokenStr(op) + " not supported on the Object datatype");
		}
	}
	else {
		wString da = a->getString();
		wString db = b->getString();
		// use strings
		switch (op) {
		case LEX_TYPES::LEX_PLUS:      return new CScriptVar(da + db, SCRIPTVAR_FLAGS::SCRIPTVAR_STRING);
		case LEX_TYPES::LEX_EQUAL:     return new CScriptVar(da == db);
		case LEX_TYPES::LEX_NEQUAL:    return new CScriptVar(da != db);
		case LEX_TYPES::LEX_L_THAN:       return new CScriptVar(da < db);
		case LEX_TYPES::LEX_LEQUAL:    return new CScriptVar(da <= db);
		case LEX_TYPES::LEX_G_THAN:       return new CScriptVar(da > db);
		case LEX_TYPES::LEX_GEQUAL:    return new CScriptVar(da >= db);
		default: throw new CScriptException("Operation " + CScriptLex::getTokenStr(op) + " not supported on the wString datatype");
		}
	}
	//実行されないコード
	//ASSERT(0);
	//return 0;
}

/// <summary>
/// copy value of val to this.
/// </summary>
/// <param name="val">varable copy from.</param>
void CScriptVar::copySimpleData(const CScriptVar* val)
{
	data = val->data;
	intData = val->intData;
	doubleData = val->doubleData;
	flags = (flags & ~SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK) | (val->flags & SCRIPTVAR_FLAGS::SCRIPTVAR_VARTYPEMASK);
}

void CScriptVar::copyValue(CScriptVar* val)
{
	if (val) {
		copySimpleData(val);
		// remove all current children
		removeAllChildren();
		// copy children of 'val'
		CScriptVarLink* child = val->firstChild;
		while (child) {
			CScriptVar* copied;
			// don't copy the 'parent' object...
			if (child->name != TINYJS_PROTOTYPE_CLASS) {
				copied = child->var->deepCopy();
			}
			else {
				copied = child->var;
			}

			addChild(child->name, copied);

			child = child->nextSibling;
		}
	}
	else {
		setUndefined();
	}
}

/// <summary>
/// Deep copy
/// </summary>
/// <returns>new variable.</returns>
CScriptVar* CScriptVar::deepCopy()
{
	CScriptVar* newVar = new CScriptVar();
	newVar->copySimpleData(this);
	// copy children
	CScriptVarLink* child = firstChild;
	while (child) {
		CScriptVar* copied;
		// don't copy the 'parent' object...
		if (child->name != TINYJS_PROTOTYPE_CLASS)
			copied = child->var->deepCopy();
		else
			copied = child->var;

		newVar->addChild(child->name, copied);
		child = child->nextSibling;
	}
	return newVar;
}

void CScriptVar::trace(const wString& indentStr, const wString& name)
{
	TRACE("%s'%s' = '%s' %s\n",
		indentStr.c_str(),
		name.c_str(),
		getString().c_str(),
		getFlagsAsString().c_str());
	wString indent = indentStr + " ";
	CScriptVarLink* link = firstChild;
	while (link) {
		link->var->trace(indent, link->name);
		link = link->nextSibling;
	}
}
wString CScriptVar::trace2(void)
{
	wString str;
	CScriptVarLink* link = firstChild;
	while (link) {
		str.Add(link->name);
		link = link->nextSibling;
	}
	return str;
}

wString CScriptVar::getFlagsAsString()
{
	wString flagstr = "";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_FUNCTION) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED) flagstr += "FUNCTION ";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED)   flagstr += "OBJECT ";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_ARRAY) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED)    flagstr += "ARRAY ";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_NATIVE) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED)   flagstr += "NATIVE ";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_DOUBLE) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED)   flagstr += "DOUBLE ";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_INTEGER) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED)  flagstr += "INTEGER ";
	if ((flags & SCRIPTVAR_FLAGS::SCRIPTVAR_STRING) != SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED)   flagstr += "STRING ";
	return flagstr;
}

/// <summary>
/// 解析可能な文字列の出力
/// </summary>
/// <returns></returns>
wString CScriptVar::getParsableString()
{
	// Numbers can just be put in directly
	if (isNumeric()) {
		return getString();
	}
	if (isFunction()) {
		wString funcStr;
		funcStr += "function (";
		// get list of parameters
		CScriptVarLink* link = firstChild;
		while (link) {
			funcStr += link->name;
			if (link->nextSibling) funcStr += ",";
			link = link->nextSibling;
		}
		// add function body
		funcStr += ") ";
		funcStr += getString();
		return funcStr.c_str();
	}
	// if it is a wString then we quote it
	if (isString()) {
		return getJSString(getString());
	}
	if (isNull()) {
		return "null";
	}
	return "undefined";
}

/// <summary>
/// JSON出力
/// </summary>
/// <param name="destination">出力するJSON</param>
/// <param name="linePrefix">行毎に付与する文字</param>
void CScriptVar::getJSON(wString& destination, const wString& linePrefix)
{
	if (isObject()) {
		wString indentedLinePrefix = linePrefix + "  ";
		// children - handle with bracketed list
		destination += "{ \n";
		CScriptVarLink* link = firstChild;
		while (link) {
			destination += indentedLinePrefix;
			destination += getJSString(link->name).c_str();
			destination += " : ";
			link->var->getJSON(destination, indentedLinePrefix);
			link = link->nextSibling;
			if (link) {
				destination += ",\n";
			}
		}
		destination += "\n";
		destination += linePrefix;
		destination += "}";
	}
	else if (isArray()) {
		wString indentedLinePrefix = linePrefix + "  ";
		destination += "[\n";
		int len = getArrayLength();
		if (len > 10000) len = 10000; // we don't want to get stuck here!

		for (int i = 0; i < len; i++) {
			getArrayIndex(i)->getJSON(destination, indentedLinePrefix);
			if (i < len - 1) destination += ",\n";
		}

		destination += "\n";
		destination += linePrefix;
		destination += "]";
	}
	else {
		// no children or a function... just write value directly
		destination += getParsableString();
	}
}


void CScriptVar::setCallback(JSCallback callback, void* userdata)
{
	jsCallback = callback;
	jsCallbackUserData = userdata;
}

/// <summary>
/// 参照設定
/// </summary>
/// <returns></returns>
CScriptVar* CScriptVar::setRef()
{
	refs++;
	return this;
}

void CScriptVar::unref()
{
	if (refs <= 0) printf("OMFG, we have unreffed too far!\n");
	if ((--refs) == 0) {
		delete this;
	}
}

int CScriptVar::getRefs()
{
	return refs;
}


// ----------------------------------------------------------------------------------- CSCRIPT
/// <summary>
/// CSCRIPT CONSTRUCTOR
/// </summary>
CTinyJS::CTinyJS()
{
	lex = 0;
	root = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT))->setRef();
	// Add built-in classes
	stringClass = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT))->setRef();
	arrayClass = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT))->setRef();
	objectClass = (new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT))->setRef();
	root->addChild("String", stringClass);
	root->addChild("Array", arrayClass);
	root->addChild("Object", objectClass);
}

/// <summary>
/// DESTRUCTOR
/// </summary>
CTinyJS::~CTinyJS()
{
	ASSERT(!lex);
	scopes.clear();
	stringClass->unref();
	arrayClass->unref();
	objectClass->unref();
	root->unref();

}

void CTinyJS::trace()
{
	root->trace();
}

/// <summary>
/// コードの実行
/// </summary>
/// <param name="code">実行するステートメント</param>
void CTinyJS::execute(const wString& code)
{
	//退避する
	CScriptLex* oldLex = lex;
	std::vector<CScriptVar*> oldScopes = scopes;

	lex = new CScriptLex(code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif
	scopes.clear();
	scopes.push_back(root);
	try {
		bool execute_flg = true;
		while (lex->tk != LEX_TYPES::LEX_EOF) {
			LEX_TYPES ret = statement(execute_flg);
			if (ret != LEX_TYPES::LEX_EOF) {
				wString errorString;
				errorString.sprintf("Syntax error at %s: %s", lex->getPosition(lex->tokenStart).c_str(), lex->getTokenStr(ret).c_str());
				throw new CScriptException(errorString.c_str());
			}
		}
	}
	catch (CScriptException* e) {
		wString msg;
		msg.sprintf("Error at %s: %s", lex->getPosition().c_str(), e->text.c_str());;
		//msg +=  e->text;
#ifdef TINYJS_CALL_STACK
		for (auto i = (int)call_stack.size() - 1; i >= 0; i--) {
			msg.cat_sprintf("\n%d: %s", i, call_stack.at(i).c_str());
		}
#endif
		delete lex;
		lex = oldLex;
		//いらないかも
		delete e;

		throw new CScriptException(msg.c_str());
	}
	delete lex;
	//復帰する
	lex = oldLex;
	scopes = oldScopes;
}
//複合式
CScriptVarLink CTinyJS::evaluateComplex(const wString& code)
{
	CScriptLex* oldLex = lex;
	std::vector<CScriptVar*> oldScopes = scopes;

	lex = new CScriptLex(code);
#ifdef TINYJS_CALL_STACK
	call_stack.clear();
#endif
	scopes.clear();
	scopes.push_back(root);
	CScriptVarLink* v = 0;
	try {
		bool execute_flg = true;
		do {
			CLEAN(v);
			v = base(execute_flg);
			if (lex->tk != LEX_TYPES::LEX_EOF) lex->match(LEX_TYPES::LEX_SEMICOLON);
		} while (lex->tk != LEX_TYPES::LEX_EOF);
	}
	catch (CScriptException* e) {
		wString msg;
		msg.sprintf("Error at %s: %s", lex->getPosition().c_str(), e->text.c_str());
#ifdef TINYJS_CALL_STACK
		for (int i = (int)call_stack.size() - 1; i >= 0; i--) {
			msg.cat_sprintf("\n%d: %s", i, call_stack.at(i).c_str());
		}
#endif
		delete lex;
		lex = oldLex;

		throw new CScriptException(msg.c_str());
	}
	delete lex;
	lex = oldLex;
	scopes = oldScopes;

	if (v) {
		CScriptVarLink r = *v;
		CLEAN(v);
		return r;
	}
	// return undefined...
	return CScriptVarLink(new CScriptVar());
}

/// <summary>
/// 式の評価
/// </summary>
/// <param name="code">評価する文字列</param>
/// <returns>評価結果</returns>
wString CTinyJS::evaluate(const wString& code)
{
	return evaluateComplex(code).var->getString();
}

void CTinyJS::parseFunctionArguments(CScriptVar* funcVar)
{
	lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
	while (lex->tk != LEX_TYPES::LEX_R_PARENTHESIS) {
		funcVar->addChildNoDup(lex->tkStr);
		lex->match(LEX_TYPES::LEX_ID);
		if (lex->tk != LEX_TYPES::LEX_R_PARENTHESIS) lex->match(LEX_TYPES::LEX_COMMA);
	}
	lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
}

/// <summary>
/// Cで実装されたコードの実行
/// </summary>
/// <param name="funcDesc"></param>
/// <param name="ptr"></param>
/// <param name="userdata"></param>
void CTinyJS::addNative(const wString& funcDesc, JSCallback ptr, void* userdata)
{
	CScriptLex* oldLex = lex;
	lex = new CScriptLex(funcDesc);

	CScriptVar* base_variable = root;

	lex->match(LEX_TYPES::LEX_R_FUNCTION);
	wString funcName = lex->tkStr;
	lex->match(LEX_TYPES::LEX_ID);
	/* Check for dots, we might want to do something like function String.subwString ... */
	while (lex->tk == LEX_TYPES::LEX_DOT) {
		lex->match(LEX_TYPES::LEX_DOT);
		CScriptVarLink* link = base_variable->findChild(funcName);
		// if it doesn't exist, make an object class
		if (!link) link = base_variable->addChild(funcName, new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT));
		base_variable = link->var;
		funcName = lex->tkStr;
		lex->match(LEX_TYPES::LEX_ID);
	}

	CScriptVar* funcVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_FUNCTION | SCRIPTVAR_FLAGS::SCRIPTVAR_NATIVE);
	funcVar->setCallback(ptr, userdata);
	parseFunctionArguments(funcVar);
	delete lex;
	lex = oldLex;

	base_variable->addChild(funcName, funcVar);
}

CScriptVarLink* CTinyJS::parseFunctionDefinition()
{
	// actually parse a function...
	lex->match(LEX_TYPES::LEX_R_FUNCTION);
	wString funcName = TINYJS_TEMP_NAME;
	/* we can have functions without names */
	if (lex->tk == LEX_TYPES::LEX_ID) {
		funcName = lex->tkStr;
		lex->match(LEX_TYPES::LEX_ID);
	}
	CScriptVarLink* funcVar = new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_FUNCTION), funcName);
	parseFunctionArguments(funcVar->var);
	int funcBegin = lex->tokenStart;
	bool noexecute = false;
	block(noexecute);
	funcVar->var->data = lex->getSubString(funcBegin);
	return funcVar;
}

/// <summary>
/// Handle a function call (assumes we've parsed the function name and we're
/// on the start bracket). 'parent' is the object that contains this method,
/// if there was one(otherwise it's just a normnal function).
/// </summary>
/// <param name="execute"></param>
/// <param name="function"></param>
/// <param name="parent"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::functionCall(bool& execute, CScriptVarLink* function, CScriptVar* parent)
{
	if (execute) {
		if (!function->var->isFunction()) {
			wString errorMsg = "Expecting '" + function->name + "' to be a function";
			throw new CScriptException(errorMsg.c_str());
		}
		lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
		// create a new symbol table entry for execution of this function
		CScriptVar* functionRoot = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_FUNCTION);
		if (parent)
			functionRoot->addChildNoDup("this", parent);
		// grab in all parameters
		CScriptVarLink* v = function->var->firstChild;
		while (v) {
			CScriptVarLink* value = base(execute);
			if (execute) {
				if (value->var->isBasic()) {
					// pass by value
					functionRoot->addChild(v->name, value->var->deepCopy());
				}
				else {
					// pass by reference
					functionRoot->addChild(v->name, value->var);
				}
			}
			CLEAN(value);
			if (lex->tk != LEX_TYPES::LEX_R_PARENTHESIS) lex->match(LEX_TYPES::LEX_COMMA);
			v = v->nextSibling;
		}
		lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
		// setup a return variable
		CScriptVarLink* returnVar = NULL;
		// execute function!
		// add the function's execute space to the symbol table so we can recurse
		CScriptVarLink* returnVarLink = functionRoot->addChild(TINYJS_RETURN_VAR);
		scopes.push_back(functionRoot);
#ifdef TINYJS_CALL_STACK
		call_stack.push_back(function->name + " from " + lex->getPosition());
#endif

		if (function->var->isNative()) {
			ASSERT(function->var->jsCallback);
			function->var->jsCallback(functionRoot, function->var->jsCallbackUserData);
		}
		else {
			/* we just want to execute the block, but something could
			 * have messed up and left us with the wrong ScriptLex, so
			 * we want to be careful here... */
			CScriptException* exception = 0;
			CScriptLex* oldLex = lex;
			CScriptLex* newLex = new CScriptLex(function->var->getString());
			lex = newLex;
			try {
				block(execute);
				// because return will probably have called this, and set execute to false
				execute = true;
			}
			catch (CScriptException* e) {
				exception = e;
			}
			delete newLex;
			lex = oldLex;

			if (exception) {
				throw exception;
			}
		}
#ifdef TINYJS_CALL_STACK
		if (!call_stack.empty()) call_stack.pop_back();
#endif
		scopes.pop_back();
		/* get the real return var before we remove it from our function */
		//resource leak
		returnVar = new CScriptVarLink(returnVarLink->var);
		functionRoot->removeLink(returnVarLink);
		delete functionRoot;
		if (returnVar) {
			return returnVar;
		}
		else {
			return new CScriptVarLink(new CScriptVar());
		}
	}
	else {
		// function, but not executing - just parse args and be done
		lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
		while (lex->tk != LEX_TYPES::LEX_R_PARENTHESIS) {
			CScriptVarLink* value = base(execute);
			CLEAN(value);
			if (lex->tk != LEX_TYPES::LEX_R_PARENTHESIS) lex->match(LEX_TYPES::LEX_COMMA);
		}
		lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
		if (lex->tk == LEX_TYPES::LEX_L_BRACE) { // TODO: why is this here?
			block(execute);
		}
		/* function will be a blank scriptvarlink if we're not executing,
		 * so just return it rather than an alloc/free */
		return function;
	}
}

CScriptVarLink* CTinyJS::factor(bool& execute)
{
	if (lex->tk == LEX_TYPES::LEX_L_PARENTHESIS) {
		lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
		CScriptVarLink* a = base(execute);
		lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
		return a;
	}
	if (lex->tk == LEX_TYPES::LEX_R_TRUE) {
		lex->match(LEX_TYPES::LEX_R_TRUE);
		return new CScriptVarLink(new CScriptVar(1));
	}
	if (lex->tk == LEX_TYPES::LEX_R_FALSE) {
		lex->match(LEX_TYPES::LEX_R_FALSE);
		return new CScriptVarLink(new CScriptVar(0));
	}
	if (lex->tk == LEX_TYPES::LEX_R_NULL) {
		lex->match(LEX_TYPES::LEX_R_NULL);
		return new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_NULL));
	}
	if (lex->tk == LEX_TYPES::LEX_R_UNDEFINED) {
		lex->match(LEX_TYPES::LEX_R_UNDEFINED);
		return new CScriptVarLink(new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_UNDEFINED));
	}
	if (lex->tk == LEX_TYPES::LEX_ID) {
		CScriptVarLink* a = execute ? findInScopes(lex->tkStr) : new CScriptVarLink(new CScriptVar());
		//printf("0x%08X for %s at %s\n", (unsigned int)a, l->tkStr.c_str(), l->getPosition().c_str());
		/* The parent if we're executing a method call */
		CScriptVar* parent = 0;

		const void* alone = NULL;
		if (execute && !a) {
			/* Variable doesn't exist! JavaScript says we should create it
			 * (we won't add it here. This is done in the assignment operator)*/
			a = new CScriptVarLink(new CScriptVar(), lex->tkStr);
			alone = static_cast<void*>(a);
		}
		lex->match(LEX_TYPES::LEX_ID);
		while (lex->tk == LEX_TYPES::LEX_L_PARENTHESIS || lex->tk == LEX_TYPES::LEX_DOT || lex->tk == LEX_TYPES::LEX_L_BRAKET) {
			if (lex->tk == LEX_TYPES::LEX_L_PARENTHESIS) { // ------------------------------------- Function Call
				a = functionCall(execute, a, parent);
			}
			else if (lex->tk == LEX_TYPES::LEX_DOT) { // ------------------------------------- Record Access
				lex->match(LEX_TYPES::LEX_DOT);
				if (execute) {
					int aa = 0;
					const wString& name = lex->tkStr;
					CScriptVarLink* child = a->var->findChild(name);
					if (!child) child = findInParentClasses(a->var, name);
					if (!child) {
						/* if we haven't found this defined yet, use the built-in
						   'length' properly */
						if (a->var->isArray() && name == "length") {
							int ll = static_cast<int>(a->var->getArrayLength());
							//aa = 1;
							child = new CScriptVarLink(new CScriptVar(ll));
						}
						else if (a->var->isString() && name == "length") {
							int ll = static_cast<int>(a->var->getString().size());

							child = new CScriptVarLink(new CScriptVar(ll));
						}
						else {
							child = a->var->addChild(name);
						}
					}
					parent = a->var;
					//不明な変数にchildを作らない                  
					if (a == alone) {
						wString errorMsg = "Object variable not defined '";
						errorMsg = errorMsg + a->name + "' must be defined";
						throw new CScriptException(errorMsg.c_str());
					}
					//多分aをCLEANしないとメモリーリーク ひでえ実装
					if (aa == 1) {
						CLEAN(a);
					}
					a = child;
				}
				lex->match(LEX_TYPES::LEX_ID);
			}
			else if (lex->tk == LEX_TYPES::LEX_L_BRAKET) { // ------------------------------------- Array Access
				lex->match(LEX_TYPES::LEX_L_BRAKET);
				CScriptVarLink* index = base(execute);
				lex->match(LEX_TYPES::LEX_R_BRAKET);
				if (execute) {
					CScriptVarLink* child = a->var->findChildOrCreate(index->var->getString());
					parent = a->var;
					a = child;
				}
				CLEAN(index);
			}
			else ASSERT(0);
		}
		return a;
	}
	if (lex->tk == LEX_TYPES::LEX_INT || lex->tk == LEX_TYPES::LEX_FLOAT) {
		CScriptVar* a = new CScriptVar(lex->tkStr,
			((lex->tk == LEX_TYPES::LEX_INT) ? SCRIPTVAR_FLAGS::SCRIPTVAR_INTEGER : SCRIPTVAR_FLAGS::SCRIPTVAR_DOUBLE));
		lex->match(lex->tk);
		return new CScriptVarLink(a);
	}
	if (lex->tk == LEX_TYPES::LEX_STR) {
		CScriptVar* a = new CScriptVar(lex->tkStr, SCRIPTVAR_FLAGS::SCRIPTVAR_STRING);
		lex->match(LEX_TYPES::LEX_STR);
		return new CScriptVarLink(a);
	}
	if (lex->tk == LEX_TYPES::LEX_L_BRACE) {
		CScriptVar* contents = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT);
		/* JSON-style object definition */
		lex->match(LEX_TYPES::LEX_L_BRACE);
		while (lex->tk != LEX_TYPES::LEX_R_BRACE) {
			wString id = lex->tkStr;
			// we only allow strings or IDs on the left hand side of an initialisation
			if (lex->tk == LEX_TYPES::LEX_STR) lex->match(LEX_TYPES::LEX_STR);
			else lex->match(LEX_TYPES::LEX_ID);
			lex->match(LEX_TYPES::LEX_COL);
			if (execute) {
				CScriptVarLink* a = base(execute);
				contents->addChild(id, a->var);
				CLEAN(a);
			}
			// no need to clean here, as it will definitely be used
			if (lex->tk != LEX_TYPES::LEX_R_BRACE) lex->match(LEX_TYPES::LEX_COMMA);
		}

		lex->match(LEX_TYPES::LEX_R_BRACE);
		return new CScriptVarLink(contents);
	}
	if (lex->tk == LEX_TYPES::LEX_L_BRAKET) {
		CScriptVar* contents = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_ARRAY);
		/* JSON-style array */
		lex->match(LEX_TYPES::LEX_L_BRAKET);
		int idx = 0;
		while (lex->tk != LEX_TYPES::LEX_R_BRAKET) {
			if (execute) {
				char idx_str[16] = {}; // big enough for 2^32
				snprintf(idx_str, sizeof(idx_str), "%d", idx);

				CScriptVarLink* a = base(execute);
				contents->addChild(idx_str, a->var);
				CLEAN(a);
			}
			// no need to clean here, as it will definitely be used
			if (lex->tk != LEX_TYPES::LEX_R_BRAKET) lex->match(LEX_TYPES::LEX_COMMA);
			idx++;
		}
		lex->match(LEX_TYPES::LEX_R_BRAKET);
		return new CScriptVarLink(contents);
	}
	if (lex->tk == LEX_TYPES::LEX_R_FUNCTION) {
		CScriptVarLink* funcVar = parseFunctionDefinition();
		if (funcVar->name != TINYJS_TEMP_NAME)
			TRACE("Functions not defined at statement-level are not meant to have a name");
		return funcVar;
	}
	if (lex->tk == LEX_TYPES::LEX_R_NEW) {
		// new -> create a new object
		lex->match(LEX_TYPES::LEX_R_NEW);
		const wString& className = lex->tkStr;
		if (execute) {
			CScriptVarLink* objClassOrFunc = findInScopes(className);
			if (!objClassOrFunc) {
				TRACE("%s is not a valid class name", className.c_str());
				return new CScriptVarLink(new CScriptVar());
			}
			lex->match(LEX_TYPES::LEX_ID);
			CScriptVar* obj = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FLAGS::SCRIPTVAR_OBJECT);
			CScriptVarLink* objLink = new CScriptVarLink(obj);
			if (objClassOrFunc->var->isFunction()) {
				CLEAN(functionCall(execute, objClassOrFunc, obj));
			}
			else {
				obj->addChild(TINYJS_PROTOTYPE_CLASS, objClassOrFunc->var);
				if (lex->tk == LEX_TYPES::LEX_L_PARENTHESIS) {
					lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
					lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
				}
			}
			return objLink;
		}
		else {
			lex->match(LEX_TYPES::LEX_ID);
			if (lex->tk == LEX_TYPES::LEX_L_PARENTHESIS) {
				lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
				lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
			}
		}
	}
	// Nothing we can do here... just hope it's the end...
	lex->match(LEX_TYPES::LEX_EOF);
	return 0;
}
/// <summary>
/// 単項演算子"!"
/// </summary>
/// <param name="execute">実行フラグ</param>
/// <returns></returns>
CScriptVarLink* CTinyJS::unary(bool& execute)
{
	CScriptVarLink* a;
	if (lex->tk == LEX_TYPES::LEX_EXCLAMATION) {
		lex->match(LEX_TYPES::LEX_EXCLAMATION); // binary not
		a = factor(execute);
		if (execute) {
			CScriptVar zero(0);
			CScriptVar* res = a->var->mathsOp(&zero, LEX_TYPES::LEX_EQUAL);
			CREATE_LINK(a, res);
		}
	}
	else {
		a = factor(execute);
	}
	return a;
}

/// <summary>
/// 
/// </summary>
/// <param name="execute"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::term(bool& execute)
{
	CScriptVarLink* a = unary(execute);
	while (lex->tk == LEX_TYPES::LEX_MUL || lex->tk == LEX_TYPES::LEX_DIV || lex->tk == LEX_TYPES::LEX_MOD) {
		LEX_TYPES op = lex->tk;
		lex->match(lex->tk);
		CScriptVarLink* b = unary(execute);
		if (execute) {
			CScriptVar* res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}
		CLEAN(b);
	}
	return a;
}
////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 表現：単項-,単項演算子、演算子等(-a++とか)
/// </summary>
/// <param name="execute"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::expression(bool& execute)
{
	bool negate = false;
	if (lex->tk == LEX_TYPES::LEX_MINUS) {
		lex->match(LEX_TYPES::LEX_MINUS);
		negate = true;
	}
	CScriptVarLink* a = term(execute);
	if (negate) {
		CScriptVar zero(0);
		CScriptVar* res = zero.mathsOp(a->var, LEX_TYPES::LEX_MINUS);
		CREATE_LINK(a, res);
	}

	while (lex->tk == LEX_TYPES::LEX_PLUS || lex->tk == LEX_TYPES::LEX_MINUS ||
		lex->tk == LEX_TYPES::LEX_PLUSPLUS || lex->tk == LEX_TYPES::LEX_MINUSMINUS) {
		LEX_TYPES op = lex->tk;
		lex->match(lex->tk);
		if (op == LEX_TYPES::LEX_PLUSPLUS || op == LEX_TYPES::LEX_MINUSMINUS) {
			if (execute) {
				CScriptVar one(1);
				CScriptVar* res = a->var->mathsOp(&one, op == LEX_TYPES::LEX_PLUSPLUS ? LEX_TYPES::LEX_PLUS : LEX_TYPES::LEX_MINUS);
				CScriptVarLink* oldValue = new CScriptVarLink(a->var);
				// in-place add/subtract
				a->replaceWith(res);
				CLEAN(a);
				a = oldValue;
			}
		}
		else {
			CScriptVarLink* b = term(execute);
			if (execute) {
				// not in-place, so just replace
				CScriptVar* res = a->var->mathsOp(b->var, op);
				CREATE_LINK(a, res);
			}
			CLEAN(b);
		}
	}
	return a;
}
////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Shift Operator.
/// </summary>
/// <param name="execute"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::shift(bool& execute)
{
	CScriptVarLink* a = expression(execute);
	if (lex->tk == LEX_TYPES::LEX_LSHIFT || lex->tk == LEX_TYPES::LEX_RSHIFT || lex->tk == LEX_TYPES::LEX_RSHIFTUNSIGNED) {
		LEX_TYPES op = lex->tk;
		lex->match(op);
		CScriptVarLink* b = base(execute);
		int shift_bits = execute ? b->var->getInt() : 0;
		CLEAN(b);
		if (execute) {
			if (op == LEX_TYPES::LEX_LSHIFT) a->var->setInt(a->var->getInt() << shift_bits);
			else if (op == LEX_TYPES::LEX_RSHIFT) a->var->setInt(a->var->getInt() >> shift_bits);
			else if (op == LEX_TYPES::LEX_RSHIFTUNSIGNED) a->var->setInt(((unsigned int)a->var->getInt()) >> shift_bits);
		}
	}
	return a;
}
////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 条件式
/// </summary>
/// <param name="execute">実行の有無</param>
/// <returns></returns>
CScriptVarLink* CTinyJS::condition(bool& execute)
{
	CScriptVarLink* a = shift(execute);
	CScriptVarLink* b = NULL;
	while (lex->tk == LEX_TYPES::LEX_EQUAL || lex->tk == LEX_TYPES::LEX_NEQUAL ||
		lex->tk == LEX_TYPES::LEX_TYPEEQUAL || lex->tk == LEX_TYPES::LEX_NTYPEEQUAL ||
		lex->tk == LEX_TYPES::LEX_LEQUAL || lex->tk == LEX_TYPES::LEX_GEQUAL ||
		lex->tk == LEX_TYPES::LEX_L_THAN || lex->tk == LEX_TYPES::LEX_G_THAN) {
		LEX_TYPES op = lex->tk;
		lex->match(lex->tk);
		b = shift(execute);
		if (execute) {
			CScriptVar* res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}
		CLEAN(b);
	}
	return a;
}
////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 結合条件式
/// </summary>
/// <param name="execute"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::logic(bool& execute)
{
	CScriptVarLink* a = condition(execute);
	while (lex->tk == LEX_TYPES::LEX_AND ||
		lex->tk == LEX_TYPES::LEX_OR ||
		lex->tk == LEX_TYPES::LEX_XOR ||
		lex->tk == LEX_TYPES::LEX_ANDAND ||
		lex->tk == LEX_TYPES::LEX_OROR) {
		bool noexecute = false;
		LEX_TYPES op = lex->tk;
		lex->match(lex->tk);
		bool shortCircuit = false;
		bool boolean = false;
		// if we have short-circuit ops, then if we know the outcome
		// we don't bother to execute the other op. Even if not
		// we need to tell mathsOp it's an & or |
		if (op == LEX_TYPES::LEX_ANDAND) {
			op = LEX_TYPES::LEX_AND;
			shortCircuit = !a->var->getBool();
			boolean = true;
		}
		else if (op == LEX_TYPES::LEX_OROR) {
			op = LEX_TYPES::LEX_OR;
			shortCircuit = a->var->getBool();
			boolean = true;
		}
		auto b = condition(shortCircuit ? noexecute : execute);
		if (execute && !shortCircuit) {
			if (boolean) {
				CScriptVar* newa = new CScriptVar(a->var->getBool());
				CScriptVar* newb = new CScriptVar(b->var->getBool());
				CREATE_LINK(a, newa);
				CREATE_LINK(b, newb);
			}
			CScriptVar* res = a->var->mathsOp(b->var, op);
			CREATE_LINK(a, res);
		}
		CLEAN(b);
	}
	return a;
}
////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 三項演算子
/// </summary>
/// <param name="execute">実行の有無</param>
/// <returns></returns>
CScriptVarLink* CTinyJS::ternary(bool& execute)
{
	CScriptVarLink* lhs = logic(execute);
	if (lex->tk == LEX_TYPES::LEX_QUESTION) {
		bool noexec = false;
		lex->match(LEX_TYPES::LEX_QUESTION);
		if (!execute) {
			CLEAN(lhs);
			CLEAN(base(noexec));
			lex->match(LEX_TYPES::LEX_COL);
			CLEAN(base(noexec));
		}
		else {
			bool first = lhs->var->getBool();
			CLEAN(lhs);
			if (first) {
				lhs = base(execute);
				lex->match(LEX_TYPES::LEX_COL);
				CLEAN(base(noexec));
			}
			else {
				CLEAN(base(noexec));
				lex->match(LEX_TYPES::LEX_COL);
				lhs = base(execute);
			}
		}
	}
	return lhs;
}
////////////////////////////////////////////////////////////////////////////////
//a=1、a+=1、a-=等
CScriptVarLink* CTinyJS::base(bool& execute)
{
	CScriptVarLink* lhs = ternary(execute);
	if (lex->tk == LEX_TYPES::LEX_EQ || lex->tk == LEX_TYPES::LEX_PLUSEQUAL || lex->tk == LEX_TYPES::LEX_MINUSEQUAL) {
		/* If we're assigning to this and we don't have a parent,
		 * add it to the symbol table root as per JavaScript. */
		if (execute && !lhs->owned) {
			if (lhs->name.length() > 0) {
				CScriptVarLink* realLhs = root->addChildNoDup(lhs->name, lhs->var);
				CLEAN(lhs);
				lhs = realLhs;
			}
			else
				TRACE("Trying to assign to an un-named type\n");
		}

		LEX_TYPES op = lex->tk;
		lex->match(lex->tk);
		CScriptVarLink* rhs = base(execute);
		if (execute) {
			if (op == LEX_TYPES::LEX_EQ) {
				lhs->replaceWith(rhs);
			}
			else if (op == LEX_TYPES::LEX_PLUSEQUAL) {
				CScriptVar* res = lhs->var->mathsOp(rhs->var, LEX_TYPES::LEX_PLUS);
				lhs->replaceWith(res);
			}
			else if (op == LEX_TYPES::LEX_MINUSEQUAL) {
				CScriptVar* res = lhs->var->mathsOp(rhs->var, LEX_TYPES::LEX_MINUS);
				lhs->replaceWith(res);
			}
			else ASSERT(0);
		}
		CLEAN(rhs);
	}
	return lhs;
}
//execute==trueならblock内を実施
LEX_TYPES CTinyJS::block(bool& execute)
{
	LEX_TYPES ret = LEX_TYPES::LEX_EOF;
	lex->match(LEX_TYPES::LEX_L_BRACE);
	int brackets = 1;
	if (execute) {
		while (lex->tk != LEX_TYPES::LEX_EOF && lex->tk != LEX_TYPES::LEX_R_BRACE) {
			ret = statement(execute);
			//この場合のみ末尾まで読み飛ばし
			if (ret == LEX_TYPES::LEX_R_BREAK || ret == LEX_TYPES::LEX_R_CONTINUE) {
				while (lex->tk != LEX_TYPES::LEX_EOF && brackets) {
					if (lex->tk == LEX_TYPES::LEX_L_BRACE) brackets++;
					if (lex->tk == LEX_TYPES::LEX_R_BRACE) brackets--;
					lex->match(lex->tk);
				}
				return ret;
			}
		}
		lex->match(LEX_TYPES::LEX_R_BRACE);
	}
	else {
		// fast skip of blocks
		while (lex->tk != LEX_TYPES::LEX_EOF && brackets) {
			if (lex->tk == LEX_TYPES::LEX_L_BRACE) brackets++;
			if (lex->tk == LEX_TYPES::LEX_R_BRACE) brackets--;
			lex->match(lex->tk);
		}
	}
	return ret;
}
//記述
LEX_TYPES  CTinyJS::statement(bool& execute)
{
	LEX_TYPES ret;
	if (lex->tk == LEX_TYPES::LEX_ID ||
		lex->tk == LEX_TYPES::LEX_INT ||
		lex->tk == LEX_TYPES::LEX_FLOAT ||
		lex->tk == LEX_TYPES::LEX_STR ||
		lex->tk == LEX_TYPES::LEX_MINUS) {
		/* Execute a simple statement that only contains basic arithmetic... */
		CLEAN(base(execute));
		lex->match(LEX_TYPES::LEX_SEMICOLON);
	}
	// Execute Block{ ...}
	else if (lex->tk == LEX_TYPES::LEX_L_BRACE) {
		/* A block of code */
		ret = block(execute);
		//単なるreturnでいいのでは？
		//if (ret == LEX_TYPES::LEX_R_BREAK || ret == LEX_TYPES::LEX_R_CONTINUE) {
		return ret;
		//}
	}
	else if (lex->tk == LEX_TYPES::LEX_SEMICOLON) {
		/* Empty statement - to allow things like ;;; */
		lex->match(LEX_TYPES::LEX_SEMICOLON);
	}
	else if (lex->tk == LEX_TYPES::LEX_R_BREAK) {
		lex->match(LEX_TYPES::LEX_R_BREAK);
		lex->match(LEX_TYPES::LEX_SEMICOLON);
		if (execute) {
			return LEX_TYPES::LEX_R_BREAK;
		}
		else {
			return LEX_TYPES::LEX_EOF;
		}
	}
	else if (lex->tk == LEX_TYPES::LEX_R_CONTINUE) {
		lex->match(LEX_TYPES::LEX_R_CONTINUE);
		lex->match(LEX_TYPES::LEX_SEMICOLON);
		if (execute)
		{
			return LEX_TYPES::LEX_R_CONTINUE;
		}
		else {
			return LEX_TYPES::LEX_EOF;
		}
	}
	else if (lex->tk == LEX_TYPES::LEX_R_VAR) {
		/* variable creation. TODO - we need a better way of parsing the left
		 * hand side. Maybe just have a flag called can_create_var that we
		 * set and then we parse as if we're doing a normal equals.*/
		lex->match(LEX_TYPES::LEX_R_VAR);
		while (lex->tk != LEX_TYPES::LEX_SEMICOLON) {
			CScriptVarLink* a = 0;
			if (execute)
				a = scopes.back()->findChildOrCreate(lex->tkStr);
			lex->match(LEX_TYPES::LEX_ID);
			// now do stuff defined with dots
			while (lex->tk == LEX_TYPES::LEX_DOT) {
				lex->match(LEX_TYPES::LEX_DOT);
				if (execute) {
					CScriptVarLink* lastA = a;
					a = lastA->var->findChildOrCreate(lex->tkStr);
				}
				lex->match(LEX_TYPES::LEX_ID);
			}
			// sort out initialiser
			if (lex->tk == LEX_TYPES::LEX_EQ) {
				lex->match(LEX_TYPES::LEX_EQ);
				CScriptVarLink* var = base(execute);
				if (execute) {
					a->replaceWith(var);
				}
				CLEAN(var);
			}
			if (lex->tk != LEX_TYPES::LEX_SEMICOLON)
				lex->match(LEX_TYPES::LEX_COMMA);
		}
		lex->match(LEX_TYPES::LEX_SEMICOLON);
	}
	else if (lex->tk == LEX_TYPES::LEX_R_LET) {
		/* variable creation. TODO - we need a better way of parsing the left
		 * hand side. Maybe just have a flag called can_create_var that we
		 * set and then we parse as if we're doing a normal equals.*/
		lex->match(LEX_TYPES::LEX_R_LET);
		while (lex->tk != LEX_TYPES::LEX_SEMICOLON) {
			CScriptVarLink* a = 0;
			if (execute)
				a = scopes.back()->findChildOrCreate(lex->tkStr);
			lex->match(LEX_TYPES::LEX_ID);
			// now do stuff defined with dots
			while (lex->tk == LEX_TYPES::LEX_DOT) {
				lex->match(LEX_TYPES::LEX_DOT);
				if (execute) {
					CScriptVarLink* lastA = a;
					a = lastA->var->findChildOrCreate(lex->tkStr);
				}
				lex->match(LEX_TYPES::LEX_ID);
			}
			// sort out initialiser
			if (lex->tk == LEX_TYPES::LEX_EQ) {
				lex->match(LEX_TYPES::LEX_EQ);
				CScriptVarLink* var = base(execute);
				if (execute) {
					a->replaceWith(var);
				}
				CLEAN(var);
			}
			if (lex->tk != LEX_TYPES::LEX_SEMICOLON)
				lex->match(LEX_TYPES::LEX_COMMA);
		}
		lex->match(LEX_TYPES::LEX_SEMICOLON);
	}
	else if (lex->tk == LEX_TYPES::LEX_R_IF) {
		//IF
		lex->match(LEX_TYPES::LEX_R_IF);
		lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
		CScriptVarLink* var = base(execute);
		lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
		bool cond = execute && var->var->getBool();
		CLEAN(var);
		bool noexecute = false; // because we need to be able to write to it
		ret = statement(cond ? execute : noexecute);
		if (ret == LEX_TYPES::LEX_R_BREAK || ret == LEX_TYPES::LEX_R_CONTINUE) {
			return ret;
		}
		if (lex->tk == LEX_TYPES::LEX_R_ELSE) {
			lex->match(LEX_TYPES::LEX_R_ELSE);
			//break continue対応. LEX_R_BREAK,LEX_R_CONTINUE以外ではLEX_EOFがかえる
			return statement(cond ? noexecute : execute);
		}
	}
	else if (lex->tk == LEX_TYPES::LEX_R_WHILE) {
		//WHILE
		// We do repetition by pulling out the wString representing our statement
		// there's definitely some opportunity for optimisation here
		lex->match(LEX_TYPES::LEX_R_WHILE);
		lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
		int whileCondStart = lex->tokenStart;
		bool noexecute = false;
		CScriptVarLink* cond = base(execute);
		bool loopCond = execute && cond->var->getBool();
		CLEAN(cond);
		CScriptLex* whileCond = lex->getSubLex(whileCondStart);
		lex->match(LEX_TYPES::LEX_R_PARENTHESIS);
		int whileBodyStart = lex->tokenStart;
		ret = statement(loopCond ? execute : noexecute);
		if (ret != LEX_TYPES::LEX_EOF) {
			wString errorString;
			errorString.sprintf("Syntax error at %s: %s", lex->getPosition(lex->tokenStart).c_str(), lex->getTokenStr(ret).c_str());
			throw new CScriptException(errorString.c_str());
		}
		CScriptLex* whileBody = lex->getSubLex(whileBodyStart);
		CScriptLex* oldLex = lex;
		//int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		//while (loopCond && loopCount-- > 0) {
		while (loopCond) {
			whileCond->reset();
			lex = whileCond;
			cond = base(execute);
			loopCond = execute && cond->var->getBool();
			CLEAN(cond);
			if (loopCond) {
				whileBody->reset();
				lex = whileBody;
				ret = statement(execute);
				if (ret == LEX_TYPES::LEX_R_BREAK) {
					break;
				}
				if (ret == LEX_TYPES::LEX_R_CONTINUE) {
					continue;
				}
			}
		}
		lex = oldLex;
		delete whileCond;
		delete whileBody;
	}
	else if (lex->tk == LEX_TYPES::LEX_R_FOR) {
		// for(statement condition; iterator)
		lex->match(LEX_TYPES::LEX_R_FOR);
		lex->match(LEX_TYPES::LEX_L_PARENTHESIS);
		ret = statement(execute); // initialisation
		if (ret > LEX_TYPES::LEX_EOF) {
			wString errorString;
			errorString.sprintf("Syntax error at %s: %s", lex->getPosition(lex->tokenStart).c_str(), lex->getTokenStr(ret).c_str());
			throw new CScriptException(errorString.c_str());
		}
		//l->match(LEX_TYPES::LEX_SEMICOLON);
		int forCondStart = lex->tokenStart;
		bool noexecute = false;
		CScriptVarLink* cond = base(execute); // condition
		bool loopCond = execute && cond->var->getBool();
		CLEAN(cond);
		CScriptLex* forCond = lex->getSubLex(forCondStart);
		lex->match(LEX_TYPES::LEX_SEMICOLON);
		int forIterStart = lex->tokenStart;
		CLEAN(base(noexecute)); // iterator
		CScriptLex* forIter = lex->getSubLex(forIterStart);
		lex->match(LEX_TYPES::LEX_R_PARENTHESIS);

		// 開始位置
		int forBodyStart = lex->tokenStart;
		// lexの最後を求めるために必要
		ret = statement(loopCond ? execute : noexecute);

		CScriptLex* forBody = lex->getSubLex(forBodyStart);
		CScriptLex* oldLex = lex;
		if (loopCond) {
			forIter->reset();
			lex = forIter;
			CLEAN(base(execute));
		}
		//int loopCount = TINYJS_LOOP_MAX_ITERATIONS;
		//while (execute && loopCond && loopCount-- > 0) {
		if (ret != LEX_TYPES::LEX_R_BREAK) {
			while (execute && loopCond) {
				forCond->reset();
				lex = forCond;
				cond = base(execute);
				loopCond = cond->var->getBool();
				CLEAN(cond);
				if (execute && loopCond) {
					forBody->reset();
					lex = forBody;
					ret = statement(execute);
					if (ret == LEX_TYPES::LEX_R_BREAK) {
						break;
					}
					//if (ret == LEX_TYPES::LEX_R_CONTINUE) {
					//	continue;
					//}
				}
				if (execute && loopCond) {
					forIter->reset();
					lex = forIter;
					CLEAN(base(execute));
				}
			}
		}
		lex = oldLex;
		delete forCond;
		delete forIter;
		delete forBody;
	}
	else if (lex->tk == LEX_TYPES::LEX_R_RETURN) {
		lex->match(LEX_TYPES::LEX_R_RETURN);
		CScriptVarLink* result = 0;
		if (lex->tk != LEX_TYPES::LEX_SEMICOLON)
			result = base(execute);
		if (execute) {
			CScriptVarLink* resultVar = scopes.back()->findChild(TINYJS_RETURN_VAR);
			if (resultVar)
				resultVar->replaceWith(result);
			else
				TRACE("RETURN statement, but not in a function.\n");
			execute = false;
		}
		CLEAN(result);
		lex->match(LEX_TYPES::LEX_SEMICOLON);
	}
	else if (lex->tk == LEX_TYPES::LEX_R_FUNCTION) {
		CScriptVarLink* funcVar = parseFunctionDefinition();
		if (execute) {
			if (funcVar->name == TINYJS_TEMP_NAME)
				TRACE("Functions defined at statement-level are meant to have a name\n");
			else
				scopes.back()->addChildNoDup(funcVar->name, funcVar->var);
		}
		CLEAN(funcVar);
	}
	else {
		lex->match(LEX_TYPES::LEX_EOF);
	}
	return LEX_TYPES::LEX_EOF;
}

/// Get the given variable specified by a path (var1.var2.etc), or return 0
CScriptVar* CTinyJS::getScriptVariable(const wString& path)
{
	// traverse path
	unsigned int prevIdx = 0;
	int    thisIdx = path.find(static_cast<unsigned char>(LEX_TYPES::LEX_DOT));
	if (thisIdx == wString::npos) {
		thisIdx = path.length();
	}
	CScriptVar* var = root;
	while (var && prevIdx < path.length()) {
		//着目している変数の最初の方
		wString el = path.substr(prevIdx, thisIdx - prevIdx);
		CScriptVarLink* varl = var->findChild(el);
		var = varl ? varl->var : 0;
		prevIdx = thisIdx + 1;
		thisIdx = path.find(static_cast<unsigned char>(LEX_TYPES::LEX_DOT), prevIdx);
		if (thisIdx == wString::npos) thisIdx = path.length();
	}
	return var;
}

/// Get the value of the given variable, or return 0
//const wString *CTinyJS::getVariable(const wString &path) {
//    CScriptVar *var = getScriptVariable(path);
//    // return result
//    if (var){
//        return &var->getString();
//    }else{
//        return 0;
//    }
//}

/// <summary>
/// set the value of the given variable, return trur if it exists and gets set
/// </summary>
/// <param name="path"></param>
/// <param name="varData"></param>
/// <returns></returns>
bool CTinyJS::setVariable(const wString& path, const wString& varData)
{
	CScriptVar* var = getScriptVariable(path);
	// return result
	if (var) {
		if (var->isInt())
			var->setInt((int)strtol(varData.c_str(), 0, 0));
		else if (var->isDouble())
			var->setDouble(strtod(varData.c_str(), 0));
		else
			var->setString(varData.c_str());
		return true;
	}
	else {
		return false;
	}
}

/// <summary>
/// Finds a child, looking recursively up the scopes
/// </summary>
/// <param name="childName"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::findInScopes(const wString& childName)
{
	for (auto s = (int)scopes.size() - 1; s >= 0; s--) {
		CScriptVarLink* v = scopes[s]->findChild(childName);
		if (v) return v;
	}
	return NULL;
}
/// <summary>
/// Look up in any parent classes of the given object
/// </summary>
/// <param name="object"></param>
/// <param name="name"></param>
/// <returns></returns>
CScriptVarLink* CTinyJS::findInParentClasses(CScriptVar* object, const wString& name)
{
	// Look for links to actual parent classes
	CScriptVarLink* parentClass = object->findChild(TINYJS_PROTOTYPE_CLASS);
	while (parentClass) {
		CScriptVarLink* implementation = parentClass->var->findChild(name);
		if (implementation) return implementation;
		parentClass = parentClass->var->findChild(TINYJS_PROTOTYPE_CLASS);
	}
	// else fake it for strings and finally objects
	if (object->isString()) {
		CScriptVarLink* implementation = stringClass->findChild(name);
		if (implementation) return implementation;
	}
	if (object->isArray()) {
		CScriptVarLink* implementation = arrayClass->findChild(name);
		if (implementation) return implementation;
	}
	CScriptVarLink* implementation = objectClass->findChild(name);
	if (implementation) return implementation;

	return 0;
}
