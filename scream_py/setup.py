from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup(
    name="scream-cli",
    version="0.1.0",
    author="Scream Developer",
    author_email="example@example.com",
    description="A comprehensive terminal-based management tool for GNU Screen sessions",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/username/scream",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],
    python_requires=">=3.6",
    entry_points={
        "console_scripts": [
            "scream=scream_py.cli:main",
        ],
    },
    install_requires=[
        "blessed",
    ],
)