# 01 仓库核验、隔离边界与工程现状审计报告（R1）

> **文档状态**：R1 审计基准已获用户确认（2026-11-25）；本报告严格基于对 `c:\Users\JINGJINGPENG\Project\FireSystem\fire` 真实文件与 Git 命令的逐项核验，以及在严格隔离边界内对允许参考材料（`new_fire\原理图.pdf`、`new_fire\desktop\prototype\`）的调研结论编制。

---

## 1. 审计结论摘要

**核心结论：`c:\Users\JINGJINGPENG\Project\FireSystem\fire` 是一个包含完整硬件板级 BSP 基础与桌面端/CI 打包骨架的“初期单体仓库骨架（Skeleton Repository）”，而非已具备完整业务功能的 FireSystem 成品代码库。**

1. **Git 仓库基准已确认**：远程仓库为 `https://github.com/moluxuanji/FireSystem.git`，主分支 `main` 最新提交 SHA 为 `a1eaa93a3a44c439071a2b564b564e017f9bbcc5`（共 6 个历史提交），附带 1 个子模块 `desktop/third_party/FluentUI`（`7e33a2f672d18239ef49ab960075b1137a1d18e7`）。
2. **实质性工程基础**：`firmware/boards/lince/kzb_f407/` 已具备生产导向的 `STM32F407ZET6` Zephyr 板级 BSP（早期无毛刺安全 IO 钳位、`kzb_f407.dts` 外设树与 `frozen-p09` 分区契约 `partitions.json`）；`desktop/` 与 `.github/workflows/release.yml` 已具备 Qt 6 + 静态 `FluentUI` + CPack/QtIFW 多平台打包流水线。
3. **关键空白与模板现状**：`firmware/app/` 与 `firmware/boot/` 均为未经修改的 Zephyr `samples/hello_world` 占位模板；`desktop/src/` 仅包含 54 行 `Main.qml` 演示窗体；`docs/`、`README.md`、`common/`、`tests/`、`west.yml` 全部缺失；CI 中测试与静态检查数量为 **0**。
4. **R1 门禁触发与用户确认**：已按 R1 规则触发暂停确认门禁，并获得用户正式确认：以 `fire`（`main @ a1eaa93`）作为正确基准仓库继续演进，同时保留工作区原本存在的 2 处本地修改（`desktop/CMakeLists.txt`、`desktop/src/FluentUIDeps.qml`）纳入开发分支管理。

---

## 2. Git 仓库状态与历史核验

所有 Git 核验均在 `c:\Users\JINGJINGPENG\Project\FireSystem\fire` 路径下通过只读 Git 命令直接采集：

| 核验项 | 真实核验结果 | 采集依据 |
|---|---|---|
| 远程地址 (`origin`) | `https://github.com/moluxuanji/FireSystem.git` (fetch & push) | `git remote -v` |
| 主干基准分支 | `main`（与 `remotes/origin/main` 同步） | `git show-ref` |
| 最新基线提交 SHA | `a1eaa93a3a44c439071a2b564b564e017f9bbcc5` | `git show-ref` / `git log` |
| 基线发布标签 | `v0.1.0` -> `06f9527b9318d6339baf00f23a3414f564e7c21c` | `git show-ref` |
| 子模块状态 | `7e33a2f672d18239ef49ab960075b1137a1d18e7 desktop/third_party/FluentUI (1.7.7-134-g7e33a2f6)` | `git submodule status`、`.gitmodules:1-3` |
| 历史提交总数 | `main` 上共 6 个提交（`bd27db7` .. `a1eaa93`） | `git log -n 10 --format="%H %s" origin/main` |

### 2.1 `main` 分支完整提交历史（6 个基线提交）

1. `a1eaa93a3a44c439071a2b564b564e017f9bbcc5` — `feat(desktop): integrate zhuzichu520/FluentUI via git submodule`
2. `826023394cf9dcb8885f0cc6aaa2586e6491dcd7` — `chore(release): standardize package naming and use QtIFW 4.8.1 for system-locale UI`
3. `2afd78251267258a7f58d8b6aef3ca57c4ec7329` — `fix(ci): use Qt 6.10.* for cross-platform aqtinstall and upgrade Node 24 actions`
4. `ef36c81bf8e98bfe80a1d7fe60f1670da73b1beb` — `fix: resolve Linux output dir collision and upgrade CI Qt to 6.12.0 for macOS`
5. `fd7b01778fd90c827a66ab946986f50f62b78b1c` — `fix(ci): avoid unsupported env context in build-firmware job name`
6. `bd27db75854c1bbcf613386fc8951eb476617716` — `chore: initial FireSystem monorepo with desktop CPack and CI release workflow`

### 2.2 R1 审计时发现的 2 处预先存在本地未提交改动

在启动任何 R2 开发之前，`git status` 与 `git diff` 显示 `main` 工作区存在以下 2 处本地未提交优化改动（合计 +10 行）：

1. **`desktop/CMakeLists.txt`（第 19–28 行）**：
   - 在 `add_subdirectory(third_party/FluentUI/src EXCLUDE_FROM_ALL)` 前后包裹 `set(QT_NO_STANDARD_PROJECT_SETUP ON)` / `unset(QT_NO_STANDARD_PROJECT_SETUP)`，避免与根工程第 14 行的 `qt_standard_project_setup(REQUIRES 6.8)` 冲突（原因为 `desktop/third_party/FluentUI/src/CMakeLists.txt:13-15` 内部再次调用了 `qt_standard_project_setup()`）。
   - 为 `fluentui` 目标附加 `/wd4996`（MSVC）或 `-Wno-deprecated-declarations`（GCC/Clang），抑制子模块内部 Qt 弃用警告。
2. **`desktop/src/FluentUIDeps.qml`（第 1 行）**：
   - 顶部增加 `// qmllint disable unused-imports`，抑制为 `qmlimportscanner` 保留的 10 个传递性 QML 模块导入告警。

按 R1 用户确认，上述 2 处改动已通过提交 `405c1b120b8c89efa9004a597357aaba42da857f`（`chore(desktop): preserve pre-existing FluentUI CMake and qmllint fixes`）完整保留在开发分支 `feat/r1-audit-plan-desktop-settings` 上，未改动或覆盖 `main`。

### 2.3 本地未跟踪产物清单（受 `.gitignore` 忽略）

依据 `.gitignore`（第 1–16 行），以下本地环境与构建产物属于未跟踪目录/文件：
- `desktop/builds/`（本地 `Qt-6.12.0-mingw_64-x86_64/{Debug,Release}` 预编译输出）
- `desktop/packages/`（本地 CPack 产物：`FireSystem-0.1.0-windows-x64-portable.zip`、`FireSystem-0.1.0-windows-x64-setup.exe`）
- `desktop/.vscode/`（本地 Qt 6.12.0 MinGW 构建/调试配置 `settings.json`、`tasks.json`、`launch.json`）
- `firmware/fire.code-workspace`（指向本机 `zephyr_workspace/zephyrproject`、`boot`、`app` 的工作区文件）
- `firmware/app/build/` 与 `firmware/boot/build/`（本地 Zephyr `kzb_f407` 骨架编译产物，含 `zephyr.bin` 18,320 B）

---

## 3. `fire/` 仓库逐文件盘点与工程完整性评估

### 3.1 仓库根目录概览

根目录仅包含：`.git/`、`.github/`、`desktop/`、`firmware/`、`.gitignore`（16 行）、`.gitmodules`（3 行）。**以下关键工程目录/文件在基准仓库中全部缺失**：
- `docs/`（架构、需求、接口与计划文档——由本轮 M1 补齐）
- `README.md`（项目入口说明）
- `west.yml`（West 清单固定与模块版本锁定文件）
- `common/`（跨端共享协议/校验/常量契约）
- `tests/`（根目录或跨模块自动化测试目录）

### 3.2 `firmware/boards/lince/kzb_f407/`（10 个文件，实质性硬件板级 BSP）

该目录是仓库中最具工程成熟度的核心部分，面向 `STM32F407ZET6`（`KZB-V1.1` 控制板）实现了完整的 Zephyr 板级定义、早期安全 IO 钳位与冻结分区契约：

| 文件路径 | 行数 | 核验内容与工程作用 | 现状与缺口评估 |
|---|---:|---|---|
| `board.yml` | 6 | 定义 `name: kzb_f407`、`full_name: Lince KZB-V1.1 STM32F407ZE Controller`、`vendor: lince`、`socs: [stm32f407xx]`（`:1-6`） | 完整 |
| `board.cmake` | 4 | 配置 J-Link SWD 烧录器参数 `--device=STM32F407ZE --iface=swd --speed=1000 --reset`（`:3-4`） | 完整 |
| `CMakeLists.txt` | 3 | 注册板级库并编译 `board.c`（`:1-3`） | 未包含 `bsp_clock_sram.c` |
| `Kconfig` | 4 | 定义 `KZB_EARLY_SAFE_IO`（`default y`，依赖 `BOARD_KZB_F407`，`:1-4`） | 完整 |
| `Kconfig.kzb_f407` | 3 | 选中 `SOC_STM32F407XE` 与 `BOARD_EARLY_INIT_HOOK`（`:1-3`） | 完整 |
| `kzb_f407_defconfig` | 7 | 启用 `ARM_MPU`、`HW_STACK_PROTECTION`、`SERIAL`、`CONSOLE`、`UART_CONSOLE`、`GPIO`、`PINCTRL`（`:1-7`） | 缺少应用/Boot 所需的 Flash/Net/CAN/Watchdog 默认配置（留待 `prj.conf` 分层开启） |
| `kzb_early_state.h` | 35 | 定义全局快照结构体 `struct kzb_early_board_state`（`:19-30`）：`rcc_csr`、`dbgmcu_idcode`、`flash_size_kib`、`uid_words[3]`、5 路早期引脚回读及 `early_clamp_ok` 标志 | 完整 |
| `board.c` | 94 | 实现 `board_early_init_hook()`（`:44-94`）：采用**先写 `BSRR` 再切换 `MODER`** 的无毛刺时序（`:23-36`）钳位 `PE5`(`DR1=0`)、`PE6`(`DR2=0`)、`PC8`(`RS485-A DE=0`)、`PF11`(`RS485-B DE=0`)、`PG8`(`LED TX=1`) 并回读 `IDR` 校验（`:58-76`）；锁存 `RCC->CSR`（仅 `#ifndef CONFIG_MCUBOOT` 时置位 `RMVF`，使应用层可读取复位原因，`:83-86`）；读取 `DBGMCU->IDCODE`、Flash 容量（`0x1FFF7A22U`）及 96 位 UID（`0x1FFF7A10U..18U`）（`:89-93`） | 完整且符合安全要求 |
| `kzb_f407.dts` | 240 | 完整外设树：8 MHz HSE bypass -> 168 MHz PLL（APB1 42 MHz / APB2 84 MHz，`:35-61`）；`&i2c3` 禁用（`D43`，`:63-66`）；`flash0` 内部 512 KiB 分区（`boot_partition` 128 KiB、`slot0_partition` 384 KiB，`:68-84`）；`usart1`(115200)、`usart6`/`usart3`(19200)、`can1`/`can2`(500 kbps)、`mac` RMII + `lan8720a_phy`、`spi2` + `w5500_eth`、`spi3` + `nor_flash`（`jedec-id = [ef 40 21]` 128 MiB 4 字节寻址与 9 个固定分区，`:160-226`）、`usbotg_fs`、`rng`、`iwdg`、`zephyr,user` 安全 IO 引脚（`:21-32`） | **存在明确缺口**：① `:42` 注释引用 `bsp_clock_sram.c`，但该文件在仓库中**不存在**；② 缺失外部 FSMC SRAM (`U18` `IS62WV51216BLL`) 设备树节点；③ `&clk_lse` 当前为 `disabled`；④ RS-485 DE 引脚暂放于 `zephyr,user`（未绑定到 `&usart6`/`&usart3` 原生 `rs485-de-gpios`）；⑤ `w5500_eth` 使用 `zephyr,random-mac-address` 且 SPI 速率保守为 `1000000` (1 MHz) |
| `partitions.json` | 150 | 冻结分区契约 `schemaVersion: "kzb.partitions.v1"`、`freezeStatus: "frozen-p09"`（`:2-3`）：MCUboot `swap-scratch`、`headerSize: 1024`、`align: 1`、`maxSectors: 96`、`trailerReserveBytes: 4096`、`maxSignedImageBytes: 389120`、`ecdsa-p256`、`security-counter`、`uuidVidNamespace: "lince.firesystem"`、`uuidCidName: "kzb_f407_v1.1"`（`:7-18`），完整对齐 `flash0` 与 `nor_flash` 各分区偏移/容量 | 完整；其内部注释已指明 `P10`/`P11` 阶段才会实现 `identity`/`config-a`/`config-b`/`action-journal` 业务层 |

### 3.3 `firmware/app/` 与 `firmware/boot/`（空白示例模板）

| 目录 | 跟踪文件清单 | 真实内容核验 | 完整性结论 |
|---|---|---|---|
| `firmware/app/` | `CMakeLists.txt` (8L)、`prj.conf` (1L)、`README.rst` (33L)、`src/main.c` (14L) | `README.rst:1-33` 与 `src/main.c:1-14` 为 Zephyr `samples/hello_world` 原样复制（`printf("Hello World! %s\n", CONFIG_BOARD_TARGET);`）；`prj.conf:1` 为 `# nothing here` | **空白占位模板**：无任何外设驱动、通信栈、故障诊断、存储管理、工作流引擎或 OTA 代理代码 |
| `firmware/boot/` | `CMakeLists.txt` (8L)、`prj.conf` (1L)、`README.rst` (33L)、`src/main.c` (14L) | 与 `firmware/app/` 完全相同的 `samples/hello_world` 原样复制 | **空白占位模板**：未集成 MCUboot、`sysbuild`、ECDSA-P256 公钥、USB CDC ACM / UART 救援或 VID/CID 防降级策略 |

### 3.4 `desktop/`（Qt 6 + FluentUI 集成演示骨架与打包配置）

| 路径 | 行数 / 状态 | 核验内容与工程作用 | 完整性结论 |
|---|---|---|---|
| `desktop/CMakeLists.txt` | 140 行 | `project(FireSystem VERSION 0.1.0 LANGUAGES CXX)`（`:3`）；要求 `Qt6 6.8`（`Quick`, `QuickControls2`, `Widgets`, `PrintSupport`，`:7-12`）；静态构建 `third_party/FluentUI/src`（`FLUENTUI_BUILD_STATIC_LIB=ON`, `FLUENTUI_BUILD_EXAMPLES=OFF`，`:17-20`）；配置跨平台 CPack (`ZIP;IFW` / `TGZ;IFW`) 与自动探测 QtIFW（`:33-140`） | 打包框架成熟；尚无 `Qt6::Network`、`Qt6::Test`/`QuickTest`、`CTest` 与多子目录模块架构 |
| `desktop/src/CMakeLists.txt` | 69 行 | 定义目标 `FireSystemApp`（`OUTPUT_NAME "FireSystem"`，`WIN32_EXECUTABLE TRUE`，`:1,15-22`）与 QML 模块 `FireSystem.UI 1.0`（仅 `Main.qml`、`FluentUIDeps.qml`，`:7-13`），链接 Qt6 与 `fluentui`/`fluentuiplugin`，生成部署脚本（`:59-69`） | 仅最小启动骨架 |
| `desktop/src/main.cpp` | 34 行 | 设置 `QT_QPA_PLATFORM=windows:darkmode=2` 与 `QT_QUICK_CONTROLS_STYLE=Basic`（`:12-15`），注册 `FluentUI::registerTypes(&engine)`（`:23`），加载 `FireSystem.UI` `Main`（`:31`） | 仅最小启动入口 |
| `desktop/src/Main.qml` | 54 行 | 仅渲染 `960x640` 窗体，包含深浅色切换按钮、文本 `"FluentUI 集成成功"`（`:44`）及按钮 `"测试提示"`（`:50-51`） | **最小演示页**：无导航栏、无“设置”模块、无 RBAC/许可/多语言/后端服务 |
| `desktop/src/FluentUIDeps.qml` | 15 行 | 声明 10 个传递性 Qt QML 模块导入以供 `qmlimportscanner` 部署（`:1-15`） | 完整且需继续维护 |
| `desktop/cmake/CPackOptions.cmake.in` | 11 行 | 规范安装包命名：Windows IFW `*-setup`、Linux/macOS IFW `*-installer`、便携包 `*-portable`（`:1-11`） | 完整 |
| `desktop/installer/installscript.qs.in` | 34 行 | QtIFW 安装脚本，支持 Windows 快捷方式与 Linux `.desktop` 创建（`:1-34`） | 完整 |
| `desktop/third_party/FluentUI/` | 子模块 | `zhuzichu520/FluentUI` @ `7e33a2f6`（提供 `FluWindow`、`FluNavigationView`、`FluTableView`、`FluContentDialog` 等 90+ WinUI 3 控件） | 可直接复用，无需新增第三方依赖 |

### 3.5 `.github/workflows/release.yml`（281 行，仅 Tag 发布工作流）

`fire/.github/` 下仅有 `.github/workflows/release.yml`（281 行，9,785 字节），触发条件仅为 `push.tags: ['v*']` 与 `workflow_dispatch`（`:3-7`）：

| Job | 关键配置与工具链版本 | 发现的重大工程缺陷 |
|---|---|---|
| `build-desktop`（`:20-159`） | 3 平台矩阵（`windows-latest` MSVC、`ubuntu-latest` GCC、`macos-latest` Clang）；Qt `6.10.*`（`modules: 'qt5compat qtshadertools'`，`:13,68`）；QtIFW `4.8.1`（`:14`）；CMake + Ninja Release 构建并运行 `cpack` | **无测试、无静态检查**：未执行 `ctest`、`qmllint`、`clang-tidy` 或 Debug/Test 构建；且 CI Qt 版本（`6.10.*`）与本地开发机（`6.12.0` MinGW）不同，桌面源码必须严守 Qt `6.8–6.12` 兼容性 |
| `build-firmware`（`:161-259`） | `ubuntu-latest`；Zephyr `v4.5.0`（`:15`）、Zephyr SDK `1.0.1`（`arm-zephyr-eabi`，`:16`）、板型 `kzb_f407`（`:17`）；分别构建 `firmware/app` 与 `firmware/boot` 并输出 `.bin`/`.hex` | ① **隐藏断链风险**：第 208 行 `west update --narrow --fetch-opt=--depth=1 hal_stm32 cmsis_6 cmsis picolibc` **缺少 `mcuboot`、`mbedtls`、`zcbor`、`littlefs`**，一旦启用 MCUboot 或安全/存储子系统将直接构建失败；② 产物未签名（无 `imgtool`）；③ **0 个 `west twister` / `ztest` / `checkpatch` 步骤** |
| `release`（`:261-281`） | 仅在 `refs/tags/v*` 时汇总产物并发布 GitHub Release | **无 PR / `push` CI 门禁**：仓库不存在 `.github/workflows/ci.yml` |

### 3.6 模块完整性汇总矩阵

| 领域 | 在 `fire/` 中的现状 | 真实依据 |
|---|---|---|
| 仓库与打包/发布骨架 | **已具备（仅打包）** | `desktop/CMakeLists.txt`、`CPackOptions.cmake.in`、`installscript.qs.in`、`.github/workflows/release.yml`、`FluentUI` 子模块 |
| 硬件板级 BSP (`kzb_f407`) | **已具备核心基础（缺 `bsp_clock_sram.c` 与 FSMC DTS）** | `firmware/boards/lince/kzb_f407/` 10 个文件 |
| Bootloader (`firmware/boot/`) | **空白占位模板** | `hello_world` 原样复制（14 行 `main.c`） |
| 应用固件 (`firmware/app/`) | **空白占位模板** | `hello_world` 原样复制（14 行 `main.c`） |
| 桌面端核心服务与 UI (`desktop/src/`) | **仅 54 行集成演示窗体** | `Main.qml` 仅含提示按钮；无“设置”及任何业务模块 |
| 跨端协议/契约 (`common/`) | **完全缺失** | 无共享头文件、无协议编解码、无 Schema |
| 测试与 CI 质量门禁 (`tests/`, `ci.yml`) | **完全缺失（0 测试 / 0 Lint）** | `release.yml` 全文无任何测试或静态检查指令 |
| 工程文档 (`docs/`, `README.md`) | **完全缺失** | 本轮 M1 从零建立 `docs/` |

---

## 4. `new_fire` 存在登记、隔离边界与 R1 确认记录

### 4.1 `new_fire` 存在登记（仅记录目录名）

核查确认磁盘上存在 `c:\Users\JINGJINGPENG\Project\FireSystem\new_fire`，其顶层目录结构包括：
- `原理图.pdf`（硬件原理图文件）
- `common/`、`desktop/`（含 `prototype/`）、`docs/`、`firmware/`、`tests/`

### 4.2 严格隔离边界声明

依据 **R1 强制隔离规范**，团队全体成员遵守以下只读例外与访问禁令：
1. **允许只读参考范围（仅限以下 2 类目标）**：
   - 硬件原理图：`c:\Users\JINGJINGPENG\Project\FireSystem\new_fire\原理图.pdf`（`KZB-V1.1` `2025/4/10` STM32F407ZET6 原理图，用于核对硬件外设与引脚真实性）。
   - 上位机原型：`c:\Users\JINGJINGPENG\Project\FireSystem\new_fire\desktop\prototype\`（共 20 个 HTML/CSS/JS/Python 原型文件：`accounts.js`、`build_portable.py`、`FireSystem.html`、`firmware-packages.js`、`firmware.css`、`firmware.js`、`icons.js`、`index.html`、`locale.js`、`model.js`、`offline-files.js`、`operations.js`、`preview.py`、`service.css`、`service.js`、`studio.css`、`studio.js`、`train.js`、`workbench.css`、`workbench.js`，用于指导 R2 Qt 上位机“设置”界面设计及全平台原型需求对齐）。
2. **绝对禁区**：严禁打开、读取、复制或借用 `new_fire` 中除上述两项以外的任何源代码（`common/`、`firmware/`、`desktop/app/`、`desktop/core/`、`desktop/contracts/`、`desktop/ui/`、`tests/` 等）与架构/规划文档（`docs/`）。本轮文档编写工作自身甚至未直接打开 `new_fire` 内任何文件，全部经由前置专项勘察交接报告获取已核验的原理图与原型事实。

### 4.3 R1 门禁触发与用户确认记录

- **门禁触发（2026-11-25T11:59Z–12:18Z）**：因核查确认 `fire/` 仅为板级 BSP + 打包骨架 + `hello_world`/FluentUI 演示模板，触发 R1“若源码不完整则暂停向用户确认正确源码位置”门禁。
- **用户正式确认（2026-11-25T12:18:01Z）**：
  1. 确认以 `c:\Users\JINGJINGPENG\Project\FireSystem\fire`（`main @ a1eaa93`）作为正确演进起点，继续推进文档体系建立与 `fire\desktop`“设置”界面开发及独立审查。
  2. 确认保留 `desktop/CMakeLists.txt` 与 `desktop/src/FluentUIDeps.qml` 两处既有未提交修改，纳入后续开发分支统一管理。
