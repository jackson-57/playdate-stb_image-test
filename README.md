# playdate-stb_image-test
A minimal test app for the [Playdate](https://play.date/) game console that displays images of standard formats using a [fork](https://github.com/jackson-57/stb-playdate-fixes/blob/master/stb_image.h) of the [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) single-file library.

# Build instructions
Clone the repository, then clone the submodules using `git submodule update --init --recursive`. Build with CMake, following the instructions from [Inside Playdate](https://sdk.play.date/2.0.3/Inside%20Playdate%20with%20C.html#_command_line).