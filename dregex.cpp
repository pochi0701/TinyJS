#include "dregex.h"
#include "wizd_String.h"
using namespace std;

// ------------------------------------------------------------
// match 事前コンパイル・インスタンス不要版
int dregex::match(const wString text, const wString pattern)
{
    int cflags = REG_NOSUB|REG_EXTENDED;
    regex_t re;
    wString tmp = pattern;
    if( tmp[0] == '/' ){
        while( tmp[tmp.length()-1] != '/' ){
            if ( tmp[tmp.length()-1] == 'i' ){
                cflags |= REG_ICASE;
                tmp = tmp.substr(0,tmp.length()-1);
            }else if ( tmp[tmp.length()-1] == 'x' ){
                cflags |= REG_EXTENDED;
                tmp = tmp.substr(0,tmp.length()-1);
            }else if ( tmp[tmp.length()-1] == 'm' ){
                cflags &= ~REG_NEWLINE;
                tmp = tmp.substr(0,tmp.length()-1);
            }else if ( tmp[tmp.length()-1] == 's' ){
                cflags |= REG_NEWLINE;
                tmp = tmp.substr(0,tmp.length()-1);
            }else{
                //not pattern
                return 0;
            }
        }
        tmp = tmp.substr(1,tmp.length()-2);
    }else{
        // not pattern
        return 0;
    }
    
    if (regcomp(&re, (char*)tmp.c_str(), cflags)) {
        return 1; // syntax error.
    }
    int res = regexec(&re, (char*)text.c_str(), 0, 0, 0);
    regfree(&re);
    return !(res == REG_NOMATCH);
}

// ------------------------------------------------------------

// replace 本体
int dregex::replace(wString* result, const wString text, regex_t re, const wString replacement, const bool global)
{
    const size_t nbmat = 10;        // 最初に見つかったマッチ範囲とグループ1～9までの範囲、計10個を保持
                                    // グループを10個以上使うような複雑なパターンは避け、複数回行うべき
    regmatch_t mat[nbmat];          // マッチ構造体
    char h_work[1024]={0};          // ホールドスペース用の作業バッファ
    char* sp = (char*)text.c_str(); // ターゲット文字列のベースポインタ
    size_t cnt = 0;                 // 元文字列の走査位置
    int shift = 0;                  // 置換する事により生じる元文字列とのずれ量を保持
    *result = text;
    do {
        if (regexec(&re, (char*)(sp+cnt), nbmat, mat, 0)) {
            if (!cnt) return 1; // replace する事がなかった
            break;
        }
        // グループ文字列をホールドスペースに積む
        vector<wString> hs;
        for (int i=1; i<(int)nbmat; i++) {
            int s = (int)mat[i].rm_so;
            int t = (int)mat[i].rm_eo;
            if (s < 0 || t < 0) {
                hs.push_back("");
                continue;
            }
            else {
                wString tmp((char*)(sp+cnt));
                tmp.copy(h_work, t-s, s);
                *(h_work + t-s) = '\0';
                hs.push_back(h_work);
            }
        }
        // 置換文字列に対するグループ文字列での置換
        wString local_rep = replacement;
        for (unsigned int i=1; i<nbmat; i++) {
            char op[3]={0};
            sprintf(op, "$%d", i);
            size_t h_index = local_rep.find(op, 0);
            if (h_index != wString::npos) {
                local_rep.replace(h_index, 2, hs.at(i-1).c_str());
            }
        }
        // 置換の実行
        int len = mat[0].rm_eo - mat[0].rm_so;
        result->replace(mat[0].rm_so+cnt+shift, len, local_rep.c_str());
        shift += local_rep.size() - len;
        cnt += mat[0].rm_eo;
    } while ( global );
    return 0;
}

// relpace 事前コンパイル・インスタンス不要版
int dregex::replace(wString* result, const wString text, const vector<wString> pattern, const vector<wString> replacement)
{
    regex_t re;
    int global = 1;
    int cflags = REG_NEWLINE|REG_NOSUB;
    wString pat;
    wString temptext;
    if( pattern.size() != replacement.size() ){
        return 1;
    }
    temptext = text;
    int num=0;
    for( size_t i = 0 ; i < pattern.size() ; i++ ){
        //パターンから修飾子を取得
        wString tmp = pattern[i];
        if( tmp[0] == '/' ){
            while( tmp[tmp.length()-1] != '/' ){
                if ( tmp[tmp.length()-1] == 'i' ){
                    cflags |= REG_ICASE;
                    tmp = tmp.substr(0,tmp.length()-1);
                }else if ( tmp[tmp.length()-1] == 'x' ){
                    cflags |= REG_EXTENDED;
                    tmp = tmp.substr(0,tmp.length()-1);
                }else if ( tmp[tmp.length()-1] == 'm' ){
                    cflags &= ~REG_NEWLINE;
                    tmp = tmp.substr(0,tmp.length()-1);
                }else if ( tmp[tmp.length()-1] == 's' ){
                    cflags |= REG_NEWLINE;
                    tmp = tmp.substr(0,tmp.length()-1);
                }else{
                    //not pattern
                    return 1;
                }
            }
            tmp = tmp.substr(1,tmp.length()-2);
        }else{
            // not pattern
            return 1;
        }
        if (regcomp(&re, (char*)tmp.c_str(), cflags&~REG_NOSUB)){
            return 1; // syntax error.
        }
        num = dregex::replace(result, temptext, re, replacement[i], global);
        regfree(&re);
        if( num == 0 ){
            temptext = *result;
        }
    }
    return num;
}

// ------------------------------------------------------------
// split 本体
int dregex::split(vector<wString>* result, const wString text, regex_t re, const bool global)
{
    regmatch_t mat[2];          //BCCでは２以上でないと正常に動作しない
    char* sp = (char*)text.c_str();
    size_t cnt = 0;
    result->clear();
    char tmp[1024]={0};
    do {
        if (regexec(&re, (char*)(sp+cnt), 2, mat, 0) == REG_NOMATCH) {
            break;
        }
        text.copy(tmp, mat[0].rm_so, cnt);
        *(tmp+mat[0].rm_so) = '\0';
        result->push_back(tmp);
        cnt += mat[0].rm_eo;
    } while (global);
    // 最終要素
    if (cnt < text.size()) {
        text.copy(tmp, text.size()-cnt, cnt);
        *(tmp+text.size()-cnt) = '\0';
        result->push_back(tmp);
    }
    if (cnt==0) return 1; // split する事がなかった
    return 0;
}

// split 事前コンパイル・インスタンス不要版(REG_NOSUBは無効)
int dregex::split(vector<wString>* result, const wString text, const wString pattern, const int cflags, const bool global)
{
    regex_t re;
    if (regcomp(&re, (char*)pattern.c_str(), cflags&~REG_NOSUB)){
        return 1; // syntax error.
    }
    int res = dregex::split(result, text, re, global);
    regfree(&re);
    return res;
}