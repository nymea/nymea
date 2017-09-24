results=`licensecheck -r -c '\.(cpp|h)$' $1 | grep -v "GPL (v2)" | grep -v "GPL (v3)" | grep -v "BSD" | grep -v "GENERATED" | grep -v "LGPL (v2.1 or later)" | grep -v "Apache (v2.0)"`
if test -z "$results"; then
  exit 0
else
  echo "*** License check failed. Offending files:"
  licensecheck -r -c '\.(cpp|h)$' $1 | grep -v "GPL (v2)" | grep -v "GPL (v2)" | grep -v "GPL (v3)"  | grep -v "BSD" | grep -v "GENERATED" | grep -v "LGPL (v2.1 or later)" | grep -v "Apache (v2.0)"
  exit 1
fi

