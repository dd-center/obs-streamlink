[English](https://github.com/dd-center/obs-streamlink/blob/master/README.md)

<div align="center">
  <h1><a href="https://github.com/dd-center/obs-streamlink/" target="_blank">OBS-StreamLink</a></h1>

![语言](https://img.shields.io/badge/%E8%AF%AD%E8%A8%80-c++-orange?style=flat-square)
![构建使用](https://img.shields.io/badge/%E6%9E%84%E5%BB%BA%E4%BD%BF%E7%94%A8-cmake-red?style=flat-square&logo=cmake)
![平台](https://img.shields.io/badge/%E5%B9%B3%E5%8F%B0-Windows-blue?style=flat-square&logo=windows)
![OBS](https://img.shields.io/badge/obs-%3E=24.0-brightgreen?style=flat-square)
![Streamlink](https://img.shields.io/badge/streamlink-%E6%9C%80%E6%96%B0-brightgreen?style=flat-square)
![国际化](https://img.shields.io/badge/%E5%9B%BD%E9%99%85%E5%8C%96-en%7Cja%7Ccn-lightgrey?style=flat-square)
[![大小](https://img.shields.io/badge/%E5%A4%A7%E5%B0%8F-24.1MB-brightgreen?style=flat-square)](https://github.com/dd-center/obs-streamlink/releases/latest)
[![许可](https://img.shields.io/github/license/dd-center/obs-streamlink?style=flat-square)](https://github.com/dd-center/obs-streamlink/blob/master/LICENSE)

</div>

使用 [Streamlink](https://streamlink.github.io/) 获取直播流的 [OBS](https://obsproject.com/) 插件。

## 👏 截图

![Screenshot1](https://raw.githubusercontent.com/dd-center/obs-streamlink/master/.github/img1.jpg)

![Screenshot2](https://raw.githubusercontent.com/dd-center/obs-streamlink/master/.github/img2.jpg)

## 🌟 功能

- **只使用 OBS 即可开始转播。** 无需 Chrome。无需网页。无需解析器。无需配置环境。无需额外应用。

- **一步上手** 因为你只需要提供转播链接。无需额外配置。

- **音频隔离** 直播流音频不再经过系统音频输出，而是直接输出到 OBS。不用关掉别的应用，甚至可以一次转播多个直播！同时你也可以通过OBS的音频监听来监听音频内容。

- 使用强大的 [**Streamlink**](https://streamlink.github.io/) 解析直播源。天生自带众多插件和活跃的社区支持。

- 支持几乎所有的直播平台因为 Streamlink 拥有众多 [插件](https://streamlink.github.io/plugin_matrix.html) 且有一个强大而活跃的社区。

- 支持所有的 Streamlink 选项，能够满足你的所有要求（开发中）。

- 只使用 OBS 核心播放组件，因此对电脑性能的消耗极低。转播时 **不会占用多于 10% 的 CPU**。

## 🔔 系统需求

只需要 OBS (>=24.0)。

## 💨 安装

只需要下载 [最新版本](https://github.com/dd-center/obs-streamlink/releases/latest) 的安装程序（[国内镜像](https://soft.danmuji.org/obs-streamlink/)）并且运行。安装程序会自动搜索并检查 OBS 的安装路径，因此只需一路回车。

## 👉 使用

- 在场景中添加一个 `StreamlinkSource` 源。

- 将直播地址粘贴入源属性窗口中，然后点击 `刷新来源列表`。如果有需要，可以设置代理服务器。

- 选择一个来源（或者保留默认的「最佳」来源）然后点击确定。你的转播现已准备就绪。

## ⚡ 性能

如果 OBS 场景中只有一个转播来源的话，程序将不会占用多于 10% 的 CPU。已经在多种处理器上测试。

## 💬 BUG 和 问题

请自由 [提交问题](https://github.com/dd-center/obs-streamlink/issues/new)。

## 💻 贡献

我们欢迎合并请求！请自由在这个项目上做贡献。

## ⚠ 许可

GPL-v3.0
