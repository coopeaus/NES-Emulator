# CLion Mac OS

## Build Settings

In CLion terminal, run the setup check script
```bash
chmod +x scripts/env-check.sh
scripts/env-check.sh
```

![Screenshot 2025-02-13 at 3 14 54 PM](https://github.com/user-attachments/assets/eae802fa-e6e9-4ad3-85b1-68b9efc8face)

Using [homebrew](https://brew.sh/), isntall the missing dependencies
```bash
brew install llvm cmake pkg-config ninja
```

Rerun the script. If the installations are still red, you must add the value of `brew --prefix` to your PATH variable. If you're not sure how to do that, check out [this](https://www.architectryan.com/2012/10/02/add-to-the-path-on-mac-os-x-mountain-lion/#.Uydjga1dXDg) article. 

At this point, only `vcpkg` should be red.

![Screenshot 2025-02-13 at 3 18 26 PM](https://github.com/user-attachments/assets/ede75d11-502a-47d9-898c-6182f6671394)

Take note of the installation paths for `clang` and `clang++`, you will need them.

In Clion, got to Settings → Build, Execution, Deployment → Toolchains.

Adjust the name of the default toolchain (or create a new one). I'll call mine Clang Toolchain.

Adjust the C and C++ compiler paths to match your installation paths. Yours may be different from mine. Mine are:
- C Compiler: `/opt/homebrew/opt/llvm/clang`
- C++ Compiler: `/opt/homebrew/opt/llvm/clang++`

![Screenshot 2025-02-13 at 4 22 10 PM](https://github.com/user-attachments/assets/5154b8cd-4fc8-45d2-b4cc-38f66ea16139)

Just below **Toolchains**, go to **CMake**

Click the plus button to add a new profile. I’ll name mine Mac_CLion

Set these fields:
- Build type: Release
- Toolchain: Clang Toolchain
- Generator: Use default (Ninja)
- CMake options
```cmake
-DBUILD_FRONTEND=ON
-DBUILD_TESTS=ON
```

Click OK. The project will try to build but fail. You'll need to install Vcpkg.

## Building the Project

Go to View → Tool Windows → Vcpkg

Look for the Vcpkg panel in the bottom left tool panel.

![Screenshot 2025-02-13 at 4 29 33 PM](https://github.com/user-attachments/assets/1cc54972-4bbb-43ea-80a7-1f0704033340)

Click on the plus icon to add Vcpkg. In the installation window, select the CMake profile you created earlier and click OK.

![Screenshot 2025-02-13 at 4 30 36 PM](https://github.com/user-attachments/assets/2d5115f4-fa46-40d7-aa4e-165d07c7d702)

Vcpkg will install and adjust your CMake settings. To see what it changed, go back to Settings → Build, Execution, Deployment → CMake.

Under **CMake options**, notice the new `DCMAKE_TOOLCHAIN_FILE` entry. This points to the Vcpkg installation and helps install and resolve C++ packages.

You'll now also be able to adjust various build settings under **Cache variables**

![Screenshot 2025-02-13 at 4 35 35 PM](https://github.com/user-attachments/assets/ec186dad-c3fb-4fcb-a291-a20725b14904)

Click OK and rebuild the project

Try to run the **emu** target from the top-right corner

![Screenshot 2025-02-13 at 4 36 47 PM](https://github.com/user-attachments/assets/262148ee-3be4-4621-b98a-2fe69b008e0d)

The emulator window should open.

Select the **AllCTest** target and click the dots on the far right to edit the test settings

![Screenshot 2025-02-13 at 4 40 44 PM](https://github.com/user-attachments/assets/9c7fd5b7-455d-4073-89d1-759fed5faa96)

Add `-j 2` in **CTest arguments** to allow your tests to run in parallel using 2 cores. Adjust to as many cores as your machine allows.

Run the **AllCTeset** target and verify it works.

---



