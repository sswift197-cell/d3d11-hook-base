🎯 Aimsy

Aimsy is an open-source, lightweight CS2 training companion focused on improving aim, mechanics, and consistency through structured drills and clean UI.

<p align="center"> <img src="./assets/aimsy-banner.png" width="720" alt="Aimsy banner"> </p>

Train smarter. Build consistency. Frag with confidence.

✨ Features

🎯 Aim & reflex training modules

🧠 Warm-up routines inspired by CS2 mechanics

📊 Session stats & progress tracking

🖥️ Clean ImGui-based interface

⚡ Fast, low-overhead standalone app

🎨 Configurable visuals & controls

📁 Project Structure
aimsy/
├─ src/
│  ├─ core/        # Core logic
│  ├─ ui/          # ImGui UI
│  ├─ features/    # Training modules
│  └─ utils/
├─ assets/
│  ├─ fonts/
│  └─ images/
├─ external/
│  └─ imgui/
├─ configs/
├─ CMakeLists.txt
└─ README.md

🛠️ Building Aimsy
Requirements

Windows 10 / 11

Visual Studio 2022

CMake 3.20+

DirectX 11

C++20

🔧 Build Instructions
git clone https://github.com/yourname/aimsy.git
cd aimsy
mkdir build
cd build
cmake .. -A x64


Then:

Open the generated .sln file

Select Release | x64

Build the solution

The compiled binary will be located in:

build/bin/Release/

▶️ Running

Aimsy runs as a standalone desktop application:

Aimsy.exe


Use it for:

pre-match warm-ups

offline practice sessions

tracking improvement over time

🔌 CS2 Integration Philosophy

Aimsy is designed to be non-invasive.

Supported / planned approaches include:

🎥 Demo (.dem) analysis

🗺️ Workshop training maps

📊 External stat imports

🧠 Manual performance tracking

Aimsy does not modify game memory or interact with CS2 internals.

⚙️ Configuration

Configuration files are stored in:

configs/


Example:

{
  "sensitivity": 1.6,
  "fov": 90,
  "theme": "dark"
}


Configs are hot-reloadable during runtime.

🎨 UI

Built using Dear ImGui

Modern rounded layout

Sidebar tab navigation

Dark-mode first design

🤝 Contributing

Contributions are welcome ❤️

Please:

keep code readable and documented

follow existing formatting and style

avoid game-modifying or invasive techniques

open an issue before large changes

🧪 Roadmap

 More drill types

 Advanced stats & charts

 Preset routines

 Cross-platform support (experimental)

📜 License

This project is licensed under the MIT License.
See LICENSE for details.

⚠️ Disclaimer

Aimsy is a training utility intended for educational and practice purposes only.
It does not alter or interfere with Counter-Strike 2.

💖 Credits

Dear ImGui

CS community

Aimsy contributors
