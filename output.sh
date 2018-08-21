#!/bin/bash


out::color(){
  if [[ $COLOR == true ]]; then
    printf "$1"
  else
    printf "\u200b"
  fi
}

out::error(){
  printf "%bError: %s%b\n" $(out::color "\033[1;31m") "$@" "$(out::color "\033[0m")"
}
out::warning(){
  printf "%bWarning: %s%b\n" $(out::color "\033[1;33m") "$@" "$(out::color "\033[0m")"
}
out::note(){
  printf "%bNote: %s%b\n" $(out::color "\033[1;34m") "$@" "$(out::color "\033[0m")"
}
