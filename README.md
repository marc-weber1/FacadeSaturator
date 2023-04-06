# WINDOWS

msbuild FacadeSaturator.sln /t:FacadeSaturator-vst2 /p:Platform=x64

Required folder structure:
```
/FacadeSaturator
/pugl (https://github.com/lv2/pugl 6e42e48)
/iPlug2 (https://github.com/iPlug2/iPlug2 latest)
```