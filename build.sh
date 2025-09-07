#!/bin/bash

# ==============================================================================
# Cross-Platform Build Script for LibrarySystem
#
# 支持的平台 (Supported Platforms):
#   - Linux (Debian/Ubuntu, Fedora/CentOS, Arch)
#   - macOS (需要 Homebrew)
#   - Windows (需要 Git Bash 和手动安装依赖)
#
# 使用方法 (Usage):
#   1. 将此脚本保存为 'build.sh' 放到项目根目录。
#   2. 给予执行权限: chmod +x build.sh
#   3. 运行脚本: ./build.sh
# ==============================================================================

# 如果任何命令执行失败，立即退出脚本
set -e

# --- 日志函数，方便输出信息 ---
log() {
    # 使用绿色输出日志，更醒目
    echo -e "\033[0;32m[构建脚本] ==> $1\033[0m"
}

# --- 错误函数 ---
error() {
    # 使用红色输出错误信息
    echo -e "\033[0;31m[构建脚本] 错误: $1\033[0m" >&2
    exit 1
}


# --- 1. 检测操作系统 ---
OS_NAME=""
case "$(uname -s)" in
    Linux*)     OS_NAME="Linux";;
    Darwin*)    OS_NAME="macOS";;
    CYGWIN*|MINGW*|MSYS*)    OS_NAME="Windows";;
    *)          OS_NAME="UNKNOWN";;
esac
log "检测到当前操作系统: $OS_NAME"


# --- 2. 检查并安装依赖 ---

# 定义一个函数用于安装 SQLite3
install_sqlite() {
    log "正在尝试安装 SQLite3..."
    case $OS_NAME in
        "Linux")
            log "检测到 Linux 系统，将使用包管理器安装。"
            if [ -x "$(command -v apt-get)" ]; then
                sudo apt-get update && sudo apt-get install -y sqlite3 libsqlite3-dev
            elif [ -x "$(command -v dnf)" ]; then
                sudo dnf install -y sqlite sqlite-devel
            elif [ -x "$(command -v yum)" ]; then
                sudo yum install -y sqlite sqlite-devel
            elif [ -x "$(command -v pacman)" ]; then
                sudo pacman -S --noconfirm sqlite3
            else
                error "未找到支持的包管理器 (apt, dnf, yum, pacman)。请手动安装 'sqlite3' 和 'libsqlite3-dev'。"
            fi
            ;;
        "macOS")
            log "检测到 macOS 系统，将使用 Homebrew 安装。"
            if ! [ -x "$(command -v brew)" ]; then
                error "未找到 Homebrew。请先访问 https://brew.sh/ 安装 Homebrew，或手动安装 SQLite3。"
            fi
            brew install sqlite
            ;;
        "Windows")
            log "在 Windows 上，脚本无法自动安装 SQLite3。"
            log "请从官网 https://www.sqlite.org/download.html 下载预编译的二进制文件。"
            log "下载后，请解压并将 'sqlite3.dll', 'sqlite3.def', 'sqlite3.exe' 放到一个在系统 PATH 中的目录。"
            error "请在手动安装 SQLite3 后重新运行此脚本。"
            ;;
        *)
            error "不支持的操作系统: $OS_NAME。请手动安装 SQLite3。"
            ;;
    esac
}

# 主逻辑 - 检查依赖
log "开始检查项目依赖..."

# 检查 SQLite3
if ! [ -x "$(command -v sqlite3)" ] || ! (echo "int main() { return 0; }" | gcc -lsqlite3 -o /dev/null -xc - >/dev/null 2>&1); then
    log "未找到 SQLite3 或其开发库。"
    read -p "是否需要脚本尝试自动安装? (y/n): " -n 1 -r
    echo # 换行
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_sqlite
        # 再次验证安装结果
        if ! [ -x "$(command -v sqlite3)" ]; then
            error "SQLite3 安装失败。请检查错误信息并手动安装。"
        fi
        log "SQLite3 已成功安装。"
    else
        log "用户取消安装。请在手动安装 SQLite3 后重新运行此脚本。"
        exit 0
    fi
else
    log "SQLite3 已安装。"
fi

# 检查 CMake
if ! [ -x "$(command -v cmake)" ]; then
    error "未找到 CMake。请先安装 CMake (https://cmake.org/download/) 并确保它在系统 PATH 中。"
fi
log "CMake 已安装。"


# --- 3. 编译项目 ---
log "开始编译项目..."

BUILD_DIR="build"
PROJECT_NAME="LibrarySystem" # 从 CMakeLists.txt 中获取的项目名称

# 根据操作系统确定可执行文件名
if [ "$OS_NAME" == "Windows" ]; then
    EXECUTABLE_NAME="$PROJECT_NAME.exe"
else
    EXECUTABLE_NAME="$PROJECT_NAME"
fi

# 清理旧的构建目录
if [ -d "$BUILD_DIR" ]; then
    log "发现旧的构建目录，正在清理..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录并进入
log "创建并进入构建目录: $BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# 运行 CMake 来配置项目
log "正在使用 CMake 配置项目..."
# 在Windows上，可能需要指定生成器，例如: cmake .. -G "MinGW Makefiles"
cmake ..

# 编译项目
log "正在编译源代码..."
cmake --build .

# 将生成的可执行文件移动到项目根目录
log "将可执行文件 '$EXECUTABLE_NAME' 移动到项目根目录..."
mv "$EXECUTABLE_NAME" ../"$EXECUTABLE_NAME"

# 返回到项目根目录
cd ..

# --- 4. 完成 ---
log "--------------------------------------------------------"
log "构建成功！"
log "可执行文件 '$EXECUTABLE_NAME' 已生成在当前文件夹中。"
log "您可以通过以下命令运行它:"
if [ "$OS_NAME" == "Windows" ]; then
    log "./$EXECUTABLE_NAME"
else
    log "./$EXECUTABLE_NAME"
fi
log "--------------------------------------------------------"

exit 0
