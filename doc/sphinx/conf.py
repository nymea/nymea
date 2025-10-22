"""Sphinx configuration for the nymea developer documentation."""
from __future__ import annotations

import os
from datetime import datetime
from pathlib import Path

# -- Paths -----------------------------------------------------------------

PROJECT_ROOT = Path(__file__).resolve().parent
REPO_ROOT = PROJECT_ROOT.parent

# -- General configuration -------------------------------------------------

project = "nymea Developer Documentation"
copyright = f"{datetime.now():%Y} nymea GmbH"
author = "nymea GmbH"

# Allow overriding the version from the environment in CI builds.
def _resolve_version() -> str:
    env_version = os.environ.get("NYMEA_VERSION")
    if env_version:
        return env_version

    version_file = REPO_ROOT / "version.h.in"
    if version_file.exists():
        for line in version_file.read_text(encoding="utf-8").splitlines():
            if "NYMEA_VERSION_STRING" in line and "define" in line:
                parts = line.split()
                if parts:
                    return parts[-1].strip('"')
    return "dev"

release = version = _resolve_version()

extensions = [
    "breathe",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.ifconfig",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
]

autosectionlabel_prefix_document = True

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Internationalisation --------------------------------------------------

language = "en"

# -- Syntax highlighting ---------------------------------------------------

primary_domain = "cpp"
highlight_language = "cpp"

# -- Breathe configuration -------------------------------------------------

DOXYGEN_XML_DIR = PROJECT_ROOT / "_doxygen" / "xml"

breathe_projects = {"libnymea": str(DOXYGEN_XML_DIR)}
breathe_default_project = "libnymea"

# Doxygen exposes Qt's ``Q_PROPERTY`` entries as ``kind="property"`` member
# definitions. Older Breathe releases do not map this kind to a Sphinx
# directive which leads to ``KeyError: 'property'`` during the build. Map the
# property kind to the regular variable handler so the documentation renders
# instead of crashing. This is a no-op with newer Breathe versions where the
# mapping already exists.
try:  # pragma: no cover - optional dependency at build time only
    from breathe.renderer.sphinxrenderer import DomainDirectiveFactory
except Exception:  # ImportError on readthedocs / when breathe is unavailable
    DomainDirectiveFactory = None

if DomainDirectiveFactory is not None:
    def _ensure_property_mapping(attr: str) -> tuple | None:
        mapping = getattr(DomainDirectiveFactory, attr, None)
        if not mapping:
            return None

        # Prefer the ``variable`` handler because Qt properties behave like
        # data members. Fall back to any available handler to avoid crashes
        # even if Breathe changes its internal registry names.
        for candidate in ("variable", "member", "function"):
            handler = mapping.get(candidate)
            if handler:
                break
        else:
            handler = next(iter(mapping.values()), None)

        if handler and "property" not in mapping:
            mapping["property"] = handler
        return handler

    fallback = _ensure_property_mapping("cpp_members")
    if fallback is None:
        fallback = _ensure_property_mapping("cpp_classes")
    elif hasattr(DomainDirectiveFactory, "cpp_classes"):
        DomainDirectiveFactory.cpp_classes.setdefault("property", fallback)

# -- Options for HTML output -----------------------------------------------

html_theme = "alabaster"
html_static_path = ["_static", "../images", "../favicons"]
html_css_files = ["main.css", "main-dark.css"]
html_js_files = ["main.js"]
html_title = "nymea Developer Documentation"
html_sidebars = {
    "**": [
        "about.html",
        "navigation.html",
        "relations.html",
        "searchbox.html",
    ]
}
html_theme_options = {
    "logo": "nymea-logo-light.svg",
    "logo_name": False,
    "description": "Guides, references and API documentation for nymea",
}

# -- Options for the Qt Help builder ---------------------------------------

qthelp_basename = "libnymea"
htmlhelp_basename = "libnymea"

# -- Misc ------------------------------------------------------------------

todo_include_todos = True

# Ensure that optional static assets exist. The build script will populate
# them, but having placeholders keeps "sphinx-build" happy when run manually.
for asset in ("main.css", "main-dark.css", "main.js", "nymea-logo-light.svg"):
    asset_path = PROJECT_ROOT / "_static" / asset
    if not asset_path.exists():
        asset_path.parent.mkdir(parents=True, exist_ok=True)
        asset_path.write_text("", encoding="utf-8")
