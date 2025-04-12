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

// --- Pipe color code definitions ---
namespace qANSI_PipeCodes {
    // Foreground colors (Renegade BBS style)
    const char* FG_BLACK        = "|00";  // Black
    const char* FG_BLUE         = "|01";  // Blue
    const char* FG_GREEN        = "|02";  // Green
    const char* FG_CYAN         = "|03";  // Cyan
    const char* FG_RED          = "|04";  // Red
    const char* FG_PURPLE       = "|05";  // Purple (Magenta)
    const char* FG_BROWN        = "|06";  // Brown (Dark Yellow)
    const char* FG_GREY         = "|07";  // Grey (White)
    
    // Bright foreground colors
    const char* FG_DARK_GREY    = "|08";  // Dark Grey (Bright Black)
    const char* FG_BRIGHT_BLUE  = "|09";  // Bright Blue
    const char* FG_BRIGHT_GREEN = "|10";  // Bright Green
    const char* FG_BRIGHT_CYAN  = "|11";  // Bright Cyan
    const char* FG_BRIGHT_RED   = "|12";  // Bright Red
    const char* FG_BRIGHT_PURPLE = "|13";  // Bright Purple (Bright Magenta)
    const char* FG_YELLOW       = "|14";  // Yellow (Bright Yellow)
    const char* FG_BRIGHT_WHITE = "|15";  // Bright White
    
    // Background colors
    const char* BG_BLACK        = "|16";  // Black background
    const char* BG_BLUE         = "|17";  // Blue background
    const char* BG_GREEN        = "|18";  // Green background
    const char* BG_CYAN         = "|19";  // Cyan background
    const char* BG_RED          = "|20";  // Red background
    const char* BG_PURPLE       = "|21";  // Purple background
    const char* BG_BROWN        = "|22";  // Brown background
    const char* BG_WHITE        = "|23";  // White background
    
    // Additional special codes
    const char* RESET           = "|RA";  // Reset all attributes
    const char* BOLD            = "|B1";  // Bold
    const char* UNDERLINE       = "|U1";  // Underline
    const char* BLINK           = "|F1";  // Blink
    const char* REVERSE         = "|R1";  // Reverse (inverse)
    const char* BOLD_OFF        = "|B0";  // Bold off
    const char* UNDERLINE_OFF   = "|U0";  // Underline off
    const char* BLINK_OFF       = "|F0";  // Blink off
    const char* REVERSE_OFF     = "|R0";  // Reverse off
}

class qANSI : public Print {
public:
    // Constructor
    qANSI(Stream &output = Serial) : _output(output), _cursorVisible(true), _pipeCodesEnabled(true) {
        // Initialize state tracking
        _currentFg = qANSI_Colors::FG_DEFAULT;
        _currentBg = qANSI_Colors::BG_DEFAULT;
        _currentAttr = qANSI_Attributes::RESET;
        
        // Initialize pipe code processing state
        _pipeSequenceState = 0;
        _pipeChar1 = '\0';
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
    
    // --- Pipe Code Methods ---
    
    // Enable or disable pipe code processing
    void enablePipeCodes(bool enable) {
        _pipeCodesEnabled = enable;
        if (!enable) {
            // Reset pipe code state machine if disabled
            _pipeSequenceState = 0;
        }
    }
    
    bool arePipeCodesEnabled() const {
        return _pipeCodesEnabled;
    }
    
    // Implement Print's write method with pipe code handling
    virtual size_t write(uint8_t c) override {
        // If pipe codes are disabled, just pass through
        if (!_pipeCodesEnabled) {
            return _output.write(c);
        }
        
        // Handle pipe codes
        switch (_pipeSequenceState) {
            case 0: // Not in a pipe sequence
                if (c == '|') {
                    _pipeSequenceState = 1;
                    return 1; // Count as written even though we're buffering
                } else {
                    return _output.write(c);
                }
                break;
                
            case 1: // Got a pipe, waiting for first command character
                _pipeChar1 = c;
                _pipeSequenceState = 2;
                return 1;
                
            case 2: // Got first command char, waiting for second command char
                _pipeSequenceState = 0; // Reset state machine
                return _processPipeCode(_pipeChar1, c);
                
            default:
                _pipeSequenceState = 0;
                return _output.write(c);
        }
    }
    
// Write string, processing pipe codes
size_t print(const char* str) {
    size_t count = 0;
    while (*str) {
        count += write((uint8_t)*str++);
    }
    return count;
}

// Print just a line ending
size_t println() {
    return write('\r') + write('\n');
}

// Print line with pipe code processing
size_t println(const char* str) {
    size_t count = print(str);
    count += println();
    return count;
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
    bool _pipeCodesEnabled;
    
    // Pipe code state machine
    uint8_t _pipeSequenceState;
    char _pipeChar1;
    
    // Helper to send raw ANSI command string
    void _sendAnsiCommand(const char* command) {
        _output.print(command);
    }
    
    // Process pipe code and return how many bytes were "written"
// Process pipe code and return how many bytes were "written"
size_t _processPipeCode(char c1, char c2) {
    // Form the two-digit code
    int code = (c1 - '0') * 10 + (c2 - '0');
    
    // Check for valid code range
    if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9') {
        switch (code) {
            // Foreground colors
            case 0: setTextColor(qANSI_Colors::FG_BLACK); break;
            case 1: setTextColor(qANSI_Colors::FG_BLUE); break;
            case 2: setTextColor(qANSI_Colors::FG_GREEN); break;
            case 3: setTextColor(qANSI_Colors::FG_CYAN); break;
            case 4: setTextColor(qANSI_Colors::FG_RED); break;
            case 5: setTextColor(qANSI_Colors::FG_MAGENTA); break;
            case 6: setTextColor(qANSI_Colors::FG_YELLOW); break; // Brown in some terminals
            case 7: setTextColor(qANSI_Colors::FG_WHITE); break;
            
            // Bright foreground colors
            case 8: setTextColor(qANSI_Colors::FG_BRIGHT_BLACK); break; // Dark Grey
            case 9: setTextColor(qANSI_Colors::FG_BRIGHT_BLUE); break;
            case 10: setTextColor(qANSI_Colors::FG_BRIGHT_GREEN); break;
            case 11: setTextColor(qANSI_Colors::FG_BRIGHT_CYAN); break;
            case 12: setTextColor(qANSI_Colors::FG_BRIGHT_RED); break;
            case 13: setTextColor(qANSI_Colors::FG_BRIGHT_MAGENTA); break;
            case 14: setTextColor(qANSI_Colors::FG_BRIGHT_YELLOW); break;
            case 15: setTextColor(qANSI_Colors::FG_BRIGHT_WHITE); break;
            
            // Background colors
            case 16: setTextBackgroundColor(qANSI_Colors::BG_BLACK); break;
            case 17: setTextBackgroundColor(qANSI_Colors::BG_BLUE); break;
            case 18: setTextBackgroundColor(qANSI_Colors::BG_GREEN); break;
            case 19: setTextBackgroundColor(qANSI_Colors::BG_CYAN); break;
            case 20: setTextBackgroundColor(qANSI_Colors::BG_RED); break;
            case 21: setTextBackgroundColor(qANSI_Colors::BG_MAGENTA); break;
            case 22: setTextBackgroundColor(qANSI_Colors::BG_YELLOW); break; // Brown
            case 23: setTextBackgroundColor(qANSI_Colors::BG_WHITE); break;
            
            // Special codes
            case 24: resetAttributes(); break;
            case 25: setTextAttribute(qANSI_Attributes::BOLD); break;
            case 26: setTextAttribute(qANSI_Attributes::UNDERLINE); break;
            case 27: setTextAttribute(qANSI_Attributes::BLINK); break;
            case 28: setTextAttribute(qANSI_Attributes::REVERSE); break;
            case 29: setTextAttribute(qANSI_Attributes::BOLD_OFF); break;
            case 30: setTextAttribute(qANSI_Attributes::UNDERLINE_OFF); break;
            case 31: setTextAttribute(qANSI_Attributes::BLINK_OFF); break;
            case 32: setTextAttribute(qANSI_Attributes::REVERSE_OFF); break;
            
            default:
                // Not a supported code, output the original sequence
                _output.write('|');
                _output.write(c1);
                return _output.write(c2) + 2;
        }
        return 3; // Pipe code handled (3 characters: |nn)
    } else {
        // Not a valid numeric code, output the original sequence
        _output.write('|');
        _output.write(c1);
        return _output.write(c2) + 2;
    }
}
};

#endif // Q_ANSI_H
