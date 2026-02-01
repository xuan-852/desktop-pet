# photo文件
里面分为左边运行和右边运行的图片，
其中0.png是基本移动模式，
1 && 4是停止时的模式，
2 && 8是降落的高度较低时的模式，
3是鼠标拖拽模式，
5是高度较高的时候的降落模式，
6 && 7点击切换模式，
各模式由于图片大小暂未调整，点击时会有些许范围突变，看上去挺突兀的。

# 右键
当你右键人物时，可以进入setting，在里面可以设置各个任务的触发权重，以及ai聊天时的背景和文字的颜色。（分为left && right 的两类）

# 双击进入聊天模式
这里内嵌了Midsummer-s-Bird的开源AI聊天模型，具体实现请参考[Midsummer-s-Bird](https://github.com/1337-ops/Midsummer-s-Bird)中的README.md。本项目只基于该模型的基础功能，不包含模型训练部分。并且借由WebView2实现了在桌面宠物中嵌入聊天功能。

******要下载WebView2运行时，请参考[Microsoft WebView2 文档](https://learn.microsoft.com/en-us/microsoft-edge/webview2/getting-started/win32)。重要！！！

# text1.c
这是一个使用 WebView2 在 Windows 上创建桌面宠物的项目源文件。

## 构建

本项目是单文件 C 程序，不依赖 msbuild。推荐使用 MinGW-w64 的 g++ 进行构建（已在 VS Code 任务中提供）。

### VS Code 任务构建

使用任务 **C/C++: gcc build text1.c (with win libs)**。

### 命令行构建

示例命令（按需调整 MinGW 路径）：

```
D:\mingw\mingw64\bin\g++.exe -I d:\C\code\external\webview2\pkg\build\native\include -Wall -Wextra -g3 d:\C\code\text1.c -o d:\C\code\output\text1.exe -lgdi32 -luser32 -lmsimg32 -lgdiplus -lole32 -luuid
```

### 安装 WebView2 运行时（重要）

### 要安装的库

- WebView2 库（已包含在项目中）
- MinGW-w64 工具链（包含 g++）

### 常见错误：找不到 WebView2.h

如果出现 `fatal error: WebView2.h: No such file or directory`，说明没有添加 WebView2 头文件路径。

请确保在编译命令里加入：

```
-I <你的工程目录>\external\webview2\pkg\build\native\include
```

如果你在别的目录编译，请把 `<你的工程目录>` 改成实际路径（例如 `D:\haha\desktop-pet-main`）。

> 说明：如果你希望用 msbuild，请先安装 Visual Studio Build Tools 并将 msbuild 加入 PATH。但本项目不需要 msbuild。

## 后话
感谢 Midsummer-s-Bird 提供的开源模型。如果你喜欢这个项目，请给 Midsummer-s-Bird 点个 star！！！！