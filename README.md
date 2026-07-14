# 24年电赛E题 — 地猛星 自动小车循迹 (MSPM0G3507)

## 📋 项目介绍
基于 TI MSPM0G3507 微控制器的自动小车循迹系统。  
配合 Keil MDK 编译烧录，VS Code 编写代码。

## 📁 目录结构

```
├── BSP/            # 板级支持包（蜂鸣器、编码器、电机、循迹等外设驱动）
├── ti/             # TI MSPM0 SDK 驱动库（driverlib）
├── keil/           # Keil 工程文件
│   ├── *.uvprojx           # 项目文件（提交到仓库）
│   ├── *.uvoptx / *.uvguix # 个人配置（已忽略，不提交）
│   └── .vscode/            # VS Code 配置（IntelliSense）
├── board.c/.h      # 板级初始化
├── ti_msp_dl_config.c/.h   # TI 配置代码
└── empty.syscfg    # SysConfig 配置文件
```

## 🔧 环境配置（每位队友必须操作）

### 1. 安装软件
- Keil MDK （建议装到 `C:\Keil_v5`）
- VS Code + C/C++ 扩展

### 2. SDK 安装
将 MSPM0 SDK **解压到 `C:\ti\mspm0_sdk_2_01_00_03`**

> SDK 下载地址：TI官网 MSPM0 软件开发套件

### 3. Keil 中配置
打开 `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx`  
如果编译报错头文件找不到，在 `Project → Options → C/C++ → Include Paths` 中修正 SDK 路径。

### 4. VS Code 中代码补全
`keil/.vscode/c_cpp_properties.json` 已配置统一路径，SDK 装到指定位置后开箱即用。

## 🔄 团队协作流程

```bash
# 1. 克隆仓库
git clone <仓库地址>

# 2. 拉取最新代码
git pull

# 3. 改完代码后提交推送
git add .
git commit -m "描述本次修改"
git push
```

> ⚠️ 注意：编译在 Keil 中进行，VS Code 只管写代码和 Git 操作。
> 每次推送前先 git pull，避免冲突。

## 📝 注意事项
- ❌ 不要提交 `.o` / `.d` / `.axf` / `.hex` 等编译产物（已通过 .gitignore 过滤）
- ❌ 不要提交 `.uvguix.*` / `.uvoptx` 等个人配置
- ✅ 只提交源码（`.c` / `.h` / `.uvprojx`）和配置文件
