#用于学生管理系统的构建
用扁平化处理信息体系，删除了之前的多级复杂的直观信息架构
# 学生成绩管理系统（C++ 课程实践）

基于控制台的学生成绩管理系统，支持学生、教师、超级管理员三类角色。
采用面向对象设计（User 继承体系），数据以 JSON 持久化，无需数据库。

---

## 一、项目状态

| 阶段 | 内容 | 状态 |
|---|---|---|
| Phase 1 | 核心骨架（数据模型 / User 体系 / JSON 持久化 / 登录 / 索引 / 批量导入接口） 
| Phase 2 | 学生端、教师端业务菜单
| Phase 3 | 管理员端、扩展功能（TXT 导入导出、可视化） 
| Phase 4 | 联调、测试、报告

> **本提交仅包含 core 模块（Phase 1）**。程序可编译运行、可登录、可读写数据；
> 三个角色的业务菜单为占位实现（登录后提示"待实现"），不影响 Phase 1 验收。

---

## 二、构建与运行

## 三、默认账号

首次运行若无管理员账号，程序自动创建：

| 角色 | 账号 | 密码 |
|---|---|---|
| 超级管理员 | `admin` | `admin123` |

- 学生 / 教师账号由管理员创建（Phase 3）。
- 删除 `data/` 目录即可重置系统。

登录流程：启动后输入 `a`（学生）/ `b`（教师）/ `c`（管理员），再输入账号密码。

---

## 四、目录结构与分工

```
project/
├── CMakeLists.txt
├── third_party/json.hpp    # 第三方持久库
├── include/
│   ├── model.h              # [A] 数据模型 + User 继承体系（冻结）
│   ├── GradeSystem.h        # [A] 核心类接口（冻结，只加不改）
│   ├── menus.h              # 三人约定：菜单函数签名
│   └── utils.h              # [A] 时间等工具
├── src/
│   ├── GradeSystem.cpp      # [A] 登录 / 索引 / CRUD
│   ├── persistence.cpp      # [A] JSON load / save
│   ├── global_information.cpp  # [A] 用户相关实现
│   ├── utils.cpp            # [A]
│   ├── menus_stub.cpp       # [B/C] 替换为自己的菜单实现
│   └── main.cpp             # [A] 登录循环 + 角色分发
└── data/                    # 运行时自动生成（JSON）
```

| 成员 | 负责模块 | 文件边界 |
|---|---|---|
| A（本人） | core：模型 / 持久化 / 账号 / 索引 / 校验 | `include/model.h` `GradeSystem.h` `src/GradeSystem.cpp` `persistence.cpp` `global_information.cpp` `utils.cpp` `main.cpp` |

---

## 五、接口契约（改动需全组通知）

`GradeSystem.h` 冻结后：**只加方法，不改已有签名**。

### 登录 / 账号

```cpp
User* login(Role r, const std::string& id, const std::string& pwd); // 失败返回 nullptr
bool  modify_password(Role r, const std::string& id,
                      const std::string& oldpwd, const std::string& newpwd);
bool  reset_password(Role r, const std::string& id, const std::string& newpwd); // 管理员
bool  set_disabled(Role r, const std::string& id, bool disabled);               // 管理员
```

### 查询（B / C 只用这些，不碰私有成员）

```cpp
Student* find_student(const std::string& id);
Teacher* find_teacher(const std::string& id);
std::vector<Student*> students_of_class(const std::string& class_id) const;
const std::vector<std::unique_ptr<Student>>& all_students() const;
const std::vector<std::string>& subjects() const;
const std::vector<class_info>& classes() const;
```

### 写入（内部自动 `save()`，含 0~100 校验）

```cpp
bool set_score(stu_id, exam_name, subject, score);
bool add_student(std::unique_ptr<Student> s);
bool remove_student(const std::string& id);
bool import_students_batch(const std::vector<Student>& batch,
                           ImportResult& result, bool skip_bad = false);
```

### 规则

1. **不得直接访问 `GradeSystem` 私有成员**，一律走公有方法。
2. 菜单函数签名固定为 `(GradeSystem&, User&)`，内部 `dynamic_cast<Student*>` 取业务数据。
3. 改密码统一调 `modify_password`，不要自行拼 hash。
4. 批量操作调 `import_*_batch`，**不要循环调 `add_student`**（避免 N 次落盘）。
5. 修改 `GradeSystem.h` 公共接口须在群里通知，改完全组拉最新。

---

## 六、数据存储

- 主存储：`data/` 下 JSON，程序启动 `load()` 读入，写操作后立即 `save()`。
- 文件：`students.json` `teachers.json` `admins.json` `classes.json` `subjects.json` `logs.json`。
- 索引：`unordered_map<string, size_t>`，**内存派生数据，不落盘**，启动时 `rebuild_index()`。
- 密码：`salt + FNV-1a hash` 存储，不存明文（课程演示级，生产应用 bcrypt/argon2）。
- 备份：直接复制 `data/` 目录即可；恢复同理。

---

## 七、关键设计说明

1. **User 继承体系**：抽象基类 `User`（虚析构 + 纯虚 `role()`），业务数据在派生类
   `Student / Teacher / Admin`；账号字段集中到 `account_info`，便于序列化。
2. **存储扁平化**：学生通过 `class_id` 关联班级，不做嵌套；`vector<unique_ptr<T>>`
   保证扩容后对象地址稳定，登录返回的裸指针不会悬空。
3. **索引存下标不存指针**：`unordered_map<string, size_t>`；删除采用末尾交换法，O(1)。
4. **持久化分离**：JSON 主存储、按类型分文件；`to_json / from_json` 与类型同处
   `namespace sms`，由 ADL 被 nlohmann/json 自动发现。
5. **多态与内存**：`unique_ptr<User>` 支撑三张用户表；菜单层通过 `role()` 分发，
   不把菜单放进 User，保持 core 不依赖业务层。

---

## 八、第三方库
用于持久化

## 九、给 B / C 的交接说明

- 菜单函数声明见 `include/menus.h`；实现替换 `src/menus_stub.cpp`（或新建文件并在
  `CMakeLists.txt` 的 `t` 库源列表中追加）。
- 登录后获得 `User&`；学生用 `dynamic_cast<Student&>(u)`，教师用 `Teacher&`。
- 数据访问只走 `GradeSystem` 公有方法；写操作内部已 `save()`，业务层不必手动落盘。
- 批量导入请调用 `import_students_batch / import_scores_batch`，
  解析外部 CSV/TXT 的职责在业务层（C 的 `import_export.cpp`），
  **JSON 库只负责解析后的落盘与回滚，不是外部文件解析器**。
- 若序列化函数需要跨文件复用，请将其移到 `include/serialization.h` 并加 `inline`，
  且保持定义在 `namespace sms` 中（否则 ADL 找不到）。

---



