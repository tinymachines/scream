/**
 * scream.c - A Comprehensive Screen Session Management Tool
 * 
 * Compile with: gcc -o scream scream.c -lncurses
 * Run with: ./scream
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>

#define MAX_SCREENS 100
#define MAX_LINE_LENGTH 256
#define MAX_CMD_LENGTH 1024
#define MAX_PROJECTS 50
#define MAX_NAME_LENGTH 64

/* Color pairs */
#define COLOR_HEADER 1
#define COLOR_SELECTED 2
#define COLOR_NORMAL 3
#define COLOR_STATUS_ATTACHED 4
#define COLOR_STATUS_DETACHED 5
#define COLOR_HELP 6
#define COLOR_ERROR 7
#define COLOR_SUCCESS 8

/* Menu states */
#define MENU_MAIN 0
#define MENU_BROWSE 1
#define MENU_CREATE 2
#define MENU_KILL 3
#define MENU_PROJECT 4
#define MENU_HELP 5

/* Screen data structure */
typedef struct {
    char full_id[64];
    char pid[16];
    char name[64];
    char timestamp[32];
    char status[16];
    int is_attached;
} Screen;

/* Project template structure */
typedef struct {
    char name[MAX_NAME_LENGTH];
    char components[10][MAX_NAME_LENGTH];
    int num_components;
} Project;

/* Global variables */
Screen screens[MAX_SCREENS];
int screen_count = 0;
int selected_index = 0;
int current_menu = MENU_MAIN;
char status_message[MAX_LINE_LENGTH] = "";
int status_type = COLOR_NORMAL;
Project projects[MAX_PROJECTS];
int project_count = 0;
int selected_project = 0;
char new_screen_name[MAX_NAME_LENGTH] = "";
int cursor_pos = 0;

/* Function prototypes */
void fetch_screens();
void draw_menu(WINDOW *win);
void draw_screens(WINDOW *win);
void draw_create_menu(WINDOW *win);
void draw_kill_menu(WINDOW *win);
void draw_project_menu(WINDOW *win);
void draw_help_menu(WINDOW *win);
void activate_screen(int index);
void kill_screen(int index);
void create_screen(char *name);
void load_projects();
void create_project_screens(int project_index);
void set_status(const char *message, int type);
void handle_input(int ch);

/* Main function */
int main() {
    int ch;
    
    /* Initialize ncurses */
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0); /* Hide cursor by default */
    
    /* Initialize colors if terminal supports them */
    if (has_colors()) {
        start_color();
        init_pair(COLOR_HEADER, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_SELECTED, COLOR_BLACK, COLOR_BLUE);
        init_pair(COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_STATUS_ATTACHED, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_STATUS_DETACHED, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_HELP, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_ERROR, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_SUCCESS, COLOR_GREEN, COLOR_BLACK);
    }
    
    /* Load project templates */
    load_projects();
    
    /* Get initial list of screens */
    fetch_screens();
    
    /* Main loop */
    while (1) {
        switch (current_menu) {
            case MENU_MAIN:
                draw_menu(stdscr);
                break;
            case MENU_BROWSE:
                draw_screens(stdscr);
                break;
            case MENU_CREATE:
                draw_create_menu(stdscr);
                break;
            case MENU_KILL:
                draw_kill_menu(stdscr);
                break;
            case MENU_PROJECT:
                draw_project_menu(stdscr);
                break;
            case MENU_HELP:
                draw_help_menu(stdscr);
                break;
        }
        
        ch = getch();
        handle_input(ch);
    }
    
    /* End ncurses mode */
    endwin();
    
    return 0;
}

/* Handle keyboard input based on current menu */
void handle_input(int ch) {
    switch (current_menu) {
        case MENU_MAIN:
            switch (ch) {
                case '1':
                case 'b':
                    current_menu = MENU_BROWSE;
                    selected_index = 0;
                    fetch_screens();
                    break;
                case '2':
                case 'c':
                    current_menu = MENU_CREATE;
                    strcpy(new_screen_name, "");
                    cursor_pos = 0;
                    curs_set(1); /* Show cursor */
                    break;
                case '3':
                case 'k':
                    current_menu = MENU_KILL;
                    selected_index = 0;
                    fetch_screens();
                    break;
                case '4':
                case 'p':
                    current_menu = MENU_PROJECT;
                    selected_project = 0;
                    break;
                case '5':
                case 'h':
                case '?':
                    current_menu = MENU_HELP;
                    break;
                case 'q':
                    endwin();
                    exit(EXIT_SUCCESS);
                    break;
            }
            break;
            
        case MENU_BROWSE:
            switch (ch) {
                case KEY_UP:
                    if (selected_index > 0) {
                        selected_index--;
                    }
                    break;
                case KEY_DOWN:
                    if (selected_index < screen_count - 1) {
                        selected_index++;
                    }
                    break;
                case '\n': /* Enter key */
                    if (screen_count > 0) {
                        activate_screen(selected_index);
                    }
                    break;
                case 'r':
                    fetch_screens();
                    set_status("Screen list refreshed", COLOR_SUCCESS);
                    break;
                case 'q':
                case KEY_BACKSPACE:
                case 27: /* ESC key */
                    current_menu = MENU_MAIN;
                    break;
            }
            break;
            
        case MENU_CREATE:
            if (ch == '\n') { /* Enter key */
                if (strlen(new_screen_name) > 0) {
                    create_screen(new_screen_name);
                    current_menu = MENU_MAIN;
                    curs_set(0); /* Hide cursor */
                } else {
                    set_status("Screen name cannot be empty", COLOR_ERROR);
                }
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (cursor_pos > 0) {
                    cursor_pos--;
                    new_screen_name[cursor_pos] = '\0';
                }
            } else if (ch == 27) { /* ESC key */
                current_menu = MENU_MAIN;
                curs_set(0); /* Hide cursor */
            } else if (isprint(ch) && cursor_pos < MAX_NAME_LENGTH - 1) {
                new_screen_name[cursor_pos++] = ch;
                new_screen_name[cursor_pos] = '\0';
            }
            break;
            
        case MENU_KILL:
            switch (ch) {
                case KEY_UP:
                    if (selected_index > 0) {
                        selected_index--;
                    }
                    break;
                case KEY_DOWN:
                    if (selected_index < screen_count - 1) {
                        selected_index++;
                    }
                    break;
                case '\n': /* Enter key */
                    if (screen_count > 0) {
                        kill_screen(selected_index);
                        fetch_screens();
                    }
                    break;
                case 'r':
                    fetch_screens();
                    set_status("Screen list refreshed", COLOR_SUCCESS);
                    break;
                case 'q':
                case KEY_BACKSPACE:
                case 27: /* ESC key */
                    current_menu = MENU_MAIN;
                    break;
            }
            break;
            
        case MENU_PROJECT:
            switch (ch) {
                case KEY_UP:
                    if (selected_project > 0) {
                        selected_project--;
                    }
                    break;
                case KEY_DOWN:
                    if (selected_project < project_count - 1) {
                        selected_project++;
                    }
                    break;
                case '\n': /* Enter key */
                    if (project_count > 0) {
                        create_project_screens(selected_project);
                        current_menu = MENU_MAIN;
                    }
                    break;
                case 'q':
                case KEY_BACKSPACE:
                case 27: /* ESC key */
                    current_menu = MENU_MAIN;
                    break;
            }
            break;
            
        case MENU_HELP:
            switch (ch) {
                case 'q':
                case KEY_BACKSPACE:
                case 27: /* ESC key */
                case '\n': /* Enter key */
                    current_menu = MENU_MAIN;
                    break;
            }
            break;
    }
}

/* Set status message with color */
void set_status(const char *message, int type) {
    strncpy(status_message, message, MAX_LINE_LENGTH - 1);
    status_message[MAX_LINE_LENGTH - 1] = '\0';
    status_type = type;
}

/* Main menu display */
void draw_menu(WINDOW *win) {
    int width, height;
    getmaxyx(win, height, width);
    
    wclear(win);
    
    /* Draw header */
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(1, 2, "SCREAM - SCREEN MANAGEMENT TOOL");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    /* Draw menu options */
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(4, 5, "1. Browse Screen Sessions");
    mvprintw(5, 5, "2. Create New Screen");
    mvprintw(6, 5, "3. Kill Screen Session");
    mvprintw(7, 5, "4. Project Templates");
    mvprintw(8, 5, "5. Help");
    mvprintw(10, 5, "q. Quit");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    /* Draw status message if any */
    if (strlen(status_message) > 0) {
        attron(COLOR_PAIR(status_type));
        mvprintw(height - 2, 2, "Status: %s", status_message);
        attroff(COLOR_PAIR(status_type));
    }
    
    /* Draw footer */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(height - 1, 2, "Press number or highlighted letter to select");
    attroff(COLOR_PAIR(COLOR_HELP));
    
    refresh();
}

/* Fetch screen sessions from screen -list command */
void fetch_screens() {
    FILE *fp;
    char cmd_output[MAX_SCREENS * MAX_LINE_LENGTH] = "";
    char line[MAX_LINE_LENGTH];
    
    /* Clear existing screens */
    screen_count = 0;
    
    /* Execute screen -list and capture output */
    fp = popen("screen -list", "r");
    if (fp == NULL) {
        set_status("Failed to run screen -list", COLOR_ERROR);
        return;
    }
    
    /* Read all output */
    while (fgets(line, sizeof(line), fp) != NULL) {
        strcat(cmd_output, line);
    }
    pclose(fp);
    
    /* For debugging - sample data if no screens found */
    if (strstr(cmd_output, "No Sockets found") != NULL) {
        set_status("No screen sessions found", COLOR_ERROR);
        return;
    }
    
    /* Process each line */
    char *line_start = cmd_output;
    char *line_end;
    
    while ((line_end = strchr(line_start, '\n')) != NULL) {
        /* Extract the current line */
        *line_end = '\0';  /* Temporarily replace newline with null terminator */
        
        /* Check if this is a screen entry (starts with tab or space and contains a dot) */
        if ((line_start[0] == '\t' || line_start[0] == ' ') && 
            strchr(line_start, '.') != NULL && 
            !strstr(line_start, "Sockets")) {
            
            /* Skip leading whitespace */
            while (*line_start == '\t' || *line_start == ' ')
                line_start++;
            
            /* Find the end of the session ID (before first tab) */
            char *id_end = strchr(line_start, '\t');
            if (id_end) {
                /* Extract the full ID */
                int id_len = id_end - line_start;
                strncpy(screens[screen_count].full_id, line_start, id_len);
                screens[screen_count].full_id[id_len] = '\0';
                
                /* Split into PID and NAME */
                char *dot = strchr(screens[screen_count].full_id, '.');
                if (dot) {
                    int pid_len = dot - screens[screen_count].full_id;
                    strncpy(screens[screen_count].pid, screens[screen_count].full_id, pid_len);
                    screens[screen_count].pid[pid_len] = '\0';
                    strcpy(screens[screen_count].name, dot + 1);
                } else {
                    strcpy(screens[screen_count].pid, screens[screen_count].full_id);
                    strcpy(screens[screen_count].name, "unknown");
                }
                
                /* Extract timestamp between first set of parentheses */
                char *time_start = strchr(id_end, '(');
                if (time_start) {
                    time_start++; /* Skip the opening parenthesis */
                    char *time_end = strchr(time_start, ')');
                    if (time_end) {
                        int time_len = time_end - time_start;
                        strncpy(screens[screen_count].timestamp, time_start, time_len);
                        screens[screen_count].timestamp[time_len] = '\0';
                        
                        /* Extract status from second set of parentheses */
                        char *status_start = strchr(time_end, '(');
                        if (status_start) {
                            status_start++; /* Skip the opening parenthesis */
                            char *status_end = strchr(status_start, ')');
                            if (status_end) {
                                int status_len = status_end - status_start;
                                strncpy(screens[screen_count].status, status_start, status_len);
                                screens[screen_count].status[status_len] = '\0';
                                screens[screen_count].is_attached = (strcmp(screens[screen_count].status, "Attached") == 0);
                            }
                        }
                    }
                }
                
                screen_count++;
                if (screen_count >= MAX_SCREENS) {
                    break;
                }
            }
        }
        
        /* Restore newline and move to next line */
        *line_end = '\n';
        line_start = line_end + 1;
    }
}

/* Screen browser display */
void draw_screens(WINDOW *win) {
    int i;
    int start_y = 4;
    int width, height;
    
    getmaxyx(win, height, width);
    
    wclear(win);
    
    /* Draw header */
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(1, 2, "SCREEN BROWSER");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    /* Draw help text */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(2, 2, "Use UP/DOWN to navigate, Enter to activate, r to refresh, q to go back");
    attroff(COLOR_PAIR(COLOR_HELP));
    
    /* Draw header row */
    mvprintw(start_y - 1, 2, "%-5s %-10s %-20s %-25s %-10s", "#", "PID", "NAME", "TIMESTAMP", "STATUS");
    
    /* If no screens found */
    if (screen_count == 0) {
        attron(COLOR_PAIR(COLOR_ERROR));
        mvprintw(start_y + 1, 2, "No screen sessions found.");
        attroff(COLOR_PAIR(COLOR_ERROR));
    } else {
        /* Draw screen list */
        for (i = 0; i < screen_count; i++) {
            if (i >= height - start_y - 3) {
                break; /* Don't draw beyond window height */
            }
            
            if (i == selected_index) {
                attron(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_NORMAL));
            }
            
            /* Print row data */
            mvprintw(start_y + i, 2, "%-5d %-10s %-20s %-25s ", 
                    i + 1, 
                    screens[i].pid, 
                    screens[i].name,
                    screens[i].timestamp);
            
            /* Print status with different color */
            if (screens[i].is_attached) {
                if (i == selected_index) {
                    attron(COLOR_PAIR(COLOR_SELECTED));
                } else {
                    attron(COLOR_PAIR(COLOR_STATUS_ATTACHED));
                }
                printw("%-10s", "Attached");
            } else {
                if (i == selected_index) {
                    attron(COLOR_PAIR(COLOR_SELECTED));
                } else {
                    attron(COLOR_PAIR(COLOR_STATUS_DETACHED));
                }
                printw("%-10s", "Detached");
            }
            
            if (i == selected_index) {
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else if (screens[i].is_attached) {
                attroff(COLOR_PAIR(COLOR_STATUS_ATTACHED));
            } else {
                attroff(COLOR_PAIR(COLOR_STATUS_DETACHED));
            }
        }
    }
    
    /* Draw status message if any */
    if (strlen(status_message) > 0) {
        attron(COLOR_PAIR(status_type));
        mvprintw(height - 2, 2, "Status: %s", status_message);
        attroff(COLOR_PAIR(status_type));
    }
    
    /* Draw footer */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(height - 1, 2, "Found %d screen sessions", screen_count);
    attroff(COLOR_PAIR(COLOR_HELP));
    
    refresh();
}

/* Create screen menu display */
void draw_create_menu(WINDOW *win) {
    int width, height;
    getmaxyx(win, height, width);
    
    wclear(win);
    
    /* Draw header */
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(1, 2, "CREATE NEW SCREEN");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    /* Draw help text */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(2, 2, "Enter screen name and press Enter to create, ESC to cancel");
    attroff(COLOR_PAIR(COLOR_HELP));
    
    /* Draw input field */
    attron(COLOR_PAIR(COLOR_NORMAL));
    mvprintw(4, 2, "Screen Name: ");
    mvprintw(4, 15, "%s", new_screen_name);
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    /* Position cursor at end of input */
    move(4, 15 + cursor_pos);
    
    /* Draw status message if any */
    if (strlen(status_message) > 0) {
        attron(COLOR_PAIR(status_type));
        mvprintw(height - 2, 2, "Status: %s", status_message);
        attroff(COLOR_PAIR(status_type));
    }
    
    refresh();
}

/* Kill screen menu display */
void draw_kill_menu(WINDOW *win) {
    int i;
    int start_y = 4;
    int width, height;
    
    getmaxyx(win, height, width);
    
    wclear(win);
    
    /* Draw header */
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(1, 2, "KILL SCREEN SESSION");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    /* Draw help text */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(2, 2, "Use UP/DOWN to navigate, Enter to kill session, r to refresh, q to go back");
    attroff(COLOR_PAIR(COLOR_HELP));
    
    /* Draw header row */
    mvprintw(start_y - 1, 2, "%-5s %-10s %-20s %-25s %-10s", "#", "PID", "NAME", "TIMESTAMP", "STATUS");
    
    /* If no screens found */
    if (screen_count == 0) {
        attron(COLOR_PAIR(COLOR_ERROR));
        mvprintw(start_y + 1, 2, "No screen sessions found.");
        attroff(COLOR_PAIR(COLOR_ERROR));
    } else {
        /* Draw screen list */
        for (i = 0; i < screen_count; i++) {
            if (i >= height - start_y - 3) {
                break; /* Don't draw beyond window height */
            }
            
            if (i == selected_index) {
                attron(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_NORMAL));
            }
            
            mvprintw(start_y + i, 2, "%-5d %-10s %-20s %-25s ", 
                    i + 1, 
                    screens[i].pid, 
                    screens[i].name,
                    screens[i].timestamp);
            
            /* Print status with different color */
            if (screens[i].is_attached) {
                if (i == selected_index) {
                    attron(COLOR_PAIR(COLOR_SELECTED));
                } else {
                    attron(COLOR_PAIR(COLOR_STATUS_ATTACHED));
                }
                printw("%-10s", "Attached");
            } else {
                if (i == selected_index) {
                    attron(COLOR_PAIR(COLOR_SELECTED));
                } else {
                    attron(COLOR_PAIR(COLOR_STATUS_DETACHED));
                }
                printw("%-10s", "Detached");
            }
            
            if (i == selected_index) {
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else if (screens[i].is_attached) {
                attroff(COLOR_PAIR(COLOR_STATUS_ATTACHED));
            } else {
                attroff(COLOR_PAIR(COLOR_STATUS_DETACHED));
            }
        }
    }
    
    /* Draw status message if any */
    if (strlen(status_message) > 0) {
        attron(COLOR_PAIR(status_type));
        mvprintw(height - 2, 2, "Status: %s", status_message);
        attroff(COLOR_PAIR(status_type));
    }
    
    /* Draw footer */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(height - 1, 2, "Found %d screen sessions", screen_count);
    attroff(COLOR_PAIR(COLOR_HELP));
    
    refresh();
}

/* Project templates menu display */
void draw_project_menu(WINDOW *win) {
    int i;
    int start_y = 4;
    int width, height;
    
    getmaxyx(win, height, width);
    
    wclear(win);
    
    /* Draw header */
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(1, 2, "PROJECT TEMPLATES");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    /* Draw help text */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(2, 2, "Use UP/DOWN to navigate, Enter to create project screens, q to go back");
    attroff(COLOR_PAIR(COLOR_HELP));
    
    /* If no projects found */
    if (project_count == 0) {
        attron(COLOR_PAIR(COLOR_ERROR));
        mvprintw(start_y + 1, 2, "No project templates found.");
        attroff(COLOR_PAIR(COLOR_ERROR));
    } else {
        /* Draw project list */
        for (i = 0; i < project_count; i++) {
            if (i >= height - start_y - 3) {
                break; /* Don't draw beyond window height */
            }
            
            if (i == selected_project) {
                attron(COLOR_PAIR(COLOR_SELECTED));
                mvprintw(start_y + i, 2, "%-5d %s", i + 1, projects[i].name);
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_NORMAL));
                mvprintw(start_y + i, 2, "%-5d %s", i + 1, projects[i].name);
                attroff(COLOR_PAIR(COLOR_NORMAL));
            }
        }
        
        /* Draw selected project details */
        if (project_count > 0) {
            int detail_y = start_y + project_count + 2;
            
            if (detail_y < height - 4) {
                attron(COLOR_PAIR(COLOR_HEADER));
                mvprintw(detail_y, 2, "Components for %s:", projects[selected_project].name);
                attroff(COLOR_PAIR(COLOR_HEADER));
                
                for (i = 0; i < projects[selected_project].num_components; i++) {
                    if (detail_y + i + 1 >= height - 3) {
                        break;
                    }
                    
                    attron(COLOR_PAIR(COLOR_NORMAL));
                    mvprintw(detail_y + i + 1, 4, "- %s", projects[selected_project].components[i]);
                    attroff(COLOR_PAIR(COLOR_NORMAL));
                }
            }
        }
    }
    
    /* Draw status message if any */
    if (strlen(status_message) > 0) {
        attron(COLOR_PAIR(status_type));
        mvprintw(height - 2, 2, "Status: %s", status_message);
        attroff(COLOR_PAIR(status_type));
    }
    
   refresh();
}

/* Help menu display */
void draw_help_menu(WINDOW *win) {
    int width, height;
    getmaxyx(win, height, width);
    
    wclear(win);
    
    /* Draw header */
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(1, 2, "HELP");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    /* Draw help content */
    attron(COLOR_PAIR(COLOR_NORMAL));
    int y = 3;
    mvprintw(y++, 2, "Scream - A Comprehensive Screen Management Tool");
    y++;
    mvprintw(y++, 2, "Main Menu:");
    mvprintw(y++, 4, "1 or b: Browse screen sessions");
    mvprintw(y++, 4, "2 or c: Create a new screen session");
    mvprintw(y++, 4, "3 or k: Kill a screen session");
    mvprintw(y++, 4, "4 or p: Select a project template");
    mvprintw(y++, 4, "5, h or ?: Show this help");
    mvprintw(y++, 4, "q: Quit");
    y++;
    mvprintw(y++, 2, "Navigation:");
    mvprintw(y++, 4, "UP/DOWN: Move selection");
    mvprintw(y++, 4, "ENTER: Select/Activate");
    mvprintw(y++, 4, "ESC or q: Go back");
    mvprintw(y++, 4, "r: Refresh screen list");
    y++;
    mvprintw(y++, 2, "Screen Sessions:");
    mvprintw(y++, 4, "- Browse mode: View and connect to existing sessions");
    mvprintw(y++, 4, "- Create mode: Start a new named screen session");
    mvprintw(y++, 4, "- Kill mode: Terminate a screen session");
    mvprintw(y++, 4, "- Project mode: Create multiple sessions from a template");
    attroff(COLOR_PAIR(COLOR_NORMAL));
    
    /* Draw footer */
    attron(COLOR_PAIR(COLOR_HELP));
    mvprintw(height - 1, 2, "Press any key to return to main menu");
    attroff(COLOR_PAIR(COLOR_HELP));
    
    refresh();
}

/* Activate a screen session */
void activate_screen(int index) {
    char command[MAX_CMD_LENGTH];
    
    endwin(); /* End ncurses mode */
    
    printf("\nActivating screen: %s\n", screens[index].full_id);
    
    /* Build the command to attach to the screen */
    sprintf(command, "screen -r %s", screens[index].full_id);
    
    /* Execute the command */
    system(command);
    
    /* Restart ncurses mode */
    refresh();
    
    /* Return to main menu */
    current_menu = MENU_MAIN;
    set_status("Returned from screen session", COLOR_SUCCESS);
}

/* Kill a screen session */
void kill_screen(int index) {
    char command[MAX_CMD_LENGTH];
    
    /* Build the command to kill the screen */
    sprintf(command, "screen -S %s -X quit", screens[index].full_id);
    
    /* Execute the command */
    if (system(command) == 0) {
        set_status("Screen session killed successfully", COLOR_SUCCESS);
    } else {
        set_status("Failed to kill screen session", COLOR_ERROR);
    }
}

/* Create a new screen session */
void create_screen(char *name) {
    char command[MAX_CMD_LENGTH];
    
    /* Build the command to create a new screen */
    sprintf(command, "screen -dmS %s", name);
    
    /* Execute the command */
    if (system(command) == 0) {
        set_status("Screen session created successfully", COLOR_SUCCESS);
    } else {
        set_status("Failed to create screen session", COLOR_ERROR);
    }
}

/* Load project templates from configuration */
void load_projects() {
    /* Initialize with some default projects */
    project_count = 0;
    
    /* Web application project */
    strcpy(projects[project_count].name, "web-app");
    strcpy(projects[project_count].components[0], "db");
    strcpy(projects[project_count].components[1], "api");
    strcpy(projects[project_count].components[2], "frontend");
    projects[project_count].num_components = 3;
    project_count++;
    
    /* Data science project */
    strcpy(projects[project_count].name, "data-science");
    strcpy(projects[project_count].components[0], "jupyter");
    strcpy(projects[project_count].components[1], "data-processor");
    strcpy(projects[project_count].components[2], "visualization");
    projects[project_count].num_components = 3;
    project_count++;
    
    /* Microservices project */
    strcpy(projects[project_count].name, "microservices");
    strcpy(projects[project_count].components[0], "auth-service");
    strcpy(projects[project_count].components[1], "api-gateway");
    strcpy(projects[project_count].components[2], "user-service");
    strcpy(projects[project_count].components[3], "notification-service");
    strcpy(projects[project_count].components[4], "logging-service");
    projects[project_count].num_components = 5;
    project_count++;
    
    /* Dev-ops project */
    strcpy(projects[project_count].name, "devops");
    strcpy(projects[project_count].components[0], "monitoring");
    strcpy(projects[project_count].components[1], "build-server");
    strcpy(projects[project_count].components[2], "staging");
    strcpy(projects[project_count].components[3], "deployment");
    projects[project_count].num_components = 4;
    project_count++;
    
    /* TODO: Load more projects from a config file */
}

/* Create screen sessions for a project template */
void create_project_screens(int project_index) {
    int i;
    char full_name[MAX_NAME_LENGTH * 2];
    
    if (project_index < 0 || project_index >= project_count) {
        set_status("Invalid project index", COLOR_ERROR);
        return;
    }
    
    for (i = 0; i < projects[project_index].num_components; i++) {
        sprintf(full_name, "%s_%s", projects[project_index].name, 
                projects[project_index].components[i]);
        
        create_screen(full_name);
    }
    
    set_status("Project screens created", COLOR_SUCCESS);
}
