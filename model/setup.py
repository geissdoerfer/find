from setuptools import setup
from setuptools import find_namespace_packages

setup(
    name="neslab-find",
    version="0.0.2",
    description="FIND model",
    author="Kai Geissdoerfer",
    author_email="kai.geissdoerfer@tu-dresden.de",
    packages=find_namespace_packages(include=["neslab.*"]),
    license="MIT",
    install_requires=["numpy", "scipy"],
    setup_requires=["pytest-runner"],
    tests_require=["pytest>=3.9"],
    url="https://find.nes-lab.org",
)
