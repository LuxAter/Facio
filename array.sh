#!/bin/bash

# array:: defines a group of functions that can be used to manipulate arrays more
# conviently. By using a specialized return array for the returned values.

array::index(){
  local val="$1"
  shift
  local arr=("$@")
  local found=false
  for i in "${!arr[@]}"; do
    if [[ "${arr[$i]}" == "${val}" ]]; then
      RET_INT=$i
      echo "${i}"
      found=true
      break
    fi
  done
  if [[ $found == false ]]; then
    RET_INT="${#arr[@]}"
    echo "${#arr[@]}"
  fi
}
array::prepend(){
  RET_ARRAY=("$@")
}
array::append(){
  local val="$1"
  shift
  RET_ARRAY=("$@" "$val")
}
array::insert(){
  local val="$1"
  local pos="$2"
  shift 2
  local arr=("$@")
  RET_ARRAY=()
  for i in $(seq 0 $((${#arr[@]}))); do
    if [[ $i -lt $pos ]]; then
      RET_ARRAY+=("${arr[$i]}")
    elif [[ $i -eq $pos ]]; then
      RET_ARRAY+=("$val")
    else
      RET_ARRAY+=("${arr[$(($i-1))]}")
    fi
  done
}
array::delete_id(){
  local pos="$1"
  shift
  local arr=("$@")
  RET_ARRAY=()
  for i in $(seq 0 $((${#arr[@]} - 2))); do
    if [[ $i -lt $pos ]]; then
      RET_ARRAY+=("${arr[$i]}")
    else
      RET_ARRAY+=("${arr[$(($i+1))]}")
    fi
  done
}
array::delete_val(){
  local val="$1"
  shift
  local arr=("$@")
  local index=$(array::index "$val" "${arr[@]}")
  array::delete_id $index "${arr[@]}"
}
array::replace_id(){
  local val="$1"
  local pos="$2"
  shift 2
  local arr=("$@")
  array::insert "$val" $pos "${arr[@]}"
  arr=("${RET_ARRAY[@]}")
  array::delete_id $(($pos+1)) "${arr[@]}"
}
array::replace_val(){
  local find="$1"
  local rep="$2"
  shift 2
  local arr=("$@")
  local index=$(array::index "$find" "${arr[@]}")
  array::replace_id "$rep" $index "${arr[@]}"
}
array::is_in(){
  local val="$1"
  shift
  local arr=("$@")
  local index=$(array::index "$val" "${arr[@]}")
  if [[ $index -eq "${#arr[@]}" ]]; then
    RET_BOOL=false
  else
    RET_BOOL=true
  fi
  echo $RET_BOOL
}
array::prepend_arr(){
  local len=$1
  shift
  RET_ARRAY=("$@")
}
array::append_arr(){
  local len=$1
  shift
  local arr1=()
  local arr2=()
  for i in $(seq 0 $(($len - 1))); do
    arr1+=("$1")
    shift
  done
  arr2=("$@")
  RET_ARRAY=("${arr2[@]}" "${arr1[@]}")
}
array::insert_arr(){
  local len=$1
  shift
  local arr1=()
  local arr2=()
  for i in $(seq 0 $(($len - 1))); do
    arr1+=("$1")
    shift
  done
  local pos=$1
  shift
  arr2=("$@")
  RET_ARRAY=()
  for i in $(seq 0 $((${#arr1[@]} + ${#arr2[@]} - 1))); do
    if [[ $i -lt $pos ]]; then
      RET_ARRAY+=("${arr2[$i]}")
    elif [[ $i -eq $pos ]] || [[ $i -lt $(($pos + $len)) ]]; then
      RET_ARRAY+=("${arr1[$(($i-$pos))]}")
    else
      RET_ARRAY+=("${arr2[$(($i-$len))]}")
    fi
  done
}
array::replace_arr(){
  local len=$1
  shift
  local arr1=()
  local arr2=()
  for i in $(seq 0 $(($len - 1))); do
    arr1+=("$1")
    shift
  done
  local pos=$1
  shift
  arr2=("$@")
  RET_ARRAY=()
  array::insert_arr $len "${arr1[@]}" $pos "${arr2[@]}"
  arr2=("${RET_ARRAY[@]}")
  array::delete_id $(($pos+$len)) "${arr2[@]}"
}
array::ret_array(){
  RET_ARRAY=("$@")
}
