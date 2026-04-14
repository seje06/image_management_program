#pragma once

#include "Models.h"

/*
    ------------------------------------------------------------------------
    Operations.h
    ------------------------------------------------------------------------
    서버의 /api/v1/process 에 전달할 operation을
    C++ 코드에서 쉽게 만들기 위한 helper 함수 선언부

    목적:
    - 사용자가 JSON 문자열을 직접 조립하지 않도록 하기
    - operation 이름(type)과 옵션(options)을 안전하게 구성하도록 돕기
    - 서버에서 지원하는 연산 목록을 SDK에서 동일하게 제공하기

    사용 예:
        request.operations.push_back(image_api::MakeGrayscale());
        request.operations.push_back(image_api::MakeBlur(9, 2.0));
        request.operations.push_back(image_api::MakeResize(512, 512));
*/

namespace image_api
{
    /*
        --------------------------------------------------------------------
        기본 전처리 / 색상 변환
        --------------------------------------------------------------------
    */

    // grayscale
    // 컬러 이미지를 흑백 이미지로 변환
    Operation MakeGrayscale();

    // invert
    // 이미지 색상 반전
    Operation MakeInvert();

    /*
        --------------------------------------------------------------------
        블러 / 필터
        --------------------------------------------------------------------
    */

    // blur
    // 가우시안 블러 적용
    // ksize : 커널 크기
    // sigma : 표준편차
    Operation MakeBlur(int ksize, double sigma);

    // median_blur
    // 중간값 블러 적용
    // salt-and-pepper 노이즈 제거 등에 사용
    Operation MakeMedianBlur(int ksize);

    // bilateral_filter
    // 엣지는 유지하면서 노이즈를 줄이는 양방향 필터
    Operation MakeBilateralFilter(int d, double sigmaColor, double sigmaSpace);

    // sharpen
    // 샤프닝 필터 적용
    Operation MakeSharpen();

    /*
        --------------------------------------------------------------------
        엣지 / 이진화
        --------------------------------------------------------------------
    */

    // canny_edge
    // 캐니 엣지 검출
    Operation MakeCannyEdge(double threshold1, double threshold2);

    // threshold
    // 이진화 처리
    // thresholdType 예:
    //   0 -> THRESH_BINARY
    //   1 -> THRESH_BINARY_INV
    //   2 -> THRESH_TRUNC
    //   3 -> THRESH_TOZERO
    //   4 -> THRESH_TOZERO_INV
    Operation MakeThreshold(double thresh, double maxValue, int thresholdType = 0);

    /*
        --------------------------------------------------------------------
        기하 변환
        --------------------------------------------------------------------
    */

    // resize
    // 이미지 크기 변경
    // interpolation 예:
    //   0 -> INTER_NEAREST
    //   1 -> INTER_LINEAR
    //   2 -> INTER_CUBIC
    //   3 -> INTER_AREA
    Operation MakeResize(int width, int height, int interpolation = 1);

    // crop
    // 이미지 일부 영역 자르기
    Operation MakeCrop(int x, int y, int width, int height);

    // flip
    // mode:
    //   1  -> 좌우 반전
    //   0  -> 상하 반전
    //  -1  -> 좌우/상하 모두 반전
    Operation MakeFlip(int mode);

    // rotate
    // angle:
    //   90 / 180 / 270
    Operation MakeRotate(int angle);

    /*
        --------------------------------------------------------------------
        밝기 / 대비
        --------------------------------------------------------------------
    */

    // brightness_contrast
    // alpha : 대비 계수
    // beta  : 밝기 보정값
    Operation MakeBrightnessContrast(double alpha, double beta);

    /*
        --------------------------------------------------------------------
        형태학 연산
        --------------------------------------------------------------------
    */

    // erode
    // 침식 연산
    Operation MakeErode(int ksize = 3, int iterations = 1);

    // dilate
    // 팽창 연산
    Operation MakeDilate(int ksize = 3, int iterations = 1);
}