#!/bin/sh
cd "$(dirname "$0")"
tar -cLf ../../kernel/amd64/sys.tar dev bin
