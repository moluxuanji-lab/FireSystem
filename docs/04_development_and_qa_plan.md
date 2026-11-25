# 04 多阶段整改开发计划、测试质量体系与交付治理（R4）

> **核心门禁约束**：本轮（**Round 1 / Phase 0**）交付 `docs/` 全部规划文档与 `fire/desktop/`“设置”界面及构建测试验证、并经独立审查通过后**立即暂停**，等待用户审查与书面批准后再推进 Phase 1 及后续全部模块开发。

---

## 1. 分阶段整改开发路线图与客观验收标准

### 阶段总览

| 阶段 | 名称 | 核心范围 | 前置依赖 | 阶段门禁 |
|---|---|---|---|---|
| **Phase 0 (Round 1)** | 仓库审计、架构规划与桌面“设置”界面 | `docs/01..04` + `docs/README.md`；`fire/desktop` 仅“设置”界面 + C++/QML 测试 + 本地构建验证 | R1 用户确认（已完成） | **Round-1 强制暂停门禁（等待用户审批）** |
| **Phase 1** | 公共契约冻结、BSP 补齐与 CI 基础建设 | 澄清 `UNC-01..09`“无法确认”项；建立 `common/`、`west.yml`、`.github/workflows/ci.yml`；补齐 `bsp_clock_sram.c` 与 FSMC SRAM DTS | Round-1 用户审批通过 + `UNC` 澄清 | CI 绿灯 + `kzb_f407` BSP 编译与静态检查通过 |
| **Phase 2** | Bootloader (`MCUboot`) 与固件高可靠底座 | 实现 `firmware/boot`（`ecdsa-p256`、`security-counter`、VID/CID、USB/UART 救援）；实现 `app` 侧 `bsp_diag`、`storage_mgr`（`config-a/b`、`action-journal`、`diag-log`、`nor-boundary`）与 `IWDG` | Phase 1 完成 + `mcuboot`/`mbedtls` 7 项评估批准 | 签名/防降级/回滚/掉电存储 `ztest` 与台架实测通过 |
| **Phase 3** | 嵌入式业务逻辑、总线采集与工作流引擎 | 实现 `safety_io`（`DR1/2`+`ACT/UDV` 闭环）、`comm_mgr`（双网口/双 CAN/双 485）、`workflow_engine`（IEC 61499/61131-3 风格确定性引擎）、`ota_agent`、`mgmt_server` | Phase 2 完成 + 协议与 IR 冻结 | `native_sim` 集成测试 + 故障注入测试通过 |
| **Phase 4** | Qt 上位机全量工作台与工程配置工具 | 在 R2 基础上扩展 `Operations`（总览/监控/告警）、`Engineering`（参数/协议/图形化流程/项目）、`Maintenance`（控制/升级/日志/终端/帮助）及 `DesktopComms` | Phase 1 协议契约 + Phase 3 固件联调接口 | 全量 `ctest` / `QuickTest` / `qmllint` + 双端模拟联调通过 |
| **Phase 5** | 真实硬件 HIL 全量联调与发布加固 | `kzb_f407` 硬件台架全覆盖测试（10 类测试第 10 项）、72h 耐久与故障注入、修复/完善 `release.yml` 签名与自动化打包发布 | Phase 3 & Phase 4 完成 + 真实 `KZB-V1.1` 硬件台架 | 10 类 QA 全部签署通过 + 维护者批准合入 `main` |

### 各阶段客观输入/输出与退出验收标准

#### Phase 0（Round 1 — 当前执行阶段）
- **交付物**：
  1. `docs/README.md`、`docs/01_repository_audit.md`、`docs/02_requirements_and_unconfirmed.md`、`docs/03_architecture_and_interfaces.md`、`docs/04_development_and_qa_plan.md`；
  2. `fire/desktop/` 下在开发分支实现的“设置”界面（设置首页 + 账号/许可证/项目模板/通用/关于 5 子页 + 最小登录与设置导入导出）及 C++/QML 测试；
  3. 独立审查（`reviewer` / `challenger` / `auditor`）报告。
- **客观退出标准**：
  - `firmware/` 与 `.github/` 保持 0 修改；`main` 分支保持洁净未被直接覆盖；
  - `fire/desktop` 在本地环境完成真实的 CMake 配置、编译、`ctest` 与 `qmllint` 检查并如实记录真实输出；
  - 文档中所有未确认硬件/协议细节均标为 `无法确认`，所有文件路径与 SHA 经独立核验无误。
- **【强制暂停门禁】**：本阶段全部交付后**必须停止推进任何后续功能开发**，向用户呈交审计/规划产物与桌面“设置”界面实测结果，获用户明确批准后方可进入 Phase 1。

#### Phase 1（公共契约、BSP 补齐与 CI 基础设施）
- **客观退出标准**：
  - `UNC-01`–`UNC-09` 形成书面接口澄清纪要（未澄清项禁止进入编码假设）；
  - `firmware/boards/lince/kzb_f407/bsp_clock_sram.c` 完成 LSEBYP 有界超时探测与 FSMC SRAM 初始化，消除 `kzb_f407.dts:42` 悬空引用；
  - 新增 `west.yml` 锁定 Zephyr `v4.5.0` 与批准的模块版本；修复 `.github/workflows/release.yml:208` 缺失模块并新增 `.github/workflows/ci.yml`（PR/Push 自动触发）。

#### Phase 2（MCUboot 与核心存储/诊断底座）
- **客观退出标准**：
  - `firmware/boot` 编译体积 `<= 131,072 B`（`boot_partition`），`firmware/app` 签名镜像 `<= 389,120 B`（`partitions.json:13`）；
  - `imgtool` ECDSA-P256 签名、错误签名拒收、低 `security-counter` 拒收、错误 `VID/CID` 拒收、未调用 `boot_write_img_confirmed()` 时自动回滚在实板或仿真验证全部通过；
  - `config-a/b` 断电回退、`action-journal` 掉电安全与 `nor-boundary`（跨 `0x01000000`）读写测试 100% 通过。

#### Phase 3（固件运行引擎、安全 IO 与总线服务）
- **客观退出标准**：
  - `board_early_init_hook()` 异常时固件安全锁止验证通过；`DR1/2` 与 `ACT/UDV` 不一致故障注入可在设定毫秒阈值内安全断电并写日志；
  - `native_sim` 下 `workflow_engine` 状态机与总线协议编解码 `ztest` 覆盖率与用例 100% 通过。

#### Phase 4（桌面全功能工作台与工程配置）
- **客观退出标准**：
  - 非专业人员可通过图形化流程画布完成端到端配置生成、本地断点仿真并导出合法参数/IR 包；
  - Windows（MSVC/MinGW）、Linux、macOS 三端 CI 构建 + `ctest` + `qmllint` + CPack 打包全绿。

#### Phase 5（真实硬件 HIL 联调与发布加固）
- **客观退出标准**：
  - 真实 `KZB-V1.1` 硬件台架完成 10 类 QA 第 10 类全部测试用例（含串口/CAN/双网口/USB 救援/电磁阀回环/断电故障注入）并出具真实硬件测试日志，零未解决阻断缺陷。

---

## 2. 完整 10 类测试与质量保证体系规划

当前 `fire/` 仓库处于 **0 测试、0 静态检查、0 CI 门禁**状态。以下 10 类测试体系必须在 Phase 1–5 逐步落地并纳入强制质量门禁：

| 序号 | 测试类别 | 目标与覆盖范围 | 推荐工具与框架 | 通过标准与门禁阶段 |
|---:|---|---|---|---|
| **1** | **静态代码检查** | 拦截 QML 语法/绑定错误、C/C++ 潜在空指针/未初始化/越界、Zephyr 代码风格与全警告零容忍 | 桌面：`qmllint`、`clang-tidy`、`cppcheck`；固件：Zephyr `checkpatch.pl`、编译器 `-Wall -Wextra -Werror` | **0 Error**（`qmllint` 在 R2 即接入；C++/C 静态检查在 Phase 1 `ci.yml` 强制门禁） |
| **2** | **单元测试** | 验证独立算法与核心类：PBKDF2-SHA256、RBAC、JSON Schema v1/v2、i18n 6 语言；嵌入式状态机、CRC/SHA256、双区头部校验 | 桌面：`Qt6::Test`、`Qt6::QuickTest` (`ctest`)；固件：Zephyr `ztest`（`native_sim` 主机仿真） | 新增/修改模块核心逻辑单测覆盖并 100% 通过（桌面 Settings 单测在 R2 落地） |
| **3** | **模块集成测试** | 桌面 C++ 服务与 QML UI 交互集成（门禁解锁、账号 CRUD、许可/恢复闭环）；嵌入式 `zbus` 多线程 + `storage_mgr` + `bsp_diag` 联动 | `Qt6::QuickTest` 隔离 `QTemporaryDir` 沙盒；Zephyr `twister` 多线程集成工程 | 各模块联调消息流无死锁、无内存泄漏、UI 与状态一致 |
| **4** | **通信协议测试** | C1–C4 协议编解码往返（Round-trip）、字节序、分包重组、畸形帧/截断帧/非法长度/CRC 错误/序号重放与乱序测试 | `common/` 共享测试向量 + C++/Zephyr Fuzz & 负向用例集 | 所有正向向量精确还原，100% 畸形/重放帧安全拒收且无崩溃/缓冲区溢出 |
| **5** | **异常与边界测试** | `nor-boundary` (`0x00FFF000`, 8 KiB) 跨越 `16 MiB` (`0x01000000`) 4 字节地址读写；双区配置损坏/全擦除；桌面 256 KiB 超限 JSON、过期许可、非法的角色越权操作 | `ztest` + SPI NOR 边界用例；桌面负向边界单测 | 跨 16 MiB 边界零地址回绕损坏；非法输入均返回明确受控错误码 |
| **6** | **故障注入测试** | 继电器 `DR1/2` 指令与 `ACT`/`UDV` 反馈不匹配（粘连/断线/欠压）；RS-485/CAN/以太网断连与强干扰；`config-a/b` 与 `action-journal` 写入中途随机断电；`IWDG` 卡死复位与 `kzb_early_state.rcc_csr` 溯源 | 仿真 Mock 故障钩子 + 程控电源/继电器台架断电测试 | 任一故障触发安全钳位并记录日志，掉电后可 100% 回退到上一完整提交态 |
| **7** | **固件升级与回滚测试** | 合法 `ecdsa-p256` 升级确认；篡改镜像/伪造签名拒收；低 `security-counter` 降级包拒收；非 `lince.firesystem`/`kzb_f407_v1.1` `VID/CID` 包拒收；未确认试启动镜像经复位/`IWDG` 后通过 `swap-scratch` 自动回滚 | `imgtool` 构造正/负向签名镜像 + `kzb_f407` 实板烧录与 USB/UART 救援测试 | 100% 拦截伪造/降级/错板固件；未确认试启动 100% 回滚成功 |
| **8** | **自动化构建** | 桌面端 3 平台 Debug/Release + CPack（Portable + IFW）；嵌入式 `boot` + `app`（`sysbuild` / 双目标）+ `imgtool` 自动签名与分区容量告警 | CMake + Ninja + CPack；`west build` + `imgtool`（同步修复 `release.yml:208` 模块列表） | 一键无人值守生成全部目标及签名产物，超出 `389,120 B` 自动失败 |
| **9** | **CI 测试** | 新增 `.github/workflows/ci.yml`，在 `pull_request` 与 `push` 时自动执行编译、静态检查、`ctest`、`qmllint`、`west twister` | GitHub Actions（`windows-latest` / `ubuntu-latest` / `macos-latest`） | PR 未通过 CI 检查一律禁止合并至 `main` |
| **10** | **真实硬件联调测试 (HIL)** | 在真实 `KZB-V1.1` 板上验证：上电瞬间示波器测量 `PE5/PE6/PC8/PF11/PG8` 无毛刺钳位、FSMC 1 MiB SRAM 全地址压力、W25Q01JV、LAN8720A/W5500 双网口、CAN1/2、RS-485 A/B、USB 救援及 `CN4/CN5` 电磁阀负载回环 | J-Link SWD + 示波器/逻辑分析仪 + CAN/485 分析仪 + 真实 `kzb_f407` 测试工装 | 出具带时间戳与硬件序列号的真实联调报告；**无真实硬件环境前严禁虚构通过** |

---

## 3. Git 分支策略、历史保留与合入审批治理

### 3.1 分支模型与历史保留纪律

1. **`main` 分支强保护**：
   - `main` 为受保护的唯一发布基线分支。**严禁**任何人员或自动化脚本直接向 `main` 提交、`git push --force` 覆盖历史或未经审批直接合并。
2. **工作分支命名规范**：
   - 功能开发：`feat/<round-or-milestone>-<topic>`（如本轮 `feat/r1-audit-plan-desktop-settings`）
   - 缺陷修复：`fix/<module>-<issue>`
   - 发布稳定：`release/vX.Y.Z`
3. **完整 Git 历史保留**：
   - 从 `main` 切出工作分支后采用增量式原子提交（Conventional Commits 风格，如 `docs:`、`feat(desktop):`、`test(desktop):`），完整保留已有历史与 R1 确认留存的本地修改记录。

### 3.2 跨模块影响分析检查单（PR 提交必填）

任何 Pull Request 涉及公共契约、构建脚本或跨端交互时，提交说明必须逐项勾选并说明影响：
- [ ] 是否修改了 `kzb_f407.dts`、`partitions.json` 或 `kzb_early_state.h`？（若“是”，需架构师 + Bootloader + 嵌入式三方确认）
- [ ] 是否修改了 `common/` 协议定义或桌面端本地 JSON Schema 版本？（若“是”，需通信 + 桌面 + 嵌入式 + QA 确认兼容与迁移策略）
- [ ] 是否修改了 `CMakeLists.txt`、`west.yml` 或 `.github/workflows/`？（若“是”，需验证全平台 CI 与 CPack 打包未破坏）
- [ ] 是否新增或变更了第三方依赖？（若“是”，必须附上 7 项评估报告与用户确认记录）
- [ ] 是否超出当前阶段授权范围？（例如本轮 R2 严禁包含“设置”之外的其他桌面子页面或 `firmware/` 代码改动）

### 3.3 PR 独立审查与主分支合入流程

1. **自测门禁**：开发者在工作分支完成自测、本地构建与全部可用测试（附真实命令与输出）。
2. **独立审查（R5 强制要求）**：由独立于实现者的角色（代码审查者 `Reviewer`、质询审查者 `Challenger`、合规审计员 `Auditor`）按核查清单独立复核文件差异、逐行代码、测试命令复跑与范围边界——**审查人员不得仅复述实现者结论**。
3. **维护者 / 用户审批合入**：所有阻塞问题清零后，呈交用户或仓库维护者最终审查批准，方可通过 PR（`--no-ff` 或标准评审合入）并入 `main` 分支。
