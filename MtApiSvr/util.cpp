#include "util.h"
#include <windows.h>
namespace util {
    // gbk  ×ª»» utf-8
    std::string GbkToUtf8(const char* src_str) {
        int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
        wchar_t* wstr = new wchar_t[len + 1];
        memset(wstr, 0, len + 1);
        MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
        len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
        char* str = new char[len + 1];
        memset(str, 0, len + 1);
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
        std::string strTemp = str;
        delete[] wstr;
        delete[] str;
        return strTemp;
    }

    std::string Utf8ToGbk(const std::string& utf8String) {
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, NULL, 0);
        wchar_t* wstr = new wchar_t[len + 1];
        memset(wstr, 0, len + 1);
        MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, wstr, len);
        len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
        char* gbkStr = new char[len + 1];
        memset(gbkStr, 0, len + 1);
        WideCharToMultiByte(CP_ACP, 0, wstr, -1, gbkStr, len, NULL, NULL);
        std::string gbkString = gbkStr;
        delete[] wstr;
        delete[] gbkStr;
        return gbkString;
    }
}
