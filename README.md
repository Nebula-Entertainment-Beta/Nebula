# Nebula
Coog Interactive's Experimental Game Engine

## Building
Nebula currently uses the WAF build system, but it may be changed out for something else in the future. Building requires python to be installed.
```
python3 ./waf configure build
```
If you want to generate a Visual Studio SLN, then just append `msvs` to it.
```
python3 ./waf configure build msvs
```
Same for `xcode6`
```
python3 ./waf configure build xcode6
```
## Typing Conventions
- `camelCase` for variables
- `PascalCase` for functions
- Braces must be on their own lines:
```cpp
int main()
{

}
```
