#include <stdio.h>
#include <sys/types.h>
#include <vector>
#ifdef linux
#include <regex.h>
#else
#include <pcreposi.h>
#define REG_EXTENDED 0
#define REG_NOSUB 0
#endif
#include "wizd_String.h"

using namespace std;

class dregex
{
    private:
    regex_t re;
    int cflags;
    //static int match(const wString text, regex_t re);
    static int replace(wString* result, const wString text, regex_t re, const wString replacement, const bool global);
    static int split(vector<wString>* result, const wString text, regex_t re, const bool global);
    public:
    dregex(){}
    ~dregex(){
        regfree(&re);
    }
    static int match(const wString text, const wString pattern);
    static int replace(wString* result, const wString text, const vector<wString> pattern, const vector<wString> replacement);
    static int split(vector<wString>* result, const wString text, const wString pattern, const int cflags, const bool global);
};