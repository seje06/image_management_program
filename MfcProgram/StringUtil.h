#pragma once

#include <afxstr.h>
#include <string>
#include <atlconv.h>

/*
    ------------------------------------------------------------------------
    StringUtil.h
    ------------------------------------------------------------------------
    CString과 std::string(UTF-8) 사이 변환 유틸리티
*/

namespace StringUtil
{
    inline std::string CStringToUtf8(const CString& text)
    {
        CW2A converted(text, CP_UTF8);
        return std::string(converted);
    }

    inline CString Utf8ToCString(const std::string& text)
    {
        CA2W converted(text.c_str(), CP_UTF8);
        return CString(converted);
    }
}