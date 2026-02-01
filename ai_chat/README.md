
<p align="right">
  <a href="./README.md">English</a> | <a href="./README.zh.md">简体中文</a>
</p>

# Midsummer's Bird

> An Elegant, Powerful, and Fully Localized AI Role-Playing Chat Frontend

-----

## ✨ Project Overview

**Midsummer's Bird** is a feature-complete AI chat application frontend designed specifically for role-playing and creative writing. Inspired by [SillyTavern](https://github.com/SillyTavern/SillyTavern), this project is an **independently developed third-party single-file UI implementation** that packages complex AI interaction features into a single HTML file, requiring no server and ready to use out of the box.

### 🔖 Relationship with SillyTavern

This project is a **third-party independent implementation inspired by SillyTavern**, not an official product or fork:

  - ✅ Compatible with SillyTavern's character card formats (Character Card V2/V3)
  - ✅ Adopts a similar design philosophy (World Info, regular expressions, etc.)
  - ❌ Not a port or clone of SillyTavern
  - ❌ Does not provide all features of SillyTavern
  - 🎯 Focuses on being **single-file, lightweight, and easy to use**

### 🌟 Core Features

#### 🤖 **Powerful API Integration**

  - Supports all OpenAI-format compatible APIs
  - Pre-configured support for: DeepSeek, OpenAI, SiliconFlow, Google Gemini, etc.
  - Independent vision API configuration supporting image understanding
  - Streaming support for real-time responses
  - Multi-configuration management with one-click switching

#### 🎭 **Complete Character Management System**

  - Flexible character card editor
  - Support for multiple opening greetings
  - Character-specific example dialogues
  - Full character import/export functionality
  - Supports Character Card V2/V3 formats

#### 📚 **World Info System**

  - Global World Info + Character-specific World Info
  - Smart keyword matching triggers
  - Depth and priority control
  - Selective matching from multiple sources (character description, dialogue, user settings, etc.)
  - Recursive exclusion functionality

#### ⚙️ **Advanced AI Preset Management**

  - Complete sampling parameter control (temperature, top\_p, frequency\_penalty, etc.)
  - Visual prompt construction system
  - Token usage estimation and context management
  - Preset import/export
  - Real-time preview of prompt structure

#### 🔧 **Regular Expression Processing**

  - Global and local regex scripts
  - Support for input/output processing
  - Flexible application scope control
  - Script enable/disable management

#### 🎨 **Theme Customization System**

  - 6 built-in beautiful themes (Default, Dark, Ocean, Forest, Violet, Sakura)
  - Complete custom CSS editor
  - Real-time preview functionality
  - Theme saving and exporting
  - CSS formatting and minification tools

#### 👤 **Multi-User Management**

  - Multi-user configuration support
  - Independent user persona descriptions
  - Custom system prompts
  - User avatars and status
  - User preference settings

#### 💬 **Rich Chat Features**

  - Streaming output for real-time display
  - Chat summarization feature (automatically adds to World Info)
  - Reselect opening greetings
  - Message editing, deletion, and regeneration
  - Image sending support
  - Chat history export/import

#### 🌍 **Complete Multi-Language Support**

  - Chinese / English bilingual interface
  - One-click switching with real-time updates
  - All features fully localized

#### 💾 **Fully Local Storage**

  - Single-file application, no installation required
  - All data is stored locally in the browser
  - Complete configuration import/export
  - Privacy-first, you own your data

-----

## 🚀 How to Use

### Quick Start

1.  **Download the file**

      - [**➡️ Click here to download the latest version (v1.0.0)**](https://github.com/csjafuwvbegscbw-star/Midsummer-s-Bird/releases/download/v1.0.0/Midsummer.s.Bird.html)

2.  **Open the application**

      - Open the `Midsummer's Bird.html` file directly in your browser
      - Or, double-click the file to run it

3.  **Configure your API**

      - Click on "API Settings" in the sidebar
      - Enter your API URL and API Key
      - Test the connection to ensure it's configured correctly

4.  **Import a Character Card**

      - Click on "Character Card"
      - Import an existing character card file (supports .json and .png formats)
      - Or, create a new character

5.  **Start chatting**

      - Select a character
      - Enter a message in the chatbox
      - Enjoy your conversation with the AI\!

-----

## 📦 Compatibility

### Supported API Providers

  - ✅ OpenAI (GPT-3.5, GPT-4, GPT-4-Turbo, GPT-4o)
  - ✅ DeepSeek (DeepSeek-Chat, DeepSeek-Coder)
  - ✅ SiliconFlow (via OpenAI compatible interface)
  - ✅ Google Gemini (Gemini-Pro, Gemini-Pro-Vision)
  - ✅ Anthropic Claude (via a proxy API)
  - ✅ All local models that support the OpenAI format (Ollama, LM Studio, etc.)

### Browser Support

  - ✅ Chrome/Edge 90+
  - ✅ Firefox 88+
  - ✅ Safari 14+
  - ✅ Mobile browsers

-----

## 🎯 Use Cases

  - 📖 **Role-Playing Creation**: Rich character management and World Info system
  - ✍️ **Creative Writing**: Flexible prompt control and AI presets
  - 🎮 **Interactive Fiction**: Complex narratives with multiple characters and scenes
  - 🧪 **AI Experimentation**: Test different parameter and prompt combinations
  - 💼 **Work Assistant**: Configure professional AI assistant characters
  - 🎓 **Learning Tool**: Engage in topical conversations and discussions with AI

-----

## 🔄 Main Differences from SillyTavern

### ✅ Advantages of Midsummer's Bird

| Feature          | Midsummer's Bird                     | SillyTavern                               |
| ---------------- | ------------------------------------ | ----------------------------------------- |
| **Deployment** | Single HTML file, double-click to run | Requires Node.js environment, complex setup |
| **File Size** | \~500KB, lightweight and portable     | Full installation \> 100MB                 |
| **Update Method**| Just download the new file           | Requires `git pull` or re-installation    |
| **Cross-Platform**| Any modern browser                   | Requires platform-specific runtime        |
| **Learning Curve**| Simple and intuitive                 | Feature-rich but more complex             |
| **Multi-Language**| Built-in Chinese/English             | Requires configuring language packs       |

### ❌ Advanced Features Exclusive to SillyTavern

The following features are **NOT supported** in Midsummer's Bird:

  - **Extension System**: SillyTavern's plugin/extension ecosystem
  - **Voice Features**: TTS (Text-to-Speech) and STT (Speech-to-Text)
  - **Advanced Image Generation**: Stable Diffusion integration, expression generation
  - **Group Chat**: Multiple characters conversing at the same time
  - **Background Music**: Scene sound effects and background music
  - **Advanced Programming Interfaces**: WebSocket, custom scripts
  - **Database Integration**: Vector databases, ChromaDB support
  - **Advanced Memory System**: Long-term memory, memory summarization
  - **Horde Integration**: Distributed AI inference network

### 🎯 Selection Guide

**Choose Midsummer's Bird if you need:**

  - ✅ A quick start with no installation or configuration
  - ✅ Portability for use on a USB drive or cloud storage
  - ✅ A clean interface focused on the chat experience
  - ✅ A lightweight solution that doesn't consume system resources
  - ✅ The basic role-playing features are enough for you

**Choose SillyTavern if you need:**

  - ✅ The full suite of advanced features and its extension ecosystem
  - ✅ Deep customization and development capabilities
  - ✅ Group chats and complex scenarios
  - ✅ Multimedia functions like voice and image generation
  - ✅ Community support and frequent updates

-----

## 🔒 Privacy and Security

  - **Runs Entirely Locally**: All data is stored in the browser's localStorage and is not uploaded to any server.
  - **API Key Protection**: Your API Key is protected as a password-type input.
  - **No Tracking**: Contains no analytics or tracking code.
  - **Offline Availability**: Besides CDN resources (Tailwind, Font Awesome), the core features work without an internet connection.

-----

## 📸 Feature Screenshots

### Main Interface

  - Clean and elegant chat interface
  - Streaming output for real-time response
  - Sidebar for quick access to all features

### API Settings

  - Intuitive configuration management
  - Support for multiple API configuration switching
  - Independent vision API settings

### Character Management

  - Visual character editor
  - Character preview cards
  - One-click import and export

### World Info

  - Tree-structured World Info management
  - Smart keyword matching
  - Global and character-specific World Info

### Theme Customization

  - 6 beautiful preset themes
  - Custom CSS editor
  - Real-time preview effects

-----

## 🛠️ Tech Stack

  - **Frontend Framework**: Vanilla JavaScript + Tailwind CSS
  - **UI Library**: Font Awesome icons
  - **Drag & Drop Sorting**: Sortable.js
  - **Storage**: Browser localStorage API
  - **AI Interface**: OpenAI-compatible APIs

-----

## 📝 Changelog

### v1.0.0 (2025-10-17)

#### 🎉 Initial Release

  - ✅ Complete AI chat functionality
  - ✅ Character management system
  - ✅ World Info system
  - ✅ AI preset management
  - ✅ Regular expression processing
  - ✅ Theme customization
  - ✅ Multi-user management
  - ✅ Chinese/English bilingual support
  - ✅ Image recognition feature
  - ✅ Chat summarization feature
  - ✅ Full import/export functionality

-----

## 🤝 Contributing

Issues and Pull Requests are welcome\!

### Contribution Guidelines

1.  Fork this repository
2.  Create your feature branch (`git checkout -b feature/AmazingFeature`)
3.  Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4.  Push to the branch (`git push origin feature/AmazingFeature`)
5.  Open a Pull Request

-----

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](https://www.google.com/search?q=LICENSE) file for details.

-----

## ⚠️ Disclaimer

### About the Project's Nature

1.  **Third-Party Independent Project**

      - This project is an **independent third-party implementation** inspired by SillyTavern.
      - It is **NOT** an official SillyTavern project, fork, or affiliated product.
      - It has no official association with the SillyTavern development team.
      - It does not represent the views or positions of the SillyTavern project.

2.  **Feature Compatibility**

      - While it supports importing SillyTavern's character card format, 100% compatibility is not guaranteed.
      - There is no guarantee of full interoperability with SillyTavern's configuration files.
      - Some advanced features may behave differently.

### Usage Risk Notice

1.  **Data Security**

      - This project uses the browser's localStorage for data storage, which may be lost due to browser cache clearing, crashes, etc.
      - It is recommended to regularly export important data (character cards, chat logs, configurations, etc.).
      - API Keys are stored locally. Please keep them secure and do not use them on public devices.

2.  **API Costs**

      - Using third-party APIs may incur fees. You are responsible for these costs.
      - Improper use may lead to unexpectedly high bills. Please set reasonable spending limits.
      - Please comply with the terms of use and restrictions of your API provider.

3.  **Content Responsibility**

      - This project is a tool. It is not responsible for the content generated or entered by the user.
      - AI-generated content may contain inaccurate, biased, or inappropriate information.
      - Users should independently judge the suitability and accuracy of the AI's output.
      - Do not use this tool to generate illegal, harmful, or unethical content.

4.  **Privacy**

      - Other than communicating with your configured API provider, this project does not collect or upload user data.
      - Please note: all content sent to the API will be processed by the corresponding service provider.
      - Do not enter sensitive personal information in your conversations (ID numbers, bank accounts, passwords, etc.).
      - When using a third-party API service, please read its privacy policy.

### Technical Limitations

1.  **Browser Dependency**

      - This project relies on modern browser features and may not work correctly on older browsers.
      - Some browsers' privacy modes may restrict localStorage functionality.
      - Behavior may vary slightly between different browsers.

2.  **Performance Limitations**

      - As a single-file application, performance issues may arise when handling very large amounts of data.
      - Extended use may lead to increased browser memory consumption.
      - It is not suitable for industrial-grade or high-concurrency use.

3.  **Maintenance and Support**

      - This is a personal open-source project and comes with no warranty or technical support.
      - There may be undiscovered bugs or security issues.
      - Feature updates and bug fixes depend on the developer's available time and effort.
      - Backward compatibility with future versions is not guaranteed.

### Legal Compliance

1.  **User Responsibility**

      - By using this software, you agree to assume all risks.
      - Please comply with the relevant laws and regulations in your region.
      - Please adhere to the terms of service of your API provider.
      - Do not use this software for any illegal or unethical purposes.

2.  **Age Restriction**

      - AI-generated content may not be suitable for minors.
      - Please ensure you meet the legal age requirements of your region.
      - Parents should supervise any use by minors.

3.  **Intellectual Property**

      - For questions regarding the copyright of content generated with this tool, please consult professional legal advice.
      - Please respect the intellectual property of others and do not generate infringing content.
      - The copyright of resources like character cards belongs to their original creators.

### Disclaimer Clause

**THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.**

**BY USING THIS SOFTWARE, YOU ACKNOWLEDGE THAT YOU HAVE READ, UNDERSTOOD, AND AGREED TO ALL THE DISCLAIMER TERMS ABOVE. IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS SOFTWARE.**

## 📸 Feature Screenshots

### Theme Customization

![Midsummer's Bird Theme Customization](https://files.catbox.moe/5pg819.jpg)

### Render Preview

![Midsummer's Bird Render Preview](https://files.catbox.moe/qzbjn9.jpg)



---

### ⚠️ **Important Note: Regarding Imported Content**

Please be aware that `Midsummer's Bird` is a tool, and its import features (including but not limited to Character Cards, World Info, AI Presets, Chat Histories, etc.) are intended solely for users to manage and use content they **have the legal right to use**.

**Before importing any file, you must ensure that you have permission from the original author, or that the file is distributed under an open-source license that permits its use.**

Please respect the intellectual property and hard work of every creator. Do not import, use, or distribute original content from others without their explicit permission.

---




-----

## 💖 Acknowledgments

  - Inspired by SillyTavern
  - Thanks to open-source projects like Tailwind CSS, Font Awesome, and Sortable.js
  - Thanks to all the users who use and support this project

-----

## 📞 Contact

  - **Issues**: [GitHub Issues](https://github.com/csjafuwvbegscbw-star)
  - [bilibili](https://space.bilibili.com/98572211)

-----

## ⭐ Star History

If you find this project useful, please give it a ⭐️ Star\!

-----

**Enjoy your conversation with AI\! 🎭✨**
