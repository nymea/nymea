"""Sphinx configuration for the nymea project documentation."""
from __future__ import annotations

import os
import sys
from datetime import datetime

# -- Path setup --------------------------------------------------------------

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LIBNYMEA_SRC = os.path.join(ROOT_DIR, "libnymea")
if os.path.isdir(LIBNYMEA_SRC) and LIBNYMEA_SRC not in sys.path:
    sys.path.insert(0, LIBNYMEA_SRC)

# -- Project information -----------------------------------------------------

project = "nymea"
author = "The nymea Project"
current_year = datetime.utcnow().year
copyright = f"{current_year}, {author}"
release = os.environ.get("NYMEA_RELEASE", "dev")
version = release

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx.ext.todo",
    "sphinx.ext.autosummary",
    "sphinx.ext.viewcode",
]

autosummary_generate = True
autodoc_default_options = {
    "members": True,
    "undoc-members": False,
    "show-inheritance": True,
}

templates_path = ["_templates"]
exclude_patterns = ["_build"]

# -- Options for HTML output -------------------------------------------------

html_theme = "alabaster"
html_static_path = ["_static"]
html_title = "nymea Documentation"
html_logo = "_static/nymea-logo.svg"
html_theme_options = {
    "description": "Official documentation for the nymea automation platform.",
}

def setup(app):
    app.add_css_file("custom.css")
