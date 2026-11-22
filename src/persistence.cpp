#include "GradeSystem.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

using nlohmann::json;

// ---------- 各结构序列化 ----------
void to_json(json& j, const grade_ele& g) {
    j = json{{"subject", g.subject}, {"score", g.score}};
}
void from_json(const json& j, grade_ele& g) {
    g.subject = j.value("subject", "");
    g.score   = j.value("score", 0.0);
}

void to_json(json& j, const exam_record& e) {
    j = json{{"exam_name", e.exam_name}, {"date", e.date}, {"grades", e.grades}};
}
void from_json(const json& j, exam_record& e) {
    e.exam_name = j.value("exam_name", "");
    e.date      = j.value("date", "");
    e.grades    = j.value("grades", std::vector<grade_ele>{});
}

void to_json(json& j, const account_info& a) {
    j = json{{"id", a.id}, {"name", a.name}, {"pwd_hash", a.pwd_hash},
             {"salt", a.salt}, {"disabled", a.disabled},
             {"login_history", a.login_history}};
}
void from_json(const json& j, account_info& a) {
    a.id            = j.value("id", "");
    a.name          = j.value("name", "");
    a.pwd_hash      = j.value("pwd_hash", "");
    a.salt          = j.value("salt", "");
    a.disabled      = j.value("disabled", false);
    a.login_history = j.value("login_history", std::vector<std::string>{});
}

void to_json(json& j, const class_info& c) {
    j = json{{"class_id", c.class_id}, {"class_name", c.class_name},
             {"grade", c.grade}, {"head_teacher_id", c.head_teacher_id}};
}
void from_json(const json& j, class_info& c) {
    c.class_id        = j.value("class_id", "");
    c.class_name      = j.value("class_name", "");
    c.grade           = j.value("grade", "");
    c.head_teacher_id = j.value("head_teacher_id", "");
}

void to_json(json& j, const log_entry& l) {
    j = json{{"time", l.time}, {"user_id", l.user_id}, {"role", l.role},
             {"action", l.action}, {"detail", l.detail}};
}
void from_json(const json& j, log_entry& l) {
    l.time    = j.value("time", "");
    l.user_id = j.value("user_id", "");
    l.role    = j.value("role", "");
    l.action  = j.value("action", "");
    l.detail  = j.value("detail", "");
}

// 三类用户分别序列化（分文件存储，避免多态混合）
void to_json(json& j, const Student& s) {
    j = json{{"acc", s.acc}, {"gender", s.gender}, {"grade", s.grade},
             {"class_id", s.class_id}, {"phone", s.phone}, {"grades", s.grades}};
}
void from_json(const json& j, Student& s) {
    s.acc      = j.value("acc", account_info{});
    s.gender   = j.value("gender", "");
    s.grade    = j.value("grade", "");
    s.class_id = j.value("class_id", "");
    s.phone    = j.value("phone", "");
    s.grades   = j.value("grades", std::vector<exam_record>{});
}

void to_json(json& j, const Teacher& t) {
    j = json{{"acc", t.acc}, {"teach_class_ids", t.teach_class_ids},
             {"subjects", t.subjects}};
}
void from_json(const json& j, Teacher& t) {
    t.acc             = j.value("acc", account_info{});
    t.teach_class_ids = j.value("teach_class_ids", std::vector<std::string>{});
    t.subjects        = j.value("subjects", std::vector<std::string>{});
}

void to_json(json& j, const Admin& a) { j = json{{"acc", a.acc}}; }
void from_json(const json& j, Admin& a) { a.acc = j.value("acc", account_info{}); }

// ---------- load ----------
template <typename T>
static bool read_user_file(const std::string& f, std::vector<std::unique_ptr<T>>& out,
                           std::vector<T>& raw) {
    std::ifstream in(f);
    if (!in) return true;                  // 文件不存在 = 空数据，不算失败
    try {
        json j; in >> j;
        raw = j.get<std::vector<T>>();
        out.clear();
        for (auto& r : raw) out.push_back(std::make_unique<T>(std::move(r)));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[load] " << f << " 解析失败: " << e.what() << "\n";
        return false;
    }
}

bool GradeSystem::load_students(const std::string& f) {
    std::vector<Student> raw;
    return read_user_file<Student>(f, students_, raw);
}
bool GradeSystem::load_teachers(const std::string& f) {
    std::vector<Teacher> raw;
    return read_user_file<Teacher>(f, teachers_, raw);
}
bool GradeSystem::load_admins(const std::string& f) {
    std::vector<Admin> raw;
    return read_user_file<Admin>(f, admins_, raw);
}

bool GradeSystem::load(const std::string& dir) {
    data_dir_ = dir;
    students_.clear(); teachers_.clear(); admins_.clear();
    classes_.clear(); subjects_.clear(); logs_.clear();

    bool ok = true;
    ok &= load_students(dir + "/students.json");
    ok &= load_teachers(dir + "/teachers.json");
    ok &= load_admins  (dir + "/admins.json");

    // 班级 / 科目 / 日志
    try {
        std::ifstream cf(dir + "/classes.json");
        if (cf) { json j; cf >> j; classes_ = j.get<std::vector<class_info>>(); }
        std::ifstream sf(dir + "/subjects.json");
        if (sf) { json j; sf >> j; subjects_ = j.get<std::vector<std::string>>(); }
        std::ifstream lf(dir + "/logs.json");
        if (lf) { json j; lf >> j; logs_ = j.get<std::vector<log_entry>>(); }
    } catch (const std::exception& e) {
        std::cerr << "[load] 附属文件解析失败: " << e.what() << "\n";
        ok = false;
    }
    rebuild_index();
    return ok;
}

// ---------- save ----------
bool GradeSystem::save() const {
    try {
        std::filesystem::create_directories(data_dir_);

        json js = json::array();
        for (auto& p : students_) js.push_back(*p);
        std::ofstream(data_dir_ + "/students.json") << js.dump(2);

        json jt = json::array();
        for (auto& p : teachers_) jt.push_back(*p);
        std::ofstream(data_dir_ + "/teachers.json") << jt.dump(2);

        json ja = json::array();
        for (auto& p : admins_) ja.push_back(*p);
        std::ofstream(data_dir_ + "/admins.json") << ja.dump(2);

        std::ofstream(data_dir_ + "/classes.json")  << json(classes_).dump(2);
        std::ofstream(data_dir_ + "/subjects.json") << json(subjects_).dump(2);
        std::ofstream(data_dir_ + "/logs.json")     << json(logs_).dump(2);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[save] 失败: " << e.what() << "\n";
        return false;
    }
}