#pragma once

#include "AppModels.h"
#include "DbConfig.h"

#include <memory>
#include <string>
#include <vector>

namespace sql
{
    class Connection;
}

/*
    ------------------------------------------------------------------------
    ImageRepository.h
    ------------------------------------------------------------------------
    MySQL Connector/C++ legacy JDBC API를 사용해서
    processed_images 테이블 CRUD를 수행하는 클래스
*/

class ImageRepository
{
public:
    ImageRepository();
    ~ImageRepository();

public:
    // DB 연결
    bool Connect(const DbConfig& config, std::string& errorMessage);

    // 테이블 생성 보장
    bool EnsureTable(std::string& errorMessage);

    // 처리 결과 insert
    bool InsertProcessedImage(const ProcessedImageRecord& record, std::string& errorMessage);

    // 리스트 조회 (이미지 데이터 제외)
    bool GetAllProcessedImages(std::vector<ProcessedImageRecord>& outRecords, std::string& errorMessage);

    // 단일 조회 (이미지 데이터 포함)
    bool GetProcessedImageById(int id, ProcessedImageRecord& outRecord, std::string& errorMessage);

    // 삭제
    bool DeleteProcessedImage(int id, std::string& errorMessage);

private:
    bool IsConnected() const;

private:
    std::unique_ptr<sql::Connection> _connection;
};