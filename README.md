# mite 
mite is a "lightweight" modal command line text editor inspired by vim

### Why?
I am creating mite as a learning project, and it has no real purpose and/or defining features making it worth using over vim

### What's with the name?
mite is an acronym for "mild inefficient text editor"

The name comes from [idkso's](https://github.com/idkso) critism of my usage of a vector data structor instead of something more efficient like ropes


## Building
```bash
git clone https://github.com/KaffeinatedKat/mite
cd mite
clang++ src/main.cpp -o mite
```

## Usage
`mite [file]`

### Controls 
mite uses bindings identical to vim

`hjkl` movement keys

`i` for insert mode

`x` to delete

`esc` to go back to command mode

*vim like command line is currently not implimented*

`w` writes the file

`q` to exit
