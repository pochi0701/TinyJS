#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>
//#include <iostream>
//#include <sstream>
#include <algorithm>
#ifdef linux
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#else
#include <process.h>
#include <dir.h>
#include <direct.h>
#include <io.h>
#endif
#include "wizd_String.h"

//---------------------------------------------------------------------------
//source
//---------------------------------------------------------------------------
wString* wString::sac[MAXSAC];
int wString::sacPtr=0;
size_t wString::npos=(size_t)(-1);
//---------------------------------------------------------------------------
int  wString::wStringInit(void)
{
    for( int i = 0 ; i < MAXSAC ; i++ ){
        sac[i] = new wString();
        sac[i]->SetLength(512);
    }
    sacPtr = 0;
    return 1;
}
//---------------------------------------------------------------------------
void wString::wStringEnd(void)
{
    for( int i = 0 ; i < MAXSAC ; i++ ){
        delete sac[i];
    }
}
//---------------------------------------------------------------------------
//リングストリングバッファ領域を取得
wString* wString::NextSac(void)
{
    wString* ptr = sac[sacPtr++];
    sacPtr = sacPtr%MAXSAC;
    //新文字列は長さ０
    ptr->len = 0;
    ptr->count = 0;
    if( ptr->String ){
        //memset(ptr->String,0,ptr->total);
        *ptr->String = 0;
    }
    return ptr;
}
//---------------------------------------------------------------------------
//通常コンストラクタ
wString::wString(void)
{
    //初期化
    len = 0;
    total = 1;
    count = 0;
    String = (char*)new char[1];
    *String = 0;
}
//---------------------------------------------------------------------------
//文字列コンストラクタ
wString::wString(const char *str)
{
    //初期化
    count = 0;
    len = strlen(str);
    if( len ){
        total = len+1;
        String = (char*)new char[total];
        strcpy(String, str);
    }else{
        total = 1;
        String = (char*)new char[1];
        String[0] = 0;
    }
}
//---------------------------------------------------------------------------
//コピーコンストラクタ
wString::wString(const wString& str)
{
    //初期化
    len   = str.len;
    if( str.len ){
        total = str.total;
        String = new char[str.total];
        memcpy(String,str.String,str.total);
        //*String = 0;
    }else{
        total = 1;
        String = (char*)new char[1];
        String[0] = 0;
    }
}
//---------------------------------------------------------------------------
//デストラクタ
wString::~wString()
{
        delete [] String;
}
//---------------------------------------------------------------------------
//ディープコピーメソッド
void wString::copy(wString* src,const wString* dst)
{
    src->myrealloc(dst->total);
    memcpy( src->String, dst->String, dst->total);
    src->len = dst->len;
    src->count = dst->count;
}
//---------------------------------------------------------------------------
//ディープコピーメソッド
void wString::copy(wString* src,const wString& dst)
{
    src->myrealloc(dst.total);
    memcpy( src->String, dst.String, dst.total);
    src->len = dst.len;
    src->count = dst.count;
}
//---------------------------------------------------------------------------
wString& wString::operator+(const wString& str) const
{
    wString* temp = NextSac();
    copy(temp,this);
    *temp += str;
    return *temp;
}
//---------------------------------------------------------------------------
wString& wString::operator+(const char* str) const 
{
    wString* temp = NextSac();
    copy(temp,this);
    *temp +=str;
    return *temp;
}
//---------------------------------------------------------------------------
wString& operator+(const char* str1, wString str2 )
{
    wString* temp = wString::NextSac();
    size_t newLen = strlen(str1)+str2.len;
    temp->myrealloc(newLen);
    strcpy(temp->String, str1);
    strcat(temp->String, str2.String);
    temp->len = newLen;
    return *temp;
}
//---------------------------------------------------------------------------
void wString::operator+=(const wString& str)
{
    count += str.count;
    unsigned int newLen = len+str.len;
    myrealloc(newLen);
    memcpy(String+len,str.String,str.len);
    String[newLen] =0;
    len    = newLen;
    return;
}
//---------------------------------------------------------------------------
void wString::operator+=(const char* str)
{
    unsigned int slen = strlen(str);
    unsigned int newLen = slen+len;
    myrealloc(newLen);
            strcpy( String+len, str );
    len = newLen;
    return;
}
//---------------------------------------------------------------------------
void wString::operator+=(const char ch)
{
    int tmpl=len>>4;
    tmpl <<= 4;
    tmpl += 64;
    myrealloc(tmpl);
        String[len] = ch;
        String[len+1] = 0;
    len+=1;
    return;
}
//---------------------------------------------------------------------------
bool wString::operator==(wString& str) const
{
    if( len  != str.len ){
        return false;
    }
    return( strncmp( String, str.String, len) == 0);
}
//---------------------------------------------------------------------------
bool wString::operator==(const char* str) const
{
    unsigned int mylen = strlen(str);
    if( len != mylen ){
        return false;
    }
    return( strncmp( String, str, len) == 0);
}
//---------------------------------------------------------------------------
bool wString::operator!=(wString& str) const
{
    if( len != str.len ){
       return true;
    }
    return( strncmp( String, str.String, len) != 0);
}
//---------------------------------------------------------------------------
bool wString::operator!=(const char* str) const
{
    unsigned int mylen = strlen(str);
    if( len != mylen ){
        return true;
    }
    return( strncmp( String, str, len) != 0);
}
//---------------------------------------------------------------------------
bool wString::operator>=(wString& str) const
{
    unsigned int maxlen = (len>str.len)?len:str.len;
    return( strncmp( String, str.String, maxlen) >= 0);
}
//---------------------------------------------------------------------------
bool wString::operator>=(const char* str) const
{
    unsigned int mylen = strlen(str);
    unsigned int maxlen = (len>mylen)?len:mylen;
    return( strncmp( String, str, maxlen) >= 0);
}
//---------------------------------------------------------------------------
bool wString::operator<=(wString& str) const
{
    unsigned int maxlen = (len>str.len)?len:str.len;
    return( strncmp( String, str.String, maxlen) <= 0);
}
//---------------------------------------------------------------------------
bool wString::operator<=(const char* str) const
{
    unsigned int mylen = strlen(str);
    unsigned int maxlen = (len>mylen)?len:mylen;
    return( strncmp( String, str, maxlen) <= 0);
}
//---------------------------------------------------------------------------
bool wString::operator>(wString& str) const
{
    unsigned int maxlen = (len>str.len)?len:str.len;
    return( strncmp( String, str.String, maxlen) > 0);
}
//---------------------------------------------------------------------------
bool wString::operator>(const char* str) const
{
    unsigned int mylen = strlen(str);
    unsigned int maxlen = (len>mylen)?len:mylen;
    return( strncmp( String, str, maxlen) > 0);
}
//---------------------------------------------------------------------------
bool wString::operator<(wString& str) const
{
    unsigned int maxlen = (len>str.len)?len:str.len;
    return( strncmp( String, str.String, maxlen) < 0);
}
//---------------------------------------------------------------------------
bool wString::operator<(const char* str) const
{
    unsigned int mylen = strlen(str);
    unsigned int maxlen = (len>mylen)?len:mylen;
    return( strncmp( String, str, maxlen) < 0);
}
//---------------------------------------------------------------------------
void wString::operator=(const wString& str)
{
    count = str.count;
    myrealloc(str.total);
    memcpy( String, str.String,str.total );
   len   = str.len;
    return;
}
//---------------------------------------------------------------------------
void wString::operator=(const char* str)
{
    count = 0;
    int newLen = strlen(str);
    myrealloc(newLen);
    strcpy( String, str);
    len = newLen;
    return;
}
//---------------------------------------------------------------------------
char wString::operator[](unsigned int index) const
{
     if( index < len ){
          return String[index];
     }else{
          perror( "out bound");
          return -1;
     }
}
//---------------------------------------------------------------------------
char wString::at(unsigned int index) const
{
    if( index < len ){
          return String[index];
     }else{
          perror( "out bound");
          return -1;
     }
} 
//---------------------------------------------------------------------------
wString& wString::SetLength(const unsigned int num)
{
    myrealloc(num);
    return *this;
}
//---------------------------------------------------------------------------
// 比較
//---------------------------------------------------------------------------
int wString::compare(const wString& str) const
{
    return strcmp( String, str.String );    
    //size_t minlen = (len>str.len)?str.len+1:len+1;
    //return memcmp( String, str.String, minlen );
}
//---------------------------------------------------------------------------
// 比較 
//---------------------------------------------------------------------------
int wString::compare(const char* str ) const
{
    return strcmp(String,str);
}
//---------------------------------------------------------------------------
// クリア 
//---------------------------------------------------------------------------
void  wString::clear(void)
{
    len = 0;
    if( String ){
        String[0] = 0;
    }
}
//---------------------------------------------------------------------------
// 部分文字列
//---------------------------------------------------------------------------
wString&  wString::SubString(int start, int mylen) const
{
    wString* temp = NextSac();
    if( mylen>0){
        temp->myrealloc( mylen );
        memcpy( temp->String, String+start,mylen);
    temp->String[mylen] = 0;
    //長さ不定。数えなおす
        temp->len = mylen;//strlen(temp->String);
    }
    return *temp;
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
size_t wString::find( const wString& str, size_t index) const
{
    char* ptr = strstr(String+index, str.String);
    if( ptr == NULL ){
        return npos;
    }else{
        return (size_t)(ptr-String);
    }
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
size_t wString::find( const char* str, size_t index ) const
{
    char* ptr = strstr(String+index, str);
    if( ptr == NULL ){
        return npos;
    }else{
        return (size_t)(ptr-String);
    }
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
size_t wString::find( char ch, size_t index ) const
{
    char str[2];   
    str[0] = ch;
    str[1] = 0;
    char* ptr = strstr(String+index, str);
    if( ptr == NULL ){
        return npos;
    }else{
        return (size_t)(ptr-String);
    }
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
int wString::Pos(const char* pattern)
{
    char* ptr = strstr(String,pattern);
    if( ptr == NULL ){
        return npos;
    }else{
        return (int)(ptr-String);
    }
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
int wString::Pos(const char* pattern,int pos)
{
    char* ptr = strstr(String+pos,pattern);
    if( ptr == NULL ){
        return npos;
    }else{
        return (int)(ptr-String);
    }
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
int wString::Pos(wString& pattern)
{
    return Pos(pattern.String);
}
//---------------------------------------------------------------------------
// 位置
//---------------------------------------------------------------------------
int wString::Pos(wString& pattern, int pos)
{
    return Pos(pattern.String,pos);
}
//---------------------------------------------------------------------------
//　Size
//---------------------------------------------------------------------------
size_t  wString::size(void) const
{
    return (size_t)len;
}
//---------------------------------------------------------------------------
//　Size
//---------------------------------------------------------------------------
size_t  wString::length(void) const
{
    return (size_t)len;
}
//---------------------------------------------------------------------------
// ファイル読み込み
//---------------------------------------------------------------------------
void wString::LoadFromFile(wString& str)
{
    LoadFromFile(str.String);
}
//---------------------------------------------------------------------------
// ファイル読み込み
//---------------------------------------------------------------------------
int wString::LoadFromFile(const char* FileName)
{
    long flen;
    int  handle;
#ifdef linux
    handle  = open(FileName,O_RDONLY | S_IREAD );
#else
    handle  = myopen(FileName,O_RDONLY | O_BINARY | S_IREAD );
#endif
    if( handle<0 ){
        return -1;
    }
    flen = lseek(handle,0,SEEK_END);
           lseek(handle,0,SEEK_SET);
    SetLength(flen+1);
    len = read( handle, String, flen);
    close(handle);
    String[len] = 0;
    //\0がある場合を考えればstrlenとってはいけない
    //len = strlen(String);
    CalcCount();
    return 0;
}
//---------------------------------------------------------------------------
// ファイル読み込み
//---------------------------------------------------------------------------
void wString::LoadFromCSV(wString& str)
{
    LoadFromCSV(str.String);
}
int isNumber(char* str)
{
    for( int i = strlen(str)-1; i >= 0 ; i-- ){
        if( (! isdigit(str[i]) ) && str[i] != '.' ){
            return 0;
        }
    }
    return 1;
}
            
//---------------------------------------------------------------------------
// ファイル読み込み
//---------------------------------------------------------------------------
void wString::LoadFromCSV(const char* FileName)
{
    int  fd;
    char s[1024];
    char t[1024];;
    int ret;
    int first=1;
    fd = myopen(FileName, O_RDONLY | O_BINARY );
    if( fd < 0 ){
        printf( "%sファイルが開けません\n", FileName );
        return;
    }
    *this = "[";
    //１行目はタイトル
    while(true){
        ret = readLine( fd, s, sizeof(s) );
        if( ret < 0 ) break;
        //分解する
        char *p = strtok(s,",");
        int ptr=0;
        if( p ){
            if( isNumber(p) ){ 
                ptr += ::sprintf( t+ptr,"%s", p );
            }else{
                ptr += ::sprintf( t+ptr,"\"%s\"", p );
            }
        }
        while( p = strtok(NULL,",") ){
            if( isNumber(p) ){ 
                ptr += ::sprintf( t+ptr,",%s", p );
            }else{
                ptr += ::sprintf( t+ptr,",\"%s\"", p );
            }
        }
        if( first ){
            first = 0;
        }else{
            *this += ",";
        }
        *this += wString("[")+t+"]";
    }
    *this += "]";
    close( fd );
    return;
}
//---------------------------------------------------------------------------
// ファイル書き込み
//---------------------------------------------------------------------------
int wString::SaveToFile(wString& str)
{
    return SaveToFile(str.String);
}
//---------------------------------------------------------------------------
// ファイル書き込み
//---------------------------------------------------------------------------
int wString::SaveToFile(const char* FileName)
{
    int handle = open(FileName,O_CREAT| O_TRUNC | O_RDWR| O_BINARY, S_IREAD| S_IWRITE);
    if( handle < 0 ){
        return handle;
    }
    write( handle,String, len);
    close( handle);
    return 0;
}
//---------------------------------------------------------------------------
// トリム
//---------------------------------------------------------------------------
wString& wString::Trim(void)
{
    wString* temp = NextSac();
    copy(temp,this);
    if( len ){
        //先頭の空白等を抜く
        while( *temp->String && *temp->String <= ' ' ){
            #ifdef linux
            char* ptr=temp->String;
            while( *(ptr+1) ){
                *ptr = *(ptr+1);
                 ptr++;
            }
            #else
            strcpy( (char*)temp->String, (char*)(temp->String+1) );
            #endif
            temp->len--;
        }
        //末尾の空白等を抜く
        while( temp->len && String[temp->len-1] <= ' ' ){
            temp->String[--temp->len] = 0;
        }
    }
    return *temp;
}
//---------------------------------------------------------------------------
// トリム
//---------------------------------------------------------------------------
wString& wString::RTrim(void)
{
    wString* temp = NextSac();
    copy(temp,this);
    if( len ){
        //末尾の空白等を抜く
        while( temp->len && String[temp->len-1] <= ' ' ){
            temp->String[--temp->len] = 0;
        }
    }
    return *temp;
}
//---------------------------------------------------------------------------
// トリム
//---------------------------------------------------------------------------
wString& wString::LTrim(void)
{
    wString* temp = NextSac();
    copy(temp,this);
    if( len ){
        //先頭の空白等を抜く
        while( *temp->String && *temp->String <= ' ' ){
            char* ptr=temp->String;
            while( *(ptr+1) ){
                *ptr = *(ptr+1);
                 ptr++;
            }
            temp->len--;
        }
    }
    return *temp;
}
//---------------------------------------------------------------------------
// substr
//---------------------------------------------------------------------------
wString& wString::substr(int index) const
{
    wString* temp = NextSac();
    int newLen = len-index;
    if( newLen>0 ){
        temp->myrealloc(newLen);
        memcpy(temp->String, String+index,newLen);
        temp->String[newLen] = 0;     
        temp->len = newLen;
    }
    return *temp;
}
//---------------------------------------------------------------------------
// substr
//---------------------------------------------------------------------------
wString& wString::substr(int index, int mylen) const
{
    wString* temp = NextSac();
    int newLen = mylen;
    if( newLen > (int)len-index ){
        newLen = len-index;
    }
    if( newLen>0 ){
        temp->myrealloc(newLen);
        memcpy(temp->String,String+index,newLen);
        temp->String[newLen] = 0;
        temp->len = newLen;
    }
    return *temp;
}
//--------------------------------------------------------------------
wString& wString::FileStats(const char* str,int mode)
{
    struct stat      stat_buf;
    wString* buf=NextSac();
    if (stat(str, &stat_buf) == 0 && mode == 0 ) {
        /* ファイル情報を表示 */
        buf->sprintf( "{\"permission\":\"%o\",\"size\":%d,\"date\":\"%s\"}",stat_buf.st_mode, stat_buf.st_size, ctime(&stat_buf.st_mtime));
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
    }else{
        //date
        if( mode == 1 ){
            //char s[128] = {0};
            //time_t timer;
            //struct tm *timeptr;
            //timer = time(NULL);
            //timeptr = localtime(&stat_buf.st_mtime);
            //strftime(s, 128, "%Y/%m/%d %H:%M:%S", timeptr);
            buf->sprintf( "%d",stat_buf.st_mtime );
        }
    }
    return *buf;
}
//---------------------------------------------------------------------------
wString& wString::FileStats(wString& str, int mode)
{
    return FileStats(str.String,mode);
}

//---------------------------------------------------------------------------
int wString::FileExists(char* str)
{
    int  flag=0;
#ifdef linux
    struct stat send_filestat;
    int  result = stat(str, &send_filestat);
    if ( ( result == 0 ) && ( S_ISREG(send_filestat.st_mode) == 1 ) ){
        flag = 1;
    }
#else
    struct ffblk send_filestat;
    int result = findfirst(str,&send_filestat, 0);
    if( result == 0 && ( send_filestat.ff_attrib & FA_DIREC ) == 0){
        flag = 1;
    }
#endif
    return flag;
}
//---------------------------------------------------------------------------
int wString::FileExists(wString& str)
{
    return FileExists(str.String);
}
//---------------------------------------------------------------------------
//パス部分を抽出
wString& wString::ExtractFileDir(wString& str)
{
    int ptr;
    //todo SJIS/EUC対応するように
    wString* temp = NextSac();
    copy(temp,&str);
    ptr = temp->LastDelimiter( DELIMITER );
    temp->len = ptr;
    temp->String[ptr] = 0;
    return *temp;
}
//---------------------------------------------------------------------------
int wString::CreateDir(wString& str)
{
    int flag=0;
    if( ! DirectoryExists(str.String) ){ 
#ifdef linux
    //0x777ではちゃんとフォルダできない
    flag = (mkdir( str.String,0777 ) != -1 );
#else
    flag = (mkdir( str.String ) != -1 );
#endif
    }
    return flag;
}
//---------------------------------------------------------------------------
//　TStringList対策
//  行数を返す
//---------------------------------------------------------------------------
void wString::ResetLength(unsigned int num)
{
    assert(total>(unsigned int)num);
    String[num] = 0;
    len = num;
}
//---------------------------------------------------------------------------
int wString::Count(void)
{
    return count;
}
//---------------------------------------------------------------------------
char* wString::c_str(void) const
{
    return String;
}
//---------------------------------------------------------------------------
int wString::Length(void) const
{
    return len;
}
//---------------------------------------------------------------------------
int wString::Total(void) const
{
    return total;
}
//---------------------------------------------------------------------------
int wString::LastDelimiter(const char* delim) const
{
    int pos = -1;
    int dlen = strlen( delim );
    for( int i = len-dlen ;i > 0 ; i--){
        if( strncmp( String+i, delim, dlen ) == 0 ){
            pos = i;
            break;
        }
    }
    return pos;
}
//---------------------------------------------------------------------------
bool wString::RenameFile(wString& src, wString& dst)
{
   if( rename(src.c_str(), dst.c_str()) >= 0 ){
        return true;
   }else{
        return false;
   }
}
//---------------------------------------------------------------------------
unsigned long wString::FileSizeByName(char* str)
{
    unsigned long pos;
    int handle;
    #ifdef linux
    handle = open(str,0);
    #else
    handle = open(str,O_BINARY);
    #endif
    pos = lseek( handle,0,SEEK_END);
    close( handle );
    return pos;
}
//---------------------------------------------------------------------------
unsigned long wString::FileSizeByName(wString& str)
{
    return FileSizeByName(str.String);
}
//---------------------------------------------------------------------------
wString& wString::ExtractFileName(const char* str, const char* delim)
{
    wString* tmp;
    tmp = NextSac();
    *tmp = str;
    return ExtractFileName(*tmp, delim);
}
//---------------------------------------------------------------------------
wString& wString::ExtractFileName(const wString& str, const char* delim)
{
    int pos = str.LastDelimiter(delim);
    wString* tmp = NextSac();
    copy(tmp,str.SubString(pos+1,str.Length()-pos+1));
    return *tmp;
}
//---------------------------------------------------------------------------
wString& wString::ExtractFileExt(const wString& str)
{
    int pos = str.LastDelimiter(".");
    wString* tmp=NextSac();
    copy(tmp,str.SubString(pos+1,str.length()-pos-1));
    return *tmp;
}

//---------------------------------------------------------------------------
wString& wString::ChangeFileExt(wString& str, const char* ext)
{
    int pos = str.LastDelimiter(".");
    wString* tmp=NextSac();
    copy(tmp,str.SubString(0,pos+1));
    *tmp += ext;
    return *tmp;
}
//---------------------------------------------------------------------------
int wString::DeleteFile(const wString& str)
{
    int flag=0;
    #ifdef linux
    flag = (unlink(str.String)==0);
    #else
    if( FileExists(str.String) ){
        if( unlink(str.String)==0){
            flag = 1;
        }
    }
    #endif
    return flag;
}
//---------------------------------------------------------------------------
int wString::DirectoryExists(char* str)
{
    int flag=0;
#ifdef linux
    struct stat send_filestat;
    int  result = stat(str, &send_filestat);
    if ( ( result == 0 ) && ( S_ISDIR(send_filestat.st_mode) == 1 ) ){
        flag = 1;
    }
#else
    struct ffblk send_filestat;
    int result = findfirst(str,&send_filestat, FA_DIREC );
    if( result >= 0){
        if ((send_filestat.ff_attrib & FA_DIREC) == FA_DIREC){
            flag = 1;
    }
    }
#endif
    return flag;
}
//---------------------------------------------------------------------------
int wString::DirectoryExists(wString& str)
{
    return DirectoryExists(str.String);
}
/********************************************************************************/
// sentence文字列内のkey文字列をrep文字列で置換する。
// sentence:元文字列
// slen:元文字列の長さ
// p:置換前の位置
// klen:置換前の長さ
// rep:置換後文字列
/********************************************************************************/
void wString::replace_character_len(const char *sentence,int slen,const char* p,int klen,const char *rep)
{
    char* str;
    int rlen=strlen((char*)rep);
    int num;
    if( klen == rlen ){
        memcpy( (void*)p,rep,rlen);
    //前詰め置換そのままコピーすればいい
    }else if( klen > rlen ){
        num = klen-rlen;
        strcpy( (char*)p,(char*)(p+num));
        memcpy( (void*)p,rep,rlen);
    //置換文字が長いので後詰めする
    }else{
        num = rlen-klen;
        //pからrlen-klenだけのばす
        for( str = (char*)(sentence+slen+num) ; str > p+num ; str-- ){
            *str = *(str-num);
        }
        memcpy( (void*)p,rep,rlen);
    }
    return;
}
//---------------------------------------------------------------------------
// フォルダのファイルを数える
// 引数　wString Path:
// 戻値　ファイルリスト。ほっておいていいが、コピーしてほしい
wString& wString::EnumFolderjson(wString& Path)
{
    DIR                  *dir;
    struct dirent        *ent;
    wString              temp;
    wString              Path2;
    std::vector<wString> list;
    Path2 = Path;
    //Directoryオープン
    if ((dir = opendir(Path.String)) != NULL){
        //ファイルリスト
        while ((ent = readdir(dir)) != NULL){
            if( strcmp(ent->d_name,"." ) != 0 &&
            strcmp(ent->d_name,"..") != 0 ){
                list.push_back("\""+Path2+DELIMITER+ent->d_name+"\"");
            }
        }
        closedir(dir);
        
        //sort
        if( list.size()>0){
            for( unsigned int ii = 0 ; ii < list.size()-1; ii++){
                for( unsigned int jj = ii+1 ; jj < list.size(); jj++){
                    if( strcmp(list[ii].c_str(),list[jj].c_str())> 0 ){
                        std::swap(list[ii],list[jj]);
                    }
                }
            }
        }        temp = "[";
        for( unsigned int i = 0 ; i < list.size() ; i++ ){
            if( i ) temp += ",";
            temp += list[i];
        }
        temp += "]";
    }else{
        perror("ディレクトリのオープンエラー");
        exit(1);
    }
    wString* ret = NextSac();
    *ret = temp;
    return *ret;
}
//wString 可変引数
int wString::sprintf(const char* format, ... )
{
    #ifndef va_copy
    String[0] = 0;
    len = 0;
    va_list ap;
    va_start(ap, format);
    vtsprintf(format, ap);
    va_end(ap);
    return len;
    #else
    int stat;
    //可変引数を２つ作る
    va_list ap1,ap2;
    va_start(ap1, format);
    va_copy (ap2, ap1);
    //最初はダミーで文字列長をシミュレート
    stat = vsnprintf(String, 0, format, ap1);
    SetLength( stat+1 );
    //実際に出力
    stat = vsprintf(String, format, ap2);
    va_end(ap1);
    va_end(ap2);
    len = stat;
    return stat;
    #endif
}
//---------------------------------------------------------------------------
//wString 可変引数
int wString::cat_sprintf(const char* format, ... )
{
    #ifndef va_copy
   int status;
   va_list ap;
   va_start(ap, format);
    status = this->vtsprintf(format, ap);
   va_end(ap);
   return status;
    #else
    int stat;
    //可変引数を２つ作る
    va_list ap1,ap2;
    va_start(ap1, format);
    va_copy (ap2, ap1);
    //最初はダミーで文字列長をシミュレート
    stat = vsnprintf(String, 0, format, ap1);
    SetLength( stat+len+1 );
    //実際に出力
    stat = vsprintf(String+len, format, ap2);
    va_end(ap1);
    va_end(ap2);
    len += stat;
    return stat;
    #endif
}
//---------------------------------------------------------------------------
//文字列をデリミタで切って、デリミタ後の文字を返す
//引数
//wString str           :入力文字列
//const char* delimstr  :切断文字列
//戻り値
//wString&              :切断後の文字列
//見つからない場合は長さ０の文字列
wString& wString::strsplit(const char* delimstr)
{

    wString* tmp = NextSac();
    int delimlen = strlen(delimstr);
    int pos = Pos(delimstr);
    if( pos >= 0 ){
        *tmp = SubString(pos+delimlen,len-pos-delimlen);
    }
    return *tmp;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void wString::myrealloc(const int newsize)
{
    if( len>=total){
        printf( "not good %d %d",len,total );
        exit( 1);
    }
    if( (int)total<=newsize){
        total = newsize+1;
        char* tmp = new char[total];
        memcpy(tmp,String,len);
        tmp[len]=0;
        delete[] String;
        String = tmp;
    }else{
        //指定サイズが元より小さいので何もしない
    }
}
//---------------------------------------------------------------------------
// **************************************************************************
//  URIエンコードを行います.
//  機能 : URIデコードを行う
//  書式 : int uri_encode
//  (char* dst,size_t dst_len,const char* src,int src_len);
//  引数 : dst 変換した文字の書き出し先.
//                 dst_len 変換した文字の書き出し先の最大長.
//                 src 変換元の文字.
//                 src_len 変換元の文字の長さ.
//  返値 : エンコードした文字の数(そのままも含む)
// **************************************************************************
char* wString::uri_encode(void)
{
    unsigned int is;
    wString dst;
    char work[8];
    // 引数チェック
    if(len == 0 ){
        //０バイト
        return (char*)"";
    }
    for ( is = 0 ; is < len ; is++){
        /* ' '(space) はちと特別扱いにしないとまずい */
        if ( String[is] == ' ' ){
            dst += "%20";
        /* エンコードしない文字全員集合 */
            //        }else if ( strchr("!$()*,-./:;?@[]^_`{}~", String[is]) != NULL ){
        }else if ( strchr("!$()*,-.:;/?@[]^_`{}~", String[is]) != NULL ){
            dst += String[is];
        /* アルファベットと数字はエンコードせずそのまま */
        }else if ( isalnum( String[is] ) ){
            dst += String[is];
        }
        /* \マークはエンコード */
        else if ( String[is] == '\\' ){
            dst += "%5c";
        /* それ以外はすべてエンコード */
        }else{
            ::sprintf(work,"%%%2X",(unsigned char)String[is]);
            dst += work;
        }
    }
    wString* temp = NextSac();
    *temp = dst;
    return temp->String;
}
int   wString::FileCopy(const char* fname_r, const char* fname_w)
{
    int fpr;
    int fpw;
    int size;
    unsigned char buf[8000];

    fpr = myopen( fname_r, O_RDONLY | O_BINARY );
    if( fpr < 0 ){
        return -1;
    }
    fpw = myopen( fname_w, O_CREAT | O_TRUNC  | O_WRONLY | O_BINARY , S_IREAD | S_IWRITE );
    if( fpw < 0 ){
        close( fpr );
        return -1;
    }
    while( 1 ){
        size = read( fpr , buf, sizeof(buf) );
        if( size <= 0 ){
            break;
        }
        write( fpw, buf, size );
    }
    close( fpr );
    close( fpw );
    
    return 0;
}
//---------------------------------------------------------------------------
void wString::Add(const char* str)
{
    *this += str;
    *this += "\r\n";
    count++;
}
//---------------------------------------------------------------------------
void wString::Add(wString& str)
{
    *this += str;
    *this += "\r\n";
    count++;
}
//---------------------------------------------------------------------------
//　TStringList対策
//  行数を返す
//---------------------------------------------------------------------------
int wString::CalcCount(void)
{
    count = 0;
    char* ptr = String;
    while( *ptr ){
        if( *ptr == '\r' ){
            ptr++;
            continue;
        }
        if( *ptr++ == '\n' ){
            count++;
        }
    }
    return count;
}
//---------------------------------------------------------------------------
//　TStringList対策
//  行数を返す
//　リエントラントでないことに注意
//　戻り値は使い捨て
wString& wString::GetListString(int pos)
{
    int   ccount = 0;
    int   ptr =0;
    int   ptr0=0;
    int   ptr1;
    int   inc=0;
    //assert(ptr>=0&&String !=NULL);
    if( len == 0 ){
        return *NextSac();
    }
    while( String[ptr] ){
        if( String[ptr] == '\r' ){
            ptr++;
            inc++;
            continue;
        }
        if( String[ptr++] == '\n' ){
            ptr1 = ptr0;
            ptr0 = ptr;
            inc++;
            if( ccount++ == pos ){
                wString* work = NextSac();
                copy(work,SubString(ptr1,ptr0-ptr1-inc));
                return *work;
            }
        }
        inc = 0;
    }
    //０バイト
    return *NextSac();
}
//---------------------------------------------------------------------------
size_t wString::copy( char *str, size_t slen, size_t index ) const
{
    strncpy( str, String+index,slen);
    str[slen] = 0;
    return slen;
}
wString& wString::replace( size_t index, size_t slen, const wString& repstr)
{
    size_t rlen = repstr.len;
    //同じ
    if( slen == rlen ){
        memcpy( (void*)(String+index), (void*)repstr.String, rlen );
        //前詰め置換そのままコピーすればいい
    }else if( slen > rlen ){
        size_t num = slen-rlen;
        char* p = String+index;
        char* q = p+num;
        while( *q ){
            *p++ = *q++;
        }
        memcpy( (void*)(String+index),(void*)(repstr.String),rlen);
        len -= num;
        String[len] = 0;
        //置換文字が長いので後詰めする
    }else{
        size_t num = rlen-slen;
        myrealloc( len+num+1 );
        for( char* p = (char*)(String+len+num) ; p > String+index+num ; p-- ){
            *p = *(p-num);
        }
        memcpy( (void*)(String+index),(void*)(repstr.String),rlen);
        len += num;
        String[len] = 0;
    }
    return *this;
}
//linux/windows共用オープン
//追加: O_CREAT | O_APPEND | O_WRONLY(またはO_RDWR) | (O_BINARY) , S_IREAD | S_IWRITE
//新規: O_CREAT | O_TRUNC  | O_WRONLY(またはO_RDWR) | (O_BINARY) , S_IREAD | S_IWRITE
//読込: O_RDONLY                                     | (O_BINARY) 
int myopen(const char* filename,int amode, int option)
{
#ifdef linux
    if( option != 0 ){
        return open(filename,amode,option);
    }else{
    return open(filename,amode);
    }
#else
    char work[1024];
    strcpy( work, filename);
    int ptr=0;
    while( work[ptr] ){
        if( work[ptr] == '/' ){
            work[ptr] = '\\';
        }
        ptr++;
    }
    if( option != 0 ){
        return open(work,amode,option);
    }else{
        return open(work,amode);
    }
#endif
}
int readLine(int fd, char *line_buf_p, int line_max)
{
    char byte_buf;
    int  line_len=0;
    int	 recv_len;
    // １行受信実行
    while ( 1 ){
        recv_len = read(fd, &byte_buf, 1);
        if ( recv_len != 1 ){ // 受信失敗チェック
            return ( -1 );
        }
        // CR/LFチェック
        if       ( byte_buf == '\r' ){
            continue;
        }else if ( byte_buf == '\n' ){
            *line_buf_p = 0;
            break;
        }
        // バッファにセット
        *line_buf_p++ = byte_buf;
        // 受信バッファサイズチェック
        if ( ++line_len >= line_max){
            // バッファオーバーフロー検知
            return ( -1 );
        }
    }
    return line_len;
}



