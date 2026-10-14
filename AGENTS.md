# AGENTS.md — Windows 构建

只记必要的事实。命令都在仓库根目录执行。

## 0. 构建

```powershell
cmake --build cmake-build-debug --target 2DFM_Player -j 14
```

- 需要已加载 MSVC 环境，见 §2。
- 产物：`cmake-build-debug\bin\2DFM_Player\2DFM_Player.exe`。生成器是 Ninja 单配置，**没有** `Debug\` 子目录；`Content` 由构建以符号链接挂到 exe 旁边，直接运行这个 exe 即可。
- `run.bat` 由 `run.bat.in` 生成，路径里带 `<Config>\` 子目录（多配置生成器的模板），与上面的布局不符，会找不到 exe。

## 1. 前提

- Visual Studio 2022 或更新版本，装「使用 C++ 的桌面开发」工作负载；CMake ≥ 3.20（根 `CMakeLists.txt` 的下限）；Ninja。
- 环境变量 `AX_ROOT` 指向 axmol 检出的根目录。缺了 configure 直接失败：
  `FATAL_ERROR: Please run setup.ps1 add system env var 'AX_ROOT' ...`
  也可以把 axmol 检出放到仓库下的 `axmol/`，那样不需要 `AX_ROOT`（见根 `CMakeLists.txt` 顶部）。
- 构建目录 `cmake-build-debug`：生成器 Ninja（单配置）、`CMAKE_BUILD_TYPE=Debug`、`AXMOL_ENGINE_SHARED=ON`（链接预编译引擎树，本工程不编译引擎源码）。
- **不要跨机器复制 `cmake-build-debug`**：`CMakeCache.txt` 与 `CMakeFiles\rules.ninja` 里写的是生成它的那台机器的绝对路径。换机器就删掉重新配置（CLion 打开工程会自动做）。

## 2. MSVC 环境

`cl.exe` 靠 `INCLUDE` / `LIB` 找标准库和 Windows SDK。普通 PowerShell 里这两个是空的，一旦真的需要编译就会报
`fatal error C1083: 无法打开包括文件: "cstddef" / "stdio.h"`。

三种加载方式任选：Developer PowerShell for VS / 下面这段 / 在 CLion 里构建。

```powershell
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
Import-Module "$vs\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64'
# 验证：$env:INCLUDE 应该非空
```

全部已是最新时，构建只跑资源同步，没有环境也能“成功”——别据此以为环境没问题。

## 3. 用智能体（DSH 等）构建前：先放开文件沙箱

**这是构建的第 0 步。**实测在 DSH 的 `workspace-write` 下：构建尾部的资源同步要跑 `fsync.ps1`（给 `bin\2DFM_Player\Content` 建符号链接、从 `%AX_ROOT%` 拷 `axslc`），这些操作被挡住后，执行它的 `powershell.exe` 空转（线程 `WaitReason=Executive`，CPU 0），`ninja` 一直在等它 → `cmake --build` 永不返回，而且**不报任何错**。

- 沙箱级别不是完全文件访问（DSH 里是 `danger-full-access`）时：**停下来**，把当前级别报给用户，请用户放开后再构建。不要在受限沙箱里试。
- 不要自行申请提权：很多会话里审批是关闭的，申请会被直接拒绝。
- 怎么查：读运行时上下文里的文件策略；DSH 没有把它放进环境变量（`DSH_*` 只有 HOME / PROFILE / SESSION_ID / SHELL / WEB_URL），查不到就问用户。
- 权限放开后，如果之前已经卡过一次，先清掉残留进程再重跑：

```powershell
Get-Process cmake,ninja,cl,link,cmd,powershell -ErrorAction SilentlyContinue | ForEach-Object {
    $w = $_.Threads | Where-Object ThreadState -ne Running | Group-Object WaitReason |
         ForEach-Object { "$($_.Name)x$($_.Count)" }
    [pscustomobject]@{ Id=$_.Id; Name=$_.ProcessName; CPU=[math]::Round($_.CPU,1); WaitReasons=($w -join ',') }
}
```

受限环境下 `Get-CimInstance Win32_Process` 会被拒（拿不到命令行），只能用 `Get-Process`。

## 4. 其它

- 每次构建都会跑 7 步 `SYNC_RESOURCE`（`fsync.ps1` 拷 axslc、建 Content 符号链接），不代表在编译。
- 引擎走预编译树（`%AX_ROOT%\prebuilt\<os>-<arch>-<cfg>[-shared]`），本工程不编译它。改了 `cmake/modules/AxmolEngineOptions.cmake` 的选项、或预编译树缺失 / 与本次选项不一致时，configure 会 WARNING，按提示执行
  `cmake --build cmake-build-debug --target axmol_engine_prebuilt`，然后重新 configure（CLion：Reload CMake Project）。
- 要全量重编 app：删掉 `cmake-build-debug\CMakeFiles\2DFM_Player.dir` 下的 `*.obj` 再构建。
- 速查：源码在 `Source/**`（`file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`，增删文件不需要手动 configure）；脚本项类型枚举 `Source/2dfm/2dfmScriptItem.hpp`；编译数据库 `cmake-build-debug\compile_commands.json`；引擎选项 `cmake/modules/AxmolEngineOptions.cmake`、预编译树逻辑 `cmake/modules/AxmolPrebuilt.cmake`。
