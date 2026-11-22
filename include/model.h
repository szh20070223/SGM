#ifndef MODEL_H
#define MODEL_H
#include <string>
#include <vector>
#include <cstddef>

enum class Role { Student = 0, Teacher = 1, Admin = 2 };

struct grade_ele {
    std::string subject;
    double score = 0.0;
};

struct exam_record {
    std::string exam_name;             // "期中"/"期末"
    std::string date;                  // "2026-11-20"
    std::vector<grade_ele> grades;
};

// 账号数据单独成 struct，方便序列化直接读写，不破坏继承
struct account_info {
    std::string id;
    std::string name;
    std::string pwd_hash;
    std::string salt;
    bool disabled = false;
    std::vector<std::string> login_history;
};

class User {
public:
    virtual ~User() = default;
    account_info acc;                         // 账号数据（public，序列化友好）

    virtual Role role() const = 0;
    virtual std::string role_name() const = 0;

    bool check_password(const std::string& pwd) const;
    void set_password(const std::string& pwd);          // 生成 salt + hash
    void add_login_record(const std::string& time);
};

class Student : public User {
public:
    std::string gender, grade, class_id, phone;
    std::vector<exam_record> grades;

    Role role() const override { return Role::Student; }
    std::string role_name() const override { return "学生"; }
};

class Teacher : public User {
public:
    std::vector<std::string> teach_class_ids;   // 所辖班级
    std::vector<std::string> subjects;          // 任教科目

    Role role() const override { return Role::Teacher; }
    std::string role_name() const override { return "教师"; }
};

class Admin : public User {
public:
    Role role() const override { return Role::Admin; }
    std::string role_name() const override { return "管理员"; }
};

struct class_info {
    std::string class_id, class_name, grade, head_teacher_id;
};

struct log_entry {
    std::string time, user_id, role, action, detail;
};

#endif