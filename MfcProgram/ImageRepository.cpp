#include "pch.h"
#include "ImageRepository.h"

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <sstream>
#include <iterator>

/*
    ------------------------------------------------------------------------
    ImageRepository.cpp
    ------------------------------------------------------------------------
    processed_images 테이블 CRUD 구현

    저장 방식:
    - image_data 를 LONGBLOB으로 저장
    - 목록 조회 시에는 BLOB을 제외한 메타데이터만 조회
    - 상세 조회 시에는 BLOB까지 함께 읽음
*/

ImageRepository::ImageRepository()
{
}

ImageRepository::~ImageRepository()
{
}

bool ImageRepository::Connect(const DbConfig& config, std::string& errorMessage)
{
    try
    {
        OutputDebugString(L"[DB] before get_mysql_driver_instance\n");
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        OutputDebugString(L"[DB] after get_mysql_driver_instance\n");

        std::string url = "tcp://" + config.host + ":" + std::to_string(config.port);

        OutputDebugString(L"[DB] before driver->connect\n");
        _connection.reset(driver->connect(url, config.user, config.password));
        OutputDebugString(L"[DB] after driver->connect\n");

        OutputDebugString(L"[DB] before setSchema\n");
        _connection->setSchema(config.database);
        OutputDebugString(L"[DB] after setSchema\n");

        return true;
    }
    catch (const sql::SQLException& ex)
    {
        errorMessage =
            std::string("what: ") + ex.what() +
            "\nSQLState: " + ex.getSQLState() +
            "\nErrorCode: " + std::to_string(ex.getErrorCode());
        return false;
    }
    catch (const std::exception& ex)
    {
        errorMessage = std::string("std::exception: ") + ex.what();
        return false;
    }
}

bool ImageRepository::IsConnected() const
{
    return _connection != nullptr;
}

bool ImageRepository::EnsureTable(std::string& errorMessage)
{
    if (!IsConnected())
    {
        errorMessage = "DB is not connected";
        return false;
    }

    try
    {
        std::unique_ptr<sql::Statement> statement(_connection->createStatement());

        statement->execute(
            "CREATE TABLE IF NOT EXISTS processed_images ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "title VARCHAR(100) NOT NULL,"
            "image_url VARCHAR(500) NOT NULL,"
            "operation_summary TEXT NOT NULL,"
            "output_format VARCHAR(20) NOT NULL,"
            "image_data LONGBLOB NOT NULL,"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
            ")"
        );

        return true;
    }
    catch (const sql::SQLException& ex)
    {
        errorMessage = ex.what();
        return false;
    }
}

bool ImageRepository::InsertProcessedImage(const ProcessedImageRecord& record, std::string& errorMessage)
{
    if (!IsConnected())
    {
        errorMessage = "DB is not connected";
        return false;
    }

    try
    {
        std::unique_ptr<sql::PreparedStatement> statement(
            _connection->prepareStatement(
                "INSERT INTO processed_images "
                "(title, image_url, operation_summary, output_format, image_data) "
                "VALUES (?, ?, ?, ?, ?)"
            )
        );

        statement->setString(1, record.title);
        statement->setString(2, record.imageUrl);
        statement->setString(3, record.operationSummary);
        statement->setString(4, record.outputFormat);

        /*
            ------------------------------------------------------------
            LONGBLOB 저장
            ------------------------------------------------------------
            setBlob은 std::istream* 를 받으므로,
            vector<unsigned char>를 string으로 감싼 뒤 istringstream으로 전달
        */
        std::string binary(
            reinterpret_cast<const char*>(record.imageData.data()),
            record.imageData.size()
        );

        std::istringstream blobStream(binary, std::ios::binary);
        statement->setBlob(5, &blobStream);

        statement->execute();
        return true;
    }
    catch (const sql::SQLException& ex)
    {
        errorMessage = ex.what();
        return false;
    }
}

bool ImageRepository::GetAllProcessedImages(std::vector<ProcessedImageRecord>& outRecords, std::string& errorMessage)
{
    outRecords.clear();

    if (!IsConnected())
    {
        errorMessage = "DB is not connected";
        return false;
    }

    try
    {
        std::unique_ptr<sql::PreparedStatement> statement(
            _connection->prepareStatement(
                "SELECT id, title, image_url, operation_summary, output_format, created_at "
                "FROM processed_images "
                "ORDER BY id DESC"
            )
        );

        std::unique_ptr<sql::ResultSet> result(statement->executeQuery());

        while (result->next())
        {
            ProcessedImageRecord record;
            record.id = result->getInt("id");
            record.title = result->getString("title");
            record.imageUrl = result->getString("image_url");
            record.operationSummary = result->getString("operation_summary");
            record.outputFormat = result->getString("output_format");
            record.createdAt = result->getString("created_at");

            outRecords.push_back(record);
        }

        return true;
    }
    catch (const sql::SQLException& ex)
    {
        errorMessage = ex.what();
        return false;
    }
}

bool ImageRepository::GetProcessedImageById(int id, ProcessedImageRecord& outRecord, std::string& errorMessage)
{
    if (!IsConnected())
    {
        errorMessage = "DB is not connected";
        return false;
    }

    try
    {
        std::unique_ptr<sql::PreparedStatement> statement(
            _connection->prepareStatement(
                "SELECT id, title, image_url, operation_summary, output_format, image_data, created_at "
                "FROM processed_images "
                "WHERE id = ?"
            )
        );

        statement->setInt(1, id);

        std::unique_ptr<sql::ResultSet> result(statement->executeQuery());

        if (!result->next())
        {
            errorMessage = "record not found";
            return false;
        }

        outRecord.id = result->getInt("id");
        outRecord.title = result->getString("title");
        outRecord.imageUrl = result->getString("image_url");
        outRecord.operationSummary = result->getString("operation_summary");
        outRecord.outputFormat = result->getString("output_format");
        outRecord.createdAt = result->getString("created_at");

        /*
            ------------------------------------------------------------
            BLOB 읽기
            ------------------------------------------------------------
            getBlob 결과를 stream으로 받아 vector<unsigned char>로 복원
        */
        std::unique_ptr<std::istream> blobStream(result->getBlob("image_data"));

        if (blobStream)
        {
            std::vector<char> temp(
                (std::istreambuf_iterator<char>(*blobStream)),
                std::istreambuf_iterator<char>()
            );

            outRecord.imageData.assign(temp.begin(), temp.end());
        }

        return true;
    }
    catch (const sql::SQLException& ex)
    {
        errorMessage = ex.what();
        return false;
    }
}

bool ImageRepository::DeleteProcessedImage(int id, std::string& errorMessage)
{
    if (!IsConnected())
    {
        errorMessage = "DB is not connected";
        return false;
    }

    try
    {
        std::unique_ptr<sql::PreparedStatement> statement(
            _connection->prepareStatement(
                "DELETE FROM processed_images WHERE id = ?"
            )
        );

        statement->setInt(1, id);
        statement->execute();

        return true;
    }
    catch (const sql::SQLException& ex)
    {
        errorMessage = ex.what();
        return false;
    }
}