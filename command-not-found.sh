# command-not-found.sh: Set command not found handler for Bash

function command_not_found_handle() {
  command-not-found-handler "${1}" "${CREW_PREFIX}/lib/crew/manifest"

  return 127
}
