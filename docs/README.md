# FireSystem 工程架构与规划文档索引

本目录存放 **通用灭火系统平台（FireSystem）** 的仓库审计、需求规格、总体双端架构、统一接口规范及分阶段测试交付计划。

## 文档导航

| 文档 | 对应要求 | 核心内容摘要 |
|---|---|---|
| [`01_repository_audit.md`](./01_repository_audit.md) | R1 | Git 仓库基准核验（`origin`、`main @ a1eaa93`、6 个基线提交、子模块 `FluentUI` 与 2 处既有本地修改）、`fire/` 全量逐文件盘点与完整性结论、`release.yml` 缺口分析、`new_fire` 存在登记与严格隔离边界及 R1 用户确认记录 |
| [`02_requirements_and_unconfirmed.md`](./02_requirements_and_unconfirmed.md) | R3 | 平台产品目标、`KZB-V1.1` 硬件对齐基线、嵌入式与桌面端功能/非功能需求清单（含参考 IEC 61499 / IEC 61131-3 的图形化配置与标准边界、故障诊断、MCUboot 升级/救援、桌面“设置”模块范围），以及显式 **`无法确认` 事项登记表（`UNC-01`..`UNC-09`）** |
| [`03_architecture_and_interfaces.md`](./03_architecture_and_interfaces.md) | R3 | 嵌入式（`STM32F407ZET6` + Zephyr 原生 API + MCUboot + `kzb.partitions.v1`）与桌面端（Qt 6 + C++ + QML + `FluentUI`）总体架构、全平台模块清单与单向依赖矩阵、统一通信协议框架、公共跨模块接口规范与变更门禁、**6 大角色职责与文件所有权**、第三方依赖零新增声明及 **7 项准入评估模板** |
| [`04_development_and_qa_plan.md`](./04_development_and_qa_plan.md) | R4 | Phase 0–Phase 5 分阶段整改路线图与客观退出标准、**Round-1 交付后强制暂停门禁**、完整 **10 类测试与质量体系**（静态检查/单测/集成/协议/边界/故障注入/升级回滚/自动构建/CI/真实硬件 HIL）及 Git 分支/历史保留/PR 独立审查与主分支审批合入治理 |

## 当前阶段声明与隔离纪律

1. **当前阶段（Round 1 / Phase 0）**：本轮仅完成 `docs/` 全套工程规划与 `fire/desktop/` 中“设置”界面（及本地构建测试与独立审查），完成后即进入 **Round-1 暂停门禁**，等待用户审查批准后再启动 Phase 1 及后续子系统（嵌入式应用/Bootloader 业务代码、桌面端工作台其余子页面等）开发。
2. **源码隔离边界**：外部参考仅限只读查阅 `new_fire\原理图.pdf`（`KZB-V1.1`）与 `new_fire\desktop\prototype\`；严禁读取或借用 `new_fire` 中其余任何源码与架构文档。
3. **工程真实性准则**：凡硬件资料与现有仓库契约中未证实的参数均标注为 `无法确认`，严禁凭空猜测或虚构构建/测试/硬件验证结论。
