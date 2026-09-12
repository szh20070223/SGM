#ifndef G_H
#define G_H
#include<vector>
#include<string>
struct grade_ele {
    std::string subject;
    double score;
};

struct exam_record {
    std::string date;      // 建议 "2024-06-01" 统一格式，或用 int 20240601 便于排序
    std::string exam_name; // 期中/期末/一模...
    std::vector<grade_ele> grades;
};

struct stu_info {
    std::string stu_id;    // 全局唯一，作为主键
    std::string name;
    std::string gender;
    std::string phone;
    std::string class_id;  // 替代嵌套
    std::vector<exam_record> grades;
};

struct class_info {
    std::string class_id;
    std::string class_name;
    std::string head_teacher_id;
    // 不存学生列表，靠 class_id 关联即可
};

// 三张"表"
extern std::vector<stu_info>   students;
extern std::vector<class_info> classes;
#endif