[中文](https://github.com/dd-center/obs-streamlink/blob/master/README_zh.md)

<p align="center"><h1>About the OBS 28 port and cross-platform support</h1></p>

The support for OBS 28 has been done. You're extremely welcomed to post your feedbacks!

For the macOS support, since I don't own a mac, currently I can't provide support for it (notably, some nasty delayed dynamic loading is required, so it's more thing to do than just compile a binary using CMake).

<div align="center">
  <h1><a href="https://github.com/dd-center/obs-streamlink/" target="_blank">OBS-StreamLink</a></h1>

![Language](https://img.shields.io/badge/language-c++-orange?style=flat-square)
![Build](https://img.shields.io/badge/build%20with-cmake-red?style=flat-square&logo=cmake)
![Platform](https://img.shields.io/badge/platform-Windows-blue?style=flat-square&logo=windows)
![OBS](https://img.shields.io/badge/obs-%3E=24.0-brightgreen?style=flat-square)
![Streamlink](https://img.shields.io/badge/streamlink-latest-brightgreen?style=flat-square)
![I18N](https://img.shields.io/badge/i18n-en%7Cja%7Ccn-lightgrey?style=flat-square)
[![Size](https://img.shields.io/badge/size-24.1MB-brightgreen?style=flat-square)](https://github.com/dd-center/obs-streamlink/releases/latest)
[![License](https://img.shields.io/github/license/dd-center/obs-streamlink?style=flat-square)](https://github.com/dd-center/obs-streamlink/blob/master/LICENSE)

</div>

[OBS](https://obsproject.com/) source plugin to receive stream using [Streamlink](https://streamlink.github.io/).

## 👏 Screenshots

![Screenshot1](https://raw.githubusercontent.com/dd-center/obs-streamlink/master/.github/img1.jpg)

![Screenshot2](https://raw.githubusercontent.com/dd-center/obs-streamlink/master/.github/img2.jpg)

## 🌟 Features

- Broadcast using **ONLY OBS**. No Chrome. No Webpage. No stream transformer. No extra environment. No extra applications.

- **HANDY** as you only need to provide channel link. No other configurations.

- **ISOLATED AUDIO** with no need to close all other applications playing sound. The audio from livestream will go directly into OBS, enabling you to even rebroadcast multiple livestreams simultaneously! You can use a

- Using powerful [**Streamlink**](https://streamlink.github.io/) to resolve stream. Born with powerful plugins and community support.

- Support almost all streaming platforms from worldwide because streamlink has a lot of [plugins](https://streamlink.github.io/plugin_matrix.html) and has a strong community.

- Support all Streamlink options so it's able to meet all your needs.

- Lowest performance consumption as it uses only OBS core modules. It will use **NO MORE THAN 10% of CPU** when playing. (Plugin + OBS, on a Ryzen 5900HS)

- Custom Streamlink arguments to match your requirements!

## 🔔 Requirements

OBS Only (>=28.0).

## 💨 Install

Just download the installer from the [latest release](https://github.com/dd-center/obs-streamlink/releases/latest) and run it. The installer will check and verify the OBS install path automatically so just click next and finish.

## 👉 Usage

- Add a `StreamlinkSource` in your scene.

- Paste your channel link and click `Refresh Source List`. Set up proxy server address if you need.

- Choose a source from the list (or automatically the best) and click OK. Your stream is now ready.

- If custom Streamlink arguments is needed, put it in an JSON object, for example: `{"http-proxy":"http://localhost:1080", "https-proxy":"http://localhost:1080", "ffmpeg-verbose": true, "ffmpeg-verbose-path": "test.txt"}`

- If you want to add custom plugins, place the `.py` files in `(obs installation path)data/obs-plugins/obs-streamlink/Python38/Lib/site-packages/streamlink/plugins`. `plugin-dirs` Streamlink option is not supported.

## ⚡ Performance

It won't consume more than 10% of CPU if there's only an instance in the active scene. Tested on several processors.

## 💬 BUGs & Issues

Feel free to [open issues](https://github.com/dd-center/obs-streamlink/issues/new).

## 💻 Contributions

PRs are welcome! Feel free to contribute on this project.

## ⚠ LICENSE

GPL-v3.0
