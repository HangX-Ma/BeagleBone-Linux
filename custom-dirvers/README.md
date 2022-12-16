# README

## IntelliSense Setting for Kernel Programing

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "<kernel directory prefix>/linux/include",
                "<kernel directory prefix>/linux/arch/arm/include",
                "<kernel directory prefix>/linux/arch/arm/include/generated"

            ],
            "defines": [
                "__GNUC__",
                "__KERNEL__",
                "MODULE"
            ],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```
