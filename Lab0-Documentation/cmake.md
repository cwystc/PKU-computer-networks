# CMake 基本使用方法

上学期有同学做Lab2和Lab3的时候反应CMake使用不是很熟练，有些同学甚至没有接触过CMake，因此这里我们为大家做了一个简单的教学

CMake是一个简单的工具，帮助我们从更抽象的角度来维护项目，而不需要去手动调整和键入编译指令等细碎的内容

## 一个简单的Example

新建一个目录并进入该目录`mkdir Test && cd Test`，在该目录下创建文件`CMakeLists.txt`，并在该文件内输入如下的内容

``` cmake
project (Test)
cmake_minimum_required(VERSION 3.10)
```

这样一个最简单的CMake项目就建立完成了

为了测试CMake的功能，我们在这个目录下新建一个`main.cpp`的文件，内容如下

``` cpp
#include <iostream>
int main(int argc, char **argv) {
    std::cout << "Hello World" << std::endl;
    return 0;
}
```

修改`CMakeLists.txt`在末尾加入一行`add_executable(hello main.cpp)`这一行代码的意义在于：

1. 新建了一个hello的可执行程序
2. 这个hello的可执行程序代码由main.cpp这个**源文件**构成

在该目录下新建一个`build`文件夹，进入这个文件夹并执行`cmake .. -G "Unix Makefiles"`

这样，我们就在`build`目录下生成了Makefile的文件，只需要输入`make`即可进行编译，编译的结果也在`build`目录中

> 实际上除了Makefile还有很多类似的系统，我这里选择Unix Makefile是因为ICS课程中已经使用过这个经典的系统，我个人比较喜欢用Ninja的系统`cmake .. -G Ninja`

此时我们应该能在`build`目录中找到一个叫做`hello`的可执行文件，和Make本身的作用相同，当我们修改了文件之后，只需要在`build`目录中重新执行`make`即可根据修改的文件重新编译

当我们修改了`CMakeLists.txt`文件，但是我们不希望更改任何设置的时候，我们只需要在`build`目录中重新执行`cmake ..`即可(不用加任何参数)

## 添加头文件

某些情况下（比如为了让文件夹更美观），我们可能会将源文件和头文件放到不同的文件夹中比如

```
- include
--- hello.hpp
- src
--- main.cpp
- CMakeLists.txt
```

但是这样，我们就需要在`hello.cpp`中引用`hello.hpp`时使用`#include "../include/hello.hpp`

为了解决这个问题，我们可以在编译选项中加入`-Iinclude`这个参数解决

在CMake中表现为，我们可以为某一个可执行文件添加一个include目录使用方法`target_include_directories`即可解决

以上面的文件夹结构为例我们可以在`CMakeLists.txt`中添加

``` cmake
add_executable(hello src/main.cpp)
target_include_directories(hello PUBLIC include)
```

这样我们就可以在`main.cpp`中使用`#include <hello.hpp>`了

## 库

### 链接库

有的时候作为一个CV程序员XD，我们可能经常使用别人的库，当需要进行链接的时候，我们会使用指令`-lXXX`这会让编译器从某些特定的路径中查找`libXXX.a`文件并进行链接

对应在CMake中的命令为`target_link_libraries`

同样以上面的内容为例子，假设一个我们要链接`libaaa.a`和`libbbb.a`，我们只需要使用`target_link_libraries(hello PUBLIC aaa bbb)`

### 创建库

另一种情况是我们希望使用自己创建的库我们可以使用指令`add_library`创建一个库

文件目录为:

```
- main.cpp
- mylib.cpp
- CMakeLists.txt
```

`main.cpp`为

``` cpp
void PrintHelloWorld();
int main() { PrintHelloWorld(); }
```

`mylib.cpp`为

``` cpp
#include <iostream>
void PrintHelloWorld() {
    std::cout << "Hello World" << std::endl;
}
```

此时我们只需要像`add_executable`一样使用`add_library`即可创建一个库

``` cmake
add_library(mlib mylib.cpp)
```

使用指令`target_link_libraries(hello PUBLIC mlib)`即可

## CMake Tricks

本章节介绍了两个CMake的实用技巧，帮助你体会到CMake系统的便利

### Cache

有的时候，我们希望通过设置一些选项，通过对这些选项的快速修改来实现不同的编译目的

举个栗子: 当有两个不同的库libA.a\libB.a，他们功能相同，实现方法不同，我们要从中做出选择的时候，当然我们可以通过手动修改`target_link_libraries(hello PUBLIC XXX)`这一行指令来实现。但是在大型项目中，快速定位到这一行代码是十分繁琐的。

因此我们可以使用如下的代码

``` cmake
set(USE_A "YES" CACHE BOOL "Use libA")
if (${USE_A})
    target_link_libraries(hello PUBLIC A)
else()
    target_link_libraries(hello PUBLIC B)
endif()
```

这几句话的意义为创建一个变量`USE_A`类型为`BOOL`默认值为`YES`，变量介绍为`Use libA`

Cache的意义在于，我们将这个变量的值保存下来

有多种手段可以修改Cache变量的值(以下指令的执行目录均在build文件夹中)

1. `cmake .. -DUSE_A=NO`
2. `ccmake`会调出一个控制台的可视化界面进行管理这也就是变量介绍的意义，我们可以很清晰看到每一个变量的意义，并在可视化界面中修改
3. 基本上所有现代IDE都支持CMake(VSCode 需要装额外的插件)，其中能看到类似`Edit CMake Cache`的字样可以通过IDE进行可视化的修改

> 实际上我们还可以利用Cahce变量做很多其他的事情包括代码生成等，大家可以发挥想象力

### 子文件夹中的CMakeLists

如果我们的目录如下

```
- lib
-- mlib
--- mylib.cpp
--- CMakeLists.txt
- main.cpp
- CMakeLists.txt
```

我们将`add_library`指令扔到了`lib/mlib/CMakeLists.txt`中，那么我们还能`target_link_libraries`吗？

我们只需要在这一行之上加入`add_subdirectory(lib/mlib)`即可载入`lib/mlib/CMakeLists.txt`中的内容
