# JavaScript Runtime

An experimental, JavaScript runtime built on Mozilla SpiderMonkey engine in C++. This project serves as a playground for understanding native C++ bindings, global execution contexts, and custom runtime environments.

TL:DR just side project

## Prerequisites

Before building, you need a compiled standalone version of the SpiderMonkey library (`libmozjs`). Make sure you know the path to your build directory (e.g., containing `dist/include` and `dist/bin`).

## Installation & Setup

### 1. Clone the Repository
```bash
git clone [https://github.com/Ali-syr/Testing-runtime.git](https://github.com/Ali-syr/Testing-runtime.git)
cd testing-runtime
```

### 2. Configure the Local Symlink
To keep the build system generic and portable, create a local symlink named `spidermonkey` pointing to your standalone SpiderMonkey build directory

```bash
ln -s /path/to/your/spidermonkey/build_directory ./spidermonkey
```

### 3. Build & Run
You can easily compile and run the runtime using the provided `Makefile` or `build.sh`

* **To compile:**
  ```bash
  make
  ```
* **To run the test script:**
  ```bash
  make run
  ```
