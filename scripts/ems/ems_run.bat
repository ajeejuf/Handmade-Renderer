@echo off

pushd ..\build\ems

python -m http.server 8080

popd