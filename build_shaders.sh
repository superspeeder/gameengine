#!/bin/bash

find assets/shaders/ ! -name *.spv -type f -exec glslc {} -o {}.spv \; -exec echo "Compiled shader {} -> {}.spv" \;
