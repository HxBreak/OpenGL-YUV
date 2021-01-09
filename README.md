OpenGL YUV
==========
OpenGL YUV is a project to display YUV(NV12/NV21) on a surface

OpenGL YUV是一个显示指定YUV数据(NV12/NV21)到surface上的项目

You can based this project to develop other any thing you want, most OpenGL interaction is write in C/C++

你可以基于此项目或者任意的部分代码继续开发其他的软件，大部分OpenGL交互都由C/C++完成

This sample uses the new [Android Studio CMake plugin](http://tools.android.com/tech-docs/external-c-builds) with C++ support.

Pre-requisites
--------------
- any device support OpenGL ES 3 API(PBO feature)
- 任何支持OpenGL ES3的设备（必须支持Pixel Buffer Object 特性）
- Android Studio 2.2 preview+ with [NDK](https://developer.android.com/ndk/) bundle.

Getting Started
---------------
1. [Download Android Studio](http://developer.android.com/sdk/index.html)
1. Launch Android Studio.
1. Open the sample directory.
1. Open *File/Project Structure...*
  - Click *Download* or *Select NDK location*.
1. Click *Tools/Android/Sync Project with Gradle Files*.
1. Click *Run/Run 'app'*.

Screenshots
-----------
![screenshot](screenshot.png)

License
-------
Copyright 2021 HxBreak.

Licensed to the Apache Software Foundation (ASF) under one or more contributor
license agreements.  See the NOTICE file distributed with this work for
additional information regarding copyright ownership.  The ASF licenses this
file to you under the Apache License, Version 2.0 (the "License"); you may not
use this file except in compliance with the License.  You may obtain a copy of
the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
License for the specific language governing permissions and limitations under
the License.
