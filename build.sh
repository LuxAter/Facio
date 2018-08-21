#!/bin/bash
ROOT=$PWD
COLOR=true
VERBOSE=false
DRY=false
source ./ret.sh
source ./output.sh
source ./bash-ini-parser
source ./array.sh
source ./argparse.sh
source ./math.sh

ini::parser "config.ini"

trap "exit 0" TERM
export TOP_PID=$$

#Settings{{{
ROOT=$PWD
POSITIONAL=()
DRY=false
HIDE=false
PERC=0
DISP_PERC=0
#}}}
#Util{{{
function Round() {
  echo $(printf %.0f $(echo "scale=0;(((10^0)*$1)+0.5)/(10^0)" | bc))
}
function Contains(){
  local e match="$1"
  shift
  for e; do [[ "$e" == "$match" ]] && echo 1; done
  echo 0
}
function Join(){ local IFS="$1"; shift; echo "$*"; }
function Replace() {
  local index=$1
  shift
  local length=$1
  shift
  local insert=()
  for i in $(seq 0 $(($length-1))); do
    insert+=("$1")
    shift
  done
  local array="$@"
  local ret=()
  for i in $(seq 0 $(("${#insert[@]}" + "${#array[@]}" - 2))); do
    if [[ $i < $index ]]; then
      ret+=("${array[$i]}")
    elif [[ $i == $index ]] || [[ $i < $(($index + $length)) ]]; then
      ret+=("${insert[$(($i-$index))]}")
    else
      ret+=("${array[$(($i-$length))]}")
    fi
  done
  echo "${ret[@]}"
}
function Load(){
  unset type
  unset dep
  unset source_dir
  unset source_file
  unset include_dir
  unset include_file
  unset build_dir
  unset exclude_dir
  unset exclude_file
  unset default
  cfg_section_$name
}
#}}}
#ArgParse{{{
function Help(){
  printf "\n"
}

function ParseOpts(){
  while [[ $# -gt 0 ]]; do
    local key="$1"
    case $key in
      -h | --help)
        Help
        kill -s TERM $TOP_PID
        ;;
      -d | --dry)
        DRY=true
        shift
        ;;
      -*)
        local chars=($(echo "${key:1}" | sed -e 's/\(.\)/\1\n/g'))
        for l in "${chars[@]}"; do
          case $l in
            d)
              DRY=true
              ;;
            *)
              printf "Unknown option -%s\n" "$l"
              ;;
          esac
        done
        shift
        ;;
      *)
        POSITIONAL+=("$key")
        shift
        ;;
    esac
  done
}
#}}}
#Dependency management{{{
function ScanDeps(){
  local files=()
  if [[ "$1" == "lib" ]]; then
    while IFS= read -r -d $'\0'; do
      if [[ $(Contains $(basename "$REPLY") ${exclude_file[@]}) == 0 ]]; then
        local match=false
        for dir in "${exclude_dir[@]}"; do
          if [[ "$REPLY" == */"$dir"/* ]]; then
            match=true
            break
          fi
        done
        if [[ $match == false ]]; then files+=("$REPLY.o"); fi
      fi
    done < <(find $source_dir/ -name "*.cpp" -print0)
  elif [[ "$1" == "obj" ]]; then
    local src=$2
    local dep="${src/%.o/.d}"
    if [[ -e "$dep" ]];then
      local file=$(<"$dep")
      file="${file#*: }"
      file=("${file//$' \\\n  '/$'\n'}")
      files=($file)
    else
      files+=("${src/%.o/}")
    fi
  elif [[ "$1" == "exe" ]]; then
    local src="$2"
    local dep="${src//.d}"
    if [[ -e "$dep" ]]; then
      local file=$(<"$dep")
      file=("${file//$' \\\n  '/$'\n'}")
      files=($file)
    fi
  fi
  if [[ ! -z "${source_file[@]}" ]]; then
    for i in $(seq 0 $((${#source_file[@]}-1))); do
      source_file[$i]="${source_file[$i]}.o"
    done
    files=("${files[@]}" "${source_file[@]}")
  fi
  if [[ ! -z "${dep[@]}" ]]; then
    files=("${files[@]}" "${dep[@]}")
  fi
  echo "${files[@]}"
}
function CheckDep(){
  local dest_file="$1"
  shift
  local deps=("$@")
  if [[ -e "$dest_file" ]]; then
    local dest=$(stat -c "%Y" "$dest_file")
    local src=()
    for d in "${deps[@]}"; do
      src+=($(stat -c "%Y" "$d"))
    done
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
#}}}
#Formatting{{{
function FormatError(){
  local str="$1"
  local ret="$2"
  printf "\033[A[%3i%%] %b%s%b\n" $DISP_PERC "\033[1;31m" "$str" "\033[0m"
  ret="${ret//$'\n'/$'\n\033[1;31m>>\033[0m  '}"
  ret="${ret//$'\\n'/'\\n'}"
  if [[ $HIDE == false ]] || [[ $HIDE -ge 1 ]]; then
    printf "\033[1;31m>>\033[0m  %b\n" "$ret"
  fi
}
function FormatWarning(){
  local str="$1"
  local ret="$2"
  printf "\033[A[%3i%%] %b%s%b\n" $DISP_PERC "\033[1;33m" "$str" "\033[0m"
  ret="${ret//$'\n'/$'\n\033[1;33m>>\033[0m  '}"
  ret="${ret//$'\\n'/'\\n'}"
  if [[ $HIDE == false ]] || [[ $HIDE -ge 2 ]]; then
    printf "\033[1;33m>>\033[0m  %b\n" "$ret"
  fi
}
function FormatNote(){
  local str="$1"
  local ret="$2"
  printf "\033[A[%3i%%] %b%s%b\n" $DISP_PERC "\033[1;36m" "$str" "\033[0m"
  ret="${ret//$'\n'/$'\n\033[1;36m>>\033[0m  '}"
  ret="${ret//$'\\n'/'\\n'}"
  if [[ $HIDE == false ]] || [[ $HIDE -ge 3 ]]; then
    printf "\033[1;36m>>\033[0m  %b\n" "$ret"
  fi
}
function ParseRet(){
  local cmd="$1"
  local str="$2"
  local ret="$3"
  if [[ $DRY == true ]]; then
    echo "$ret"
  elif [[ "$cmd" == "build" ]]; then
    if [[ "$ret" == *"error:"* ]]; then
      FormatError "$str" "$ret"
    elif [[ "$ret" == *"warning:"* ]]; then
      FormatWarning "$str" "$ret"
    elif [[ "$ret" == *"note:"* ]]; then
      FormatNote "$str" "$ret"
    fi
  # elif [[ "$cmd" == "linklib" ]]; then
  #
    # FormatNote "$str" "$ret"
  fi
}
#}}}
function Exec(){
  if [[ $DRY == true ]]; then
    echo ">>  $@"
  else
    echo "$($@ 2>&1)"
  fi
}
function Build(){
  local lang="C++"
  local deps=($(ScanDeps "obj" $1))
  deps=(${deps//' '/$'\n'})
  if [[ $(CheckDep "$1" "${deps[@]}") == 1 ]]; then
    printf "[%3i%%] %b%s%b\n" $DISP_PERC "\033[32m" "Building $lang object $1" "\033[0m"
    ret="$(Exec clang++ -fcolor-diagnostics $2 -MMD -c ${1/%.o/} -o "$1")"
    ParseRet "build" "Building $lang object $1" "$ret"
  fi
}
function LinkLib(){
  local lang="C++"
  local name="$1"
  shift
  local deps=("$@")
  if [[ $(CheckDep "$name" "${deps[@]}") == 1 ]]; then
    printf "[%3i%%] %b%s%b\n" $DISP_PERC "\033[1;32m" "Linking $lang static library $name" "\033[0m"
    ret="$(Exec ar rvs $name "${deps[@]}")"
    ParseRet "linklib" "Linking $lang static library $name" "$ret"
  fi
}
function LinkExe(){
  local lang="C++"
  local name="$1"
  local flags="$2"
  shift
  shift
  local deps=("$@")
  if [[ $(CheckDep "$name" "${deps[@]}") == 1 ]]; then
    printf "[%3i%%] %b%s%b\n" $DISP_PERC "\033[1;32m" "Linking $lang executable $name" "\033[0m"
    ret="$(Exec clang++ -fcolor-diagnostics $flags "${deps[@]}" "${dep[@]}" -o "$name")"
    ParseRet "build" "Linking $lang executable $name" "$ret"
  fi
}
function Target(){
  local name=$1
  Load $name
  printf "%b%s%b\n" "\033[1;35m" "Scaning dependencies for target $name" "\033[0m"
  local deps=("$(ScanDeps $type)")
  local step=$(echo "$STEP / (${#deps[@]} + 2)" | bc -l)
  local flags="-I$ROOT/$include_dir"
  PERC=$(echo "$PERC + $step" | bc -l)
  DISP_PERC=$(Round $PERC)
  for src in "${deps[@]}"; do
    Build "$src" "$flags"
    PERC=$(echo "$PERC + $step" | bc -l)
    DISP_PERC=$(Round $PERC)
  done
  if [[ "$type" == "lib" ]]; then
    LinkLib "$name" "${deps[@]}"
  elif [[ "$type" == "exe" ]]; then
    LinkExe "$name" "$flags" "${deps[@]}"
  fi 
  PERC=$(echo "$PERC + $step" | bc -l)
  DISP_PERC=$(Round $PERC)
  printf "[%3i%%] %s\n" $DISP_PERC "Built target $1"
}
function Clean(){
  local step=$(echo "$STEP / (0 + 1)" | bc -l)
  PERC=$(echo "$PERC + $step" | bc -l)
  DISP_PERC=$(Round $PERC)
  printf "[%3i%%] %b%s%b\n" $DISP_PERC "\033[1;34m" "Cleaned build files" "\033[0m"
}

# ParseOpts "$@"
# for arg in "${POSITIONAL[@]}"; do
#   if [[ "$arg" == *"="* ]]; then
#     var=(${arg//=/ })
#     declare "G_${var[0]}"="${var[1]}"
#     POSITIONAL=(${POSITIONAL[@]/$arg})
#   fi
# done
# OPTIONS=($(cat "config.ini" | grep "^\[.*\]$"))
# for i in $(seq 0 $((${#OPTIONS[@]}-1))); do
#   str=${OPTIONS[$i]}
#   str=${str:1:${#str}-2}
#   OPTIONS[$i]=$str
# done
# if [[ ${#POSITIONAL[@]} == 0 ]]; then
#   POSITIONAL=("${OPTIONS[@]}")
# fi
# echo "${POSITIONAL[@]}::${#POSITIONAL[@]}"
# for i in $(seq 0 $((${#POSITIONAL[@]} - 1))); do
#   if [[ "${POSITIONAL[$i]}" == "all" ]]; then
#     echo ">${POSITIONAL[@]}::${#POSITIONAL[@]}"
#     POSITIONAL=("$(Replace $i "${#OPTIONS[@]}" "${OPTIONS[@]}" "${POSITIONAL[@]}")")
#     echo ">${POSITIONAL[@]}::${#POSITIONAL[@]}"
#   fi
# done
# echo "${POSITIONAL[@]}::${#POSITIONAL[@]}"
# TARGETS=()
# for target in "${POSITIONAL[@]}"; do
#   if [[ "$target" != "clean" ]]; then
#     cfg_section_$target
#   fi
#   if [[ "$default" != "false" ]]; then
#     if [[ ! -z "${dep[@]}" ]]; then
#       for d in "${dep[@]}"; do
#         if [[ $(Contains "${TARGETS[@]}" "$d") == 0 ]]; then
#           TARGETS+=("$d")
#         fi
#       done
#     fi
#     if [[ $(Contains "$target" "${TARGETS[@]}") == 0 ]]; then
#       TARGETS+=("$target")
#     fi
#   fi
# done
# STEP=$(echo "100.0 / ${#TARGETS[@]}" | bc -l)
# for target in "${TARGETS[@]}"; do
#   printf "$target\n"
# done

ini::get_sections "config.ini"
ALL_TARGETS=("${RET_ARRAY[@]}")
argparse::parse_opts "$@"
TARGETS=("${RET_ARRAY_B[@]}")
OPTIONS=("${RET_ARRAY[@]}")
ret::clear
if [[ $(array::is_in "all" "${TARGETS[@]}") == true ]]; then
  array::replace_arr "${#ALL_TARGETS[@]}" "${ALL_TARGETS[@]}" $(array::index "all" "${TARGETS[@]}") "${TARGETS[@]}"
  TARGETS=("${RET_ARRAY[@]}")
elif [[ -z "${TARGETS[@]}" ]]; then
  for target in "${ALL_TARGETS[@]}"; do
    ini::section_$target
    if [[ "$run" != "false" ]]; then
      TARGETS+=("$target")
    fi
  done
fi
echo "${TARGETS[@]}"
echo "${OPTIONS[@]}"
echo "${ALL_TARGETS[@]}"

