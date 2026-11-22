#ifndef GRADESYSTEM_H
#define GRADESYSTEM_H
#include "model.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class GradeSystem {
public:
    GradeSystem() = default;

    // ---- 生命周期 ----
    bool load(const std::string& dir = "data");
    bool save() const;
    void rebuild_index();

    // ---- 登录 / 账号 ----
    // 成功返回对象裸指针（生命周期归本类，不转移所有权），失败 nullptr
    User* login(Role r, const std::string& id, const std::string& pwd);
    bool modify_password(Role r, const std::string& id,
                         const std::string& oldpwd, const std::string& newpwd);
    bool reset_password(Role r, const std::string& id, const std::string& newpwd);
    bool set_disabled(Role r, const std::string& id, bool disabled);
    void append_log(const std::string& user_id, const std::string& role,
                    const std::string& action, const std::string& detail);

    // ---- 查询（B/C 只用这些，不碰私有 vector）----
    Student* find_student(const std::string& id);
    Teacher* find_teacher(const std::string& id);
    Admin*   find_admin(const std::string& id);
    const std::vector<std::unique_ptr<Student>>& all_students() const { return students_; }
    const std::vector<std::unique_ptr<Teacher>>& all_teachers() const { return teachers_; }
    std::vector<Student*> students_of_class(const std::string& class_id) const;
    const std::vector<std::string>& subjects() const { return subjects_; }
    const std::vector<class_info>& classes() const { return classes_; }

    // ---- 写操作（内部自动 save）----
    bool add_student(std::unique_ptr<Student> s);
    bool add_teacher(std::unique_ptr<Teacher> t);
    bool add_admin(std::unique_ptr<Admin> a);
    bool remove_student(const std::string& id);
    bool set_score(const std::string& stu_id, const std::string& exam_name,
                   const std::string& subject, double score);   // 0~100 校验

    const std::string& data_dir() const { return data_dir_; }

private:
    bool load_students(const std::string& f);
    bool load_teachers(const std::string& f);
    bool load_admins(const std::string& f);

    std::string data_dir_ = "data";
    std::vector<std::unique_ptr<Student>> students_;
    std::vector<std::unique_ptr<Teacher>> teachers_;
    std::vector<std::unique_ptr<Admin>>   admins_;
    std::vector<class_info>   classes_;
    std::vector<std::string>  subjects_;
    std::vector<log_entry>    logs_;

    // 索引只存下标，不进文件；load 后 rebuild
    std::unordered_map<std::string, size_t> stu_index_;
    std::unordered_map<std::string, size_t> teacher_index_;
    std::unordered_map<std::string, size_t> admin_index_;
};

#endif
