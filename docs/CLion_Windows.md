# CLion Windows
How to set up your development environment for CLion on Windows.

## Build Settings
Download and install [LLVM](https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.0)

In CLion, go to **Settings -> Build, Execution, Deployment -> Toolchains**

Set the C and C++ Compiler to a Windows-friendly clang from the LLVM download:
- C Compiler: `C:\Program Files\LLVM\bin\clang-cl.exe`
- C++ Compiler: `C:\Program Files\LLVM\bin\clang-cl.exe`

![Screenshot 2025-02-13 at 2 27 54 PM](https://github.com/user-attachments/assets/3df679b4-8c54-4d27-a8b0-27bc990c022f)

Go to **CMake**, right below **Build, Execution, Deployment**.

Click the plus icon to add a profile

Name it **Windows** or something similar. I'll use **Windows2** in my examples.

Set these fields:
- Build Type: Release
- Toolchain: Use Default (Visual Studio)
- Generator: Use Default (Ninja)
- CMake options:
```bash
-DBUILD_FRONTEND=ON
-DBUILD_TESTS=ON
```
![Screenshot 2025-02-13 at 2 37 47 PM](https://github.com/user-attachments/assets/ef68bf16-5c5e-4bc6-ba23-e27a4a593a9d)

Click OK. The project will attempt to build and fail because Vcpkg is not yet installed.

## Build The Project

Go to **View -> Tool Windows -> Vcpkg**

Look for the **Vcpkg** panel in the bottom left toolbar.

![Screenshot 2025-02-13 at 2 42 41 PM](https://github.com/user-attachments/assets/5370534c-87fb-4c65-acff-1b87e4e6c493)

Click the plus icon to add a new instance. In the installation window, make sure the CMake profile you created is checked and click OK.

![Screenshot 2025-02-13 at 2 44 30 PM](https://github.com/user-attachments/assets/7d6ccf02-a7ef-487c-8dfe-a093fa7be94c)

CLion will download and install a Vcpkg

The installation should have adjusted your CMake profile settings. Go back to Settings → Build, Execution, Deployment → CMake.

Under CMake options, `DCMAKE_TOOLCHAIN_FILE` has been added, which Vcpkg uses to build and resolve dependencies.
```bash
-DBUILD_FRONTEND=ON
-DBUILD_TESTS=ON
-DCMAKE_TOOLCHAIN_FILE=C:\Users\bgevk\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Cache variables have also been populated. You can adjust various build settings here.

![Screenshot 2025-02-13 at 2 58 12 PM](https://github.com/user-attachments/assets/a6e06177-4307-44c0-aef7-1ecc00e1042e)

Click OK to close the window.

Build the project and attempt to run the **emu** target from the top-right corner.

![Screenshot 2025-02-13 at 4 46 32 PM](https://github.com/user-attachments/assets/f61daf17-61fe-4063-9904-37f79610da4d)

The build should run, and the emulator window should open. 

Go back to the top-right corner and select **AllCTest** as the target. Click on the three dots on the far right to edit the test config:

![Screenshot 2025-02-13 at 4 47 16 PM](https://github.com/user-attachments/assets/24632e40-5d35-4b84-a655-24198beb8ec3)

Add `-j 2` to **CTest arguments**. This will run tests in parallel using 2 cores. Adjust as your device allows.

Run the AllCTest target and verify that all tests pass.

---

