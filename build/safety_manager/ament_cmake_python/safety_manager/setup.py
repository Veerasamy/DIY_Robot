from setuptools import find_packages
from setuptools import setup

setup(
    name='safety_manager',
    version='1.0.0',
    packages=find_packages(
        include=('safety_manager', 'safety_manager.*')),
)
