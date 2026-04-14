#include "pch.h"
#include "ImageApiService.h"
#include "StringUtil.h"

#include "ImageApiClient.h"
#include "Operations.h"

#include <sstream>

/*
    ------------------------------------------------------------------------
    ImageApiService.cpp
    ------------------------------------------------------------------------
    cv api C++ SDK를 사용해서 서버에 이미지 처리 요청 전송
*/

ImageApiService::ImageApiService()
{
}

ImageApiService::~ImageApiService()
{
}

bool ImageApiService::ProcessImageFromUrl(
    const CString& baseUrl,
    const CString& imageUrl,
    const ProcessOptions& options,
    std::vector<unsigned char>& outImageBytes,
    CString& outOperationSummary,
    CString& outOutputFormat,
    CString& outErrorMessage)
{
    outImageBytes.clear();
    outOperationSummary.Empty();
    outOutputFormat.Empty();
    outErrorMessage.Empty();

    try
    {
        // SDK 클라이언트 생성
        image_api::ImageApiClient client(StringUtil::CStringToUtf8(baseUrl));

        // 요청 구성
        image_api::ProcessImageRequest request;
        request.imageUrl = StringUtil::CStringToUtf8(imageUrl);
        request.outputFormat = options.outputFormat;

        std::ostringstream summary;

        bool hasAnyOperation = false;

        // grayscale
        if (options.grayscale)
        {
            request.operations.push_back(image_api::MakeGrayscale());

            if (hasAnyOperation) summary << ", ";
            summary << "grayscale";
            hasAnyOperation = true;
        }

        // blur
        if (options.blur)
        {
            request.operations.push_back(image_api::MakeBlur(options.blurKsize, 2.0));

            if (hasAnyOperation) summary << ", ";
            summary << "blur(ksize=" << options.blurKsize << ")";
            hasAnyOperation = true;
        }

        // resize
        if (options.resize)
        {
            request.operations.push_back(image_api::MakeResize(options.width, options.height));

            if (hasAnyOperation) summary << ", ";
            summary << "resize(" << options.width << "x" << options.height << ")";
            hasAnyOperation = true;
        }

        // 아무 옵션도 없으면 처리 안 함
        if (!hasAnyOperation)
        {
            outErrorMessage = L"처리 옵션을 하나 이상 선택해야 함.";
            return false;
        }

        image_api::ProcessImageResponse response;
        image_api::ApiResult result = client.ProcessImage(request, response);

        if (!result.success)
        {
            outErrorMessage = StringUtil::Utf8ToCString(result.message);
            return false;
        }

        outImageBytes = response.imageBytes;
        outOperationSummary = StringUtil::Utf8ToCString(summary.str());
        outOutputFormat = StringUtil::Utf8ToCString(request.outputFormat);

        return true;
    }
    catch (const std::exception& ex)
    {
        outErrorMessage = StringUtil::Utf8ToCString(ex.what());
        return false;
    }
}