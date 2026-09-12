#ifndef GL_INFO
#define GL_INFO
#include <vector>
#include <string>
#include<unordered_map>

struct teacher_info {
    std::string t_id, name, password;
    std::vector<std::string> teach_class_ids;  // 教哪些班
    std::vector<std::string> subjects;         // 教哪些科
};

struct admin_info {
    std::string a_id, name, password;
};

extern std::vector<teacher_info> teachers;
extern std::vector<admin_info>   admins;
extern std::unordered_map<std::string, size_t> teacher_index;

#endif 
