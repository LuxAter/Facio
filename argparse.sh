#!/bin/bash

# argparse:: defines a group of functions that can be used to parse command
# line arguments and options.

argparse::help(){
  printf "%s\n\n" "Usage: build [options] [targets]"
  printf "  %s\n" "-h, --help         show list of command-line options"
  printf "  %s\n" "-d, --dry          displays commands without running them"
  printf "  %s\n" "-v=?, --verbose=?  sets verbose level(0-5)"
  printf "  %s\n" "-c, --color        enables colored output"
  printf "  %s\n" "-C, --no-color     disables colored output"

}

argparse::parse_opts(){
  RET_ARRAY=()
  while [[ $# -gt 0 ]]; do
    local key="$1"
    case $key in
      -h | --help)
        argparse::help
        exit 0
        ;;
      -d | --dry)
        DRY=true
        shift
        ;;
      -v | -v=* | --verbose | --verbose=*)
        if [[ "$key" == *=* ]]; then
          VERBOSE=${key#*=}
        else
          VERBOSE=1
        fi
        shift
        ;;
      -c | --color)
        COLOR=true
        shift
        ;;
      --no-color)
        COLOR=false
        shift
        ;;
      -*)
        local chars=($(echo "${key:1}" | sed -e 's/\(.\)/\1\n/g'))
        for l in "${chars[@]}"; do
          case $l in 
            h)
              argparse::help
              exit 0
              ;;
            d)
              DRY=true
              ;;
            v)
              VERBOSE=1
              ;;
            c)
              COLOR=true
              ;;
            C)
              COLOR=false
              ;;
            -)
              out::warning "Unknown option $key"
              break
              ;;
            *)
              out::warning "Unknown option -$l"
          esac
        done
        shift
        ;;
      *)
        if [[ "$key" == *=* ]]; then
          RET_ARRAY+=("$key")
        else
          RET_ARRAY_B+=("$key")
        fi
        shift
    esac
  done
}
