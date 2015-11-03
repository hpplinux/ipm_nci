 awk -F '|' 'BEGIN{print "<mpi>"};{print "<func idv=\""$1"\" idl=\""$2"\" >"; print "<c_api>"$3"</c_api>"; print "<f_api>"$4"</f_api>"; print $5; print "</func>";};END{print "</mpi>"}' mpi_key
