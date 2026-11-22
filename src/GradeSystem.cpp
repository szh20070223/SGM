#include "GradeSystem.h"
#include "utils.h"
#include <algorithm>

void GradeSystem::rebuild_index() {
    stu_index_.clear(); teacher_index_.clear(); admin_index_.clear();
    for (size_t i = 0; i < students_.size(); ++i) stu_index_[students_[i]->acc.id] = i;
    for (size_t i = 0; i < teachers_.size(); ++i) teacher_index_[teachers_[i]->acc.id] = i;
    for (size_t i = 0; i < admins_.size();   ++i) admin_index_[admins_[i]->acc.id]   = i;
}

User* GradeSystem::login(Role r, const std::string& id, const std::string& pwd) {
    User* u = nullptr;
    switch (r) {
        case Role::Student: {
            auto it = stu_index_.find(id);
            if (it != stu_index_.end()) u = students_[it->second].get();
            break;
        }
        case Role::Teacher: {
            auto it = teacher_index_.find(id);
            if (it != teacher_index_.end()) u = teachers_[it->second].get();
            break;
        }
        case Role::Admin: {
            auto it = admin_index_.find(id);
            if (it != admin_index_.end()) u = admins_[it->second].get();
            break;
        }
    }
    if (!u || u->acc.disabled || !u->check_password(pwd)) return nullptr;

    u->add_login_record(now_string());
    append_log(id, u->role_name(), "login", "登录成功");
    save();
    return u;
}

bool GradeSystem::modify_password(Role r, const std::string& id,
                                  const std::string& oldpwd, const std::string& newpwd) {
    User* u = nullptr;
    if (r == Role::Student) u = find_student(id);
    else if (r == Role::Teacher) u = find_teacher(id);
    else u = find_admin(id);
    if (!u || !u->check_password(oldpwd)) return false;
    u->set_password(newpwd);
    append_log(id, u->role_name(), "modify_password", "修改密码");
    save();
    return true;
}

bool GradeSystem::reset_password(Role r, const std::string& id, const std::string& newpwd) {
    User* u = (r == Role::Student) ? (User*)find_student(id)
            : (r == Role::Teacher) ? (User*)find_teacher(id)
            :                        (User*)find_admin(id);
    if (!u) return false;
    u->set_password(newpwd);
    append_log(id, u->role_name(), "reset_password", "管理员重置密码");
    save();
    return true;
}

bool GradeSystem::set_disabled(Role r, const std::string& id, bool disabled) {
    User* u = (r == Role::Student) ? (User*)find_student(id)
            : (r == Role::Teacher) ? (User*)find_teacher(id)
            :                        (User*)find_admin(id);
    if (!u) return false;
    u->acc.disabled = disabled;
    append_log(id, u->role_name(), "set_disabled", disabled ? "禁用" : "启用");
    save();
    return true;
}

void GradeSystem::append_log(const std::string& user_id, const std::string& role,
                             const std::string& action, const std::string& detail) {
    logs_.push_back({now_string(), user_id, role, action, detail});
}

Student* GradeSystem::find_student(const std::string& id) {
    auto it = stu_index_.find(id);
    return it == stu_index_.end() ? nullptr : students_[it->second].get();
}
Teacher* GradeSystem::find_teacher(const std::string& id) {
    auto it = teacher_index_.find(id);
    return it == teacher_index_.end() ? nullptr : teachers_[it->second].get();
}
Admin* GradeSystem::find_admin(const std::string& id) {
    auto it = admin_index_.find(id);
    return it == admin_index_.end() ? nullptr : admins_[it->second].get();
}

std::vector<Student*> GradeSystem::students_of_class(const std::string& class_id) const {
    std::vector<Student*> out;
    for (auto& p : students_)
        if (p->class_id == class_id) out.push_back(p.get());
    return out;
}

bool GradeSystem::add_student(std::unique_ptr<Student> s) {
    if (!s || s->acc.id.empty() || stu_index_.count(s->acc.id)) return false;
    students_.push_back(std::move(s));
    stu_index_[students_.back()->acc.id] = students_.size() - 1;
    save();
    return true;
}
bool GradeSystem::add_teacher(std::unique_ptr<Teacher> t) {
    if (!t || t->acc.id.empty() || teacher_index_.count(t->acc.id)) return false;
    teachers_.push_back(std::move(t));
    teacher_index_[teachers_.back()->acc.id] = teachers_.size() - 1;
    save();
    return true;
}
bool GradeSystem::add_admin(std::unique_ptr<Admin> a) {
    if (!a || a->acc.id.empty() || admin_index_.count(a->acc.id)) return false;
    admins_.push_back(std::move(a));
    admin_index_[admins_.back()->acc.id] = admins_.size() - 1;
    save();
    return true;
}

bool GradeSystem::remove_student(const std::string& id) {
    auto it = stu_index_.find(id);
    if (it == stu_index_.end()) return false;
    size_t idx = it->second, last = students_.size() - 1;
    if (idx != last) {                                   // 末尾交换法，O(1)
        std::swap(students_[idx], students_[last]);
        stu_index_[students_[idx]->acc.id] = idx;
    }
    students_.pop_back();
    stu_index_.erase(id);
    save();
    return true;
}

bool GradeSystem::set_score(const std::string& stu_id, const std::string& exam_name,
                            const std::string& subject, double score) {
    if (score < 0.0 || score > 100.0) return false;      // 合法区间校验
    Student* s = find_student(stu_id);
    if (!s) return false;

    exam_record* er = nullptr;
    for (auto& e : s->grades)
        if (e.exam_name == exam_name) { er = &e; break; }
    if (!er) {
        s->grades.push_back({exam_name, now_date(), {}});
        er = &s->grades.back();
    }
    for (auto& g : er->grades)
        if (g.subject == subject) { g.score = score; save(); return true; }
    er->grades.push_back({subject, score});
    save();
    return true;
}

