![PaperScope Logo](assets/paperscope-logo.png)

# PaperScope Manager

This guide will help you set up the PaperScope Manager on both Windows and Mac. 


---


## Prerequisites

- Git
- [Qt Framework](https://www.qt.io/download-open-source)
- [CMake](https://cmake.org) (Windows)
- [Homebrew](https://brew.sh) (Mac)


---


## Setup

### 1. OpenCV

#### Windows

Download the OpenCV (min v4.10) source code from the [OpenCV GitHub repository](https://github.com/opencv/opencv) and the [OpenCV extra modules](https://github.com/opencv/opencv_contrib).

Open your console and navigate into the OpenCV repository folder:
```
cd opencv-4.10
```

Create a build directory and navigate to it:
```
mkdir build && cd build
```

Configure the build with CMake:
```
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules
```

Build OpenCV:
```
cmake --build . --config Release
```

If compiling OpenCV doesn't work, you can use the precompiled files available in the `thirdparty/opencv` directory. For a detailed setup guide visit https://jinscott.medium.com/build-opecv-on-windows-with-cuda-f880270eadb0


#### Mac

Install OpenCV and pkg-config using Homebrew:
```
brew install opencv pkg-config
```

Ensure that pkg-config can find OpenCV:
```
export PKG_CONFIG_PATH=/usr/local/opt/opencv/lib/pkgconfig
```

You can verify the installation by running:
```
pkg-config --modversion opencv4
```
If the installation is successful, you should see the version number of OpenCV.

### 2. QT Creator

Open the Qt Creator application and the PaperScope project via the `PaperScopeManager.pro` file located in the root directory of the repository.

Configure the project to use the appropriate Qt kit for your platform.

Build the project by clicking on the "Build" button or selecting `Build > Build Project` from the menu.

Run the application by clicking on the "Run" button or selecting `Build > Run` from the menu.


---


## How To Use

tbd