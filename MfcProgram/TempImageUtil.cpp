#include "pch.h"
#include "TempImageUtil.h"

#include <afx.h>

/*
    ------------------------------------------------------------------------
    TempImageUtil.cpp
    ------------------------------------------------------------------------
    이미지 바이트를 TEMP 폴더에 저장
*/

namespace TempImageUtil
{
    bool SaveBytesToTempFile(
        const std::vector<unsigned char>& imageBytes,
        const CString& outputFormat,
        CString& outTempFilePath)
    {
        outTempFilePath.Empty();

        if (imageBytes.empty())
        {
            return false;
        }

        TCHAR tempPath[MAX_PATH] = { 0 };
        DWORD length = ::GetTempPath(MAX_PATH, tempPath);

        if (length == 0 || length > MAX_PATH)
        {
            return false;
        }

        CString extension = L".png";
        if (outputFormat.CompareNoCase(L"jpg") == 0 || outputFormat.CompareNoCase(L"jpeg") == 0)
        {
            extension = L".jpg";
        }

        CString filePath;
        filePath.Format(L"%sresult_preview_%llu%s",
            tempPath,
            ::GetTickCount64(),
            extension.GetString());

        try
        {
            CFile file(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
            file.Write(imageBytes.data(), static_cast<UINT>(imageBytes.size()));
            file.Close();

            outTempFilePath = filePath;
            return true;
        }
        catch (CFileException* ex)
        {
            ex->Delete();
            return false;
        }
    }
}