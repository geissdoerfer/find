from setuptools import setup
from setuptools import find_namespace_packages
import pathlib

# The directory containing this file
HERE = pathlib.Path(__file__).parent

# The text of the README file
README = (HERE / "README.md").read_text()

setup(
    name="neslab-find",
    version="0.0.4",
    description="FIND model",
    long_description=README,
    long_description_content_type="text/markdown",
    author="Kai Geissdoerfer",
    author_email="kai.geissdoerfer@tu-dresden.de",
    packages=find_namespace_packages(include=["neslab.*"]),
    license="MIT",
    install_requires=["numpy", "scipy"],
    tests_require=["pytest"],
    extras_require={"examples": ["matplotlib"]},
    url="https://find.nes-lab.org",
)
