// file: serial.c

extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);

#define COM1 0x3F8
#define MAX_NODES 64
#define BIN_FILE_SIZE 1024

#define TYPE_FREE 0
#define TYPE_DIR  1
#define TYPE_FILE 2
void helper_print_num(int num, char *video_memory, int *cursor_ptr) {
    char buf[10];
    int i = 0;
    int cursor_pos = *cursor_ptr; // Dereference it here
    
    // ... [keep your existing code loop inside the function body exactly as it is] ...
    
    *cursor_ptr = cursor_pos; // Save the updated cursor pos back before returning
}

struct FSNode {
    char name[16];
    int type;                  
    int parent_index;          
    unsigned char code[BIN_FILE_SIZE]; 
    int size;                  
};

struct FSNode fs_nodes[MAX_NODES];
int current_dir_idx = 0; 

int mystrcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] != '\0' || s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return 0;
}

int find_free_node() {
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type == TYPE_FREE) return i;
    }
    return -1;
}

void init_filesystem() {
    for (int i = 0; i < MAX_NODES; i++) fs_nodes[i].type = TYPE_FREE;
    
    fs_nodes[0].type = TYPE_DIR;
    fs_nodes[0].name[0] = '/';
    fs_nodes[0].name[1] = '\0';
    fs_nodes[0].parent_index = 0; 
    
    fs_nodes[1].type = TYPE_DIR;
    fs_nodes[1].name[0] = 'b'; fs_nodes[1].name[1] = 'i'; fs_nodes[1].name[2] = 'n'; fs_nodes[1].name[3] = '\0';
    fs_nodes[1].parent_index = 0;
}

void serial_putc(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0); 
    outb(COM1, c);
}
void serial_print(const char *str) {
    while (*str) serial_putc(*str++);
}
char serial_getc() {
    while ((inb(COM1 + 5) & 0x01) == 0); 
    return inb(COM1);
}

void download_package_from_debian(const char *package_name, char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    int slot = find_free_node();

    if (slot == -1) {
        char *err = "E: Out of memory block descriptors.\n";
        while(*err) { video_memory[cursor_pos] = *err++; video_memory[cursor_pos+1] = 0x0C; cursor_pos += 2; }
        *cursor_ptr = cursor_pos; return;
    }

    fs_nodes[slot].type = TYPE_FILE;
    fs_nodes[slot].parent_index = 1; 
    fs_nodes[slot].size = 0;

    int n_idx = 0;
    while(package_name[n_idx] != '\0' && n_idx < 15) {
        fs_nodes[slot].name[n_idx] = package_name[n_idx];
        n_idx++;
    }
    fs_nodes[slot].name[n_idx] = '\0';

    char *mock_program_text = "NANO EDITOR V1.0: Active and processing document buffers.\n\r";
    
    int p_idx = 0;
    while (mock_program_text[p_idx] != '\0' && fs_nodes[slot].size < (BIN_FILE_SIZE - 1)) {
        fs_nodes[slot].code[fs_nodes[slot].size] = (unsigned char)mock_program_text[p_idx];
        fs_nodes[slot].size++;
        p_idx++;
    }
    fs_nodes[slot].code[fs_nodes[slot].size] = '\0';

    char *success_msg = "Success: Extracted local archive to /bin/";
    while(*success_msg) { video_memory[cursor_pos] = *success_msg++; video_memory[cursor_pos+1] = 0x0A; cursor_pos += 2; }
    for(int i=0; fs_nodes[slot].name[i] != '\0'; i++) { video_memory[cursor_pos] = fs_nodes[slot].name[i]; video_memory[cursor_pos+1] = 0x0A; cursor_pos += 2; }
    
    cursor_pos = ((cursor_pos / 160) + 1) * 160;
    *cursor_ptr = cursor_pos;
}

void fs_mkdir(const char *dir_name, char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    int slot = find_free_node();
    if (slot == -1) return;
    fs_nodes[slot].type = TYPE_DIR;
    fs_nodes[slot].parent_index = current_dir_idx;
    int i = 0; while (dir_name[i] != '\0' && i < 15) { fs_nodes[slot].name[i] = dir_name[i]; i++; }
    fs_nodes[slot].name[i] = '\0';
    *cursor_ptr = cursor_pos;
}

void fs_cd(const char *target, char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    if (mystrcmp(target, "..") == 0) {
        current_dir_idx = fs_nodes[current_dir_idx].parent_index;
        return;
    }
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type == TYPE_DIR && fs_nodes[i].parent_index == current_dir_idx) {
            if (mystrcmp(fs_nodes[i].name, target) == 0) {
                current_dir_idx = i;
                return;
            }
        }
    }
    char *err = "E: Directory not found.\n";
    while(*err) { video_memory[cursor_pos] = *err++; video_memory[cursor_pos+1] = 0x0C; cursor_pos += 2; }
    *cursor_ptr = cursor_pos;
}

void fs_ls(char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type != TYPE_FREE && fs_nodes[i].parent_index == current_dir_idx && i != current_dir_idx) {
            int j = 0;
            while (fs_nodes[i].name[j] != '\0') {
                video_memory[cursor_pos] = fs_nodes[i].name[j];
                video_memory[cursor_pos+1] = (fs_nodes[i].type == TYPE_DIR) ? 0x0B : 0x07; 
                cursor_pos += 2; j++;
            }
            if (fs_nodes[i].type == TYPE_DIR) {
                video_memory[cursor_pos] = '/'; video_memory[cursor_pos+1] = 0x0B; cursor_pos += 2;
            }
            video_memory[cursor_pos] = ' '; video_memory[cursor_pos+1] = 0x07; cursor_pos += 2;
        }
    }
    cursor_pos = ((cursor_pos / 160) + 1) * 160;
    *cursor_ptr = cursor_pos;
}

void fs_cp(const char *src, const char *dest, char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    int src_idx = -1;
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type == TYPE_FILE && fs_nodes[i].parent_index == current_dir_idx && mystrcmp(fs_nodes[i].name, src) == 0) {
            src_idx = i; break;
        }
    }
    if (src_idx == -1) {
        char *err = "E: Source file not found.\n";
        while(*err) { video_memory[cursor_pos] = *err++; video_memory[cursor_pos+1] = 0x0C; cursor_pos += 2; }
        *cursor_ptr = cursor_pos; return;
    }
    int slot = find_free_node();
    if (slot == -1) return;
    fs_nodes[slot].type = TYPE_FILE;
    fs_nodes[slot].parent_index = current_dir_idx;
    fs_nodes[slot].size = fs_nodes[src_idx].size;
    int k = 0; while(dest[k] != '\0' && k < 15) { fs_nodes[slot].name[k] = dest[k]; k++; }
    fs_nodes[slot].name[k] = '\0';
    for(int m = 0; m < fs_nodes[src_idx].size; m++) fs_nodes[slot].code[m] = fs_nodes[src_idx].code[m];
}

void fs_mv(const char *src, const char *dest) {
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type != TYPE_FREE && fs_nodes[i].parent_index == current_dir_idx && mystrcmp(fs_nodes[i].name, src) == 0) {
            int k = 0; while(dest[k] != '\0' && k < 15) { fs_nodes[i].name[k] = dest[k]; k++; }
            fs_nodes[i].name[k] = '\0';
            break;
        }
    }
}

void fs_cat(const char *filename, char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    int target_idx = -1;
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type == TYPE_FILE && fs_nodes[i].parent_index == current_dir_idx && mystrcmp(fs_nodes[i].name, filename) == 0) {
            target_idx = i; break;
        }
    }
    if (target_idx == -1) {
        char *err = "E: File not found.\n";
        while(*err) { video_memory[cursor_pos] = *err++; video_memory[cursor_pos+1] = 0x0C; cursor_pos += 2; }
        *cursor_ptr = cursor_pos; return;
    }
    for (int m = 0; m < fs_nodes[target_idx].size; m++) {
        char data_byte = (char)fs_nodes[target_idx].code[m];
        if (data_byte == '\n' || data_byte == '\r') cursor_pos = ((cursor_pos / 160) + 1) * 160;
        else { video_memory[cursor_pos] = data_byte; video_memory[cursor_pos+1] = 0x0B; cursor_pos += 2; }
    }
    cursor_pos = ((cursor_pos / 160) + 1) * 160;
    *cursor_ptr = cursor_pos;
}

int execute_bin_program(const char *full_command, char *video_memory, int *cursor_ptr) {
    char prog_name[32];
    char arg_name[32];
    int i = 0;

    while (full_command[i] != ' ' && full_command[i] != '\0' && i < 31) {
        prog_name[i] = full_command[i];
        i++;
    }
    prog_name[i] = '\0';

    int a_idx = 0;
    if (full_command[i] == ' ') {
        i++; 
        while (full_command[i] != '\0' && a_idx < 31) {
            arg_name[a_idx++] = full_command[i++];
        }
    }
    arg_name[a_idx] = '\0';

    for (int idx = 0; idx < MAX_NODES; idx++) {
        if (fs_nodes[idx].type == TYPE_FILE && fs_nodes[idx].parent_index == 1) {
            if (mystrcmp(fs_nodes[idx].name, prog_name) == 0) {
                
                if (mystrcmp(prog_name, "nano") == 0 && a_idx > 0) {
                    int file_slot = -1;
                    for(int f=0; f < MAX_NODES; f++) {
                        if (fs_nodes[f].type == TYPE_FILE && fs_nodes[f].parent_index == current_dir_idx && mystrcmp(fs_nodes[f].name, arg_name) == 0) {
                            file_slot = f; break;
                        }
                    }
                    if (file_slot == -1) {
                        file_slot = find_free_node();
                        if (file_slot != -1) {
                            fs_nodes[file_slot].type = TYPE_FILE;
                            fs_nodes[file_slot].parent_index = current_dir_idx;
                            int nc=0; while(arg_name[nc] != '\0') { fs_nodes[file_slot].name[nc] = arg_name[nc]; nc++; }
                            fs_nodes[file_slot].name[nc] = '\0';
                            fs_nodes[file_slot].size = 0;
                        }
                    }

                    if (file_slot == -1) return 0;

                    for(int m = 0; m < 80 * 25 * 2; m += 2) { video_memory[m] = ' '; video_memory[m+1] = 0x1F; } 
                    
                    char *bar = " Simple-Nano Editor v1.0   |   Press ESC key to Save and Exit Document";
                    for(int b=0; bar[b] != '\0'; b++) { video_memory[b*2] = bar[b]; video_memory[b*2+1] = 0x70; } 

                    int editor_cursor = 160; 
                    for(int m=0; m < fs_nodes[file_slot].size; m++) {
                        char cb = fs_nodes[file_slot].code[m];
                        if(cb == '\n' || cb == '\r') editor_cursor = ((editor_cursor / 160) + 1) * 160;
                        else { video_memory[editor_cursor] = cb;
                        video_memory[editor_cursor+1] = 0x1F; editor_cursor += 2; }}char edit_map[128];for(int k=0; k<128; k++) edit_map[k] = 0;edit_map[0x10]='q'; edit_map[0x11]='w'; edit_map[0x12]='e'; edit_map[0x13]='r'; edit_map[0x14]='t'; edit_map[0x15]='y'; edit_map[0x16]='u'; edit_map[0x17]='i'; edit_map[0x18]='o'; edit_map[0x19]='p';edit_map[0x1E]='a'; edit_map[0x1F]='s'; edit_map[0x20]='d'; edit_map[0x21]='f'; edit_map[0x22]='g'; edit_map[0x23]='h'; edit_map[0x24]='j'; edit_map[0x25]='k'; edit_map[0x26]='l';// FIXED: Re-added missing equals signs for bottom layout row mappings
                        edit_map[0x2C]='z'; edit_map[0x2D]='x'; edit_map[0x2E]='c'; edit_map[0x2F]='v'; edit_map[0x30]='b'; edit_map[0x31]='n'; edit_map[0x32]='m'; edit_map[0x39]=' ';edit_map[0x02]='1'; edit_map[0x03]='2'; edit_map[0x04]='3'; edit_map[0x05]='4'; edit_map[0x06]='5'; edit_map[0x07]='6'; edit_map[0x08]='7'; edit_map[0x09]='8'; edit_map[0x0A]='9'; edit_map[0x0B]='0';unsigned char last_sc = 0;fs_nodes[file_slot].size = 0;while(1) {unsigned char status = inb(0x64);if(status & 0x01) {unsigned char sc = inb(0x60);if(sc != last_sc) {last_sc = sc;if(sc < 0x80) {if(sc == 0x01) {break;}if(sc == 0x1C) {if (fs_nodes[file_slot].size < BIN_FILE_SIZE - 1) {fs_nodes[file_slot].code[fs_nodes[file_slot].size++] = '\n';editor_cursor = ((editor_cursor / 160) + 1) * 160;}}// ADDED: Backspace handling inside Nano
                        else if(sc == 0x0E) {if(fs_nodes[file_slot].size > 0) {fs_nodes[file_slot].size--;editor_cursor -= 2;video_memory[editor_cursor] = ' ';video_memory[editor_cursor+1] = 0x1F;}}else {char typed = edit_map[sc];if(typed != 0 && fs_nodes[file_slot].size < BIN_FILE_SIZE - 1) {video_memory[editor_cursor] = typed;video_memory[editor_cursor+1] = 0x1F;editor_cursor += 2;fs_nodes[file_slot].code[fs_nodes[file_slot].size++] = typed;}}}}}}for(int m = 0; m < 80 * 25 * 2; m += 2) { video_memory[m] = ' '; video_memory[m+1] = 0x07; }char *header = "--- Simple-Unix OS v1.0 [Dynamic Binary Registry Active] ---";for(int h=0; header[h] != '\0'; h++) { video_memory[h*2] = header[h]; video_memory[h*2+1] = 0x0B; }int cursor_pos = 160;char *save_msg = "[Success: Document closed and synchronized cleanly to RAM node]";while(*save_msg) { video_memory[cursor_pos] = *save_msg++; video_memory[cursor_pos+1] = 0x0A; cursor_pos += 2; }cursor_pos = ((cursor_pos / 160) + 1) * 160;*cursor_ptr = cursor_pos;return 1;}}}}return 0;}void get_current_dir_name(char *dest_str) {int idx = 0;while(fs_nodes[current_dir_idx].name[idx] != '\0') {dest_str[idx] = fs_nodes[current_dir_idx].name[idx];idx++;}dest_str[idx] = '\0';}// --- NEOFETCH STYLE ASCII SYSTEM INFO COMMAND ---
void fs_info(char *video_memory, int *cursor_ptr) {
    int cursor_pos = *cursor_ptr;
    int file_count = 0;
    int dir_count = 0;

    // Calculate system stats
    for (int i = 0; i < MAX_NODES; i++) {
        if (fs_nodes[i].type == TYPE_FILE) file_count++;
        if (fs_nodes[i].type == TYPE_DIR) dir_count++;
    }

    // Explicit 2D Array layout mapping the ascii text graphic on the left
    char *ascii_art[] = {
        "   ---/-\\---    ",
        "  /         \\   ",
        " |   O   O   |  ",
        " |     ^     |  ",
        "  \\  \\___/  /   ",
        "   ---------    "
    };

    // Explicit 2D Array layout mapping system metadata on the right
    char *sys_info[] = {
        "OS: Simple-Unix OS v1.0",
        "Kernel: i686-freestanding",
        "Uptime: 100% Offline Mode",
        "Shell: Dual Unix/English Shell",
        "Folders Active: ",
        "Files Stored: "
    };

    // Draw row-by-row layout blocks side-by-side
    for (int row = 0; row < 6; row++) {
        // 1. Draw the ASCII graphic component (Cyan text)
        char *art_ptr = ascii_art[row];
        while (*art_ptr) {
            video_memory[cursor_pos] = *art_ptr++;
            video_memory[cursor_pos+1] = 0x0B; // Light Cyan
            cursor_pos += 2;
        }

        // 2. Draw the Text metadata component (White/Gray text)
        char *info_ptr = sys_info[row];
        while (*info_ptr) {
            video_memory[cursor_pos] = *info_ptr++;
            video_memory[cursor_pos+1] = 0x0F; // Bright White
            cursor_pos += 2;
        }

        // Handle calculated number values inside row loops dynamically
        if (row == 4) helper_print_num(dir_count, video_memory, &cursor_pos);
        else if (row == 5) helper_print_num(file_count, video_memory, &cursor_pos);
        

        // Step to next text display row safely
        cursor_pos = ((cursor_pos / 160) + 1) * 160;
    }

    *cursor_ptr = cursor_pos;
}
                        
