# Simple Vulkan Solar System
[![Ask DeepWiki](https://devin.ai/assets/askdeepwiki.png)](https://deepwiki.com/Nsr-Alhabsi/Simple_Vulkan_SolarSystem)

This project is a simple 2D solar system simulation built from the ground up using the Vulkan graphics API in C++. It demonstrates core Vulkan concepts such as the rendering pipeline, swap chains, command buffers, and shader management. The simulation displays a sun, an orbiting planet, and a moon, all animated using a custom animation system.

## Features

- **Vulkan Rendering:** A custom, lightweight rendering engine built on the Vulkan API.
- **2D Solar System:** Renders a simple solar system with a sun, planet, and moon.
- **Animation System:** `lvs_game_animations` provides a flexible animation framework supporting:
    - Rotation, Translation, and Scaling animations.
    - A wide variety of easing functions (Linear, Sine, Quad, Cubic, Expo, etc.) for smooth motion.
    - Animation repetition, delays, and callbacks.
- **JSON Configuration:** Easily configure window settings like size, title, and background color via `AppSettings.json`.
- **Component-Based Objects:** Uses a simple game object and transform component structure.
- **GLSL Shaders:** Utilizes GLSL for vertex and fragment shaders to draw objects.

## Getting Started

Follow these instructions to get a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You will need the following software installed on your system:

- **Vulkan SDK:** Download and install the Vulkan SDK from [LunarG](https://www.lunarg.com/vulkan-sdk/).
- **GLFW:** A multi-platform library for window and input management.
- **GLM:** OpenGL Mathematics library.
- **nlohmann/json:** A JSON library for C++.
- **C++17 Compiler:** A compiler that supports C++17, such as GCC (g++). `make` and other build tools are also required.

This project is configured to be built in a MinGW or similar Unix-like environment on Windows.

### Installation & Building

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/nsr-alhabsi/simple_vulkan_solarsystem.git
    cd simple_vulkan_solarsystem
    ```

2.  **Update Vulkan SDK Path:**
    The `compile.bat` script has a hardcoded path to the Vulkan SDK. You may need to edit it to match your installation path.
    ```batch
    // compile.bat
    C:\VulkanSDK\<Your-SDK-Version>\Bin\glslc.exe ...
    ```

3.  **Compile Shaders:**
    Run the `compile.bat` script to compile the GLSL shaders into SPIR-V format. This will generate `.spv` files in the `shaders/` directory.
    ```bash
    ./compile.bat
    ```

4.  **Build the Project:**
    Use the provided `makefile` to compile the C++ source code.
    ```bash
    make all
    ```
    This will create an executable file `MyVulkanProject.exe` inside a newly created `build/` directory.

## Usage

After successfully building the project, you can run the simulation using the `make` command:

```bash
make run
```

Alternatively, you can run the executable directly:

```bash
./build/MyVulkanProject.exe
```

To clean the build files, run:

```bash
make clean
```

## Configuration

You can customize the simulation's display settings by modifying the `AppSettings.json` file. This allows you to change the window dimensions, title, and background color without recompiling the code.

```json
{
  "Display_Settings": {
    "Screen": {
      "Height": 1300,
      "Width": 1300
    },
    "Background_Color": [0.1, 0.1, 0.1, 1.0],
    "Window_Title": "Vulkan Simulation",
    "viewport": {
      "Same_as_Screen": true,
      "Start_point": {
        "X": 0,
        "Y": 0
      },
      "End_Point": {
        "X": 800,
        "Y": 600
      },
      "Min_Depth": 0,
      "Max_Depth": 1
    }
  }
}
```

## Project Structure

The repository is organized into several key directories:

-   `src/`: Contains all C++ source code.
    -   `core/`: Core Vulkan engine components (device, pipeline, swap chain, window setup).
    -   `models/`: Game objects, model data, render systems, and the animation manager.
    -   `ADDONS/`: Utility files for JSON parsing and console color output.
    -   `main.cpp`: The main entry point of the application.
    -   `vulkan_simulation.hpp/.cpp`: The main application class that orchestrates the simulation.
-   `shaders/`: Contains GLSL vertex and fragment shaders (`.vert`, `.frag`) and their compiled SPIR-V counterparts (`.spv`).
-   `AppSettings.json`: Configuration file for display settings.
-   `makefile`: Build script for compiling the C++ application.
-   `compile.bat`: Batch script for compiling shaders.