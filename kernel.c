extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char data);
extern void fs_info(char *vid, int *cur);

char saved_package[40];

int strncmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

extern int mystrcmp(const char *s1, const char *s2);
extern void download_package_from_debian(const char *pkg, char *vid, int *cur);
extern void init_filesystem();
extern void fs_mkdir(const char *name, char *vid, int *cur);
extern void fs_cd(const char *name, char *vid, int *cur);
extern void fs_ls(char *vid, int *cur);
extern void fs_cp(const char *s, const char *d, char *vid, int *cur);
extern void fs_mv(const char *s, const char *d);
extern void fs_cat(const char *name, char *vid, int *cur);
extern void get_current_dir_name(char *dest);
extern int execute_bin_program(const char *name, char *vid, int *cur);
extern void fs_info(char *vid, int *cur);

void kernel_main() {
    init_filesystem();
    char *video_memory = (char*) 0xB8000;
    
    for(int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i+1] = 0x07;
    }
    
    char *header = "--- Simple-Unix OS v1.0 [Dynamic Binary Registry Active] ---";
    int idx = 0;
    while(header[idx] != '\0') {
        video_memory[idx * 2] = header[idx];
        video_memory[idx * 2 + 1] = 0x0B;
        idx++;
    }
    

    
    int cursor_pos = 160;
    char command_buffer[80];
    int cmd_idx = 0;
    int is_asking_password = 0;

    char keyboard_map[128];
    for(int i = 0; i < 128; i++) keyboard_map[i] = 0;

    keyboard_map[0x01] = 27;  
    keyboard_map[0x02] = '1'; keyboard_map[0x03] = '2'; keyboard_map[0x04] = '3';
    keyboard_map[0x05] = '4'; keyboard_map[0x06] = '5'; keyboard_map[0x07] = '6';
    keyboard_map[0x08] = '7'; keyboard_map[0x09] = '8'; keyboard_map[0x0A] = '9';
    keyboard_map[0x0B] = '0'; keyboard_map[0x0C] = '-'; keyboard_map[0x0D] = '=';
    keyboard_map[0x0E] = '\b'; 

    keyboard_map[0x0F] = '\t'; 
    keyboard_map[0x10] = 'q'; keyboard_map[0x11] = 'w'; keyboard_map[0x12] = 'e';
    keyboard_map[0x13] = 'r'; keyboard_map[0x14] = 't'; keyboard_map[0x15] = 'y';
    keyboard_map[0x16] = 'u'; keyboard_map[0x17] = 'i'; keyboard_map[0x18] = 'o';
    keyboard_map[0x19] = 'p'; keyboard_map[0x1A] = '['; keyboard_map[0x1B] = ']';
    keyboard_map[0x1C] = '\n'; 

    keyboard_map[0x1E] = 'a'; keyboard_map[0x1F] = 's'; keyboard_map[0x20] = 'd';
    keyboard_map[0x21] = 'f'; keyboard_map[0x22] = 'g'; keyboard_map[0x23] = 'h';
    keyboard_map[0x24] = 'j'; keyboard_map[0x25] = 'k'; keyboard_map[0x26] = 'l';
    keyboard_map[0x27] = ';'; keyboard_map[0x28] = '\''; keyboard_map[0x29] = '`';

    keyboard_map[0x2B] = '\\';
    keyboard_map[0x2C] = 'z';  keyboard_map[0x2D] = 'x'; keyboard_map[0x2E] = 'c';
    keyboard_map[0x2F] = 'v';  keyboard_map[0x30] = 'b'; keyboard_map[0x31] = 'n';
    keyboard_map[0x32] = 'm';  keyboard_map[0x33] = ','; keyboard_map[0x34] = '.';
    keyboard_map[0x35] = '/';

    keyboard_map[0x37] = '*'; 
    keyboard_map[0x39] = ' '; 
    keyboard_map[0x4A] = '-'; 
    keyboard_map[0x4E] = '+'; 

    keyboard_map[0x47] = '7'; keyboard_map[0x48] = '8'; keyboard_map[0x49] = '9';
    keyboard_map[0x4B] = '4'; keyboard_map[0x4C] = '5'; keyboard_map[0x4D] = '6';
    keyboard_map[0x4F] = '1'; keyboard_map[0x50] = '2'; keyboard_map[0x51] = '3';
    keyboard_map[0x52] = '0'; keyboard_map[0x53] = '.';

    char path_buf[16];
    get_current_dir_name(path_buf);
    int p_init = 0;
    while(path_buf[p_init] != '\0') {
        video_memory[cursor_pos] = path_buf[p_init];
        video_memory[cursor_pos+1] = 0x0A; 
        cursor_pos += 2; p_init++;
    }
    video_memory[cursor_pos] = '>';
    video_memory[cursor_pos+1] = 0x0E; 
    video_memory[cursor_pos+2] = ' ';
    cursor_pos += 4; 

    unsigned char last_scancode = 0;

    while(1) {
        unsigned char status = inb(0x64);
        if(status & 0x01) {
            unsigned char scancode = inb(0x60);
            if (scancode != last_scancode) {
                last_scancode = scancode;
                if (scancode < 0x80) {
                    if (scancode == 0x1C) {
                        command_buffer[cmd_idx] = '\0';
                        cursor_pos = ((cursor_pos / 160) + 1) * 160;
                        
                        if (is_asking_password) {
                            if (command_buffer[0] == '1' && command_buffer[1] == '2' && command_buffer[2] == '3' && command_buffer[3] == '4' && command_buffer[4] == '\0') {
                                download_package_from_debian(saved_package, video_memory, &cursor_pos);
                            } else {
                                char *f = "vnc-sudo: Incorrect password.";
                                while(*f) { video_memory[cursor_pos] = *f++; video_memory[cursor_pos+1] = 0x0C; cursor_pos += 2; }
                            }
                            is_asking_password = 0;
                        } else {
                            if (strncmp(command_buffer, "install ", 8) == 0) {
                                int pi = 0; while(command_buffer[8+pi] != '\0' && pi < 39) { saved_package[pi] = command_buffer[8+pi]; pi++; }
                                saved_package[pi] = '\0';
                                char *p = "[sudo] password: ";
                                while(*p) { video_memory[cursor_pos] = *p++; video_memory[cursor_pos+1] = 0x0E; cursor_pos += 2; }
                                is_asking_password = 1;
                            }
                            else if (strncmp(command_buffer, "clear", 5) == 0 || strncmp(command_buffer, "wipe-screen", 11) == 0) {
                                for(int i = 160; i < 80 * 25 * 2; i += 2) { video_memory[i] = ' '; video_memory[i+1] = 0x07; }
                                cursor_pos = 160;
                            }
                            else if (strncmp(command_buffer, "mkdir ", 6) == 0 || strncmp(command_buffer, "create-folder ", 14) == 0) {
                                int offset = (command_buffer[0] == 'm') ? 6 : 14;
                                fs_mkdir(&command_buffer[offset], video_memory, &cursor_pos);
                            }
                            else if (strncmp(command_buffer, "cd ", 3) == 0 || strncmp(command_buffer, "go-to-folder ", 13) == 0) {
                                int offset = (command_buffer[0] == 'c') ? 3 : 13;
                                fs_cd(&command_buffer[offset], video_memory, &cursor_pos);
                            }
                            else if (strncmp(command_buffer, "ls", 2) == 0 || strncmp(command_buffer, "show-everything", 15) == 0) {
                                fs_ls(video_memory, &cursor_pos);
                            }
                                                        // WHAT TO WRITE: Add this branch inside your command parser stack
                            else if (strncmp(command_buffer, "neofetch", 8) == 0 || strncmp(command_buffer, "info", 4) == 0) {
                                fs_info(video_memory, &cursor_pos);
                            }

                            else if (strncmp(command_buffer, "cat ", 4) == 0 || strncmp(command_buffer, "read-program ", 13) == 0) {
                                int offset = (command_buffer[0] == 'c') ? 4 : 13;
                                fs_cat(&command_buffer[offset], video_memory, &cursor_pos);
                            }
                            else if (strncmp(command_buffer, "mv ", 3) == 0 || strncmp(command_buffer, "rename-file ", 12) == 0) {
                                int offset = (command_buffer[0] == 'm') ? 3 : 12;
                                int sp = offset; while (command_buffer[sp] != ' ' && command_buffer[sp] != '\0') sp++;
                                if (command_buffer[sp] == ' ') {
                                    command_buffer[sp] = '\0';
                                    fs_mv(&command_buffer[offset], &command_buffer[sp + 1]);
                                }
                            }
                            else if (strncmp(command_buffer, "cp ", 3) == 0 || strncmp(command_buffer, "copy-file ", 10) == 0) {
                                int offset = (command_buffer[0] == 'c') ? 3 : 10;
                                int sp = offset; while (command_buffer[sp] != ' ' && command_buffer[sp] != '\0') sp++;
                                if (command_buffer[sp] == ' ') {
                                    command_buffer[sp] = '\0';
                                    fs_cp(&command_buffer[offset], &command_buffer[sp + 1], video_memory, &cursor_pos);
                                }
                            }
                            else if (cmd_idx > 0) {
                                int ran_program = execute_bin_program(command_buffer, video_memory, &cursor_pos);
                                if (!ran_program) {
                                    char *e = "E: Command unknown.";
                                    while(*e) { video_memory[cursor_pos] = *e++; video_memory[cursor_pos+1] = 0x0C; cursor_pos += 2; }
                                }
                            }
                        }

                        if (!is_asking_password) {
                            cursor_pos = ((cursor_pos / 160) + 1) * 160;
                            cmd_idx = 0;
                            get_current_dir_name(path_buf);
                            int p = 0; while(path_buf[p] != '\0') { video_memory[cursor_pos] = path_buf[p]; video_memory[cursor_pos+1] = 0x0A; cursor_pos += 2; p++; }
                            video_memory[cursor_pos] = '>'; video_memory[cursor_pos+1] = 0x0E; video_memory[cursor_pos+2] = ' '; cursor_pos += 4;
                        } else {
                            cmd_idx = 0;
                        }
                    }
                    else if (scancode == 0x0E) {
                        if (cmd_idx > 0) { cmd_idx--; cursor_pos -= 2; video_memory[cursor_pos] = ' '; video_memory[cursor_pos+1] = 0x07; }
                    }
                    else {
                        char typed_char = keyboard_map[scancode];
                        if (typed_char != 0 && cmd_idx < 79) {
                        if (is_asking_password) video_memory[cursor_pos] = '*';
                        else video_memory[cursor_pos] = typed_char;
                        video_memory[cursor_pos+1] = 0x0F;
                        cursor_pos += 2;
                        command_buffer[cmd_idx++] = typed_char;
                        
                        }
                        }
                        }
                        }
                        }
                        }
                        }
