"""Tests for the screen_manager module."""

import unittest
from unittest.mock import patch, MagicMock
from scream_py.screen_manager import ScreenManager

class TestScreenManager(unittest.TestCase):
    """Test cases for ScreenManager class."""
    
    @patch('scream_py.screen_manager.Terminal')
    @patch('scream_py.screen_manager.ProjectTemplates')
    def setUp(self, mock_templates, mock_terminal):
        """Set up test case."""
        # Mock terminal
        self.mock_term = mock_terminal.return_value
        self.mock_term.height = 25
        self.mock_term.width = 80
        
        # Mock project templates
        mock_templates_instance = mock_templates.return_value
        mock_templates_instance.get_projects.return_value = [
            {
                "name": "test-project",
                "components": ["test-comp1", "test-comp2"]
            }
        ]
        
        # Create ScreenManager instance
        self.manager = ScreenManager()
    
    @patch('scream_py.screen_manager.subprocess.check_output')
    def test_fetch_screens_success(self, mock_check_output):
        """Test fetching screen sessions successfully."""
        # Mock subprocess output
        mock_output = b"There are screens on:\n\t123.test\t(01/01/2023)\t(Detached)\n\t456.test2\t(01/01/2023)\t(Attached)\n2 Sockets in /run/screen/S-user.\n"
        mock_check_output.return_value = mock_output
        
        # Call method
        self.manager._fetch_screens()
        
        # Verify results
        self.assertEqual(len(self.manager.screens), 2)
        self.assertEqual(self.manager.screens[0]["pid"], "123")
        self.assertEqual(self.manager.screens[0]["name"], "test")
        self.assertEqual(self.manager.screens[0]["is_attached"], False)
        self.assertEqual(self.manager.screens[1]["pid"], "456")
        self.assertEqual(self.manager.screens[1]["name"], "test2")
        self.assertEqual(self.manager.screens[1]["is_attached"], True)
    
    @patch('scream_py.screen_manager.subprocess.check_output')
    def test_fetch_screens_no_screens(self, mock_check_output):
        """Test fetching when no screens exist."""
        # Mock subprocess output
        mock_output = b"No Sockets found in /run/screen/S-user.\n"
        mock_check_output.return_value = mock_output
        
        # Call method
        self.manager._fetch_screens()
        
        # Verify results
        self.assertEqual(len(self.manager.screens), 0)
        self.assertEqual(self.manager.status_message, "No screen sessions found")
        self.assertEqual(self.manager.status_type, "error")

if __name__ == '__main__':
    unittest.main()
