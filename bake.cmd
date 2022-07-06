@echo off
pushd %~pd0
clang -I inc -c src/unicode.c -o unicope.lib
popd
