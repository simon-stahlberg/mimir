import os
import sys
from skbuild import setup

# Set the directory containing your CMakeLists.txt
cmake_dir = os.path.abspath(os.path.dirname(__file__))

setup(
    name='mimir',
    version='0.1.0',
    description='Mimir: Lifted PDDL parsing and expansion library.',
    author='Simon Stahlberg',
    author_email='simon.stahlberg@gmail.com',
    cmake_args=[
        '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' +
            os.path.join(cmake_dir, 'mimir')
    ],
    packages=['mimir'],
    cmake_source_dir='.'
)
