"""Project template definitions for Scream."""

class ProjectTemplates:
    """Manages project templates for screen sessions."""
    
    def __init__(self):
        """Initialize default project templates."""
        self.templates = [
            {
                "name": "web-app",
                "components": ["db", "api", "frontend"]
            },
            {
                "name": "data-science",
                "components": ["jupyter", "data-processor", "visualization"]
            },
            {
                "name": "microservices",
                "components": [
                    "auth-service",
                    "api-gateway",
                    "user-service",
                    "notification-service",
                    "logging-service"
                ]
            },
            {
                "name": "devops",
                "components": ["monitoring", "build-server", "staging", "deployment"]
            }
        ]
    
    def get_projects(self):
        """Get all project templates.
        
        Returns:
            list: List of project template dictionaries
        """
        return self.templates
    
    def get_project_by_name(self, name):
        """Get a project template by name.
        
        Args:
            name (str): Project template name
            
        Returns:
            dict: Project template dictionary or None if not found
        """
        for template in self.templates:
            if template["name"] == name:
                return template
        return None
