#include <stdio.h>
#include <sys/types.h>
#include <vector>
#ifdef __linux__
#include <regex>
#else
#include <regex>
#endif
#include "ltn_String.h"

using namespace std;

class dregex
{
private:
    //regex  re;
    //int cflags;
    //static int match(const wString text, regex re);
    //static int replace(wString* result, const wString text, regex re, const wString replacement, const bool global);
    //static int split2(vector<wString>* result, const wString text, regex re, const bool global);
public:
    dregex(void){}
    ~dregex(void){
    //    regfree(&re);
    }
    static int match(const wString& text, const wString& pattern);
    static int replace(wString* result, const wString text, const vector<wString> pattern, const vector<wString> replacement);
    //static int split(vector<wString>* result, const wString text, const wString pattern, const bool global);
};
