# 沉浸式全局翻译工具

一个基于 Tauri 的跨平台桌面应用，支持在任意应用中选中文本后，通过全局快捷键弹出浮窗，调用百度翻译 API，实现中英文自动互译并可一键复制或粘贴。同时可选开启实时语音翻译（会议模式），捕获其他应用的音频输出进行边听边译。

## 核心功能

- **全局快捷键触发**：默认 `Ctrl+Alt+T`，唤起翻译流程。  
- **智能抓取文本**：模拟复制操作读取剪贴板中选中文本。  
- **中英自动互译**：调用百度翻译 API，自动检测并在中文↔英文间切换。  
- **气泡浮窗展示**：无边框、透明背景、始终置顶，显示翻译结果。  
- **快速操作**：提供“复制”和“应用”按钮，分别写入剪贴板或模拟 `Ctrl+V` 将译文粘回原程序。  
- **实时语音翻译 (会议模式，可选)**：捕获系统音频回环流，将其他应用（如会议软件）的 PCM 音频送入百度实时语音翻译 API，实时显示翻译文本。

## 实现概览

1. **后端 (Rust / Tauri)**  
   - 注册全局快捷键（`tauri::globalShortcut`）。  
   - 使用 `tauri-plugin-clipboard` 读取剪贴板内容。  
   - 通过 `reqwest` 调用百度文本翻译 API，生成签名（`md5`）并处理响应。  
   - 控制 `bubble` 窗口显示位置（`tauri-plugin-positioner`）及状态。  
   - 暴露命令给前端处理“复制”和“应用”按钮。  

2. **前端 (HTML/CSS/JS)**  
   - 极简静态页面，透明背景的翻译卡片。  
   - 监听后端事件（`translation-ready`），渲染译文。  
   - 处理按钮点击，调用 `invoke` 执行复制或粘贴命令。  
   - 监听失焦和 `Esc` 事件，自动关闭浮窗。  

3. **实时语音翻译 (会议模式，可选)**  
   - 使用系统音频回环捕获库（如 Windows WASAPI 回环、PulseAudio 回环、CoreAudio AudioUnit 或 `cpal` 的回环模式）采集其他应用的 PCM 音频流。  
   - 用 `tokio-tungstenite` 建立 WebSocket 连接至百度实时语音翻译 API，实时推送音频分片并接收识别及翻译结果帧。  
   - 解析并动态更新翻译内容至气泡界面。

## 技术栈

- **框架**：Tauri 1.x  
- **语言**：Rust、HTML/CSS、JavaScript  
- **主要依赖**：  
  - 文本翻译：`tauri-plugin-clipboard`、`tauri-plugin-positioner`、`reqwest`、`serde`、`md5`、`rand`、`enigo`  
  - 语音翻译：系统音频回环捕获库（`cpal` 回环 或 `wasapi`/`pulseaudio`/`coreaudio`），`tokio-tungstenite`、`serde_json`  
  - 可选：系统托盘(`tray-icon`)、UI动画(`egui`/`slint`)  

## 部署与配置

1. 在环境变量中配置 `BAIDU_APPID` 和 `BAIDU_KEY`（文本翻译），以及获取并管理 `access_token`（语音翻译）。  
2. 修改 `tauri.conf.json`，确保 `main` 窗口不可见，`bubble` 窗口透明并默认隐藏。  
3. 执行 `npm run tauri build` 生成各平台可执行文件。

---

*简单上手，沉浸体验，助你在任意场景中流畅翻译与实时会议同传。*
