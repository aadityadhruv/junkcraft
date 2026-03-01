#!/bin/bash

for file in $(ls -1 ~/Sprites); do
    output=$(basename -s ".aseprite" ${file})
    ~/git/tools/aseprite/build/bin/aseprite -b ~/Sprites/${file} --sheet textures/${output}.png
done

