#pragma once

#include <afxstr.h>
#include <vector>

/*
    ------------------------------------------------------------------------
    TempImageUtil.h
    ------------------------------------------------------------------------
    BLOB 이미지 데이터를 임시 파일로 저장해서
    기존 ShowImageInControl() 재사용 가능하게 만드는 유틸리티
*/

namespace TempImageUtil
{
    bool SaveBytesToTempFile(
        const std::vector<unsigned char>& imageBytes,
        const CString& outputFormat,
        CString& outTempFilePath
    );
}