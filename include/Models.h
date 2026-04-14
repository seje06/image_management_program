#pragma once

#include <string>
#include <vector>

namespace image_api
{
    /*
        --------------------------------------------------------------------
        Operation
        --------------------------------------------------------------------
        서버의 operations 배열 안에 들어갈 "작업 하나"를 표현하는 구조체이다.

        현재 서버는 아래와 같은 형태를 기대한다.
        {
          "type": "blur",
          "options": {
            "ksize": 9,
            "sigma": 2.0
          }
        }

        여기서 SDK는 사용자가 C++ 코드로 작업을 만들기 쉽게 하기 위해,
        type과 optionsJson 문자열을 들고 있다.

        optionsJson을 문자열로 둔 이유:
        - 공개 헤더에서 nlohmann::json 의존성을 강제하지 않기 위함
        - 사용자는 MakeBlur, MakeResize 같은 helper 함수만 쓰면 됨

        필드 설명:
        - type        : 작업 이름 (grayscale, blur, canny_edge, resize 등)
        - optionsJson : options 객체를 JSON 문자열로 담은 값
                        예: "{\"ksize\":9,\"sigma\":2.0}"
                        옵션이 없으면 빈 문자열을 쓴다.
    */
    struct Operation
    {
        std::string type;
        std::string optionsJson;
    };

    /*
        --------------------------------------------------------------------
        ProcessImageRequest
        --------------------------------------------------------------------
        /api/v1/process 요청에 들어갈 데이터를 담는 구조체이다.

        서버 기준 요청 예시는 다음과 같다.
        {
          "image_url": "https://...",
          "operations": [ ... ],
          "output_format": "jpg"
        }

        필드 설명:
        - imageUrl     : 처리할 원본 이미지 URL
        - operations   : 순서대로 적용할 작업 목록
        - outputFormat : 결과 포맷 (jpg / jpeg / png)
    */
    struct ProcessImageRequest
    {
        std::string imageUrl;
        std::vector<Operation> operations;
        std::string outputFormat = "jpg";
    };

    /*
        --------------------------------------------------------------------
        ProcessImageResponse
        --------------------------------------------------------------------
        /api/v1/process 성공 응답에서 SDK가 사용자에게 돌려줄 데이터 구조체이다.

        서버는 결과 이미지를 "바이너리 바이트"로 응답 본문에 넣고,
        Content-Type 과 X-Image-Width / X-Image-Height 헤더를 함께 보낸다.

        필드 설명:
        - imageBytes  : 결과 이미지 파일 바이트 전체
        - contentType : image/jpeg, image/png 같은 MIME 타입
        - width       : 결과 이미지 가로 크기 (헤더에서 읽음)
        - height      : 결과 이미지 세로 크기 (헤더에서 읽음)
    */
    struct ProcessImageResponse
    {
        std::vector<unsigned char> imageBytes;
        std::string contentType;
        int width = 0;
        int height = 0;
    };

    /*
        --------------------------------------------------------------------
        HealthCheckResponse
        --------------------------------------------------------------------
        /health 엔드포인트의 JSON 응답에서 중요한 값만 담는 구조체이다.

        서버 응답 예:
        {
          "status": "ok",
          "message": "server is running"
        }
    */
    struct HealthCheckResponse
    {
        std::string status;
        std::string message;
    };
}
