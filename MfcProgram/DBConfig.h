#pragma once

#include <string>

/*
    ------------------------------------------------------------------------
    DbConfig.h
    ------------------------------------------------------------------------
    DB 접속 정보 정의

    참고:
    - Docker MySQL을 로컬 3306으로 publish 했다는 전제
    - 실제 계정/비밀번호/DB명은 네 환경에 맞게 수정
*/

struct DbConfig
{
    std::string host = "localhost";
    int port = 3306;
    std::string user = "cv_user";
    std::string password = "1234";
    std::string database = "ImageManagement";
};