Import("env")

env.Append(
    CCFLAGS=[
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
    ],
    LINKFLAGS=[
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        # "-u,_printf_float",
    ]
)