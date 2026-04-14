#pragma once

#include <string>
#include <vector>

#include <curl/curl.h>

namespace image_api::internal
{
    /*
        --------------------------------------------------------------------
        HttpResponse
        --------------------------------------------------------------------
        낮은 수준의 HTTP 응답 데이터를 담는 내부 구조체이다.

        이 구조체는 SDK 외부 사용자에게 공개되지 않는다.
        SDK 내부에서만 사용한다.

        필드 설명:
        - statusCode  : HTTP 상태 코드 (200, 400, 500 등)
        - contentType : 응답의 Content-Type 헤더 값
        - imageWidth  : X-Image-Width 헤더 값
        - imageHeight : X-Image-Height 헤더 값
        - body        : 응답 본문 바이트 전체
    */
    struct HttpResponse
    {
        long statusCode = 0;
        std::string contentType;
        int imageWidth = 0;
        int imageHeight = 0;
        std::vector<unsigned char> body;
    };

    // libcurl 전역 초기화를 안전하게 1회 수행한다.
    bool EnsureCurlGlobalInit(std::string& outErrorMessage);

    // HTTP GET 요청 수행
    bool HttpGet(const std::string& url, HttpResponse& outResponse, std::string& outErrorMessage);

    // HTTP POST 요청 수행
    bool HttpPostJson(const std::string& url,
                      const std::string& jsonBody,
                      HttpResponse& outResponse,
                      std::string& outErrorMessage);

    // JSON 에러 응답 본문에서 사용자 친화적인 메시지를 뽑아낸다.
    std::string ParseServerErrorMessage(const HttpResponse& response);
}
