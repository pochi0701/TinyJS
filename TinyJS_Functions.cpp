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
#include <math.h>
#include <cstdlib>
#include <sstream>
#include <stdio.h>

using namespace std;
// ----------------------------------------------- Actual Functions
void scTrace(CScriptVar *c, void *userdata) {
    CTinyJS *js = (CTinyJS*)userdata;
    js->root->trace();
}

void scObjectDump(CScriptVar *c, void *) {
    c->getParameter("this")->trace("> ");
}

void scObjectClone(CScriptVar *c, void *) {
    CScriptVar *obj = c->getParameter("this");
    c->getReturnVar()->copyValue(obj);
}

void scMathRand(CScriptVar *c, void *) {
    c->getReturnVar()->setDouble((double)rand()/RAND_MAX);
}

void scMathRandInt(CScriptVar *c, void *) {
    int min = c->getParameter("min")->getInt();
    int max = c->getParameter("max")->getInt();
    int val = min + (int)(rand()%(1+max-min));
    c->getReturnVar()->setInt(val);
}
/////////////////////////////////////////////////////////////////////////
void scPrint(CScriptVar *c, void *) {
    wString val = c->getParameter("val")->getString();
    printf("%s",val.c_str());
    c->getReturnVar()->setString(val);
}
//URL Encode
void scEncodeURI(CScriptVar *c, void *) {
    wString url = c->getParameter("url")->getString();
    url = url.uri_encode();
    c->getReturnVar()->setString(url);
}
//URL Encode
void scFileExists(CScriptVar *c, void *) {
    wString file = c->getParameter("file")->getString();
    bool flag  = wString::FileExists(file);
    c->getReturnVar()->setInt(flag);
}
/////////////////////////////////////////////////////////////////////////
void scTrim(CScriptVar *c, void *) {
    wString val = c->getParameter("this")->getString();
    val = val.Trim();
    c->getReturnVar()->setString(val);
}
//
void scRTrim(CScriptVar *c, void *) {
    wString val = c->getParameter("this")->getString();
    val = val.RTrim();
    c->getReturnVar()->setString(val);
}
//
void scLTrim(CScriptVar *c, void *) {
    wString val = c->getParameter("this")->getString();
    val = val.LTrim();
    c->getReturnVar()->setString(val);
}
/////////////////////////////////////////////////////////////////////////
void scCharToInt(CScriptVar *c, void *) {
    wString str = c->getParameter("ch")->getString();
    int val = 0;
    if (str.length()>0){
        val = (int)str.c_str()[0];
    }
    c->getReturnVar()->setInt(val);
}

void scStringIndexOf(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    wString search = c->getParameter("search")->getString();
    size_t p = str.find(search);
    int val = (p==string::npos) ? -1 : p;
    c->getReturnVar()->setInt(val);
}

void scStringSubstring(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    int lo = c->getParameter("lo")->getInt();
    int hi = c->getParameter("hi")->getInt();

    int l = hi-lo;
    if (l>0 && lo>=0 && lo+l<=(int)str.length())
      c->getReturnVar()->setString(str.substr(lo, l));
    else
      c->getReturnVar()->setString("");
}
void scStringSubstr(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    int lo = c->getParameter("lo")->getInt();
    int hi = c->getParameter("hi")->getInt();

    if (hi>0 && lo>=0 && lo+hi<=(int)str.length())
      c->getReturnVar()->setString(str.substr(lo, hi));
    else
      c->getReturnVar()->setString("");
}

void scStringCharAt(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    int p = c->getParameter("pos")->getInt();
    if (p>=0 && p<(int)str.length()){
      c->getReturnVar()->setString(str.substr(p, 1));
    }else{
      c->getReturnVar()->setString("");
    }
}

void scStringCharCodeAt(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    int p = c->getParameter("pos")->getInt();
    if (p>=0 && p<(int)str.length())
      c->getReturnVar()->setInt(str.at(p));
    else
      c->getReturnVar()->setInt(0);
}

void scStringSplit(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    wString sep = c->getParameter("separator")->getString();
    CScriptVar *result = c->getReturnVar();
    result->setArray();
    int length = 0;

    size_t pos = str.find(sep);
    while (pos != string::npos) {
      result->setArrayIndex(length++, new CScriptVar(str.substr(0,pos)));
      str = str.substr(pos+1);
      pos = str.find(sep);
    }

    if (str.size()>0)
      result->setArrayIndex(length++, new CScriptVar(str));
}
//Replace
void scStringReplace(CScriptVar *c, void *) {
    wString str = c->getParameter("this")->getString();
    wString before = c->getParameter("before")->getString();
    wString after = c->getParameter("after")->getString();
    //strの中のbeforeを探す
    size_t pos = 0;
    pos = str.find(before,pos);
    while( pos != string::npos){
        str = str.substr(0,pos)+after+str.substr(pos+before.length());
        pos = str.find(before,pos); 
    }
    c->getReturnVar()->setString(str);
}

void scStringFromCharCode(CScriptVar *c, void *) {
    char str[2];
    str[0] = c->getParameter("char")->getInt();
    str[1] = 0;
    c->getReturnVar()->setString(str);
}

void scIntegerParseInt(CScriptVar *c, void *) {
    wString str = c->getParameter("str")->getString();
    int val = strtol(str.c_str(),0,0);
    c->getReturnVar()->setInt(val);
}

void scIntegerValueOf(CScriptVar *c, void *) {
    wString str = c->getParameter("str")->getString();

    int val = 0;
    if (str.length()==1)
      val = str[0];
    c->getReturnVar()->setInt(val);
}

void scJSONStringify(CScriptVar *c, void *) {
    wString result;
    c->getParameter("obj")->getJSON(result);
    c->getReturnVar()->setString(result.c_str());
}

void scExec(CScriptVar *c, void *data) {
    CTinyJS *tinyJS = (CTinyJS *)data;
    wString str = c->getParameter("jsCode")->getString();
    tinyJS->execute(str);
}

void scEval(CScriptVar *c, void *data) {
    CTinyJS *tinyJS = (CTinyJS *)data;
    wString str = c->getParameter("jsCode")->getString();
    c->setReturnVar(tinyJS->evaluateComplex(str).var);
}

void scArrayContains(CScriptVar *c, void *data) {
  CScriptVar *obj = c->getParameter("obj");
  CScriptVarLink *v = c->getParameter("this")->firstChild;

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

void scArrayRemove(CScriptVar *c, void *data) {
  CScriptVar *obj = c->getParameter("obj");
  vector<int> removedIndices;
  CScriptVarLink *v;
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
      for (size_t i=0;i<removedIndices.size();i++)
        if (n>=removedIndices[i])
          newn--;
      if (newn!=n)
        v->setIntName(newn);
      v = v->nextSibling;
  }
}

void scArrayJoin(CScriptVar *c, void *data) {
  wString sep = c->getParameter("separator")->getString();
  CScriptVar *arr = c->getParameter("this");

  wString sstr;
  int l = arr->getArrayLength();
  for (int i=0;i<l;i++) {
    if (i>0){
        sstr += sep;
    }
    sstr += arr->getArrayIndex(i)->getString();
  }

  c->getReturnVar()->setString(sstr.c_str());
}

// ----------------------------------------------- Register Functions
void registerFunctions(CTinyJS *tinyJS) {
    tinyJS->addNative("function exec(jsCode)", scExec, tinyJS); // execute the given code
    tinyJS->addNative("function eval(jsCode)", scEval, tinyJS); // execute the given wString (an expression) and return the result
    tinyJS->addNative("function trace()", scTrace, tinyJS);
    tinyJS->addNative("function Object.dump()", scObjectDump, 0);
    tinyJS->addNative("function Object.clone()", scObjectClone, 0);
    tinyJS->addNative("function Math.rand()", scMathRand, 0);
    tinyJS->addNative("function Math.randInt(min, max)", scMathRandInt, 0);
    tinyJS->addNative("function charToInt(ch)", scCharToInt, 0); //  convert a character to an int - get its value
    tinyJS->addNative("function String.indexOf(search)", scStringIndexOf, 0); // find the position of a wString in a string, -1 if not
    tinyJS->addNative("function String.substring(lo,hi)", scStringSubstring, 0);
    tinyJS->addNative("function String.substr(lo,hi)", scStringSubstr, 0);
    tinyJS->addNative("function String.charAt(pos)", scStringCharAt, 0);
    tinyJS->addNative("function String.charCodeAt(pos)", scStringCharCodeAt, 0);
    tinyJS->addNative("function String.fromCharCode(char)", scStringFromCharCode, 0);
    tinyJS->addNative("function String.split(separator)", scStringSplit, 0);
    tinyJS->addNative("function String.replace(before,after)",scStringReplace, 0 );
    tinyJS->addNative("function Integer.parseInt(str)", scIntegerParseInt, 0); // wString to int
    tinyJS->addNative("function Integer.valueOf(str)", scIntegerValueOf, 0); // value of a single character
    tinyJS->addNative("function JSON.stringify(obj, replacer)", scJSONStringify, 0); // convert to JSON. replacer is ignored at the moment
    // JSON.parse is left out as you can (unsafely!) use eval instead
    tinyJS->addNative("function Array.contains(obj)", scArrayContains, 0);
    tinyJS->addNative("function Array.remove(obj)", scArrayRemove, 0);
    tinyJS->addNative("function Array.join(separator)", scArrayJoin, 0);
    tinyJS->addNative("function Print(val)", scPrint, 0);
    tinyJS->addNative("function encodeURI(url)", scEncodeURI, 0);
    tinyJS->addNative("function File.file_exists(file)", scFileExists, 0);
    tinyJS->addNative("function String.trim()", scTrim, 0);
    tinyJS->addNative("function String.rtrim()", scRTrim, 0);
    tinyJS->addNative("function String.ltrim()", scLTrim, 0);
}

