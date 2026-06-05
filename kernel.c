void kernel_main() {
    char *video_memory = (char*) 0xB8000;
    
    // Clear screen initially
    for(int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i+1] = 0x07;
    }
    
    // Print OS Title Header
    char *header = "--- Simple-Unix OS v1.0 [Full Keyboard Active] ---";
    int idx = 0;
    while(header[idx] != '\0') {
        video_memory[idx * 2] = header[idx];
        video_memory[idx * 2 + 1] = 0x0B; // Cyan header
        idx++;
    }
    
    int cursor_pos = 160;       // Start on line 2
    char command_buffer[80];    // Stores characters typed by user
    int cmd_idx = 0;            // Word length tracker
    int is_asking_password = 0; // Password mode flag

    // --- FULL KEYBOARD SCANCODE MAP ---
    // Index matches the IBM PC hardware scancode sent by the keyboard
    char keyboard_map[128];
    for(int i = 0; i < 128; i++) keyboard_map[i] = 0; // Clear map first

    // Numbers Row
    keyboard_map[0x02] = '1'; keyboard_map[0x03] = '2'; keyboard_map[0x04] = '3';
    keyboard_map[0x05] = '4'; keyboard_map[0x06] = '5'; keyboard_map[0x07] = '6';
    keyboard_map[0x08] = '7'; keyboard_map[0x09] = '8'; keyboard_map[0x0A] = '9';
    keyboard_map[0x0B] = '0';

    // Letters - Row 1 (QWERTY)
    keyboard_map[0x10] = 'q'; keyboard_map[0x11] = 'w'; keyboard_map[0x12] = 'e';
    keyboard_map[0x13] = 'r'; keyboard_map[0x14] = 't'; keyboard_map[0x15] = 'y';
    keyboard_map[0x16] = 'u'; keyboard_map[0x17] = 'i'; keyboard_map[0x18] = 'o';
    keyboard_map[0x19] = 'p';

    // Letters - Row 2 (ASDF)
    keyboard_map[0x1E] = 'a'; keyboard_map[0x1F] = 's'; keyboard_map[0x20] = 'd';
    keyboard_map[0x21] = 'f'; keyboard_map[0x22] = 'g'; keyboard_map[0x23] = 'h';
    keyboard_map[0x24] = 'j'; keyboard_map[0x25] = 'k'; keyboard_map[0x26] = 'l';

    // Letters - Row 3 (ZXCV)
    keyboard_map[0x2C] = 'z'; keyboard_map[0x2D] = 'x'; keyboard_map[0x2E] = 'c';
    keyboard_map[0x2F] = 'v'; keyboard_map[0x30] = 'b'; keyboard_map[0x31] = 'n';
    keyboard_map[0x32] = 'm';

    // Special Keys
    keyboard_map[0x39] = ' '; // Spacebar

    // Print prompt symbol
    video_memory[cursor_pos] = '>';
    video_memory[cursor_pos+1] = 0x0E; // Yellow prompt color
    video_memory[cursor_pos+2] = ' ';
    cursor_pos += 4; 

    unsigned char last_scancode = 0;

    while(1) {
        unsigned char status = 0;
        __asm__ volatile("inb $0x64, %0" : "=a"(status));
        
        if(status & 0x01) {
            unsigned char scancode = 0;
            __asm__ volatile("inb $0x60, %0" : "=a"(scancode));
            
            if (scancode != last_scancode) {
                if (scancode < 0x80) { // Key Press event
                    
                    // ==========================================
                    // CASE 1: ENTER KEY (0x1C)
                    // ==========================================
                    if (scancode == 0x1C) {
                        command_buffer[cmd_idx] = '\0'; 
                        cursor_pos = ((cursor_pos / 160) + 1) * 160; // Break to next line
                        
                        if (is_asking_password) {
                            if (command_buffer[0] == '1' && command_buffer[1] == '2' && command_buffer[2] == '3' && command_buffer[3] == '4' && command_buffer[4] == '\0') {
                                char *success = "Reading package lists... Done\nBuilding dependency tree... Done\nFetching layout... [OK]";
                                int s = 0;
                                while(success[s] != '\0') {
                                    if(success[s] == '\n') {
                                        cursor_pos = ((cursor_pos / 160) + 1) * 160;
                                    } else {
                                        video_memory[cursor_pos] = success[s];
                                        video_memory[cursor_pos+1] = 0x0A; // Green
                                        cursor_pos += 2;
                                    }
                                    s++;
                                }
                            } else {
                                char *fail = "vnc-sudo: Incorrect password attempt.";
                                int f = 0;
                                while(fail[f] != '\0') {
                                    video_memory[cursor_pos] = fail[f];
                                    video_memory[cursor_pos+1] = 0x0C; // Red
                                    cursor_pos += 2;
                                    f++;
                                }
                            }
                            is_asking_password = 0;
                        } 
                        else {
                            // Advanced APT parsing check: does it start with "install "?
                            if (command_buffer[0] == 'i' && command_buffer[1] == 'n' && command_buffer[2] == 's' && command_buffer[3] == 't' && command_buffer[4] == 'a' && command_buffer[5] == 'l' && command_buffer[6] == 'l' && command_buffer[7] == ' ') {
                                char *pass_prompt = "[sudo] password for root: ";
                                int p = 0;
                                while(pass_prompt[p] != '\0') {
                                    video_memory[cursor_pos] = pass_prompt[p];
                                    video_memory[cursor_pos+1] = 0x0E; 
                                    cursor_pos += 2;
                                    p++;
                                }
                                is_asking_password = 1;
                            } 
                            else if (command_buffer[0] == 'c' && command_buffer[1] == 'l' && command_buffer[2] == 'e' && command_buffer[3] == 'a' && command_buffer[4] == 'r' && command_buffer[5] == '\0') {
                                for(int i = 160; i < 80 * 25 * 2; i += 2) {
                                    video_memory[i] = ' ';
                                    video_memory[i+1] = 0x07;
                                }
                                cursor_pos = 160;
                            }
                            else if (cmd_idx > 0) {
                                char *err = "E: Command unknown. Try: install [package] or clear";
                                int e = 0;
                                while(err[e] != '\0') {
                                    video_memory[cursor_pos] = err[e];
                                    video_memory[cursor_pos+1] = 0x0C;
                                    cursor_pos += 2;
                                    e++;
                                }
                            }
                        }

                        if (!is_asking_password) {
                            cursor_pos = ((cursor_pos / 160) + 1) * 160;
                            cmd_idx = 0;
                            video_memory[cursor_pos] = '>';
                            video_memory[cursor_pos+1] = 0x0E;
                            video_memory[cursor_pos+2] = ' ';
                            cursor_pos += 4;
                        } else {
                            cmd_idx = 0; 
                        }
                    }
                    
                    // ==========================================
                    // CASE 2: LOOK UP MAP FOR LETTERS / NUMBERS
                    // ==========================================
                    else {
                        char typed_char = keyboard_map[scancode];

                        if (typed_char != 0 && cmd_idx < 79) {
                            if (is_asking_password) {
                                video_memory[cursor_pos] = '*';
                            } else {
                                video_memory[cursor_pos] = typed_char;
                            }
                            
                            video_memory[cursor_pos+1] = 0x0F; // High white text
                            cursor_pos += 2;
                            command_buffer[cmd_idx] = typed_char;
                            cmd_idx++;
                        }
                    }
                }
                last_scancode = scancode;
            }
        }
    }
}

