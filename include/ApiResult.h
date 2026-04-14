#pragma once

#include <string>

namespace image_api
{
    /*
        --------------------------------------------------------------------
        ApiResult
        --------------------------------------------------------------------
        SDK 함수의 성공/실패 여부와 메시지를 담는 가장 기본적인 결과 구조체이다.

        왜 이런 구조를 쓰는가?
        - C++에서는 예외 기반으로만 흐름을 제어하지 않는 경우가 많다.
        - 특히 SDK 성격의 라이브러리는 사용자가 "실패 원인 문자열"을 쉽게
          받아볼 수 있도록 명시적인 결과 구조를 두는 편이 편리하다.

        필드 설명:
        - success : 함수 수행 성공 여부
        - message : 성공/실패에 대한 부가 설명
    */
    struct ApiResult
    {
        bool success = false;
        std::string message;

        // 성공 결과를 만들기 위한 편의 함수
        static ApiResult Ok(const std::string& message = "ok")
        {
            return { true, message };
        }

        // 실패 결과를 만들기 위한 편의 함수
        static ApiResult Fail(const std::string& message)
        {
            return { false, message };
        }
    };
}
