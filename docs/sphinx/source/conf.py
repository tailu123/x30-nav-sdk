import os
import sys

# 设置路径
sys.path.insert(0, os.path.abspath('../../..'))

# 项目信息
project = 'RobotServer SDK'
copyright = '2025, RobotServer Team'
author = 'RobotServer Team'
version = '0.1.0'
release = '0.1.0'

# 扩展
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.todo',
    'sphinx.ext.intersphinx',
    'breathe',
]

# Breathe 配置
breathe_projects = {
    "RobotServer SDK": "../../doxygen/xml/"
}
breathe_default_project = "RobotServer SDK"
breathe_default_members = ('members', 'undoc-members')

# 主题
html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    'navigation_depth': 4,
    'titles_only': False,
    'logo_only': False,
}
html_logo = None  # 可以设置为您的项目logo
html_favicon = None  # 可以设置为您的项目favicon

# 多语言支持
language = 'zh_CN'
locale_dirs = ['../locale/']
gettext_compact = False

# 语言选项
languages = {
    'zh_CN': '简体中文',
    'en': 'English',
    'zh_TW': '繁體中文',
}

# HTML输出选项
html_static_path = ['_static']
templates_path = ['_templates']
html_css_files = ['custom.css']

# 添加语言切换器
html_context = {
    'languages': languages,
    'current_language': language,
    'current_version': version,
}

# 其他设置
todo_include_todos = True
source_suffix = '.rst'
master_doc = 'index'
exclude_patterns = []
pygments_style = 'sphinx'
