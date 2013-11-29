#! /bin/sh
#
# $Id$
#
#.intro: A pretty backend for mut.
#
# The idea is to present the information in a more readable form.
# 
#.status: partially works but definitely not production quality!

# The following is the default formatter.  Change it to whatever formatter
# you prefer or to "cat" if you don't have or perfer to to use a formatter.
formatter=fmt


display ()
{
  echo $message_text $1 | ${formatter}
  message_text=""
}


display_backtrace()
{
  if test -z "$1"
  then
    display_backtrace_result=$*
    return;
  fi
  nframes=$1
  shift 1
  while test $nframes -ne 0
  do
    if test "$1" = "?"
    then
      shift 2
      break;
    else
      echo "    " 0x$3 = $1\(0x$2+$4\) $5:$6;
      shift 6
      nframes=`expr $nframes - 1`;
    fi
  done
  display_backtrace_result=$*
}


#
# @@@ the mem.usage needs more work
#

info_message()
{
  msg=$1;  shift 1

  case $msg in

    trace)
      display "called $1($2)" ;;

    process.exited) ;;

    backtrace.off)
      display "not enough information available to provide
        a symbolic backtrace." ;;

    mem.usage)
      display "memory usage";
      display_backtrace $* ;;

    *)
      echo "FATAL ERROR: no handler for information message \`$msg'."
      echo "Please report this to nineties-retro@mail.com" ;;
  esac
}


ffm_error_tail()
{
   display "which had already been deallocated by _$1_"
   shift 1
   display_backtrace $*
}

wum_error_tail()
{
   display "allocated by _$1_"
   shift 1
   display_backtrace $*
}

#
# CMM and MMM messages should only occur if the malloc implementation has gone 
# haywire and so no effort has been made to generate a backtrace.
#

error_message()
{
  msg=$1;  shift 1

  case $msg in
    CMM)
      display "CMM: _$1_ has returned an object at address 0x$3 which
               was allocated by _$2_ and has not been deallocated." ;;
    FFM)
      display "FFM: attempt to deallocate object at address $2 by _$1_"
      shift 2;
      display_backtrace $*
      ffm_error_tail $display_backtrace_result ;;
    FUM)
      display "FUM: attempt to deallocate an unallocated object at
          address 0x$2 by _$1_"
      shift 2;
      display_backtrace $* ;;
    MLK)
      display "MLK: $2 byte object allocated by _$1_ at address 0x$3 has not
               been deallocated."
      shift 3
      display_backtrace $* ;;
    MMM)
      display "MMM: _$1_ has returned an object at address 0x$3 which was
        allocated by _$2_ and has not been deallocated." ;;
    WUM)
      display "WUM: write to unallocated memory at address 0x$4 detected
        during _$1_ of $3 byte object at address 0x$2" 
      shift 5;
      display_backtrace $*
      wum_error_tail $display_backtrace_result ;;
    *)
      echo "FATAL ERROR: no handler for error message \`$msg'."
      echo "Please report this to nineties-retro@mail.com" ;;
  esac
}


while read prog_name type rest
do
  message_text="$prog_name $type"
  case $type in
    error) error_message $message $rest ;;
    info) info_message $message $rest ;;
    *) echo "FATAL ERROR: no handler for message type \`$type'."
       echo "Please report this to nineties-retro@mail.com" ;;
  esac
done
