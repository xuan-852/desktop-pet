里面分为左边运行和右边运行的图片，其中0.png是基本移动模式，1是基本移动模式，3是鼠标拖拽模式，5是降落模式，剩下的都是点击切换模式，各模式由于图片大小暂未调整，点击时会有些许范围突变，看上去挺突兀的。

## 构建

本项目是单文件 C 程序，不依赖 msbuild。推荐使用 MinGW-w64 的 g++ 进行构建（已在 VS Code 任务中提供）。

### VS Code 任务构建

使用任务 **C/C++: gcc build text1.c (with win libs)**。

### 命令行构建

示例命令（按需调整 MinGW 路径）：

```
D:\mingw\mingw64\bin\g++.exe -I d:\C\code\external\webview2\pkg\build\native\include -Wall -Wextra -g3 d:\C\code\text1.c -o d:\C\code\output\text1.exe -lgdi32 -luser32 -lmsimg32 -lgdiplus -lole32 -luuid
```

> 说明：如果你希望用 msbuild，请先安装 Visual Studio Build Tools 并将 msbuild 加入 PATH。但本项目不需要 msbuild。
