# 03 总体架构设计、模块清单与统一接口规范（R3）

> **设计原则**：① 嵌入式端严格以 **Zephyr RTOS v4.5.0 原生驱动与子系统 API** 为核心（**显式禁止以 STM32 HAL 替代或绕过 Zephyr 框架**）+ `STM32F407ZET6` + `MCUboot`；② 桌面端严格以 **Qt 6 (`>= 6.8`) + C++ + QML + `FluentUI` 原生能力**为唯一技术路线，禁止两套冲突 UI 风格与重复建设；③ 严禁未审批新增第三方依赖，严禁未经评审擅自修改跨模块公共接口；④ 凡当前无法确认的线缆协议字段或运行时细节一律标记为 **`无法确认`**。

---

## 1. 总体双端系统架构

```
+=======================================================================================+
|                            桌面端上位机 (Qt 6 + C++ + QML + FluentUI)                   |
|  +---------------------------------------------------------------------------------+  |
|  |  FireSystem.UI (QML / FluentUI WinUI 3)                                         |  |
|  |   [本轮 R2 仅交付] 设置壳层与 5 子页 (账号/许可/模板/通用/关于) + 最小登录           |  |
|  |   [后续 Phase 4]  运行总览/监控/告警 | 参数/协议/流程设计 | 控制/升级/日志/终端/帮助  |  |
|  +----------------------------------------+----------------------------------------+  |
|                                           | Q_PROPERTY / Q_INVOKABLE / Signals        |
|  +----------------------------------------v----------------------------------------+  |
|  |  FireSystemCore (C++ 静态库：鉴权/RBAC/离线文件/偏好/模板评审/i18n；后续含通信客户端) |  |
|  +----------------------------------------+----------------------------------------+  |
+===========================================|===========================================+
                                            | 统一通信协议框架 (以太网/USB/RS485/CAN — 协议细节待确认)
+===========================================|===========================================+
|                     嵌入式端 (STM32F407ZET6 + Zephyr RTOS v4.5.0 + MCUboot)             |
|  +----------------------------------------v----------------------------------------+  |
|  |  应用固件 (firmware/app, slot0 @ 0x08020000, 384 KiB)                            |  |
|  |   comm_mgr | mgmt_server | workflow_engine | safety_io | storage_mgr | ota_agent |  |
|  |   bsp_diag (zbus 事件总线 / watchdog / logging / bootutil 试启动确认)             |  |
|  +----------------------------------------+----------------------------------------+  |
|  |  Zephyr 原生驱动与板级 BSP (kzb_f407: board.c 早期钳位 + bsp_clock_sram + DTS)    |  |
|  +----------------------------------------+----------------------------------------+  |
|  |  MCUboot (firmware/boot @ 0x08000000, 128 KiB): ECDSA-P256 / VID+CID / 救援口     |  |
|  +---------------------------------------------------------------------------------+  |
|  硬件底座 (KZB-V1.1): 512 KiB Flash | 1 MiB FSMC SRAM | 128 MiB SPI3 NOR | 双网口/CAN/485 |
+=======================================================================================+
```

### 1.1 嵌入式端技术路线约束（`STM32F407ZET6` + Zephyr + MCUboot）

1. **Zephyr 原生优先、禁止 STM32 HAL 替代**：
   - 外设访问必须通过 Zephyr DeviceTree（`kzb_f407.dts`）与标准子系统 API：`gpio`、`uart`、`can`、`net`（`eth_stm32_hal` + `lan8720a_phy`、`wiznet,w5500`）、`flash_map` / `nvs` / `littlefs`、`watchdog`（`&iwdg`）、`entropy`（`&rng`）、`usbd`（`&usbotg_fs`）、`zbus`、`logging`、`bootutil`。
   - **唯一允许的直接寄存器操作例外**：已冻结在 `firmware/boards/lince/kzb_f407/board.c` 中的 `board_early_init_hook()`（在 Zephyr 驱动尚未初始化前写 `BSRR`/`MODER` 完成无毛刺安全钳位及锁存 `RCC->CSR`、`DBGMCU->IDCODE`、`UID`），以及后续 `bsp_clock_sram.c` 在内核初始化早期执行的 `LSEBYP` 有界超时探测与 FSMC 控制器寄存器配置。应用层代码**严禁**绕过 Zephyr API 直接调用 `HAL_GPIO_*`、`HAL_UART_*`、`HAL_CAN_*` 等 STM32Cube HAL。
2. **冻结存储分区契约（`partitions.json` `kzb.partitions.v1` / `frozen-p09`）**：

| 存储芯片 | DT 节点 / 标签 | 起始偏移 | 容量 | 用途与安全约束 |
|---|---|---:|---:|---|
| `flash0` (`0x08000000`, 512 KiB) | `boot_partition` / `mcuboot` | `0x00000000` | `0x00020000` (128 KiB) | 扇区 0–4，MCUboot（ECDSA-P256 + 防降级 + VID/CID + USB/UART 救援） |
| `flash0` | `slot0_partition` / `image-0` | `0x00020000` | `0x00060000` (384 KiB) | 扇区 5–7，主执行镜像（`headerSize=1024`，`maxSignedImageBytes=389120`，`trailer=4096`） |
| `nor_flash` (W25Q01JV, 128 MiB) | `nor_reserved_guard` / `nor-guard` | `0x00000000` | `0x00100000` (1 MiB) | 出厂/探针保留保护窗口，应用层禁止擦写 |
| `nor_flash` | `slot1_partition` / `image-1` | `0x00100000` | `0x00060000` (384 KiB) | MCUboot 候选升级槽（96 × 4 KiB 扇区） |
| `nor_flash` | `scratch_partition` / `image-scratch` | `0x00160000` | `0x00020000` (128 KiB) | `swap-scratch` 交换区（>= 片内最大 128 KiB 扇区） |
| `nor_flash` | `identity_partition` / `identity` | `0x00180000` | `0x00010000` (64 KiB) | 板卡身份、序列号、确定性 MAC 与信任锚点 |
| `nor_flash` | `config_a_partition` / `config-a` | `0x00190000` | `0x00020000` (128 KiB) | 事务性配置库 A 区（双区原子切换） |
| `nor_flash` | `config_b_partition` / `config-b` | `0x001B0000` | `0x00020000` (128 KiB) | 事务性配置库 B 区（双区原子切换） |
| `nor_flash` | `action_journal_partition` / `action-journal` | `0x001D0000` | `0x00020000` (128 KiB) | 掉电安全电磁阀/气瓶动作状态日志 |
| `nor_flash` | `diag_log_partition` / `diag-log` | `0x00200000` | `0x00DF0000` (14,614,528 B) | 轮转诊断与审计日志存储 |
| `nor_flash` | `nor_boundary_test_partition` / `nor-boundary` | `0x00FFF000` | `0x00002000` (8 KiB) | 跨越 16 MiB (`0x01000000`) 4 字节寻址边界专用测试区 |

### 1.2 桌面端技术路线约束（Qt 6 + C++ + QML + FluentUI）

1. **工具链与版本兼容窗口**：源码与 CMake 必须同时兼容 **Qt 6.8（最低基线）– 6.10（CI `release.yml:13`）– 6.12（本地 MinGW 工具链）**，以及 **MSVC / MinGW GCC / Linux GCC / macOS Clang**；禁止使用仅 Qt 6.11+ 新增私有 API 或编译器专属扩展。
2. **UI 风格与分层解耦**：
   - **唯一 UI 体系**：所有界面采用 `desktop/third_party/FluentUI`（WinUI 3 控件体系：`FluWindow`、`FluNavigationView`、`FluScrollablePage`、`FluFrame`、`FluTableView`、`FluContentDialog` 等）配合自定义轻量化组件，禁止混用 Qt Widgets 对话框拼凑主视图或引入两套冲突主题系统。
   - **逻辑与视图分离**：所有业务状态、密码学（`Qt6::Network` `QPasswordDigestor`）、JSON Schema 校验、本地存储、6 语言字典均置于 C++ 静态库 `FireSystemCore` 中；QML (`FireSystem.UI`) 仅负责响应式绑定与交互渲染，从而支撑无头 `Qt6::Test` 与隔离沙盒 `Qt6::QuickTest`。

---

## 2. 完整模块清单与依赖关系矩阵

> **防重复建设红线**：同一能力在全仓库只能存在单一权威模块（例如：SHA-256 与公共帧校验放 `common/`；桌面端所有设置状态统一由 `FireSystemCore` 管理，禁止 QML 页面各自读写配置文件）。

### 2.1 全平台模块清单

| 所属端 | 模块名 | 规划/现存路径 | 核心职责 | 阶段归属 |
|---|---|---|---|---|
| **Boot** | `kzb_boot` | `firmware/boot/` | MCUboot 引导、ECDSA-P256 验签、VID/CID 与安全计数器防降级、USB/UART1 救援 | Phase 2 |
| **BSP** | `kzb_f407_bsp` | `firmware/boards/lince/kzb_f407/` | DTS、`board.c` 早期钳位、`kzb_early_state.h`、`partitions.json`；Phase 1 补齐 `bsp_clock_sram.c` 与 FSMC SRAM DTS | 已有 + Phase 1 |
| **FW App** | `bsp_diag` | `firmware/app/src/diag/` | 消费 `kzb_early_state`（含 `rcc_csr`）、执行试启动健康检查、`boot_write_img_confirmed()` 确认、IWDG 守护 | Phase 2 |
| **FW App** | `storage_mgr` | `firmware/app/src/storage/` | 封装 `identity`、`config-a/b` 双区事务读写、`action-journal` 掉电日志、`diag-log` 轮转日志及 `nor-boundary` 校验 | Phase 2 |
| **FW App** | `safety_io` | `firmware/app/src/io/` | `DR1/DR2` 电磁阀驱动、`ACT1/2` 动作与 `UDV1/2` 欠压反馈互锁校验、`P1` 按钮、`P2` LED（基于 Zephyr `gpio`） | Phase 3 |
| **FW App** | `comm_mgr` | `firmware/app/src/comm/` | 统一调度双网口（`LAN8720A`/`W5500`）、双 CAN（500 kbps）、双 RS-485（19200 bps）与 USB OTG FS | Phase 1 / 3 |
| **FW App** | `workflow_engine` | `firmware/app/src/workflow/` | 借鉴 IEC 61499/61131-3 的确定性功能块/状态机运行引擎，订阅 `zbus` 事件并驱动 `safety_io` | Phase 3 |
| **FW App** | `ota_agent` | `firmware/app/src/ota/` | 接收上位机固件分片写入 `slot1_partition`、校验头/计数器并请求重启切换 | Phase 2 / 3 |
| **FW App** | `mgmt_server` | `firmware/app/src/mgmt/` | 对上位机暴露设备识别、配置事务下发、日志导出、控制调试与升级命令 | Phase 3 |
| **Shared** | `common_contracts` | `common/`（规划新建） | 跨端共享错误码、分区/版本常量、SHA-256/CRC 算法、协议编解码与流程 IR 校验器（纯 C11 无 OS 依赖） | Phase 1 |
| **Desktop** | `FireSystemCore` | `desktop/src/core/` | **R2 已规划**：`AccountManager`、`OfflineFileManager`、`PreferencesManager`、`TemplateReviewManager`、`I18nCatalog`、`SettingsService`；**Phase 4 扩展**：设备发现/协议栈/工作流编译器 | R2 + Phase 4 |
| **Desktop** | `FireSystem.UI` (Settings) | `desktop/src/qml/` | **R2 范围**：`Main.qml`、`LoginPage`、`SettingsPage`（及 5 个子页：`Accounts`/`License`/`Templates`/`General`/`About`）+ 基础组件 | **R2 (本轮)** |
| **Desktop** | `FireSystem.UI` (Studio & Ops) | `desktop/src/qml/`（后续） | **Phase 4 范围**：`Overview`/`Monitor`/`Alarms`/`Config`/`Protocol`/`Workflow`/`Projects`/`Controls`/`Upgrade`/`Logs`/`Terminal`/`Help` | Phase 4 |
| **Desktop** | `DesktopComms` | `desktop/src/comms/`（后续） | 基于 `Qt6::Network`、`Qt6::SerialPort`、`Qt6::SerialBus` 实现与 `mgmt_server` 通信 | Phase 4 |

### 2.2 模块单向依赖关系矩阵

- **Bootloader**：`kzb_boot` -> `kzb_f407_bsp` + Zephyr `bootutil`/`mcuboot`（**禁止**依赖 `firmware/app` 或 `common` 高层业务）。
- **嵌入式应用内部**：
  - `bsp_diag` / `storage_mgr` / `safety_io` / `comm_mgr` -> 仅允许调用 Zephyr 原生子系统、`kzb_f407_bsp` 与 `common_contracts`；模块间通过 **`zbus` 主题与显式服务接口** 通信，禁止循环依赖。
  - `workflow_engine` -> 仅消费标准化传感器/IO 事件，通过 `safety_io` 接口申请动作并写入 `storage_mgr` (`action-journal`)，**禁止直接操作硬件寄存器或网络 Socket**。
  - `ota_agent` -> 依赖 `storage_mgr`/`flash_map` 与 Zephyr `bootutil`，**禁止绕过 `slot1_partition` 直接擦写 `slot0_partition`**。
- **桌面端内部**：
  - `FireSystem.UI` (QML) -> 仅绑定 `FireSystemCore` 暴露的 `Q_OBJECT` 门面（如 `SettingsService`、`I18nCatalog`），**禁止 QML 直接读写本地文件或内嵌密码学逻辑**。
  - `DesktopComms` -> 依赖 `common_contracts`（编解码）与 `Qt6::Network`/`SerialPort`/`SerialBus`，向 `FireSystemCore` 上报结构化领域模型。

---

## 3. 统一通信协议框架与公共跨模块接口规范

### 3.1 逻辑通道划分与物理接口现状

为避免后期各端各自定义私有报文导致耦合混乱，全系统统一规划 4 类逻辑通信通道，映射到硬件物理口：

| 逻辑通道 | 业务用途 | 物理承载接口（`kzb_f407.dts` 已定物理参数） | 当前契约状态 |
|---|---|---|---|
| **C1 管理与遥测通道** | 设备发现/心跳、IO 状态、流程状态遥测、控制指令、调试终端 | 双以太网 `LAN8720A` (RMII) / `W5500` (SPI2) 或 `USB OTG FS` | 物理分工（`LAN8720A` vs `W5500`）、传输端口、报头、加密套件（如 DTLS/TLS/明文）均为 **`无法确认`** |
| **C2 工程配置与发布通道** | 参数包（双区事务 `config-a/b`）、协议表、流程 IR 二进制包下发与激活 | 同 C1（带事务校验与回滚确认） | 分包格式、IR 二进制编码、校验与激活原语均为 **`无法确认`** |
| **C3 固件升级与救援通道** | OTA 镜像分块传输至 `slot1_partition`；Boot 阶段 `USB OTG FS` / `USART1` (115200) 救援 | 以太网（App OTA） / `USB OTG FS` & `USART1`（Boot 救援） | MCUboot 镜像格式已冻结为 `ecdsa-p256` + `header=1024`；线缆传输协议（如 SMP/MCUmgr vs 自定义分块协议）与物理救援触发条件均为 **`无法确认`** |
| **C4 现场总线采集通道** | 外部探测器/传感器数据采集与总线设备联动 | `CAN1`/`CAN2` (500 kbps, 采样点 875‰)、`USART6`/`USART3` RS-485 (19200 bps) | 外部传感器协议、CAN ID 分配、RS-485 从站地址与寄存器定义均为 **`无法确认`** |
| **C5 调试与诊断输出** | Zephyr Shell / 内核日志输出、日志分区拉取 | `USART1` (`PA9/PA10`, 115200 bps 控制台)；`diag-log` / `action-journal` 拉取 | `USART1` 物理速率已确认 (`115200`)；在线日志批量拉取报文格式 **`无法确认`** |

### 3.2 已明确/已冻结的公共跨模块接口清单

#### 接口 1：`kzb_early_state.h`（BSP -> Boot / App 早期状态快照）
- **文件**：`firmware/boards/lince/kzb_f407/kzb_early_state.h:19-34`
- **契约内容**：`extern struct kzb_early_board_state kzb_early_state;`，包含 `rcc_csr`、`dbgmcu_idcode`、`flash_size_kib`、`uid_words[3]`、`dr1_early`、`dr2_early`、`rs485a_de_early`、`rs485b_de_early`、`led_idle_early`、`early_clamp_ok`。
- **使用约束**：只允许 `board_early_init_hook()` 写入，App `bsp_diag` 只读查询（在读取 `rcc_csr` 评估上电/看门狗复位后再做上报）。

#### 接口 2：`partitions.json`（Bootloader / App / 构建脚本共享存储契约）
- **文件**：`firmware/boards/lince/kzb_f407/partitions.json:1-150`
- **契约内容**：`schemaVersion: "kzb.partitions.v1"`、`freezeStatus: "frozen-p09"`、`swapMode: "swap-scratch"`、`signatureType: "ecdsa-p256"`、`downgradePrevention: "security-counter"`、`uuidVidNamespace: "lince.firesystem"`、`uuidCidName: "kzb_f407_v1.1"`，及 `flash0` / `nor_flash` 全部精确偏移与容量。
- **变更门禁**：任何固件端代码与 `imgtool` 签名脚本必须以该文件与 `kzb_f407.dts` 为唯一事实源（Single Source of Truth），禁止硬编码不一致偏移量。

#### 接口 3：桌面端 `FireSystemCore` <-> `FireSystem.UI` 公共服务接口（R2）
为支撑 R2“设置”界面实现与跨轮次演进，`FireSystemCore` 对 QML 暴露稳定、可测试的服务门面：

1. **`SettingsService`（通过 `QQmlApplicationEngine::setInitialProperties` 注入 `Main.qml` `service` 属性的门面，`desktop/src/core/SettingsService.h:26-173`）**：
   - **常量属性（`CONSTANT`）**：`i18n`（`QObject*`，指向内部 `I18nCatalog`）、`templateId`、`templateBoardId`（`kzb_f407`）、`templateVersion`。
   - **只读状态属性（`NOTIFY stateChanged`）**：`currentPage`、`signedIn`、`licensed`、`unlocked`（`licensed && signedIn`）、`canManage`（`unlocked && role ∈ ['admin','debugger']`）、`canIssue`（`currentAccount != null && role == 'debugger'`）、`canRecover`、`currentAccountId`、`currentAccountRole`、`computerName`、`licenseNotAfter`、`licenseRequestId`、`recoveryRequestId`、`accountGeneration`、`lang`、`theme`、`accountsModel`、`templateReviewStatus`、`hasPendingSettingsImport`、`pendingImportHasAppearance`、`pendingImportAccountCount`、`issuedLicenseRequest`、`issuedLicenseError`、`issuedRecoveryRequest`、`issuedRecoveryError`、`lastError`。
   - **可写属性（`READ`/`WRITE`, `NOTIFY stateChanged`）**：`accountSearchQuery`（写方法 `setAccountSearchQuery`）。
   - **关键操作（`Q_INVOKABLE`）**：
     - 导航与鉴权：`pageAvailable(page)`、`accessLanding()`、`navigateTo(page)`、`signIn(id, password)`、`signOut()`；
     - 账号管理：`saveAccount(originalId, id, role, enabled, hasPassword, password)`、`changeOwnPassword(oldPassword, newPassword, confirmPassword)`、`deleteAccount(id)`；
     - 离线许可：`exportLicenseRequestToFile(fileUrlOrPath)`、`importLicenseFromFile(fileUrlOrPath)`、`clearLicense()`、`importLicenseRequestForIssue(fileUrlOrPath)`、`exportIssuedLicenseToFile(fileUrlOrPath, mode, years, notAfter)`；
     - 离线恢复：`exportRecoveryRequestToFile(fileUrlOrPath)`、`importRecoveryFromFile(fileUrlOrPath)`、`importRecoveryRequestForIssue(fileUrlOrPath)`、`exportIssuedRecoveryToFile(fileUrlOrPath)`；
     - 设置导入/导出：`exportSettingsToFile(fileUrlOrPath)`、`prepareSettingsImportFromFile(fileUrlOrPath)`、`confirmSettingsImport()`、`cancelSettingsImport()`；
     - 模板与外观：`recordTemplateReview()`、`exportTemplateReviewToFile(fileUrlOrPath)`、`setLanguage(lang)`、`setTheme(theme)`；
     - 辅助方法：`suggestedExportFileName(kind)`、`maxUtcYearOffset()`、`todayUtcIso()`。
   - **信号**：`stateChanged()`、`toastRequested(message, kind)`。
2. **`I18nCatalog`（6 语言实时响应式服务，`desktop/src/core/I18nCatalog.h:12-35`）**：
   - 属性 `lang`（`zh-CN | en | ru | de | fr | es`，`WRITE setLang`）、`revision`（均绑定 `NOTIFY langChanged`）与 `languageOptions`（`CONSTANT`）；方法 `tr(zhSource, arg0)`、`t(zhSource, rev, arg0)` 与信号 `langChanged()`，随 `lang` 变更即时触发 QML 文本重求值；保留技术标识（`kzb_f407`、`sim.baseline_ring`、`SIM-ROOT-01`、`1.0.0-rc1`、`WF02-v1`、`p04.config.draft1`）不翻译。
3. **`PreferencesManager`（外观与语言持久化，`desktop/src/core/PreferencesManager.h:12-25`）**：
   - 负责 `PreferencesData`（`lang`、`theme`，其中 `theme` 取值 `system | light | dark`）在应用配置目录的 `load()`/`save()` 持久化（测试中可注入 `QTemporaryDir`），并通过 `SettingsService::lang` / `theme` 与 `setLanguage()` / `setTheme()` 同步驱动 `FluTheme.darkMode`。

#### 接口 4：桌面端本地 JSON 数据交换契约（对齐 `new_fire\desktop\prototype`）
- **账号库（`accounts.json`, `schemaVersion: 3`, `presetRevision: 1`）**：`generation`、`debug` 对象与 `managed` 数组；口令哈希固定为 PBKDF2-HMAC-SHA256 (`iterations=600000`, `salt` 32 位小写十六进制, `hash` 64 位小写十六进制)。
- **离线许可/恢复文件（`version: 2`, 最大导入 `256 KiB`）**：
  - `firesystem.prototype.license.request` / `firesystem.prototype.license`（当前标记 `"demo": true, "scope": "workstation"`，生产 PKI 签发体制标注 **`无法确认`**）
  - `firesystem.prototype.recovery.request` / `firesystem.prototype.recovery`（绑定 `machineId` + `generation` 防重放）
- **设置备份文件**：`firesystem.prototype.settings`（`version: 1`，含 `appearance: {lang, theme}` 与无 `token` 的 `accounts` 数组），导入同时兼容原型历史格式 `firesystem.prototype.backup` (`version: 2`)。
- **模板复核清单**：`template-review.json`（`demo: true`, `template`, `boardId: "kzb_f407"`, `version`, `review`, `deployable: false`）。

### 3.3 跨模块公共接口变更审批门禁

**严禁任何角色未经审查直接修改以下跨模块公共接口**：
1. `firmware/boards/lince/kzb_f407/kzb_f407.dts`、`partitions.json`、`kzb_early_state.h`；
2. 未来 `common/` 下的协议帧结构体、枚举错误码与流程 IR Schema；
3. 桌面端 `FireSystemCore` 对 QML 暴露的公共属性/方法签名及本地 JSON Schema 版本字段。

**变更流程**：提出变更 RFC（附跨模块影响面清单）-> 系统架构师 + 通信/嵌入式/桌面端/QA 相关方联合会审 -> 同步更新对应测试用例与文档 -> 审批通过后合入。

---

## 4. 6 大角色职责分工与文件所有权矩阵

| 角色 | 核心技术职责 | 主责目录 / 文件所有权 | 禁止越权修改清单 |
|---|---|---|---|
| **1. 嵌入式研发** | 基于 Zephyr 原生 API 开发时钟/SRAM BSP、`storage_mgr`、`safety_io`、`comm_mgr` 驱动适配、`workflow_engine` 与 `bsp_diag` | `firmware/app/src/**`、`firmware/boards/lince/kzb_f407/bsp_clock_sram.c`（新增） | 不得擅自修改 `firmware/boot/`、`partitions.json`、`desktop/` 及 `common/` 公共协议签名 |
| **2. Qt 桌面端研发** | 基于 Qt 6 + C++ + QML + `FluentUI` 开发 `FireSystemCore` 与 `FireSystem.UI`、i18n/主题与 CPack 打包兼容 | `desktop/CMakeLists.txt`、`desktop/src/**`、`desktop/cmake/**`、`desktop/installer/**` | 不得擅自修改 `firmware/`、`common/` 协议定义或未经批准引入第三方库；本轮严禁开发“设置”之外的子页面 |
| **3. 通信协议研发** | 定义并维护 C1–C5 统一通信协议框架、线缆帧格式、安全加密、错误码、`common/` 编解码器与测试向量 | `common/**`（规划）、`docs/03_*` 协议章节 | 不得绕过架构评审修改硬件总线 DTS 参数，不得直接修改 UI 表现层代码 |
| **4. Bootloader 与固件升级研发** | 维护 `firmware/boot/`（MCUboot 配置、`ecdsa-p256`、`security-counter`、VID/CID、USB/UART 救援）、`imgtool` 签名与试启动回滚策略 | `firmware/boot/**`、`firmware/app/src/ota/**`（接口协同）、签名脚本与分区消费逻辑 | 不得擅自改动 `partitions.json` 偏移而不经架构与嵌入式联合审批；不得删减 `board_early_init_hook` 安全钳位 |
| **5. 系统架构设计** | 统筹总体双端架构、模块边界、公共接口审批、硬件/协议 `无法确认` 项澄清推进、第三方依赖 7 项评估把关及 `main` 分支合并审批 | `docs/**`、`west.yml`（规划）、跨模块公共契约评审签字 | 不得跳过独立审查直接发布，不得隐瞒未确认事项或擅自猜测外部硬件规范 |
| **6. 测试与质量保证** | 建立并维护 10 类测试体系（`ctest`/`QtQuickTest`/`ztest`/协议模糊与边界/故障注入/升级回滚/CI `ci.yml`/HIL 验收），执行发布门禁审查 | `desktop/tests/**`、`tests/**`（规划）、`.github/workflows/ci.yml`（规划） | 不得为使测试通过而修改被测业务代码中的安全校验或硬编码假结果（遵守绝对真实性） |

---

## 5. 第三方依赖管控政策与 7 项评估机制

### 5.1 本轮（Round 1）依赖冻结声明

- **本轮新增未批准第三方依赖数量：0**。
- 桌面端仅允许使用：
  1. 仓库既有子模块 `desktop/third_party/FluentUI`（`7e33a2f6`, Apache-2.0 / MIT 既已纳入基线）；
  2. Qt 6 官方核心与测试模块（`Qt6::Quick`、`Qt6::QuickControls2`、`Qt6::Widgets`、`Qt6::PrintSupport`、`Qt6::Network`、`Qt6::Test`、`Qt6::QuickTest`，均包含在 `qtbase`/`qtdeclarative` 中；注意若后续新增超出 `release.yml:68` `qt5compat qtshadertools` 之外的独立 Qt 插件模块如 `Qt6::SerialPort`/`Qt6::SerialBus`，须在变更 CI 脚本前同步说明）。

### 5.2 重大第三方依赖准入 7 项强制评估模板

未来任何阶段（如 Phase 1+）拟引入新第三方依赖（包括但不限于：在 `release.yml:208` 启用 Zephyr 官方模块 `mcuboot`、`mbedtls`、`zcbor`、`littlefs`，或桌面端潜在新库）前，提出方**必须**提交包含以下 **7 项** 内容的书面评估，经架构师审查并获得用户确认后方可使用（当前尚未引入的候选模块均标注为 **`待用户确认`**）：

| 评估序号 | 强制评估维度 | 审查要点与准入标准 |
|---|---|---|
| **1** | **功能与适用场景** | 精准界定解决的具体问题、在双端架构中的边界，确认无仓库既有模块重复建设。 |
| **2** | **技术栈兼容性** | 验证是否兼容 Zephyr `v4.5.0` / SDK `1.0.1`（或 Qt `6.8–6.12` + Windows MSVC/MinGW + Linux + macOS 全矩阵）及现有 CMake/Kconfig 构建体系。 |
| **3** | **项目活跃度与维护情况** | 核查上游提交频率、Issue 响应周期、LTS 支持状态、是否为 Zephyr/Qt 官方上游维护模块。 |
| **4** | **开源许可证** | 明确许可证类型（MIT / BSD-3 / Apache-2.0 优先），排查 GPL/AGPL 等强传染性许可证对商业/闭源发布的影响。 |
| **5** | **安全性与已知风险** | 核查历史 CVE、密码学实现合规性（如恒定时间比较、内存安全、Fuzz 覆盖情况）及供应链来源可信度。 |
| **6** | **Flash / RAM / 性能及依赖开销** | 针对 `STM32F407ZET6`（内部 `slot0` 仅 384 KiB，片内 SRAM 192 KiB）严格量化 ROM/RAM 占用与堆栈开销，避免溢出 `maxSignedImageBytes=389120`；桌面端量化安装包体积与传递依赖数。 |
| **7** | **与原生实现优缺点对比** | 对比“采用该依赖”与“基于 Zephyr/Qt6 原生 API 自研实现”的工期、长期维护成本、缺陷风险与可替代性结论。 |

#### 后续阶段候选第三方/Zephyr 模块预登记清单（状态：`待用户确认`）

1. **`mcuboot` + `mbedtls` + `zcbor`（Zephyr 官方托管模块，Phase 2 候选）**：用于 `firmware/boot` 实现 `partitions.json` 已规定的 `ecdsa-p256`、`swap-scratch`、`security-counter` 与 MCUmgr CBOR 救援；需在 Phase 1 末完成上述 7 项评估（重点核算 `slot0` 384 KiB 与 `boot_partition` 128 KiB 空间余量），获用户确认后同步更新 `release.yml:208` 与 `west.yml`。
2. **`littlefs`（Zephyr 官方托管模块，Phase 2 候选）**：用于 `nor_flash` 上 `diag-log` 或 `config-a/b` 掉电安全存储方案对比（对比直接基于 `flash_map` + 双区校验记录或 `nvs` 的原生实现优缺点）。
3. **桌面端后续通讯辅助库（Phase 4 候选）**：优先采用 Qt 6 原生 `Qt6::Network`（含 `QDtls`）、`Qt6::SerialPort`、`Qt6::SerialBus`（`QCanBus`），原则上无需引入第三方网络/串口库；如确有特殊工业协议需求须走 7 项评估。
