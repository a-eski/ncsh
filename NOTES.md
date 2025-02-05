# ncsh Notes, References, Bugs, and Random Stuff

## References

### Misc

* [bash](https://aosabook.org/en/v1/bash.html)
* [ascii codes](https://theasciicode.com.ar/)
* [gcc options](https://gcc.gnu.org/onlinedocs/gcc-10.4.0/gcc/Instrumentation-Options.html)
* [XTerm control sequences](http://invisible-island.net/xterm/ctlseqs/ctlseqs.html)
* [VT220 Emulation](http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html)

### Other Shells in C

* [PShell](https://github.com/PhilippRados/PShell/tree/master)
* [psh](https://github.com/proh14/psh)
* [esh](https://github.com/aperezdc/esh)
* [zsh](https://github.com/zsh-users/zsh)
* [bash](https://github.com/bminor/bash)
* [esh google](https://github.com/google/esh)

### Libraries Considered

#### Line Readers

* [linenoise](https://github.com/antirez/linenoise/blob/master/README.markdown)
* [termbox2](https://github.com/termbox/termbox2/blob/master/README.md)
* [tuibox](https://github.com/Cubified/tuibox)
* [isocline](https://github.com/daanx/isocline/blob/main/src/common.h)
* [gnu readline](https://savannah.gnu.org/git/?group=readline)

#### curses/ncurses-like Libraries

* [notcurses](https://github.com/dankamongmen/notcurses?tab=readme-ov-file)
* [CursedGl](https://github.com/saccharineboi/CursedGL)

### fzf

* [fzf-native original](https://github.com/nvim-telescope/telescope-fzf-native.nvim)
* [fzf-native fork](https://github.com/a-eski/telescope-fzf-native.nvim)

## Bugs

* handle deleting when input is over multiple lines
* handle copying/pasting when input is multiple lines
* handle copying/pasting when input has spaces

## Windows Notes

``` C
#ifdef WIN32 || _WIN32 || _WIN64

#endif // WIN32

#ifdef linux || __unix__

#endif // linux

//CreateProcess
```

## Fuzzing Notes

* [llvm libFuzzer](https://llvm.org/docs/LibFuzzer.html#corpus)
* [libFuzzer tutorial](https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md#seed-corpus)

## Z Notes

* z add /mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh
* z add /mnt/c/Users/Alex/source/repos/PersonalRepos/ttytest2
* z print

## Docker Notes

``` sh
sudo docker build . --tag ncsh-docked --file ./dockerfile
sudo docker run -d ncsh-docked
docker run -ti --rm -v $(shell pwd):/docker ncsh-docked "make && ruby ./src/integration_tests/tests.rb"

sudo docker run -ti --rm -v $(shell pwd):/ncsh ncsh-docked "make && ruby ./integration_tests/integration_test.rb"

sudo docker run -ti --rm -v $(shell pwd):/ncsh ncsh-docked "./tests_it.sh"

learning
sudo docker build -t ncsh-docked .
sudo docker run ncsh-docked
```
