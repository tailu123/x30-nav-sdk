#!/bin/bash

# 设置工作目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# 生成 Doxygen 文档
echo "生成 Doxygen 文档..."
doxygen Doxyfile

# 提取待翻译文本
echo "提取待翻译文本..."
cd sphinx
sphinx-build -b gettext source locale/gettext

# 更新翻译文件
echo "更新翻译文件..."
sphinx-intl update -p locale/gettext -l en
sphinx-intl update -p locale/gettext -l zh_TW

# 生成简体中文文档
echo "生成简体中文文档..."
sphinx-build -b html source build/zh_CN

# 生成英文文档
echo "生成英文文档..."
sphinx-build -b html -D language=en source build/en

# 生成繁体中文文档
echo "生成繁体中文文档..."
sphinx-build -b html -D language=zh_TW source build/zh_TW

# 创建索引页面
echo "创建索引页面..."
cat > build/index.html << EOF
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta http-equiv="refresh" content="0; url=zh_CN/index.html">
    <title>RobotServer SDK 文档</title>
    <script type="text/javascript">
        window.location.href = "zh_CN/index.html";
    </script>
</head>
<body>
    如果页面没有自动跳转，请点击 <a href="zh_CN/index.html">这里</a>。
</body>
</html>
EOF

echo "文档生成完成！"
echo "您可以通过浏览器打开 ./docs/sphinx/build/index.html 查看文档。"
