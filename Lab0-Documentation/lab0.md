# Lab0

> 请从[北大教学网](https://course.pku.edu.cn)获取接受本Lab任务的链接 

设计本Lab的主要目的是为了让同学们能够更加快速的上手Github Classroom

在本Lab中你要做的事情非常简单,我们将在下面的内容中介绍

本Lab不计入本课程的分数,但是我们仍希望你能够完成,一方面可以确认Github Classroom名单中人员和你的Github账号的绑定情况,另一方面你可以开始上手使用Github以及Github Classroom实现自动评测

## 编译与执行

### 编译环境

第一步当然是将你获得的该仓库Clone到本地找一个你喜欢的目录,执行如下指令:`git clone git@github.com:XXX/XXXX`

你可以不在本地搭建测试环境,但是我们仍然建议你这样做,以一定程度上熟悉CMake的使用

你需要的环境:

TA的系统: Ubuntu 18.04

1. G++
2. CMake
3. (Optional) Ninja (和之前使用的make是类似的)

对于一个已经写好的编译方法,我们保存在根目录的`CMakeLists.txt`中

### 编译准备

通过完成如下的步骤我们可以完成编译之前的准备工作(只需要完成一次即可,此后的每一次编译无需完成)

在命令行/终端/Terminal中执行如下的指令

1. `mkdir -p build`
2. `cd build`
3. `cmake .. -G "Unix Makefiles"` 如果您正确安装了 Ninja可以使用 `cmake .. -G Ninja`

如此我们完成了编译的准备工作

### 编译

在build目录中,我们只需要执行`make`指令即可编译(如果你使用了Ninja,那么请使用指令`ninja`进行编译)

### 执行

这里执行之后我们注意到build目录下生成了hellonetwork的可执行程序,不妨执行一下`./hellonetwork`执行即可

可执行程序`hellonetwork`接收一个参数,该参数为`0`或`1`,不同的参数会输出不同的结果

## 你要做什么

在本Lab中,你只需要使得你的程序在执行`./hellonetwork 1`的时候能够输出`Hello Network Hello PKU`即可

为此，你需要进行两个修改

1. 修改`CMakeLists.txt`使其能够生成可执行文件`hellonetwork`
2. 修改代码`hellonetwork.cpp`使得其能够在执行上述的命令时输出正确的值

提交代码的方式非常简单

如果你已经对Git的使用方法非常熟练了那么实际上你只需要将项目push到Github上即可

即在项目的根目录中执行

``` bash
git add .
git commit -am “Update”
git push
```

## 评测

在你将项目Push到Github之后,在仓库的Action选项卡中能看见自动评测的进度和结果
