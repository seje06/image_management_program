#pragma once

#include "AppModels.h"

#include <afxstr.h>
#include <string>
#include <vector>

/*
    ------------------------------------------------------------------------
    ImageApiService.h
    ------------------------------------------------------------------------
    cv api C++ SDK를 감싸는 서비스 클래스

    역할:
    - UI 입력값을 SDK 요청으로 변환
    - 처리 결과 이미지 바이트를 받아 반환
    - 사람이 읽기 쉬운 operation summary도 같이 생성
*/

class ImageApiService
{
public:
    ImageApiService();
    ~ImageApiService();

public:
    bool ProcessImageFromUrl(
        const CString& baseUrl,
        const CString& imageUrl,
        const ProcessOptions& options,
        std::vector<unsigned char>& outImageBytes,
        CString& outOperationSummary,
        CString& outOutputFormat,
        CString& outErrorMessage);
};