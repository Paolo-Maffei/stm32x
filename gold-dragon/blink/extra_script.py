Import("env")

platform = env.PioPlatform()

env.Prepend(
    UPLOADERFLAGS=["-s", platform.get_package_dir("tool-openocd") or ""]
)
env.Append(
    UPLOADERFLAGS=["-c", "program {$SOURCE} verify reset; shutdown"]
)
env.Replace(
    UPLOADER="openocd",
    UPLOADCMD="$UPLOADER $UPLOADERFLAGS"
)
