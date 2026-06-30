# Build for x86_64 architecture only
APP_ABI :=  arm64-v8a

# Optional: Set Android API level
APP_PLATFORM := android-21

# Optional: Enable C++ exceptions and RTTI if using C++
APP_CPPFLAGS += -fexceptions -frtti

# Optional: Optimization level (default is -O2)
APP_OPTIM := release

# Optional: STL library selection
APP_STL := c++_static

# Optional: Build all available ABIs (comment out APP_ABI above if using this)
#APP_ABI := all

# Optional: Enable C++11 features
APP_CPPFLAGS += -std=c++17
