echo "### Cleaning for build ###"
gmake clean
echo "### Starting DB build ###"
gmake -j20 > ../DB_BUILD_LOG.txt 2> ../ERROR_LOG.txt
