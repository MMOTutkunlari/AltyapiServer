echo "### Metin2 Game Builder ###"
echo "### Cleaning ...."
gmake clean
echo "### Building ###"
gmake -j20 > ./LIB_BUILD_LOG.txt 2> ./ERROR_LOG.txt
echo "### Done Building ###"