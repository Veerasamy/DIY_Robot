import os
from glob import glob
from setuptools import find_packages, setup

package_name = 'speedbot_control'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        # Config files
        (os.path.join('share', package_name, 'config'),
            glob('config/*.yaml')),
        # Launch files
        (os.path.join('share', package_name, 'launch'),
            glob('launch/*launch.[pxy][yma]*')),
    ],
    install_requires=['setuptools', 'pyserial'],
    zip_safe=True,
    maintainer='Arnab',
    maintainer_email='Arnab@todo.todo',
    description='Control stack for autonomous speed course robot (Ackermann steering)',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'drive_controller = speedbot_control.drive_controller_node:main',
            'motor_bridge = speedbot_control.motor_bridge_node:main',
        ],
    },
)

