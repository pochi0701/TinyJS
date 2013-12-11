#ifndef WIZDSTRINGH
#define WIZDSTRINGH

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
//#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
//#include <iostream>
#ifdef linux
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



#define MAXSAC 100
#define HTTP_STR_BUF_SIZE (1024*2)
#define DELIMITER "/"
class wString
{
public:
        wString(void);
        ~wString(void);
        wString(const char* str);
        wString(const wString& str);

        //friend std::ostringstream& operator<<(std::ostringstream& os, wString str );
        wString& operator+(const wString str) const;
        wString& operator+(const char* str) const;
        friend wString& operator+(const char* str1, wString str2 );
        void     operator=(const wString& str);
        void     operator=(const char* str);
        bool     operator==(wString& str) const;
        bool     operator==(const char* str) const;
        bool     operator!=(wString& str) const;
        bool     operator!=(const char* str) const;
        bool     operator>=(wString& str) const;
        bool     operator>=(const char* str) const;
        bool     operator<=(wString& str) const;
        bool     operator<=(const char* str) const;
        bool     operator>(wString& str) const;
        bool     operator>(const char* str) const;
        bool     operator<(wString& str) const;
        bool     operator<(const char* str) const;
        void     operator+=(char ch);
        void     operator+=(const wString& str);
        void     operator+=(const char* str);
        char     operator[](unsigned int index) const;
        char            at(unsigned int index) const;
        wString&        SubString(int start, int len);
        int             compare(const wString& str) const;
        int             compare(const char* str ) const;
        void            clear(void);
        void            LoadFromFile(const char* FileName);
        void            LoadFromFile(wString& str);
        void            SaveToFile(const char* FileName);
        void            SaveToFile(wString& str);
        bool            SetListString(wString& src,int pos);
        bool            SetListString(const char* src,int pos);
        wString&        GetListString(int pos);
        int             Pos(const char* pattern);
        int             Pos(wString& pattern);
        int             Pos(const char* pattern,int pos);
        int             Pos(wString& pattern,int pos);
        void            CalcCount(void);
        
        size_t          size(void) const;
        size_t          length(void) const;
        wString&        Trim(void);
        wString&        RTrim(void);
        wString&        LTrim(void);
        int             Total(void) const;
        char*           c_str(void) const;
        void            ResetLength(unsigned int num);
        int             Length(void) const ;
        int             LastDelimiter(const char* delim);
        void            SetLength(unsigned int num);
        int             sprintf(const char* format, ... );
        wString&        strsplit(const char* delimstr);
        char*           uri_encode(void);
        size_t          find( const wString& str, size_t index=0) const;
        size_t          find( const char* str, size_t index=0 ) const;
        size_t          find( char ch, size_t index = 0) const;
        static bool     RenameFile(wString& src, wString& dst);
        static bool     DeleteFile(wString& str);
        static bool     DirectoryExists(char* str);
        static bool     DirectoryExists(wString& str);
        static bool     FileExists(char* str);
        static bool     FileExists(wString& str);
        static wString& ExtractFileDir(wString& str);
        static wString& ExtractFileName(char* str, const char* delim = DELIMITER);
        static wString& ExtractFileName(wString& str, const char* delim = DELIMITER);
        static bool     CreateDir(wString& str);
        static wString& ChangeFileExt(wString& str, const char* ext);
        static unsigned long FileSizeByName(char* str);
        static unsigned long FileSizeByName(wString& str);
        static int      wStringInit(void);
        static void     wStringEnd(void);
        static wString& EnumFolder(wString& Path);
        static bool     checkUrl( wString& url );
        wString& substr(int index, int length) const;
        wString& substr(int index) const;
        static wString* NextSac(void);
        static void     copy(wString* src,const wString* dst);
        static void     copy(wString* src,const wString& dst);
        static inline char*    myrealloc(char* ptr, int size, int dispose=0);
        static size_t npos;
private:
        unsigned int len;	//実際の長さ
        unsigned int total;	//保存領域の長さ
        int count;
        char* String;
        void replace_character_len(const char *sentence,
                                   int slen,
                                   const char* p,
                                   int klen,
                                   const char *rep);
         static wString* sac[MAXSAC];
         static int sacPtr;
         static int init;
};
#endif
