#!/bin/bash
node cmdgen.js
cc cmds.c t0.c -Wall -g -o t0
