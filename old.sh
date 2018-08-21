#!/bin/bash

source ./bash-ini-parser

LANG=C++
ROOT=$PWD
DRY=false

cfg_parser "config.ini"
function Exec(){
  if [[ $DRY == true ]]; then
    echo "<DRY>$@"
  else
    local ret="$($@ 2>&1)"
    if [[ ! -z "$ret" ]]; then
      if [[ "$ret" == *"error"* ]]; then
        echo "<ERR>$ret"
      else
        echo "<WAR>$ret"
      fi
    fi
  fi
}

function Contains(){
  exclude=$1
  name=$(basename "$2")
  if [[ "${exclude[@]}" =~ "$name" ]]; then
    echo "1"
  else
    echo "0"
  fi
}

function CheckDep(){
  local deps="$2"
  if [[ -e "$1" ]]; then
    local dest=$(stat -c "%Y" "$1")
    local src=$(stat -c "%Y" "${deps[@]}")
    local set=false
    for i in "${src[@]}"; do
      if [[ $dest < $i ]]; then
        echo "1"
        set=true
        break
      fi
    done
    if [[ $set == false ]]; then
      echo "0"
    fi
  else
    echo "1"
  fi
}

function Scan(){
  printf "%b%s%b\n" "\033[1;35m" "Scaning dependencies for target $1" "\033[0m"
}
function LinkLib(){
  src="$2"
  if [[ $(CheckDep "$1" "${src[@]}") == 1 ]]; then
    printf "[%3i%%] %b%s%b\n" $3 "\033[1;32m" "Linking $LANG static library $1" "\033[0m"
    src+=("tmp.o")
    ret=$(Exec ar rvs $1 "${src[@]}")
    echo "$ret"
  fi
}
function Built(){
  printf "[%3i%%] %s\n" $2 "Build target $1"
}
function Return(){
  msg="$1"
  if [[ "$msg" == \<ERR\>* ]]; then
    msg="${msg:5:${#msg}-1}"
    msg="${msg//$'\n'/$'\n\033[1;31m>>>\033[0m    '}"
    printf "\033[A[%3i%%] %b%s%b\n" $5 "\033[1;31m" "Building $LANG $4 $2.o" "\033[0m"
    printf "\033[1;31m>>>\033[0m    %b\n" "$msg" 
  elif [[ "$msg" == \<WAR\>* ]]; then
    msg="${msg:5:${#msg}-1}"
    msg="${msg//$'\n'/$'\n\033[1;33m>>>\033[0m    '}"
    printf "\033[A[%3i%%] %b%s%b\n" $5 "\033[1;33m" "Building $LANG $4 $2.o" "\033[0m"
    printf "\033[1;33m>>>\033[0m    %b\n" "$msg" 
  elif [[ "$msg" == \<DRY\>* ]]; then
    msg="${msg:5:${#msg}-1}"
    printf ">>  %b\n" "$msg" 
  fi
  printf ""
}
function Build(){
  if [[ $(CheckDep "$i.o" "$i") == 1 ]]; then
    printf "[%3i%%] %b%s%b\n" $3 "\033[32m" "Building $LANG object $1.o" "\033[0m"
    ret=$(Exec clang -fcolor-diagnostics $2 -MMD -c $1 -o "$1.o")
    Return "$ret" $1 $2 "object" $3
  fi
}

function Target(){
  name=$1
  perc=$2
  step=$3
  Scan $name
  cfg_section_$name
  SOURCE=()
  while IFS= read -r -d $'\0'; do
    if [[ $(Contains "${exclude_file[@]}" "$REPLY") == 0 ]]; then
      SOURCE+=("$REPLY")
    fi
  done < <(find $source_dir/ -name "*.cpp" -print0)
  step=$(($step / (${#SOURCE[@]} + 1)))
  flags="-I$ROOT/$include_dir"
  for i in "${SOURCE[@]}"; do
    Build "$i" "$flags" $perc
    perc=$(($perc + $step))
  done
  OBJ=()
  for i in "${SOURCE[@]}"; do
    OBJ+=("$i.o")
  done
  LinkLib "$name" "${OBJ[@]}" $perc
  perc=$(($perc + $step))
  Built $name $perc
}

Target "libsigni.a" 0 100
# printf "%s\n" "${SOURCE[@]}"
