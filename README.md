# Surge
This is a small tool i made as a Surge deathblade main in lost ark.  
It gives audio feedback at 18,19,20 stacks.

It screenshots the Desktop, pattern checks the surge icon and if found runs Tesseract ORC over it 

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/aLArAlI8MC4/0.jpg)](https://www.youtube.com/watch?v=aLArAlI8MC4)

I wont provide binaries for security reasons and because its too much work.

## Building
You will need vcpkg, cmake and msvc.

* install Tesseract `vcpkg install tesseract --triplet x64-windows`
* create a `build` folder
* open a console in that folder
* run ` cmake -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ..` or replace the vcpkg path if you changed it
* run `cmake --build . --config Release`

This will build the exe and create it in Release. It spawns a tray icon so you dont have to start it from a terminal,
 but keep in mind it uses the assets folder relative so start it from the build folder.

## LICENSE
you can do what you want with this but im not responsible for anything.