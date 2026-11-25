# 02 系统需求清单与未确认事项登记表（R3）

> **编制依据**：本文档综合以下 4 项已核验输入编制——① 用户最终产品目标与 R1–R5 约束；② `fire/firmware/boards/lince/kzb_f407/` 硬件板级 BSP（`board.c`、`kzb_f407.dts`、`partitions.json`）；③ 硬件原理图 `new_fire\原理图.pdf`（`KZB.SchDoc`, `KZB-V1.1`, `2025/4/10`）；④ 上位机原型 `new_fire\desktop\prototype\` 交互与功能定义。**凡当前证据无法证实的硬件芯片、总线协议、安全根或运行时边界，均显式标注为 `无法确认`。**

---

## 1. 产品目标与系统总览

FireSystem 通用灭火系统平台面向**非专业现场调试与运维人员**及**研发/工程交付人员**，构建由以下两部分协同组成的双端高可靠消防控制平台：

1. **嵌入式端（现场主控单元 `KZB-V1.1`）**：基于 `STM32F407ZET6`（168 MHz Cortex-M4, 512 KiB 片内 Flash）+ `Zephyr RTOS (v4.5.0)` + `MCUboot`，外接 1 MiB FSMC SRAM（`U18`）与 128 MiB SPI3 NOR Flash（`U25` `W25Q01JVZEIQ`），提供多总线通信、双路 24 V 电磁阀隔离驱动与动作/欠压反馈闭环检测、确定性联动逻辑执行、掉电安全日志与高安全固件升级/救援能力。
2. **桌面端（上位机工作台与工程工具）**：基于 `Qt 6 (>= 6.8)` + `C++` + `QML` + `FluentUI`（WinUI 3 风格），提供统一的系统设置与权限/许可治理、图形化工作流与协议配置、实时监控告警、离线仿真验证、固件升级及多平台安装包（Windows/Linux/macOS）。

---

## 2. 硬件平台基线核验摘要（`KZB-V1.1` vs `kzb_f407.dts`）

| 硬件子系统 | 原理图 (`KZB.SchDoc`) 核验事实 | `kzb_f407.dts` / BSP 映射状态 | 备注 / 风险点 |
|---|---|---|---|
| 主控与时钟 | `U16`=`STM32F407ZET6`；`X3` 8 MHz 有源晶振 (`PH0`)；`U21` 32.768 kHz 有源晶振 (`PC14`)；`U23` `LM385M3-2.5` 基准源 | HSE 8 MHz bypass -> PLL 168 MHz (`kzb_f407.dts:35-61`)；`&clk_lse` 暂禁用（`:41-44`） | 缺失 `bsp_clock_sram.c` 的 LSEBYP 有界超时探测 |
| 外部 RTC | `U22`（未标型号 8 脚 I2C RTC）+ `U20` + 电池 `B1` (`PC9`/`PA8` I2C3) | `&i2c3` 永久禁用（`/* D43: External U22 RTC is unused and battery-free */`, `:63-66`） | 软件按 `D43` 禁用；实际硬件型号与是否贴片需确认 |
| 外部存储器 | `U18` `IS62WV51216BLL-55TLI`（1 MiB FSMC SRAM）；`U25` `W25Q01JVZEIQ`（128 MiB SPI3 NOR） | `nor_flash` 已映射 9 个固定分区（`:166-225`）；**FSMC SRAM 未在 DTS 定义** | 需补齐 FSMC SRAM 初始化及（若使用）DTS 节点 |
| 双以太网 | `U19` `LAN8720A`（RMII）；`U17` `W5500`（SPI2，`R44/R47/R50` 标注 `预留`）；均经磁耦 `HX1188NLT` 引出至 `CN6` | `&mac` + `lan8720a_phy` (`:123-142`)；`&spi2` + `w5500_eth` (`:144-158`) | 生产时双网口职责分工（维护网口 vs 业务网口）**无法确认** |
| 双隔离 CAN | `U6`/`U10` `ISO1050DUBR`，TVS/PPTC 保护，拨码 `SW2` 切 120 Ω 终端电阻，接 `CN3` | `&can1` (`PB8`/`PB9`), `&can2` (`PB5`/`PB6`), 均 500 kbps / 875‰ 采样点 (`:107-121`) | 上层协议（CANopen/J1939/自定义帧）**无法确认** |
| 双隔离 RS-485 | `U7`/`U13`（未标型号 16 脚隔离收发器），拨码 `SW1` 终端电阻，接 `CN2` | `&usart6` (19200, DE=`PC8`)、`&usart3` (19200, DE=`PF11`) (`:29-30,93-105`) | `U7`/`U13` 具体芯片型号及总线应用层协议**无法确认** |
| 调试与 USB | `P3` (`USART1` `PA9/PA10`)；`P4` (SWD)；`P5` (NRST)；`P6` (`USB OTG FS` `PA11/PA12`) | `&usart1` 115200 控制台 (`:86-91`)；`&usbotg_fs` (`:228-232`) | 用于控制台输出与 MCUboot USB/UART 救援 |
| 电磁阀执行与反馈 | 驱动 `PE5`/`PE6` -> `U4`/`U5` (`CYTLP127`) -> `K1`/`K2` (`HF49FD`) -> `CN4`/`CN5`；动作反馈 `ACT1/2` (`PF6`/`PF7`)；欠压反馈 `UDV1/2` (`PE3`/`PE4`) | `zephyr,user` 已映射 `dr1/dr2`（高有效）与 `cn4-4/6`、`cn5-4/6`（低有效）（`:21-27`）；`board.c:58-62` 上电无毛刺钳位为安全态 | 原理图 `CN5` 存在网络标号笔误（`MCU-ACT1-PF7`/`MCU-UDV1-PE4`），软件 DTS 映射本身正确；外接电磁阀电气特性**无法确认** |
| 其他孤立 IO | `P1` 光耦隔离按钮输入 (`U14` -> `PF10`)；`P2` 隔离单线输出 (`PG8` -> `U15` `ISO7710FDR` -> `LED-SDA`) | `button-gpios` (`PF10`, 低有效)、`led-tx-gpios` (`PG8`, 空闲高) (`:28,31`) | `P1` 现场按钮语义与 `P2` `LED-SDA` 通信时序协议**无法确认** |
| 电源与安全 | `CN1` 24 V 输入 -> 双隔离 `URB2405S-10WR3` (`U1`/`U2`) + `AMS1117-3.3` (`U3`) | 片内 `rng` + `iwdg` (`:234-240`) | **无板载温度/烟感/压力传感器，无外部看门狗芯片** |

---

## 3. 系统功能需求清单

### 3.1 嵌入式核心安全与业务运行需求（`REQ-FW`）

| 需求编号 | 需求名称 | 详细规格与行为约束 | 来源依据 | 目标阶段 |
|---|---|---|---|---|
| `REQ-FW-01` | 上电前安全 IO 无毛刺钳位与自检 | 在 Zephyr 内核与外设驱动启动前（`board_early_init_hook`），先写 `BSRR` 再配置 `MODER`，强制 `PE5(DR1)=0`、`PE6(DR2)=0`、`PC8(RS485A_DE)=0`、`PF11(RS485B_DE)=0`、`PG8(LED_TX)=1` 并回读 `IDR` 写入 `kzb_early_state.early_clamp_ok`。应用启动后必须核查该标志，任一异常须阻止进入正常联动放行态并记录严重告警。 | `board.c:23-77`、`kzb_early_state.h:19-34` | Phase 1 / 2 |
| `REQ-FW-02` | 时钟稳健启动与外部 SRAM 初始化 | 实现 `bsp_clock_sram.c`：对 `U21` 32.768 kHz `LSEBYP` 执行**有界超时探测**防止晶振故障导致死机卡死；完成 `U18`（1 MiB FSMC SRAM）时序初始化与读写校验。 | `kzb_f407.dts:41-44`、原理图 `U18`/`U21` | Phase 1 |
| `REQ-FW-03` | 电磁阀闭环驱动与动作/欠压监测 | 通过 `dr1`/`dr2` 驱动 `CN4`/`CN5` 电磁阀，实时监测 `ACT1/2` 动作反馈与 `UDV1/2` 24 V 欠压反馈。若驱动指令态与反馈态超时不匹配（如未动作、粘连或欠压），立即触发安全切断、上报故障告警并记入掉电安全日志。 | `kzb_f407.dts:22-27`、原理图 `CN4`/`CN5` | Phase 3 |
| `REQ-FW-04` | 多总线数据采集与协同通信 | 基于 Zephyr 原生驱动管理 `CAN1/2`（500 kbps）、`RS-485 A/B`（19200 bps，含 DE 方向切换）、双以太网（`LAN8720A` + `W5500`）与 `USB OTG FS`，为流程引擎与上位机提供统一通道。具体应用层帧格式见“无法确认”表。 | `kzb_f407.dts:93-164,228-232` | Phase 1 / 3 |
| `REQ-FW-05` | 确定性工作流执行引擎 | 在嵌入式端运行由上位机编译下发的图形化流程定义（确定性周期调度、状态迁移、延时/计数/联锁保护与安全降级回退）。禁止动态任意脚本解释导致不可预测内存/时延抖动。 | 用户目标、原型 `workflow` | Phase 3 |

### 3.2 图形化配置与工作流需求（`REQ-CFG`，参考 IEC 61499 / IEC 61131-3）

| 需求编号 | 需求名称 | 详细规格与行为约束 | 标准边界声明 | 目标阶段 |
|---|---|---|---|---|
| `REQ-CFG-01` | 非专业人员图形化工程配置向导 | 允许现场或工程人员以可视化表单与图形化画布完成设备参数配置（IO 映射、阈值、联锁策略、模板选用）与一键发布，避免手写底层寄存器或 C 代码。 | 面向工程易用性设计，模板与配置模型需与 `kzb_f407` 强校验绑定 | Phase 4 |
| `REQ-CFG-02` | 图形化逻辑编排（借鉴 IEC 61499 与 IEC 61131-3 概念） | ① 借鉴 **IEC 61499 功能块（FB）**：以“输入事件/数据 -> 功能块 -> 输出事件/数据”有向连线表达传感器输入、表决逻辑（如双判据确认）、延时器与电磁阀动作输出；② 借鉴 **IEC 61131-3 周期扫描与状态转换（SFC/状态机概念）**：支持分阶段状态迁移（待机/预警/延时倒计时/释放/锁定/故障）、条件守卫与本地断点单步仿真。 | **明确裁剪声明**：本平台**不实现**完整 IEC 61499 分布式跨设备执行环境或编译器标准，亦**不实现** IEC 61131-3 全部编程语言（IL/LD/ST/FBD 完整规范），仅采纳其清晰的功能块封装与状态转移图工程心智模型。 | Phase 3 / 4 |
| `REQ-CFG-03` | 配置静态校验与双区事务提交 | 编译/发布流程 IR 与配置包前必须校验通道合法性、参数上下限、安全互锁（如禁止绕过欠压保护）；下发至板端时写入 `nor_flash` 的 `config-a`/`config-b` 双事务区，校验通过并原子切换后生效，断电或校验失败自动回退上一有效区。 | `partitions.json:98-116`（128 KiB × 2） | Phase 2 / 4 |

### 3.3 故障检测、诊断与掉电安全日志需求（`REQ-DIAG`）

| 需求编号 | 需求名称 | 详细规格与行为约束 | 来源依据 | 目标阶段 |
|---|---|---|---|---|
| `REQ-DIAG-01` | 复位原因跨引导传递与追踪 | MCUboot 阶段保留 `RCC->CSR` 不清除 `RMVF`，由应用固件读取 `kzb_early_state.rcc_csr` 识别上电复位、独立看门狗复位 (`IWDG`)、软件复位、欠压复位并清除标志后上报记录。 | `board.c:83-86`、`kzb_f407.dts:238-240` | Phase 2 |
| `REQ-DIAG-02` | 掉电安全动作/气瓶状态日志 (`action-journal`) | 在 `nor_flash` `0x001D0000`（128 KiB）维护掉电安全追加/环形事务日志，记录电磁阀驱动指令、气瓶/动作状态变化与关键安全事件，单条记录带 CRC/序号防半写入损坏。 | `partitions.json:118-126`、`kzb_f407.dts:210-213` | Phase 2 |
| `REQ-DIAG-03` | 轮转诊断与审计日志 (`diag-log`) | 在 `nor_flash` `0x00200000`（14,614,528 B ≈ 13.94 MiB）实现跨 16 MiB 边界安全兼容的轮转日志存储，记录系统异常、总线错误、自检结果、配置变更及升级事件，支持上位机按页/按时间检索导出。 | `partitions.json:128-146`、`kzb_f407.dts:215-223` | Phase 2 / 4 |
| `REQ-DIAG-04` | 独立看门狗 (`IWDG`) 协同守护 | 通过 Zephyr 原生 `watchdog` 驱动管理 `&iwdg`，仅当核心安全采样、反馈检测与通信任务按期健康心跳时喂狗；任务卡死或死锁由 IWDG 硬件强制复位并回到安全钳位态。 | `kzb_f407.dts:18,238-240` | Phase 2 |

### 3.4 固件升级与多级恢复需求（`REQ-OTA`）

| 需求编号 | 需求名称 | 详细规格与行为约束 | 来源依据 | 目标阶段 |
|---|---|---|---|---|
| `REQ-OTA-01` | MCUboot 可信验签启动 | `boot_partition`（128 KiB）运行 MCUboot，采用 `ecdsa-p256` 验证 `slot0_partition`（`image-0`, 384 KiB, `headerSize: 1024`, `maxSignedImageBytes: 389120`）与 `slot1_partition`（NOR `0x00100000`, 384 KiB, 96×4 KiB 扇区）镜像签名，使用 `scratch_partition`（NOR `0x00160000`, 128 KiB）执行 `swap-scratch` 搬运。 | `partitions.json:7-86`、`kzb_f407.dts:74-82,185-193` | Phase 2 |
| `REQ-OTA-02` | 防降级与硬件平台防错刷 | 强制启用 MCUboot `security-counter` 单调递增防降级保护，并在镜像 TLV/扩展校验中绑定 `VID`（命名空间 `lince.firesystem`）与 `CID`（`kzb_f407_v1.1`），拒绝非本板型或低安全计数器固件。 | `partitions.json:15-17` | Phase 2 |
| `REQ-OTA-03` | 试启动健康评估与自动回滚 | 新固件首次交换启动后处于未确认（Test）状态；仅当硬件基础自检（含 `early_clamp_ok`、存储挂载、看门狗与核心通道初始化）全部通过后才调用 `boot_write_img_confirmed()` 确认；否则下次复位/IWDG 超时由 MCUboot 自动回滚至原镜像。 | `kzb_early_state.h:5-7`、`partitions.json:8-15` | Phase 2 |
| `REQ-OTA-04` | 串行/USB 离线救援通道 | 当 `slot0` 与 `slot1` 均损坏或应用无法启动时，Bootloader 支持通过 `USB OTG FS`（`P6`）或 `USART1`（`P3` 115200 bps）进入救援模式接收合法签名镜像刷写。具体按键/进入条件与线上救援触发协议待确认。 | `partitions.json:35`、`kzb_f407.dts:86-91,228-232` | Phase 2 |

### 3.5 Qt 桌面端需求（`REQ-DT`：本轮“设置”模块 vs 后续工作台规划）

#### A. 本轮（Round 1 / M2）严格范围——“设置”模块需求
依据 `new_fire\desktop\prototype`（`workbench.js`、`service.js`、`accounts.js`、`offline-files.js`、`model.js`、`locale.js`），本轮仅在 `fire\desktop` 实现以下功能（其余业务子页面**严禁提前开发**）：

| 需求编号 | 模块/子页 | 核心功能需求 | 来源引用 |
|---|---|---|---|
| `REQ-DT-SET-01` | 设置首页 (`settings`) 与门禁壳层 | 提供设置导航入口与 5 个子页面入口卡片；未授权或未登录时仅开放 `login`、`settings`、`settings-accounts`、`settings-license`、`settings-general`、`settings-about`（`settings-templates` 必须被锁定隐藏）；仅当 `unlocked() && role ∈ ['admin','debugger']` 时在设置首页展示“导入设置 / 导出设置”面板（支持 `firesystem.prototype.settings` v1 与兼容旧版 `backup` v2，导入前二次确认并替换本地操作员/管理员账号后退出登录）。同时提供最小 `login` 登录/切换账号界面与顶部账号头像菜单以支撑 RBAC 验证。 | `workbench.js:64-68`、`service.js:818-845,956,1050-1059` |
| `REQ-DT-SET-02` | 账号管理 (`settings-accounts`) | 支持 3 类角色 RBAC：`operator`（操作员）、`admin`（管理员）、`debugger`（调试员，固定账号 `debug`，预设密码 `lince`；预设 `admin` 初始空密码）；口令采用 **PBKDF2-HMAC-SHA256（600,000 次迭代，16 B 随机盐，32 B 派生密钥）**；支持账号检索、增删改（禁止删除自身及 `debug`，`admin` 不可新建/删除 `debugger`）、修改自身密码；内置“最后管理员离线恢复”双端流程（请求代次 `generation` 防重放，调试员签发恢复文件）。 | `accounts.js:8-180`、`service.js:771-800,827-838` |
| `REQ-DT-SET-03` | 许可证管理 (`settings-license`) | 支持查看本机名、许可状态（有效/已失效）、UTC 有效期、待处理请求编号；支持导出许可请求文件（v2 JSON，256 KiB 导入上限校验）、导入许可文件、管理员/调试员清除许可（二次确认）；调试员可见右侧“签发许可文件”面板（支持按年数或按 UTC 截止日期签发）。明确标注当前处于本地仿真/原型演示域（`demo: true`）。 | `offline-files.js:1-112`、`service.js:733-770` |
| `REQ-DT-SET-04` | 项目模板 (`settings-templates`) | 仅在 `unlocked()` 后可见；展示当前模板（如 `sim.baseline_ring`）、适用板型（`kzb_f407`）与评审状态（待复核/已复核）；支持管理员/调试员记录模板复核并导出 `template-review.json`（含 `"demo": true, "deployable": false`）。 | `service.js:801-806,949`、`model.js:2-5` |
| `REQ-DT-SET-05` | 通用设置 (`settings-general`) | 严格对齐原型 `service.js:813-817`，仅提供“语言”（6 语言：`zh-CN`、`en`、`ru`、`de`、`fr`、`es` 实时生效并保留原始技术标识不翻译）与“界面主题”（`system` 跟随系统、`light` 浅色、`dark` 深色实时生效）两项配置及持久化存储。不提前堆砌原型 `locale.js:238-259` 中未渲染的冗余 Mock 选项。 | `service.js:813-817`、`model.js:31-43`、`workbench.js:33-46` |
| `REQ-DT-SET-06` | 关于与信任目录 (`settings-about`) | 展示“版本与兼容性”表（桌面兼容基线 `1.0.0-rc1`、项目模板 `1.2.1`、管理 API `1.0`、配置模型 `p04.config.draft1`、流程 IR `WF02-v1`）；在 `unlocked()` 后展示“信任目录”（目录域：仅演示；模板发布：演示模板评审；签发根：原型样例根 · `SIM-ROOT-01`；部署许可：未获生产签发），并提供展开信任声明交互。 | `service.js:807-812` |

#### B. 后续阶段规划（Round 1 审批通过后方启动）
- **运行监控域**：运行总览（`overview`）、设备监控与 IO/流程/标识视图（`monitor`）、告警中心（`alarms`）。
- **工程配置域**：参数配置与 JSONC 发布（`config`）、协议设计器（`protocol`）、图形化流程设计器与断点仿真（`workflow`）、项目切换（`projects`）。
- **维护工具域**：控制与气瓶管理（`controls`）、固件升级与验签下发（`upgrade`）、日志与审计查询（`logs`）、串口/CAN/Telnet 终端（`terminal`）、离线帮助手册（`help`）。

---

## 4. 非功能性需求（`REQ-NFR`）

| 维度 | 具体要求 |
|---|---|
| **安全性（Safety & Security）** | ① 上电微秒级完成继电器与总线方向安全钳位，未确认不得动作；② 固件启动链强制 ECDSA-P256 验签与安全计数器防降级；③ 桌面账号口令禁止明文存储，强制 PBKDF2-SHA256 60 万轮派生；④ 所有外部导入 JSON 限制体积（`<= 256 KiB`）并做严格字段类型校验。 |
| **可靠性与掉电鲁棒性** | 双区配置 `config-a/b` 与 `action-journal` 必须能承受任意时刻掉电而不破坏前一已提交状态；`nor-boundary` 分区验证 16 MiB（`0x01000000`）4 字节寻址切换零地址回绕错误。 |
| **技术栈纯洁性与风格统一** | 嵌入式端以 Zephyr 原生子系统为主，禁止引入 STM32 HAL 替代 Zephyr API；桌面端以 Qt 6 + C++ + QML + `FluentUI` 为唯一 UI 体系，代码兼容 `Qt 6.8–6.12` 及 `MSVC / MinGW / GCC / Clang`；禁止引入未经 7 项评估与用户批准的第三方库。 |
| **可测试性与 CI 门禁** | 桌面端核心逻辑须与 QML 视图解耦（静态库 `FireSystemCore`），支持无头 `ctest` 与 `QtQuickTest`；嵌入式业务须支持 `native_sim` 下的 `ztest` 仿真单测与真实台架 HIL 回归。 |

---

## 5. 明确“无法确认（UNCONFIRMED）”事项登记表

> **工程纪律**：以下所有事项在当前允许核验的仓库源码、`kzb_f407` BSP 与 `KZB-V1.1` 原理图中均**缺乏充分依据**，统一标注为 **`无法确认`**。研发与架构阶段**严禁擅自猜测或硬编码最终生产契约**，必须由对应负责人与硬件/系统方澄清后再冻结。

| 编号 | 待确认事项 | 当前已知事实 | 标注 `无法确认` 的具体内容与原因 | 影响模块 | 确认责任人 |
|---|---|---|---|---|---|
| `UNC-01` | RS-485 芯片 `U7`/`U13` 与 RTC `U22` 精确器件型号及贴片状态 | 原理图绘制 `U7`/`U13`（16 脚隔离 485）与 `U22`（8 脚 I2C3 RTC+`B1`）；DTS 注释 `D43` 称 `U22` 未使用且无电池而禁用 `&i2c3` | `U7`/`U13`/`U22` 原理图未标注具体型号；量产 BOM 是否贴装 `U22`/`B1` 及 `R44/R47/R50`（`预留`）配置 **无法确认** | `firmware/boards/lince/kzb_f407` | 硬件工程师 / 嵌入式研发 |
| `UNC-02` | 外部探测器与传感器接入方案及协议 | `KZB.SchDoc` 板载**无任何温度、烟感、压力或火焰传感器** | 现场外部探测器的物理通道归属（CAN1/2 vs RS-485 A/B）、从站地址、寄存器/报文格式、波特率（是否固定 19200 / 500k）、量纲与采样周期 **无法确认** | `comm_mgr`、`workflow_engine`、`protocol` | 系统架构师 / 硬件工程师 / 通信协议研发 |
| `UNC-03` | 外部硬件看门狗芯片是否存在 | 原理图未绘制外部独立看门狗芯片，DTS 仅启用片内 `&iwdg` | 系统级安全需求是否仅依赖 STM32 片内 `IWDG`/`WWDG`，抑或外部扩展板存在独立看门狗 **无法确认** | `bsp_diag`、硬件安全评估 | 硬件工程师 / 系统架构师 |
| `UNC-04` | 双以太网 `LAN8720A` 与 `W5500` 的生产功能分工 | 二者物理上共同引出至 `CN6` 接插件；原型 `studio.js` 中曾暗示维护/业务分工，但 `fire/` 仓库无任何协议/网络契约 | 两路以太网口在正式产品中的职责划分（如哪路用于上位机配置/OTA、哪路用于车厢/站台业务网如 TRDP/UDP）、IP 分配机制及是否支持冗余切换 **无法确认** | `comm_mgr`、桌面端通信层 | 系统架构师 / 通信协议研发 |
| `UNC-05` | `P1`（`BUTTON`）、`P2`（`LED-SDA`）及 `CN2`–`CN6` 外部电气与语义 | `P1` 经光耦接 `PF10`；`P2` 经 `ISO7710FDR` 接 `PG8` (`LED-SDA`)；`CN4/5` 接 24 V 电磁阀及 `ACT`/`UDV` | `P1` 是紧急释放、消音、复位还是救援触发按键 **无法确认**；`P2` `LED-SDA` 协议（WS2812 归零码 vs 自定义单线串口）**无法确认**；电磁阀额定电流/吸合保持脉冲时序与压力开关逻辑极性 **无法确认** | `safety_io`、`boot_rescue` | 硬件工程师 / 嵌入式研发 |
| `UNC-06` | 统一现场通信协议帧格式、端口与安全加密套件 | `fire/` 中 `common/` 与协议模块完全缺失；`kzb_f407.dts` 仅定死物理层基础速率（UART1 115200 / RS485 19200 / CAN 500k） | 以太网端口号、传输协议（UDP/TCP/TRDP/Modbus-TCP）、应用层报头/TLV/CRC 格式、加密传输方案（DTLS 1.2/1.3 PSK 或证书认证或明文）、CAN ID 分配规则（标准帧/扩展帧、CANopen/J1939）与心跳超时阈值 **无法确认** | `common/`、`comm_mgr`、`desktop` 通信客户端 | 通信协议研发 / 系统架构师 |
| `UNC-07` | 生产级 PKI / 离线签发根证书与许可密码学体制 | 原型仅使用 `SIM-ROOT-01` 演示标示，`offline-files.js` JSON 无真实非对称数字签名；`partitions.json` 仅定义固件端 `ecdsa-p256` | 桌面端正式许可文件/恢复文件的签名算法（如 Ed25519 / ECDSA-P256）、离线签发 CA 架构、硬件加密芯片或设备绑定证书格式 **无法确认** | `OfflineFileManager`、发布工具链、`identity` 分区 | 系统架构师 / 安全负责人 |
| `UNC-08` | 最终工作流 IR 二进制编码与运行周期约束 | 原型仅提供 `WF02-v1` 与 `p04.config.draft1` 本地仿真命名占位 | 嵌入式 `workflow_engine` 可接受的最终二进制 IR 指令集格式、最大功能块/状态节点数限制、RAM 预算（是否必须依赖 FSMC 1 MiB SRAM）与硬实时扫描周期（ms 级指标） **无法确认** | `workflow_engine`、`WorkflowStudio`、`common/` | 系统架构师 / 嵌入式研发 / Qt 桌面端研发 |
| `UNC-09` | 固件安全通道与救援触发物理条件 | `partitions.json:35` 注明 `USB rescue / UART / downgrade / VID+CID`；`boot/` 当前为空白模板 | 现场通过 `P6`(USB) 或 `P3`(UART1) 强制进入 MCUboot 串行救援的具体物理触发方式（如上电组合键/超时窗口/应用指令写保留 RAM 标志）与认证机制 **无法确认** | `firmware/boot/`、`upgrade` 页面 | Bootloader 研发 / 硬件工程师 |
