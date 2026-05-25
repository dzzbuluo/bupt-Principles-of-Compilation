# 词法分析程序（LEX/flex）

## 一、准备环境（三选一）
1) MSYS2（推荐）
- 安装 MSYS2 后打开 MINGW64 终端：
  - pacman -S --noconfirm flex gcc

2) WSL(Ubuntu)
- sudo apt update && sudo apt install -y flex gcc

3) WinFlexBison + Mingw-w64（图形包）
- 安装 win_flex 可执行与 gcc，并将二者加入 PATH。

## 二、生成与编译
- 在本目录执行（MSYS2/WSL 终端）：
```
flex scanner.l        # 生成 lex.yy.c
gcc lex.yy.c -o scanner
```
- Windows 本机若用 win_flex：
```
win_flex scanner.l
gcc lex.yy.c -o scanner.exe
```

## 三、运行
```
./scanner < test.c > tokens.txt 2> errors.txt
```
- 标准输出：逐行打印 Token（行:列  类型  词素）与最后的 Summary。
- 标准错误：词法错误（非法字符、未闭合注释/字符串等）。

## 四、文件说明
- scanner.l：LEX 规则（C 子集 + 状态机 + 统计 + 错误恢复）
- test.c：示例输入（含正常与错误用例）
- tokens.txt：Token 输出示例（运行后生成）
- errors.txt：错误输出示例（运行后生成）

## 五、报告撰写提纲
- 题目与要求概述
- 设计说明：Token 列表、正则规则、start conditions、错误恢复策略、位置与统计维护
- 测试报告：输入用例（正常/边界/错误）、运行结果截图、分析说明



