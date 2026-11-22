#include "model.h"

bool User::check_password(const std::string& pwd) const {
    return acc.pwd == pwd;              // 直接明文比对
}

void User::set_password(const std::string& pwd) {
    acc.pwd = pwd;                      // 直接赋值
}

void User::add_login_record(const std::string& time) {
    acc.login_history.push_back(time);
    if (acc.login_history.size() > 50)
        acc.login_history.erase(acc.login_history.begin());
}