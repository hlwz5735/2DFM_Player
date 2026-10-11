# Input 模块（1P 已按此重构）

> **状态：1P 重构已完成**（结构与接口按本文件实施；2P 只留了状态格子与接口参数，键位未接）。
> 设计与迁移注意事项见下，2P 以后照此追加即可。

## 目标

- 最多 **2 个控制器**（1P / 2P），各自独立保存自己的按键状态。
- 每个控制器保存**上一帧**和**当前帧**两份状态，对外只暴露三种判定：**按下 / 按住 / 抬起**。
- 已实施范围：**1P**。

## 每个控制器记录的状态

| 项 | 取值 | 说明 |
| --- | --- | --- |
| D-Pad | `1`~`9` | 小键盘顺序：1 左下、2 下、3 右下、4 左、5 回中、6 右、7 左上、8 上、9 右上 |
| 攻击键 | A~F（6 个） | 每个键独立记 0/1 |
| 开始键 | 0/1 | |
| 暂停键 | 0/1 | |

> 方向值 1~9 由四个方向键（上/下/左/右）**组合**出来（`Input::getDPad()`），
> 所以“某个方向分量按下/按住”是原始判定，方向值只是它的派生结果。

## 状态判定

每个控制器各存一份 `prev` 与 `cur`：

| 语义 | 判定 | 接口 |
| --- | --- | --- |
| 按下 | `prev == 0 && cur == 1` | `isDirPressed` / `isAttackPressed` / `isButtonPressed` |
| 按住 | `prev == 1 && cur == 1` | `isDirHeld` / `isAttackHeld` / `isButtonHeld` |
| 抬起 | `prev == 1 && cur == 0` | `isDirReleased` / `isAttackReleased` / `isButtonReleased` |

- **“按住不放”的完整语义 = 按下 ‖ 按住**（即“这一帧确实按着”）。
- 攻击键与开始/暂停额外记录**按住时长**（`getAttackHeldTime` / `getButtonHeldTime`，单位秒，松手归零），
  留给以后的蓄力类指令。
- 方向键不做“按住的时长”：方向是四个键的组合，按住的时长按分量记意义不大。

## 1P 键位

| 动作 | 键 |
| --- | --- |
| 上(8) / 下(2) / 左(4) / 右(6) | `W` / `S` / `A` / `D` |
| 攻击 A / B / C / D / E / F | `J` / `U` / `K` / `I` / `O` / `L` |
| 开始 | 数字 `1` |
| 暂停 | `ESC` |

> 与重构前的差别：**开始键从 `ENTER` 改成数字 `1`**。

## 2P 怎么加

1. 在 `Input::init()` 的键位表里追加一份 `player = PlayerIndex::P2` 的绑定（状态格子已经留好）；
2. 2P 的 `Input::PlayerIndex::P2` 状态目前全是回中/未按，不接键位时读取它不会出问题。

## 对状态机的影响：连跳不再需要额外标志位

`CharacterController` 原来为了连跳自己记了 `prevDPad`，还额外维护了一个“落地后允许按住起跳”的许可标志
`jumpAllowedWhileHeld`。**这两个现在都删掉了**：

- 起跳条件就是“当前方向值属于上类（7/8/9）”，也就是**“上类方向按下或按住”**；
- 浮空中不处理输入、起跳过程中提前返回，所以按住不放不会中途重复触发；
- 于是“按住上方向 → 落地后又跳”自然成立，就是连跳；左右也按着时方向值本身是 7/9，自动跳对应方向。

## 两个必须注意的时序问题

1. **翻页时序**：`Input::update()` 由调度器以**优先级 2**驱动，而场景 / 节点是**优先级 0**，
   所以每帧次序是“节点先读输入 → `update()` 再翻页”，读到的 `prev` 才是真正的上一帧。
   **任何地方都不要再手动调一次 `Input::update()`**，否则会把本帧状态写进“上一帧”，
   按下 / 抬起判定会全部失效。（`PlayerTestScene` 里原来就有这么一句，已删除。）
2. **按下是“帧采样”而不是“事件锁存”**：按下 = 上一帧没按、这一帧按着。
   若某次按下与抬起都发生在同两帧之间（极端快的点按），这一帧采样不到 —— 与重构前行为一致。

## 重构时顺带修掉的两个问题

- `isAnyAttackButtonDown()` 的循环上界写成了 `< F`，**漏掉了 F 键**；新的 `isAnyAttackPressed()` 覆盖 A~F。
- `cleanup()` 重复调用会对已释放的监听器再 `release()` 一次；现在清过一次就把指针置空。

## 调用点（已迁移）

| 位置 | 用到的接口 |
| --- | --- |
| `CharacterController` | `getDPad()`、`isUpDirection()` / `isDownDirection()` |
| `CharSelectionScene` | `isDirPressed(Up/Down/Left/Right)`、`isAnyAttackPressed()` |
| `TitleScene` | `isDirPressed(Left/Right)`、`isAnyAttackPressed()` |
| `OpenningScene` | `isAnyAttackPressed()` |
| `StageTestScene` | `isAttackPressed(E)`、`isAttackPressed(F)` |
| `GameBootstrap` / `AppDelegate` | `init()` / `cleanup()` |
