"""Tests for the project_templates module."""

import unittest
from scream_py.project_templates import ProjectTemplates

class TestProjectTemplates(unittest.TestCase):
    """Test cases for ProjectTemplates class."""
    
    def setUp(self):
        """Set up test case."""
        self.templates = ProjectTemplates()
    
    def test_get_projects(self):
        """Test getting all projects."""
        projects = self.templates.get_projects()
        self.assertIsInstance(projects, list)
        self.assertGreater(len(projects), 0)
        
        # Check structure of first project
        first_project = projects[0]
        self.assertIn('name', first_project)
        self.assertIn('components', first_project)
        self.assertIsInstance(first_project['components'], list)
    
    def test_get_project_by_name(self):
        """Test getting project by name."""
        # Get an existing project
        project = self.templates.get_project_by_name('web-app')
        self.assertIsNotNone(project)
        self.assertEqual(project['name'], 'web-app')
        
        # Try to get a non-existent project
        project = self.templates.get_project_by_name('nonexistent-project')
        self.assertIsNone(project)

if __name__ == '__main__':
    unittest.main()
