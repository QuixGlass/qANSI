/*
 * qANSI.h - Quick ANSI Terminal Control Library
 * 
 * A single-header library for controlling ANSI terminals on Arduino.
 * Provides direct terminal operations without buffering.
 *
 * Features:
 * - Basic ANSI terminal control (cursor, colors, attributes)
 * - Direct output to terminal
 * - Minimal memory footprint
 * - Inherits from Arduino's Print class
 *
 * License: MIT License
 */

#ifndef Q_ANSI_H
#define Q_ANSI_H

#include <Arduino.h>
#include <Print.h>

// --- ANSI Color/Attribute Constants ---
namespace qANSI_Colors {
    const uint8_t FG_BLACK   = 30;
    const uint8_t FG_RED     = 31;
    const uint8_t FG_GREEN   = 32;
    const uint8_t FG_YELLOW  = 33;
    const uint8_t FG_BLUE    = 34;
    const uint8_t FG_MAGENTA = 35;
    const uint8_t FG_CYAN    = 36;
    const uint8_t FG_WHITE   = 37;
    const uint8_t FG_DEFAULT = 39;

    const uint8_t BG_BLACK   = 40;
    const uint8_t BG_RED     = 41;
    const uint8_t BG_GREEN   = 42;
    const uint8_t BG_YELLOW  = 43;
    const uint8_t BG_BLUE    = 44;
    const uint8_t BG_MAGENTA = 45;
    const uint8_t BG_CYAN    = 46;
    const uint8_t BG_WHITE   = 47;
    const uint8_t BG_DEFAULT = 49;

    // Bright foreground colors 
    const uint8_t FG_BRIGHT_BLACK   = 90; // Often gray
    const uint8_t FG_BRIGHT_RED     = 91;
    const uint8_t FG_BRIGHT_GREEN   = 92;
    const uint8_t FG_BRIGHT_YELLOW  = 93;
    const uint8_t FG_BRIGHT_BLUE    = 94;
    const uint8_t FG_BRIGHT_MAGENTA = 95;
    const uint8_t FG_BRIGHT_CYAN    = 96;
    const uint8_t FG_BRIGHT_WHITE   = 97;
}

namespace qANSI_Attributes {
    const uint8_t RESET        = 0;
    const uint8_t BOLD         = 1;
    const uint8_t UNDERLINE    = 4;
    const uint8_t BLINK        = 5;
    const uint8_t REVERSE      = 7;
    const uint8_t CONCEALED    = 8;

    // Attribute off codes
    const uint8_t BOLD_OFF       = 22;
    const uint8_t UNDERLINE_OFF  = 24;
    const uint8_t BLINK_OFF      = 25;
    const uint8_t REVERSE_OFF    = 27;
    const uint8_t CONCEALED_OFF  = 28;
}

class qANSI : public Print {
public:
    // Constructor
    qANSI(Stream &output = Serial) : _output(output), _cursorVisible(true) {
        // Initialize state tracking
        _currentFg = qANSI_Colors::FG_DEFAULT;
        _currentBg = qANSI_Colors::BG_DEFAULT;
        _currentAttr = qANSI_Attributes::RESET;
    }

    // Initialize terminal
    void begin(uint8_t defaultFg = qANSI_Colors::FG_DEFAULT, 
               uint8_t defaultBg = qANSI_Colors::BG_DEFAULT) {
        _currentFg = defaultFg;
        _currentBg = defaultBg;
        _currentAttr = qANSI_Attributes::RESET;
        
        // Reset terminal
        resetAttributes();
        clearScreen();
    }

    // --- Direct Terminal Control Methods ---
    
    // Clear entire screen
    void clearScreen() {
        _sendAnsiCommand("\033[2J");
        setCursor(1, 1);
    }
    
    // Clear from cursor to end of screen
    void clearToEndOfScreen() {
        _sendAnsiCommand("\033[0J");
    }
    
    // Clear from cursor to end of line
    void clearToEndOfLine() {
        _sendAnsiCommand("\033[0K");
    }
    
    // Set cursor position (ANSI is 1-based)
    void setCursor(uint8_t col, uint8_t row) {
        char posBuf[20];
        sprintf(posBuf, "\033[%d;%dH", row, col);
        _sendAnsiCommand(posBuf);
    }
    
    // Move cursor up
    void cursorUp(uint8_t lines = 1) {
        char buf[15];
        sprintf(buf, "\033[%dA", lines);
        _sendAnsiCommand(buf);
    }
    
    // Move cursor down
    void cursorDown(uint8_t lines = 1) {
        char buf[15];
        sprintf(buf, "\033[%dB", lines);
        _sendAnsiCommand(buf);
    }
    
    // Move cursor right
    void cursorRight(uint8_t cols = 1) {
        char buf[15];
        sprintf(buf, "\033[%dC", cols);
        _sendAnsiCommand(buf);
    }
    
    // Move cursor left
    void cursorLeft(uint8_t cols = 1) {
        char buf[15];
        sprintf(buf, "\033[%dD", cols);
        _sendAnsiCommand(buf);
    }
    
    // Set cursor visibility
    void setCursorVisible(bool visible) {
        _cursorVisible = visible;
        _sendAnsiCommand(visible ? "\033[?25h" : "\033[?25l");
    }
    
    bool isCursorVisible() const {
        return _cursorVisible;
    }
    
    // --- Text Appearance Methods ---
    
    // Set foreground color
    void setTextColor(uint8_t fg) {
        _currentFg = fg;
        char buf[15];
        sprintf(buf, "\033[%dm", fg);
        _sendAnsiCommand(buf);
    }
    
    // Set background color
    void setTextBackgroundColor(uint8_t bg) {
        _currentBg = bg;
        char buf[15];
        sprintf(buf, "\033[%dm", bg);
        _sendAnsiCommand(buf);
    }
    
    // Set both colors
    void setTextColor(uint8_t fg, uint8_t bg) {
        char buf[20];
        sprintf(buf, "\033[%d;%dm", fg, bg);
        _sendAnsiCommand(buf);
        _currentFg = fg;
        _currentBg = bg;
    }
    
    // Set text attribute
    void setTextAttribute(uint8_t attr) {
        _currentAttr = attr;
        char buf[15];
        sprintf(buf, "\033[%dm", attr);
        _sendAnsiCommand(buf);
    }
    
    // Reset all text attributes
    void resetAttributes() {
        _sendAnsiCommand("\033[0m");
        _currentAttr = qANSI_Attributes::RESET;
        _currentFg = qANSI_Colors::FG_DEFAULT;
        _currentBg = qANSI_Colors::BG_DEFAULT;
    }
    
    // Save cursor position
    void saveCursor() {
        _sendAnsiCommand("\033[s");
    }
    
    // Restore cursor position
    void restoreCursor() {
        _sendAnsiCommand("\033[u");
    }
    
    // Implement Print's write method
    virtual size_t write(uint8_t c) override {
        return _output.write(c);
    }
    
    // Get current foreground color
    uint8_t getCurrentFgColor() const {
        return _currentFg;
    }
    
    // Get current background color
    uint8_t getCurrentBgColor() const {
        return _currentBg;
    }
    
    // Get current attribute
    uint8_t getCurrentAttribute() const {
        return _currentAttr;
    }

protected:
    Stream &_output;
    uint8_t _currentFg;
    uint8_t _currentBg;
    uint8_t _currentAttr;
    bool _cursorVisible;
    
    // Helper to send raw ANSI command string
    void _sendAnsiCommand(const char* command) {
        _output.print(command);
    }
};

#endif // Q_ANSI_H
