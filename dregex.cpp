// ==========================================================================
//code=UTF8	tab=4
//
// Lutino:	Application SErver.
//
// 		dregex.cpp
//		$Revision: 1.0 $
//		$Date: 2018/02/12 21:11:00 $
//
// ==========================================================================
//---------------------------------------------------------------------------
#include "define.h"
#include "dregex.h"
#include "ltn_String.h"
#include "ltn_tools.h"
using namespace std;

// Helper function: Extract regex pattern and flags from /pattern/flags format
// Returns true if successful, false if format is invalid
// Updates 'pattern_out' with the extracted pattern
// Updates 'icase' with case-insensitive flag state
static bool extract_pattern(const wString& input, wString& pattern_out, bool& icase)
{
	icase = false;
	
	// Validate input
	if (input.empty() || input[0] != '/') {
		debug_log_output("dregex::extract_pattern - ERROR: pattern must start with /");
		return false;
	}
	
	// Find the closing slash, protecting against infinite loops
	size_t len = input.length();
	size_t last_slash = input.rfind('/');
	
	if (last_slash == 0 || last_slash == string::npos) {
		debug_log_output("dregex::extract_pattern - ERROR: pattern must end with /");
		return false;
	}
	
	// Extract flags from after the closing slash
	for (size_t i = last_slash + 1; i < len; i++) {
		char c = input[i];
		if (c == 'i') {
			icase = true;
		}
		// Other flags (x, m, s) are parsed but not used in std::regex currently
		else if (c != 'x' && c != 'm' && c != 's') {
			wString msg = "dregex::extract_pattern - ERROR: unknown flag '";
			msg += c;
			msg += "' in pattern";
			debug_log_output(msg.c_str());
			return false;
		}
	}
	
	// Extract pattern between first and last slash
	if (last_slash <= 1) {
		debug_log_output("dregex::extract_pattern - ERROR: empty pattern detected");
		return false;
	}
	
	pattern_out = input.substr(1, last_slash - 1);
	return true;
}

// match - Pre-compiled instance-free version with infinite loop protection
int dregex::match(const wString& text, const wString& pattern)
{
	wString extracted_pattern;
	bool icase = false;
	
	if (!extract_pattern(pattern, extracted_pattern, icase)) {
		return 0;
	}
	
	try {
		std::regex::flag_type flags = icase ? std::regex::icase : std::regex::ECMAScript;
		std::regex re(extracted_pattern.c_str(), flags);
		return std::regex_search(text.c_str(), re);
	}
	catch (const regex_error& err)
	{
		IGNORE_PARAMETER(err);
		debug_log_output("dregex::match - ERROR: regex compilation or execution error");
		return 0;
	}
	catch (const exception& e)
	{
		IGNORE_PARAMETER(e);
		debug_log_output("dregex::match - ERROR: unexpected exception occurred");
		return 0;
	}
}

// replace - Safe implementation with infinite loop protection
// Processes multiple pattern/replacement pairs with proper error handling
int dregex::replace(wString* result, const wString text, const vector<wString> pattern, const vector<wString> replacement)
{
	if (!result) {
		debug_log_output("dregex::replace - ERROR: null result pointer");
		return 0;
	}
	
	if (pattern.size() != replacement.size()) {
		debug_log_output("dregex::replace - ERROR: pattern and replacement size mismatch");
		*result = text;
		return 1;
	}
	
	wString temptext = text;
	
	for (size_t i = 0; i < pattern.size(); i++) {
		wString extracted_pattern;
		bool icase = false;
		
		if (!extract_pattern(pattern[i], extracted_pattern, icase)) {
			debug_log_output("dregex::replace - ERROR: failed to extract pattern at index");
			*result = text;
			return 0;
		}
		
		try {
			std::regex::flag_type flags = icase ? std::regex::icase : std::regex::ECMAScript;
			std::regex re(extracted_pattern.c_str(), flags);
			
			string mytext = temptext.c_str();
			string repl = replacement[i].c_str();
			temptext = (std::regex_replace(mytext, re, repl)).c_str();
		}
		catch (const regex_error& err)
		{
			IGNORE_PARAMETER(err);
			debug_log_output("dregex::replace - ERROR: regex compilation or replacement error");
			*result = text;
			return 0;
		}
		catch (const exception& e)
		{
			IGNORE_PARAMETER(e);
			debug_log_output("dregex::replace - ERROR: unexpected exception occurred");
			*result = text;
			return 0;
		}
	}
	
	*result = temptext;
	return 1;
}
#if 0
// ------------------------------------------------------------
// split 本体
int dregex::split2(vector<wString>* result, const wString text, regex re, const bool global)
{
	//regmatch mat[2];          //BCCでは２以上でないと正常に動作しない
	char* sp = (char*)text.c_str();
	size_t cnt = 0;
	result->clear();
	char tmp[1024] = {};
	do {
		//if (regexec(&re, (char*)(sp+cnt), 2, mat, 0) == REG_NOMATCH) {
		//    break;
		//}
		text.copy(tmp, mat[0].rm_so, cnt);
		*(tmp + mat[0].rm_so) = '\0';
		result->push_back(tmp);
		cnt += mat[0].rm_eo;
	} while (global);
	// 最終要素
	if (cnt < text.size()) {
		text.copy(tmp, text.size() - cnt, cnt);
		*(tmp + text.size() - cnt) = '\0';
		result->push_back(tmp);
	}
	if (cnt == 0) return 1; // split する事がなかった
	return 0;
}

// split 事前コンパイル・インスタンス不要版(REG_NOSUBは無効)
int dregex::split(vector<wString>* result, const wString text, const wString pattern, const bool global)
{
	regex re(pattern.c_str());
	//if (regcomp(&re, (char*)pattern.c_str(), cflags&~REG_NOSUB)){
	//    return 1; // syntax error.
	//}
	int res = dregex::split2(result, text, re, global);
	//regfree(&re);
	return res;
}
#endif
