results=`licensecheck -r -c '\.(cpp|h)$' $1 | grep -v "GPL (v2)" | grep -v "BSD" | grep -v "GENERATED"`
if test -z "$results"; then
  exit 0
else
  echo "*** License check failed. Offending files:"
  licensecheck -r -c '\.(cpp|h)$' $1 | grep -v "GPL (v2)" | grep -v "GENERATED"
  exit 1
fi

