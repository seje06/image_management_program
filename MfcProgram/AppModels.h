#pragma once

#include <string>
#include <vector>

/*
    ------------------------------------------------------------------------
    AppModels.h
    ------------------------------------------------------------------------
    MFC UI / DB / SDK 계층이 공통으로 사용하는 데이터 구조체 정의
*/

struct ProcessOptions
{
    // grayscale 적용 여부
    bool grayscale = false;

    // blur 적용 여부 및 blur 커널 크기
    bool blur = false;
    int blurKsize = 9;

    // resize 적용 여부 및 목표 크기
    bool resize = false;
    int width = 512;
    int height = 512;

    // 출력 포맷
    // 현재 UI에는 별도 선택 컨트롤이 없으므로 기본 png 사용
    std::string outputFormat = "png";
};

struct ProcessedImageRecord
{
    int id = 0;

    // 사용자가 입력한 제목
    std::string title;

    // 서버에 전달한 원본 이미지 URL
    std::string imageUrl;

    // 처리 옵션 요약 문자열
    std::string operationSummary;

    // 출력 포맷 (png / jpg 등)
    std::string outputFormat;

    // DB에 저장할 실제 결과 이미지 바이트
    std::vector<unsigned char> imageData;

    // 저장 시각 문자열
    std::string createdAt;
};