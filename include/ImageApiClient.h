#pragma once

#include <string>

#include "ApiResult.h"
#include "Models.h"

namespace image_api
{
    /*
        --------------------------------------------------------------------
        ImageApiClient
        --------------------------------------------------------------------
        이미지 처리 서버와 통신하는 SDK의 핵심 클라이언트 클래스이다.

        책임:
        - 서버 기본 주소(base URL) 보관
        - /health 요청
        - /api/v1/process 요청

        의도적으로 분리한 것:
        - HTTP 통신의 낮은 수준 구현은 src/HttpUtils.* 에 숨김
        - Operation 생성 helper는 Operations.* 로 분리
        - 요청/응답 데이터 모델은 Models.h 로 분리

        이렇게 나누는 이유:
        - 사용자는 이 헤더만 보고도 SDK 사용법을 이해할 수 있어야 한다.
        - 내부 구현(libcurl 세부 처리)은 공개 API에서 숨기는 편이 낫다.
    */
    class ImageApiClient
    {
    public:
        // 기본 생성자. baseUrl은 나중에 SetBaseUrl로 지정할 수 있다.
        ImageApiClient() = default;

        // 기본 서버 주소를 받는 생성자
        explicit ImageApiClient(const std::string& baseUrl);

        // 서버 기본 주소 설정
        void SetBaseUrl(const std::string& baseUrl);

        // 현재 설정된 서버 기본 주소 반환
        const std::string& GetBaseUrl() const;

        /*
            ----------------------------------------------------------------
            HealthCheck
            ----------------------------------------------------------------
            서버 /health 엔드포인트를 호출하여 서버 상태를 확인한다.

            성공 시:
            - result.success == true
            - outResponse.status / outResponse.message 에 값이 채워짐

            실패 시:
            - result.success == false
            - result.message 에 실패 원인이 들어감
        */
        ApiResult HealthCheck(HealthCheckResponse& outResponse) const;

        /*
            ----------------------------------------------------------------
            ProcessImage
            ----------------------------------------------------------------
            서버 /api/v1/process 엔드포인트를 호출하여 이미지를 처리한다.

            성공 시:
            - result.success == true
            - outResponse.imageBytes 등에 결과가 채워짐

            실패 시:
            - result.success == false
            - result.message 에 실패 원인이 들어감
        */
        ApiResult ProcessImage(const ProcessImageRequest& request, ProcessImageResponse& outResponse) const;

    private:
        // base URL 끝에 '/'가 있든 없든 안전하게 경로를 붙이기 위한 내부 유틸
        std::string BuildUrl(const std::string& path) const;

    private:
        std::string _baseUrl;
    };
}
