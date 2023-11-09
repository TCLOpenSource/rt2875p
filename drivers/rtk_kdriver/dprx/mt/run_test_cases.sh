TARGET_PATH=$1
echo "TARGET_PATH=$TARGET_PATH"
find $TARGET_PATH -name "Makefile" > test_items
sed -i {s/Makefile//} test_items
awk  '{printf "make clean -C %s; make -C %s -j 8\n", $1, $1}' test_items > .test_items.sh
rm test_items
sh .test_items.sh
