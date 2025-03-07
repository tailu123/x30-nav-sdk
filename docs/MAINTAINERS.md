# RobotServer SDK 文档维护指南

本指南提供了关于如何维护 RobotServer SDK 文档的说明。

## 文档系统概述

RobotServer SDK 使用 Doxygen + Sphinx 构建文档系统：

- **Doxygen**：从代码注释中提取 API 信息
- **Breathe**：作为 Doxygen 和 Sphinx 之间的桥梁
- **Sphinx**：生成美观、交互式的文档网站
- **sphinx-intl**：提供多语言支持

## 目录结构

```
docs/
├── Doxyfile                # Doxygen 配置文件
├── build_docs.sh           # 文档构建脚本
├── doxygen/                # Doxygen 输出目录
│   ├── html/               # Doxygen HTML 输出
│   └── xml/                # Doxygen XML 输出 (供 Breathe 使用)
├── sphinx/                 # Sphinx 文档
│   ├── source/             # 源文件
│   │   ├── _static/        # 静态文件 (CSS, JS, 图片等)
│   │   ├── _templates/     # 模板文件
│   │   ├── *.rst           # reStructuredText 文档
│   │   └── conf.py         # Sphinx 配置
│   ├── locale/             # 翻译文件
│   │   ├── gettext/        # 提取的待翻译文本
│   │   ├── en/             # 英文翻译
│   │   └── zh_TW/          # 繁体中文翻译
│   └── build/              # 构建输出
│       ├── zh_CN/          # 简体中文文档
│       ├── en/             # 英文文档
│       └── zh_TW/          # 繁体中文文档
└── MAINTAINERS.md          # 本文件
```

## 代码注释规范

为了让 Doxygen 正确提取 API 信息，请遵循以下代码注释规范：

### 类注释

```cpp
/**
 * @brief 类的简短描述
 *
 * 类的详细描述，可以包含多行文本。
 */
class MyClass {
    // ...
};
```

### 方法注释

```cpp
/**
 * @brief 方法的简短描述
 *
 * 方法的详细描述，可以包含多行文本。
 *
 * @param param1 参数1的描述
 * @param param2 参数2的描述
 * @return 返回值的描述
 * @throws ExceptionType 可能抛出的异常及原因
 * @note 其他需要注意的事项
 * @see OtherClass 相关类或方法的引用
 */
ReturnType myMethod(Type1 param1, Type2 param2);
```

### 枚举注释

```cpp
/**
 * @brief 枚举的简短描述
 *
 * 枚举的详细描述，可以包含多行文本。
 */
enum class MyEnum {
    /**
     * @brief 第一个枚举值的描述
     */
    VALUE1,

    /**
     * @brief 第二个枚举值的描述
     */
    VALUE2
};
```

### 多语言注释

可以使用特殊标记为不同语言提供不同的文档：

```cpp
/**
 * @brief 方法的简短描述
 * @zh 方法的中文简短描述
 * @en Method short description in English
 * @tw 方法的繁體中文簡短描述
 */
void myMethod();
```

## 文档编写指南

### reStructuredText 基础语法

Sphinx 使用 reStructuredText (RST) 格式编写文档。以下是一些基本语法：

#### 标题

```rst
一级标题
=======

二级标题
-------

三级标题
^^^^^^^

四级标题
"""""""
```

#### 列表

```rst
* 无序列表项 1
* 无序列表项 2
  * 嵌套列表项

1. 有序列表项 1
2. 有序列表项 2
   a. 嵌套有序列表项
```

#### 代码块

```rst
.. code-block:: cpp

   #include <iostream>

   int main() {
       std::cout << "Hello, World!" << std::endl;
       return 0;
   }
```

#### 链接

```rst
`链接文本 <https://example.com>`_

内部链接: :doc:`quick_start`
```

#### 表格

```rst
.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - 列 1 标题
     - 列 2 标题
   * - 行 1, 列 1
     - 行 1, 列 2
   * - 行 2, 列 1
     - 行 2, 列 2
```

### Breathe 指令

Breathe 提供了一系列指令，用于从 Doxygen XML 输出中提取 API 信息：

```rst
.. doxygenclass:: MyClass
   :members:
   :protected-members:
   :private-members:
   :undoc-members:

.. doxygenfunction:: myFunction

.. doxygenenum:: MyEnum
```

## 多语言支持

### 提取待翻译文本

```bash
cd docs/sphinx
sphinx-build -b gettext source locale/gettext
```

### 更新翻译文件

```bash
sphinx-intl update -p locale/gettext -l en
sphinx-intl update -p locale/gettext -l zh_TW
```

### 编辑翻译文件

翻译文件位于 `docs/sphinx/locale/<language>/LC_MESSAGES/` 目录下，使用 `.po` 格式。

示例 `.po` 文件内容：

```
#: ../../source/index.rst:2
msgid "RobotServer SDK 文档"
msgstr "RobotServer SDK Documentation"
```

### 构建特定语言的文档

```bash
# 构建英文文档
sphinx-build -b html -D language=en source build/en

# 构建繁体中文文档
sphinx-build -b html -D language=zh_TW source build/zh_TW
```

## 文档构建

### 本地构建

```bash
# 确保脚本有执行权限
chmod +x docs/build_docs.sh

# 运行构建脚本
./docs/build_docs.sh
```

构建完成后，可以通过浏览器打开 `docs/sphinx/build/index.html` 查看文档。

### CI/CD 构建

当推送到 `main` 分支时，GitHub Actions 会自动构建文档并部署到 GitHub Pages。

## 文档维护流程

1. **代码更新时**：确保更新相应的代码注释
2. **文档内容更新**：修改 Sphinx 源文件 (`docs/sphinx/source/*.rst`)
3. **翻译更新**：
   - 提取待翻译文本
   - 更新翻译文件
   - 编辑翻译文件
4. **构建和检查**：本地构建文档并检查
5. **部署**：推送到 `main` 分支，CI/CD 会自动部署

## 常见问题

### Doxygen 未提取某些 API 信息

- 检查代码注释格式是否正确
- 检查 `Doxyfile` 配置是否正确
- 尝试手动运行 Doxygen: `cd docs && doxygen Doxyfile`

### Breathe 无法找到 Doxygen XML 输出

- 确保 Doxygen 已成功运行并生成 XML 输出
- 检查 `conf.py` 中的 `breathe_projects` 配置是否正确

### 翻译未生效

- 确保已提取待翻译文本并更新翻译文件
- 检查翻译文件格式是否正确
- 确保使用正确的语言代码构建文档

## 联系方式

如有任何问题，请联系文档维护团队：

- 邮件：docs@example.com
- GitHub Issue: https://github.com/tailu123/robotserver-sdk/issues
