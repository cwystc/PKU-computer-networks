# Git&Github

## Git
Git 是一个开源的分布式版本控制系统，常用于便捷高效地处理任何或大或小的项目。

Git 是 Linus Torvalds 为了帮助管理 Linux 内核开发而开发的一个开放源码的版本控制软件。

我们强烈推荐你使用 git 管理你的项目。
### 安装 Git
在 linux 下，你只需要执行
``` bash
apt-get install git
```
在 windows 和 mac 下， git 提供了图形化安装方式。

### Git 基本操作

#### 创建仓库

`git init` 用于创建并初始化一个新的仓库，你可以在该仓库开始构建项目。

在任意位置新建文件夹 `mkdir lab0` ，在该目录下执行 `git init` 会在该位置创建本地 git 仓库

``` bash
$ mkdir lab0
$ cd lab0
$ git init
```

除了新建一个仓库，你也可以使用 `git clone` 命令拷贝一个现有仓库，我们以 Lab0 的模板仓库为例，在任意位置执行以下指令
``` bash
$ git clone https://github.com/N2Sys-EDU/Lab0-Introduction-To-Classroom.git
```
这条指令将位于 `https://github.com/N2Sys-EDU/Lab0-Introduction-To-Classroom.git` 的远程仓库克隆到本地，你将会在当前目录下发现目录 `Lab0-Introduction-To-Classroom/`

#### 提交与修改
当你在项目中做了一些修改后，比如创建一个新文件 `touch README.md` ，你可以使用 `git add` 命令来将你的修改添加到暂存区，例如 `git add README.md` ，你也可以使用 `git add .` 来将所有修改添加到暂存区。
``` bash
$ touch README.md
$ git add README.md
```

在确认了你的改动之后，你可以将文件提交到仓库中，但在此之前，你需要先设置你的用户信息
``` bash
git config --global user.name "labman008"
git config --global user.email "labman008@pku.edu.cn"
```
`--global` 用于指明作用域为全局，相应的你也可以使用 `--local` 来使得配置仅在当前仓库生效。

之后，你可以使用 `git commit` 指令将暂存区的文件提交到本地仓库中，这将会在仓库中创建一个快照，或者说项目的一个版本，你可以利用 git 在不同版本之间快捷的切换，换言之你不用再担心因为反复修改而失去了第一份能运行的代码了。
``` bash
$ git commit -m "first commit"
```
`git commit -m [message]` 用于为你的提交添加一些说明。另外，你也可以使用 `git commit -a` 来跳过 `git add`

## Github

现在，你已经掌握了使用 git 管理本地仓库的基本方法。如果你想通过 git 分享你的代码或者与其他人合作开发，就需要将数据放到一台其他开发人员能够连接的服务器上。

Github 是一个在线软件软代码托管服务平台，使用 git 作为版本控制软件。截至 2022 年 6 月， github 已有超过 5700 万注册用户和 1.9 亿代码库（包括至少 2800 万开源代码库），是世界上最大的代码托管网站和开源社区。

在本次 Lab 中，我们使用 github 作为 git 远程仓库，并借助 github 的 classroom 功能进行测试。

### 创建账号

打开 [github官网](https://github.com/) ，点击右上角 `sign up` ，跟随指引创建你的 github 账号。

### 创建仓库

登入后点击左侧 New 或右上角加号 - `New repository` 新建一个仓库，键入仓库名和其他你觉得需要的信息后点击 `Create repository` 即可完成创建。当然，在本次 lab 中，你只需要使用 classroom 自动创建的仓库而不需要自己新建仓库。

### 配置 ssh key

为了在本地仓库和远程仓库间进行传输的安全性，需要进行验证。我们推荐你使用 ssh 进行加密传输，为此你需要在 github 上添加你的 ssh 公钥。

如果你对 ssh 的工作方式感兴趣，可以自行 google

#### 生成 SSH Key

在本地使用 `ssh-keygen` 命令生成密钥。简单起见，这里我们使用 `ssh-keygen` 的默认生成方式，你可以查询该指令的参数来修改生成方式。
``` bash
$ ssh-keygen
```
你可以简单的键入三次回车来生成密钥，生成的密钥在 `~/.ssh/` 目录下。

#### 添加 SSH Key

回到 github ，点击右上角头像 - `Settings` ，然后点击左侧 `SSH and GPG keys` 进入 ssh key 配置界面。点击 `New SSH key` 添加新的密钥。

复制本地 `~/.ssh/id_rsa.pub` 中的 key 粘贴进 `Key` 中，在 `Title` 一栏你可以为该密钥命名。

在 linux 上你可以使用 
``` bash
$ cat ~/.ssh/id_rsa.pub
``` 
获取生成的公钥。

输入完后，点击 `Add SSH key` 完成添加。

你可以在本地执行 
``` bash
$ ssh -T git@github.com
```
来测试是否添加成功。

### 克隆仓库到本地

打开你想要 clone 的远程仓库，比如 classroom 自动新建的你的 lab0 仓库，点击绿色的 `Code` 按钮，选择 `SSH` ，复制下方的链接。

在本地执行
``` bash
$ git clone git@github.com:N2Sys-EDU/lab0-introduction-xxx.git
```
将 clone 后的链接换成刚刚复制的链接，如果之前的配置正确，你将在本地看到 clone 下来的本地仓库。

## More git

现在，你已经可以在 clone 下来的本地仓库中完成 Lab 了。最后，我们需要在本地仓库和远程仓库间进行同步。

### push

你可以通过 `git push` 命令将本地仓库推送到远端。注意，只有已提交的更改才会被推送。即，假设你修改了 lab0 仓库中的 hellonetwowrk.cpp 文件，那么你可以通过以下指令更新远程仓库
``` bash
$ git add hellonetwork.cpp
$ git commit -m "hellonetwork"
$ git push
```
或者简单的使用
``` bash
$ git commit -am "hellonetwork"
$ git push
```

### pull

你可以通过 `git pull` 命令将远端仓库的更新拉取到本地。这主要用于合作开发或者使用多台设备进行开发。
``` bash
$ git pull
```

到这里为止，你已经学会了本 lab 需要用到的所有内容，之后的内容我们只做一个简单的介绍，如有需求，可以自行 google

### branch

你可以通过 `git branch` 命令来基于当前版本创建一个新的分支，不同的分支创建后互相独立。之后，你可以通过 `git checkout` 命令来切换分支。举例来说，
``` bash
$ git branch new-branch
$ git checkout new-branch
```
对分支 new-branch 的修改不会影响到原分支，同样的，对原分支的修改将不再影响 new-branch 你可以再次执行 `git checkout main` 回到原分支， `main` 是 github 的默认分支。

另外，你可以通过 `git merge` 命令合并两个分支。

### reset

`reset` 命令用于版本回滚，即回退到提交过的某一版本，具体用法大家可以自行 google