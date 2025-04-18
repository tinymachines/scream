"""Screen Manager module for Scream."""

import os
import re
import subprocess
from blessed import Terminal
from .project_templates import ProjectTemplates

class ScreenManager:
    """Manages GNU Screen sessions."""

    # Menu states
    MENU_MAIN = 0
    MENU_BROWSE = 1
    MENU_CREATE = 2
    MENU_KILL = 3
    MENU_PROJECT = 4
    MENU_HELP = 5

    def __init__(self):
        """Initialize the screen manager."""
        self.term = Terminal()
        self.screens = []
        self.selected_index = 0
        self.current_menu = self.MENU_MAIN
        self.status_message = ""
        self.status_type = "normal"
        self.projects = ProjectTemplates().get_projects()
        self.selected_project = 0
        self.new_screen_name = ""
        self.cursor_pos = 0

    def run(self):
        """Main application loop."""
        with self.term.fullscreen(), self.term.cbreak(), self.term.hidden_cursor():
            self._fetch_screens()
            while True:
                self._draw_screen()
                key = self.term.inkey()
                if not self._handle_input(key):
                    break

    def _fetch_screens(self):
        """Fetch screen sessions from 'screen -list' command."""
        self.screens = []
        try:
            output = subprocess.check_output(["screen", "-list"]).decode("utf-8")
            
            # If no screens found
            if "No Sockets found" in output:
                self._set_status("No screen sessions found", "error")
                return

            # Parse the output
            lines = output.split('\n')
            pattern = r'\s*(\d+)\.([^\s]+)\s+\(([^)]+)\)\s+\(([^)]+)\)'
            
            for line in lines:
                match = re.search(pattern, line)
                if match:
                    pid, name, timestamp, status = match.groups()
                    full_id = f"{pid}.{name}"
                    is_attached = (status == "Attached")
                    
                    self.screens.append({
                        "full_id": full_id,
                        "pid": pid,
                        "name": name,
                        "timestamp": timestamp,
                        "status": status,
                        "is_attached": is_attached
                    })
                    
        except subprocess.CalledProcessError:
            self._set_status("Failed to run screen -list", "error")
            
    def _set_status(self, message, status_type="normal"):
        """Set status message with type."""
        self.status_message = message
        self.status_type = status_type

    def _draw_screen(self):
        """Draw the current screen based on menu state."""
        print(self.term.clear())
        
        if self.current_menu == self.MENU_MAIN:
            self._draw_main_menu()
        elif self.current_menu == self.MENU_BROWSE:
            self._draw_screen_browser()
        elif self.current_menu == self.MENU_CREATE:
            self._draw_create_menu()
        elif self.current_menu == self.MENU_KILL:
            self._draw_kill_menu()
        elif self.current_menu == self.MENU_PROJECT:
            self._draw_project_menu()
        elif self.current_menu == self.MENU_HELP:
            self._draw_help_menu()
            
        # Draw status message if any
        if self.status_message:
            y = self.term.height - 2
            color = {
                "normal": self.term.normal,
                "error": self.term.red,
                "success": self.term.green
            }.get(self.status_type, self.term.normal)
            
            print(self.term.move(y, 2) + color + f"Status: {self.status_message}" + self.term.normal)
    
    def _draw_main_menu(self):
        """Draw the main menu."""
        print(self.term.move(1, 2) + self.term.green + "SCREAM - SCREEN MANAGEMENT TOOL" + self.term.normal)
        
        print(self.term.move(4, 5) + "1. Browse Screen Sessions")
        print(self.term.move(5, 5) + "2. Create New Screen")
        print(self.term.move(6, 5) + "3. Kill Screen Session")
        print(self.term.move(7, 5) + "4. Project Templates")
        print(self.term.move(8, 5) + "5. Help")
        print(self.term.move(10, 5) + "q. Quit")
        
        print(self.term.move(self.term.height - 1, 2) + self.term.cyan + 
              "Press number or highlighted letter to select" + self.term.normal)

    def _draw_screen_browser(self):
        """Draw the screen browser menu."""
        print(self.term.move(1, 2) + self.term.green + "SCREEN BROWSER" + self.term.normal)
        print(self.term.move(2, 2) + self.term.cyan + 
              "Use UP/DOWN to navigate, Enter to activate, r to refresh, q to go back" + 
              self.term.normal)
        
        # Draw header row
        print(self.term.move(4 - 1, 2) + 
              f"{'#':<5} {'PID':<10} {'NAME':<20} {'TIMESTAMP':<25} {'STATUS':<10}")
        
        # If no screens found
        if not self.screens:
            print(self.term.move(4 + 1, 2) + self.term.red + "No screen sessions found." + self.term.normal)
        else:
            # Draw screen list
            for i, screen in enumerate(self.screens):
                if i >= self.term.height - 4 - 3:
                    break  # Don't draw beyond window height
                
                # Choose color based on selection and status
                if i == self.selected_index:
                    style = self.term.black_on_blue
                elif screen["is_attached"]:
                    style = self.term.green
                else:
                    style = self.term.yellow
                
                # Print row data
                print(self.term.move(4 + i, 2) + style + 
                      f"{i+1:<5} {screen['pid']:<10} {screen['name']:<20} {screen['timestamp']:<25} " + 
                      f"{screen['status']:<10}" + self.term.normal)
        
        # Draw footer
        print(self.term.move(self.term.height - 1, 2) + self.term.cyan + 
              f"Found {len(self.screens)} screen sessions" + self.term.normal)

    def _draw_create_menu(self):
        """Draw the create screen menu."""
        print(self.term.move(1, 2) + self.term.green + "CREATE NEW SCREEN" + self.term.normal)
        print(self.term.move(2, 2) + self.term.cyan + 
              "Enter screen name and press Enter to create, ESC to cancel" + 
              self.term.normal)
        
        # Draw input field
        print(self.term.move(4, 2) + "Screen Name: " + self.new_screen_name)
        
        # Position cursor at end of input
        print(self.term.move(4, 15 + len(self.new_screen_name)))

    def _draw_kill_menu(self):
        """Draw the kill screen menu."""
        print(self.term.move(1, 2) + self.term.green + "KILL SCREEN SESSION" + self.term.normal)
        print(self.term.move(2, 2) + self.term.cyan + 
              "Use UP/DOWN to navigate, Enter to kill session, r to refresh, q to go back" + 
              self.term.normal)
        
        # Draw header row
        print(self.term.move(4 - 1, 2) + 
              f"{'#':<5} {'PID':<10} {'NAME':<20} {'TIMESTAMP':<25} {'STATUS':<10}")
        
        # If no screens found
        if not self.screens:
            print(self.term.move(4 + 1, 2) + self.term.red + "No screen sessions found." + self.term.normal)
        else:
            # Draw screen list (same as browser but with different title)
            for i, screen in enumerate(self.screens):
                if i >= self.term.height - 4 - 3:
                    break  # Don't draw beyond window height
                
                # Choose color based on selection and status
                if i == self.selected_index:
                    style = self.term.black_on_blue
                elif screen["is_attached"]:
                    style = self.term.green
                else:
                    style = self.term.yellow
                
                # Print row data
                print(self.term.move(4 + i, 2) + style + 
                      f"{i+1:<5} {screen['pid']:<10} {screen['name']:<20} {screen['timestamp']:<25} " + 
                      f"{screen['status']:<10}" + self.term.normal)
        
        # Draw footer
        print(self.term.move(self.term.height - 1, 2) + self.term.cyan + 
              f"Found {len(self.screens)} screen sessions" + self.term.normal)

    def _draw_project_menu(self):
        """Draw the project templates menu."""
        print(self.term.move(1, 2) + self.term.green + "PROJECT TEMPLATES" + self.term.normal)
        print(self.term.move(2, 2) + self.term.cyan + 
              "Use UP/DOWN to navigate, Enter to create project screens, q to go back" + 
              self.term.normal)
        
        # If no projects found
        if not self.projects:
            print(self.term.move(4 + 1, 2) + self.term.red + "No project templates found." + self.term.normal)
        else:
            # Draw project list
            for i, project in enumerate(self.projects):
                if i >= self.term.height - 4 - 3:
                    break  # Don't draw beyond window height
                
                style = self.term.black_on_blue if i == self.selected_project else self.term.normal
                print(self.term.move(4 + i, 2) + style + 
                      f"{i+1:<5} {project['name']}" + self.term.normal)
            
            # Draw selected project details
            if self.projects:
                detail_y = 4 + len(self.projects) + 2
                
                if detail_y < self.term.height - 4:
                    project = self.projects[self.selected_project]
                    print(self.term.move(detail_y, 2) + self.term.green + 
                          f"Components for {project['name']}:" + self.term.normal)
                    
                    for i, component in enumerate(project['components']):
                        if detail_y + i + 1 >= self.term.height - 3:
                            break
                        print(self.term.move(detail_y + i + 1, 4) + f"- {component}")

    def _draw_help_menu(self):
        """Draw the help menu."""
        print(self.term.move(1, 2) + self.term.green + "HELP" + self.term.normal)
        
        y = 3
        print(self.term.move(y, 2) + "Scream - A Comprehensive Screen Management Tool")
        y += 2
        
        print(self.term.move(y, 2) + "Main Menu:")
        y += 1
        print(self.term.move(y, 4) + "1 or b: Browse screen sessions")
        y += 1
        print(self.term.move(y, 4) + "2 or c: Create a new screen session")
        y += 1
        print(self.term.move(y, 4) + "3 or k: Kill a screen session")
        y += 1
        print(self.term.move(y, 4) + "4 or p: Select a project template")
        y += 1
        print(self.term.move(y, 4) + "5, h or ?: Show this help")
        y += 1
        print(self.term.move(y, 4) + "q: Quit")
        y += 2
        
        print(self.term.move(y, 2) + "Navigation:")
        y += 1
        print(self.term.move(y, 4) + "UP/DOWN: Move selection")
        y += 1
        print(self.term.move(y, 4) + "ENTER: Select/Activate")
        y += 1
        print(self.term.move(y, 4) + "ESC or q: Go back")
        y += 1
        print(self.term.move(y, 4) + "r: Refresh screen list")
        y += 2
        
        print(self.term.move(y, 2) + "Screen Sessions:")
        y += 1
        print(self.term.move(y, 4) + "- Browse mode: View and connect to existing sessions")
        y += 1
        print(self.term.move(y, 4) + "- Create mode: Start a new named screen session")
        y += 1
        print(self.term.move(y, 4) + "- Kill mode: Terminate a screen session")
        y += 1
        print(self.term.move(y, 4) + "- Project mode: Create multiple sessions from a template")
        
        print(self.term.move(self.term.height - 1, 2) + self.term.cyan + 
              "Press any key to return to main menu" + self.term.normal)

    def _handle_input(self, key):
        """Handle keyboard input based on current menu.
        
        Returns:
            bool: True to continue running, False to exit
        """
        # Main menu
        if self.current_menu == self.MENU_MAIN:
            if key.lower() == 'q':
                return False  # Exit the app
            elif key in ['1', 'b']:
                self.current_menu = self.MENU_BROWSE
                self.selected_index = 0
                self._fetch_screens()
            elif key in ['2', 'c']:
                self.current_menu = self.MENU_CREATE
                self.new_screen_name = ""
                self.cursor_pos = 0
            elif key in ['3', 'k']:
                self.current_menu = self.MENU_KILL
                self.selected_index = 0
                self._fetch_screens()
            elif key in ['4', 'p']:
                self.current_menu = self.MENU_PROJECT
                self.selected_project = 0
            elif key in ['5', 'h', '?']:
                self.current_menu = self.MENU_HELP
        
        # Browse menu
        elif self.current_menu == self.MENU_BROWSE:
            if key.name == 'KEY_UP' and self.selected_index > 0:
                self.selected_index -= 1
            elif key.name == 'KEY_DOWN' and self.selected_index < len(self.screens) - 1:
                self.selected_index += 1
            elif key == 'r':
                self._fetch_screens()
                self._set_status("Screen list refreshed", "success")
            elif key in ['q', '\n', '\r'] or key.name in ['KEY_ESCAPE', 'KEY_BACKSPACE']:
                if key in ['\n', '\r'] and self.screens:  # Enter key
                    self._activate_screen(self.selected_index)
                else:  # Back to main menu
                    self.current_menu = self.MENU_MAIN
        
        # Create menu
        elif self.current_menu == self.MENU_CREATE:
            if key in ['\n', '\r']:  # Enter key
                if self.new_screen_name:
                    self._create_screen(self.new_screen_name)
                    self.current_menu = self.MENU_MAIN
                else:
                    self._set_status("Screen name cannot be empty", "error")
            elif key.name in ['KEY_ESCAPE']:
                self.current_menu = self.MENU_MAIN
            elif key.name in ['KEY_BACKSPACE'] and self.new_screen_name:
                self.new_screen_name = self.new_screen_name[:-1]
            elif key and not key.is_sequence and len(self.new_screen_name) < 64:
                self.new_screen_name += key
        
        # Kill menu
        elif self.current_menu == self.MENU_KILL:
            if key.name == 'KEY_UP' and self.selected_index > 0:
                self.selected_index -= 1
            elif key.name == 'KEY_DOWN' and self.selected_index < len(self.screens) - 1:
                self.selected_index += 1
            elif key == 'r':
                self._fetch_screens()
                self._set_status("Screen list refreshed", "success")
            elif key in ['\n', '\r'] and self.screens:  # Enter key
                self._kill_screen(self.selected_index)
                self._fetch_screens()
            elif key in ['q'] or key.name in ['KEY_ESCAPE', 'KEY_BACKSPACE']:
                self.current_menu = self.MENU_MAIN
        
        # Project menu
        elif self.current_menu == self.MENU_PROJECT:
            if key.name == 'KEY_UP' and self.selected_project > 0:
                self.selected_project -= 1
            elif key.name == 'KEY_DOWN' and self.selected_project < len(self.projects) - 1:
                self.selected_project += 1
            elif key in ['\n', '\r'] and self.projects:  # Enter key
                self._create_project_screens(self.selected_project)
                self.current_menu = self.MENU_MAIN
            elif key in ['q'] or key.name in ['KEY_ESCAPE', 'KEY_BACKSPACE']:
                self.current_menu = self.MENU_MAIN
        
        # Help menu
        elif self.current_menu == self.MENU_HELP:
            self.current_menu = self.MENU_MAIN  # Any key returns to main menu
        
        return True  # Continue running
    
    def _activate_screen(self, index):
        """Activate a screen session."""
        if 0 <= index < len(self.screens):
            screen_id = self.screens[index]["full_id"]
            os.system(f"screen -r {screen_id}")
            self.current_menu = self.MENU_MAIN
            self._set_status("Returned from screen session", "success")
    
    def _kill_screen(self, index):
        """Kill a screen session."""
        if 0 <= index < len(self.screens):
            screen_id = self.screens[index]["full_id"]
            status = os.system(f"screen -S {screen_id} -X quit")
            if status == 0:
                self._set_status("Screen session killed successfully", "success")
            else:
                self._set_status("Failed to kill screen session", "error")
    
    def _create_screen(self, name):
        """Create a new screen session."""
        status = os.system(f"screen -dmS {name}")
        if status == 0:
            self._set_status("Screen session created successfully", "success")
        else:
            self._set_status("Failed to create screen session", "error")
    
    def _create_project_screens(self, project_index):
        """Create screen sessions for a project template."""
        if 0 <= project_index < len(self.projects):
            project = self.projects[project_index]
            for component in project["components"]:
                full_name = f"{project['name']}_{component}"
                self._create_screen(full_name)
            self._set_status("Project screens created", "success")
