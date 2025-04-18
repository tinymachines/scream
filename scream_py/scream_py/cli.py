"""Command-line interface for Scream."""

import sys
from scream_py.screen_manager import ScreenManager

def main():
    """Entry point for the application."""
    try:
        manager = ScreenManager()
        manager.run()
    except KeyboardInterrupt:
        print("\nExiting Scream...")
        sys.exit(0)

if __name__ == "__main__":
    main()
