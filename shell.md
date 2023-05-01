# Shell

> - **Submitter**: stonepage, rapiz
> - **Time limit** 2 weeks

## 简介

你要在linux系统下实现一个terminal+shell

## Terminal

你需要在real的终端里实现一个虚假的terminal，实现以下功能

1. 【basic】在终端中输入命令，以行为单位，按回车执行
2. 【basic】左右方向键可以移动光标
3. 【basic】ctrl+c可以停止正在执行的命令，ctrl+d可以退出终端，回到原来的terminal

## Shell

你需要写一个C/C++命令行解释器来在fake终端中解析一些命令，通过**libc中的系统调**用实现以下功能

**无特殊说明，则不需实现带命令参数**

1. 实现一些shell内建命令

   1. 【basic】echo
   2. 【basic】exit
   3. 【basic】cd pwd 
   4. 【advanced】kill 

2. 实现一些shell特性

   1. 重定向和管道（不用考虑太复杂的边界情况）

      - 【basic】I/O重定向（> <）  管道 （|） 

        保证一条指令只出现一个重定向或管道（>|<）

      - 【advanced】一条命令中>|<出现多次，并完成非法指令的报错

        你的程序应该可以处理以下命令： cat < file1 | xargs cat > file3 

   2. 后台运行 &

3. 实现一些命令

   1. 这些命令放在哪里呢？
      - 【basic】存储在一个固定的目录
      - 【advanced】定义一个环境变量文件
      - 【advanced】实现内建命令export
   2. ls
      - 【basic】实现ls
      - 【advanced】实现ls -l
   3. 显示文件
      - 【basic】cat 
      - 【advanced】less                  
   4. xargs
      - 【basic】 一个参数即可
      - 【advanced】xargs -d
   5. 文件操作
      - 【basic】实现touch和mkdir
      - 【advanced】实现cp、mv、rm、ln、ln -s

4. ~~【lol】让程序员喜欢用你的shell~~

   1. ~~色彩高亮~~
   2. ~~命令提示~~
   3. ~~智能补全~~
   4. ~~热键绑定~~
   5. ~~可开发定制插件~~
   6. ~~可开发定制主题~~

## DDL

两周完成

但下周检查希望你能完成绝大部分basic

## Hint

你可能会经常用到man命令

网上的资料相当多，所以在这就不提供链接了

网上的资料相当多，但是请面向man pages编程而非面向blog

网上的资料相当多，因此code review会拉满（禁止copy

听说stdlib.h里有个函数叫system，出于显而易见的原因，本次任务禁止使用此函数和及与其功能相似的函数

系统调用多查 glibc

支持光标移动的读取输入看 `man 3 readline`

