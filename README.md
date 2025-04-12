# qANSI - Lightweight ANSI Terminal Control Library for Arduino

qANSI is a lightweight yet powerful library for controlling ANSI terminals from Arduino. It provides direct terminal operations and virtual terminal support with optimized rendering.

## Features

### qANSI - Basic ANSI Terminal Control

- **Direct Terminal Control**: Cursor positioning, color setting, text attributes
- **Minimal Memory Footprint**: Efficient implementation for resource-constrained environments
- **Standard Integration**: Inherits from Arduino's Print class for familiar interface
- **State Tracking**: Intelligent tracking of terminal state to minimize commands

### qANSI_VT - Virtual Terminal System

- **In-Memory Buffer**: Represents the terminal state in memory for optimal updates
- **Change Detection**: Only sends ANSI commands for cells that have changed
- **Multiple Update Strategies**: Adaptive rendering based on change patterns
- **Position Anywhere**: Place virtual terminals anywhere on the physical screen
- **Scrolling & Wrapping**: Full support for content overflow with configurable behaviors

## Installation

1. Download the repository
2. Copy the `qANSI.h` and `qANSI_VT.h` files to your project directory
3. Include the headers in your sketch

## Basic Usage

### Simple Terminal Control

```cpp
#include "qANSI.h"

qANSI terminal(Serial);

void setup() {
  Serial.begin(115200);
  
  // Initialize terminal
  terminal.begin();
  terminal.clearScreen();
  
  // Move cursor and print colored text
  terminal.setCursor(10, 5);
  terminal.setTextColor(qANSI_Colors::FG_GREEN);
  terminal.print("Hello, ANSI Terminal!");
  
  // Add some styling
  terminal.setCursor(10, 7);
  terminal.setTextAttribute(qANSI_Attributes::BOLD);
  terminal.setTextColor(qANSI_Colors::FG_RED);
  terminal.print("This text is bold and red!");
}

void loop() {
  // Your code here
}
```

### Virtual Terminal Example

```cpp
#include "qANSI.h"
#include "qANSI_VT.h"

// Create a virtual terminal of size 30x10 positioned at (5,5)
qANSI_VT vt(30, 10, 5, 5, Serial);

void setup() {
  Serial.begin(115200);
  
  // Initialize a virtual terminal
  vt.begin(qANSI_Colors::FG_WHITE, qANSI_Colors::BG_BLUE);
  vt.clear();
  
  // Set text properties
  vt.setTextColor(qANSI_Colors::FG_YELLOW);
  vt.setTextAttribute(qANSI_Attributes::BOLD);
  
  // Print to the virtual terminal
  vt.setCursor(1, 1);
  vt.println("This is a virtual terminal!");
  vt.println("It has its own buffer and");
  vt.println("only updates what changes.");
  
  // Display the terminal content
  vt.display();
}

void loop() {
  // Update content
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    // Print the current millis
    vt.setCursor(1, 5);
    vt.print("Millis: ");
    vt.print(millis());
    
    // Update the screen - only changed cells will be redrawn
    vt.display();
    
    lastUpdate = millis();
  }
}
```

## Multiple Virtual Terminals

```cpp
#include "qANSI.h"
#include "qANSI_VT.h"

// Create two virtual terminals
qANSI_VT vt1(20, 8, 2, 2, Serial);  // 20x8 at position (2,2)
qANSI_VT vt2(20, 8, 2, 12, Serial); // 20x8 at position (2,12)

void setup() {
  Serial.begin(115200);
  
  // Initialize terminal 1 - Blue background
  vt1.begin(qANSI_Colors::FG_WHITE, qANSI_Colors::BG_BLUE);
  vt1.clear();
  vt1.println("Terminal 1");
  vt1.display();
  
  // Initialize terminal 2 - Red background
  vt2.begin(qANSI_Colors::FG_WHITE, qANSI_Colors::BG_RED);
  vt2.clear();
  vt2.println("Terminal 2");
  vt2.display();
}

void loop() {
  // Update terminals independently
  static unsigned long lastUpdate1 = 0;
  static unsigned long lastUpdate2 = 0;
  
  // Update terminal 1 every second
  if (millis() - lastUpdate1 > 1000) {
    vt1.setCursor(1, 3);
    vt1.print("Update: ");
    vt1.print(millis() / 1000);
    vt1.display();
    lastUpdate1 = millis();
  }
  
  // Update terminal 2 every 1.5 seconds
  if (millis() - lastUpdate2 > 1500) {
    vt2.setCursor(1, 3);
    vt2.print("Update: ");
    vt2.print(millis() / 1000);
    vt2.display();
    lastUpdate2 = millis();
  }
}
```

## Advanced Features

### Line Wrapping and Scrolling

```cpp
// Enable automatic line wrapping
vt.setLineWrapping(true);

// Enable automatic scrolling when cursor moves past bottom
vt.setScrolling(true);

// Manually scroll up by 2 lines
vt.scrollUp(2);
```

### Cursor Control

```cpp
// Show or hide the cursor
vt.setCursorVisible(true);

// Position the cursor
vt.setCursor(10, 5);

// Get current cursor position
uint8_t x = vt.getCursorX();
uint8_t y = vt.getCursorY();
```

### Debug Mode

```cpp
// Print with detailed cursor movement tracing
vt.debugPrint("This will trace every character");
```

## Optimization Notes

The virtual terminal implementation uses three different update strategies to optimize performance:

1. **Full Redraw**: Used when most cells have changed or on first display
2. **Row-Based Update**: Redraws entire rows that contain any changes
3. **Sparse Update**: Targets only specific changed cells when changes are minimal

These strategies are automatically selected based on the pattern of changes to minimize the amount of data sent to the terminal.

## License

MIT License
